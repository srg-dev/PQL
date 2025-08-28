#include <string.h>
#include <stdlib.h>
#include "pql_defines.h"
#include "pql_externs.h"
#include "gtk_pql_externs.h"

double			min_freq, max_freq;
static double	min_amp, max_amp;

static void sp_plot();
#define YAXISPOS	28

void trans_plot_files(GtkWidget *da, GdkPixmap *pixmap, int wind_width, int wind_height, int win_record)
{
	int      	i, str_width, fontH;
	static int	labelW;
	double    	pow(), min, max;
	float	    scale,
				gainFactor= (settings.general.display.mode == GATHERMODE &&
							  settings.general.display.gather.gain[SPECTRASCR] > 0)
								? settings.general.display.gather.gain[SPECTRASCR]
								: 1.0;
	graph	    trans_tmp;
	char      axis_label[80];
	int       axis_label_y_pos;
	int		label_width;
	GSList	*trcIter;
	traceInfo	*trcData;
  
	settings.general.font.DA = da;
	settings.general.font.font = LABELF;
	states.spectra.trace_depth[states.currentScreen] = 
			(float) (wind_height - (MAG_T_MARGIN + MAG_B_MARGIN)) /
		  			(float) states.trace.numSelected;

	// find the widest label we need to accommodate, but do it only once per configure/startup event
	if (!states.spectra.plot.label_width[states.currentScreen])
	{
		for (trcIter = states.trace.selection, label_width=0;
			 trcIter;
			 trcIter = g_slist_next(trcIter))
		{
			trcData = trcIter->data;
	  		str_width = stringWidth(trcData, states.spectra.trace_depth[states.currentScreen], LABEL);
			if (str_width > label_width)
				label_width = str_width;
		}
		states.spectra.plot.label_width[states.currentScreen] = settings.general.label.length ? 
																settings.general.label.length : 
																label_width;
	}
	label_width = states.spectra.plot.label_width[states.currentScreen];
	fontH = fontSIZE("9", HEIGHT);

	for (trcIter = states.trace.selection, i=0;
		 trcIter;
		 trcIter = g_slist_next(trcIter), i++)
	{
		if (win_record != i && win_record != -1)
			continue;		// plotting a single record?

		trcData = trcIter->data;

	    if (trcData->spectra.beg == -1 ) 
	      continue;
    
    	if (settings.spectra.display.amp == WSCALETRC) 
		{
      		max_amp = settings.spectra.display.yMax ? settings.spectra.display.yMax : trcData->spectra.ampMax;
      		min_amp = max_amp / pow((double) 10., (double) settings.spectra.display.decades);
    	}
    	else 
		{  // scale by window
      		max_amp = settings.spectra.display.yMax ? settings.spectra.display.yMax : states.spectra.plot.glob[SPEC_MAXAMP];
      		states.spectra.plot.glob[SPEC_MINAMP] = (float) ((double) max_amp) /
					  						pow((double) 10., (double) settings.spectra.display.decades);
      		min_amp = states.spectra.plot.glob[SPEC_MINAMP];
    	}

    	max_freq = settings.spectra.display.xMax ? settings.spectra.display.xMax : states.spectra.plot.glob[SPEC_MAXFREQ];
    	states.spectra.plot.glob[SPEC_MINFREQ] = 
    	min_freq = settings.spectra.display.xMin 
					? settings.spectra.display.xMin 
					: 1. / states.spectra.limits.selection.user.length;
    
		if (settings.spectra.display.overlay) 
		{
			viewport(&trcData->spectra.GR[states.currentScreen], label_width+YAXISPOS, wind_width-10,
						MAG_T_MARGIN, wind_height - MAG_B_MARGIN);
		}
		else 
		{	
			viewport(&trcData->spectra.GR[states.currentScreen], label_width+YAXISPOS, wind_width-10,
						i * states.spectra.trace_depth[states.currentScreen] + MAG_T_MARGIN + 1, 
						(i+1) * states.spectra.trace_depth[states.currentScreen] + MAG_T_MARGIN - 1);
		}
      
		max = max_amp;
		min = min_amp;
		if (gainFactor != 1.0)
		{
			double mid, diffH;
			diffH = (max - min)/2.0;
			mid = max - diffH;
			max = mid + diffH/(gainFactor);
			min = mid - diffH/(gainFactor);
		}

		window(&trcData->spectra.GR[states.currentScreen], 
						min_freq, max_freq, 
						min, max, 
						settings.spectra.display.xAxis, settings.spectra.display.yAxis);
						
    	sp_plot(pixmap, &trcData->spectra.GR[states.currentScreen], 
					trcData->spectra.numPts / 2, 
					trcData->spectra.data, i);

		// print the trace labels
		switch(settings.general.display.mode)
		{
			case TRACEMODE:
				printLabel(pixmap, trcData, i, states.spectra.trace_depth[states.currentScreen], 0, SPECTRASCR);
			break;
			case GATHERMODE:
			{	// only print labels when they don't overlap in GATHERMODE
				static int last = 0;
				int	middle = (states.trace.trace_depth[states.currentScreen]*i) +
								states.trace.trace_depth[states.currentScreen]/2;
				if (i==0)
					last = 0;	// new plot - start over
				if (middle - fontH > last)
				{
					printLabel(pixmap, trcData, i, 
								states.spectra.trace_depth[states.currentScreen], 0, SPECTRASCR);
					last = middle + fontH/2;
				}
			}
			break;
		}

		// print the y-axis
		if (!settings.spectra.display.overlay &&
			settings.general.display.mode == TRACEMODE) 
		{	//  draw y-axis - only for individual traces, and in TRACEMODE
			if (trcData->data.head.units == PQL_UNITS_VOLTS && 
				trcData->data.fileType == SEGY_DATA) 
			{	// scale the amplitudes
  				scale = trcData->data.head.scale_fac / (float) trcData->data.head.gainConst;
				min_amp *= scale;
				max_amp *= scale;
			}
			    	  
  			viewport(&trans_tmp, label_width, label_width+YAXISPOS, 
						i * states.spectra.trace_depth[states.currentScreen] + MAG_T_MARGIN + 1, 
						(i + 1) * states.spectra.trace_depth[states.currentScreen] + MAG_T_MARGIN-1);
			window(&trans_tmp, 0, YAXISPOS, min_amp, max_amp, 0, settings.spectra.display.yAxis);
			yaxis(pixmap, GXC[NORMALGC][SPECTRASCR], &trans_tmp, 
							(double) 15, (double) min_amp, (double) max_amp, 
							1, -1, 0, (double) 0.0, MYFLOAT);
		}
	}		// end for

	if (settings.spectra.display.overlay && 
		settings.spectra.display.amp != WSCALETRC)
	{	// print yaxis - overlay mode
		viewport(&trans_tmp, label_width, label_width+YAXISPOS, 
					MAG_T_MARGIN+1, wind_height - MAG_B_MARGIN-1); 
		window(&trans_tmp, 0, YAXISPOS, 
						min_amp, max_amp, 0, settings.spectra.display.yAxis);
		yaxis(pixmap, GXC[NORMALGC][SPECTRASCR],&trans_tmp,
					(double) 15, (double) min_amp, (double) max_amp,
					1, -1, 0, (double) 0.0, MYFLOAT);
	}

	if ( min_freq == max_freq) 
	{
	    fprintf(stdout,"Spectra Problem: min_freq == max_freq, cannot process\n");
	    return;
	}

	// print top xAxis in seconds (period), only if log scale
	if (settings.spectra.display.xAxis == LOG)
	{	
		sprintf(axis_label,"Period (Sec)");
		labelW = fontSIZE(axis_label, WIDTH);
		axis_label_y_pos = 5;
		fontDraw(pixmap, axis_label, ((label_width-labelW)<0)?0:(label_width-labelW), 
					axis_label_y_pos, NORMALGC, SPECTRASCR, -1, -1);

		viewport(&states.spectra.GR[states.currentScreen], 
					wind_width-10, label_width+YAXISPOS, 
					0, MAG_T_MARGIN);
		window(&states.spectra.GR[states.currentScreen], 
				  1./max_freq, 1./min_freq, 
				  (double) 0, (double) MAG_T_MARGIN, 
				  settings.spectra.display.xAxis, 0);
  
		xaxis(pixmap, GXC[NORMALGC][SPECTRASCR], 
				  &states.spectra.GR[states.currentScreen], 
				  1./max_freq, (double) 10.0, 
				  1./min_freq, -1, 1, -1, (double) 0.0, TRUE, (double) 0.0);
	}

	// print bottom xAxis in hertz (frequency)
	sprintf(axis_label,"Frequency (Hz)");
	axis_label_y_pos = ((int) wind_height - 20);
	settings.general.font.font = AXISF;
	labelW = fontSIZE(axis_label, WIDTH);
	fontDraw(pixmap, axis_label, ((label_width-labelW)<0)?0:(label_width-labelW), 
					axis_label_y_pos, NORMALGC, SPECTRASCR, -1, -1);

	viewport(&states.spectra.GR[states.currentScreen], label_width+YAXISPOS, wind_width-10,
				  wind_height - MAG_B_MARGIN, wind_height);
	window(&states.spectra.GR[states.currentScreen], 
				  min_freq, max_freq, 
				  (double) 0, (double) MAG_B_MARGIN, 
				  settings.spectra.display.xAxis, 0);
  
	xaxis(pixmap, GXC[NORMALGC][SPECTRASCR], 
				  &states.spectra.GR[states.currentScreen], 
				  min_freq, (double) 19, max_freq, 1, -1, -1, (double) 0.0, FALSE, (double) 0.0);

	// set the pixel coords of the plot margins - needed elsewhere
	states.spectra.limits.margin[states.currentScreen].left = label_width;
	states.spectra.limits.margin[states.currentScreen].right = wind_width - 10;
	return;
}

static void sp_plot(GdkPixmap *pixmap, graph *gr, int npts, char *udata, int trans_rec)
{
  double          x, xmin, ymin, ymax, xmax;
  float          *ptr, delta;
  int             skip, n_data;
  int             pix_x0, pix_x1, pix_y0, pix_y1;
  double          exp();
  traceInfo		 *trcData = g_slist_nth_data(states.trace.selection, trans_rec);
  GdkGC	*localGC;
  
  if (settings.spectra.display.overlay)
  {
	int gcidx = trans_rec % 30;
	localGC = OGXC[gcidx];
  }
  else
  {
	localGC = GXC[NORMALGC][SPECTRASCR];
  }
  
//  if (trcData->spectra.beg == -1)
//    return;

  delta = (1./(2. * trcData->data.head.sampInt/1000000.)) / npts;
  xmin = min_freq;
  xmax = max_freq;
  ymin = min_amp;
  ymax = max_amp;
  
  skip = (int) (xmin * 2 * npts * (trcData->data.head.sampInt / 1000000.));
  if (skip < 0)
      skip = 0;
  n_data = npts - skip;
  if (n_data < 2)
    return;	// nothing left to plot

  ptr = (float *) udata;
  ptr = (float *) (ptr + (skip * 2));
  /*	else		not interested in phase right now
	ptr= (float *) (ptr+(skip*2)+1);  */

  x = xmin;
  pix_x0 = USER2PIXX(gr, x);
  
  if ((double) *ptr < ymin)
      pix_y0 = USER2PIXY(gr, ymin);
  else if ((double) *ptr > ymax)
      pix_y0 = USER2PIXY(gr, ymax);
  else
	  pix_y0 = USER2PIXY(gr, (double) *ptr);
  
   // first line segment to be drawn is a special case, since the x
   // increment is not delta
   
  ptr = (float *) (ptr + 2);
  x += (double) delta;
  pix_x1 = USER2PIXX(gr, x);
  if ( /* (spec_plot_type == AMP) && */ ((double) *ptr < ymin))
    pix_y1 = USER2PIXY(gr, ymin);
  else if ((double) *ptr > ymax)
      pix_y1 = USER2PIXY(gr, ymax);
  else
    pix_y1 = USER2PIXY(gr, (double) *ptr);

  gdk_draw_line(pixmap, localGC, pix_x0, pix_y0, pix_x1, pix_y1);
  pix_x0 = pix_x1;
  pix_y0 = pix_y1;
  n_data--;
  
  for (; n_data > 1; n_data--, pix_x0 = pix_x1, pix_y0 = pix_y1) 
  {
    ptr = (float *) (ptr + 2);
    x += (double) delta;
	if (x > xmax)	// have we breached the absolute bound?
		break;

    pix_x1 = USER2PIXX(gr, x);
    if (((double) *ptr < ymin))
      pix_y1 = USER2PIXY(gr, ymin);
  else if ((double) *ptr > ymax)
      pix_y1 = USER2PIXY(gr, ymax);
    else
      pix_y1 = USER2PIXY(gr, (double) *ptr);

    gdk_draw_line(pixmap, localGC, pix_x0, pix_y0, pix_x1, pix_y1);
  }
}

int transform_proc(GtkWidget *da, GdkPixmap *pixmap, int wind_width, int wind_height, int restart)
{

/*******************************************************************************

this procedure sets up the variables necessary for plotting the spectra
of the selected traces on the main screen.

*******************************************************************************/

  if (states.magnify.limits.selection.user.left || 
  	  states.magnify.limits.selection.user.right)
  {
  	states.spectra.limits.selection.user.left = states.magnify.limits.selection.user.left;
  	states.spectra.limits.selection.user.right = states.magnify.limits.selection.user.right;
  }
  else
  {
  	states.spectra.limits.selection.user.left = 
  		PIX2USERX(&states.trace.GR[MAIN], 
  					states.trace.limits.selection.pix[states.currentScreen].left);
    	if (states.trace.limits.selection.pix[states.currentScreen].right == 0) 
		{
        	states.spectra.limits.selection.user.right = 
							states.spectra.limits.selection.user.left + 
							settings.magnify.length;
			if (states.spectra.limits.selection.user.right > 
					states.trace.plot.length[settings.trace.time])
	    		states.spectra.limits.selection.user.right = 
	    			states.trace.plot.length[settings.trace.time];

			states.trace.limits.selection.pix[states.currentScreen].right = 
							USER2PIXX(&states.trace.GR[MAIN], 
										states.spectra.limits.selection.user.right);
    	}
    	else
    	{
			states.spectra.limits.selection.user.right = 
						PIX2USERX(&states.trace.GR[MAIN], 
									states.trace.limits.selection.pix[states.currentScreen].right);
		}
  }
  states.spectra.limits.selection.user.length = (float) (states.spectra.limits.selection.user.right - 
				  									states.spectra.limits.selection.user.left);
  
  if (restart)
  {
  	gboolean first;
	GSList *trcIter;
	traceInfo *trcData;
		for(trcIter = states.trace.traces[CURGRP];
			trcIter;
			trcIter = g_slist_next(trcIter))
	  {	// go through whole list
	  	trcData = trcIter->data;
		if (trcData && 
			trcData->spectra.data)
		{	// free up previous computation
			free(trcData->spectra.data);
			trcData->spectra.data = NULL;
		}
	  }   	// end for 
	  
	for (trcIter = states.trace.selection, first=TRUE;
		 trcIter;
		 trcIter = g_slist_next(trcIter), first=FALSE)
	{
	  trcData = trcIter->data;
	  get_trans_times(trcData, states.spectra.limits.selection.user.left,
	  				  states.spectra.limits.selection.user.right, settings.trace.time,
	  				  states.trace.absTime.start);
	  get_trans_data(trcData);
	  taper(trcData);
	  transform_data(trcData, settings.spectra.display.xMin ? 
							settings.spectra.display.xMin : 
							1. / states.spectra.limits.selection.user.length, 
							&settings.spectra.display);
	  setSpecGlobals(states.spectra.plot.glob, first, trcData, &settings.spectra.display);
	}
  }		//  end if
  
  trans_plot_files(da, pixmap, wind_width, wind_height, -1);
  return PQLXSUCCESS;
}
