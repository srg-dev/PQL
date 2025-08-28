#include "pql_defines.h"
#include "pql_externs.h"

void get_maxmin(traceInfo *trc, DATAINFO *dataInfo)
{
/*******************************************************************************

find the maximum and minimum amplitude of an individual
trace.  called only once for each trace in any one
invocation of the program.

*******************************************************************************/
	short	*sptr;
	gint32	*ptr;
	float	*lptr;
	double	*dptr;
	double 	*minPtr, *maxPtr, *meanPtr, *diffPtr, mean=0, val;
	int		format = dataInfo->head.data_form, samples = dataInfo->head.numSamples;
	int		i, lessSamples, gapSamps, gapNum;
    
	sptr = (short *) dataInfo->data;
	ptr = (gint32 *) dataInfo->data;
	lptr = (float *) dataInfo->data;
	dptr = (double *) dataInfo->data;
	minPtr = &dataInfo->amps.min;
	maxPtr = &dataInfo->amps.max;
	meanPtr = &dataInfo->amps.mean;
	diffPtr = &dataInfo->amps.diff;

	DATAP(format, 0, val);
	*minPtr = val;
  	*maxPtr = val;

#if 0
code for gaps/olaps managed as a list
	GSList	*glapIter;
	glaps	*glapPtr;

	glapIter = trc->trace.gaps;
	glapPtr = glapIter->data;
   	for (i=lessSamples=0; i<samples; ++i)
   	{
		if (glapPtr)
		{
			if (i==glapPtr->bounds[GLAPBEG])
			{
				gapSamps = (glapPtr->bounds[GLAPEND] - glapPtr->bounds[GLAPBEG]);
				i += gapSamps;
				lessSamples += gapSamps;
				glapIter = g_slist_next(glapIter);
				glapPtr = glapIter->data;
			}
		}
#endif
				
   	for (i=lessSamples=gapNum=0; i<samples; ++i)
	{
		if ((gapNum/2) < trc->trace.numGaps)
		{
			if (i==trc->trace.gaps[gapNum])
			{	// don't take into account the zero-filled gap data
				gapSamps = (trc->trace.gaps[gapNum+1] - trc->trace.gaps[gapNum]);
				i += gapSamps;
				lessSamples += gapSamps;
				gapNum += 2;
			}
		}
		if (i>=samples)		// no more data to check
			continue;

		DATAP(format, i, val);
  		mean += val;
  		if (val > *maxPtr) 
  		{
			*maxPtr = val;
			continue;
   		}
      
   		if (val < *minPtr)
			*minPtr = val;
   	}
	*meanPtr = mean/(samples-lessSamples);
	*diffPtr = *maxPtr - *minPtr;
}

void get_maxmin_amp(traceInfo *trc)
{
	get_maxmin(trc, &trc->data.dataInfo);
}

