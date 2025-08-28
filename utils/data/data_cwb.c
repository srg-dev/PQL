#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#ifndef MAC
#include <sys/types.h>
#endif
#include <sys/stat.h>           // stat(), etc
#include <unistd.h>             // getcwd(), etc
#include "pql_defines.h"
#include "pql_externs.h"
#include "libmseed.h"
#include "steim_DMC.h"
#ifdef SOLARIS2
#include "libgen.h"
#endif

gboolean beepMe(gpointer nil);

#define GAPSTART	(numGaps*2)
#define GAPTERM		(numGaps*2+1)
#define OLAPSTART	(numOlaps*2)
#define OLAPTERM	(numOlaps*2+1)

typedef struct _CHANINFO
{
	char *channel;
	char *fileName;
	MSRecord	*msr;
} CHANINFO;

#define	CLOSEMSR(msfp, msr)				ms_readmsr_r(msfp, msr, NULL, 0, NULL, NULL, 0, 0, 0)
#define READMSRDATA(msfp, msr, file)	ms_readmsr_r(msfp, msr, file, 0, NULL, NULL, 1, TRUE, 0)
#define READMSTGDATA(mstg, file)		ms_readtraces(mstg, file, -1, -1.0, -1.0, 0, 1, TRUE, 0)
#define READMSRNDATA(msfp, msr, file)	ms_readmsr_r(msfp, msr, file, 0, NULL, NULL, 1, FALSE, 0)
#define READMSTGNDATA(mstg, file)		ms_readtraces(mstg, file, -1, -1.0, -1.0, 0, 1, FALSE, 0)

#define ISVALIDENCODING(a)	((a==1 || \
							 a==3 || \
							 a==10 || \
							 a==11 || \
							 a==30 || \
							 a==32 || \
							 a==4 || \
							 a==5 || \
							 a==12 || \
							 a==13 || \
							 a==14) ? TRUE : FALSE)

static void setStartTime (hptime_t starttime, traceInfo *trc)
{
	BTime		btime;
	struct ptime    date;

  	ms_hptime2btime(starttime, &btime);
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

static int get_mseed_hedr_cwb(traceInfo *trace)
{
	int		retVal;
	char 		*chr;
	MSRecord	**msr;
	MSFileParam **msfp;	

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

	switch((*msr)->encoding)
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
						trace->data.fileName, (*msr)->encoding);
			CLOSEMSR(msfp, msr);
			free(msr);
			free(msfp);
			return(FILE_READ_ERROR);
		break;
  	}
  	setStartTime((*msr)->starttime, trace);
	trace->data.head.gainConst = 1;
	trace->data.head.scale_fac = 1;
	strncpy(trace->data.head.network, (*msr)->network, 2);
	strncpy(trace->data.head.station, (*msr)->station, 5);
	strncpy(trace->data.head.location, (*msr)->location, 2);
	strncpy(trace->data.head.channel, (*msr)->channel, 3);
	g_strstrip(trace->data.head.network);
	g_strstrip(trace->data.head.station);
	g_strstrip(trace->data.head.location);
	g_strstrip(trace->data.head.channel);
	trace->data.head.sampInt = 1000000./(*msr)->samprate;
	trace->data.dataInfo.head.numSamples = 0;

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

	CLOSEMSR(msfp, msr);
	free(msr);
	free(msfp);
	return PQLXSUCCESS;
}

static int	get_mseed_trace_cwb(traceInfo *trace, int *tot_alloc)
{
	gboolean		chkGaps=TRUE, first;		// LOlap = FALSE, 
	int			numGaps=0, numOlaps=0, totSamp=0;
	int			oneSample=trace->data.head.sampInt, copyStart;
	int			gapFill, retVal, copySamples;
	int			dataSize=trace->data.head.dataSize;
	int			olapStartPt, olapNpts, olapStart, olapEnd;
	char			*dataPtr=trace->data.dataInfo.data;
	OVERLAP		*overlap;
	hptime_t		prev_endT, gapTFill, olapStartTm;		// prev_startT, 
	MSTraceGroup	**mstg;
	MSTrace 	*trc;

	mstg = calloc(1, sizeof(MSTraceGroup *));
	retVal = READMSTGDATA(mstg, trace->data.fileName);
	if (retVal != MS_NOERROR)
	{
		mst_freegroup(mstg);
		free(mstg);
		return(FAILURE);
	}
		
	mst_groupsort((*mstg), FALSE);
	mst_groupheal((*mstg), -1.0, -1.0);
//	mst_printtracelist(*mstg, 0, 1, 1);

	if (trace->data.dataInfo.head.numSamples)
	{	// have we read the trace data already once?
		chkGaps = FALSE;
	}

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
//			prev_startT = trc->starttime;			// startTime of first trace
			setStartTime(trc->starttime, trace);		// reset start time
			prev_endT = trc->endtime;				// endTime of first trace
			totSamp = trc->samplecnt;
			if((totSamp) > *tot_alloc) 
			{	// more pts than space to save them, go get more memory
				if ((dataPtr = realloc(dataPtr, (totSamp)*dataSize))==NULL)
				{
					mst_freegroup(mstg);
					free(mstg);
					return(FAILURE);
				}
				else
				{
					*tot_alloc = totSamp;
				}
			}
			memcpy(dataPtr, trc->datasamples, trc->samplecnt*dataSize);
			first=FALSE;
			continue;
		}

		copySamples = trc->samplecnt;
		copyStart = 0;

		// check for OVERLAP
		if (trc->starttime <= prev_endT)
		{
//			LOlap = TRUE;
			olapStartTm = (trc->starttime - prev_endT) - oneSample;
			olapStartPt = olapStartTm * (1./trace->data.head.sampInt);
			olapNpts = 	(trc->endtime <= prev_endT)		// overlap is full or partial?
						?	trc->samplecnt									// full
						:	olapStartTm * (1./trace->data.head.sampInt) * -1;	// partial
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
				// the starting point of the overlap
				trace->trace.overlaps[OLAPSTART] = totSamp + olapStartPt;
				// the ending point of the overlap
				trace->trace.overlaps[OLAPTERM] = trace->trace.overlaps[OLAPSTART] + olapNpts ;	
					
				// the starting point of the overlap
				olapStart = totSamp + olapStartPt;
				// the ending point of the overlap
				olapEnd = olapStart + olapNpts ;	

				// new overlapping data management - saving both overlapping data sements for display
				overlap = calloc(1, sizeof(OVERLAP));
				overlap->bounds[START] = olapStart;
				overlap->bounds[END] = olapEnd;
				overlap->len = olapNpts;
				overlap->data = malloc(overlap->len * dataSize);
				// copy overlapping portion to overlap structure
				memcpy(overlap->data, trc->datasamples, overlap->len*dataSize);
				if (trc->endtime > prev_endT)
				{	// set up subsequent data copy (below) of remaining data
					copySamples = trc->samplecnt - olapNpts;
					copyStart = olapNpts;
				}
				else
				{	// total overlap makes no copy to main data array
					copySamples = 0;
					copyStart = 0;
				}
				trace->trace.olapData = g_slist_append(trace->trace.olapData, overlap);
			}
			numOlaps++;
		}
//		else
//		{
//			LOlap = FALSE;
//		}

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
			if((copySamples + (totSamp+gapFill)) > *tot_alloc) 
			{	// more pts than space to save them, go get more memory
				if ((dataPtr = realloc(dataPtr, (copySamples + (totSamp+gapFill)) * dataSize))==NULL)
				{
					mst_freegroup(mstg);
					free(mstg);
					return(FAILURE);
				}
				*tot_alloc = copySamples + (totSamp+gapFill);
			}
			memset(dataPtr+(totSamp*dataSize), 0, gapFill*dataSize);	// zero-out the data
			totSamp += gapFill;			// add the missing data points to grand total
			numGaps++;
		}

		if((copySamples + totSamp) > *tot_alloc) 
		{	// more pts than space to save them, go get more memory
			if ((dataPtr = realloc(dataPtr, (copySamples + totSamp) * dataSize))==NULL)
			{
				mst_freegroup(mstg);
				return(FAILURE);
			}
			*tot_alloc = copySamples + totSamp;
		}

		// collect data -> copy from mseed data buffer to data analysis buffer
		if (copySamples)
		{
			memcpy((void *)(dataPtr+(totSamp*dataSize)), (void *) (trc->datasamples+(copyStart*dataSize)), 
						copySamples*dataSize);
			totSamp += copySamples;
		}

		// only set these when processing a real trace, i.e., don't put inside for()
//		prev_startT=trc->starttime;
		if (trc->endtime > prev_endT)	// keep the greatest end time, not this trc
			prev_endT=trc->endtime;
	}
	mst_freegroup(mstg);
	free(mstg);

	if (chkGaps)
	{
		trace->trace.numGaps = numGaps;
		trace->trace.numOlaps = numOlaps;
	}
	trace->data.dataInfo.data = dataPtr;
	trace->data.head.length = (totSamp)*(oneSample/1000000.) - (oneSample/1000000.);

	return(totSamp);
}

#include <sys/socket.h>
#include <sys/types.h>
#include <resolv.h>
#include <netinet/in.h>
#include <arpa/inet.h>

enum {
	CWB_SOCKET_OPEN,
	CWB_SOCKET_RELEASE,
	CWB_SOCKET_CLOSE
};
enum {
	CWBCONNNUM,
	CWBCONNSD
};

#define CWB_EOR_STRING	"<EOR>"
#define MAX_CWB_THREDS 	4
static GQueue	*cwbQ;
static int	numThreds, thredCntr;
typedef struct _CWB_THRED_ARGS {
	traceInfo *trc;
//	GThreadFunc *returnTo;
	int  sd;
	int	 thredNum;
	int	 returnCode;
} CWB_THRED_ARGS;
struct {
	char	*inetAddr;
	int		port;
} CWBSRVR;
static void read_CWB_data(CWB_THRED_ARGS *);

GStaticMutex myCWBMutex = G_STATIC_MUTEX_INIT;
static int getSocketD(int action, int sdOld)
{
	int		i=0, *sd;
	static int sdConn[2][MAX_CWB_THREDS];
	struct sockaddr_in cwb_addr;
	
	switch(action)
	{
		case CWB_SOCKET_OPEN:
			g_static_mutex_lock(&myCWBMutex);	
				while(sdConn[CWBCONNNUM][i++]&&(i<MAX_CWB_THREDS));
				sdConn[CWBCONNNUM][--i] = TRUE;					// mark as in-use
			g_static_mutex_unlock(&myCWBMutex);

//fprintf(stderr, "socket request (%d)... \n", i);
			sd = &sdConn[CWBCONNSD][i];
			if (!*sd)
			{	// get a connection outside the lock
				*sd = socket(PF_INET, SOCK_STREAM, 0);
				memset(&cwb_addr, 0, sizeof(cwb_addr));
				cwb_addr.sin_family = AF_INET;
				cwb_addr.sin_port = htons(CWBSRVR.port);
				inet_aton(CWBSRVR.inetAddr, &cwb_addr.sin_addr);
				if ((connect(*sd, (struct sockaddr *) &cwb_addr, sizeof(cwb_addr)))==-1)
				{
					g_idle_add(beepMe, NULL);
					fprintf(stderr, "Cannot connect to CWB Server (IP = %s)\n",
								inet_ntoa(cwb_addr.sin_addr));
					*sd = 0;
				}
//fprintf(stderr, "socket opened (%d): %d\n", i, *sd);
			}
//fprintf(stderr, "returned (%d): %d\n", i, *sd);
		break;

		case CWB_SOCKET_RELEASE:
//fprintf(stderr, "release request (%d)\n", sdOld);
			while(sdConn[CWBCONNSD][i++] != sdOld);
			i--;
			sd = &sdConn[CWBCONNSD][i];
//#if 0
// next two lines used only when closing the socket after each individual read
			shutdown(*sd, SHUT_RDWR);
			*sd = 0;
//fprintf(stderr, "socket closed: %d\n", i);
//#endif
			g_static_mutex_lock(&myCWBMutex);	
				sdConn[CWBCONNNUM][i] = FALSE;				// mark as available
			g_static_mutex_unlock(&myCWBMutex);
//fprintf(stderr, "socket released: %d\n", i);
		break;

		case CWB_SOCKET_CLOSE:
			for(i=0;i<MAX_CWB_THREDS;i++)
			{	// close all open sockets
				if (sdConn[CWBCONNSD][i])
				{
					shutdown(sdConn[CWBCONNSD][i], SHUT_RDWR);
					sdConn[CWBCONNSD][i] = 0;
//fprintf(stderr, "socket closed: %d\n", i);
				}
				sdConn[CWBCONNNUM][i] = FALSE;
			}
			sd = &sdConn[CWBCONNSD][0];
		break;		
	}

	return (*sd);
}

static gboolean readCWBReturn(CWB_THRED_ARGS *argsOld)
{
	traceInfo *trcNxt, *trc;
	CWB_THRED_ARGS *argsNew;

//fprintf(stderr, "threadNum: %d\n", ++thredCnt);
	numThreds--;
	getSocketD(CWB_SOCKET_RELEASE, argsOld->sd);	// only called when closing socket each time
	
	if (cwbQ &&
		cwbQ->length)
	{
		numThreds++;
		trcNxt = g_queue_pop_head(cwbQ);
		argsNew = calloc(1, sizeof(CWB_THRED_ARGS));
//		argsNew->sd = argsOld->sd;		// only when re-using an already open socket
		argsNew->trc = trcNxt;
		argsNew->thredNum = thredCntr++;
		makeThread(ASYNC, (GThreadFunc) read_CWB_data, argsNew, NULL);		// request next trace from CWB
	}
	else
	{
		if (!numThreds)
		{
			getSocketD(CWB_SOCKET_CLOSE, 0);
			thredCntr=0;
			free(CWBSRVR.inetAddr);
			CWBSRVR.inetAddr = NULL;
			trc = argsOld->trc;
			if (trc->excerpt.CWB.mainReturn)
				g_idle_add(trc->excerpt.CWB.mainReturn, NULL);		// return to the main thread
		}
	}
	
	free(argsOld);
	return FALSE;
}

static void set_CWB_hedr(traceInfo *trc)
{
	trc->data.fileName = g_strdup("CWB Extract");
	trc->data.head.longFN = trc->data.fileName;
	trc->data.head.shortFN = g_strdup(trc->data.fileName);
	trc->data.head.descr1 = g_strdup_printf("%i %03i %02i:%02i" ,
	    trc->data.head.year, trc->data.head.day, trc->data.head.hour, trc->data.head.minute);
	trc->data.head.descr2 = g_strdup_printf("%s.%s.%s.%s",
		trc->data.head.network,
		trc->data.head.station,
		trc->data.head.location,
		trc->data.head.channel);
}

static void read_CWB_data(CWB_THRED_ARGS *args)
{
	traceInfo *trc = args->trc;
	int 	i;
	char	*request;
	char	buf[4096];
	int		ret, readBytes, byteLoc = 0;

	if (!args->sd)
	{
		args->sd = getSocketD(CWB_SOCKET_OPEN, 0);
	}
	
	if (args->sd)
	{
		FILE *fp;
		char *tmpFile;
	    struct stat file_stat;
	    int		tot_alloc, dataSize, totSamp;

		if (trc->excerpt.epochS)
		{	// must set the start time
			PTIME	date;
			passcal_etoh(&date, trc->excerpt.epochS);
			trc->data.head.epochs = trc->excerpt.epochS;
			trc->data.head.length = trc->excerpt.length;
			trc->data.head.year = date.yr;
			trc->data.head.day = date.jday;
			trc->data.head.hour = date.hr;
			trc->data.head.minute = date.mn;
			trc->data.head.second = 0;
			trc->data.head.m_secs = 0;
		}
		request = g_strdup_printf("'-s' '%2s     %3s%2s' '-b' '%4d,%03d-%02d:%02d:%02d' '-d' '%d'\n",
									trc->data.head.network,
									trc->data.head.channel,
									trc->data.head.location,
									trc->data.head.year,
									trc->data.head.day,
									trc->data.head.hour,
									trc->data.head.minute,
									trc->data.head.second,
									(int) trc->excerpt.length);
		for(i=0;i<strlen(trc->data.head.station) && i < 5;i++)
			request[8+i] = trc->data.head.station[i];

//fprintf(stderr, "sd: %d\trequest: %s\n", args->sd, request);
		ret = send(args->sd, request, strlen(request), 0);
		while (ret != -1)
		{
			memset(buf, 0, sizeof(buf));
			readBytes = recvfrom(args->sd, buf, 4096, 0, NULL, NULL);
//fprintf(stderr, "sd: %d\tbytes read: %d\n", args->sd, readBytes);
			if (readBytes == 0 ||
				!strncmp(buf, CWB_EOR_STRING, strlen(CWB_EOR_STRING)))
			{
//fprintf(stderr, "sd: %d\tbuf: %s\n", args->sd, buf);
				break;
			}
			trc->data.dataInfo.data = realloc(trc->data.dataInfo.data, byteLoc + readBytes);
			memcpy(&trc->data.dataInfo.data[byteLoc], buf, readBytes);
			byteLoc += readBytes;
		}
//fprintf(stderr, "sd: %d\ttotal bytes: %d\n", args->sd, byteLoc);

		if (byteLoc)
		{
			tmpFile = g_strdup_printf("/tmp/%s%s%s%s%d.msd", 
										trc->data.head.network,
										trc->data.head.station,
										trc->data.head.channel,
										trc->data.head.location,
										args->thredNum);
			do 
			{
				if ((fp=fopen(tmpFile, "wb")))
				{
					fwrite(trc->data.dataInfo.data, byteLoc, 1, fp);
					fclose(fp);
					free(trc->data.dataInfo.data);
					trc->data.dataInfo.data = NULL;
					trc->data.fileName = g_strdup(tmpFile);
					args->returnCode = get_mseed_hedr_cwb(trc);
					
				    // stat() the entry for the filesize
				    if (stat(trc->data.fileName, &file_stat) == -1) 
					{	// if we can't stat the file, then we can't read it either, move on...
					    args->returnCode = FILE_DNE;
					    break;
				    }
		
					// a guess at how much space we need, most likely too much, but we give it back
					tot_alloc = file_stat.st_size;
					dataSize = 4;	//trc->data.head.dataSize;
					if ((trc->data.dataInfo.data = malloc(tot_alloc * dataSize))==NULL)
					{	// initialize memory allocation
						fprintf(stderr, "Unable to allocate %d bytes of data.\n", 
										(int) (tot_alloc*dataSize));
						args->returnCode = NO_MEMORY;
						break;
					}
		
					totSamp = get_mseed_trace_cwb(trc, &tot_alloc);
					if (totSamp == FAILURE)
					{
						free(trc->data.dataInfo.data);
						trc->data.dataInfo.data = NULL;
						args->returnCode = FAILURE;
						break;
					}
					trc->data.dataInfo.head.numSamples = totSamp;
					
					if (!totSamp)
					{
						free(trc->data.dataInfo.data);
						trc->data.dataInfo.data = NULL;
						args->returnCode = NOT_ENOUGH_DATA;			// in this case, none!
						break;
					}
			
					if (tot_alloc - totSamp)
					{	// if we alloc'd too much, give it back
						if ((trc->data.dataInfo.data = 
									realloc(trc->data.dataInfo.data, totSamp*dataSize))==NULL)
						{	// this should *never* fail since we're only ever giving memory back
							free(trc->data.dataInfo.data);
							trc->data.dataInfo.data = NULL;
							args->returnCode = NO_MEMORY;
							break;
						}
					}
					get_maxmin_amp(trc);
					remove(tmpFile);
				}
				else
					free(tmpFile);
			} while(FALSE);				// only one-time through
		}
		else
		{
			fprintf(stderr, "No Data returned from CWB request:\t%s", request);
		}
	
		set_CWB_hedr(trc);		// fill in the extra header fields
		free(request);
	}
	
	readCWBReturn(args);
}

static void get_cwb_data_MT(traceInfo *trc)
{
	CWB_THRED_ARGS *args;
	
	if (!cwbQ)
	{
		cwbQ = g_queue_new();
		thredCntr = 0;
	}

	if (numThreds >= MAX_CWB_THREDS)
	{
		g_queue_push_tail(cwbQ, trc);
	}
	else
	{
		numThreds++;
		args = calloc(1, sizeof(CWB_THRED_ARGS));
		args->trc = trc;
		args->thredNum = thredCntr++;
		makeThread(ASYNC, (GThreadFunc) read_CWB_data, args, NULL);		// request next trace from CWB
	}
}

int get_cwb_data(traceInfo *trc)
{
	if (!CWBSRVR.inetAddr)
	{
		CWBSRVR.inetAddr = g_strdup(trc->excerpt.CWB.inetAddr);
		CWBSRVR.port = trc->excerpt.CWB.port;
	}
		 
	if (trc->excerpt.CWB.mainReturn)
	{	// multi-threaded mode
		get_cwb_data_MT(trc);
	}
	else
	{	// coming from PQL, we do not multi-thread
		CWB_THRED_ARGS *args;
		numThreds = 1;
		args = calloc(1, sizeof(CWB_THRED_ARGS));
		args->trc = trc;
		read_CWB_data(args);	
	}
	trc->data.dataInfo.head.data_form = trc->data.head.data_form;
	trc->data.dataInfo.head.sampInt = trc->data.head.sampInt;
	trc->data.dataInfo.head.secsPsamp = (trc->data.head.sampInt/1000000.);
	
	return PQLXSUCCESS;
}
