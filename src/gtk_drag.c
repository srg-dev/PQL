#include "pql_defines.h"
#include "pql_externs.h"
#include "gtk_pql_externs.h"

#include <math.h>
#include <string.h>

#define IN 0
#define OUT 1

extern int      mag_x, mag_y;
extern int      max_jump;   
extern int      min_jump;
extern double   mag_mouse_time;
extern double   mag_mouse_amp;


void traceDrag(GtkWidget *da, GdkEventMotion *event, GdkPixmap *pixmap)
{
	int win_rec; 
	static int lastRec=-1;
	traceInfo *trcData;

	if (event->x < states.trace.limits.margin[states.currentScreen].left)	//right)
	{		// dragging to select traces only allowed within label region	
			states.user.event = SELECTTR;		
	}

	switch(states.user.keyActive)
	{
		case NONE:	// select multiple traces event
			switch(states.user.event)
			{
				case SELECTTR:
					if (states.currentScreen != MAIN)
					{
						break;			// only available on MAIN screen
					}
		
					win_rec = (int) (event->y) / states.trace.trace_depth[MAIN];
					if (lastRec == win_rec || 
						 (win_rec + states.trace.startRec) >= states.general.numTraces)
						break;			// don't process already processed or non-existant trace
		
					lastRec = win_rec;
					trcData = g_slist_nth_data(traces, win_rec + states.trace.startRec);
					trcData->trace.selected = !trcData->trace.selected;
					settings.general.font.DA = da;
					printLabel(pixmap, trcData, win_rec, 
								states.trace.trace_depth[states.currentScreen], 0, TRACESCR);
					gtk_widget_queue_draw_area(da, 4, 
									states.trace.trace_depth[states.currentScreen]*win_rec,
									states.trace.limits.margin[MAIN].left,
									states.trace.limits.margin[MAIN].right);
		
					break;
			}				// end switch(states.user.event)
			break;

		case CTRL:		// scan data points event
			// if we breach the edges, hardcode edge point
      		if (event->x < states.trace.limits.margin[states.currentScreen].left)
			{
      			event->x = states.trace.limits.margin[states.currentScreen].left;
			}
			if (event->x > states.trace.limits.margin[states.currentScreen].right)
			{
				event->x = states.trace.limits.margin[states.currentScreen].right;
			}
      		// process only if within valid section of screen
			if (event->y >= da->allocation.height - 24)
			{
				states.msg = g_strdup("Selection not within TRACE plot");
				break;
			}

			win_rec = (event->y) / states.trace.trace_depth[states.currentScreen];

			tracePoint( win_rec, (int) event->x, states.user.pointPtr);
	
			break;			
	}						// end switch(user.mouseLeft)

	return;
}

void magDrag(GtkWidget *da, GdkEventMotion *event, GdkPixmap *pixmap)
{
	int 		zoomDir, win_rec, scrollDir;
	int			min;
	double		diff, diffL=0, diffR=0;
	static double zAnchor;
	traceInfo	*trcData;
	
	switch(states.user.keyActive)
	{
		case NONE:
			if (event->x <= states.magnify.limits.margin[states.currentScreen].right && 
				states.user.event == VLINE) 	// set this only the first time we're called for each drag
			{	// click inside plot margin boundaries?
				if (event->y < MAG_T_MARGIN ||
					states.user.drag.xAxis)
					states.user.event = XAXIS;
				else if (event->y < da->allocation.height - MAG_B_MARGIN)
				{
					states.user.event = ZOOM;
					trcData = states.trace.selection->data;
					zAnchor = PIX2USERX(&trcData->magnify.GR[states.currentScreen], (int) event->x);
				}
			}

			switch(states.user.event)
			{
				case ZOOM:
					if (event->x > states.user.mouseLoc.startX)
					{	// we're zooming IN
						min = getMinDisp();
						if (min < 3)
						{
							gdk_beep();
							dispMsg(DISPLAY, "Cannot Further ZOOM");
							break;
						}
						zoomDir = 1;
					}
					else	// or we're zooming out
						zoomDir = -1;
		
					diff = (states.magnify.limits.selection.user.right - 
							states.magnify.limits.selection.user.left);
					switch(settings.magnify.anchor)
					{
						case LEFT:
						case RIGHT:
						case MIDDLE:
							diffL = (diff / (100/settings.magnify.zoomFactor)) * 
									factors[settings.magnify.anchor*2] * zoomDir;
							diffR = (diff / (100/settings.magnify.zoomFactor)) * 
									factors[settings.magnify.anchor*2+1] * zoomDir * -1;
						break;

						case MOUSE:
							diffL = (diff / (100/settings.magnify.zoomFactor)) *
									((zAnchor-states.magnify.limits.selection.user.left)/(diff/2.)) * zoomDir;
							diffR = (diff / (100/settings.magnify.zoomFactor)) *
									((states.magnify.limits.selection.user.right-zAnchor)/(diff/2.)) * zoomDir * -1;
						break;
					}
					states.magnify.limits.selection.user.left += diffL;
					states.magnify.limits.selection.user.right += diffR;
					states.user.mouseLoc.startX = event->x;
		
					// handle breeching of absolute bounds
					if (states.magnify.limits.selection.user.left < 
							states.trace.GR[states.currentScreen].user_xmin)
					{
						states.magnify.limits.selection.user.left = 
							states.trace.GR[states.currentScreen].user_xmin;
					}
					if (states.magnify.limits.selection.user.right > 
							states.trace.GR[states.currentScreen].user_xmax)
					{
						states.magnify.limits.selection.user.right = 
							states.trace.GR[states.currentScreen].user_xmax;
					}
				
					states.reset.reset[MAGNIFYSCR] = TRUE;
					states.reset.reset[SPECTRASCR] = TRUE;
					states.split.refreshPixmap[SPLIT2][MAGNIFYSCR] = TRUE;
					states.split.refreshPixmap[SPLIT2][SPECTRASCR] = TRUE;
					states.split.refreshPixmap[SPLIT3][MAGNIFYSCR] = TRUE;
					states.split.refreshPixmap[SPLIT3][SPECTRASCR] = TRUE;
				break;

				case XAXIS:
				{
					int x1, x2, diff;
					x1 = USER2PIXX(&states.magnify.mainGR[states.currentScreen], 
									states.magnify.limits.selection.user.left); 
					x2 = USER2PIXX(&states.magnify.mainGR[states.currentScreen], 
									states.magnify.limits.selection.user.right);
					if ((event->x < x1 ||	
						event->x > x2) &&			// can't drag unless within selection box
						!states.user.drag.xAxis)	// unless we've already started dragging
						break;

					diff = x2-x1;
					x1 = event->x - diff/2;
					x2 = x1 + diff;
					
					states.user.drag.xAxis = TRUE;
					states.user.mouseLoc.startX = event->x;
					
	    			states.magnify.limits.selection.user.left = 
	    					PIX2USERX(&states.magnify.mainGR[states.currentScreen], x1);
	    			states.magnify.limits.selection.user.right = 
	    					PIX2USERX(&states.magnify.mainGR[states.currentScreen], x2);

					states.trace.limits.selection.pix[states.currentScreen].left = 
							USER2PIXX(&states.trace.GR[states.currentScreen], 
											states.magnify.limits.selection.user.left);
					states.trace.limits.selection.pix[states.currentScreen].right = 
							USER2PIXX(&states.trace.GR[states.currentScreen], 
											states.magnify.limits.selection.user.right);

					if (states.trace.limits.selection.pix[states.currentScreen].left < 
							states.trace.GR[states.currentScreen].pix_xmin)
					{
						states.trace.limits.selection.pix[states.currentScreen].left = 
								states.trace.GR[states.currentScreen].pix_xmin;
						states.trace.limits.selection.pix[states.currentScreen].right = 
								states.trace.limits.selection.pix[states.currentScreen].left + 
								states.trace.limits.selection.pix[states.currentScreen].length;
					}
					if (states.trace.limits.selection.pix[states.currentScreen].right > 
							states.trace.GR[states.currentScreen].pix_xmax)
					{
						states.trace.limits.selection.pix[states.currentScreen].right = 
								states.trace.GR[states.currentScreen].pix_xmax;
						states.trace.limits.selection.pix[states.currentScreen].left = 
								states.trace.limits.selection.pix[states.currentScreen].right - 								states.trace.limits.selection.pix[states.currentScreen].length;
					}

	    			states.magnify.limits.selection.user.left = 
	    						PIX2USERX(&states.trace.GR[states.currentScreen], 
												states.trace.limits.selection.pix[states.currentScreen].left);
					states.magnify.limits.selection.user.right = 
								PIX2USERX(&states.trace.GR[states.currentScreen], 
												states.trace.limits.selection.pix[states.currentScreen].right);

					states.reset.reset[MAGNIFYSCR] = 
							states.reset.reset[SPECTRASCR] = 
							states.split.refreshPixmap[SPLIT2][MAGNIFYSCR] = 
							states.split.refreshPixmap[SPLIT3][MAGNIFYSCR] = 
							states.split.refreshPixmap[SPLIT3][TRACESCR] = 
							states.split.refreshPixmap[SPLIT2][TRACESCR] = 
							states.split.refreshPixmap[SPLIT2][SPECTRASCR] = 
							states.split.refreshPixmap[SPLIT3][SPECTRASCR] = TRUE;
				}	
				break;
			}			// end switch(states.user.event)
		break;

		case CTRL:
			// if we breach the edges, hardcode edge point
      		if (event->x < states.magnify.limits.margin[states.currentScreen].left)
			{
      			event->x = states.magnify.limits.margin[states.currentScreen].left;
			}
			if (event->x > states.magnify.limits.margin[states.currentScreen].right)
			{
				event->x = states.magnify.limits.margin[states.currentScreen].right;
			}
      		// process only if within valid section of screen
			if (event->y >= da->allocation.height - MAG_B_MARGIN || event->y < MAG_T_MARGIN)
			{
				states.msg = g_strdup("Selection not within trace ");
				break;
			}

			win_rec = (settings.magnify.overlay 
						? states.user.topRec.magnify 
						: (event->y - MAG_T_MARGIN) / 
							(states.magnify.trace_depth[states.currentScreen] + MAG_TFORM_MARGIN));
			trcData = g_slist_nth_data(states.trace.selection, win_rec);
			
			if (!settings.magnify.overlay)
						states.user.topRec.magnify = win_rec;

			magPoint(	win_rec, 
						states.magnify.limits.selection.user.left, 
						states.magnify.limits.selection.user.right,
						trcData->magnify.plot.increment, 
						(int) event->x,
						states.user.pointPtr);
			
		break;

		case DKEY:
			if (event->x > states.user.mouseLoc.startX)
			{	// we're moving right
				scrollDir = -1;
			}
			else
			{	// we're moving left
				scrollDir = 1;
			}

   			states.magnify.limits.selection.user.left += (scrollDir *
								(states.magnify.limits.selection.user.length/25));
			states.magnify.limits.selection.user.right += (scrollDir *
								(states.magnify.limits.selection.user.length/25));
			if (states.magnify.limits.selection.user.left < 0)
			{
				states.magnify.limits.selection.user.left = 0;
				states.magnify.limits.selection.user.right = states.magnify.limits.selection.user.length;
				gdk_beep();
			}
			if (states.magnify.limits.selection.user.right > states.magnify.absTime.diff)
			{
				states.magnify.limits.selection.user.right = states.magnify.absTime.diff;
				states.magnify.limits.selection.user.left = states.magnify.absTime.diff -
								states.magnify.limits.selection.user.length;
				gdk_beep();
			}

			states.user.mouseLoc.startX = event->x;
			states.reset.reset[MAGNIFYSCR] = 
					states.reset.reset[SPECTRASCR] = 
					states.split.refreshPixmap[SPLIT2][MAGNIFYSCR] = 
					states.split.refreshPixmap[SPLIT3][MAGNIFYSCR] = 
					states.split.refreshPixmap[SPLIT3][TRACESCR] = 
					states.split.refreshPixmap[SPLIT2][TRACESCR] = 
					states.split.refreshPixmap[SPLIT2][SPECTRASCR] = 
					states.split.refreshPixmap[SPLIT3][SPECTRASCR] = TRUE;
		break;
	}		// end switch(states.user.keyActive)

	return;
}

void specDrag(GtkWidget *da, GdkEventMotion *event, GdkPixmap *pixmap)
{
	int trans_trace_no;
	traceInfo *trcData;

	trans_trace_no = (settings.spectra.display.overlay ? states.user.topRec.spectra : 
						(event->y - MAG_T_MARGIN) / states.spectra.trace_depth[states.currentScreen]);
	trcData = g_slist_nth_data(states.trace.selection, trans_trace_no);
	switch(states.user.keyActive)
	{
		case CTRL:
			specPoint(trcData, (int) event->x, &states.spectra.point.window);
		break;
	}

	return;
}

void doDrag(GtkWidget *da, GdkEventMotion *event, GdkPixmap *pixmap, int scr)
{
	settings.general.font.DA = da;

	switch (scr)
	{
		case TRACESCR:
			traceDrag(da, event, pixmap);
			break;

		case MAGNIFYSCR:
			magDrag(da, event, pixmap);
			break;

		case SPECTRASCR:
			specDrag(da, event, pixmap);
			break;
	}
	return;
}
