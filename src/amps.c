#include "pql_defines.h"
#include "pql_externs.h"

void get_max_mins()
{

/*******************************************************************************

this procedure finds the maximum and minimum amplitudes for each trace as 
well as for all the traces which will be printed on the main screen.  it 
also determines the absolute start time of the traces being printed, as
well as the maximum time window required for plotting in relative time mode.

*******************************************************************************/

    double	    end_time1;
    float	    scale;
	traceInfo	*trcData;
	GSList		*trcIter;

    states.trace.absAmp.diff = 
    states.trace.absAmp.diffVolts = 0;

    states.trace.absTime.start = 1893456000.;		// epochs(2030-1-1), won't work after this date
    states.trace.absTime.end = 0;
    states.trace.relTimeLen = 0;

	for(trcIter = states.trace.traces[CURGRP];
		trcIter;
		trcIter = g_slist_next(trcIter))
	{
		trcData = trcIter->data;
		if (!trcData->data.dataInfo.data)
			continue;	// CWB might return nothing
			
	    if (!trcData->data.dataInfo.amps.max && 
	    	!trcData->data.dataInfo.amps.min)
		{	// if already processed, results are saved, don't do twice
	    	get_maxmin_amp(trcData);
			trcData->trace.RMS = RMS(0, 0, trcData);
		}
	
		if((trcData->data.dataInfo.amps.max - trcData->data.dataInfo.amps.min) > states.trace.absAmp.diff) 
		    states.trace.absAmp.diff = trcData->data.dataInfo.amps.max - trcData->data.dataInfo.amps.min;
	
		scale = trcData->data.head.scale_fac / trcData->data.head.gainConst;
	
		if((trcData->data.dataInfo.amps.max - trcData->data.dataInfo.amps.min) * scale 
				> states.trace.absAmp.diffVolts)
		    states.trace.absAmp.diffVolts = (trcData->data.dataInfo.amps.max - 
												trcData->data.dataInfo.amps.min) * scale;
	
		if (states.trace.absTime.start > trcData->data.head.epochs)
		{
		  states.trace.absTime.start = trcData->data.head.epochs;
		}
		
		end_time1 = trcData->data.head.epochs + (trcData->data.dataInfo.head.numSamples * 
					(float) (trcData->data.head.sampInt / 1000000.));
		
		if (states.trace.absTime.end < end_time1)
		  states.trace.absTime.end = end_time1;
		states.trace.absTime.diff = states.trace.absTime.end - states.trace.absTime.start;
	
		if (states.trace.relTimeLen < end_time1 - trcData->data.head.epochs)
		  states.trace.relTimeLen = end_time1 - trcData->data.head.epochs;
    }
}
