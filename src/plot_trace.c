#include <string.h>
#include <sys/time.h>
#include <stdlib.h>
#include "pql_defines.h"
#include "pql_externs.h"
#include "gtk_pql_externs.h"

#define PIXELSPTRC 14

double ceil();
static void plot(GdkPixmap *pixmap, graph *gr, traceInfo *, double begin, double offset, int inc);
static void plotOlap(GdkPixmap *pixmap, graph *gr, traceInfo *, double begin, double offset, int inc);

static gboolean splitPlot;

static DATAINFO *setDataPtr(traceInfo *trcData)
{
	DATAINFO *dataPtr;
	dataPtr = &trcData->data.dataInfo;
	if (splitPlot &&					// only on SPLIT
		states.tForm.on && 				// only if transform is ON
		trcData->data.tForm.data &&		// only if there is transform data
		settings.split.traceDisp == TFORMANA)		// only if specified by user to be displayed
	{
		dataPtr = &trcData->data.tForm;
	}
	return dataPtr;
}

int get_data_increment(traceInfo *trcData, double secs, int pixwidth)
{
    float           step;
    double          inc;
    
	// determine the decimation increment of a record to be plotted,
	// given the sampling rate, time interval and window width
    step = (float) (secs / (float) pixwidth) / ((float) trcData->data.head.sampInt / (float) 1000000);

    inc = ceil(step);
    if (inc <= 1.)
		return (1);
    else
		return ((int) inc);
}

void plot_files(GtkWidget *da, GdkPixmap *pixmap, int pWidth, int pHeight, gboolean splitP) 
{
    int		i, fontH, x1;
    float   scale;
	int		label_width, h1;
	DATAINFO	*dataInfo=NULL;
    char	axis_label[80], *msg;
    int		axis_label_y_pos=(int)(pHeight-20), str_width, scr, labelW;
    GSList	*trcIter, *trcList;
    traceInfo *trcData=NULL;
	float	gainFactor;

	if (pHeight == 1)
		return;
		
	splitPlot = splitP;
	settings.general.font.DA = da;
	settings.general.font.font = LABELF;
	trcList = splitPlot
				? states.trace.selection
				: states.trace.traces[CURGRP];

   	states.trace.trace_depth[states.currentScreen] = (float) (pHeight - TRC_B_MARGIN) / 
   														(float) (g_slist_length(trcList));

	if (settings.general.display.mode == GATHERMODE &&
		states.trace.trace_depth[states.currentScreen] < PIXELSPTRC &&
		settings.general.display.gather.gain[TRACESCR] == 1.0 &&
		!splitPlot)
	{
		float gainDisp = (float) PIXELSPTRC/states.trace.trace_depth[states.currentScreen];
		char *gainDispS = g_strdup_printf("%.2f", gainDisp);
		free(settings.general.display.gather.gainS[TRACESCR]);
		gtk_entry_set_text(GTK_ENTRY(dispGain[TRACESCR]), gainDispS);
		settings.general.display.gather.gainS[TRACESCR] = gainDispS;
		settings.general.display.gather.gain[TRACESCR] = gainDisp;
		setDispGain(TRACESCR, MAGNIFYSCR);
	}
	gainFactor = (settings.general.display.mode == GATHERMODE &&
					  settings.general.display.gather.gain[TRACESCR] > 0)
						? settings.general.display.gather.gain[TRACESCR]
						: 1.0;

	// find the widest label we need to accommodate
	if (!states.trace.plot.label_width[states.currentScreen])
	{
		for(trcIter = states.trace.traces[CURGRP], label_width=0;
			trcIter;
			trcIter = g_slist_next(trcIter))
		{
			trcData = trcIter->data;
	  		str_width = stringWidth(trcData, states.trace.trace_depth[states.currentScreen], LABEL);
			if (str_width > label_width)
				label_width = str_width;
		}
		states.trace.plot.label_width[states.currentScreen] = 	settings.general.label.length 
																? settings.general.label.length 
																: label_width;
	}
	label_width = states.trace.plot.label_width[states.currentScreen];
	fontH = fontSIZE("9", HEIGHT);
	
	// set up some numbers for subsequent plotting
	for(trcIter = trcList, i=0;
		trcIter;
		trcIter = g_slist_next(trcIter))
	{
		trcData = trcIter->data;
		dataInfo = setDataPtr(trcData);

		// set up the start and end times
		switch (settings.trace.time) 
		{
			case RELATIVE:
				trcData->trace.plot.startTime = trcData->data.head.epochs;
				trcData->trace.plot.endTime1 = trcData->trace.plot.startTime + 
												states.trace.relTimeLen;	// end of plot
				trcData->trace.plot.offset = 0;
			break;
			case ABSOLUTE:
				trcData->trace.plot.startTime = states.trace.absTime.start;
				trcData->trace.plot.endTime1 = states.trace.absTime.end;	// end of plot
				trcData->trace.plot.offset = trcData->data.head.epochs - states.trace.absTime.start;
			break;
		}

		// get the actual max and mins we'll be using for the plot
		switch (settings.trace.amp) 
		{
			case WSCALETRC:
				trcData->trace.plot.minAmp = dataInfo->amps.min;
				trcData->trace.plot.maxAmp = dataInfo->amps.max;
			break;
			case WSCALEWIND:
				switch (trcData->data.head.units) 
				{
					case PQL_UNITS_VOLTS:
						scale = trcData->data.head.scale_fac / trcData->data.head.gainConst;
			
						trcData->trace.plot.minAmp = dataInfo->amps.min * scale - 
									(states.trace.absAmp.diffVolts - (dataInfo->amps.max -
									dataInfo->amps.min)*scale)/2;
						trcData->trace.plot.maxAmp = dataInfo->amps.max * scale + 
									(states.trace.absAmp.diffVolts - 
										(dataInfo->amps.max - dataInfo->amps.min)*scale)/2;
						trcData->trace.plot.minAmp /= scale;
						trcData->trace.plot.maxAmp /= scale;	//  convert back to counts
					break;
					default:
						trcData->trace.plot.minAmp = dataInfo->amps.min - 
									(states.trace.absAmp.diff - 
										(dataInfo->amps.max - dataInfo->amps.min))/2;
						trcData->trace.plot.maxAmp = dataInfo->amps.max + 
									(states.trace.absAmp.diff - 
										(dataInfo->amps.max - dataInfo->amps.min))/2;
					break;
				}
			break;
		}
	}

	for(trcIter = trcList, i=0;
		trcIter;
		trcIter = g_slist_next(trcIter), i++)
	{
		double	max, min;
		
		trcData = trcIter->data;
		dataInfo = setDataPtr(trcData);
		if (!dataInfo->data)
			continue;

		viewport(&trcData->trace.GR[states.currentScreen], 
					label_width+PQLLABELXTRA, pWidth - PQLINCXTRA, 
					i * states.trace.trace_depth[states.currentScreen] + 1, 
					(i + 1) * states.trace.trace_depth[states.currentScreen] - 1);

		max = trcData->trace.plot.maxAmp;
		min = trcData->trace.plot.minAmp;
		if (gainFactor != 1.0)
		{
			double mid, diffH;
			diffH = (max - min)/2.0;
			mid = max - diffH;
			max = mid + diffH/(gainFactor);
			min = mid - diffH/(gainFactor);
		}
		window(&trcData->trace.GR[states.currentScreen], 
						(double) (trcData->trace.plot.startTime), 
						(double) (trcData->trace.plot.endTime1),
						(double) (min), 
						(double) (max), 0, 0);

		trcData->trace.plot.increment = get_data_increment(trcData, 
					trcData->trace.plot.endTime1 - trcData->trace.plot.startTime,
		    		USER2PIXX(&trcData->trace.GR[states.currentScreen], 
		    						trcData->trace.plot.endTime1) - 
					USER2PIXX(&trcData->trace.GR[states.currentScreen], 
									trcData->trace.plot.startTime));
	
	}
	
	// and do the plotting
	for(trcIter = trcList, i=0;
		trcIter;
		trcIter = g_slist_next(trcIter), i++)
	{
		trcData = trcIter->data;
		scr = splitPlot ? SPLITSCR : TRACESCR;
		if (settings.general.display.mode == TRACEMODE)
		{
			printLabel(pixmap, trcData, i, states.trace.trace_depth[states.currentScreen], 
						pWidth - PQLINCXTRA, scr);
		}
		else
		{
			static int last = 0;
			int	middle = (states.trace.trace_depth[states.currentScreen]*i) +
							states.trace.trace_depth[states.currentScreen]/2;
			if (i==0)
				last = 0;
			if (middle - fontH > last)
			{
				printLabel(pixmap, trcData, i, states.trace.trace_depth[states.currentScreen], 
							pWidth - PQLINCXTRA, scr);
				last = middle + fontH/2;
			}
		}

		if (!trcData->data.dataInfo.data)
		{
			msg = g_strdup("CWB returned NO Data - see stderr for details");
			h1 = i*states.trace.trace_depth[states.currentScreen] + 
					states.trace.trace_depth[states.currentScreen]/2;
			fontDraw(pixmap, msg, label_width+PQLLABELXTRA, h1, NORMALGC, SAMESCR, -1, -1);
			free(msg);
			continue;					// we might have empty traces, don't plot nothing!
		}

		plot(pixmap, &trcData->trace.GR[states.currentScreen], trcData, 
						trcData->trace.plot.startTime, 
						trcData->trace.plot.offset, 
						trcData->trace.plot.increment);

		if (!states.tForm.on  &&		// plot 2ndary overlap only when data not transformed
			trcData->trace.olapData)	// and when olap data actually exists to plot
			plotOlap(pixmap, &trcData->trace.GR[states.currentScreen], trcData, 
						trcData->trace.plot.startTime, 
						trcData->trace.plot.offset, 
						trcData->trace.plot.increment);
   }

	if (settings.general.display.mode == TRACEMODE &&
		splitPlot == FALSE)
	{
		switch(i)
		{
			case 1:
				sprintf(axis_label, "%d / %d", states.trace.startRec+1, 
										states.general.numTraces);
			break;
			default:
				sprintf(axis_label, "%d-%d / %d", states.trace.startRec+1, 
										states.trace.startRec+i,
										states.general.numTraces);
			break;
		}
		fontDraw(pixmap, axis_label, 5, axis_label_y_pos-15, NORMALGC, SAMESCR, -1, -1);
	}

    states.trace.limits.margin[states.currentScreen].left = 
					USER2PIXX(&trcData->trace.GR[states.currentScreen], 
									(double) trcData->trace.plot.startTime);

    states.trace.limits.margin[states.currentScreen].right = 
					USER2PIXX(&trcData->trace.GR[states.currentScreen], 
									(double) trcData->trace.plot.endTime1);

    states.trace.limits.margin[states.currentScreen].length =	
							states.trace.limits.margin[states.currentScreen].right - 
							states.trace.limits.margin[states.currentScreen].left;

    states.trace.plot.length[settings.trace.time] = trcData->trace.plot.endTime1 - 
													trcData->trace.plot.startTime;

    viewport(&states.trace.GR[states.currentScreen], 
			states.trace.limits.margin[states.currentScreen].left, 
			states.trace.limits.margin[states.currentScreen].right, 
			(int) (pHeight - TRC_B_MARGIN), (int) pHeight);
    window(&states.trace.GR[states.currentScreen], 
			(double) 0, trcData->trace.plot.endTime1 - trcData->trace.plot.startTime, 
			(double) TRC_B_MARGIN, (double) 0, 0, 0);

	settings.general.font.font = AXISF;
	switch(settings.trace.time)
	{
		case RELATIVE:
			sprintf(axis_label,"Seconds");
			labelW = fontSIZE(axis_label, WIDTH);
			x1 = ( states.trace.limits.margin[states.currentScreen].left < labelW)
					? 0
					:  states.trace.limits.margin[states.currentScreen].left - labelW;
			fontDraw(pixmap, axis_label, x1, axis_label_y_pos, NORMALGC, SAMESCR, -1, -1);
		    xaxis(pixmap, GXC[NORMALGC][TRACESCR], 
				&states.trace.GR[states.currentScreen], 
				(double) 0.0, (double) 9, 
				(double) states.trace.plot.length[settings.trace.time], 1, -1, -1, 
				(double) 0.0, FALSE, (double) 0);
		break;

		case ABSOLUTE:
			xAxisABS(pixmap, GXC[NORMALGC][TRACESCR], 
							&states.trace.GR[states.currentScreen], 
							states.trace.absTime.start, TRACESCR, LABEL_BELOW);
		break;
	}
	
	if (states.currentScreen != MAIN &&
		!states.trace.limits.selection.pix[states.currentScreen].left)
	{	// set pixel bounds first time called for split
		if (states.magnify.limits.selection.user.left)
		{	// take from mag screen definition
			states.trace.limits.selection.pix[states.currentScreen].left =
				USER2PIXX(&states.trace.GR[states.currentScreen],
					states.magnify.limits.selection.user.left);
			states.trace.limits.selection.pix[states.currentScreen].right =
				USER2PIXX(&states.trace.GR[states.currentScreen],
					states.magnify.limits.selection.user.right);
		}
		else if (states.trace.limits.selection.pix[MAIN].left)
		{	// mag not yet defined, take from trace boundaries
			states.trace.limits.selection.pix[states.currentScreen].left =
				USER2PIXX(&states.trace.GR[states.currentScreen],
					PIX2USERX(&states.trace.GR[MAIN], states.trace.limits.selection.pix[MAIN].left));
			states.trace.limits.selection.pix[states.currentScreen].right =
				USER2PIXX(&states.trace.GR[states.currentScreen],
					PIX2USERX(&states.trace.GR[MAIN], states.trace.limits.selection.pix[MAIN].right));
		}
	}
}

#define FLAGMARGIN	2
#define	FLAGFACTOR	12
#define MAXFLAG		10
#define MINFLAG		5

typedef struct
{
	traceInfo *trcData;
	int		stop, dataSize, dataType;
	int		/*rec_no,*/ ptInc, decInc, inc;
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
		gboolean	gapON, olapON;
	} GO;
	GdkGC		*gc;
} plotInfo;

#include <stdlib.h>

static	int	*vertical, flagSize, totVert;

static void	checkGO(GdkPixmap *pixmap, graph *gr, plotInfo *pInfo, int x, int y)
{
	GdkPoint	flag[3];
	static int	vertPos, gapIter;
	int			gapInc;
	int			pix_x0, pix_y0, pix_x1=0, pix_y1, decInc;
	double		pt1=0, pt2=0;
	gboolean	gap = FALSE;
	traceInfo	*trcData = pInfo->trcData;

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
	{	// gaps left to check?  
		if (settings.trace.gFlag)
		{	// undone: set up start variables for case when beyond first gap in same vertical pixel
			gapIter = 0;
			vertPos=0;
			while(x < vertical[vertPos])
				vertPos++;
		}

		if (pInfo->ptInc+pInfo->decInc >= pInfo->GO.gaps[GAPBEG])
		{	// hit a gap, process:
			gap = TRUE;
			pInfo->gc = GGXC[TRACESCR][0];
			if (settings.trace.gFlag)	// && !gapON)
			{	
				pix_x0 = USER2PIXX(gr, pInfo->tm);
				if(!gapIter)	// draw vertical line only once at a single X pixel
					gdk_draw_line(pixmap, pInfo->gc,
						pix_x0, gr->pix_ymin + vertPos*(flagSize+1) + FLAGMARGIN, 
						pix_x0, y);
				flag[0].x = pix_x0;
				flag[0].y = gr->pix_ymin + vertPos*(flagSize+1) + gapIter*(flagSize+1) + FLAGMARGIN;
				flag[1].x = pix_x0;
				flag[1].y = gr->pix_ymin + vertPos*(flagSize+1) + gapIter*(flagSize+1) + flagSize + FLAGMARGIN;
				flag[2].x = pix_x0 + flagSize;
				flag[2].y = gr->pix_ymin + vertPos*(flagSize+1) + gapIter*(flagSize+1) + flagSize/2 + FLAGMARGIN;
				gdk_draw_polygon(pixmap, pInfo->gc, TRUE, flag, 3);
				if (vertPos+gapIter>=totVert)
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
    			switch (pInfo->dataType)
				{
					case BIT32:
						pt1 = (double) pInfo->data.ptr[pInfo->GO.gaps[GAPBEG]-1];
						pt2 = (double) pInfo->data.ptr[pInfo->GO.gaps[GAPEND]];
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

				pInfo->ptInc += (pInfo->inc * (gapInc));
	    		pInfo->tm += (pInfo->tm_inc * (double) (gapInc));

	    		pix_x1 = USER2PIXX(gr, pInfo->tm);
	    		pix_y1 = USER2PIXY(gr, pt2);

	    		gdk_draw_line(pixmap, pInfo->gc, pix_x0, pix_y0, pix_x1, pix_y1);
			}

			pInfo->GO.gapNum++;
			pInfo->gc = GXC[NORMALGC][TRACESCR];
			decInc = pInfo->GO.gaps[GAPBEG-1] - pInfo->ptInc;
			pInfo->decInc = decInc;				// this value reset only on gaps
		}
#if 0
		else
		{	// no gap
//			decInc = 0;
		}
#endif
	}		// end if (gapNum < trcData->trace.numGaps)

	if (pInfo->GO.olapNum < trcData->trace.numOlaps)
	{	// overlaps to check?
		if (settings.trace.oFlag && !pInfo->GO.olapON)
		{
			gapIter = 0;
			vertPos=0;
			while(x < vertical[vertPos] && vertPos<totVert)
				vertPos++;
		}

		if (!pInfo->GO.olapON && pInfo->ptInc+pInfo->decInc >= pInfo->GO.olaps[OLAPBEG])
		{	// overlap found
			pInfo->gc = PGXC[TRACESCR][0];
			if (settings.trace.oFlag) 	// && !pInfo->GO.olapON)
			{
				if (gap)
					x = pix_x1;
				if(!gapIter)	// draw vertical line only once at a single X pixel
					gdk_draw_line(pixmap, pInfo->gc, x, gr->pix_ymin + vertPos*(flagSize+1) + FLAGMARGIN, x, y);
				flag[0].x = x;
				flag[0].y = gr->pix_ymin + vertPos*(flagSize+1) + gapIter*(flagSize+1) + FLAGMARGIN;
				flag[1].x = x;
				flag[1].y = gr->pix_ymin+ + vertPos*(flagSize+1) + gapIter*(flagSize+1) + flagSize + FLAGMARGIN;
				flag[2].x = x + flagSize;
				flag[2].y = gr->pix_ymin+ vertPos*(flagSize+1) + gapIter*(flagSize+1) + flagSize/2 + FLAGMARGIN;
				gdk_draw_polygon(pixmap, pInfo->gc, TRUE, flag, 3);
				if (vertPos+gapIter>=totVert)
				{
					totVert += 50;
					vertical = (int *) realloc(vertical, totVert*sizeof(int));
				}
				vertical[vertPos+gapIter] = flag[2].x + 1;
				gapIter++;
			}	// end if
			pInfo->GO.olapON = TRUE;
		}	// end if

		if (pInfo->GO.olapON && 
			pInfo->ptInc+pInfo->decInc >= pInfo->GO.olaps[OLAPEND])
		{	// overlap stop, return to current GC
			pInfo->GO.olapNum++;
			pInfo->gc = GXC[NORMALGC][TRACESCR];
			pInfo->GO.olapON = FALSE;
		}
	}		// end if(trace.overlaps)

	pInfo->stop = trcData->data.dataInfo.head.numSamples;
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

}

static void initData(plotInfo *plotI)
{
	traceInfo	*trcData = plotI->trcData;
	DATAINFO	*dataInfo = setDataPtr(trcData);
	
	flagSize = 0;
	vertical = NULL;

	plotI->dataType = dataInfo->head.data_form;
	plotI->dataSize = dsizes[plotI->dataType];
	switch (plotI->dataType) 
	{
		case BIT32:
			plotI->data.ptr = (gint32 *) dataInfo->data;
		break;

		case BIT16:
			plotI->data.sptr = (short *) dataInfo->data;
		break;

		case MY_FLOAT:
			plotI->data.lptr = (float *) dataInfo->data;
		break;

		case MY_DOUBLE:
			plotI->data.dptr = (double *) dataInfo->data;
		break;
	}

	plotI->GO.gaps = trcData->trace.gaps;
	plotI->GO.olaps = trcData->trace.overlaps;
    plotI->tm_inc = (double) (((trcData->data.head.sampInt) / 1000000.) * ((float) plotI->inc));

	plotI->gc = GXC[NORMALGC][TRACESCR];
	plotI->GO.gapON = plotI->GO.olapON = FALSE;
	plotI->GO.olapNum = plotI->GO.gapNum = 0;
	plotI->decInc = 0;
	plotI->ptInc = 0;

	plotI->stop = dataInfo->head.numSamples;					// default to end of trace
	if (plotI->GO.gapNum < trcData->trace.numGaps)
	{
		plotI->stop = plotI->GO.gaps[0];						// start of first gap
	}

	if (plotI->GO.olapNum < trcData->trace.numOlaps) 
	{
		if (plotI->GO.gaps)
		{	// check if gap is stopping first
			if (plotI->GO.olaps[0] < plotI->GO.gaps[0])
			{
				plotI->stop = plotI->GO.olaps[0];				// start of first overlap && before first gap
			}
		}
		else
		{
			plotI->stop = plotI->GO.olaps[0];					// start of first overlap, no gaps to consider
		}
	}

}

static void plot(GdkPixmap *pixmap, graph *gr, traceInfo *trcData, double begin, double offset, int inc)
{
    int     y1, y2, pix_x0, pix_x1, pix_y0, pix_y1;
    double  fmax, fmin, fpt, sps, tmStart;
	short	*sptr;
	gint32	*ptr;
	float	*lptr;
	double	*dptr;
	plotInfo	pInfo;
	
	memset(&pInfo, 0, sizeof(plotInfo));
	pInfo.trcData = trcData;
	pInfo.inc = inc;
	tmStart = begin + offset;
    pInfo.tm = tmStart;
	initData(&pInfo);
	sps = 1000000. / (double) trcData->data.head.sampInt;
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

	pix_x0 = pix_x1 = USER2PIXX(gr, pInfo.tm);
	DATAP(pInfo.dataType, pInfo.ptInc, fmin);
	pix_y0 = pix_y1 =  USER2PIXY(gr, (double) fmin);

	for (
			;pInfo.ptInc < trcData->data.dataInfo.head.numSamples - pInfo.inc; 
			pix_x0 = pix_x1, pix_y0 = pix_y1)
	{
	    if (pInfo.inc > 1) 
		{
			DATAP(pInfo.dataType, (pInfo.ptInc+pInfo.decInc), fmin);
			fmax = fmin;
			for (fpt = fmax; 									// first data point for this x-pixel
					(pInfo.decInc < pInfo.inc) && 				// loop until we've hit the end of the decimation
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
			y1 = USER2PIXY(gr, (double) fmin);
			y2 = USER2PIXY(gr, (double) fmax);
			gdk_draw_line(pixmap, pInfo.gc, pix_x1, y1, pix_x1, y2);
	    }

		if (pInfo.ptInc+pInfo.decInc == pInfo.stop)
		{	// hit a gap or overlap
			int	lastStop = pInfo.stop;
			checkGO(pixmap, gr, &pInfo, pix_x1, pix_y1);
			while (pInfo.stop <= lastStop)
			{	// handle overlaps of overlaps
		    	pix_x1 = USER2PIXX(gr, (double) (tmStart + (double) (pInfo.stop/sps)));
				checkGO(pixmap, gr, &pInfo, pix_x1, pix_y1);
			}
	    	pix_x1 = USER2PIXX(gr, pInfo.tm);
	    	DATAP(pInfo.dataType, (pInfo.ptInc+pInfo.decInc), fmin);
	    	pix_y1 = USER2PIXY(gr, (double) fmin);
		}
		else
		{	// no gap, proceed to next decimation boundary
			pInfo.ptInc += pInfo.inc;
		    pInfo.tm += pInfo.tm_inc;
	    	pix_x1 = USER2PIXX(gr, pInfo.tm);
	    	DATAP(pInfo.dataType, (pInfo.ptInc), fmin);
	    	pix_y1 = USER2PIXY(gr, (double) fmin);
	    	gdk_draw_line(pixmap, pInfo.gc, pix_x0, pix_y0, pix_x1, pix_y1);
			pInfo.decInc = 0;
		}
	}

    if (pInfo.inc > 1)
	{	// draw the points for the last increment
    	DATAP(pInfo.dataType, (pInfo.ptInc+pInfo.decInc), fmin);
		fmax = fmin;
		for (fpt = fmin;
				pInfo.decInc + pInfo.ptInc < trcData->data.dataInfo.head.numSamples;
				pInfo.decInc++)
		{
			DATAP(pInfo.dataType, (pInfo.ptInc+pInfo.decInc), fpt);
	    	if (fpt < fmin)
				fmin = fpt;
	    	if (fpt > fmax)
				fmax = fpt;
		}
		y1 = USER2PIXY(gr, (double) fmin);
		y2 = USER2PIXY(gr, (double) fmax);
		gdk_draw_line(pixmap, pInfo.gc, pix_x1, y1, pix_x1, y2);
    }		

	if(vertical)
		free(vertical);
}

static void plotOlap(GdkPixmap *pixmap, graph *gr, traceInfo *trcData, double begin, double offset, int inc)
{
    int     y1, y2, pix_x0, pix_x1, pix_y0, pix_y1;
    double  fmax, fmin, fpt, sps;
	short	*sptr;
	gint32	*ptr;
	float	*lptr;
	double	*dptr;
	plotInfo	pInfo;
	GSList	*iter;
	OVERLAP	*overlap;
	
	memset(&pInfo, 0, sizeof(plotInfo));
	pInfo.trcData = trcData;
	pInfo.inc = inc;
	pInfo.gc = PGXC[TRACESCR][1];		// 2ndary overlap colour
    pInfo.tm_inc = (double) (((trcData->data.head.sampInt) / 1000000.) * ((float) pInfo.inc));
	sps = 1000000. / (double) trcData->data.head.sampInt;
	pInfo.dataType = trcData->data.head.data_form;
	pInfo.dataSize = dsizes[pInfo.dataType];
	
	for(iter=trcData->trace.olapData;
		iter;
		iter=g_slist_next(iter))
	{	// loop over all overlaps
		overlap = iter->data;
	    pInfo.tm = begin + offset + (double) (overlap->bounds[START]/sps);
		pInfo.ptInc = 0;
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
	
		for (	pInfo.decInc = overlap->bounds[START]%pInfo.inc+1;	// start pt inside our decimation - 1st time
				pInfo.ptInc < overlap->len - pInfo.inc; 
				pix_x0 = pix_x1, pix_y0 = pix_y1)
		{
		    if (pInfo.inc > 1) 
			{
				DATAP(pInfo.dataType, (pInfo.ptInc+pInfo.decInc), fmin);
				fmax = fmin;
				for (	;
						(pInfo.decInc < pInfo.inc) &&	// loop until we've hit the end of the decimation
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
	
	    if (pInfo.inc > 1 &&
			pInfo.decInc == 0)		// if pInfo.decInc != 0 then we're already done
		{	// draw the points for the last increment
	    	DATAP(pInfo.dataType, (pInfo.ptInc+pInfo.decInc), fmin);
			fmax = fmin;
			for (	;
					pInfo.decInc + pInfo.ptInc < overlap->len;
					pInfo.decInc++)
			{
				DATAP(pInfo.dataType, (pInfo.ptInc+pInfo.decInc), fpt);
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
