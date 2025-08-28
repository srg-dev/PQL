#include <string.h>
#include <stdlib.h>
#include "pql_defines.h"
#include "pql_externs.h"
#include "segy.h"

#ifdef SOLARIS2
#include "libgen.h"
#endif
	
unsigned short swap_2byte(unsigned short nValue);
guint32 swap_4byte(guint32 nValue);
float	swap_4byteF(char *nValue);

#define SEGY32	1
#define SEGY16	0	// SEGY data formats

void swapSegyHdr(struct SegyHead *segyHdr)
{
	segyHdr->lineSeq = swap_4byte(segyHdr->lineSeq);
	segyHdr->reelSeq = swap_4byte(segyHdr->reelSeq);
	segyHdr->event_number = swap_4byte(segyHdr->event_number);
	segyHdr->channel_number = swap_4byte(segyHdr->channel_number);
	segyHdr->energySourcePt = swap_4byte(segyHdr->channel_number);
	segyHdr->cdpEns = swap_4byte(segyHdr->cdpEns);
	segyHdr->traceInEnsemble = swap_4byte(segyHdr->traceInEnsemble);
	segyHdr->traceID = swap_2byte(segyHdr->traceID);
	segyHdr->vertSum = swap_2byte(segyHdr->vertSum);
	segyHdr->horSum = swap_2byte(segyHdr->horSum);
	segyHdr->dataUse = swap_2byte(segyHdr->dataUse);
	segyHdr->sourceToRecDist = swap_4byte(segyHdr->sourceToRecDist);
	segyHdr->recElevation = swap_4byte(segyHdr->recElevation);
	segyHdr->sourceSurfaceElevation = swap_4byte(segyHdr->sourceSurfaceElevation);
	segyHdr->sourceDepth = swap_4byte(segyHdr->sourceDepth);
	segyHdr->datumElevRec = swap_4byte(segyHdr->datumElevRec);
	segyHdr->datumElevSource = swap_4byte(segyHdr->datumElevSource);
	segyHdr->sourceWaterDepth = swap_4byte(segyHdr->sourceWaterDepth);
	segyHdr->recWaterDepth = swap_4byte(segyHdr->recWaterDepth);
	segyHdr->elevationScale = swap_2byte(segyHdr->elevationScale);
	segyHdr->coordScale = swap_2byte(segyHdr->coordScale);
	segyHdr->sourceLongOrX = swap_4byte(segyHdr->sourceLongOrX);
	segyHdr->sourceLatOrY = swap_4byte(segyHdr->sourceLatOrY);
	segyHdr->recLongOrX = swap_4byte(segyHdr->recLongOrX);
	segyHdr->recLatOrY = swap_4byte(segyHdr->recLatOrY);
	segyHdr->coordUnits = swap_2byte(segyHdr->coordUnits);
	segyHdr->weatheringVelocity = swap_2byte(segyHdr->weatheringVelocity);
	segyHdr->subWeatheringVelocity = swap_2byte(segyHdr->subWeatheringVelocity);
	segyHdr->sourceUpholeTime = swap_2byte(segyHdr->sourceUpholeTime);
	segyHdr->recUpholeTime = swap_2byte(segyHdr->recUpholeTime);
	segyHdr->sourceStaticCor = swap_2byte(segyHdr->sourceStaticCor);
	segyHdr->recStaticCor = swap_2byte(segyHdr->recStaticCor);
	segyHdr->totalStatic = swap_2byte(segyHdr->totalStatic);
	segyHdr->lagTimeA = swap_2byte(segyHdr->lagTimeA);
	segyHdr->lagTimeB = swap_2byte(segyHdr->lagTimeB);
	segyHdr->delay = swap_2byte(segyHdr->delay);
	segyHdr->muteStart = swap_2byte(segyHdr->muteStart);
	segyHdr->muteEnd = swap_2byte(segyHdr->muteEnd);
	segyHdr->sampleLength = swap_2byte(segyHdr->sampleLength);
	segyHdr->deltaSample = swap_2byte(segyHdr->deltaSample);
	segyHdr->gainType = swap_2byte(segyHdr->gainType);
	segyHdr->gainConst = swap_2byte(segyHdr->gainConst);
	segyHdr->initialGain = swap_2byte(segyHdr->initialGain);
	segyHdr->correlated = swap_2byte(segyHdr->correlated);
	segyHdr->sweepStart = swap_2byte(segyHdr->sweepStart);
	segyHdr->sweepEnd = swap_2byte(segyHdr->sweepEnd);
	segyHdr->sweepLength = swap_2byte(segyHdr->sweepLength);
	segyHdr->sweepType = swap_2byte(segyHdr->sweepType);
	segyHdr->sweepTaperAtStart = swap_2byte(segyHdr->sweepTaperAtStart);
	segyHdr->sweepTaperAtEnd = swap_2byte(segyHdr->sweepTaperAtEnd);
	segyHdr->taperType = swap_2byte(segyHdr->taperType);
	segyHdr->aliasFreq = swap_2byte(segyHdr->aliasFreq);
	segyHdr->aliasSlope = swap_2byte(segyHdr->aliasSlope);
	segyHdr->notchFreq = swap_2byte(segyHdr->notchFreq);
	segyHdr->notchSlope = swap_2byte(segyHdr->notchSlope);
	segyHdr->lowCutFreq = swap_2byte(segyHdr->lowCutFreq);
	segyHdr->hiCutFreq = swap_2byte(segyHdr->hiCutFreq);
	segyHdr->lowCutSlope = swap_2byte(segyHdr->lowCutSlope);
	segyHdr->hiCutSlope = swap_2byte(segyHdr->hiCutSlope);
	segyHdr->year = swap_2byte(segyHdr->year);
	segyHdr->day = swap_2byte(segyHdr->day);
	segyHdr->hour = swap_2byte(segyHdr->hour);
	segyHdr->minute = swap_2byte(segyHdr->minute);
	segyHdr->second = swap_2byte(segyHdr->second);
	segyHdr->timeBasisCode = swap_2byte(segyHdr->timeBasisCode);
	segyHdr->traceWeightingFactor = swap_2byte(segyHdr->traceWeightingFactor);
	segyHdr->phoneRollPos1 = swap_2byte(segyHdr->phoneRollPos1);
	segyHdr->phoneFirstTrace = swap_2byte(segyHdr->phoneFirstTrace);
	segyHdr->phoneLastTrace = swap_2byte(segyHdr->phoneLastTrace);
	segyHdr->gapSize = swap_2byte(segyHdr->gapSize);
	segyHdr->taperOvertravel = swap_2byte(segyHdr->taperOvertravel);
	segyHdr->totalStaticHi = swap_2byte(segyHdr->totalStaticHi);
	segyHdr->samp_rate = swap_4byte(segyHdr->samp_rate);
	segyHdr->data_form = swap_2byte(segyHdr->data_form);
	segyHdr->m_secs = swap_2byte(segyHdr->m_secs);
	segyHdr->trigyear = swap_2byte(segyHdr->trigyear);
	segyHdr->trigday = swap_2byte(segyHdr->trigday);
	segyHdr->trighour = swap_2byte(segyHdr->trighour);
	segyHdr->trigminute = swap_2byte(segyHdr->trigminute);
	segyHdr->trigsecond = swap_2byte(segyHdr->trigsecond);
	segyHdr->trigmills = swap_2byte(segyHdr->trigmills);
	segyHdr->scale_fac = swap_4byteF((char *) &(segyHdr->scale_fac));
	segyHdr->inst_no = swap_2byte(segyHdr->inst_no);
	segyHdr->not_to_be_used = swap_2byte(segyHdr->not_to_be_used);
	segyHdr->num_samps = swap_4byte(segyHdr->num_samps);
	segyHdr->max = swap_4byte(segyHdr->max);
	segyHdr->min = swap_4byte(segyHdr->min);
   return;
}

int get_segy_hedr(FILE *fp, traceInfo *trace)
{
  struct SegyHead	segy_head;
  struct ptime    date;
  char	*chr;
  float	oneSample;

	rewind(fp);

	if (fread(&segy_head, 240, 1, fp) == 0) 
	{
		return(FAILURE);
	}

	if (trace->data.head.endianSwap)
	{
		swapSegyHdr(&segy_head);
	}

    trace->data.head.data_form = ((segy_head.data_form == SEGY32) ? BIT32 : BIT16);
    trace->data.head.dataSize = ((segy_head.data_form == SEGY32) ? sizeof(int) : sizeof(short));
    if (segy_head.year < 1970) 
	{
      segy_head.year = 1970;
	  segy_head.day = 0;
	  segy_head.hour = 0;
	  segy_head.minute = 0;
	  segy_head.second = 0;
	  segy_head.m_secs = 0;
    }
    trace->data.head.year = segy_head.year;
  	trace->data.head.day = segy_head.day;
   	trace->data.head.hour = segy_head.hour;
   	trace->data.head.minute = segy_head.minute;
   	trace->data.head.second = segy_head.second;
   	trace->data.head.m_secs = segy_head.m_secs;
    trace->data.head.gainConst = segy_head.gainConst;
    trace->data.head.scale_fac = segy_head.scale_fac;
	if(trace->data.head.gainConst == 0) 		
	  		trace->data.head.gainConst = 1.0;
	if(trace->data.head.scale_fac == 0)  
	  		trace->data.head.scale_fac = 1.0;

    if (segy_head.num_samps == 0)
		trace->data.dataInfo.head.numSamples = segy_head.sampleLength;
    else
		trace->data.dataInfo.head.numSamples = segy_head.num_samps;

    if (segy_head.deltaSample != 1)
		trace->data.head.sampInt = segy_head.deltaSample;
    else
		trace->data.head.sampInt = segy_head.samp_rate;

	
    oneSample = 1./(1000000./trace->data.head.sampInt);
	trace->data.head.length = (double) (trace->data.dataInfo.head.numSamples * 
									(float) (trace->data.head.sampInt/1000000.) - oneSample);

	sprintf(trace->data.head.sensorID, "%d", segy_head.inst_no);
	sprintf(trace->data.head.channel, "%d", (int) segy_head.channel_number);

      trace->data.head.descr1 = g_strdup_printf("%i %03i %02i:%02i" ,
	      segy_head.year,
	      segy_head.day,
	      segy_head.hour,
	      segy_head.minute);
      
      trace->data.head.descr2 = g_strdup_printf("ID: %03i C: %i" , 
      								segy_head.inst_no, (int) segy_head.channel_number);

	trace->data.head.longFN = trace->data.fileName;
	trace->data.head.shortFN = 
#ifndef WIN32
				g_strdup((chr = strrchr(trace->data.fileName,'/')) ? chr+1 : trace->data.fileName);
#else
				g_strdup((chr = strrchr(trace->data.fileName,'\\')) ? chr+1 : trace->data.fileName);
#endif
	
	date.yr = trace->data.head.year;
	date.day = trace->data.head.day;
	date.hr = trace->data.head.hour;
	date.mn = trace->data.head.minute;
	date.sec = trace->data.head.second;
	trace->data.head.epochs = passcal_htoe(&date, FALSE) + trace->data.head.m_secs/1000.;

	trace->data.head.fileHdr = calloc(1, sizeof(struct SegyHead));
	memcpy(trace->data.head.fileHdr, &segy_head, sizeof(struct SegyHead));

	return PQLXSUCCESS;
}

int
get_segy_data(traceInfo *trace)
{

/*******************************************************************************

this procedure reads in the header and corresponding data for each trace to be
plotted onto the main screen.  each header and array for data is allocated
each time the header and data are read in.  before reading in new headers and
data (via the next or prev buttons on main screen) the space used to store the
old headers and data is freed.

*******************************************************************************/

    int		    no_read, i;
    char            dummy[3];
    struct SegyHead	segy_head;
	FILE *fp;

	if ((fp=fopen(trace->data.fileName, "rb")) == NULL)
	{
		return(FAILURE);
	}

    if (fread(&segy_head, 240, 1, fp) == 0) {	/* read header   */
		fprintf(stderr, "Unable to read header for file %s, skipping.", trace->data.fileName);
		return(FAILURE);
    }

	if (trace->data.head.endianSwap)
	{
		swapSegyHdr(&segy_head);
	}

    if (trace->data.head.sampInt < 1) 
	{
		return(FAILURE);
    }

    if (trace->data.dataInfo.head.numSamples <= 0) 
	{
		return (FAILURE);
    }
	
	if ((trace->data.dataInfo.data = (char *) calloc((unsigned) trace->data.dataInfo.head.numSamples, 
												trace->data.head.dataSize)) == NULL) 
	{
		return(FAILURE);
	}
	if ((no_read = fread(trace->data.dataInfo.data, 
							trace->data.head.dataSize, 
							trace->data.dataInfo.head.numSamples, fp)) != trace->data.dataInfo.head.numSamples) 
	{	
		fprintf(stderr, "%s:\tRead %d samples (Expected %d), possible problem with header\n",  
			trace->data.head.longFN, no_read, trace->data.dataInfo.head.numSamples);
		trace->data.dataInfo.head.numSamples = no_read;
	}
	
	if (trace->data.head.endianSwap)
	{
		switch (segy_head.data_form) 
		{	
			case SEGY32:
				{
					gint32 *ln32_dp = (gint32 *) trace->data.dataInfo.data;
					for(i=0;i<no_read;i++)
					{
						ln32_dp[i] = swap_4byte(ln32_dp[i]);
					}
				}
				break;
		
			case SEGY16:
				{
					short *sh_dp = (short *) trace->data.dataInfo.data;
					for(i=0;i<no_read;i++)
					{
						sh_dp[i] = swap_2byte(sh_dp[i]);
					}
				}
				break;
		}
	}

    if (fread(dummy, 1, 2, fp) != 0) 
	{
      fprintf(stderr,"%s:\tUnread data points in file, possible problem with the header\n",  
					  trace->data.head.longFN);
    }

	trace->data.dataInfo.head.data_form = trace->data.head.data_form;
	trace->data.dataInfo.head.sampInt = trace->data.head.sampInt;
	trace->data.dataInfo.head.secsPsamp = (trace->data.head.sampInt/1000000.);

	fclose(fp);
    return(0);
}

static int check_endian(struct SegyHead *segy_head)
{
	int	ret=FAILURE;
  do {
  if (	!((segy_head->year ==0) || 
		(segy_head->samp_rate == 0 && segy_head->deltaSample == 0) ||
		(segy_head->day < 0 || segy_head->day > 366) ||
		(segy_head->hour < 0 || segy_head->hour > 23) ||
		(segy_head->minute < 0 || segy_head->minute > 59) ||
		(segy_head->second < 0 || segy_head->second > 60))) 
  {		// file succeeds criteria for SEGY
	ret = FALSE;
  	break;
  }

  // swap some header values (endianness) and check again
  segy_head->year = swap_2byte(segy_head->year);
  segy_head->day = swap_2byte(segy_head->day);
  segy_head->hour = swap_2byte(segy_head->hour);
  segy_head->minute = swap_2byte(segy_head->minute);
  segy_head->second = swap_2byte(segy_head->second);
  segy_head->samp_rate = swap_4byte(segy_head->samp_rate);
  
  if (	!((segy_head->year ==0) || 
		(segy_head->samp_rate == 0 && segy_head->deltaSample == 0) ||
		(segy_head->day < 0 || segy_head->day > 366) ||
		(segy_head->hour < 0 || segy_head->hour > 23) ||
		(segy_head->minute < 0 || segy_head->minute > 59) ||
		(segy_head->second < 0 || segy_head->second > 60))) 
  {		// file succeeds criteria for SEGY
	ret = TRUE;
  }
  break;
  } while(1);
  return ret;
}

int check_segy(FILE *fp, traceInfo *trace)
{
  int ret=FAILURE;
  struct SegyHead	segy_head;

  rewind(fp);

  if (fread(&segy_head, 240, 1, fp) == 0) 
  {	
  	return(ret);
  }

  ret = check_endian(&segy_head);
  
  if (ret != FAILURE)
  {
	trace->data.fileType = SEGY_DATA;
	trace->data.head.endianSwap = ret;
	ret = get_segy_hedr(fp, trace);
	rewind(fp);
  }
  return ret;
}

GSList *IDsegy(FILE *fp, char *fileName)
{
  int ret=FAILURE, endianSwap;
  struct SegyHead	segy_head;
  traceInfo	*trace;
  GSList *traces=NULL;

  rewind(fp);

  if (fread(&segy_head, 240, 1, fp) == 0) 
  {	
  	return(NULL);
  }

  ret = endianSwap = check_endian(&segy_head);
  
  if (endianSwap != FAILURE)
  {
  	trace = calloc(1, sizeof(traceInfo));
  	trace->data.fileName = g_strdup(fileName);
	trace->data.fileType = SEGY_DATA;
	trace->data.head.endianSwap = endianSwap;
	ret = get_segy_hedr(fp, trace);
	if (ret==PQLXSUCCESS)
		traces = g_slist_append(traces, trace);
	else
		freeData2(trace);
	rewind(fp);
  }
  return traces;
}
