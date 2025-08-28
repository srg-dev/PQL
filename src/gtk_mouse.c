#include "pql_defines.h"
#include "pql_externs.h"
#include "gtk_pql_externs.h"

#include <math.h>
#include <string.h>

/*******************************************************************************

these procedures define the events for all three screens.

*******************************************************************************/

#define	ONE	0
#define TWO 1
#define PIXEXTRA	8

gint insSort(traceInfo *one, traceInfo *two)
{
	return(one->data.position.current - two->data.position.current);
}

void traceMouse(GtkWidget *da, GdkEventButton *event, GdkPixmap *pixmap)
{
	int		win_rec, trace_no, Xval;
	traceInfo *trcData;

    switch (event->button) 
	{
    	case 1:
			switch(event->type)
			{
				case GDK_BUTTON_PRESS:
					states.user.mouseLeft = ACTIVE;
					states.user.mouseLoc.startX = event->x;
					states.user.mouseLoc.startY = event->y;
					states.user.drag.startRec = event->y / 
													states.trace.trace_depth[states.currentScreen];
					states.user.event = NONE;

					switch(states.user.keyActive)
					{
						case NONE:
							if (event->x < states.trace.limits.margin[states.currentScreen].left-PIXEXTRA &&
								event->y < states.trace.trace_depth[states.currentScreen]
												*settings.trace.numPlots)
							{	// click inside label?
									states.user.event = SELECTTR;		
							}
							else 
							if (event->x <= states.trace.limits.margin[states.currentScreen].right+PIXEXTRA &&
								event->x > states.trace.limits.margin[states.currentScreen].left-PIXEXTRA)
							{	// click inside plot margin boundaries?
									states.user.event = MAGBOUNDARY;
							}
						break;

						case CTRL:
							// save which screen defined these points
							states.trace.point.defGR = states.currentScreen;
							states.user.pointPtr = &states.trace.point.window.point[0];
							if (states.trace.point.window.point[0].xVal && 
								states.trace.point.window.point[1].xVal)
							{	// two points already defined, start over
								memset(&states.trace.point.window.point[0], 0, 
										sizeof(states.trace.point.window.point[0]));
								memset(&states.trace.point.window.point[1], 0, 
										sizeof(states.trace.point.window.point[1]));
							}
							else if (states.trace.point.window.point[0].xVal)
							{	// got the first, get the second
								states.user.pointPtr = &states.trace.point.window.point[1];
							}
						break;
					}
				break;
					
				case GDK_BUTTON_RELEASE:
					states.user.mouseLeft = INACTIVE;
		
					// continue processing LEFT click event
					switch(states.user.keyActive)
					{
						case SHIFT:		// process Move Trace to First request
							if (states.currentScreen != MAIN)
							{	// don't allow this functionality on the split screen
								break;
							}
							
				      		trace_no = event->y / states.trace.trace_depth[states.currentScreen];
				
				      		if ((trace_no >= settings.trace.numPlots) || 
								((states.trace.startRec + trace_no) >= states.general.numTraces))
								break;
					  
							gdk_window_set_cursor(topLevel, hourCursor);
							while (g_main_context_iteration(NULL, FALSE));
				      		states.trace.startRec += trace_no;
							states.reset.reset[TRACESCR] = TRUE;
							makeDispList(INITGROUP);
						break;
		
						case CTRL:		// process Identify Data Point request
							if (states.user.mouseLoc.startX != event->x)
							{	// don't process if we've been draggin
								break;
							}

				      		// process only if within valid section of screen
				      		if (event->x < states.trace.limits.margin[states.currentScreen].left || 
								event->x > states.trace.limits.margin[states.currentScreen].right ||
								event->y >= da->allocation.height - 24)
							{
								states.msg = g_strdup("Selection not within TRACE plot");
								break;
							}
				
							trace_no = (event->y) / states.trace.trace_depth[states.currentScreen];
							if (trace_no + states.trace.startRec >= states.general.numTraces)
							{
								states.msg = g_strdup("Selection not within a TRACE plot");
								break;
							}
							states.user.pointPtr->rec_num = trace_no;
							tracePoint( trace_no, (int) event->x, states.user.pointPtr);
						break;
		
						case NONE:		// process simple LEFT click event
							switch(states.user.event)
							{
								case MAGBOUNDARY:		// process Define Magnify Boundary request
									Xval = event->x;
									if (Xval < states.trace.limits.margin[states.currentScreen].left)
									{	// snap boundary to left edge
										Xval = states.trace.limits.margin[states.currentScreen].left;
									}
									else if (Xval > states.trace.limits.margin[states.currentScreen].right)
									{	// snap boundary to right edge
										Xval = states.trace.limits.margin[states.currentScreen].right;
									}

						      		if ((!states.trace.limits.selection.pix[states.currentScreen].left ) || 
										(states.trace.limits.selection.pix[states.currentScreen].left && 
										states.trace.limits.selection.pix[states.currentScreen].right)) 
									{
										states.trace.limits.selection.pix[states.currentScreen].left = Xval;
										states.trace.limits.selection.pix[states.currentScreen].right = 0;
						      		}
						      		else 
									{	// pix.left already specified, get pix.right
										states.trace.limits.selection.pix[states.currentScreen].right = Xval;
										if(states.trace.limits.selection.pix[states.currentScreen].right == 
												states.trace.limits.selection.pix[states.currentScreen].left) 
										{
							  				states.trace.limits.selection.pix[states.currentScreen].right++;
										}
										if (states.trace.limits.selection.pix[states.currentScreen].left >
												Xval)
										{	// make left really left and vice-versa
							  				states.trace.limits.selection.pix[states.currentScreen].right = 
												states.trace.limits.selection.pix[states.currentScreen].left;
							  				states.trace.limits.selection.pix[states.currentScreen].left =
							  					Xval;
										}
						      		}
									states.reset.reset[MAGNIFYSCR] = states.reset.reset[SPECTRASCR] = TRUE;
									states.split.refreshPixmap[SPLIT2][MAGNIFYSCR] = 
									states.split.refreshPixmap[SPLIT3][MAGNIFYSCR] = TRUE;
									states.split.refreshPixmap[SPLIT2][SPECTRASCR] = 
									states.split.refreshPixmap[SPLIT3][SPECTRASCR] = TRUE;
									states.magnify.limits.selection.user.left = 
									states.magnify.limits.selection.user.right = 0;		// reset
									states.magnify.selectionChanged = TRUE;
					      		break;					// done with this mouse event
		
								case SELECTTR:		// process Select Trace Event
									if (states.currentScreen != MAIN)
									{	// don't allow this functionality on the split screen
										break;
									}

									states.user.drag.endRec = event->y / 
																states.trace.trace_depth[states.currentScreen];
									if (states.user.drag.startRec == states.user.drag.endRec)
									{		// no dragging, just a single click
										win_rec = states.user.drag.startRec;
//										trcData = g_slist_nth_data(traces, win_rec + states.trace.startRec);
										trcData = g_slist_nth_data(states.trace.traces[CURGRP], win_rec);
										if (!trcData)		// don't process if no record
											break;
										trcData->trace.selected = !trcData->trace.selected;
										printLabel(pixmap, trcData, win_rec, 
														states.trace.trace_depth[states.currentScreen], 
														0, TRACESCR);
		
										gtk_widget_queue_draw_area(da, 4, 
													states.trace.trace_depth[states.currentScreen]*win_rec,
													states.trace.limits.margin[MAIN].left,
													states.trace.limits.margin[MAIN].right);
									}
		
									states.split.refreshPixmap[SPLIT2][TRACESCR] = 
									states.split.refreshPixmap[SPLIT3][TRACESCR] =
									states.split.refreshPixmap[SPLIT2][MAGNIFYSCR] = 
									states.split.refreshPixmap[SPLIT3][MAGNIFYSCR] =
									states.split.refreshPixmap[SPLIT2][SPECTRASCR] = 
									states.split.refreshPixmap[SPLIT3][SPECTRASCR] = TRUE;
									makeSelectionList(FALSE);
								break;
							}			// end switch(states.user.event)
					}				// end switch(states.user.keyActive)
				break;

				default:
					break;
			}				// end switch(event->type)
		break;
    }						// end switch(event->button)

}

void magMouse(GtkWidget *da, GdkEventButton *event, GdkPixmap *pixmap)
{
	int 	*ptr, idx, Xval;
	float	jumpU;
	int		direction;
	static	int	mag_trace_no;
	static traceInfo	*trcData;
	pointID		*pointPtr;
	
    switch (event->button) 
	{
		case 1:
			switch (event->type)
			{
				case GDK_BUTTON_PRESS:
					states.user.mouseLeft = ACTIVE;
					states.user.mouseLoc.startX = event->x;
					states.user.mouseLoc.startY = event->y;
					// save start position to determine later if we dragged or not
					states.user.drag.startRec = event->x;
					states.user.event = NONE;
					mag_trace_no = (settings.magnify.overlay 
									? states.user.topRec.magnify 
									: (event->y - MAG_T_MARGIN) / 
										(states.magnify.trace_depth[states.currentScreen] + MAG_TFORM_MARGIN));
					trcData = g_slist_nth_data(states.trace.selection, mag_trace_no);		

					switch(states.user.keyActive)
					{
						case DKEY: 
							states.user.event = NONE;
						break;
						case ZKEY: case XKEY:
							states.user.event = VLINE;
						break;
						
						case NONE:
						case ALT:
						case SHIFT:
//							if (event->y >= da->allocation.height - MAG_B_MARGIN)
							if (event->y < MAG_T_MARGIN)
							{	// click is in top x-axis region
								states.user.event = XAXIS;
							} 
							else
							{
								if (event->x < states.magnify.limits.margin[states.currentScreen].left)
								{	// click inside label?
									if (event->y < da->allocation.height - MAG_B_MARGIN && 
											event->y > MAG_T_MARGIN)
									{
										if (settings.magnify.overlay)
											states.user.event = REPLOT;		// only inside label area
										else
											states.user.event = SELECTTR;
									}
								}
								else if (event->x <= 
											states.magnify.limits.margin[states.currentScreen].right) 
								{	// click inside plot margin boundaries?
									if (event->y < da->allocation.height - MAG_B_MARGIN)
									{
										if (event->y > MAG_T_MARGIN)
											states.user.event = VLINE;		// only inside plot area
									}
								}
							}
						break;

						case CTRL:
							states.magnify.point.defGR = states.currentScreen;	// save who defined these points
							if (!trcData)
								break;
							switch (settings.magnify.Wamp)
							{
								case WSCALETRC:
									pointPtr = states.user.pointPtr = &trcData->magnify.point.trace.point[0];
								break;
								case WSCALEWIND:
									pointPtr = states.user.pointPtr = &states.magnify.point.window.point[0];
								break;
							}
							if (pointPtr->xVal && (pointPtr+1)->xVal)
							{	// two points already defined, start over
								memset(pointPtr, 0, sizeof(points));
							}
							else if (pointPtr->xVal)
							{	// just the first, get the second
								states.user.pointPtr = (pointPtr+1);
							}
						break;
					}
				break;

				case GDK_BUTTON_RELEASE:
					states.user.mouseLeft = INACTIVE;
		
					switch(states.user.keyActive)
					{
						case DKEY: case ZKEY: case XKEY:
						case NONE:
							// process event
							switch(states.user.event)
							{
								case SELECTTR:
									if (!trcData)		// don't process if no record
										break;
									trcData->magnify.selected = !trcData->magnify.selected;
									printLabel(pixmap, trcData, mag_trace_no, 
													states.magnify.trace_depth[states.currentScreen], 
													0, MAGNIFYSCR);

									gtk_widget_queue_draw_area(da, 4, 
												states.magnify.trace_depth[states.currentScreen]*mag_trace_no,
												states.magnify.limits.margin[MAIN].left,
												states.magnify.limits.margin[MAIN].right);
									states.split.refreshPixmap[SPLIT2][MAGNIFYSCR] = 
									states.split.refreshPixmap[SPLIT3][MAGNIFYSCR] =
									states.split.refreshPixmap[SPLIT2][SPECTRASCR] = 
									states.split.refreshPixmap[SPLIT3][SPECTRASCR] = TRUE;
									makeMAGSelectionList(FALSE);
								break;
								
								case REPLOT:						// bring trace to TOP
									if (!settings.magnify.overlay)
										break;						// only in overlay mode
				
									states.user.topRec.magnify = (event->y - MAG_T_MARGIN) / 
												(states.magnify.trace_depth[states.currentScreen] + MAG_TFORM_MARGIN);
									mag_plot_files(da, pixmap, da->allocation.width, 
													da->allocation.height, states.user.topRec.magnify);
									gtk_widget_queue_draw_area(da, 
												states.magnify.limits.margin[states.currentScreen].left,
												MAG_T_MARGIN, 
												states.magnify.limits.margin[states.currentScreen].length,
												da->allocation.height - (MAG_T_MARGIN+MAG_B_MARGIN));
								break;
				
								case VLINE:		// draw vertical line(s)
									Xval = event->x;
						      		if ((!states.magnify.limits.selection.pix.left ) || 
													(states.magnify.limits.selection.pix.left && 
													 states.magnify.limits.selection.pix.right)) 
									{
										states.magnify.limits.selection.pix.left = Xval;
										states.magnify.limits.selection.pix.right = 0;
										if (states.currentScreen == MAIN)
										{	// change to the appropriate button
											gtk_widget_hide(Mmag);
											gtk_widget_show(Msnap);
										}
										else
										{
											gtk_widget_hide(Hmag);
											gtk_widget_show(Hsnap);
//											gtk_widget_hide(Hmag[states.currentScreen]);
//											gtk_widget_show(Hsnap[states.currentScreen]);
										}
						      		}
						      		else 
									{	// pix.left already specified, get pix.right
										states.magnify.limits.selection.pix.right = Xval;
										if(states.magnify.limits.selection.pix.right == 
															states.magnify.limits.selection.pix.left) 
										{
							  				states.magnify.limits.selection.pix.right++;
										}
										if (states.magnify.limits.selection.pix.left > Xval)
										{
							  				states.magnify.limits.selection.pix.right = 
															states.magnify.limits.selection.pix.left;
							  				states.magnify.limits.selection.pix.left = Xval;
										}
										if (states.currentScreen == MAIN)
										{	// change to the appropriate button
											gtk_widget_hide(Msnap);
											gtk_widget_show(Mmag);
										}
										else
										{	// split screen widgets
											gtk_widget_hide(Hsnap);
											gtk_widget_show(Hmag);
										}
						      		}
								break;
				
								case XAXIS:
								{
									int x1, x2;
									// did we move the axis, or should we move to the point?
									if (states.user.drag.xAxis)
									{	// we've moved the axis, reset and leave
										states.user.drag.xAxis = FALSE;
										break;		
									}
		
									jumpU = (float) (states.magnify.limits.selection.user.length * 
													settings.magnify.scrollFactor/100.);
									x1 = USER2PIXX(&states.magnify.mainGR[states.currentScreen], 
												states.magnify.limits.selection.user.left);
									x2 = USER2PIXX(&states.magnify.mainGR[states.currentScreen], 
												states.magnify.limits.selection.user.right);
									direction = (x1 + (x2-x1)/2 > event->x)
												? -1
												: 1;

									// reset the user values
									states.magnify.limits.selection.user.left += (jumpU * direction);
									states.magnify.limits.selection.user.right += (jumpU * direction);
								
									// indicate user values should not be taken from limit.pix
									states.magnify.limits.selection.pix.left = 
											states.magnify.limits.selection.pix.right = 0;
								
									// don't breach the boundaries
									if (states.magnify.limits.selection.user.left < 
											states.trace.GR[states.currentScreen].user_xmin)
									{
										gdk_beep();
										states.trace.limits.selection.pix[states.currentScreen].left = 
												states.trace.GR[states.currentScreen].pix_xmin;
										states.trace.limits.selection.pix[states.currentScreen].right = 
												states.trace.limits.selection.pix[states.currentScreen].left + 
												states.trace.limits.selection.pix[states.currentScreen].length;
										states.magnify.limits.selection.user.left = 
												PIX2USERX(&states.trace.GR[states.currentScreen], 
												states.trace.limits.selection.pix[states.currentScreen].left);
										states.magnify.limits.selection.user.right = 
												states.magnify.limits.selection.user.left + 
												states.magnify.limits.selection.user.length;
									}
									if (states.magnify.limits.selection.user.right > 
											states.trace.GR[states.currentScreen].user_xmax)
									{
										gdk_beep();
										states.trace.limits.selection.pix[states.currentScreen].right = 
												states.trace.GR[states.currentScreen].pix_xmax;
										states.trace.limits.selection.pix[states.currentScreen].left = 
												states.trace.limits.selection.pix[states.currentScreen].right - 
												states.trace.limits.selection.pix[states.currentScreen].length;
										states.magnify.limits.selection.user.right = 
												PIX2USERX(&states.trace.GR[states.currentScreen], 
												states.trace.limits.selection.pix[states.currentScreen].right);
										states.magnify.limits.selection.user.left = 
												states.magnify.limits.selection.user.right - 
												states.magnify.limits.selection.user.length;
									}
									
									states.reset.reset[MAGNIFYSCR] = TRUE;
									states.reset.reset[SPECTRASCR] = TRUE;
									// set the flag for refresh of drawing
									states.split.refreshPixmap[SPLIT2][MAGNIFYSCR] = TRUE;
									states.split.refreshPixmap[SPLIT2][SPECTRASCR] = TRUE;
									states.split.refreshPixmap[SPLIT3][MAGNIFYSCR] = TRUE;
									states.split.refreshPixmap[SPLIT3][SPECTRASCR] = TRUE;
								}
								break;
									
				
								case NONE:
								break;		// click outside all valid event regions, i.e., nothing to do, just leave
							}				// end switch(states.user.event)
							break;
		
					case CTRL:
							if (states.user.mouseLoc.startX != event->x ||	// don't process if we've been draggin
								!states.user.pointPtr)		// or if press event hasn't been registered
							{	
								break;
							}

				      		// process only if within valid section of screen
				      		if (event->x < states.magnify.limits.margin[states.currentScreen].left || 
								event->x > states.magnify.limits.margin[states.currentScreen].right ||
								event->y >= da->allocation.height - MAG_B_MARGIN || 
								event->y < MAG_T_MARGIN)
							{
								states.msg = g_strdup("Selection not within TRACE plot");
								break;
							}
				
							if (!settings.magnify.overlay)
								states.user.topRec.magnify = mag_trace_no;
							magPoint(	mag_trace_no, 
										states.magnify.limits.selection.user.left, 
										states.magnify.limits.selection.user.right,
										trcData->magnify.plot.increment, 
										(int) event->x,
										states.user.pointPtr);
					break;
					
		
					case ALT:
							switch(states.user.event)
							{
								case VLINE:						
									if (!states.magnify.pick.curPick)
									{
										states.msg = g_strdup("Must first choose a pick: keys 1-0 (0=10)");
										break;
									}
									states.magnify.pick.curPick--;	// a bit non-kosher, but it's easier for the test above

									if (!settings.magnify.pickTag[states.magnify.pick.curPick][0])
									{
										states.msg = g_strdup_printf("Nothing defined for pick # %d, please define via Controls panel first.", 
											states.magnify.pick.curPick+1);
										break;
									}

									// verify main Pick has been chosen
									if (settings.magnify.pick == WSCALETRC)
									{	// pick by TRACE
										ptr = &trcData->magnify.pick.trace.pick[states.magnify.pick.curPick];
									}
									else
									{	// pick by WINDOW
										ptr = &states.magnify.pick.window.pick[states.magnify.pick.curPick];
									}
									if (!*ptr)
									{
										states.msg = g_strdup("Must first choose the main PICK before selecting error PICK.");
										break;
									}

									if (settings.magnify.pick == WSCALETRC)
									{	// pick by TRACE
										ptr = &trcData->magnify.pick.trace.errorPick[states.magnify.pick.curPick][0];
									}
									else
									{	// pick by WINDOW
										ptr = &states.magnify.pick.window.errorPick[states.magnify.pick.curPick][0];
									}

									idx = ONE;
									if (ptr[ONE] && ptr[TWO])
									{	// two error picks already defines, start over
										memset(ptr, 0, sizeof(states.magnify.pick.window.errorPick[states.magnify.pick.curPick]));
										idx = ONE;
									}
									else if(ptr[ONE])
									{
										idx = TWO;
									}
									ptr[idx] = (int) event->x;

								break;
							}
					break;

					case SHIFT:
							switch(states.user.event)
							{
								case VLINE:					
									if (!states.magnify.pick.curPick)
									{
										states.msg = g_strdup("Must first choose a pick: keys 1-0 (0=10)");
										break;
									}
									states.magnify.pick.curPick--;	// a bit non-kosher, but it's easier for the test above

									if (!settings.magnify.pickTag[states.magnify.pick.curPick][0])
									{
										states.msg = g_strdup_printf( 
											"Nothing defined for pick # %d, please define via Controls panel first.", 
											states.magnify.pick.curPick+1);
										break;
									}

									// only allow picking in OVERLAY mode to be scope of WINDOW
									if (settings.magnify.pick == WSCALETRC && 
										settings.magnify.overlay)
									{
										states.msg = g_strdup("PICK Scope can only be WINDOW in Overlay Mode");
										break;
									}

									if (settings.magnify.pick == WSCALETRC)
									{	// pick by TRACE
										ptr = &trcData->magnify.pick.trace.pick[states.magnify.pick.curPick];
									}
									else
									{	// pick by WINDOW
										ptr = &states.magnify.pick.window.pick[states.magnify.pick.curPick];
									}
									*ptr = (int) event->x;

								break;

								case XAXIS:						
									states.trace.limits.selection.pix[states.currentScreen].left = 
										USER2PIXX(&states.trace.GR[MAIN], 
										PIX2USERX(&states.magnify.GR[states.currentScreen], (int) event->x))
											- states.trace.limits.selection.pix[states.currentScreen].length/2;
									states.trace.limits.selection.pix[states.currentScreen].right =
										states.trace.limits.selection.pix[states.currentScreen].left
											+ states.trace.limits.selection.pix[states.currentScreen].length;
					
									if (states.trace.limits.selection.pix[states.currentScreen].left < 
											states.trace.GR[MAIN].pix_xmin)
									{
										states.trace.limits.selection.pix[states.currentScreen].left = 
											states.trace.GR[MAIN].pix_xmin;
										states.trace.limits.selection.pix[states.currentScreen].right =
											states.trace.limits.selection.pix[states.currentScreen].left + 
											states.trace.limits.selection.pix[states.currentScreen].length;
									}
									if (states.trace.limits.selection.pix[states.currentScreen].right > 
											states.trace.GR[MAIN].pix_xmax)
									{
										states.trace.limits.selection.pix[states.currentScreen].right = 
											states.trace.GR[MAIN].pix_xmax;
										states.trace.limits.selection.pix[states.currentScreen].left =
											states.trace.limits.selection.pix[states.currentScreen].right - 
											states.trace.limits.selection.pix[states.currentScreen].length;
									}
					    			states.magnify.limits.selection.user.left = 
					    				PIX2USERX(&states.trace.GR[MAIN], 
										states.trace.limits.selection.pix[states.currentScreen].left);
									states.magnify.limits.selection.user.right = 
										PIX2USERX(&states.trace.GR[MAIN],
										states.trace.limits.selection.pix[states.currentScreen].right);
									states.reset.reset[MAGNIFYSCR] = 
										states.split.refreshPixmap[SPLIT2][MAGNIFYSCR] = 
										states.split.refreshPixmap[SPLIT3][MAGNIFYSCR] = 
										states.split.refreshPixmap[SPLIT3][TRACESCR] = 
										states.split.refreshPixmap[SPLIT2][TRACESCR] = 
										states.split.refreshPixmap[SPLIT2][SPECTRASCR] = 
										states.split.refreshPixmap[SPLIT3][SPECTRASCR] = TRUE;
			
								break;
							}	// end switch(states.user.event)

						break;
					}		// end switch(states.user.keyActive)
					break;

				default:
					break;
			}		// end switch(event->type)
			break;
	}					// end switch(event->button)

}

void specMouse(GtkWidget *da, GdkEventButton *event, GdkPixmap *pixmap)
{
	int trans_trace_no, Xval;
	traceInfo *trcData;

	trans_trace_no = (settings.spectra.display.overlay ? states.user.topRec.spectra : 
					(event->y - MAG_T_MARGIN) / states.spectra.trace_depth[states.currentScreen]);
	trcData = g_slist_nth_data(states.trace.selection, trans_trace_no);

    switch (event->button) 
	{
		case 1:
			switch(event->type)
			{
				case GDK_BUTTON_PRESS:
					states.user.mouseLeft = ACTIVE;
					states.user.mouseLoc.startX = event->x;
					states.user.mouseLoc.startY = event->y;
					states.user.event = NONE;
					states.spectra.point.defGR = states.currentScreen;	// save who defined these points
					switch(states.user.keyActive)
					{
						case NONE:
							if (event->x < states.spectra.limits.margin[states.currentScreen].left)
							{	// click inside label?
								states.user.event = REPLOT;		// only inside label area
							}
							else if (event->x <= states.spectra.limits.margin[states.currentScreen].right)
							{	// click inside plot margin boundaries?
								states.user.event = FREQCUTOFF;		// only inside plot area
							}
							break;
					}
				break;
					
				case GDK_BUTTON_RELEASE:
					states.user.mouseLeft = INACTIVE;

					switch(states.user.keyActive)
					{
						case NONE:		// process FREQ Cutoff and Overlay REPLOT requests
							switch(states.user.event)
							{
								case REPLOT:
									if (!settings.spectra.display.overlay ||	// only in overlay mode
										(event->x > states.spectra.limits.margin[states.currentScreen].left))		// only on trace name
										break;
				
									states.user.topRec.spectra = (event->y - MAG_T_MARGIN) / 
																	states.spectra.trace_depth[states.currentScreen];
									trans_plot_files(da, pixmap, da->allocation.width, 
													da->allocation.height, states.user.topRec.spectra);
									gtk_widget_queue_draw_area(da, 
											states.spectra.limits.margin[states.currentScreen].left, 0, 
											states.spectra.limits.margin[states.currentScreen].length, 
											da->allocation.height);
								break;
				
								case FREQCUTOFF:
									Xval = event->x;
				
						      		if ((!states.spectra.limits.selection.pix.left ) || 
													(states.spectra.limits.selection.pix.left && 
													 states.spectra.limits.selection.pix.right)) 
									{
										states.spectra.limits.selection.pix.left = Xval;
										states.spectra.limits.selection.user.left = 
												PIX2USERX(&states.spectra.GR[states.currentScreen], Xval);
										states.spectra.limits.selection.pix.right = 0;
										states.spectra.limits.selection.user.right = 0;
						      		}
						      		else 
									{	// pix.left already specified, get pix.right
										states.spectra.limits.selection.pix.right = Xval;
										states.spectra.limits.selection.user.right = 
												PIX2USERX(&states.spectra.GR[states.currentScreen], (int) Xval);
										if(Xval == states.spectra.limits.selection.pix.left) 
										{
							  				states.spectra.limits.selection.pix.right = ++Xval;
											states.spectra.limits.selection.user.right = 
													PIX2USERX(&states.spectra.GR[states.currentScreen], (int) Xval);
											break;
										}
										if (states.spectra.limits.selection.pix.left > Xval)
										{	// switch if necessary
							  				states.spectra.limits.selection.pix.right = 
													states.spectra.limits.selection.pix.left;
											states.spectra.limits.selection.user.right = 
													states.spectra.limits.selection.user.left;
							  				states.spectra.limits.selection.pix.left = Xval;
											states.spectra.limits.selection.user.left = 
													PIX2USERX(&states.spectra.GR[states.currentScreen], Xval);
											break;
										}
				
						      		}
								break;
				
								case NONE:
								break;
							}
						break;
		
						case CTRL:		// process IDENTIFY and SCAN Data Point Requests
							states.user.mouseLeft = INACTIVE;
		
				      		// process only if within valid section of screen
				      		if (event->x < states.spectra.limits.margin[states.currentScreen].left || 
								event->x > states.spectra.limits.margin[states.currentScreen].right ||
								event->y >= da->allocation.height - 24)
							{
								states.msg = g_strdup("Selection not within TRACE plot");
								break;
							}
				
							specPoint(trcData, (int) event->x, &states.spectra.point.window);
						break;
		
						case SHIFT:	
							states.user.mouseLeft = INACTIVE;

							if (states.spectra.limits.selection.user.left)		// only if something is set
							{
								int custom = FILTERCUSTOMLISTNUM;
								if (settings.general.filter.filts[custom]->poles[HIGHF] && 
									!settings.general.filter.filts[custom]->poles[LOWF])	
								{	// high-pass only, set
									settings.general.filter.filts[custom]->cutoff[HIGHF] =
										states.spectra.limits.selection.user.left;
									break;		
								}
								if (!settings.general.filter.filts[custom]->poles[HIGHF] &&
									settings.general.filter.filts[custom]->poles[LOWF]) 
								{	// low-pass only, set
									settings.general.filter.filts[custom]->cutoff[LOWF] =
										states.spectra.limits.selection.user.left;
									break;	
								}
								// both filters are on
								settings.general.filter.filts[custom]->cutoff[HIGHF] = 
									states.spectra.limits.selection.user.left;
								if (states.spectra.limits.selection.user.right)
								{
									settings.general.filter.filts[custom]->cutoff[LOWF] = 
										states.spectra.limits.selection.user.right;
								}

//								states.filter.newFilter[states.filter.active] = TRUE;
								if (states.tForm.on && trcData->data.tForm.data)
								{	// if filter is on, we should re-execute with new values, make it so...
									applyFilter(states.currentPage);
								}
							}
						break;
					}		// end switch(states.user.keyActive)
				break;

				default:
				break;
			}				// end switch(event->type)
		break;
	}				// end switch(event->button)
}

void doMouse(GtkWidget *da, GdkEventButton *event, GdkPixmap *pixmap, int scr)
{
	settings.general.font.DA = da;
	switch (scr)
	{
		case TRACESCR:
			traceMouse(da, event, pixmap);
		break;

		case MAGNIFYSCR:
			magMouse(da, event, pixmap);
		break;

		case SPECTRASCR:
			specMouse(da, event, pixmap);
		break;
    }
}

void makeSelectionList(gboolean selectAll)
{
	GSList *trcIter;
	traceInfo *trcData;
	
	g_slist_free(states.trace.selection);
	states.trace.selection = NULL;
	for(trcIter = states.trace.traces[CURGRP];
		trcIter;
		trcIter = g_slist_next(trcIter))
	{
		trcData = trcIter->data;
		if (selectAll ||
			trcData->trace.selected)
		{
			states.trace.selection = g_slist_insert_sorted(states.trace.selection, 
				trcData, (GCompareFunc) insSort);
		}
	}

	states.trace.numSelected = g_slist_length(states.trace.selection);
	states.reset.reset[MAGNIFYSCR] = TRUE;
	states.reset.reset[SPECTRASCR] = TRUE;
	memset(&states.magnify, 0, sizeof(states.magnify));
	memset(&states.spectra, 0, sizeof(states.spectra));
#if 0
5-apr-10
since transforms can occur on individual (selected) traces, this is no longer necessary
	states.tForm.on = OFF;
	setRadios(FILTERTYPE);		// selection has changed, turn transform off
#endif
	states.trace.selectionChanged = TRUE;
	states.magnify.selectionChanged = TRUE;
}

void makeMAGSelectionList(gboolean selectAll)
{
	GSList *trcIter;
	traceInfo *trcData;
	
	g_slist_free(states.magnify.selection);
	states.magnify.selection = NULL;
	for(trcIter = states.trace.selection;
		trcIter;
		trcIter = g_slist_next(trcIter))
	{
		trcData = trcIter->data;
		if (selectAll ||
			trcData->magnify.selected)
		{
			states.magnify.selection = g_slist_insert_sorted(states.magnify.selection, 
				trcData, (GCompareFunc) insSort);
		}
	}

	states.reset.reset[MAGNIFYSCR] = TRUE;
	states.reset.reset[SPECTRASCR] = TRUE;
	states.split.resetPixmaps = TRUE;
}
