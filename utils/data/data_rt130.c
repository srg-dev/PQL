#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <glib.h>
#include "pql_defines.h"
#include "pql_externs.h"
#include "reftek.h"

static gboolean rev;

static int rd_pack(FILE *fp, unsigned char *array)
{
	int 	packet_type, ConvertRefType();
	
	if (!fread(array, 1, RT130_PKT_SIZE, fp))
	  return EOF;
	
	switch ( packet_type = ConvertRefType( (char *) &array[0])) 
	{
		case DT:
		case ET:
		case EH:
		case DS:
		case SC:
		case CD:
		case OM:
		case AD:
		case DR:
		case SH:
		case FD:
		break;
		  
		default:		// invalid REFTEK packet type
		  packet_type = 0;
	}
	
	return packet_type;
}

static traceInfo *make_rt130_trace(EHINFO *EHInfo, CHANINFO *chn)
{
	traceInfo	*trace;
	struct ptime    date;
	char		*chr;
	GString	*sensorID;
	
	trace = calloc(1, sizeof(traceInfo));
	
	trace->data.fileType = RT130_DATA;
	
	passcal_etoh(&date, (double) EHInfo->epoch.sec);
	trace->data.head.year = date.yr;
	trace->data.head.day = get_julday(date.mo, date.day, date.yr);	
	trace->data.head.hour = date.hr;
	trace->data.head.minute = date.mn;
	trace->data.head.second = date.sec;
	trace->data.head.m_secs = EHInfo->epoch.usec/1000;
	trace->data.head.epochs = EHInfo->epoch.sec + trace->data.head.m_secs/1000.;

	trace->data.head.gainConst = (chn->gain ? chn->gain : 1);
	trace->data.head.scale_fac = 1;
	
	sprintf(trace->data.head.sensorID, "%0x", EHInfo->unit_id);
	sensorID = g_string_new(trace->data.head.sensorID);
	g_string_ascii_up(sensorID);
	sprintf(trace->data.head.sensorID, "%s", sensorID->str);
	g_string_free(sensorID, TRUE);
	
	trace->data.head.sampInt = 1000000./EHInfo->sampsPsec;
	sprintf(trace->data.head.channel, "%d:%d", chn->strmNm, chn->chnNm);

    trace->data.head.descr1 = g_strdup_printf("%i %03i %02i:%02i" ,
	      trace->data.head.year,
	      trace->data.head.day,
	      trace->data.head.hour,
	      trace->data.head.minute);
    trace->data.head.descr2 = g_strdup_printf("ID: %s C: %s" , 
    		trace->data.head.sensorID, 
			trace->data.head.channel);

	trace->data.fileName = g_strdup(EHInfo->fileName);
	trace->data.head.longFN = trace->data.fileName;
	trace->data.head.shortFN = 
#ifndef WIN32
			g_strdup_printf("%s.%s",
			((chr = strrchr(trace->data.fileName,'/')) ? chr+1 : trace->data.fileName),
			trace->data.head.channel);
#else
			g_strdup_printf("%s.%s",
			((chr = strrchr(trace->data.fileName,'\\')) ? chr+1 : trace->data.fileName),
			trace->data.head.channel);
#endif
	
	switch(chn->data_form)
	{
		case 0x16:
			trace->data.head.data_form = BIT16;
			trace->data.head.dataSize = sizeof(short);
		break;
		case 0x32:
		case 0xc0:
		case 0xc2:
			trace->data.head.data_form = BIT32;
			trace->data.head.dataSize = sizeof(int);
		break;
	}
	trace->data.dataInfo.head.numSamples = chn->numSamps;

	return(trace);
}

static gint getChannel(CHANINFO *one, int *chnNum)
{
	return(one->chnNum - *chnNum);
}

static GSList *get_rt130_hedr(FILE *fp, char *fileName)
{
	RTTime		start;
	EHINFO		EHInfo;
    unsigned char  block[RT130_PKT_SIZE];
	int			packet_type, chnNum, *gain=NULL;
	CHANINFO	*chanInfo;
	GSList		*channels=NULL, *chn, *traces=NULL;

	rewind(fp);
	EHInfo.fileName = fileName;
	while ((packet_type = rd_pack(fp, block)) != EOF)
    {
		switch (packet_type)
        {
        	case DT:
        	{
				chnNum = RT130CHANNELNUM;
				if (!(chn = g_slist_find_custom(channels, &chnNum, (GCompareFunc) getChannel)))
				{	// make new channel entry
					chanInfo = calloc(1, sizeof(CHANINFO));
					chanInfo->fileName = fileName;
					chanInfo->chnNum = chnNum;				// channel number
					chanInfo->data_form = RT130DATAFORM;	// RT130 data format: 16, 32, 0xc0, 0xc2
					chanInfo->strmNm = RT130STREAMNM;		// stream name
					chanInfo->chnNm = RT130CHANNELNM;		// channel name
					if (gain)
						chanInfo->gain = gain[chanInfo->chnNm-1];
					channels = g_slist_prepend(channels, chanInfo);
					chn = channels;
				}
				chanInfo = chn->data;
  				chanInfo->numSamps += RT130NUMSAMPS;
			}	
            break;

			case EH:
			{
				int	totChans, i;
				EventHeader *eh = (EventHeader *) block;
				EHInfo.unit_id  = RT130UNITID;
				EHInfo.sampsPsec = atof(eh->sample_rate);
				ascii2bin (&start, &eh->first_sample_time);
				gmt2epoch (&EHInfo.epoch, &start, 0);
				if (!gain)
				{
					totChans = atoi(eh->totalChannels);
					if (totChans)
					{
						gain = calloc(totChans, sizeof(int *));
						for(i=0;i<totChans;i++)
							RT130CHGAIN(eh->gain[i], gain[i])
					}
				}
			}
            break;
		}
	}	

	for(chn = channels; chn; chn = g_slist_next(chn))
	{
		traces = g_slist_prepend(traces, make_rt130_trace(&EHInfo, (CHANINFO*)chn->data));
		free(chn->data);
		chn->data = NULL;
	}
	g_slist_free(channels);
	free(gain);

	return(traces);
}

int get_rt130_data(traceInfo *trace)
{
    unsigned char	block[RT130_PKT_SIZE];
	int		packet_type, chnNum;
	int		sampsRead=0, numSamps;
	float	oneSample;
	int		last_x=INT_MIN;	// initialize data decompression - 1st point is real
	FILE *fp;

	if ((fp=fopen(trace->data.fileName, "rb")) == NULL)
	{
		return(FAILURE);
	}

	if ((trace->data.dataInfo.data = (char *)
				calloc((unsigned)trace->data.dataInfo.head.numSamples, 
						trace->data.head.dataSize)) == NULL) 
	{
		return(FAILURE);
	}

	while ((packet_type = rd_pack(fp, block)) != EOF)
    {
		switch (packet_type)
        {
        	case DT:
        	{
				chnNum = RT130CHANNELNUM;
				if (chnNum == HDRCHANNELNUM(trace->data.head.channel))
				{
					numSamps = RT130NUMSAMPS;
					switch(RT130DATAFORM)
					{
						case 0x32:
						{
							int *data_ptr;
							data_ptr = (int *) &block[RT130_HEADER_SIZE];
							memcpy(&trace->data.dataInfo.data[sampsRead * trace->data.head.dataSize],
									data_ptr, trace->data.head.dataSize * numSamps);
							sampsRead += numSamps;
						}
						break;
						
						case 0x16:
						{
							short *data_ptr;
							data_ptr = (short *) &block[RT130_HEADER_SIZE];
							memcpy(&trace->data.dataInfo.data[sampsRead * trace->data.head.dataSize], 
									data_ptr, trace->data.head.dataSize * numSamps);
							sampsRead += numSamps;
						}
						break;
						
						case 0xc0:
						{
							int	data[RT130_C0_MAX_SAMPS];
							memset(data, 0, sizeof(int)*RT130_C0_MAX_SAMPS);
							numSamps = steim1 ((char *) block, data, &last_x, DEFAULT_MSEED_BLOCKSIZE, rev);
							if (numSamps == -1)	
							{	// error
								free(trace->data.dataInfo.data);
								trace->data.dataInfo.data = NULL;
								fclose(fp);
								return(FAILURE);
							}
							memcpy(&trace->data.dataInfo.data[sampsRead * trace->data.head.dataSize], 
									data, trace->data.head.dataSize * numSamps);
							sampsRead += numSamps;
						}
						break;

						case 0xc2:
						{
							int	data[RT130_C2_MAX_SAMPS];
							memset(data, 0, sizeof(int)*RT130_C2_MAX_SAMPS);
							numSamps = steim2 ((char *) block, data, &last_x, DEFAULT_MSEED_BLOCKSIZE, rev);
							if (numSamps == -1)	
							{	// error
								free(trace->data.dataInfo.data);
								trace->data.dataInfo.data = NULL;
								fclose(fp);
								return(FAILURE);
							}
							memcpy(&trace->data.dataInfo.data[sampsRead * trace->data.head.dataSize], 
									data, trace->data.head.dataSize * numSamps);
							sampsRead += numSamps;
						}
						break;
					}
				}
			}	
            break;
		}
	}	

	trace->data.dataInfo.head.numSamples = sampsRead;
    oneSample = 1./(1000000./trace->data.head.sampInt);
	trace->data.head.length = (double) (trace->data.dataInfo.head.numSamples * 
								(float) (trace->data.head.sampInt/1000000.) - oneSample);
	trace->data.dataInfo.head.data_form = trace->data.head.data_form;
	trace->data.dataInfo.head.sampInt = trace->data.head.sampInt;
	trace->data.dataInfo.head.secsPsamp = (trace->data.head.sampInt/1000000.);
	fclose(fp);
	return(PQLXSUCCESS);
}

static int	rt130_check(FILE *fp, char * fileName)
{
    unsigned char	block[RT130_PKT_SIZE];
	int		packet_type, ret = FAILURE;

	rewind(fp);
	
	packet_type = rd_pack(fp, block);
	switch(packet_type)
	{	// we define succes as: first block of file is EH and represents a valid data format
		case EH:
			switch(RT130DATAFORM)
			{
				case 0x16:
				case 0x32:
				case 0xc0:
				case 0xc2:
					ret = PQLXSUCCESS;
				break;
				default:
					fprintf(stderr, "%s: First Packet is EH (good), data format is %x (unhandled)\n", 
								fileName, RT130DATAFORM);
				break;
			}
		break;
	}
	return ret;
}
	
GSList *IDrt130(FILE *fp, char *fileName)
{
  static gboolean first = TRUE;
  int 	 ret;
  GSList *traces=NULL;

  if (first)
  {
	  rev = (endian() == LITTLE_END) ? TRUE : FALSE;
	  first = FALSE;
  }
  rewind(fp);

  ret = rt130_check(fp, fileName);
  
  if (ret != FAILURE)
  {
	traces = get_rt130_hedr(fp, fileName);
	rewind(fp);
  }
  return traces;
}

GSList *get_rt130_SOH(FILE *fp)
{
	GSList	*sohMsgList=NULL;
    unsigned char	block[RT130_PKT_SIZE];
	int		packet_type;
	char	*CRLF = g_strdup_printf("%c%c", CR, LF);

	rewind(fp);
	packet_type = rd_pack(fp, block);
	while(packet_type != EOF)
	{
		switch(packet_type)
		{
			case SH:
			{
				static int seqNum=1;
				char *ptr, *loc, *time;
        		RT130_SOH_PKT	*soh = (RT130_SOH_PKT *) block;
        		SOHINFO			*sohInfo = calloc(1, sizeof(SOHINFO));
        		
        		sohInfo->seqnum = seqNum++;
        		sohMsgList = g_slist_append(sohMsgList, sohInfo);
				ptr = (char *) soh->information;
				while((int) ((long)ptr-(long)soh) < RT130_PKT_SIZE)
				{
					SOHMSG *sohMsg;
					sohMsg = calloc(1, sizeof(SOHMSG));
					time = ptr;
//					sohMsg->year = RT125AYEAR(time);
					sohMsg->jday = atoi(time);
					time += 4;
					sohMsg->hour = atoi(time);
					time += 3;
					sohMsg->minute = atoi(time);
					time += 3;
					sohMsg->second = atoi(time);
					ptr = time+3;
					loc = strstr(ptr, CRLF);
					if (!loc)
						break;
					sohMsg->msg = g_strndup((gchar *) ptr, (int) (loc - ptr));
//fprintf(stderr, "%04d-%03d/%02d:%02d:%02d\tmsg:\t%s\n",
//sohMsg->year, sohMsg->jday, sohMsg->hour, sohMsg->minute, sohMsg->second, sohMsg->msg);
					sohInfo->msgs = g_slist_append(sohInfo->msgs, sohMsg);
					ptr += 2 + (int) (loc-ptr);
				}
			}
			break;
		}
		packet_type = rd_pack(fp, block);
	}
	
	free(CRLF);
	return(sohMsgList);
}
