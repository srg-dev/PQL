#include <stdlib.h>
#include <string.h>
#include <math.h>
#ifndef MAC
#include <values.h>
#endif
#include "pql_defines.h"
#include "pql_externs.h"
#include "gtk_pql_externs.h"

void get_abs_mag_times()
{
	// compute the overall magnify window length 
	// called only once on MAGNIFY tab select
	// the result, states.magnify.absTime.diff is used by the magDrag() routine
	// attn: the other members of states.magnify.absTime, other than diff, are
	// re-used and reset elsewhere!
	double	traceEnd;
	GSList  *trcIter;
	traceInfo *trcData;

	memset(&states.magnify.absTime, 0, sizeof(states.magnify.absTime));
	states.magnify.absTime.start = 1893456000.;		// epochs(1-JAN-2030)
	switch(settings.trace.time)
	{
		case ABSOLUTE:
			for (trcIter = states.trace.selection;
				 trcIter;
				 trcIter = g_slist_next(trcIter))
			{
				trcData = trcIter->data;
				if (states.magnify.absTime.start > trcData->data.head.epochs)
					states.magnify.absTime.start = trcData->data.head.epochs;

				traceEnd = trcData->data.head.epochs + 
							(trcData->data.dataInfo.head.numSamples * 
								(float) (trcData->data.head.sampInt / 1000000.));
		
				if (states.magnify.absTime.end < traceEnd)
					states.magnify.absTime.end = traceEnd;
			}
			states.magnify.absTime.diff = states.magnify.absTime.end - states.magnify.absTime.start;
		break;

		case RELATIVE:
			for (trcIter = states.trace.selection;
				 trcIter;
				 trcIter = g_slist_next(trcIter))
			{
				trcData = trcIter->data;
				traceEnd = (trcData->data.dataInfo.head.numSamples * 
								(float) (trcData->data.head.sampInt / 1000000.));

				if (states.magnify.absTime.diff < traceEnd)
					states.magnify.absTime.diff = traceEnd;
			}
		break;
	}
}				
				
void get_mag_times()
{
  
  /**************************************************************************
    
    this procedure determines which part of the trace to magnify and sets the
    appropriate variables used later in plotting.
    
    ***********************************************************************/
  
	float	    length_of_trace;
	double      user_x1, user_x2;
	double      targ_diff;
  
	int         temp_int_2;
	double      temp_float, temp_float_2;
	GSList  	*trcIter;
	traceInfo 	*trcData;
  
  user_x1 = states.magnify.limits.selection.user.left;
  user_x2 = states.magnify.limits.selection.user.right;

  switch (settings.trace.time) 
  {
  	case ABSOLUTE:
		states.magnify.absTime.start = states.trace.absTime.end;
		for (trcIter = states.trace.selection;
			 trcIter;
			 trcIter = g_slist_next(trcIter))
		{
			trcData = trcIter->data;
		     length_of_trace = ((float) (trcData->data.dataInfo.head.numSamples) / 
							 (1000000. / trcData->data.head.sampInt));
		      
		     if (((user_x1 + states.trace.absTime.start) > (trcData->data.head.epochs +
		     													length_of_trace)) || 
					((user_x2 + states.trace.absTime.start) < trcData->data.head.epochs)) 
			 {
				trcData->magnify.begin = -1;	//  nothing to magnify
				continue;
		     }
		      
		     if ((user_x1 + states.trace.absTime.start) < trcData->data.head.epochs) 
			 {	//  left boundary is before trace 
				trcData->magnify.begin = 0;
				trcData->magnify.offset = (trcData->data.head.epochs - 
												(user_x1 + states.trace.absTime.start));
		     }
		     else 
			 {	//  left boundary is in middle of trace
				trcData->magnify.begin = user_x1 + (states.trace.absTime.start - 
														trcData->data.head.epochs);
				trcData->magnify.offset = 0;
				temp_int_2 = (ceil)(((1000000.0/trcData->data.head.sampInt) * trcData->magnify.begin));
				temp_float = trcData->data.head.epochs - states.trace.absTime.start;
				temp_float_2 = 1 / (1000000.0/trcData->data.head.sampInt);
				temp_float_2 *= (temp_int_2);
				temp_float += temp_float_2;
				temp_float -= user_x1;
				trcData->magnify.offset = temp_float;
		     }
		      
		    if ((user_x2 + states.trace.absTime.start) > 
		    	(trcData->data.head.epochs + length_of_trace))
			{	//  right boundary is after end of trace
			  trcData->magnify.end = length_of_trace;
			}
		    else
			{	//  right boundary is in middle of trace
				trcData->magnify.end = ((user_x2 + states.trace.absTime.start) 
										- trcData->data.head.epochs);
			}
			if (states.magnify.absTime.start > (user_x1 + states.trace.absTime.start))
			{
				states.magnify.absTime.start = user_x1 + states.trace.absTime.start;
			}
		}
    break;
    
  case RELATIVE:
	for (trcIter = states.trace.selection;
		 trcIter;
		 trcIter = g_slist_next(trcIter))
	{
	  trcData = trcIter->data;
      trcData->magnify.offset = 0;
      length_of_trace = (float) trcData->data.dataInfo.head.numSamples / 
			  (1000000. / trcData->data.head.sampInt);
      
      if (user_x1 > length_of_trace)	trcData->magnify.begin = -1;
      else								trcData->magnify.begin = user_x1;
      
      if (user_x2 > length_of_trace)	trcData->magnify.end = length_of_trace;
      else								trcData->magnify.end = user_x2;
      
      // since the exact time that corresponds with the limits lines
      // in the main window will probably not coincide exactly with a
      // data point of the magnified trace, we must calculate the
      // offset of the trace in the magnify window.  The assumption is
      // that the begin time calculated is between the last point not
      // to be plotted and the first point to be plotted.  Therefore
      // we should calculate the faction of the spacing that the first
      // point must be moved in order for the point to be correctly
      // plotted on the screen

      trcData->magnify.offset = (ceil(user_x1)) - user_x1;
      targ_diff = (trcData->data.head.sampInt) / 1000000.0;
      for (;trcData->magnify.offset > targ_diff; trcData->magnify.offset -= targ_diff);
    } 	// end for
  break;
  } 	// end switch 
}

double RMS(int startPt, int endPt, traceInfo *trcData)
{
	int		dataFormat = trcData->data.head.data_form;
	int		ptInc, nPts;
	double	av=0, sum=0, val;
    gint32  *ptr;
    short   *sptr;
    float	*lptr;
    double	*dptr, dataV;
	DATAINFO *dataInfo;

#if 0
	if (states.filter.on)
	{
		dataInfo = g_slist_nth_data(trcData->data.dataFilt,states.filter.active);
		lptr = (float *) dataInfo->data.;
		dataFormat = MY_FLOAT;
	}
#endif
	if (states.tForm.on && trcData->data.tForm.data)
	{
		dataInfo = &trcData->data.tForm;
		lptr = (float *) dataInfo->data;
		dataFormat = MY_FLOAT;
	}
	else
	{
		dataInfo = &trcData->data.dataInfo;
		switch(dataFormat)
		{
			case MY_FLOAT:
				lptr = (float *) dataInfo->data;
			break;
			case BIT32:
				ptr = (gint32 *) dataInfo->data;
			break;
			case BIT16:
				sptr = (short *) dataInfo->data;
			break;
			case MY_DOUBLE:
				dptr = (double *) dataInfo->data;
			break;
		}
	}
	
	if (startPt == 0 &&
		endPt == 0)
	{
		endPt = dataInfo->head.numSamples - 1;
	}
	nPts = endPt - startPt + 1;
	
	for (ptInc = startPt; ptInc <= endPt; ++ptInc)
	{
		DATAP(dataFormat, ptInc, dataV);
		av += dataV;
	}
	av /= (float) nPts;

	for (ptInc = startPt; ptInc <= endPt; ++ptInc)
	{
		DATAP(dataFormat, ptInc, dataV);
		val = dataV - av;
		sum += val * val;
	}
	val = sqrt(sum) / sqrt((double) nPts - 1);

	return(val);
}

void get_mag_max_mins(gboolean init)
{

/*******************************************************************************

this procedure finds the max and min amplitudes for the traces which have
been selected for magnification.  it finds the max and min of only the section
of the traces which has been selected.  this procedure is called each time
the magnify screen is required to be refreshed.

*******************************************************************************/
    int         i, dataFormat;
	float		pct;
	char		maxS[10], minS[10];
    float	    scale;
    gint32      *ptr=NULL;
    short       *sptr=NULL;
    float		*lptr=NULL;
    double		sps, *dptr=NULL, dataV=0;
    int         temp_i, ptInc, startPoint, endPoint;
	static int	*pts[2], lastActiveFilter=-1;
	gboolean	zoomOUT;
	DATAINFO	*dataInfo=NULL;
	GSList		*trcIter;
	traceInfo	*trcData;

    states.magnify.absAmp.diff = 0;
    states.magnify.absAmp.diffVolts = 0;
    states.magnify.absAmp.min = 100000000.;
    states.magnify.absAmp.max = states.magnify.absAmp.min * -1;

	if (init || (lastActiveFilter != states.tForm.filter))
	{
		if (pts[0])
		{
			free(pts[0]);
			free(pts[1]);
		}
		pts[0] = calloc(states.trace.numSelected, sizeof (int*));
		pts[1] = calloc(states.trace.numSelected, sizeof (int*));
	}
	lastActiveFilter = states.tForm.filter;

	for (trcIter = states.trace.selection, i=0;
		 trcIter;
		 trcIter = g_slist_next(trcIter), i++)
	{
		trcData = trcIter->data;
		sps = 1000000. / (double) trcData->data.head.sampInt;
		startPoint = (int) ((ceil)(trcData->magnify.begin * sps));		// beginning point
		endPoint = (int) (trcData->magnify.end * sps); 					// ending point
		zoomOUT = FALSE;
		if (!init &&
			(startPoint > 0) && 
			(pts[START][i] >= startPoint && pts[END][i] <= endPoint) &&
			(trcData->magnify.amps.min != G_MAXFLOAT) &&
			(trcData->magnify.amps.max != -1*G_MAXFLOAT))
		{
			zoomOUT = TRUE;
		}

#if 0
		if (states.filter.on)
		{
			dataInfo = g_slist_nth_data(trcData->data.dataFilt,states.filter.active);
			lptr = (float *) dataInfo->data;
			dataFormat = MY_FLOAT;
		}
#endif
		if (states.tForm.on && trcData->data.tForm.data)
		{
			dataInfo = &trcData->data.tForm;
			dataFormat = MY_FLOAT;
		}
		else
		{
			dataInfo = &trcData->data.dataInfo;
			dataFormat = trcData->data.head.data_form;
		}
		switch(dataFormat)
		{
			case MY_FLOAT:
				lptr = (float *) dataInfo->data;
			break;
			case BIT32:
				ptr = (gint32 *) dataInfo->data;
			break;
			case BIT16:
				sptr = (short *) dataInfo->data;
			break;
			case MY_DOUBLE:
				dptr = (double *) dataInfo->data;
			break;
		}

		sps = 1000000. / (double) trcData->data.head.sampInt;
		if (!zoomOUT)
		{
			trcData->magnify.amps.min = G_MAXFLOAT;
			trcData->magnify.amps.max = -1*(trcData->magnify.amps.min);
		}

		if (trcData->magnify.begin == -1) 
		{
			trcData->magnify.amps.max = G_MAXFLOAT;
			trcData->magnify.amps.min = -1*(trcData->magnify.amps.max);
			pts[START][i] = 0;
			pts[END][i] = 0;
	  		continue;             /*  nothing to magnify  */
		}
	
		trcData->magnify.RMS = RMS(startPoint, endPoint, trcData);
		
		switch(settings.magnify.Tamp)
		{	// using max amplitude of only visible portion of trace
			case DISPS:
			case FIXEDS:
				if (!zoomOUT)
				{
					for (ptInc = startPoint; ptInc < endPoint; ++ptInc)
		  			{
						DATAP(dataFormat, ptInc, dataV);
						if (trcData->trace.numGaps)
						{	// if we have gaps and a zero value, assume it's gap data and don't consider it
							if (dataV == 0.)
								continue;	
						}
		    			if (dataV > trcData->magnify.amps.max)
		      			{
							trcData->magnify.amps.max = dataV;
							if ( trcData->magnify.amps.min < trcData->magnify.amps.max)
			  				// to allow for case where fist data point is the minimum
			  					continue;
		      			}
		    			if (dataV < trcData->magnify.amps.min)
		      				trcData->magnify.amps.min = dataV;
					}
					pts[START][i] = startPoint;
					pts[END][i] = endPoint;
				}
				else
				{	// when zooming out, don't recompute max/min for values previously considered
					for (ptInc = startPoint; ptInc < pts[START][i]; ++ptInc)
		  			{
						DATAP(dataFormat, ptInc, dataV);
						if (trcData->trace.numGaps)
						{	// if we have gaps and a zero value, assume it's gap data and don't consider it
							if (dataV == 0.)
								continue;		
						}
		    			if (dataV > trcData->magnify.amps.max)
		      			{
							trcData->magnify.amps.max = dataV;
							if ( trcData->magnify.amps.min < trcData->magnify.amps.max)
			  				// to allow for case where fist data point is the minimum
			  					continue;
		      			}
		    			if (dataV < trcData->magnify.amps.min)
		      				trcData->magnify.amps.min = dataV;
					}
					for (ptInc=endPoint; ptInc > pts[END][i]; --ptInc)
		  			{
						DATAP(dataFormat, ptInc, dataV);
						if (trcData->trace.numGaps)
						{	// if we have gaps and a zero value, assume it's gap data and don't consider it
							if (dataV == 0.)
								continue;		
						}
		    			if (dataV > trcData->magnify.amps.max)
		      			{
							trcData->magnify.amps.max = dataV;
							if ( trcData->magnify.amps.min < trcData->magnify.amps.max)
			  				// to allow for case where fist data point is the minimum
			  					continue;
		      			}
		    			if (dataV < trcData->magnify.amps.min)
		      				trcData->magnify.amps.min = dataV;
					}
					pts[START][i] = startPoint;
					pts[END][i] = endPoint;
				}
			break;

			case WINDS:
				trcData->magnify.amps.max = dataInfo->amps.max;
				trcData->magnify.amps.min = dataInfo->amps.min;
			break;
		}	// end switch

		// the following is to account for the case where the trace is a flat line
		if ((trcData->magnify.amps.max) == (trcData->magnify.amps.min)) 
		{
			trcData->magnify.amps.max += 1.;
			trcData->magnify.amps.min -= 1.;
		}
		  
		// let's try to account for the case where there are no data points on display
		if ((trcData->magnify.amps.max == (-1 * G_MAXFLOAT)) && 
			(trcData->magnify.amps.min == (G_MAXFLOAT))) 
		{
		  trcData->magnify.amps.min = -1.0;
		  trcData->magnify.amps.max = 1.0;
		}
	
		switch(settings.magnify.Tamp)
		{
			case DISPS:
				if ((trcData->magnify.amps.max - trcData->magnify.amps.min) > states.magnify.absAmp.diff)
		    		states.magnify.absAmp.diff = trcData->magnify.amps.max - trcData->magnify.amps.min;
			break;
	
			case WINDS:
				if ((dataInfo->amps.max - dataInfo->amps.min) > states.magnify.absAmp.diff)
	    				states.magnify.absAmp.diff = dataInfo->amps.max - dataInfo->amps.min;
			break;
	
			case FIXEDS:
			{
				double mxmn;
				switch(states.currentScreen)
				{
					case MAIN:
						strcpy(maxS, gtk_entry_get_text(GTK_ENTRY(MTfixed[MAX])));
						strcpy(minS, gtk_entry_get_text(GTK_ENTRY(MTfixed[MIN])));
					break;
					case SPLIT2:
					case SPLIT3:
						strcpy(maxS, gtk_entry_get_text(GTK_ENTRY(HTfixed[MAX])));
						strcpy(minS, gtk_entry_get_text(GTK_ENTRY(HTfixed[MIN])));
					break;
				}
				if (getUserMaxMin(maxS, &mxmn))
				{
					pct = mxmn;
					trcData->magnify.amps.max = trcData->magnify.amps.max - 
							(float) ((trcData->magnify.amps.max - trcData->magnify.amps.min)/2) * 
								(float) (pct/100.);
				}
				else
				{	// user-specified MAX
					if (strlen(maxS))
						trcData->magnify.amps.max = mxmn;
				}
				if (getUserMaxMin(minS, &mxmn))
				{
					pct = mxmn;
					trcData->magnify.amps.min = trcData->magnify.amps.min + 
							(float) ((trcData->magnify.amps.max - trcData->magnify.amps.min)/2) * 
								(float) (pct/100.);
				}
				else
				{	// user-specified MAX
					if (strlen(minS))
						trcData->magnify.amps.min = mxmn;
				}
	
				if (trcData->magnify.amps.max < trcData->magnify.amps.min)
				{	// reverse them if they're backwards 
					int tmp;
					tmp = trcData->magnify.amps.max;
					trcData->magnify.amps.max = trcData->magnify.amps.min;
					trcData->magnify.amps.min = tmp;
				}
				if ((trcData->magnify.amps.max - trcData->magnify.amps.min) > states.magnify.absAmp.diff)
		    		states.magnify.absAmp.diff = trcData->magnify.amps.max - trcData->magnify.amps.min;
		    }
			break;
		}
	
		scale = trcData->data.head.scale_fac / trcData->data.head.gainConst;
	
		if ((trcData->magnify.amps.max - trcData->magnify.amps.min) * scale > states.magnify.absAmp.diffVolts)
		    states.magnify.absAmp.diffVolts = (trcData->magnify.amps.max - trcData->magnify.amps.min) * scale;
	
		if(trcData->data.head.units == PQL_UNITS_COUNTS) 
		{
		  if ((trcData->magnify.amps.max - trcData->magnify.amps.min) >= 2.0)
		  {
		      temp_i = (trcData->magnify.amps.max - trcData->magnify.amps.min)/2;
		      if (temp_i*2 != (trcData->magnify.amps.max - trcData->magnify.amps.min)) 
			  { // this is to eliminate the 0.5 display variable when printing the y axis 
				  trcData->magnify.amps.max += 1; 	
		      }
		    }
		}
	
		if (trcData->magnify.amps.max > states.magnify.absAmp.max)
			states.magnify.absAmp.max = trcData->magnify.amps.max;
		if (trcData->magnify.amps.min < states.magnify.absAmp.min)
			states.magnify.absAmp.min = trcData->magnify.amps.min;
    }				// end for
}				// end get_mag_max_mins()

static int             max_jump = 0;
static int             min_jump = 0;
static double          mag_mouse_time = 0.0;
static double          mag_mouse_amp = 0.0;
static int             mag_x, mag_y;

static void mag_point(traceInfo *trcData, double begin, double end, int inc, int m_x)
{
	double         tm, tm_inc;
	int            ndata, dataLoc, dataFormat;
	int            y1, y2, pix_x0, pix_x1, pix_y0, pix_y1, i;
	double         fmax, fmin;
	gint32         *ptr=NULL;
	short          *sptr=NULL;
	float	         *lptr=NULL;
	double		 *dptr=NULL;
	float	         plot_length;
	double         l_trace;
	double         old_time;
	double         old_amp, val=0;
	graph			 *gr;
	char			 *udata;
	
	max_jump = 0;
	min_jump = 0;
	mag_mouse_time = 0.0;
	mag_mouse_amp = 0.0;
	
	gr = &trcData->magnify.GR[states.currentScreen];
	udata = trcData->magnify.data_start;
	tm = begin + trcData->magnify.offset;
	tm_inc = (double) (((trcData->data.head.sampInt) / 1000000.) * ((float) inc));
	plot_length = trcData->magnify.end - trcData->magnify.begin;
	l_trace = ((float) trcData->data.dataInfo.head.numSamples / (1000000. / trcData->data.head.sampInt));
	ndata = (int) ((1000000. / trcData->data.head.sampInt) * plot_length);
	
	// if end of trace don't print last data point
	if(((ceil)(trcData->magnify.end)) == ((ceil)(l_trace)))
	ndata -= inc;
	
	dataLoc = 0;
	if (!(states.tForm.on && trcData->data.tForm.data))
	{
		dataFormat = trcData->data.head.data_form;
		switch (dataFormat)
		{
			case BIT32:
				ptr = (gint32 *) udata;
			break;
			case BIT16:
				sptr = (short *) udata;
			break;
			case MY_FLOAT:
				lptr = (float *) udata;
			break;
			case MY_DOUBLE:
				dptr = (double *) udata;
			break;
		}
	}
	else
	{
		lptr = (float *) udata;
		dataFormat = MY_FLOAT;
	}

	DATAP(dataFormat, dataLoc, val);
	/* point to start and end points */
	pix_x0 = USER2PIXX(gr, tm);
	pix_y0 = USER2PIXY(gr, val);
	
	old_time = tm;
	old_amp = val;
	
	tm += tm_inc;
	dataLoc += inc;
  	DATAP(dataFormat, dataLoc, val);
	pix_x1 = USER2PIXX(gr, tm);
	pix_y1 = USER2PIXY(gr, val);
	
	if ( m_x <= pix_x0) 
	{
	  mag_x = pix_x0;
	  mag_y = pix_y0;
	  // now set global time and amp to real values 
	  mag_mouse_time = old_time;
	  mag_mouse_amp = old_amp;
	  return;
	}	
	
	// if pointing to spot before first data point 
	if ( m_x <= pix_x1) 
	{
	  if ((m_x - pix_x0) < (pix_x1 - m_x)) 
	  {
	    mag_x = pix_x0;
	    mag_y = pix_y0;
	    // now set global time and amp to real values
	    mag_mouse_time = old_time;
	    mag_mouse_amp = old_amp;
	  }
	  else 
	  {
	    mag_x = pix_x1;
	    mag_y = pix_y1;
	    // now set global time and amp to real values 
      	    mag_mouse_time = tm;
	    mag_mouse_amp = val;
	    if ( pix_x1 > states.magnify.limits.margin[states.currentScreen].right) 
		{ // if next data point is out of bounds
	      mag_x = pix_x0;
	      mag_y = pix_y0;
	      // now set global time and amp to real values
	      mag_mouse_time = old_time;
	      mag_mouse_amp = old_amp;
	    }
	  }
	  return;
	}
	
	pix_x0 = pix_x1;
	pix_y0 = pix_y1;
	ndata -= inc;

	old_time = tm;
	old_amp = val;

	// check to see if we are even going to enter the for loop 
	if (ndata < inc) 
	{ // now set global time and amp to real values
	  mag_mouse_time = tm;
	  mag_mouse_amp = val;
	}
	
	// look at the rest of the data points 
	for (; ndata >= inc; ndata -= inc, pix_x0 = pix_x1, pix_y0 = pix_y1) 
	{
	  dataLoc += inc;
  	  DATAP(dataFormat, dataLoc, val);
	  tm += tm_inc;
	  
	  pix_x1 = USER2PIXX(gr, tm);
	  pix_y1 = USER2PIXY(gr, val);
	  
	  if (( m_x > pix_x0) && ( m_x <= pix_x1)) 
	  {
	    // if data is between prev and next point
	    if ((m_x - pix_x0) < (pix_x1 - m_x)) 
		{
	      mag_x = pix_x0;
	      mag_y = pix_y0;
	      mag_mouse_time = old_time;
	      mag_mouse_amp = old_amp;
	    }
	    else 
		{
	      mag_x = pix_x1;
	      mag_y = pix_y1;
	      mag_mouse_time = tm;
	      mag_mouse_amp = val;
	    }
	    
	    if (inc > 1) 
		{
	      fmax = fmin = val;
	      for (i=1; (i < inc) && (dataLoc < ndata); i++) 
		  {
  			DATAP(dataFormat, dataLoc+i, val);
			if (val < fmin)
		  		fmin = val;
			if (val > fmax)
		  		fmax = val;
	      }
	      y1 = USER2PIXY(gr, (double) fmin);
	      y2 = USER2PIXY(gr, (double) fmax);
	      
	      mag_y = (y1 + y2)/2;
	      max_jump = y2;
	      min_jump = y1;
	      
	      mag_mouse_time = 0.0;
	      mag_mouse_amp = 0.0;
	      mag_mouse_time = old_time;
	      mag_mouse_amp = old_amp;
	    }
	    
	    if ( pix_x1 > states.magnify.limits.margin[states.currentScreen].right) 
		{ // make sure this data point is in bounds
	      mag_x = pix_x0;
	      mag_y = pix_y0;
	      mag_mouse_time = old_time;
	      mag_mouse_amp = old_amp;
	    }
	    return;		// we're done
	  }	
	  
	  if (inc > 1) 
	  {
	      fmax = fmin = val;
	      for (i=1; (i < inc) && (dataLoc < ndata); i++) 
		  {
  			DATAP(dataFormat, dataLoc+i, val);
			if (val < fmin)
		  		fmin = val;
			if (val > fmax)
		  		fmax = val;
	      }
	    y1 = USER2PIXY(gr, (double) fmin);
	    y2 = USER2PIXY(gr, (double) fmax);
	  }			
	  old_time = tm;
	  old_amp = val;
	
	// just in case mouse is past last data point 
	  mag_mouse_time = old_time;
	  mag_mouse_amp = old_amp;
    }

  mag_x = pix_x1;
  mag_y = pix_y1;
  
  return;
}

void magPoint(int mag_rec_no, double begin, double end, int inc, int m_x, pointID *ptr)
{

	float	scale;
  	graph	*gr;
  	traceInfo *trcData = g_slist_nth_data(states.trace.selection, mag_rec_no);

	mag_point(trcData, begin, end, inc, m_x);

	if (settings.trace.time == RELATIVE)
	{
		ptr->epoch = (double) (trcData->data.head.epochs + mag_mouse_time);
	}
	else
	{
		ptr->epoch = (double) (states.trace.absTime.start + mag_mouse_time);
	}
	passcal_etoh(&ptr->timeVal, ptr->epoch);
	if (min_jump && max_jump)
	{		// amplitude as range?
		gr = &trcData->magnify.GR[states.currentScreen];
		ptr->minAmp = PIX2USERY(gr, min_jump);
		ptr->maxAmp = PIX2USERY(gr, max_jump);
	}
	else
	{		// amplitude as single point?
		ptr->minAmp = ptr->maxAmp = mag_mouse_amp;
	}

	if (trcData->data.fileType == SEGY_DATA &&
		trcData->data.head.units == PQL_UNITS_VOLTS)
	{	// convert to VOLTS if necessary
		scale = trcData->data.head.scale_fac / (float) trcData->data.head.gainConst;
		ptr->minAmp *= scale;
		ptr->maxAmp *= scale;
	}

	ptr->xVal = mag_x;
	ptr->yVal = mag_y;
}
