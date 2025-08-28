#include <string.h>
#include <stdlib.h>
#include "pql_defines.h"
#include "pql_externs.h"
#include "gtk_pql_externs.h"

#include <math.h>

#define LLEFT 0
#define LRIGHT 1
#define X	0
#define Y	1
#define POINT1	0
#define POINT2	1

			// [MAIN|SPLIT2|SPLIT3][pixmapID][extraType]
static int	trace[3][TMAXPIXMAPS][2], 	// [LLEFT|LRIGHT]
			magnify[3][MMAXPIXMAPS][2],	// [LLEFT|LRIGHT]
			magnifyPick[3][MMAXPIXMAPS][MAXPICKS],	// [PICKNUM]
			magnifyEPick[3][MMAXPIXMAPS][MAXPICKS][2],	// [LLEFT|LRIGHT][ERROR1|ERROR2]
			*magnifyTPick, *magnifyTEPick, *magnifyTPoint,	// by TRACE picks
			spectra[3][SMAXPIXMAPS][2],	// [LLEFT|LRIGHT]
			point[3][3][2][2];			// [POINT1|POINT2][TRACE|MAG|SPEC][MAIN|SPLIT2|SPLIT3][X|Y]

static int	iWidth, iHeight;
static gboolean okay, drawn;

#define PICKLABELXPOS	3
#define PICKLABELYPOS	2

static
void plotPicks(GtkWidget *da, GdkPixmap *pixmap, int pixmapID, int scr)
{
	int		Lval, i, j, k, *scrPick, *userPick;
	gboolean	pick=FALSE;
	char	errorStr[12];
	GSList	*trcIter;
	traceInfo *trcData;
	int	y1, y2;

	settings.general.font.DA = da;
	settings.general.font.font = AXISF;
	settings.general.font.GC = GXC[INVGC][scr];

	if (pixmapID != PRINTDISPLAY)		// don't keep track if printing
	{
	for(i=0;i<MAXPICKS && settings.magnify.pickTag[i][0]; ++i)
	{	// deal with WINDOW scope picks
		// undraw whatever's already there
		if(magnifyPick[states.currentScreen][pixmapID][i])
		{
			gdk_draw_line(pixmap, GXC[INVGC][scr], 
					magnifyPick[states.currentScreen][pixmapID][i], 1, 
					magnifyPick[states.currentScreen][pixmapID][i], iHeight);
			gdk_draw_rectangle(pixmap, GXC[ABNORMALGC][scr], TRUE, 
					magnifyPick[states.currentScreen][pixmapID][i]+PICKLABELXPOS, 2, 
					fontSIZE(settings.magnify.pickTag[i], WIDTH) + 1, 
					fontSIZE(settings.magnify.pickTag[i], HEIGHT) + 1);
			magnifyPick[states.currentScreen][pixmapID][i] = 0;
		}
	}

	for(k=0;k<states.trace.numSelected;k++)
	for(i=0;i<MAXPICKS && settings.magnify.pickTag[i][0]; ++i)
	{	// deal with TRACE scope picks
		// undraw whatever's already there
		y1 = MAG_T_MARGIN + (k+1)*MAG_TFORM_MARGIN + k*states.magnify.trace_depth[states.currentScreen];
		y2 = y1 + states.magnify.trace_depth[states.currentScreen] - 1;
		
		scrPick = &magnifyTPick[k*(3*MMAXPIXMAPS*MAXPICKS) + 
								states.currentScreen*(MMAXPIXMAPS*MAXPICKS) +
								pixmapID*(MAXPICKS) +
								i];
		if(*scrPick)
		{
			gdk_draw_line(pixmap, GXC[INVGC][scr], 
					*scrPick, y1, 
					*scrPick, y2); 
			gdk_draw_rectangle(pixmap, GXC[ABNORMALGC][scr], TRUE, 
					*scrPick, y1,
					fontSIZE(settings.magnify.pickTag[i], WIDTH) + 1 + PICKLABELXPOS, 
					fontSIZE(settings.magnify.pickTag[i], HEIGHT) + 1 + PICKLABELYPOS);
			*scrPick = 0;
		}

		for(j=0;j<2;j++)
		{
			// undraw any error picks
			scrPick = &magnifyTEPick[k*(3*MMAXPIXMAPS*MAXPICKS*2) + 
									states.currentScreen*(MMAXPIXMAPS*MAXPICKS*2) +
									pixmapID*(MAXPICKS*2) +
									i*2 +
									j];
			if(*scrPick)
			{
				gdk_draw_line(pixmap, GXC[INVGC][scr], 
						*scrPick, y1,
						*scrPick, y2);
				sprintf(errorStr, "e%s", settings.magnify.pickTag[i]);
				gdk_draw_rectangle(pixmap, GXC[ABNORMALGC][scr], TRUE, 
					*scrPick, y1,	//MAG_T_MARGIN + k*states.magnify.trace_depth[states.currentScreen],
					fontSIZE(errorStr, WIDTH) + 1 + PICKLABELXPOS, 
					fontSIZE(errorStr, HEIGHT) + 1 + PICKLABELYPOS);
				*scrPick = 0;
			}
		}
	}
	}
	
	if (settings.magnify.pick == WSCALEWIND)
	{	// plot by WINDOW
		for(i=0;i<MAXPICKS && settings.magnify.pickTag[i][0]; ++i)
		{
			// draw what's currently defined
			if(states.magnify.pick.window.pick[i])
			{
				pick=TRUE;
				Lval = states.magnify.pick.window.pick[i];
				gdk_draw_rectangle(pixmap, GXC[ABNORMALGC][scr], TRUE, 
						Lval, 1, 
						fontSIZE(settings.magnify.pickTag[i], WIDTH) + 1 + PICKLABELXPOS, 
						fontSIZE(settings.magnify.pickTag[i], HEIGHT) + 1 + PICKLABELXPOS);
				gdk_draw_line(pixmap, GXC[INVGC][scr], 
						Lval, 1,
						Lval, iHeight);
				fontDraw(pixmap, settings.magnify.pickTag[i], 
						Lval + PICKLABELXPOS, 2, 
						NORMALGC, MAGNIFYSCR, -1, -1);
				if (pixmapID != PRINTDISPLAY)		// don't keep track if printing
					magnifyPick[states.currentScreen][pixmapID][i] = Lval;
			}
	
			// plot currently defined error picks
			for(j=0;j<2;j++)
			{
				// undraw whatever's already there
				if (pixmapID != PRINTDISPLAY)		// don't keep track if printing
				if(magnifyEPick[states.currentScreen][pixmapID][i][j])
				{
					gdk_draw_line(pixmap, GXC[INVGC][scr], 
							magnifyEPick[states.currentScreen][pixmapID][i][j], 1, 
							magnifyEPick[states.currentScreen][pixmapID][i][j], iHeight);
					sprintf(errorStr, "e%s", settings.magnify.pickTag[i]);
					gdk_draw_rectangle(pixmap, GXC[ABNORMALGC][scr], TRUE, 
						magnifyEPick[states.currentScreen][pixmapID][i][j]+PICKLABELXPOS, 2, 
						fontSIZE(errorStr, WIDTH) + 1, fontSIZE(errorStr, HEIGHT) + 1);
					magnifyEPick[states.currentScreen][pixmapID][i][j] = 0;
				}
		
				// draw what's currently defined
				if(states.magnify.pick.window.errorPick[i][j])
				{
					Lval = states.magnify.pick.window.errorPick[i][j];
					sprintf(errorStr, "e%s", settings.magnify.pickTag[i]);
					gdk_draw_rectangle(pixmap, GXC[ABNORMALGC][scr], TRUE, 
							Lval, 1, 
							fontSIZE(errorStr, WIDTH) + 1 + PICKLABELXPOS, 
							fontSIZE(errorStr, HEIGHT) + 1 + PICKLABELXPOS);
					gdk_draw_line(pixmap, GXC[INVGC][scr], 
							Lval, 1,
							Lval, iHeight);
					fontDraw(pixmap, errorStr,
							Lval + PICKLABELXPOS, 2, 
							NORMALGC, MAGNIFYSCR, -1, -1);
					if (pixmapID != PRINTDISPLAY)		// don't keep track if printing
						magnifyEPick[states.currentScreen][pixmapID][i][j] = Lval;
				}
			}
		}
	}
	else
	{		// plot by TRACE
		for (trcIter = states.trace.selection, k=0;
			 trcIter;
			 trcIter = g_slist_next(trcIter), k++)
		{
		trcData = trcIter->data;
		for(i=0;i<MAXPICKS && settings.magnify.pickTag[i][0]; ++i)
		{
			// plot currently defined picks
			if (pixmapID != PRINTDISPLAY)		// don't keep track if printing
			scrPick = &magnifyTPick[k*(3*MMAXPIXMAPS*MAXPICKS) + 
									states.currentScreen*(MMAXPIXMAPS*MAXPICKS) +
									pixmapID*(MAXPICKS) +
									i];
	
			y1 = MAG_T_MARGIN + (k+1)*MAG_TFORM_MARGIN + k*states.magnify.trace_depth[states.currentScreen];
			y2 = y1 + states.magnify.trace_depth[states.currentScreen] - 1;
			// draw what's currently defined
			userPick = &trcData->magnify.pick.trace.pick[i];
			if(*userPick)
			{
				pick=TRUE;
				Lval = *userPick;
				gdk_draw_rectangle(pixmap, GXC[ABNORMALGC][scr], TRUE, 
						Lval, y1,	//MAG_T_MARGIN + k*states.magnify.trace_depth[states.currentScreen],
						fontSIZE(settings.magnify.pickTag[i], WIDTH) + 1 + PICKLABELXPOS, 
						fontSIZE(settings.magnify.pickTag[i], HEIGHT) + 1 + PICKLABELYPOS);
				gdk_draw_line(pixmap, GXC[INVGC][scr], 
						Lval, y1,	//MAG_T_MARGIN + k*states.magnify.trace_depth[states.currentScreen],
						Lval, y2);	//MAG_T_MARGIN + (k+1)*states.magnify.trace_depth[states.currentScreen] - 1);
				fontDraw(pixmap, settings.magnify.pickTag[i], 
						Lval + PICKLABELXPOS, y1 + PICKLABELYPOS,
//						MAG_T_MARGIN + k*states.magnify.trace_depth[states.currentScreen] + PICKLABELYPOS, 
						NORMALGC, MAGNIFYSCR, -1, -1);
				if (pixmapID != PRINTDISPLAY)		// don't keep track if printing
					*scrPick = Lval;
			}
	
			// plot currently defined error picks
			for(j=0;j<2;j++)
			{
				if (pixmapID != PRINTDISPLAY)		// don't keep track if printing
					scrPick = &magnifyTEPick[k*(3*MMAXPIXMAPS*MAXPICKS*2) + 
										states.currentScreen*(MMAXPIXMAPS*MAXPICKS*2) +
										pixmapID*(MAXPICKS*2) +
										i*2 +
										j];

				// draw what's currently defined
				userPick = &trcData->magnify.pick.trace.errorPick[i][j];
				if(*userPick)
				{
					Lval = *userPick;
					sprintf(errorStr, "e%s", settings.magnify.pickTag[i]);
					gdk_draw_rectangle(pixmap, GXC[ABNORMALGC][scr], TRUE, 
						Lval, y1,	//MAG_T_MARGIN + k*states.magnify.trace_depth[states.currentScreen],
						fontSIZE(errorStr, WIDTH) + 1 + PICKLABELXPOS, 
						fontSIZE(errorStr, HEIGHT) + 1 + PICKLABELYPOS);
					gdk_draw_line(pixmap, GXC[INVGC][scr], 
							Lval, y1,	//MAG_T_MARGIN + k*states.magnify.trace_depth[states.currentScreen],
							Lval, y2);	//MAG_T_MARGIN + (k+1)*states.magnify.trace_depth[states.currentScreen] - 1);
					fontDraw(pixmap, errorStr,
							Lval + PICKLABELXPOS, y1 + PICKLABELYPOS,
//							MAG_T_MARGIN + k*states.magnify.trace_depth[states.currentScreen] + PICKLABELYPOS, 
							NORMALGC, MAGNIFYSCR, -1, -1);
					if (pixmapID != PRINTDISPLAY)		// don't keep track if printing
						*scrPick = Lval;
				}
			}
		}
		}
	}

	// print the comment
#define COMMENTXPOS	5
#define COMMENTYPOS	5

	if (pick && settings.magnify.pickComment[0])
	{
		int x, y;

		settings.general.font.font = LABELF;
		settings.general.font.GC = GXC[NORMALGC][scr];

// position comment just above xaxis, just to the right of the yaxis
		x = states.magnify.limits.margin[states.currentScreen].left + COMMENTXPOS; 
		y = MAG_T_MARGIN + states.trace.numSelected * MAG_TFORM_MARGIN +
				 	states.trace.numSelected*states.magnify.trace_depth[states.currentScreen] -
					fontSIZE(settings.magnify.pickComment, HEIGHT) - 
					COMMENTYPOS;
		gdk_draw_rectangle(pixmap, GXC[ABNORMALGC][scr], TRUE, 
			x-1, y-1,
			fontSIZE(settings.magnify.pickComment, WIDTH) + 2,
			fontSIZE(settings.magnify.pickComment, HEIGHT) + 2);
		fontDraw(pixmap, settings.magnify.pickComment, x, y, NORMALGC, MAGNIFYSCR, -1, -1);
	}

	states.magnify.pick.curPick = 0;	// reset to undefined pick, i.e., force user to define every time

	for (trcIter = states.trace.selection, k=0;
		 trcIter;
		 trcIter = g_slist_next(trcIter), k++)
	{
		int		userPick, lineS, lineE;
		gboolean	windTrue = FALSE;

		trcData = trcIter->data;

		for(i=0;i<trcData->trace.numPicks;i++)
		{
			if (!drawn &&
				trcData->trace.picks[i].pickT > states.magnify.limits.selection.user.left &&
				trcData->trace.picks[i].pickT < states.magnify.limits.selection.user.right)
			{	// old pick falls in our window, plot
				userPick = USER2PIXX(&trcData->magnify.GR[states.currentScreen], 
								trcData->trace.picks[i].pickT);
				switch(trcData->trace.picks[i].pickS)
				{
					case WSCALEWIND:
						if(trcData->trace.picks[i].pickE)
						{
							sprintf(errorStr, "e%s", 
										settings.magnify.pickTag[trcData->trace.picks[i].pickN]);
						}
						else
						{
							sprintf(errorStr, "%s", 
										settings.magnify.pickTag[trcData->trace.picks[i].pickN]);
						}
						lineS = 1;
						lineE = iHeight;
						windTrue = TRUE;
					break;
		
					case WSCALETRC:
						if(trcData->trace.picks[i].pickE)
						{
							sprintf(errorStr, "e%s", 
										settings.magnify.pickTag[trcData->trace.picks[i].pickN]);
						}
						else
						{
							sprintf(errorStr, "%s", 
										settings.magnify.pickTag[trcData->trace.picks[i].pickN]);
						}
						lineS = MAG_T_MARGIN + k*MAG_TFORM_MARGIN + k*states.magnify.trace_depth[states.currentScreen];
						lineE = MAG_T_MARGIN + k*MAG_TFORM_MARGIN + (k+1)*states.magnify.trace_depth[states.currentScreen] - 1;
					break;
				}
				gdk_draw_rectangle(pixmap, GXC[ABNORMALGC][scr], TRUE, 
					userPick, lineS,
					fontSIZE(errorStr, WIDTH) + 1 + PICKLABELXPOS, 
					fontSIZE(errorStr, HEIGHT) + 1 + PICKLABELYPOS);
				gdk_draw_line(pixmap, GXC[INVGC][scr], 
					userPick, lineS,
					userPick, lineE); 
				fontDraw(pixmap, errorStr,
					userPick + PICKLABELXPOS, lineS + PICKLABELYPOS,
					NORMALGC, MAGNIFYSCR, -1, -1);
			}
		}
		if(windTrue)
			break;
	}
	drawn = TRUE;
}

static
void plotLine(GtkWidget *da, GdkPixmap *pixmap, int pixmapID, int scr, bounds *pix)
{
	int	*linePtr=NULL, Lval, Rval;

	switch(scr)
	{
		case TRACESCR:
			if (pixmapID != PRINTDISPLAY)		// don't keep track if printing
			{
				linePtr = trace[states.currentScreen][pixmapID];
			}
		break;

		case MAGNIFYSCR:
			if (pixmapID != PRINTDISPLAY)		// don't keep track if printing
			{
				linePtr = magnify[states.currentScreen][pixmapID];
			}
		break;

		case SPECTRASCR:
			if (pixmapID != PRINTDISPLAY)		// don't keep track if printing
			{
				linePtr = spectra[states.currentScreen][pixmapID];
			}
		break;
	}

	if (pixmapID != PRINTDISPLAY)		// don't keep track if printing
	{
	if (linePtr[LLEFT])
	{
  			gdk_draw_line(pixmap, GXC[INVGC][scr], linePtr[LLEFT], 0, linePtr[LLEFT], iHeight);
			linePtr[LLEFT]=0;
	}

	if (linePtr[LRIGHT])
	{
  			gdk_draw_line(pixmap, GXC[INVGC][scr], linePtr[LRIGHT], 0, linePtr[LRIGHT], iHeight);
			linePtr[LRIGHT]=0;
	}
	}

	Lval = 0;
	if (pix->left)
	{
		Lval = pix->left;
  		gdk_draw_line(pixmap, GXC[INVGC][scr], Lval, 0, Lval, iHeight);
	}
	Rval=0;
	if (pix->right && (pix->right != pix->left))
	{
		Rval = pix->right;
  		gdk_draw_line(pixmap, GXC[INVGC][scr], Rval, 0, Rval, iHeight);
	}
	
	if (pixmapID != PRINTDISPLAY)		// don't keep track if printing
	{
		linePtr[LLEFT] = Lval;
		linePtr[LRIGHT] = Rval;
	}
}

static void pointVals(pointID *pointPtr, char *str, int pointNum, int dataFormat)
{
	char	string[255];
	sprintf(string, "[ #%d: Time =  %4d %03d:%02d:%02d:%06.3f / ", 
				pointNum, pointPtr->timeVal.yr,
				julian(&pointPtr->timeVal), pointPtr->timeVal.hr, 
				pointPtr->timeVal.mn, pointPtr->timeVal.sec);
	strcpy(str, string);
			
	if (pointPtr->minAmp == pointPtr->maxAmp)
	{
		switch(dataFormat)
		{
			case MY_FLOAT:
			case MY_DOUBLE:
				sprintf(string, "Amp = %6.8f ", pointPtr->minAmp);
			break;
			default:
				sprintf(string, "Amp = %d ", (int) pointPtr->minAmp);
			break;
		}
	}
	else
	{
		switch(dataFormat)
		{
			case MY_FLOAT:
			case MY_DOUBLE:
				sprintf(string, "Amp Range = < %6.8f ~ %6.8f > ", 
							pointPtr->minAmp, pointPtr->maxAmp);
			break;
			default:
				sprintf(string, "Amp Range = < %d ~ %d > ", 
							(int) pointPtr->minAmp, (int) pointPtr->maxAmp);
			break;
		}
	}
	strcat(string, "]");
	strcat(str, string);
}

static void plotCross(GdkPixmap *pixmap, int pixmapID, int scr)
{
	pointID	*pointPtr=NULL, pointDiff;
	int		*scrPoint=NULL, xpos;
	int		msg, k, i, dataFormat;
	char	str[256];
	GSList	*trcIter;
	traceInfo *trcData;

	// first deal with previously drawn crosshairs: undraw them if they exist
	if (pixmapID != PRINTDISPLAY)		// don't keep track if printing
	switch(scr)
	{
		case TRACESCR:
		case SPECTRASCR:
			for(i=0;i<2; ++i)
			{
				scrPoint = (int *) &point[scr][states.currentScreen][i];
				xpos = i ? 5 : 10;
				if(scrPoint[X])
				{
		  			gdk_draw_line(pixmap, GXC[INVGC][scr], 
							scrPoint[X] - xpos, scrPoint[Y] - 10,
							scrPoint[X] + xpos, scrPoint[Y] + 10);
		  			gdk_draw_line(pixmap, GXC[INVGC][scr], 
							scrPoint[X] + xpos, scrPoint[Y] - 10,
							scrPoint[X] - xpos, scrPoint[Y] + 10);
					scrPoint[X] = scrPoint[Y] = 0;
				}
			}
		break;

		case MAGNIFYSCR:
			for(k=0;k<states.trace.numSelected;k++)
			for(i=0;i<2; ++i)
			{
				scrPoint = &magnifyTPoint[k*(3*2*2) + 					// array element 1
										  states.currentScreen*2*2 +	// array element 2
										  i*2];							// array element 3
				xpos = i ? 5 : 10;
				if(scrPoint[X])
				{
		  			gdk_draw_line(pixmap, GXC[INVGC][scr], 
							scrPoint[X] - xpos, scrPoint[Y] - 10,
							scrPoint[X] + xpos, scrPoint[Y] + 10);
		  			gdk_draw_line(pixmap, GXC[INVGC][scr], 
							scrPoint[X] + xpos, scrPoint[Y] - 10,
							scrPoint[X] - xpos, scrPoint[Y] + 10);
					scrPoint[X] = scrPoint[Y] = 0;
				}
			}
		break;
	}

	// redraw what is currently defined
	msg = NONE;
	switch(scr)
	{
		case TRACESCR:
			trcData = g_slist_nth_data(traces, states.trace.point.window.point[0].rec_num);
	
			dataFormat = trcData->data.head.data_form;
			for(i=0;i<2; ++i)
			{
				pointPtr = &states.trace.point.window.point[i];
				xpos = i ? 5 : 10;
				if (pointPtr->xVal)
				{
					int xVal, yVal;

					xpos = i ? 5 : 10;
					if (states.trace.point.defGR != states.currentScreen)
					{
						graph *fromGR, *toGR;
						fromGR = &trcData->trace.GR[states.trace.point.defGR];
						if (states.trace.point.defGR == MAIN)
						{	// from MAIN to SPLIT | PRINT
							if (states.currentScreen == PRINT)
							{	// to PRINT
								toGR = &trcData->trace.GR[states.currentScreen];
							}
							else
							{	// to SPLIT
								traceInfo *trcData = g_slist_nth_data(states.trace.selection, pointPtr->rec_num);
								toGR = &trcData->trace.GR[states.currentScreen];
							}
						}
						else
						{	// from SPLIT to MAIN | PRINT
							if (states.currentScreen == PRINT)
							{	// to PRINT
								toGR = &trcData->trace.GR[states.currentScreen];
							}
							else
							{	// to MAIN
								traceInfo *trcData = g_slist_nth_data(states.trace.selection, pointPtr->rec_num);
								toGR = &trcData->trace.GR[states.currentScreen];
							}
						}
						xVal = USER2PIXX(toGR, PIX2USERX(fromGR, pointPtr->xVal));
						yVal = USER2PIXY(toGR, PIX2USERY(fromGR, pointPtr->yVal));
					}
					else
					{
						xVal = pointPtr->xVal;
						yVal = pointPtr->yVal;
					}
			  		gdk_draw_line(pixmap, GXC[INVGC][scr], 
								xVal - xpos, yVal -10,
								xVal + xpos, yVal + 10);
			  		gdk_draw_line(pixmap, GXC[INVGC][scr], 
								xVal + xpos, yVal -10,
								xVal - xpos, yVal + 10);
					msg = i;
					if (pixmapID != PRINTDISPLAY)		// don't keep track if printing
					{
						point[scr][states.currentScreen][i][X] = xVal;
						point[scr][states.currentScreen][i][Y] = yVal;
					}
				}
			}
		break;

		case MAGNIFYSCR:
			for (trcIter = states.trace.selection, k=0;
				 trcIter;
				 trcIter = g_slist_next(trcIter), k++)
			{
				trcData = trcIter->data;
				dataFormat = trcData->data.head.data_form;
				for(i=0;i<2; ++i)
				{
					if (pixmapID != PRINTDISPLAY)		// don't keep track if printing
						scrPoint = &magnifyTPoint[k*(3*2*2) + 
											states.currentScreen*2*2 +
											i*2];

					if (settings.magnify.Wamp == WSCALETRC)
					{
						pointPtr = &trcData->magnify.point.trace.point[i];
					}
					else
					{
						pointPtr = &states.magnify.point.window.point[i];
					}
					if(pointPtr->xVal)
					{
						int xVal, yVal;

						xpos = i ? 5 : 10;
						if (states.magnify.point.defGR != states.currentScreen)
						{
							graph *fromGR, *toGR;
							fromGR = &trcData->magnify.GR[states.magnify.point.defGR];
							toGR = &trcData->magnify.GR[states.currentScreen];
							xVal = USER2PIXX(toGR, PIX2USERX(fromGR, pointPtr->xVal));
							yVal = USER2PIXY(toGR, PIX2USERY(fromGR, pointPtr->yVal));
						}
						else
						{
							xVal = pointPtr->xVal;
							yVal = pointPtr->yVal;
						}
			  			gdk_draw_line(pixmap, GXC[INVGC][scr], 
								xVal - xpos, yVal - 10,
								xVal + xpos, yVal + 10);
			  			gdk_draw_line(pixmap, GXC[INVGC][scr], 
								xVal + xpos, yVal -10,
								xVal - xpos, yVal + 10);
						if (pixmapID != PRINTDISPLAY)		// don't keep track if printing
						{
							scrPoint[X] = xVal;
							scrPoint[Y] = yVal;
						}
					}
				}
				if (settings.magnify.Wamp == WSCALEWIND)	// only two points, don't loop over traces
					break;
			}
			msg = POINT1;
			if (settings.magnify.Wamp == WSCALETRC)
			{
				traceInfo *trcData = g_slist_nth_data(states.trace.selection, states.user.topRec.magnify);
				pointPtr = &trcData->magnify.point.trace.point[1];
			}
			else
			{
				pointPtr = &states.magnify.point.window.point[1];
			}
			if (pointPtr->epoch)
				msg = POINT2;

		break;

		case SPECTRASCR:
			pointPtr = &states.spectra.point.window;
			if (pointPtr->xVal)
			{
				int xVal, yVal;

				xpos = i ? 5 : 10;
#if 0
this code will not work - commented out for now - undone: make it work
				if (states.spectra.point.defGR != states.currentScreen)
				{
					graph *fromGR, *toGR;
					fromGR = &states.spectra.GRs[states.spectra.point.defGR][k];
					toGR = &states.spectra.GRs[states.currentScreen][k];
					xVal = USER2PIXX(toGR, PIX2USERX(fromGR, pointPtr->xVal));
					yVal = USER2PIXY(toGR, PIX2USERY(fromGR, pointPtr->yVal));
				}
				else
				{
					xVal = pointPtr->xVal;
					yVal = pointPtr->yVal;
				}
#endif
					xVal = pointPtr->xVal;
					yVal = pointPtr->yVal;
		  		gdk_draw_line(pixmap, GXC[INVGC][scr], 
							xVal - 10, yVal -10,
							xVal + 10, yVal + 10);
		  		gdk_draw_line(pixmap, GXC[INVGC][scr], 
							xVal + 10, yVal -10,
							xVal - 10, yVal + 10);
				msg = POINT1;
				if (pixmapID != PRINTDISPLAY)		// don't keep track if printing
				{
					point[scr][states.currentScreen][0][X] = xVal;
					point[scr][states.currentScreen][0][Y] = yVal;
				}
			}
		break;
	}

	if (pixmapID != PRINTDISPLAY)		// don't keep track if printing
	{
	switch(msg)
	{
		case POINT1:
			switch(scr)
			{
				case TRACESCR:
				case MAGNIFYSCR:
					if (scr==TRACESCR)
					{
						pointPtr = &states.trace.point.window.point[0];
					}
					else
					{
						if (settings.magnify.Wamp == WSCALETRC)
						{
							traceInfo *trcData = g_slist_nth_data(states.trace.selection,
													states.user.topRec.magnify);
							pointPtr = &trcData->magnify.point.trace.point[0];
						}
						else
						{
							pointPtr = &states.magnify.point.window.point[0];
						}
					}

					if (!pointPtr->epoch)
						break;

					pointVals(pointPtr, str, 1, dataFormat);
					states.msg = g_strdup(str);
				break;
	
				case SPECTRASCR:
				{
					char *tmp;
					if (settings.spectra.display.xAxis == LOG)
					{
	   					sprintf(str, "Frequency = % 3.4f Hz\t// Period = % 4.1f Sec\t// ", 
										pointPtr->freqVal, 1./pointPtr->freqVal);
					}
					else
					{
	   					sprintf(str, "Frequency = % 3.4f Hz\t// ", pointPtr->freqVal);
					}
					states.msg = g_strdup(str);
					sprintf(str, "Amplitude = %7.6f", pointPtr->minAmp);
					tmp = states.msg;
					states.msg = g_strconcat(tmp, str, NULL);
					free(tmp);
				}
				break;
			}		// end switch(scr)
		break;

		case POINT2:
			switch(scr)
			{
				case TRACESCR:
				case MAGNIFYSCR:
				{
					char *tmp;
					if (scr==TRACESCR)
					{
						pointPtr = &states.trace.point.window.point[0];
					}
					else
					{
						if (settings.magnify.Wamp == WSCALETRC)
						{
							traceInfo *trcData = g_slist_nth_data(states.trace.selection,
													states.user.topRec.magnify);
							pointPtr = &trcData->magnify.point.trace.point[0];
						}
						else
						{
							pointPtr = &states.magnify.point.window.point[0];
						}
					}

					// DIFFERENCE
					memset(&pointDiff, 0, sizeof(pointDiff));
					pointDiff.epoch = (pointPtr[1].epoch - pointPtr[0].epoch);
					pointDiff.maxAmp = (pointPtr[1].maxAmp - pointPtr[0].maxAmp);
					pointDiff.minAmp = (pointPtr[1].minAmp - pointPtr[0].minAmp);
					
					sprintf(str, "[ DELTA: Time = %6.3f secs / ", pointDiff.epoch);
					states.msg = g_strdup(str);
					
					if (pointDiff.minAmp == pointDiff.maxAmp)
					{
						switch(dataFormat)
						{
							case MY_FLOAT:
							case MY_DOUBLE:
								sprintf(str, "Amp = %6.5f ]\t", pointDiff.minAmp);
							break;
							default:
								sprintf(str, "Amp = %d ]\t", (int) pointDiff.minAmp);
							break;
						}
					}
					else
					{
						switch(dataFormat)
						{
							case MY_FLOAT:
							case MY_DOUBLE:
								sprintf(str, "Amp Range = <%6.5f ~ %6.5f> ]\t", 
												pointDiff.minAmp, pointDiff.maxAmp);
							break;
							default:
								sprintf(str, "Amp Range = < %d ~ %d > ]\t", 
												(int) pointDiff.minAmp, 
												(int) pointDiff.maxAmp);
							break;
						}
					}
					tmp = states.msg;
					states.msg = g_strconcat(tmp, str, NULL);
					free(tmp);
	
					// POINT 2
					pointVals(&pointPtr[1], str, 2, dataFormat);
					strcat(str, "\t");
					tmp = states.msg;
					states.msg = g_strconcat(tmp, str, NULL);
					free(tmp);

					// POINT 1
					pointVals(&pointPtr[0], str, 1, dataFormat);
					tmp = states.msg;
					states.msg = g_strconcat(tmp, str, NULL);
					free(tmp);
				}
				break;

/*	no POINT2 case for SPECTRA screen at this time
				case SPECTRASCR:
					pointPtr = &states.spectra.point;
	   				sprintf(str, "Frequency:  %3.4f Hz | ", pointPtr->freqVal);
					states.msg = g_strdup(str);
	
					if (pointPtr->minAmp == pointPtr->maxAmp)
					{
						sprintf(str, "Amplitude: %7.6f", pointPtr->minAmp);
					}

					strcat(states.msg, str);
				break;
*/
			}		// end switch(scr)

		break;
	}		// end switch(msg)

	if (states.msg && okay)
	{
		dispMsg(DISPLAY, states.msg);
		free(states.msg);
	}
	states.msg = NULL;
	}
}

void
plot_extras(GtkWidget *da, GdkPixmap *pixmap, int pixmapID, int scr, int firstCall, int imageW, int imageH)
{
	iWidth = imageW;
	iHeight = imageH;

	dispMsg(UNDISPLAY, NULL);
	if (states.msg)
	{
		dispMsg(DISPLAY, states.msg);
		free(states.msg);
		states.msg = NULL;
		gdk_beep();
		okay = FALSE;
	}
	else
	{
		okay = TRUE;
	}

    switch (scr) 
	{
 		case TRACESCR:
			if (pixmapID == TRACENONE)
			{
				memset(trace[states.currentScreen], 0, sizeof(trace[states.currentScreen]));
				break;
			}

			if (firstCall)
			{
				memset(trace[states.currentScreen], 0, sizeof(trace[states.currentScreen]));
				memset(&point[scr][states.currentScreen], 0, sizeof(point[scr][states.currentScreen]));
			}

			// lines to draw?
			plotLine(da, pixmap, pixmapID, scr, &states.trace.limits.selection.pix[states.currentScreen]);

			// draw crosshairs and corresponding value?
			plotCross(pixmap, pixmapID, scr);

		break;

 		case MAGNIFYSCR:
			if (!states.trace.limits.selection.pix[states.currentScreen].left || !states.trace.numSelected)
			{
				memset(&magnify[states.currentScreen], 0, sizeof(magnify[states.currentScreen]));
				break;
			}
			
			if (firstCall)
			{
				drawn=FALSE;
				memset(&magnify[states.currentScreen], 0, sizeof(magnify[states.currentScreen]));
				memset(&magnifyPick[states.currentScreen], 0, sizeof(magnifyPick[states.currentScreen]));
				memset(&magnifyEPick[states.currentScreen], 0, sizeof(magnifyEPick[states.currentScreen]));
				if (magnifyTPick)
				{
					free(magnifyTPick);
					free(magnifyTEPick);
					free(magnifyTPoint);
					magnifyTPick = NULL;
					magnifyTEPick = NULL;
					magnifyTPoint = NULL;
				}
				if ((magnifyTPoint = calloc(sizeof(int), states.trace.numSelected*3*2*2)) == NULL)
				{
					fprintf(stderr, "Unable to allocate memory, exiting\n");
					exit (-1);
				}
				if ((magnifyTPick = calloc(sizeof(int), states.trace.numSelected*3*MMAXPIXMAPS*MAXPICKS)) == NULL)
				{
					fprintf(stderr, "Unable to allocate memory, exiting\n");
					exit (-1);
				}
				if ((magnifyTEPick = calloc(sizeof(int), states.trace.numSelected*3*MMAXPIXMAPS*MAXPICKS*2)) == NULL)
				{
					fprintf(stderr, "Unable to allocate memory, exiting\n");
					exit (-1);
				}
			}

			// lines to draw?
			plotLine(da, pixmap, pixmapID, scr, &states.magnify.limits.selection.pix);

			// plot picks 
			plotPicks(da, pixmap, pixmapID, scr);

			// draw crosshairs and corresponding value?
			plotCross(pixmap, pixmapID, scr);

		break;

 		case SPECTRASCR:
			if (!states.trace.limits.selection.pix[states.currentScreen].left || !states.trace.numSelected)
			{
				break;
			}

			if (firstCall)
			{
				memset(&spectra[states.currentScreen], 0, sizeof(spectra[states.currentScreen]));
				memset(&point[scr][states.currentScreen], 0, sizeof(point[scr][states.currentScreen]));
				break;
			}

			// draw frequency cutoff line?
			plotLine(da, pixmap, pixmapID, scr, &states.spectra.limits.selection.pix);

			// clear our values to plot only once no matter what
			if (states.spectra.limits.selection.pix.left && states.spectra.limits.selection.pix.right) 	
			{
				states.spectra.limits.selection.pix.left =
				states.spectra.limits.selection.pix.right = 0;
			}

			// draw point identifier?
			plotCross(pixmap, pixmapID, scr);

		break;
	}

	return;
}
