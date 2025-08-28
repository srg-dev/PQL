#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#ifndef MAC
#include <sys/types.h>
#endif
//#include <sys/stat.h>           // stat(), etc
//#include <unistd.h>             // getcwd(), etc
#include "pql_defines.h"
#include "pql_externs.h"
#include "libmseed.h"
#include "steim_DMC.h"
#ifdef SOLARIS2
#include "libgen.h"
#endif
#include "pql_mseed.h"

static GStaticMutex mplexMutex = G_STATIC_MUTEX_INIT;		// mutex to manage mplex_files
static MPLX_MSEED	*mplex_options;			// multi-plex read options
static GHashTable	*mplex_files;			// hash table of MMINDEX lists

static gint getChannel(MM_TRCINFO *one, char *channelName)
{
	return(strcmp(one->channelName, channelName));
}

static gint chanFind(char *a, char *b)
{
	return (strcmp(a, b));	
}

static gboolean includeChan(char *channel)
{
	// first check against our list of channels we absolutely ignore
	if (!strcmp(channel, "ACE") ||			// ACE channel ignored
		!strcmp(channel, "LOG") ||			// LOG channel ignored
		!strcmp(channel, "OCF"))			// OCF channel ignored
		return FALSE;
		
	if (!mplex_options)
		return TRUE;

	// check against our include list, if specified
	if (mplex_options->incChans)
	{	// include list / exclude list mutually exclusive
		if (g_slist_find_custom(mplex_options->incChans, channel, (GCompareFunc) chanFind))
			return TRUE;
		return FALSE;
	}
	
	// check against our exclude list, if specified
	if (mplex_options->excChans)
	{
		if (g_slist_find_custom(mplex_options->excChans, channel, (GCompareFunc) chanFind))
			return FALSE;
	}
	
	return TRUE;
}

static gint timeOrder(MMPACKET *one, MMPACKET *two)
{
	gint64 diff = (one->startTime - two->startTime);
	return((diff > 0) ? 1 : (diff < 0) ? -1 : 0);
}

static gint packetOrder(MMPACKET *one, MMPACKET *two)
{
	gint64 diff = (one->index - two->index);
	return((diff > 0) ? 1 : (diff < 0) ? -1 : 0);
}

static gint getChannelIndex(MMINDEX *one, char *channelName)
{
	return(strcmp(one->channelName, channelName));
}

static void free_MMFILE(MMFILE *MM_file)
{
	MMINDEX		*mmindex;
	GSList		*iterIndex, *iterPacket;
	
	// free the MMINDEX data
	for(iterIndex = MM_file->channels;
		iterIndex;
		iterIndex = g_slist_next(iterIndex))
	{	
		mmindex = iterIndex->data;
		free(mmindex->channelName);
		free(mmindex->network);
		free(mmindex->station);
		free(mmindex->location);
		free(mmindex->channel);
		for(iterPacket = mmindex->packets;
			iterPacket;
			iterPacket = g_slist_next(iterPacket))
		{
			free(iterPacket->data);
		}
		g_slist_free(mmindex->packets);
		free(mmindex);
	}
	g_slist_free(MM_file->channels);
	free(MM_file->fileName);
	free(MM_file);
}

static void setMMTRCINFO(MM_TRCINFO *trcInfo, MSRecord *msr)
{
	trcInfo->network = msr->network;
	trcInfo->station = msr->station;
	trcInfo->location = msr->location;
	trcInfo->channel = msr->channel;
	trcInfo->sampRate = msr->samprate;
	trcInfo->sampleCnt = msr->samplecnt;
	trcInfo->encoding = msr->encoding;
	trcInfo->startTime = msr->starttime;
}

static void setStartTime(hptime_t startTime, traceInfo *trc)
{
	BTime		btime;
	struct ptime    date;

	trc->data.head.MSstartTime = startTime;
  	ms_hptime2btime(startTime, &btime);
	trc->data.head.year = btime.year;
	trc->data.head.day = btime.day;
	trc->data.head.hour = btime.hour;
	trc->data.head.minute = btime.min;
	trc->data.head.second = btime.sec;
	trc->data.head.m_secs = btime.fract/10;

	date.yr = trc->data.head.year;
	date.day = trc->data.head.day;
	date.hr = trc->data.head.hour;
	date.mn = trc->data.head.minute;
	date.sec = trc->data.head.second;

	if (trc->data.head.descr1)
		free(trc->data.head.descr1);
	trc->data.head.descr1 = g_strdup_printf("%i %03i %02i:%02i" ,
	    date.yr, date.day, date.hr, date.mn);

	trc->data.head.epochs = passcal_htoe(&date, FALSE) + trc->data.head.m_secs/1000.;
}

static int setHedrVals(MM_TRCINFO *trc, traceInfo *trace)
{
	switch(trc->encoding)
  	{
		case 0:		// DE_ASCII
			trace->data.head.data_form = MY_CHAR;
			trace->data.head.dataSize = sizeof(char);
		break;

		case 1:		// DE_INT16
		case 3:		// DE_INT32
		case 10:	// DE_STEIM1
		case 11:	// DE_STEIM2
		case 30:	// DE_SRO
		case 32:	// DE_DWWSSN
			trace->data.head.data_form = BIT32; 
			trace->data.head.dataSize = sizeof(int);
		break;

		case 4:		// DE_FLOAT32
			trace->data.head.data_form = MY_FLOAT; 
			trace->data.head.dataSize = sizeof(float);
		break;

		case 5:		// DE_FLOAT64
			trace->data.head.data_form = MY_DOUBLE; 
			trace->data.head.dataSize = sizeof(double);
		break;

		case 12:	// DE_GEOSCOPE24
		case 13:	// DE_GEOSCOPE163
		case 14:	// DE_GEOSCOPE164
			trace->data.head.data_form = MY_FLOAT; 
			trace->data.head.dataSize = sizeof(float);
		break;

		default:	// UNDEFINED and UNKNOWN
			fprintf(stderr, "%s: Unknown encoding type: %d\n", 
						trace->data.fileName, trc->encoding);
			return(FILE_READ_ERROR);
		break;
  	}
  	setStartTime(trc->startTime, trace);
	trace->data.head.sampInt = SAMPINT(trc->sampRate);
	return(PQLXSUCCESS);
}

static traceInfo *make_mseed_trace(MM_TRCINFO *trc, int fileType)
{
	char		*chr;
	traceInfo	*trace;

	trace = calloc(1, sizeof(traceInfo));
	
	trace->data.fileType = fileType;
  	trace->data.head.units = PQL_UNITS_COUNTS;
	trace->data.fileName = g_strdup(trc->fileName);

	setHedrVals(trc, trace);
	
	trace->data.head.gainConst = 1;
	trace->data.head.scale_fac = 1;
	strncpy(trace->data.head.network, trc->network, 2);
	strncpy(trace->data.head.station, trc->station, 5);
	strncpy(trace->data.head.location, trc->location, 2);
	strncpy(trace->data.head.channel, trc->channel, 3);
	g_strstrip(trace->data.head.network);
	g_strstrip(trace->data.head.station);
	g_strstrip(trace->data.head.location);
	g_strstrip(trace->data.head.channel);

	trace->data.dataInfo.head.numSamples = trc->sampleCnt;

	trace->data.head.descr2 = g_strdup_printf("%s.%s.%s.%s",
		trace->data.head.network,
		trace->data.head.station,
		trace->data.head.location,
		trace->data.head.channel);

	trace->data.head.longFN = trace->data.fileName;
	switch(fileType)
	{
		case MSEED_DATA:
			trace->data.head.shortFN = 
#ifndef WIN32
				g_strdup((chr = strrchr(trace->data.fileName,'/')) ? chr+1 : trace->data.fileName);
#else
				g_strdup((chr = strrchr(trace->data.fileName,'\\')) ? chr+1 : trace->data.fileName);
#endif
		break;
		case MSEED_MPLEX_DATA:
			trace->data.head.shortFN = 
#ifndef WIN32
						g_strdup_printf("%s (%s)", 
						((chr = strrchr(trace->data.fileName,'/'))
						? chr+1 : trace->data.fileName),
						trc->channel);
#else
						g_strdup_printf("%s (%s)", 
						((chr = strrchr(trace->data.fileName,'\\'))
						? chr+1 : trace->data.fileName),
						trc->channel);
#endif
			trace->mplex.packets = trc->packets;
			// order the trace packets by index/packet number
			// this is necessary since this is the order the packets are read from the file
			// and this must be sequential
			trace->mplex.packets = g_slist_sort(trace->mplex.packets, (GCompareFunc) packetOrder);
			trace->mplex.blkSize = trc->blkSize;
		break;
	}

	return trace;
}

static MMFILE *index_mplex(char *fileName)
{
	MMFILE		*mmfile;
	MMINDEX		*mmindex;
	MMPACKET	*mmpacket;
	MSRecord	*msr=NULL;
	MSFileParam *msfp=NULL;
	GSList		*chn, *iterIndex;
	int 		packet_num = 0, retVal, ttlChans=0;
	char		channelName[35];
	GSList		*MM_index = NULL;
	int			blkSize;

	// index a file only once
	if (mplex_files &&
		(mmfile = g_hash_table_lookup(mplex_files, fileName)))
	{
		return mmfile;
	}

	// read all headers, indexing each packet accordingly
	retVal = READMSRNDATA(&msfp, &msr, fileName);
	if (retVal != MS_NOERROR)
	{	// read failure, shouldn't happen                             
		CLOSEMSR(&msfp, &msr);
		return NULL;
	}

	mmfile = calloc(1, sizeof(MMFILE));
	mmfile->fileName = g_strdup(fileName);
	blkSize = mmfile->blkSize = msr->reclen;

	for(packet_num = 0;
		retVal == MS_NOERROR;
		retVal = READMSRNDATA(&msfp, &msr, fileName), packet_num++)
	{
		if (!(ISVALIDENCODING(msr->encoding))) 		// not a valid encoding
			continue;
		
		if (blkSize &&							// once 0, no need to ever check again
			(blkSize - msr->reclen))			// block size has changed
		{
			mmfile->blkSize = 0;				// downstream indicator of multiple block sizes
			blkSize = 0;
		}
		
		sprintf(channelName, "%s.%s.%s.%s", 
						msr->network, msr->station,
						msr->location, msr->channel);
//fprintf(stderr, "chn: %s\n", channelName);
		chn = g_slist_find_custom(MM_index, channelName, (GCompareFunc) getChannelIndex);
		if (chn)
		{	// channel already defined, add the packet info to the packet list
			mmindex = chn->data;
			mmindex->sampleCnt += msr->samplecnt;
			mmpacket = calloc(1, sizeof(MMPACKET));
			mmpacket->index = packet_num;
			mmpacket->startTime = msr->starttime;
			mmpacket->sampleCnt = msr->samplecnt;
			mmindex->packets = g_slist_prepend(mmindex->packets, mmpacket);
		}
		else
		{	// first time for channel, create channel entry and add packet info
//fprintf(stderr, "chn: %s\n", channelName);
			ttlChans++;
			mmindex = calloc(1, sizeof(MMINDEX));
			mmindex->channelName = g_strdup(channelName);
			mmindex->network = g_strdup(msr->network);
			mmindex->station = g_strdup(msr->station);
			mmindex->location = g_strdup(msr->location);
			mmindex->channel = g_strdup(msr->channel);
			mmindex->startTime = msr->starttime;
			mmindex->sampleCnt = msr->samplecnt;
			mmindex->sampRate = msr->samprate;
			mmindex->encoding = msr->encoding;
			MM_index = g_slist_prepend(MM_index, mmindex);
			                                                                                      
			mmpacket = calloc(1, sizeof(MMPACKET));
			mmpacket->index = packet_num;
			mmpacket->startTime = msr->starttime;
			mmpacket->sampleCnt = msr->samplecnt;
			mmindex->packets = g_slist_prepend(mmindex->packets, mmpacket); 
		}
	}
	CLOSEMSR(&msfp, &msr);

	for(iterIndex = MM_index;
		iterIndex;
		iterIndex = g_slist_next(iterIndex))
	{
		mmindex = iterIndex->data;
		// order the packets by time
		// this is necessary since traces are constructed based on time
		// while looping over the channel's packet list
		mmindex->packets = g_slist_sort(mmindex->packets, (GCompareFunc) timeOrder);
	}

#if 0	// list the results of our indexing... for debugging only
{
	GSList	*iterPacket;
	for(iterIndex = MM_index;
		iterIndex;
		iterIndex = g_slist_next(iterIndex))
	{
		mmindex = iterIndex->data;
		fprintf(stderr, "channel: %s\n", mmindex->channel);
		for(iterPacket = mmindex->packets;
			iterPacket;
			iterPacket = g_slist_next(iterPacket))
		{
			mmpacket = iterPacket->data;
			fprintf(stderr, "\tpkt: %d\tstart: %lld\n", mmpacket->index, (gint64) mmpacket->startTime);
		}
	}
}
#endif

	mmfile->channels = MM_index;
	if (ttlChans > 1)
	{	// don't save single-plex files
		g_static_mutex_lock(&mplexMutex);
		if (!mplex_files)
			mplex_files = g_hash_table_new_full(g_str_hash, g_str_equal, 
							(GDestroyNotify)free, (GDestroyNotify)free_MMFILE);
		g_static_mutex_unlock(&mplexMutex);
		g_hash_table_insert(mplex_files, g_strdup(fileName), mmfile);
	}

	return mmfile;
}

static GSList *makeTraces(MMFILE *mmfile)
{
	GSList 		*MM_index = mmfile->channels;
	MMINDEX		*mmindex;
	MMPACKET	*mmpacket;
	MM_TRCINFO	*trcInfo;
	GSList		*iterIndex, *iterPacket;
	GSList		*traces1=NULL, *trc;
	char		*channelName;
	gboolean	makeTrace;
	hptime_t	traceStart=0;

	// loop over indexed channels, identifying the individual traces to be made
	for(iterIndex = MM_index;
		iterIndex;
		iterIndex = g_slist_next(iterIndex))
	{	// loop over all indexed channels, extracting the traces accordingly
		mmindex = iterIndex->data;
		if (!(includeChan(mmindex->channel)))			// not a channel we're interested in
			continue;

		// iterate over the packets of the channel
		channelName = mmindex->channelName;
		for(iterPacket = mmindex->packets, makeTrace = FALSE;
			iterPacket;
			iterPacket = g_slist_next(iterPacket), makeTrace = FALSE)
		{
			mmpacket = iterPacket->data;

			trc = g_slist_find_custom(traces1, channelName, (GCompareFunc) getChannel);
			if (trc)
			{	// trace already made
				trcInfo = trc->data;
				if (mplex_options &&
					mplex_options->length)
				{	// check if our trace window length has been breached
					if ((mmpacket->startTime - traceStart)/ONEMIL >=  mplex_options->length)
					{	// new trace required
						makeTrace = TRUE;
					}
				}
			}	
			else
			{	// no trace yet, make it
				makeTrace = TRUE;
			}
	
			if (makeTrace)
			{	// make new trace file entry
				trcInfo = calloc(1, sizeof(MM_TRCINFO));
				trcInfo->fileName = g_strdup(mmfile->fileName);
				trcInfo->channelName = g_strdup(channelName);
				trcInfo->network = g_strdup(mmindex->network);
				trcInfo->station = g_strdup(mmindex->station);
				trcInfo->location = g_strdup(mmindex->location);
				trcInfo->channel = g_strdup(mmindex->channel);
				trcInfo->blkSize = mmfile->blkSize;
				trcInfo->startTime = mmpacket->startTime;
				trcInfo->sampRate = mmindex->sampRate;
				trcInfo->sampleCnt = mmpacket->sampleCnt;
				trcInfo->encoding = mmindex->encoding;
				trcInfo->packets = g_slist_prepend(trcInfo->packets, mmpacket);
				traces1 = g_slist_prepend(traces1, trcInfo);
				traceStart = mmpacket->startTime;
			}
			else
			{	// packet belongs to trace
				trcInfo->sampleCnt += mmpacket->sampleCnt;
				trcInfo->packets = g_slist_prepend(trcInfo->packets, mmpacket);
			}
		}
	}
	
	// sort the packets of our new trace list
	// but only if we have multiple block sizes (subsequently reading by LOOP)
	// i.e., reading by SEEK does not require this list to be ordered
	if (mmfile->blkSize)
	{
		for(trc = traces1;
			trc;
			trc = g_slist_next(trc))
		{
			trcInfo = trc->data;
			if (trcInfo &&
				trcInfo->packets)
				trcInfo->packets = g_slist_reverse(trcInfo->packets);
		}
	}

	return(traces1);
}

static double epochTime(hptime_t starttime)
{
	BTime		btime;
	struct ptime    date;
	
	ms_hptime2btime(starttime, &btime);
	date.yr = btime.year;
	date.day = btime.day;
	date.hr = btime.hour;
	date.mn = btime.min;
	date.sec = btime.sec;
	return (double) (passcal_htoe(&date, FALSE));
}
				
LOGDATA *makeLogs(char *fileName)
{
	GSList 		*MM_index;
	int			blkSize;
	MMINDEX		*mmindex;
	MMPACKET	*mmpacket;
	MMFILE 		*mmfile;
	GSList		*iterIndex, *iterPacket;
	LOGDATA		*logData=NULL;
	LOGTEXT		*logText;
	MSRecord	**msr, *lastmsr;
	MSFileParam **msfp;
	int			index;
	off_t		offset;
	gboolean	first;

	if (!mplex_files)
		return NULL;
	
	if (!(mmfile = g_hash_table_lookup(mplex_files, fileName)))
	{
		return NULL;
	}

	MM_index = mmfile->channels;
	blkSize = mmfile->blkSize;

	msr = calloc(1, sizeof(MSRecord *));
	msfp = calloc(1, sizeof(MSFileParam *));

	// loop over all indexed channels, extracting the log packets accordingly
	for(iterIndex = MM_index;
		iterIndex;
		iterIndex = g_slist_next(iterIndex))
	{                                                                  
		mmindex = iterIndex->data;
		if (strcmp(mmindex->channel, "LOG"))
			continue;

		mmindex->packets = g_slist_sort(mmindex->packets, (GCompareFunc) timeOrder);
		logData = calloc(1, sizeof(LOGDATA));
		logData->fileName = g_strdup(mmfile->fileName);
		
		// iterate over the packets of the LOG channel, extract the data and insert
		for(iterPacket = mmindex->packets, first=TRUE, lastmsr = NULL;
			iterPacket;
			iterPacket = g_slist_next(iterPacket))
		{
			mmpacket = iterPacket->data;
			index = mmpacket->index;										// get the packet number we need to read
			offset = (off_t) blkSize * -1 * (index);						// set read position to the packet number
			READMSRDATA2(msfp, msr, logData->fileName, offset);	// extract the data packet
			
			if (first)
			{
				first = FALSE;
				logData->startT = epochTime((*msr)->starttime);
				logData->station = g_strdup_printf("%s.%s.%s.%s",
									(*msr)->network, (*msr)->station, (*msr)->location,
									(*msr)->channel);
			}
			logText = calloc(1, sizeof(LOGTEXT));
			logText->startT = epochTime((*msr)->starttime);
			logText->text = g_strndup((*msr)->datasamples, (*msr)->numsamples);
			logData->text = g_slist_prepend(logData->text, logText);
			lastmsr = *msr;
		}
		if (lastmsr)
		{
			logData->endT = epochTime(lastmsr->starttime);
		}
		logData->text = g_slist_reverse(logData->text);
		CLOSEMSR(msfp, msr);
		break;
	}
	free(msr);
	free(msfp);
	
	return logData;
}

static GSList *get_mseed_hedrM(char *fileName)
{
	int			fileType;
	MMFILE		*mmfile;
	MMINDEX		*mmindex;
	MM_TRCINFO	*trcInfo;
	GSList		*traces1=NULL, *traces2=NULL, *trc;
	char		*channelName;
	GSList		*MM_index;                                             

	// index the entire contents of the file
	mmfile = index_mplex(fileName);
	if (!mmfile)                                                        
		return NULL;
	
	MM_index = mmfile->channels;

	switch(g_slist_length(MM_index))
	{
		case 1:
			fileType = MSEED_DATA;
		break;
		default:
			fileType = MSEED_MPLEX_DATA;
		break;
	}
	
	switch(fileType)
	{
		case MSEED_DATA:	// single-plex file
			mmindex = MM_index->data;
			channelName = mmindex->channelName;
			trcInfo = calloc(1, sizeof(MM_TRCINFO));
			trcInfo->fileName = g_strdup(fileName);
			trcInfo->channelName = g_strdup(channelName);
			trcInfo->network = g_strdup(mmindex->network);
			trcInfo->station = g_strdup(mmindex->station);
			trcInfo->location = g_strdup(mmindex->location);
			trcInfo->channel = g_strdup(mmindex->channel);
			trcInfo->startTime = mmindex->startTime;
			trcInfo->sampRate = mmindex->sampRate;
			trcInfo->sampleCnt = mmindex->sampleCnt;
			trcInfo->encoding = mmindex->encoding;
//			trcInfo->packets = g_slist_prepend(trcInfo->packets, mmpacket);
			traces1 = g_slist_prepend(traces1, trcInfo);
		break;
		
		case MSEED_MPLEX_DATA:
			traces1 = makeTraces(mmfile);
		break;
	}

	// make the PQL trace(s)
	for(trc = traces1; trc; trc = g_slist_next(trc))
	{
		trcInfo = trc->data;                 
		traces2 = g_slist_prepend(traces2, make_mseed_trace(trcInfo, fileType));
		free(trcInfo->fileName);
		free(trcInfo->channelName);
		free(trcInfo->network);
		free(trcInfo->station);
		free(trcInfo->location);
		free(trcInfo->channel);
		free(trcInfo);
		trc->data = NULL;
	}
	g_slist_free(traces1);
	
	return(traces2);
}

int get_mseed_hedr(traceInfo *trace)
{
	int		retVal;
	char 		*chr;
	MSRecord	**msr;
	MSFileParam **msfp;
	MM_TRCINFO		trcInfo;

	msr = calloc(1, sizeof(MSRecord *));
	msfp = calloc(1, sizeof(MSFileParam *));                             
	
	// get the first header, but no data                                          
	retVal = READMSRNDATA(msfp, msr, trace->data.fileName);
	if (retVal != MS_NOERROR)	// error reading file
	{
		free(msr);
		free(msfp);
		return (FILE_READ_ERROR);
	}
	// save the first read record for later use
//	trcInfo->channelName = g_strdup(channelName);
	trcInfo.network = g_strdup((*msr)->network);
	trcInfo.station = g_strdup((*msr)->station);
	trcInfo.location = g_strdup((*msr)->location);
	trcInfo.channel = g_strdup((*msr)->channel);
	trcInfo.sampRate = (*msr)->samprate;
	trcInfo.sampleCnt = (*msr)->samplecnt;
	trcInfo.encoding = (*msr)->encoding;
	trcInfo.startTime = (*msr)->starttime;
	
	if (!trace->data.dataInfo.head.numSamples)
	{	// when traces passed to PQL (e.g. from PSD::Detail), numSamples may already be filled
		trace->data.dataInfo.head.numSamples = (*msr)->samplecnt;
		while((retVal = READMSRNDATA(msfp, msr, trace->data.fileName))==MS_NOERROR)
		{	// read each block of data individually to get the total samples
			trace->data.dataInfo.head.numSamples += (*msr)->samplecnt;
		}
	}
	else
	{
		retVal = MS_ENDOFFILE;		// set for next test
	}

#if 0
updated to take pre-set value into account
	trace->data.dataInfo.head.numSamples = (*msr)->samplecnt;
	while((retVal = READMSRNDATA(msfp, msr, trace->data.fileName))==MS_NOERROR)
	{	// read each block of data individually to get the total samples
		trace->data.dataInfo.head.numSamples += (*msr)->samplecnt;
	}
#endif

	if (retVal != MS_ENDOFFILE)	// error reading file
	{
		CLOSEMSR(msfp, msr);
		free(msr);
		free(msfp);
		return (FILE_READ_ERROR);
	}

	switch(setHedrVals(&trcInfo, trace))
	{
		case FILE_READ_ERROR:
			CLOSEMSR(msfp, msr);
			free(msr);
			free(msfp);
			free(trcInfo.network);                             
			free(trcInfo.station);
			free(trcInfo.location);                                                       
			free(trcInfo.channel);
			return(FILE_READ_ERROR);
		break;
		default:
		break;
  	}
		
	CLOSEMSR(msfp, msr);
	free(msr);
	free(msfp);

	trace->data.head.gainConst = 1;
	trace->data.head.scale_fac = 1;
	strncpy(trace->data.head.network, trcInfo.network, 2);
	strncpy(trace->data.head.station, trcInfo.station, 5);
	strncpy(trace->data.head.location, trcInfo.location, 2);
	strncpy(trace->data.head.channel, trcInfo.channel, 3);
	g_strstrip(trace->data.head.network);
	g_strstrip(trace->data.head.station);
	g_strstrip(trace->data.head.location);
	g_strstrip(trace->data.head.channel);

	trace->data.head.descr2 = g_strdup_printf("%s.%s.%s.%s",
		trace->data.head.network,
		trace->data.head.station,
		trace->data.head.location,
		trace->data.head.channel);

	trace->data.head.longFN = trace->data.fileName;
	trace->data.head.shortFN = 
#ifndef WIN32
				g_strdup((chr = strrchr(trace->data.fileName,'/')) ? chr+1 : trace->data.fileName);
#else
				g_strdup((chr = strrchr(trace->data.fileName,'\\')) ? chr+1 : trace->data.fileName);
#endif

	free(trcInfo.network);
	free(trcInfo.station);
	free(trcInfo.location);
	free(trcInfo.channel);
	return PQLXSUCCESS;
}

static int pluckLOOP(MSTraceGroup **mstg, traceInfo *trace)
{
	MSRecord	**msr;
	MSFileParam **msfp;
	MSTrace		*msTrc;
	GSList		*iterPacket;
	int			packet_num, index;
	int			retVal = MS_NOERROR;

	msr = calloc(1, sizeof(MSRecord *));
	msfp = calloc(1, sizeof(MSFileParam *));
	
	for(iterPacket = trace->mplex.packets, packet_num = 0;
		iterPacket &&
		retVal == MS_NOERROR;
		iterPacket = g_slist_next(iterPacket), packet_num++)
	{
		index = ((MMPACKET *)(iterPacket->data))->index;			// get the packet number we need to read
		for(;
			packet_num < index &&
			retVal == MS_NOERROR;
			packet_num++)
			retVal = READMSRNDATA(msfp, msr, trace->data.fileName);	// forward to the next packet to extract

		if (retVal != MS_NOERROR)
			continue;
		
		retVal = READMSRDATA(msfp, msr, trace->data.fileName);		// extract the data packet
		msTrc = mst_addmsrtogroup (*mstg, *msr, 0, -2.0, -2.0);		// add to our trace group
		if (!msTrc)
			retVal = FAILURE;
	}
	CLOSEMSR(msfp, msr);
	free(msr);
	free(msfp);
	return(retVal);
}

static int pluckSEEK(MSTraceGroup **mstg, traceInfo *trace)
{
	MSRecord	**msr;
	MSFileParam **msfp;
	MSTrace		*msTrc;
	GSList		*iterPacket;
	int			index, retVal;
	off_t		offset;
	int			blkSize = trace->mplex.blkSize;

	msr = calloc(1, sizeof(MSRecord *));
	msfp = calloc(1, sizeof(MSFileParam *));

	for(iterPacket = trace->mplex.packets, retVal = MS_NOERROR;
		iterPacket && retVal == MS_NOERROR;
		iterPacket = g_slist_next(iterPacket))
	{
		index = ((MMPACKET *)(iterPacket->data))->index;				// get the packet number we need to read
		offset = (off_t) blkSize * -1 * (index);						// set read position to the packet number
		retVal = READMSRDATA2(msfp, msr, trace->data.fileName, offset);	// extract the data packet
		msTrc = mst_addmsrtogroup (*mstg, *msr, 0, -1.0, -1.0);			// add to our trace group
		if (!msTrc)
			retVal = FAILURE;
	}
	CLOSEMSR(msfp, msr);
	free(msr);
	free(msfp);
	return retVal;
}

static int pluckPackets(MSTraceGroup **mstg, traceInfo *trace)
{
	int	retVal;
	
	switch(trace->mplex.blkSize)
	{
		case 0:									// blkSize = 0 means the file has multiple block sizes
			retVal = pluckLOOP(mstg, trace);	// LOOP over the packets to extract, slower but guaranteed
		break;
		default:								// blkSize <> 0 means the file has a single block size
			retVal = pluckSEEK(mstg, trace);	// SEEK over the packets to extract, way quicker
		break;
	}
	return(retVal);
}

enum {
	OLAP_FULL,
	OLAP_PARTIAL
};

static int	get_mseed_trace(traceInfo *trace, int *ttlMemAlloc)
{
	gboolean	chkGaps=TRUE, first;
	int			numGaps=0, numOlaps=0, totSamp=0;
	int			copyStart;
	double		oneSample=0;
	int			gapFill, retVal, copySamples;
	int			dataSize=0;
	int			olapStartPt, olapNpts, olapStart, olapEnd;
	char		*dataPtr=trace->data.dataInfo.data;
	OVERLAP		*overlap;
	hptime_t	prev_endT=0, gapTFill, olapStartTm;
	MSTraceGroup	**mstg=NULL;
	MSTrace 	*trc;

	switch(trace->data.fileType)
	{
		case MSEED_DATA:	// single-plex - read using standard libmseed function
			mstg = calloc(1, sizeof(MSTraceGroup *));
			retVal = READMSTGDATA(mstg, trace->data.fileName);
			if (retVal != MS_NOERROR)
			{
				mst_freegroup(mstg);
				free(mstg);
				return(FAILURE);
			}
		break;
		case MSEED_MPLEX_DATA:	// multi-plex - extract the individual packets from the file
			mstg = calloc(1, sizeof(MSTraceGroup *));
			*mstg = mst_initgroup(NULL);
			retVal = pluckPackets(mstg, trace);
			if (retVal != MS_NOERROR)
			{
				mst_freegroup(mstg);
				return(FAILURE);
			}
		break;
	}

	mst_groupsort((*mstg), FALSE);
	mst_groupheal((*mstg), -1.0, -1.0);
//	mst_printtracelist(*mstg, 0, 1, 1);

	for(trc=(*mstg)->traces, first=TRUE;
		trc;
		trc=trc->next)
	{
		if (!trc->samplecnt)	// traces w/out data we ignore
			continue;
			
		if ((strcmp(trc->network, trace->data.head.network)) ||
			(strcmp(trc->station, trace->data.head.station)) ||
			(strcmp(trc->location, trace->data.head.location)) ||
			(strcmp(trc->channel, trace->data.head.channel)))
		{
			if (first)
				continue;		// haven't found our channel yet
			else
				break;			// we've already processed our channel, leave
		}

		if (first)
		{	// first trace found is special case
			if (!trace->data.head.sampInt)	// possibly not yet correctly set
			{
				trace->data.head.sampInt = SAMPINT(trc->samprate);
				switch(trc->sampletype)
				{
					case 'a':
						trace->data.head.data_form = MY_CHAR;
						trace->data.head.dataSize = sizeof(char);
					break;
					case 'i':
						trace->data.head.data_form = BIT32;
						trace->data.head.dataSize = sizeof(int);
					break;
					case 'f':
						trace->data.head.data_form = MY_FLOAT;
						trace->data.head.dataSize = sizeof(float);
					break;
					case 'd':
						trace->data.head.data_form = MY_DOUBLE;
						trace->data.head.dataSize = sizeof(double);
					break;
				}
			}
			oneSample = trace->data.head.sampInt;
			dataSize = trace->data.head.dataSize;
			//prev_startT = trc->starttime;			// startTime of first trace
			setStartTime(trc->starttime, trace);	// reset start time
			prev_endT = trc->endtime;				// endTime of first trace
			totSamp = trc->samplecnt;
			if((totSamp)*dataSize > *ttlMemAlloc) 
			{	// more pts than space to save them, go get more memory
				if ((dataPtr = realloc(dataPtr, (totSamp)*dataSize))==NULL)
				{
					mst_freegroup(mstg);
					free(mstg);
					return(FAILURE);
				}
				*ttlMemAlloc = totSamp*dataSize;
			}
			memcpy(dataPtr, trc->datasamples, trc->samplecnt*dataSize);
			first=FALSE;
			continue;
		}

		copySamples = trc->samplecnt;
		copyStart = 0;

		// check for OVERLAP
		if (trc->starttime < (prev_endT + oneSample))
		{
			int	olapSpan;
			
			olapStartTm = -1 * (trc->starttime - (prev_endT + oneSample));		// number of overlap seconds
			olapStartPt = olapStartTm / oneSample;								// number of overlap points
			olapSpan = (trc->endtime <= (prev_endT + oneSample)) ? OLAP_FULL : OLAP_PARTIAL;	// overlap is full or partial?
			olapNpts = 	(olapSpan == OLAP_FULL)
						?	trc->samplecnt										// full
						:	olapStartPt;										// partial
						
			if (chkGaps)
			{	// save info only once, and only when overlap does not cause a reset (previous if)
#if 0
	code for when gaps/overlaps are managed as lists
		glaps		*glapPtr;
					glapPtr = calloc(1, sizeof(glaps));
					glapPtr->bounds[GLAPBEG] = totSamp;
					glapPtr->bounds[GLAPEND] = totSamp + gapFill;
					trace->trace.overlaps = g_slist_append(trace->trace.overlaps, glapPtr);
#endif
				if (!trace->trace.overlaps)
				{
					trace->trace.overlaps = calloc(1, sizeof(int)*2);
				}
				else
				{
					trace->trace.overlaps = realloc(trace->trace.overlaps, 
															(numOlaps+1)*(sizeof(int)*2));
				}
				// set the starting and ending points of the overlap
				olapStart = trace->trace.overlaps[OLAPSTART] = totSamp - olapStartPt;
				olapEnd = trace->trace.overlaps[OLAPTERM] = olapStart + olapNpts ;	

				// new overlapping data management - saving both overlapping data sements for display
				overlap = calloc(1, sizeof(OVERLAP));
				overlap->bounds[START] = olapStart;
				overlap->bounds[END] = olapEnd;
				overlap->len = olapNpts;
				overlap->data = malloc(overlap->len * dataSize);
				// copy the overlapping data to the overlap data structure
				memcpy(overlap->data, trc->datasamples, overlap->len*dataSize);
				
				// set up subsequent data copy (below) of remaining data
				// N.B. full overlap makes no copy to main data array
				copySamples = (olapSpan == OLAP_PARTIAL) ? trc->samplecnt - olapNpts : 0;
				copyStart = (olapSpan == OLAP_PARTIAL) ? olapNpts : 0;

				trace->trace.olapData = g_slist_append(trace->trace.olapData, overlap);
			}
			numOlaps++;
		}

		// check for GAP, but not on first trace
		if (trc->starttime > prev_endT + oneSample)
		{
			gapTFill = (trc->starttime - prev_endT) - oneSample;
			gapFill = gapTFill * (1./trace->data.head.sampInt);
			if (chkGaps)
			{	// if we've already read the data once, we have this info
#if 0
code for when gaps/overlaps are managed as lists
				glapPtr = calloc(1, sizeof(glaps));
				glapPtr->bounds[GLAPBEG] = totSamp;
				glapPtr->bounds[GLAPEND] = totSamp + gapFill;
				trace->trace.gaps = g_slist_append(trace->trace.gaps, glapPtr);
#endif
				if (!trace->trace.gaps)
				{
					trace->trace.gaps = calloc(1, sizeof(int)*2);		// first gap
				}
				else
				{
					trace->trace.gaps = realloc(trace->trace.gaps, 
												(numGaps+1)*sizeof(int)*2);
				}
				trace->trace.gaps[numGaps*2] = totSamp;				// first data point of gap
				trace->trace.gaps[numGaps*2+1] = totSamp + gapFill;	// first next valid data point
			}
			if((copySamples + (totSamp+gapFill))*dataSize > *ttlMemAlloc) 
			{	// more pts than space to save them, go get more memory
				if ((dataPtr = realloc(dataPtr, (copySamples + (totSamp+gapFill)) * dataSize))==NULL)
				{
					mst_freegroup(mstg);
					free(mstg);
					return(FAILURE);
				}
				*ttlMemAlloc = (copySamples + (totSamp+gapFill))*dataSize;
			}
			memset(dataPtr+(totSamp*dataSize), 0, gapFill*dataSize);	// zero-out the data
			totSamp += gapFill;			// add the missing data points to grand total
			numGaps++;
		}

		if((copySamples + totSamp)*dataSize > *ttlMemAlloc) 
		{	// more pts than space to save them, go get more memory
			if ((dataPtr = realloc(dataPtr, (copySamples + totSamp) * dataSize))==NULL)
			{
				mst_freegroup(mstg);
				free(mstg);
				return(FAILURE);
			}
			*ttlMemAlloc = (copySamples + totSamp)*dataSize;
		}

		// collect data -> copy from mseed data buffer to data analysis buffer
		if (copySamples)
		{
			memcpy((void *) (dataPtr+(totSamp*dataSize)), (void *) (trc->datasamples+(copyStart*dataSize)), 
						copySamples*dataSize);
			totSamp += copySamples;
		}

		// only set these when processing a real trace, i.e., don't put inside for()
		//prev_startT = trc->starttime;
		if (trc->endtime > prev_endT)	// keep the greatest end time, not necessarily this trc
			prev_endT = trc->endtime;
	}
	mst_freegroup(mstg);
	free(mstg);

	if (chkGaps)
	{
		trace->trace.numGaps = numGaps;
		trace->trace.numOlaps = numOlaps;
	}
	trace->data.dataInfo.data = dataPtr;
	trace->data.head.length = (totSamp-1)*(oneSample/ONEMIL);

	return(totSamp);
}

//	get_mseed_data(): entry point for acquiring data from a mini-seed format file.
//	algorithm:
//		for efficiency reasons, we first read the file packet to packet (msr_readmsr())
//		if no gaps or overlaps are detected, we're done and return the data; this is very fast.
//		if gaps and overlaps are detected (or any other problems), we reread the trace 
//		with msr_readtraces() followed by mst_heal() and mst_sort() since the previously 
//		read packets may simply be out-of-order instead of representing real gaps and/or 
//		overlaps.

int get_mseed_data(traceInfo *trace)
{
	char		*dataPtr;
	gboolean	reRead=FALSE;
	int			ttlPtsAlloc, ttlMemAlloc, retVal, dataSize;
	int			totSamp=0;
	double		oneSample;
	hptime_t	curStart, nextStart=0;
	MSRecord	**msr;
	MSFileParam **msfp;
	MM_TRCINFO		trcInfo;

	if (!trace->data.head.year)
	{	// header not yet read, go get it first
		retVal = get_mseed_hedr(trace);		// don't need to check for endianness
		if (retVal != PQLXSUCCESS)
			return(retVal);
	}

	if (trace->data.dataInfo.head.numSamples > (TOOMANYPOINTS))
	{	// numPts is > (TOOMANYPOINTS), i.e., of suspect quality and potentially damaging, so we reject it
		return(FILE_TOO_LARGE);
	}
	if (!trace->data.dataInfo.head.numSamples)
	{
		return(NOT_ENOUGH_DATA);
	}

	dataSize = trace->data.head.dataSize;
	ttlMemAlloc = trace->data.dataInfo.head.numSamples * dataSize;
	if (ttlMemAlloc &&
		((dataPtr = trace->data.dataInfo.data = malloc(ttlMemAlloc))==NULL))
	{	// initialize memory allocation
		fprintf(stderr, "Unable to allocate %d bytes of data.\n", ttlMemAlloc);
		return(NO_MEMORY);
	}

	switch(trace->data.fileType)
	{
		case MSEED_MPLEX_DATA:
			totSamp = get_mseed_trace(trace, &ttlMemAlloc);
			if (totSamp == FAILURE)
			{
				free(trace->data.dataInfo.data);
				trace->data.dataInfo.data = NULL;
				return(FAILURE);
			}
			trace->data.dataInfo.head.numSamples = totSamp;
			trace->data.dataInfo.head.data_form = trace->data.head.data_form;
			trace->data.dataInfo.head.sampInt = trace->data.head.sampInt;
			trace->data.dataInfo.head.secsPsamp = (trace->data.head.sampInt/ONEMIL);
		break;
		
		case MSEED_DATA:
			oneSample = trace->data.head.sampInt;
			trace->data.dataInfo.head.data_form = trace->data.head.data_form;
			trace->data.dataInfo.head.sampInt = trace->data.head.sampInt;
			trace->data.dataInfo.head.secsPsamp = (trace->data.head.sampInt/ONEMIL);

			msr = calloc(1, sizeof(MSRecord *));
			msfp = calloc(1, sizeof(MSFileParam *));
			
			while((retVal = READMSRDATA(msfp, msr, trace->data.fileName))==MS_NOERROR)
			{	// read each block of data individually for gap and overlap detection
				if ((strcmp((*msr)->location, trace->data.head.location)) ||
					((*msr)->encoding == 0) ||	// non-data packets ignored
					(!(*msr)->samplecnt)) 		// packets with no data ignored
					continue;

				if (!trace->data.head.sampInt)
				{	// possibly not yet correctly set
					setMMTRCINFO(&trcInfo, *msr);
					switch(setHedrVals(&trcInfo, trace))
					{
						case FILE_READ_ERROR:
							CLOSEMSR(msfp, msr);
							free(msr);
							free(msfp);
							free(trace->data.dataInfo.data);
							trace->data.dataInfo.data = NULL;
							return(FAILURE);
						break;
						default:
						break;
					}
					oneSample=trace->data.head.sampInt;
					dataSize=trace->data.head.dataSize;
					ttlMemAlloc = trace->data.dataInfo.head.numSamples * dataSize;
					if (ttlMemAlloc &&
						((dataPtr = trace->data.dataInfo.data = malloc(ttlMemAlloc))==NULL))
					{	// initialize memory allocation
						fprintf(stderr, "Unable to allocate %d bytes of data.\n", ttlMemAlloc);
						return(NO_MEMORY);
					}
				}
				curStart = (*msr)->starttime;
				if (nextStart)
				{
					if (curStart > nextStart)
					{	// we have a gap, read file differently
						reRead = TRUE;
						break;
					}
					else
					{
						if (curStart < nextStart)
						{	// we have an overlap, read file differently
							reRead = TRUE;
							break;
						}
					}
				}
				// set the next block's expected start time
				nextStart = msr_endtime((*msr)) + oneSample;
				
				ttlPtsAlloc = (*msr)->samplecnt + totSamp;	// ttl points needed for allocation
				if(ttlPtsAlloc*dataSize > ttlMemAlloc) 
				{	// more pts than space to save them, go get more memory
					ttlMemAlloc = ttlPtsAlloc * dataSize;
					if ((dataPtr = realloc(dataPtr, ttlMemAlloc))==NULL)
					{
						CLOSEMSR(msfp, msr);
						free(msr);
						free(msfp);
						free(trace->data.dataInfo.data);
						trace->data.dataInfo.data = NULL;
						return(NO_MEMORY);
					}
					trace->data.dataInfo.data = dataPtr;
				}
		
				// collect data -> copy from mseed data buffer to data analysis buffer
				memcpy(dataPtr+(totSamp*dataSize), (*msr)->datasamples, 
						(*msr)->samplecnt*dataSize);
				totSamp += (*msr)->samplecnt;
			}
			CLOSEMSR(msfp, msr);
			free(msr);
			free(msfp);
		
			trace->data.dataInfo.data = dataPtr;
			if (reRead || retVal != MS_ENDOFFILE)
			{	// disk image not in order or there was a problem, read again, but smarter
				totSamp = get_mseed_trace(trace, &ttlMemAlloc);
				if (totSamp == FAILURE)
				{
					free(trace->data.dataInfo.data);
					trace->data.dataInfo.data = NULL;
					return(FAILURE);
				}
				trace->data.dataInfo.head.numSamples = totSamp;
			}
			else
			{	// take our values from this routine
				trace->data.dataInfo.head.numSamples = totSamp;
				trace->data.head.length = (totSamp-1)*(oneSample/ONEMIL);
			}
		break;
	}	// end switch(dataType)

	if (!totSamp)
	{
		free(trace->data.dataInfo.data);
		trace->data.dataInfo.data = NULL;
		return(NOT_ENOUGH_DATA);			// in this case, none!
	}
	
	return PQLXSUCCESS;
}

static gboolean check_MSFile(char *fileName)
{
	MSRecord	**msr;
	MSFileParam **msfp;
	gboolean	ret = TRUE;
	int			retVal;

	msr = calloc(1, sizeof(MSRecord *));
	msfp = calloc(1, sizeof(MSFileParam *));

	// read first header
	retVal = READMSRNDATA(msfp, msr, fileName);
	if (retVal != MS_NOERROR ||
		(((*msr)->reclen % 256) != 0))
	{
		ret = FALSE;
	}
	CLOSEMSR(msfp, msr);
	free(msr);
	free(msfp);
	return ret;
}

int	check_mseed(FILE *fp, traceInfo *trace)
{
	int		ret = FAILURE;
	
	if (check_MSFile(trace->data.fileName))
	{
		trace->data.fileType = MSEED_DATA;
		ret = get_mseed_hedr(trace);
	}
	
	if (ret != PQLXSUCCESS)
		return (FAILURE);
	else
		return (PQLXSUCCESS);
}

gboolean IDmseed(FILE *fp, char *fileName, MPLX_MSEED *chnOpts, GSList	**traces)
{
	mplex_options = chnOpts;
	
	if (check_MSFile(fileName))
	{
		*traces = get_mseed_hedrM(fileName);
		return TRUE;
	}
	else
		return FALSE;
}
