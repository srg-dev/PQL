#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "pql_defines.h"
#include "pql_externs.h"
#include "reftek.h"

static int rd_pack(FILE *fp, unsigned char *array)
{
  int 	packet_type;
  
  if (!fread(array, 1, RT125_PKT_SIZE, fp))
      return EOF;

  switch (packet_type = array[0])
  {
	case PIC:
	case PSH:
	case PDT:
	case PET:
	case PER:
    break;
      
    default:		// invalid RT125 packet type
      packet_type = -10;
  }
  return packet_type;
}

static traceInfo *make_rt125_trace(RT125ATRACE *event)
{
	traceInfo	*trace;
	char		*chr;
	struct ptime date;
	
	trace = calloc(1, sizeof(traceInfo));
	
	trace->data.fileType = RT125_DATA;
	
	trace->data.head.year = event->year;
	trace->data.head.day = event->jday;	
	trace->data.head.hour = event->hour;
	trace->data.head.minute = event->minute;
	trace->data.head.second = event->second;
//	trace->data.head.m_secs = EHInfo->epoch.usec/1000;

	date.yr = trace->data.head.year;
	date.day = trace->data.head.day;
	date.hr = trace->data.head.hour;
	date.mn = trace->data.head.minute;
	date.sec = trace->data.head.second;
//	passcal_month_day(&date, date.day);
	trace->data.head.epochs = passcal_htoe(&date, FALSE);	// + trc->data.head.m_secs/1000.;

	trace->data.head.gainConst = event->gain;
	trace->data.head.scale_fac = 1;
	
	sprintf(trace->data.head.sensorID, "%0d", event->unit_id);
	
	trace->data.head.sampInt = 1000000./event->sampsPsec;
	sprintf(trace->data.head.channel, "%d", event->eventNum);

    trace->data.head.descr1 = g_strdup_printf("%i %03i %02i:%02i" ,
	      trace->data.head.year,
	      trace->data.head.day,
	      trace->data.head.hour,
	      trace->data.head.minute);
    trace->data.head.descr2 = g_strdup_printf("ID: %s E#: %d" , 
    		trace->data.head.sensorID,
    		event->eventNum);

	trace->data.fileName = g_strdup(event->fileName);
	trace->data.head.longFN = trace->data.fileName;
	trace->data.head.shortFN = 
#ifndef WIN32
			g_strdup_printf("%s:#%d",
			((chr = strrchr(trace->data.fileName,'/')) ? chr+1 : trace->data.fileName),
			event->eventNum);
#else
			g_strdup_printf("%s:#%d",
			((chr = strrchr(trace->data.fileName,'\\')) ? chr+1 : trace->data.fileName),
			event->eventNum);
#endif
	
	trace->data.head.data_form = BIT32;
	trace->data.head.dataSize = sizeof(int);
	trace->data.dataInfo.head.numSamples = event->numSamps;

	return(trace);
}

static gint getEvent(RT125ATRACE *one, int *eventNum)
{
	return(one->eventNum - *eventNum);
}

static GSList *get_rt125_hedr(FILE *fp, char *fileName)
{
    unsigned char  block[RT125_PKT_SIZE];
	int			packet_type;
	RT125ATRACE	*rt125Event;
	GSList		*events=NULL, *event, *traces;

	rewind(fp);
	while ((packet_type = rd_pack(fp, block)) != EOF)
    {
		switch (packet_type)
        {
        	case PDT:
        	{
        		RT125_DATA_PKT	*dp = (RT125_DATA_PKT *) block;
        		RT125_EXT_HEAD	*extH = (RT125_EXT_HEAD *) &dp->data[0];
        		int				eventNum = RT125AEVENTNUM(dp->eventNum);

        		if ((dp->flags | RT125_EVENT_FIRST_PAGE) ||
        			(dp->flags | RT125_EXTENDED_HEADER))
        		{
					if (!(event = g_slist_find_custom(events, &eventNum, 
										(GCompareFunc) getEvent)))
					{	// don't make two 2 traces for the same event
						rt125Event = calloc(1, sizeof(RT125ATRACE));
						rt125Event->fileName = fileName;
						rt125Event->unit_id = RT125AUNITID(dp->unitID);
						rt125Event->year = RT125AYEAR(extH->eventTime);
						rt125Event->jday = RT125AJDAY(extH->eventTime);
						rt125Event->hour = RT125AHOUR(extH->eventTime);
						rt125Event->minute = RT125AMIN(extH->eventTime);
						rt125Event->second = RT125ASEC(extH->eventTime);
						rt125Event->sampsPsec = RT125ASPS(extH->sampleRate);
						rt125Event->eventNum = RT125AEVENTNUM(dp->eventNum);
						events = g_slist_prepend(events, rt125Event);
					}
					else
					{
						rt125Event = event->data;
					}
				}
				else
				{	// not the first data record, get our record
					if (!(event = g_slist_find_custom(events, &eventNum, 
										(GCompareFunc) getEvent)))
					{
						continue;
					}
					rt125Event = event->data;
				}
				rt125Event->numSamps += dp->sampleCnt;
			}	
            break;
		}
	}	

	for(traces = NULL, event = events; event; event = g_slist_next(event))
	{
		rt125Event = event->data;
		if (rt125Event->numSamps)
		{	// if gain == 0 then there was no data packet, ignore
			traces = g_slist_prepend(traces, make_rt125_trace(rt125Event));
		}
//		free(rt125Event->eventTime);
		free(event->data);
		event->data = NULL;
	}
	g_slist_free(events);

	return(traces);
}

int get_rt125_data(traceInfo *trace)
{
    unsigned char	block[RT125_PKT_SIZE];
	int		packet_type;
	int		sampsRead=0;
	float	oneSample;
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
        	case PDT:
        	{
        		int		i, rtCntr, *intPtr;
        		RT125_DATA_PKT	*dp = (RT125_DATA_PKT *) block;
        		if ((RT125AEVENTNUM(dp->eventNum) != atoi(trace->data.head.channel)))
				{	// data packet event # does not match our trace, skip it
					continue;
				}
				intPtr = (int *) &trace->data.dataInfo.data[sampsRead*trace->data.head.dataSize];
        		if ((dp->flags | RT125_EVENT_FIRST_PAGE) ||
        			(dp->flags | RT125_EXTENDED_HEADER))
        		{
        			rtCntr = 9;
        		}
        		else
        		{
        			rtCntr = 0;
        		}
				for(i = 0;
					i < dp->sampleCnt;
					i++, sampsRead++, intPtr++, rtCntr+=3)
				{
					*intPtr = (int) ((dp->data[rtCntr]<<16) | 
							  		 (dp->data[rtCntr+1]<<8) | 
							  		 (dp->data[rtCntr+2]));
					if (dp->data[rtCntr] & 0x80)
						*intPtr = *intPtr | 0xFF000000;
				}
			}	
            break;
		}
	}	

	if (!sampsRead)
	{
		free(trace->data.dataInfo.data);
		trace->data.dataInfo.data = NULL;
		return NOT_ENOUGH_DATA;
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

static int	rt125_check(FILE *fp)
{
    unsigned char	block[RT125_PKT_SIZE];
	int		packet_type, ret = FAILURE, i;
	gboolean	foundPSH=FALSE, foundPDT=FALSE;

	rewind(fp);
	
	packet_type = rd_pack(fp, block);
	for(i=0;
			i<RT125_MAX_PKT_RD && 
			packet_type != EOF;
		i++)
	{
		switch(packet_type)
		{	// we define succes as: successfully identify one each of 
			// SOH and DATA packets within the first RT125_MAX_PKT_RD packets
			case PSH:
				foundPSH = TRUE;
			break;
			case PDT:
				foundPDT = TRUE;
			break;
		}
		if (foundPSH && foundPDT)
		{
			ret = PQLXSUCCESS;
			break;
		}
		packet_type = rd_pack(fp, block);
	}
	return ret;
}
		
GSList *IDrt125(FILE *fp, char *fileName)
{
  int 	 ret;
  GSList *traces=NULL;

  rewind(fp);

  ret = rt125_check(fp);
  
  if (ret != FAILURE)
  {
	traces = get_rt125_hedr(fp, fileName);
	rewind(fp);
  }
  return traces;
}

GSList *get_rt125_SOH(FILE *fp)
{
	GSList	*sohMsgList=NULL;
    unsigned char	block[RT125_PKT_SIZE];
	int		packet_type;
	char	*CRLF = g_strdup_printf("%c%c", CR, LF);

	rewind(fp);
	
	packet_type = rd_pack(fp, block);
	while(packet_type != EOF)
	{
		switch(packet_type)
		{
			case PSH:
			{
				int		i, msgCnt;
				unsigned char	*time;
				char *ptr, *loc;
        		RT125_SOH_PKT	*soh = (RT125_SOH_PKT *) block;
        		SOHINFO			*sohInfo = calloc(1, sizeof(SOHINFO));
        		
        		sohInfo->seqnum = RT125ASOHSEQN(soh->sequenceNum);
        		sohMsgList = g_slist_append(sohMsgList, sohInfo);
				msgCnt = (int) soh->msgCnt;
				ptr = (char *) soh->msgs;
				for(i=0;i<msgCnt;i++)
				{
					SOHMSG *sohMsg;
					sohMsg = calloc(1, sizeof(SOHMSG));
					time = (unsigned char *) ptr;
					sohMsg->year = RT125AYEAR(time);
					sohMsg->jday = RT125AJDAY(time);
					sohMsg->hour = RT125AHOUR(time);
					sohMsg->minute = RT125AMIN(time);
					sohMsg->second = RT125ASEC(time);
					ptr += 6;
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
