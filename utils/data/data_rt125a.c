#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "pql_defines.h"
#include "pql_externs.h"
#include "reftek.h"

static int rd_pack(FILE *fp, unsigned char *array)
{
  int 	packet_type;
  
  if (!fread(array, 1, RT125A_PKT_SIZE, fp))
      return EOF;

  switch (packet_type = array[0])
  {
	case PIC:
	case PSH:
	case PDT:
	case PET:
	case PER:
    break;
      
    default:		// invalid RT125A packet type
      packet_type = -10;
  }
  return packet_type;
}

static traceInfo *make_rt125A_trace(RT125ATRACE *event)
{
	traceInfo	*trace;
	char		*chr;
	struct ptime date;
	
	trace = calloc(1, sizeof(traceInfo));
	
	trace->data.fileType = RT125A_DATA;
	
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
//			trace->data.head.channel);

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

static gint getEvent(RT125ATRACE *one, char *eventTime)
{	// return 0 on matching entry
	if (	one->year == RT125AYEAR(eventTime) &&
			one->jday == RT125AJDAY(eventTime) &&
			one->hour == RT125AHOUR(eventTime) &&
			one->minute == RT125AMIN(eventTime) &&
			one->second == RT125ASEC(eventTime))
		return 0;
	else
		return 1;
}

static GSList *get_rt125A_hedr(FILE *fp, char *fileName)
{
    unsigned char  block[RT125A_PKT_SIZE];
	int			packet_type;
	RT125ATRACE	*rt125AEvent;
	GSList		*events=NULL, *event, *traces;

	// first get all event table information
	rewind(fp);
	while ((packet_type = rd_pack(fp, block)) != EOF)
    {
		switch (packet_type)
        {
			case PET:
			{
				int		i;
				RT125A_EVENT *evt = (RT125A_EVENT *) block;

				for(i=0;i<evt->numEntries;i++)
				{
					if (!(event = g_slist_find_custom(events, &evt->events[i].eventTime[0],
									(GCompareFunc) getEvent)))
					{	// guarantees no duplicates
						rt125AEvent = calloc(1, sizeof(RT125ATRACE));
						rt125AEvent->fileName = fileName;
						rt125AEvent->unit_id = RT125AUNITID(evt->unitID);
						rt125AEvent->year = RT125AYEAR(evt->events[i].eventTime);
						rt125AEvent->jday = RT125AJDAY(evt->events[i].eventTime);
						rt125AEvent->hour = RT125AHOUR(evt->events[i].eventTime);
						rt125AEvent->minute = RT125AMIN(evt->events[i].eventTime);
						rt125AEvent->second = RT125ASEC(evt->events[i].eventTime);
						events = g_slist_prepend(events, rt125AEvent);
#if 0
fprintf(stderr, "Event:\t%04d-%03d-%02d:%02d:%02d\n", 
	rt125AEvent->year,
	rt125AEvent->jday,
	rt125AEvent->hour,
	rt125AEvent->minute,
	rt125AEvent->second);
#endif
					}
				}
			}
            break;
		}
	}	

	// now match up events defined in event tables with data packets
	rewind(fp);
	while ((packet_type = rd_pack(fp, block)) != EOF)
    {
		switch (packet_type)
        {
        	case PDT:
        	{
        		RT125A_DATA_PKT	*dp = (RT125A_DATA_PKT *) block;
#if 0
fprintf(stderr, "Data:\t%04d-%03d-%02d:%02d:%02d\n", 
	RT125AYEAR(dp->eventTime),
	RT125AJDAY(dp->eventTime),
	RT125AHOUR(dp->eventTime),
	RT125AMIN(dp->eventTime),
	RT125ASEC(dp->eventTime));
#endif
				if (!(event = g_slist_find_custom(events, &dp->eventTime[0], 
										(GCompareFunc) getEvent)))
				{	// a data packet time does not match anything in our event table, move on
					continue;
				}
				rt125AEvent = event->data;
				if (!rt125AEvent->sampsPsec)
				{
					rt125AEvent->sampsPsec = RT125ASPS(dp->sampleRate);
					RT125AGAIN(dp->gain, rt125AEvent->gain)
					rt125AEvent->eventNum = RT125AEVENTNUM(dp->eventNum);
					
				}
				rt125AEvent->numSamps += dp->sampleCnt;
			}	
            break;
		}
	}	

	for(traces = NULL, event = events; event; event = g_slist_next(event))
	{
		rt125AEvent = event->data;
		if (rt125AEvent->gain)
		{	// if gain == 0 then there was no data packet, ignore
			traces = g_slist_prepend(traces, make_rt125A_trace(rt125AEvent));
		}
		free(event->data);
		event->data = NULL;
	}
	g_slist_free(events);

	return(traces);
}

int get_rt125A_data(traceInfo *trace)
{
    unsigned char	block[RT125A_PKT_SIZE];
	int		packet_type;
	int		sampsRead=0, *intPtr;
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

	intPtr = (int *) trace->data.dataInfo.data;
	while ((packet_type = rd_pack(fp, block)) != EOF)
    {
		switch (packet_type)
        {
        	case PDT:
        	{
        		int		i, rtCntr;
        		RT125A_DATA_PKT	*dp = (RT125A_DATA_PKT *) block;
				if ((RT125AYEAR(dp->eventTime) != trace->data.head.year) ||
					(RT125AJDAY(dp->eventTime) != trace->data.head.day) ||
					(RT125AHOUR(dp->eventTime) != trace->data.head.hour) ||
					(RT125AMIN(dp->eventTime) != trace->data.head.minute) ||
					(RT125ASEC(dp->eventTime) != trace->data.head.second))
				{	// data packet time does not match our trace, skip it
					continue;
				}
				for(i = 0, rtCntr = 0;
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

static int	rt125A_check(FILE *fp)
{
    unsigned char	block[RT125A_PKT_SIZE];
	int		packet_type, ret = FAILURE, i;
	gboolean	foundPET=FALSE, foundPDT=FALSE;

	rewind(fp);
	
	packet_type = rd_pack(fp, block);
	for(i=0;
			i<RT125A_MAX_PKT_RD && 
			packet_type != EOF;
		i++)
	{
		switch(packet_type)
		{	// we define succes as: successfully identify one each of 
			// EVENT and DATA packets within the first RT125A_MAX_PKT_RD packets
			case PET:
				foundPET = TRUE;
			break;
			case PDT:
				foundPDT = TRUE;
			break;
		}
		if (foundPET && foundPDT)
		{
			ret = PQLXSUCCESS;
			break;
		}
		packet_type = rd_pack(fp, block);
	}
	return ret;
}
		
GSList *IDrt125A(FILE *fp, char *fileName)
{
  int 	 ret;
  GSList *traces=NULL;

  rewind(fp);

  ret = rt125A_check(fp);
  
  if (ret != FAILURE)
  {
	traces = get_rt125A_hedr(fp, fileName);
	rewind(fp);
  }
  return traces;
}

GSList *get_rt125A_SOH(FILE *fp)
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
