#include <string.h>
#include <math.h>
#include <stdlib.h>
#include "pql_defines.h"
#include "pql_externs.h"
#include "gtk_pql_externs.h"

static void mag_plot(GdkPixmap *, graph *, int , char *, double , double , int); 
static void plotOlap(GdkPixmap *pixmap, graph *gr, int record, 
						char *udata, double begin, double end, int inc);

void
mag_plot_files(GtkWidget *da, GdkPixmap *pixmap, int wind_width, int wind_height, int win_record)
{
	int         i, j, k, step, fontW, fontH;
	float	    scale;
	graph	    mag_tmp;
	double		temp_float;
	int			temp_int, dataS;
	int			temp_int_2;
	int         x, yPos, yPos1, yPos2;
	char        axis_label[80], *dataPtr; 
	int			inc, label_width, str_width, labelW;
	float		gainFactor = (settings.general.display.gather.gain[MAGNIFYSCR] > 0)
								? settings.general.display.gather.gain[MAGNIFYSCR]
								: 1.0;
	gboolean	plotted=FALSE, gainON;
	DATAINFO	*dataInfo;
	GSList		*trcIter;
	traceInfo	*trcData;
	char		*tFormStr;

	settings.general.font.DA = da;
	settings.general.font.font = LABELF;
  
	states.magnify.trace_depth[states.currentScreen] = 
			(float) (wind_height - (MAG_T_MARGIN + MAG_B_MARGIN) - 
						(MAG_TFORM_MARGIN)*states.trace.numSelected) / 
		  			(float) states.trace.numSelected;

	// find the widest label we need to accommodate, 
	// but do it only once per configure/startup event
	if (!states.magnify.plot.label_width[states.currentScreen])
	{
		for (trcIter = states.trace.selection, label_width=0;
			 trcIter;
			 trcIter = g_slist_next(trcIter))
		{
			trcData = trcIter->data;
	  		str_width = stringWidth(trcData, states.magnify.trace_depth[states.currentScreen] + MAG_TFORM_MARGIN, LABEL);
			if (str_width > label_width)
				label_width = str_width;
		}
		states.magnify.plot.label_width[states.currentScreen] = settings.general.label.length ? 
																settings.general.label.length : 
																label_width;
	}
	label_width = states.magnify.plot.label_width[states.currentScreen];
	fontW = fontSIZE("9", WIDTH);
	fontH = fontSIZE("9", HEIGHT);

	// get some statistics
	for (trcIter = states.trace.selection, i=0, inc=0;
		 trcIter;
		 trcIter = g_slist_next(trcIter), i++)
	{
		if (win_record != i && 
			win_record != -1)
			continue;		

		trcData = trcIter->data;

		switch (settings.magnify.Wamp)
		{
			case WSCALETRC:		// WINDOW Scale = TRACE
				trcData->magnify.plot.minAmp = trcData->magnify.amps.min;
				trcData->magnify.plot.maxAmp = trcData->magnify.amps.max;
			break;
			
			case WSCALEWIND:	// WINDOW Scale = WINDOW
				if (!settings.magnify.overlay)
				{	// normal mode, not overlay
					switch (trcData->data.head.units) 
					{
						case PQL_UNITS_VOLTS:
							scale = trcData->data.head.scale_fac / trcData->data.head.gainConst;
					
							trcData->magnify.plot.minAmp = trcData->magnify.amps.min * scale - 
								(states.magnify.absAmp.diffVolts - (trcData->magnify.amps.max - 
								trcData->magnify.amps.min)*scale)/2;
							trcData->magnify.plot.maxAmp = trcData->magnify.amps.max * scale + 
								(states.magnify.absAmp.diffVolts - (trcData->magnify.amps.max - 
								trcData->magnify.amps.min)*scale)/2;
							trcData->magnify.plot.minAmp /= scale;
							trcData->magnify.plot.maxAmp /= scale;
						break;
						default:
							trcData->magnify.plot.minAmp = trcData->magnify.amps.min -
								(states.magnify.absAmp.diff - 
								(trcData->magnify.amps.max - trcData->magnify.amps.min))/2;
							trcData->magnify.plot.maxAmp = trcData->magnify.amps.max + 
								(states.magnify.absAmp.diff - 
								(trcData->magnify.amps.max - trcData->magnify.amps.min))/2;
						break;
					}
				}
				else
				{	// overlay mode
					trcData->magnify.plot.minAmp = states.magnify.absAmp.min;
					trcData->magnify.plot.maxAmp = states.magnify.absAmp.max;
				}
			break;
	    }

		// we need to determine the number of digits of the decimation factor, 
		// make a close guess first since the actual increment partially depends 
		// on how much space is necessary to print the increment,
		// i.e., a circular dependency we can't get around
    	step = (int) ceil((float) ((states.magnify.limits.selection.user.length) /
										(float) (wind_width-(label_width*1.5)-20)) / 
									((float) trcData->data.head.sampInt / (float) 1000000));
		for (j=0,k=step;k>0;k/=10,j++);
		if (j > inc)
			inc = j;		// this is the maximum digits required to plot the decimation
	}

	for (trcIter = states.trace.selection, i=0;
		 trcIter;
		 trcIter = g_slist_next(trcIter), i++)
	{
		double	max, min;
		
		if (win_record != i && win_record != -1)
			continue;		

		trcData = trcIter->data;
		if (settings.magnify.overlay) 
		{
      		viewport(&trcData->magnify.GR[states.currentScreen], 
					label_width+PQLLABELXTRA, 					// left x-pixel
					wind_width - (fontW*inc+PQLINCXTRA), 		// right x-pixel
					MAG_T_MARGIN, 								// top y-pixel
					(states.trace.numSelected) * states.magnify.trace_depth[states.currentScreen] + MAG_T_MARGIN);
    	}
    	else 
		{
			yPos1 = i * states.magnify.trace_depth[states.currentScreen] + 
						MAG_T_MARGIN + (i+1)*MAG_TFORM_MARGIN + 1; 
			yPos2 = yPos1 + states.magnify.trace_depth[states.currentScreen]; 

      		viewport(&trcData->magnify.GR[states.currentScreen], 
						label_width+PQLLABELXTRA, wind_width - (fontW*inc+PQLINCXTRA), 
						yPos1, yPos2);
    	}
    
		max = trcData->magnify.plot.maxAmp;
		min = trcData->magnify.plot.minAmp;
		gainON = (!states.magnify.selection) 
					? TRUE
					: g_slist_find(states.magnify.selection, trcData)
						? TRUE
						: FALSE;
		if (gainFactor != 1.0 &&
			gainON)
		{
			double mid, diffH;
			diffH = (max - min)/2.0;
			mid = max - diffH;
			max = mid + diffH/(gainFactor);
			min = mid - diffH/(gainFactor);
		}
    	window(&trcData->magnify.GR[states.currentScreen], 
					(states.magnify.limits.selection.user.left), 
					(states.magnify.limits.selection.user.right), 
					min, max, 0, 0);

     	trcData->magnify.plot.increment = get_data_increment(trcData, 
										states.magnify.limits.selection.user.length,
			     						USER2PIXX(&trcData->magnify.GR[states.currentScreen], 
														states.magnify.limits.selection.user.length) - 
				 						USER2PIXX(&trcData->magnify.GR[states.currentScreen],
				 										(double) 0));
	}

	for (trcIter = states.trace.selection, i=0;
		 trcIter;
		 trcIter = g_slist_next(trcIter), i++)
	{
		if (win_record != i && 
			win_record != -1)
			continue;		

		trcData = trcIter->data;

		temp_float=((1000000.0/trcData->data.head.sampInt) *trcData->magnify.begin);
		temp_int_2 = ((ceil)(temp_float)); // this is the number of data
    	
		if (states.tForm.on && trcData->data.tForm.data)
		{
			dataS = dsizes[MY_FLOAT];
			dataInfo = &trcData->data.tForm;
			dataPtr = dataInfo->data;
		}
		else
		{
			dataPtr = trcData->data.dataInfo.data;
			dataS = dsizes[trcData->data.head.data_form];
		}
		temp_int = temp_int_2 * dataS;
		trcData->magnify.data_start = (char *) (((long) dataPtr) + temp_int);
      
		if (!i)
		{	// only need to do this once
			states.magnify.limits.margin[states.currentScreen].left = 
		  					USER2PIXX(&trcData->magnify.GR[states.currentScreen], 
											states.magnify.limits.selection.user.left);

			states.magnify.limits.margin[states.currentScreen].right = 
		  					USER2PIXX(&trcData->magnify.GR[states.currentScreen], 
											states.magnify.limits.selection.user.right);

			states.magnify.limits.margin[states.currentScreen].length =	
		  					states.magnify.limits.margin[states.currentScreen].right - 
		  					states.magnify.limits.margin[states.currentScreen].left;
		}
		
	    if (trcData->magnify.begin != -1) 
		{
		    mag_plot(pixmap, &trcData->magnify.GR[states.currentScreen], i, 
						trcData->magnify.data_start, 
						states.magnify.limits.selection.user.left, 
						states.magnify.limits.selection.user.right, 
						trcData->magnify.plot.increment);
		}

		if (!(states.tForm.on && trcData->data.tForm.data))
		    plotOlap(pixmap, &trcData->magnify.GR[states.currentScreen], i, 
					trcData->magnify.data_start, 
					states.magnify.limits.selection.user.left, 
					states.magnify.limits.selection.user.right, 
					trcData->magnify.plot.increment); 

	    if (trcData->magnify.begin != -1) 
		{
			plotted=TRUE;
#if 0
			if (trcData->data.fileType == SEGY_DATA &&
				settings.general.units == MYVOLTS) 
			{
				scale = trcData->data.head.scale_fac / 
					(float) trcData->data.head.gainConst;
	
				viewport(&mag_tmp, label_width+PQLLABELXTRA, wind_width - (fontW*inc+PQLINCXTRA), 
      				MAG_T_MARGIN +
					USER2PIXY(&trcData->magnify.GR[states.currentScreen], 
									trcData->magnify.GR[states.currentScreen].user_ymax ), 
		   			USER2PIXY(&trcData->magnify.GR[states.currentScreen], 
									trcData->magnify.GR[states.currentScreen].user_ymin ));
				trcData->magnify.plot.minAmp *= scale;
				trcData->magnify.plot.maxAmp *= scale;
				window(&mag_tmp, 
						(states.magnify.limits.selection.user.left), 
						(states.magnify.limits.selection.user.right), 
						trcData->magnify.plot.minAmp,
						trcData->magnify.plot.maxAmp,
						0, 0);
			}
			else
			{
				mag_tmp = trcData->magnify.GR[states.currentScreen];
			}
#endif
			mag_tmp = trcData->magnify.GR[states.currentScreen];

			if (settings.general.display.mode == TRACEMODE)
			{
				printLabel(pixmap, trcData, i, states.magnify.trace_depth[states.currentScreen],
						wind_width - PQLINCXTRA - fontW*inc, MAGNIFYSCR);
			}
			else
			{
				static int last;
				int	middle = (states.trace.trace_depth[states.currentScreen]*i) +
							states.trace.trace_depth[states.currentScreen]/2;
				if (!i)
					last = 0;
				if (middle - fontH > last)
				{
					printLabel(pixmap, trcData, i, states.magnify.trace_depth[states.currentScreen], 
								wind_width - PQLINCXTRA - fontW*inc, MAGNIFYSCR);
					last = middle + fontH/2;
				}
			}

			if (!settings.magnify.overlay &&	// don't draw individual y axes if overlay mode
				settings.general.display.mode == TRACEMODE) 
			{
				yaxis(pixmap, GXC[NORMALGC][MAGNIFYSCR], &mag_tmp, 
					(double) PIX2USERX(&mag_tmp, label_width+PQLLABELXTRA - 10),
	      			(double) mag_tmp.user_ymin, (double) mag_tmp.user_ymax, 1, -1, 0, (double) 0.0,
	      			MYFLOAT);

//	      			((settings.general.units == MYVOLTS ||
//	      				states.tForm.filter == settings.general.filter.AGCidx) 
//	      				? MYFLOAT : MYFLOAT));

				if (states.tForm.on && trcData->data.tForm.data)
					tFormStr = trcData->data.tForm.tFormStr;
				else
					tFormStr = g_strdup("Original");
				x = label_width+PQLLABELXTRA;
				yPos = MAG_T_MARGIN + i*states.magnify.trace_depth[states.currentScreen] +
						i*MAG_TFORM_MARGIN + 1;
				fontDraw(pixmap, tFormStr, x, yPos, NORMALGC, SAMESCR, -1, -1);
			}
		}	// end if
	}		// end for

	if (settings.magnify.overlay && 
		(settings.magnify.Wamp != WSCALETRC)) 
	{
		yaxis(pixmap, GXC[NORMALGC][MAGNIFYSCR], &mag_tmp, 
					(double) PIX2USERX(&mag_tmp, label_width+PQLLABELXTRA - 10),
	      			(double) mag_tmp.user_ymin, (double) mag_tmp.user_ymax, 1, -1, 0, (double) 0.0,
	      			MYINT);
//	      			((settings.general.units == MYVOLTS) ? MYFLOAT : MYINT));
	}

	if (win_record == -1)
	{	// don't need to do any of this when bringing a trace to the front in overlay mode
		//  print x axis corresponding to main screen
		viewport(&states.magnify.mainGR[states.currentScreen], 
				  states.magnify.limits.margin[states.currentScreen].left, 
				  states.magnify.limits.margin[states.currentScreen].right,
				  0, MAG_T_MARGIN);
		
		window(&states.magnify.mainGR[states.currentScreen], 
				  (double) 0, (double) states.trace.plot.length[settings.trace.time], 
				  (double) 0.0, (double) MAG_T_MARGIN, 
				  0, (double) 0.0);
		settings.general.font.font = AXISF;
		switch(settings.trace.time)
		{
			case RELATIVE:
				xaxis(pixmap, GXC[NORMALGC][MAGNIFYSCR], &states.magnify.mainGR[states.currentScreen], 
						(double) 0.0, (double) 8.0, 
						(double) states.trace.plot.length[settings.trace.time], 
						-1, 1, -1, (double) 0.0, FALSE, (double) 0);
				sprintf(axis_label,"Seconds");
				labelW = fontSIZE(axis_label, WIDTH);
				x = (states.magnify.limits.margin[states.currentScreen].left < labelW)
						? 0
						: states.magnify.limits.margin[states.currentScreen].left - labelW;
				yPos = 0;
				fontDraw(pixmap, axis_label, x, yPos, NORMALGC, SAMESCR, -1, -1);
			break;
			
			case ABSOLUTE:
				xAxisABS(pixmap, GXC[NORMALGC][MAGNIFYSCR], 
								&states.magnify.mainGR[states.currentScreen], 
								states.trace.absTime.start, MAGNIFYSCR, LABEL_ABOVE);
			break;
		}

		{	// draw our box on the main window x-axis
			int mag_x1, mag_x2;
			mag_x1 = USER2PIXX(&states.magnify.mainGR[states.currentScreen], 
				  	PIX2USERX(&states.trace.GR[states.currentScreen], 
				  					states.trace.limits.selection.pix[states.currentScreen].left));
			mag_x2 = USER2PIXX(&states.magnify.mainGR[states.currentScreen], 
			  		PIX2USERX(&states.trace.GR[states.currentScreen], 
			  						states.trace.limits.selection.pix[states.currentScreen].right));
			if (mag_x1==mag_x2)
				mag_x2++;
			gdk_draw_rectangle(pixmap, GXC[INVGC][MAGNIFYSCR], TRUE, 
					mag_x1, MAG_T_MARGIN-10, mag_x2 - mag_x1, 10);
		}

		//  print x axis corresponding to magnify screen 
		if (plotted)
		{
			viewport(&states.magnify.GR[states.currentScreen], 
					  states.magnify.limits.margin[states.currentScreen].left, 
					  states.magnify.limits.margin[states.currentScreen].right, 
					  wind_height - MAG_B_MARGIN, wind_height);
			window(&states.magnify.GR[states.currentScreen], 
					  (double) 0, (double) states.magnify.limits.selection.user.length, 
					  (double) MAG_B_MARGIN, (double) 0, 0, 0);
					  
			switch(settings.trace.time)
			{
				case RELATIVE:
					xaxis(pixmap, GXC[NORMALGC][MAGNIFYSCR], &states.magnify.GR[states.currentScreen], 
						  (double) 0.0, (double) 9.0, 
						  (double) states.magnify.limits.selection.user.length, 
						  1, -1, -1, (double) 0.0, FALSE, (double) 0.0);
				break;
				
				case ABSOLUTE:
					xAxisABS(pixmap, GXC[NORMALGC][MAGNIFYSCR], 
								&states.magnify.GR[states.currentScreen], 
								states.magnify.absTime.start, MAGNIFYSCR, LABEL_BELOW);
				break;
			}
		}
		// reset our GR to absolute mag screen values for use elsewhere
		viewport(&states.magnify.GR[states.currentScreen], 
				  states.magnify.limits.margin[states.currentScreen].left, 
				  states.magnify.limits.margin[states.currentScreen].right, 
				  wind_height - MAG_B_MARGIN, wind_height);
    	window(&states.magnify.GR[states.currentScreen], 
					(states.magnify.limits.selection.user.left), 
					(states.magnify.limits.selection.user.right), 
					(double) 0.0, (double) 0.0,	0, 0);				  
	}

  return;
}

typedef struct
{
	int		stop, lastPoint, dataSize, dataType;
	int		mag_rec_no, /*rec_no,*/ ptInc, decInc, inc;
	double	tm, tm_inc;
	struct
	{
    	gint32	*ptr;
    	short	*sptr;
    	float	*lptr;
		double	*dptr;
	} data;
	struct
	{
		int		*gaps, *olaps;
		int		gapNum, olapNum;
		int		gapON, olapON;
	} GO;
	GdkGC		*gc;
} plotInfo;

static int setGO(GdkPixmap *pixmap, graph *gr, plotInfo *pInfo)
{
	int		pix_x0, pix_y0, pix_x1, pix_y1;
	int		gapInc, ret=FALSE, decInc;
	double	pt1, pt2;
	traceInfo *trcData = g_slist_nth_data(states.trace.selection, pInfo->mag_rec_no);

	if (settings.magnify.overlay)
		return ret;		// no gaps or overlaps displayed in overlay mode

	decInc = 0;

	if (trcData->trace.numGaps)
	{
		for(pInfo->GO.gapNum=0, pInfo->GO.gaps = trcData->trace.gaps;		// find the current gap 
			pInfo->GO.gapNum < trcData->trace.numGaps &&					// while still gaps defined
			pInfo->GO.gaps[GAPBEG] < pInfo->ptInc;							// until the gap start > our point
			pInfo->GO.gapNum++);

		if (pInfo->GO.gapNum)		// if we crossed one, reset our counter if we're in the middle of the overlap
			if (pInfo->ptInc < pInfo->GO.gaps[(pInfo->GO.gapNum-1)*2+1])
				pInfo->GO.gapNum--;

		if (pInfo->GO.gapNum < trcData->trace.numGaps)
		if (pInfo->ptInc > (pInfo->GO.gaps[GAPBEG]) &&
			pInfo->ptInc < (pInfo->GO.gaps[GAPEND]))
		{	// starting in the middle of a gap
			gapInc = (pInfo->GO.gaps[(pInfo->GO.gapNum)*2+1] - pInfo->ptInc)/ pInfo->inc;
	
			if (gapInc)
			{	// gap ends after the first pixel, draw the gap line
				pInfo->gc = GGXC[MAGNIFYSCR][0];
				if (gapInc + states.magnify.limits.margin[states.currentScreen].left > 
							 states.magnify.limits.margin[states.currentScreen].right)
				{	// special case: only gap on entire display
					pix_y0 = MAG_T_MARGIN + (states.magnify.trace_depth[states.currentScreen] * 
											(pInfo->mag_rec_no) + 
											states.magnify.trace_depth[states.currentScreen]/2);
	    			gdk_draw_line(pixmap, pInfo->gc, 
									states.magnify.limits.margin[states.currentScreen].left, pix_y0,
									states.magnify.limits.margin[states.currentScreen].right, pix_y0);
					ret = TRUE;		// tell the caller we're done plotting
				}
				else
				{	// draw gap fill to next valid point
					int	dataType;
					if (states.tForm.on && trcData->data.tForm.data)
					{
						dataType = MY_FLOAT;
					}
					else
					{
						dataType = trcData->data.head.data_form;
					}
    				switch (dataType)
					{
						// pt1 = last point before current gap
						// pt2 = beginning of next good data
						case BIT32:
							pt1 = (double) pInfo->data.ptr[pInfo->GO.gaps[(pInfo->GO.gapNum)*2]-1]; 
							pt2 = (double) pInfo->data.ptr[pInfo->GO.gaps[(pInfo->GO.gapNum)*2+1]];
						break;
      					case BIT16:
							pt1 = (double) pInfo->data.sptr[pInfo->GO.gaps[GAPBEG]-1];
							pt2 = (double) pInfo->data.sptr[pInfo->GO.gaps[GAPEND]];
						break;
      					case MY_FLOAT:
							pt1 = (double) pInfo->data.lptr[pInfo->GO.gaps[GAPBEG]-1];
							pt2 = (double) pInfo->data.lptr[pInfo->GO.gaps[GAPEND]];
						break;
      					case MY_DOUBLE:
							pt1 = (double) pInfo->data.dptr[pInfo->GO.gaps[GAPBEG]-1];
							pt2 = (double) pInfo->data.dptr[pInfo->GO.gaps[GAPEND]];
						break;
					}

		    		pix_x0 = USER2PIXX(gr, pInfo->tm);
		    		pix_y0 = USER2PIXY(gr, pt1);
		
					if (pix_y0 < MAG_T_MARGIN + 
							states.magnify.trace_depth[states.currentScreen]*(pInfo->mag_rec_no))
					{	// don't plot off the top
						pix_y0 = MAG_T_MARGIN + 
									states.magnify.trace_depth[states.currentScreen]*(pInfo->mag_rec_no);
					}
					if (pix_y0 > MAG_T_MARGIN + 
							states.magnify.trace_depth[states.currentScreen]*(pInfo->mag_rec_no+1))
					{	// don't plot off the bottom
						pix_y0 = MAG_T_MARGIN + 
									states.magnify.trace_depth[states.currentScreen]*(pInfo->mag_rec_no+1);
					}

					pInfo->ptInc += (pInfo->inc * (gapInc));
		    		pInfo->tm += (pInfo->tm_inc * (double) (gapInc));
		
		    		pix_x1 = USER2PIXX(gr, pInfo->tm);
		    		pix_y1 = USER2PIXY(gr, pt2);
		
					if (pix_y1 < MAG_T_MARGIN + 
							states.magnify.trace_depth[states.currentScreen]*(pInfo->mag_rec_no))
					{	// don't plot off the top
						pix_y1 = MAG_T_MARGIN + 
									states.magnify.trace_depth[states.currentScreen]*(pInfo->mag_rec_no);
					}
					if (pix_y1 > MAG_T_MARGIN + 
							states.magnify.trace_depth[states.currentScreen]*(pInfo->mag_rec_no+1))
					{	// don't plot off the bottom
						pix_y1 = MAG_T_MARGIN + 
									states.magnify.trace_depth[states.currentScreen]*(pInfo->mag_rec_no+1);
					}

					// don't draw off the right edge 
					if (pix_x1 > states.magnify.limits.margin[states.currentScreen].right)
						pix_x1 = states.magnify.limits.margin[states.currentScreen].right;
		
		    		gdk_draw_line(pixmap, pInfo->gc, pix_x0, pix_y0, pix_x1, pix_y1);

					pInfo->GO.gapNum++;
					pInfo->gc = GXC[NORMALGC][MAGNIFYSCR];
					decInc = pInfo->GO.gaps[GAPBEG-1] - pInfo->ptInc;
				}
			}
			else
			{
				decInc = pInfo->GO.gaps[GAPBEG] - pInfo->ptInc;
			}
		}
	}

	if (trcData->trace.overlaps)
	{
		for(pInfo->GO.olapNum=0, pInfo->GO.olaps = trcData->trace.overlaps;		// find the current olap 
			pInfo->GO.olapNum < trcData->trace.numOlaps &&					// while still olaps defined
			pInfo->GO.olaps[OLAPBEG] < pInfo->ptInc;							// until the olap start > our point
			pInfo->GO.olapNum++);

		if (pInfo->GO.olapNum)		// if we crossed one, reset our counter if we're in the middle of the overlap
			if (pInfo->ptInc < pInfo->GO.olaps[(pInfo->GO.olapNum-1)*2+1])
				pInfo->GO.olapNum--;

		if ((pInfo->ptInc > pInfo->GO.olaps[OLAPBEG]) &&
			(pInfo->ptInc < pInfo->GO.olaps[OLAPEND]))
		{
			pInfo->gc = PGXC[MAGNIFYSCR][0];
			pInfo->GO.olapON = TRUE;
		}
	}

	pInfo->decInc = decInc;
	pInfo->stop = pInfo->lastPoint;
	if (pInfo->GO.olapON)
	{
		pInfo->stop = pInfo->GO.olaps[OLAPEND];		// end of current overlap
	}
	else
	{
		if (pInfo->GO.gapNum < trcData->trace.numGaps)
		{
			pInfo->stop = pInfo->GO.gaps[GAPBEG];		// start of next gap
		}
		if (pInfo->GO.olapNum < trcData->trace.numOlaps) 
		{
			if (pInfo->GO.gapNum < trcData->trace.numGaps)
			{	// check if gap is stopping first
				if (pInfo->GO.olaps[OLAPBEG] < pInfo->GO.gaps[GAPBEG])
				{
					pInfo->stop = pInfo->GO.olaps[OLAPBEG];		// start of next overlap
				}
			}
			else
			{
				pInfo->stop = pInfo->GO.olaps[OLAPBEG];		// start of next overlap
			}
		}
	}
	return ret;
}

#define FLAGMARGIN	2
#define	FLAGFACTOR	12
#define MAXFLAG		10
#define MINFLAG		5

static	int	*vertical, flagSize, totVert;

static int	checkGO(GdkPixmap *pixmap, graph *gr, plotInfo *pInfo, int x, int y)
{
	GdkPoint	flag[3];
	static int	vertPos, gapIter;
	int			gapInc, ret=FALSE, decInc;
	int			pix_x0, pix_x1=0, pix_y1;
	double		pt2=0;
	gboolean	gap = FALSE;
	traceInfo	*trcData = g_slist_nth_data(states.trace.selection, pInfo->mag_rec_no);

	if (settings.magnify.overlay)
		return ret;		// no gaps or overlaps displayed in overlay mode

	if (!flagSize)
	{
		flagSize = gr->pix_ydif/FLAGFACTOR;
		flagSize = (flagSize > MAXFLAG) 
						? MAXFLAG 
						: ((flagSize < MINFLAG) 
								? MINFLAG 
								: flagSize);

		totVert = 50;	// initialize size of vertical array to 50
		vertical = (int *) calloc(totVert, sizeof(int));
	}

	if (pInfo->GO.gapNum < trcData->trace.numGaps)
	{	// have we hit a gap?  change colour if so...
		if (settings.magnify.gFlag)
		{	// undone: set up start variables for case when beyond first gap in same vertical pixel
			gapIter = 0;
			vertPos=0;
			while(x < vertical[vertPos])
				vertPos++;
		}

		if (pInfo->ptInc+pInfo->decInc >= pInfo->GO.gaps[GAPBEG])
		{	// hit a gap, process:
			gap = TRUE;
			pInfo->gc = GGXC[MAGNIFYSCR][0];
			pix_x0 = USER2PIXX(gr, pInfo->tm);
			if (settings.magnify.gFlag)	// && !gapON)
			{	
				if(!gapIter)	// draw vertical line only once at a single X pixel
					gdk_draw_line(pixmap, pInfo->gc,
						pix_x0, gr->pix_ymin + vertPos*(flagSize+1) + FLAGMARGIN, 
						pix_x0, y);
				flag[0].x = pix_x0;
				flag[0].y = gr->pix_ymin + vertPos*(flagSize+1) + gapIter*(flagSize+1) + FLAGMARGIN;
				flag[1].x = pix_x0;
				flag[1].y = gr->pix_ymin + vertPos*(flagSize+1) + gapIter*(flagSize+1) + 
								flagSize + FLAGMARGIN;
				flag[2].x = pix_x0 + flagSize;
				flag[2].y = gr->pix_ymin + vertPos*(flagSize+1) + gapIter*(flagSize+1) + 
								flagSize/2 + FLAGMARGIN;
				gdk_draw_polygon(pixmap, pInfo->gc, TRUE, flag, 3);
				if (vertPos+gapIter>totVert)
				{
					totVert += 50;
					vertical = (int *) realloc(vertical, totVert*sizeof(int));
				}
				vertical[vertPos+gapIter] = flag[2].x + 1;
				gapIter++;
			}

			gapInc = (pInfo->GO.gaps[GAPEND] - 
						pInfo->GO.gaps[GAPBEG] - 
						(pInfo->ptInc - pInfo->GO.gaps[GAPBEG])) /
									pInfo->inc;

			if (gapInc)
			{	// draw gap fill to next valid point, but only if next valid point is in new vertical pixel
				int	dataType;
				if (states.tForm.on && trcData->data.tForm.data)
				{
					dataType = MY_FLOAT;
				}
				else
				{
					dataType = trcData->data.head.data_form;
				}
    			switch (dataType)
				{
					case BIT32:
						pt2 = (double) pInfo->data.ptr[pInfo->GO.gaps[GAPEND]];
						break;

      				case BIT16:
						pt2 = (double) pInfo->data.sptr[pInfo->GO.gaps[GAPEND]];
						break;

      				case MY_FLOAT:
						pt2 = (double) pInfo->data.lptr[pInfo->GO.gaps[GAPEND]];
						break;

      				case MY_DOUBLE:
						pt2 = (double) pInfo->data.dptr[pInfo->GO.gaps[GAPEND]];
						break;
				}

				pInfo->ptInc += (pInfo->inc * (gapInc));
	    		pInfo->tm += (pInfo->tm_inc * (double) (gapInc));

	    		pix_x1 = USER2PIXX(gr, pInfo->tm);
				if (settings.magnify.Tamp == FIXEDS)
				{
					if (pt2<trcData->magnify.amps.min)
						pt2 = trcData->magnify.amps.min;
					if (pt2>trcData->magnify.amps.max)
						pt2 = trcData->magnify.amps.max;
				}
	    		pix_y1 = USER2PIXY(gr, pt2);

				// don't draw off the edge if next valid point isn't visible
				if (pix_x1 > states.magnify.limits.margin[states.currentScreen].right)
				{
					pix_x1 = states.magnify.limits.margin[states.currentScreen].right;
					ret = TRUE;
				}

				if (pix_y1 < MAG_T_MARGIN + 
							states.magnify.trace_depth[states.currentScreen] *
							(pInfo->mag_rec_no))
				{
					pix_y1 = MAG_T_MARGIN + 
							states.magnify.trace_depth[states.currentScreen] *
							(pInfo->mag_rec_no);
					ret = TRUE;
				}
				if (pix_y1 > MAG_T_MARGIN + 
							states.magnify.trace_depth[states.currentScreen] *
							(pInfo->mag_rec_no+1))
				{
					pix_y1 = MAG_T_MARGIN + 
							states.magnify.trace_depth[states.currentScreen] *
							(pInfo->mag_rec_no+1);
					ret = TRUE;
				}

	    		gdk_draw_line(pixmap, pInfo->gc, pix_x0, y, pix_x1, pix_y1);
			}

			pInfo->GO.gapNum++;
			pInfo->gc = GXC[NORMALGC][MAGNIFYSCR];
			decInc = pInfo->GO.gaps[GAPBEG-1] - pInfo->ptInc;
			pInfo->decInc = decInc;
		}
		else
		{
			decInc = 0;
		}
	}		// end if (gapNum < trcData->trace.numGaps)

	if (pInfo->GO.olapNum < trcData->trace.numOlaps)
	{
		if (settings.magnify.oFlag && !pInfo->GO.olapON)
		{
			gapIter = 0;
			vertPos=0;
			while(x < vertical[vertPos])
				vertPos++;
		}

		if (!pInfo->GO.olapON && 
			pInfo->ptInc+pInfo->decInc >= pInfo->GO.olaps[OLAPBEG])
		{	// overlap found
			pInfo->gc = PGXC[MAGNIFYSCR][0];
			if (settings.magnify.oFlag) 	// && !pInfo->GO.olapON)
			{	
				if (gap)
					x = pix_x1;
				if(!gapIter)	// draw vertical line only once at a single X pixel
					gdk_draw_line(pixmap, pInfo->gc, x, 
							gr->pix_ymin + vertPos*(flagSize+1) + FLAGMARGIN, 
							x, y);
				flag[0].x = x;
				flag[0].y = gr->pix_ymin + vertPos*(flagSize+1) + 
							gapIter*(flagSize+1) + FLAGMARGIN;
				flag[1].x = x;
				flag[1].y = gr->pix_ymin+ + vertPos*(flagSize+1) + 
							gapIter*(flagSize+1) + flagSize + FLAGMARGIN;
				flag[2].x = x + flagSize;
				flag[2].y = gr->pix_ymin+ vertPos*(flagSize+1) + 
							gapIter*(flagSize+1) + flagSize/2 + FLAGMARGIN;
				gdk_draw_polygon(pixmap, pInfo->gc, TRUE, flag, 3);
				if (vertPos+gapIter>totVert)
				{
					totVert += 50;
					vertical = (int *) realloc(vertical, totVert*sizeof(int));
				}
				vertical[vertPos+gapIter] = flag[2].x + 1;
				gapIter++;
			}	// end if
			pInfo->GO.olapON = TRUE;
		}	// end while

		if (pInfo->GO.olapON && pInfo->ptInc+pInfo->decInc >= 
				pInfo->GO.olaps[OLAPEND])
		{	// overlap stop, return to current GC
			pInfo->GO.olapNum++;
			pInfo->gc = GXC[NORMALGC][MAGNIFYSCR];
			pInfo->GO.olapON = FALSE;
		}
	}		// end if(magnify.overlaps)

	pInfo->stop = pInfo->lastPoint;
	if (pInfo->GO.olapON)
	{
		pInfo->stop = pInfo->GO.olaps[OLAPEND];		// end of current overlap
	}
	else
	{
		if (pInfo->GO.gapNum < trcData->trace.numGaps)
		{
			pInfo->stop = pInfo->GO.gaps[GAPBEG];		// start of next gap
		}
		if (pInfo->GO.olapNum < trcData->trace.numOlaps) 
		{
			if (pInfo->GO.gapNum < trcData->trace.numGaps)
			{	// check if gap is stopping first
				if (pInfo->GO.olaps[OLAPBEG] < 
					pInfo->GO.gaps[GAPBEG])
				{
					pInfo->stop = pInfo->GO.olaps[OLAPBEG];		// start of next overlap
				}
			}
			else
			{
				pInfo->stop = pInfo->GO.olaps[OLAPBEG];		// start of next overlap
			}
		}
	}
	return ret;
}

static void initData(plotInfo *plotI, char *udata)
{
	DATAINFO	*dataInfo;
	traceInfo	*trcData = g_slist_nth_data(states.trace.selection, plotI->mag_rec_no);

	if (states.tForm.on && trcData->data.tForm.data)
	{
		dataInfo = &trcData->data.tForm;
		plotI->data.lptr = (float *) dataInfo->data;
		plotI->dataSize = sizeof(float);
		plotI->ptInc = ((long) udata - (long) plotI->data.lptr)/plotI->dataSize;
		plotI->dataType = MY_FLOAT;
	}
	else
	{
		plotI->dataType = trcData->data.head.data_form;
		plotI->dataSize = dsizes[plotI->dataType];
		switch (plotI->dataType) 
		{
			case BIT32:
				plotI->data.ptr = (gint32 *) trcData->data.dataInfo.data;
				plotI->ptInc = ((long) udata - (long) plotI->data.ptr)/plotI->dataSize;
			break;

			case BIT16:
				plotI->data.sptr = (short *) trcData->data.dataInfo.data;
				plotI->ptInc = ((long) udata - (long) plotI->data.sptr)/plotI->dataSize;
			break;

			case MY_FLOAT:
				plotI->data.lptr = (float *) trcData->data.dataInfo.data;
				plotI->ptInc = ((long) udata - (long) plotI->data.lptr)/plotI->dataSize;
			break;

			case MY_DOUBLE:
				plotI->data.dptr = (double *) trcData->data.dataInfo.data;
				plotI->ptInc = ((long) udata - (long) plotI->data.dptr)/plotI->dataSize;
			break;
		}
	}
}

static void
mag_plot(GdkPixmap *pixmap, graph *gr, int mag_rec_no, char *udata, 
				double begin, double end, int inc)
{
	int		ndata;
	int		y1, y2, pix_x0, pix_x1, pix_y0, pix_y1;
	double	fmax, fmin=0, fpt, sps, tmStart;
	float	plot_length;
	double	ceil();
	float	l_trace;
	GdkGC	*localGC;
	short	*sptr=NULL;
	gint32	*ptr=NULL;
	float	*lptr=NULL;
	double	*dptr=NULL;
  	plotInfo	pInfo;
  	traceInfo *trcData = g_slist_nth_data(states.trace.selection, mag_rec_no);

	memset(&pInfo, 0, sizeof(pInfo));
	flagSize=0;
	vertical = NULL;
  
	if (settings.magnify.overlay)
	{
		int	gcidx = mag_rec_no % 30;
		pInfo.gc = localGC = OGXC[gcidx];
	}
	else
	{
		pInfo.gc = localGC = GXC[NORMALGC][MAGNIFYSCR];
	}

	pInfo.mag_rec_no = mag_rec_no;
	trcData->magnify.num_points_shown = 0;
    l_trace = ((float) trcData->data.dataInfo.head.numSamples / 
				(1000000. / trcData->data.head.sampInt));
    
	pInfo.GO.gapON = pInfo.GO.olapON = FALSE;
	pInfo.GO.olapNum = pInfo.GO.gapNum = 0;

    tmStart = begin + trcData->magnify.offset;
    pInfo.tm = tmStart;

	pInfo.inc = inc;
	pInfo.tm_inc = (double) (((trcData->data.head.sampInt) / 1000000.) * 
								((float) pInfo.inc));
	plot_length = trcData->magnify.end - trcData->magnify.begin;
	ndata = (int) ((1000000. / trcData->data.head.sampInt) * plot_length);

	if (ndata == 0) 
	{
    	return;
	}
    
    if(((ceil)(trcData->magnify.end)) == ((ceil)(l_trace)))
      ndata -= inc;		 // if end of trace don't print last data point
	
	initData(&pInfo, udata);
	switch(pInfo.dataType)
	{
		case MY_FLOAT:
			lptr = (float *) pInfo.data.lptr;
		break;

		case BIT32:
			ptr = (gint32 *) pInfo.data.ptr;
		break;

		case BIT16:
			sptr = (short *) pInfo.data.sptr;
		break;

		case MY_DOUBLE:
			dptr = (double *) pInfo.data.dptr;
		break;
	}

	pInfo.stop = pInfo.lastPoint = pInfo.ptInc + ndata + 1;
	trcData->magnify.num_points_shown = ndata;
	sps = 1000000. / (double) trcData->data.head.sampInt;

	if (setGO(pixmap, gr, &pInfo))
	{	// nothing to plot
		return;
	}

	pix_x0 = pix_x1 = USER2PIXX(gr, pInfo.tm);
	DATAP(pInfo.dataType, (pInfo.ptInc+pInfo.decInc), fmin);
	if (settings.magnify.Tamp == FIXEDS)
	{
		if (fmin<trcData->magnify.amps.min)
			fmin = trcData->magnify.amps.min;
		if (fmin>trcData->magnify.amps.max)
			fmin = trcData->magnify.amps.max;
	}
	pix_y0 = pix_y1 = USER2PIXY(gr, fmin);

	for (	
		;pInfo.ptInc < pInfo.lastPoint - pInfo.inc; 
		pix_x0 = pix_x1, pix_y0 = pix_y1)
	{
	    if (pInfo.inc > 1) 
		{
			DATAP(pInfo.dataType, (pInfo.ptInc+pInfo.decInc), fmin);
			fmax = fmin;
			for (fpt = fmax; 		// first data point for this x-pixel location
					(pInfo.decInc < pInfo.inc) && 		// loop until we've hit the end of the decimation
					(pInfo.ptInc + pInfo.decInc < pInfo.stop);	// or until we hit a stopping point: gap or end
					pInfo.decInc++) 
			{
				DATAP(pInfo.dataType, (pInfo.ptInc+pInfo.decInc), fpt);
		    	if (fpt < fmin)
				{
					fmin = fpt;
					continue;
				}
		    	if (fpt > fmax)
					fmax = fpt;
			}
			if (settings.magnify.Tamp == FIXEDS)
			{
				if (fmin<trcData->magnify.amps.min)
					fmin = trcData->magnify.amps.min;
				if(fmax>trcData->magnify.amps.max)
					fmax = trcData->magnify.amps.max;
				if (fmin>trcData->magnify.amps.max)
					fmin = trcData->magnify.amps.max;
				if(fmax<trcData->magnify.amps.min)
					fmax = trcData->magnify.amps.min;
			}
			y1 = USER2PIXY(gr, (double) fmin);
			y2 = USER2PIXY(gr, (double) fmax);
			gdk_draw_line(pixmap, pInfo.gc, pix_x1, y1, pix_x1, y2);
	    }			// end if

		if (pInfo.ptInc+pInfo.decInc == pInfo.stop) 
		{	// hit a gap or overlap
			int	lastStop = pInfo.stop;
			if (checkGO(pixmap, gr, &pInfo, pix_x1, pix_y1) &&
				pInfo.stop > lastStop)
				break;	// nothing more to plot
			while (pInfo.stop < lastStop)
			{	// handle overlaps of overlaps
		    	pix_x1 = USER2PIXX(gr, (double) (pInfo.stop/sps));
				lastStop = pInfo.stop;
				if (checkGO(pixmap, gr, &pInfo, pix_x1, pix_y1) &&
					pInfo.stop > lastStop)
					break;	// nothing more to plot
			}
			DATAP(pInfo.dataType, (pInfo.ptInc+pInfo.decInc), fmin);
			if (settings.magnify.Tamp == FIXEDS)
			{
				if (fmin<trcData->magnify.amps.min)
					fmin = trcData->magnify.amps.min;
				if (fmin>trcData->magnify.amps.max)
					fmin = trcData->magnify.amps.max;
			}
	    	pix_x1 = USER2PIXX(gr, pInfo.tm);
	    	pix_y1 = USER2PIXY(gr, fmin);
		}
		else
		{	// no gap, proceed to next decimation boundary
			if(pInfo.inc == 1 && pInfo.ptInc+1 == pInfo.stop)
			{	// force a checkGO; special case when decimation is 1
				pInfo.decInc++;
				continue;
			}
			pInfo.ptInc += pInfo.inc;
		    pInfo.tm += pInfo.tm_inc;
	    	pix_x1 = USER2PIXX(gr, pInfo.tm);
			DATAP(pInfo.dataType, (pInfo.ptInc), fmin);
			if (settings.magnify.Tamp == FIXEDS)
			{
				if (fmin<trcData->magnify.amps.min)
					fmin = trcData->magnify.amps.min;
				if (fmin>trcData->magnify.amps.max)
					fmin = trcData->magnify.amps.max;
			}
	    	pix_y1 = USER2PIXY(gr, fmin);
	    	gdk_draw_line(pixmap, pInfo.gc, pix_x0, pix_y0, pix_x1, pix_y1);
			pInfo.decInc = 0;
		}
	}			// end for 
	
    if (pInfo.inc > 1) 
	{	// draw the points for the last increment
		DATAP(pInfo.dataType, (pInfo.ptInc), fmin);
		fmax = fmin;
		for (fpt = fmin, pInfo.decInc=0; 
				pInfo.decInc + pInfo.ptInc < ndata;
				pInfo.decInc++)
		{
			DATAP(pInfo.dataType, pInfo.decInc, fpt);
	    	if (fpt < fmin)
				fmin = fpt;
	    	if (fpt > fmax)
				fmax = fpt;
		}
		if (settings.magnify.Tamp == FIXEDS)
		{
			if (fmin<trcData->magnify.amps.min)
				fmin = trcData->magnify.amps.min;
			if(fmax>trcData->magnify.amps.max)
				fmax = trcData->magnify.amps.max;
			if (fmin>trcData->magnify.amps.max)
				fmin = trcData->magnify.amps.max;
			if(fmax<trcData->magnify.amps.min)
				fmax = trcData->magnify.amps.min;
		}
		y1 = USER2PIXY(gr, (double) fmin);
		y2 = USER2PIXY(gr, (double) fmax);
		gdk_draw_line(pixmap, pInfo.gc, pix_x1, y1, pix_x1, y2);
    }			// end if

	if (vertical)
		free(vertical);
}					// end plot()

#define OVERLAPBOUND(bd)	((double) ((double) overlap->bounds[bd] / sps))

static void 
plotOlap(GdkPixmap *pixmap, graph *gr, int record, char *udata, double begin, double end, int inc)
{
    int     y1, y2, pix_x0, pix_x1, pix_y0, pix_y1;
    double  fmax, fmin, fpt, offset;
    double	sps;
	short	*sptr;
	gint32	*ptr;
	float	*lptr;
	double	*dptr;
	plotInfo	pInfo;
	GSList	*iter;
	OVERLAP	*overlap;
	float	plot_length;
	int		ndata, startPt;
	traceInfo *trcData = g_slist_nth_data(states.trace.selection, record);
	
	memset(&pInfo, 0, sizeof(plotInfo));
	offset = trcData->magnify.offset;
	pInfo.inc = inc;
	pInfo.gc = PGXC[MAGNIFYSCR][1];		// 2ndary overlap colour
    pInfo.tm_inc = (double) (((trcData->data.head.sampInt) / 1000000.) * ((float) pInfo.inc));
	pInfo.ptInc = 0;
	sps = 1000000. / (double) trcData->data.head.sampInt;
	pInfo.dataType = trcData->data.head.data_form;
	pInfo.dataSize = dsizes[pInfo.dataType];
	startPt = ((long) udata - (long) trcData->data.dataInfo.data)/pInfo.dataSize;

	for(iter=trcData->trace.olapData;
		iter;
		iter=g_slist_next(iter))
	{	// loop over all overlaps
		overlap = iter->data;
		if (OVERLAPBOUND(START) > end ||
			OVERLAPBOUND(END) < begin)
			continue;
		
		if (OVERLAPBOUND(START) < begin)
		{	// display of overlap starts in the middle of the overlap
			pInfo.ptInc = (begin - OVERLAPBOUND(START)) * sps;
		    pInfo.tm = begin + offset - (trcData->data.head.sampInt/1000000.);
		}
		else
		{	// display of overlap starts in the middle of the display
			pInfo.ptInc = 0;	
		    pInfo.tm = begin + offset + ((overlap->bounds[START]-startPt)/sps);
		}
		plot_length = trcData->magnify.end - trcData->magnify.begin;
		ndata = (int) ((1000000. / trcData->data.head.sampInt) * plot_length);
		pInfo.lastPoint = (pInfo.ptInc + ndata + 1) > overlap->len ? overlap->len : (pInfo.ptInc + ndata + 1);
		
		switch (trcData->data.head.data_form) 
		{
			case BIT32:
				ptr = pInfo.data.ptr = (gint32 *) overlap->data;
			break;
			case BIT16:
				sptr = pInfo.data.sptr = (short *) overlap->data;
			break;
			case MY_FLOAT:
				lptr = pInfo.data.lptr = (float *) overlap->data;
			break;
			case MY_DOUBLE:
				dptr = pInfo.data.dptr = (double *) overlap->data;
			break;
		}
	
		pix_x0 = pix_x1 = USER2PIXX(gr, pInfo.tm);
		DATAP(pInfo.dataType, pInfo.ptInc, fmin);
		pix_y0 = pix_y1 =  USER2PIXY(gr, (double) fmin);
	
		for (	pInfo.decInc = pInfo.ptInc%pInfo.inc+1;	// start pt inside our decimation - 1st time
				pInfo.ptInc < pInfo.lastPoint - pInfo.inc; 
				pix_x0 = pix_x1, pix_y0 = pix_y1)
		{
		    if (pInfo.inc > 1) 
			{
				DATAP(pInfo.dataType, (pInfo.ptInc+pInfo.decInc), fmin);
				fmax = fmin;
				for (	fpt = fmax;								// first data point for this x-pixel
						(pInfo.decInc < pInfo.inc) &&			// loop until we've hit the end of the decimation
						(pInfo.decInc < overlap->len);
						pInfo.decInc++) 
				{
					DATAP(pInfo.dataType, (pInfo.ptInc+pInfo.decInc), fpt);
			    	if (fpt < fmin)
					{
						fmin = fpt;
						continue;
					}
			    	if (fpt > fmax)
						fmax = fpt;
				}
				y1 = USER2PIXY(gr, (double) fmin);
				y2 = USER2PIXY(gr, (double) fmax);
				gdk_draw_line(pixmap, pInfo.gc, pix_x1, y1, pix_x1, y2);
		    }
	
			// proceed to next decimation boundary
			pInfo.ptInc += pInfo.inc;
		    pInfo.tm += pInfo.tm_inc;
	    	pix_x1 = USER2PIXX(gr, pInfo.tm);
	    	DATAP(pInfo.dataType, (pInfo.ptInc), fmin);
	    	pix_y1 = USER2PIXY(gr, (double) fmin);
	    	gdk_draw_line(pixmap, pInfo.gc, pix_x0, pix_y0, pix_x1, pix_y1);
			pInfo.decInc = 0;
		}
	
	    if (pInfo.inc > 1) 
		{	// draw the points for the last increment
	    	DATAP(pInfo.dataType, (pInfo.ptInc+pInfo.decInc), fmin);
			fmax = fmin;
			for (fpt = fmin;
					pInfo.decInc + pInfo.ptInc < overlap->len;
					pInfo.decInc++)
			{
				DATAP(pInfo.dataType, (pInfo.decInc), fpt);
		    	if (fpt < fmin)
					fmin = fpt;
		    	if (fpt > fmax)
					fmax = fpt;
			}
			y1 = USER2PIXY(gr, (double) fmin);
			y2 = USER2PIXY(gr, (double) fmax);
			gdk_draw_line(pixmap, pInfo.gc, pix_x1, y1, pix_x1, y2);
	    }		
	}
}

void magnify_proc(GtkWidget *da, GdkPixmap *pixmap, int wind_width, int wind_height, gboolean init)
{
// this procedure sets up the variables necessary for plotting the magnification
// of the selected traces from the main screen.

	graph	*locGR = &states.trace.GR[states.currentScreen];

	if (!locGR->pix_xmax)	// can't draw anything until the trace screen has been drawn
		return;
		
	if (!states.magnify.limits.selection.user.left && 
		!states.magnify.limits.selection.user.right)
	{	// if magnify request originates from trace screen
    	states.magnify.limits.selection.user.left = 
    			PIX2USERX(locGR, states.trace.limits.selection.pix[states.currentScreen].left);
    	if (states.trace.limits.selection.pix[states.currentScreen].right == 0) 
		{
        	states.magnify.limits.selection.user.right = 
							states.magnify.limits.selection.user.left + settings.magnify.length;
			if (states.magnify.limits.selection.user.right > states.trace.plot.length[settings.trace.time])
	    		states.magnify.limits.selection.user.right = states.trace.plot.length[settings.trace.time];

			states.trace.limits.selection.pix[states.currentScreen].right = 
							USER2PIXX(locGR, states.magnify.limits.selection.user.right);
    	}
    	else
			states.magnify.limits.selection.user.right = 
							PIX2USERX(locGR, states.trace.limits.selection.pix[states.currentScreen].right);
		init = TRUE;
	}
	else
	{	// if magnify request originates from magnify screen
		// reverse engineer the user coords back to pixel boundary values for TRACE screen
		if (!states.user.drag.xAxis)
		{	// don't do this if dragging
			locGR = &states.trace.GR[states.currentScreen];
			states.trace.limits.selection.pix[states.currentScreen].left = 
				USER2PIXX(locGR, states.magnify.limits.selection.user.left);
			states.trace.limits.selection.pix[states.currentScreen].right = 
				USER2PIXX(locGR, states.magnify.limits.selection.user.right);
		}
	}

    states.magnify.limits.selection.user.length = 
    			states.magnify.limits.selection.user.right - 
				states.magnify.limits.selection.user.left;
   	states.trace.limits.selection.pix[states.currentScreen].length = 
   				states.trace.limits.selection.pix[states.currentScreen].right - 
				states.trace.limits.selection.pix[states.currentScreen].left;
	states.magnify.limits.selection.pix.left = states.magnify.limits.selection.pix.right = 0;

	memset(&states.magnify.GR[states.currentScreen], 0, sizeof(states.magnify.GR[states.currentScreen]));

    get_mag_times();
    get_mag_max_mins(init);

	mag_plot_files(da, pixmap, wind_width, wind_height, -1);
	return;
}
