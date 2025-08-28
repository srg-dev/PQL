#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <gdk/gdkkeysyms.h>

#include "pql_defines.h"
#include "pql_externs.h"
#include "gtk_pql_externs.h"

static GtkWidget *InfoWin;
static GtkWidget *SOHWin;
static void logClear();

int	page(int scr)
{	// helper function identifying which page is associated with which scr
	int page=0;
	switch(scr)
	{
		case TRACESCR:
			page = TRACESCR;
		break;

		case MAGNIFYSCR:
			page = MAGNIFYSCR;
		break;

		case SPECTRASCR:
			page = SPECTRASCR;
		break;

		case SPLITDA3T:
		case SPLITDA3M:
		case SPLITDA3B:
		case SPLITDA2T:
		case SPLITDA2B:
			page = SPLITSCR;
		break;
	}

	return page;
}

gboolean pixmap_cfg(GtkWidget *widget, GdkEventConfigure *event, gpointer s)
{	// callback for all drawing areas - configure event
	int	scr = GPOINTER_TO_INT(s);
	static int count2=0, count3=0;

	if (!hourCursor)
	{
		topLevel = gdk_window_get_toplevel(widget->window);
		hourCursor = gdk_cursor_new_for_display(gdk_display_get_default (), GDK_WATCH);
	}
		
	switch(scr)
	{
		case TRACESCR:
		case MAGNIFYSCR:
		case SPECTRASCR:
			states.reset.refresh[scr] = TRUE;		// set when app has been resized by the user only
		break;

		case SPLITDA3T:
		case SPLITDA3M:
		case SPLITDA3B:
			if (count3 < 3)
			{	// ignore first call
				count3++;
				return TRUE;
			}			
			states.split.resetPixmap[states.split.panes][scr-SPLITDA3T] = TRUE;
		break;

		case SPLITDA2T:
		case SPLITDA2B:
			if (count2 < 2)
			{	// ignore first call
				count2++;
				return TRUE;
			}			
			states.split.resetPixmap[states.split.panes][scr-SPLITDA2T] = TRUE;
		break;
	}
	
	if (states.input.numFiles &&
		states.general.plot.state == NOTRACES)
	{
		readHdr_1();
		states.general.plot.state = READHDRS;
	}

	if (page(scr) == states.currentPage)
	{
		states.reset.cursor = TRUE;
		makePixmap(scr);
	}
	
	states.currentScr = scr;
	return TRUE;
}

gboolean pixmap_exp(GtkWidget *widget, GdkEventExpose *event, gpointer s)
{	// callback for all drawing areas - expose event
	int	scr = GPOINTER_TO_INT(s);
	if (!DAPixmap[scr] || (page(scr) != states.currentPage))		// startup timing issue
		return TRUE;

	gdk_draw_drawable(widget->window,
					widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
					DAPixmap[scr],
					event->area.x, event->area.y,
					event->area.x, event->area.y,
					event->area.width, event->area.height);

	if (states.reset.cursor == TRUE)
	{
		gdk_window_set_cursor(topLevel, NULL);
		states.reset.cursor = FALSE;
	}
	
	switch(scr)
	{
		case SPLITDA3M:
			states.split.realized[0] = TRUE;
			// fall-through...
		case SPLITDA3B:
			states.split.realized[1] = TRUE;
		break;
		case SPLITDA2B:
			states.split.realized[2] = TRUE;
		break;
	}
	return TRUE;
}

gint nextTrace(GtkButton *button, gpointer d)
{	// called by Next and PREVIOUS buttons on TRACE screen
	int		dir = GPOINTER_TO_INT(d);
	selectT(NULL, GINT_TO_POINTER(DESELECT));	// clean up selection list
	switch(dir)
	{
		case TRUE:		//  NEXT button
			if (!states.trace.traces[NEXTGRP])
			{
				dispMsg(DISPLAY, "At END of trace file list... No more traces to plot");
				gdk_beep();
				return TRUE;
			}
		break;

		case FALSE:		//  PREVIOUS button
			if (!states.trace.traces[PREVGRP])
			{
				dispMsg(DISPLAY, "At BEGINNING of trace file list... No more traces to plot");
				gdk_beep();
				return TRUE;
			}
		break;
	}

	makeDispList(dir ? NEXTGROUP : PREVGROUP);
	states.general.plot.state = READDATA;
	makePixmap(TRACESCR);
	return TRUE;
}

gint restartTrace(GtkButton *button, gpointer s)
{	// called by Continue, Restart, & Sort buttons from control panel
	int		startRec = GPOINTER_TO_INT(s);
	states.general.plot.state = READDATA;
	states.reset.reset[TRACESCR] = TRUE;
	states.trace.startRec = startRec;

	makeDispList(INITGROUP);
	return TRUE;
}

static void closePopUps(int which)
{
	switch(which)
	{
		case ALLPOPUPS:
			if (InfoWin)
				gtk_widget_hide(InfoWin);
			if (SOHWin)
				gtk_widget_hide(SOHWin);
		break;
		case INFOPOPUP:
			if (InfoWin)
				gtk_widget_hide(InfoWin);
		break;
		case SOHPOPUP:
			if (SOHWin)
				gtk_widget_hide(SOHWin);
		break;
	}
}

gboolean grabFocus(GtkWidget *widget, GdkEvent *event, gpointer nil)
{	// to receive key press events in the drawing area, we must grab the focus when mouse enters

	switch(event->type)
	{
		case GDK_ENTER_NOTIFY:
			gtk_grab_add(GTK_WIDGET (widget));
		break;
		case GDK_LEAVE_NOTIFY:
			closePopUps(INFOPOPUP);
			gtk_grab_remove(GTK_WIDGET (widget));
		break;
		default:
		break;
	}
	return TRUE;
}

#if 0
void setDU(int dispScale)
{
	static int numItems=TTLDISPUNITS;
	int	i, units;

	switch (dispScale)
	{
		case (MYVOLTS+1):							// only AH, SAC, and NANO data on display
			units = settings.general.units = MYVOLTS;		// define only MYVOLTS allowed
		break;
	  
		case (MYCOUNTS+1):							// only MSEED data on display
			units = settings.general.units = MYCOUNTS;		// define only MYCOUNTS allowed
		break;

		default:									// SEGY data or mix of above on display
			settings.general.units = MYCOUNTS;		// define both allowed
			units = TTLDISPUNITS;
		break;
	}

	g_signal_handler_block(Tcbo[TRCDISPU], signals[DISPSGNL][TRACESIGNAL]);
	g_signal_handler_block(Mcbo[MAGDISPU], signals[DISPSGNL][MAGSIGNAL]);
	g_signal_handler_block(Scbo[SPECDISPU], signals[DISPSGNL][SPECSIGNAL]);

	for(i=0;i<numItems;i++)
	{
		gtk_combo_box_remove_text(GTK_COMBO_BOX(Tcbo[TRCDISPU]), 0);
		gtk_combo_box_remove_text(GTK_COMBO_BOX(Mcbo[MAGDISPU]), 0);
		gtk_combo_box_remove_text(GTK_COMBO_BOX(Scbo[SPECDISPU]), 0);
	}
	
	switch (units)
	{
		case MYVOLTS:
		case MYCOUNTS:
			gtk_combo_box_append_text(GTK_COMBO_BOX(Tcbo[TRCDISPU]), dispunits[units]);
			gtk_combo_box_append_text(GTK_COMBO_BOX(Mcbo[MAGDISPU]), dispunits[units]);
			gtk_combo_box_append_text(GTK_COMBO_BOX(Scbo[SPECDISPU]), dispunits[units]);
			gtk_combo_box_set_active(GTK_COMBO_BOX(Tcbo[TRCDISPU]), 0);
			gtk_combo_box_set_active(GTK_COMBO_BOX(Mcbo[MAGDISPU]), 0);
			gtk_combo_box_set_active(GTK_COMBO_BOX(Scbo[SPECDISPU]), 0);
			numItems = 1;
		break;
		case TTLDISPUNITS:
			for(i=0;i<TTLDISPUNITS;i++)
			{
				gtk_combo_box_append_text(GTK_COMBO_BOX(Tcbo[TRCDISPU]), dispunits[i]);
				gtk_combo_box_append_text(GTK_COMBO_BOX(Mcbo[MAGDISPU]), dispunits[i]);
				gtk_combo_box_append_text(GTK_COMBO_BOX(Scbo[SPECDISPU]), dispunits[i]);
			}
			gtk_combo_box_set_active(GTK_COMBO_BOX(Tcbo[TRCDISPU]), settings.general.units);
			gtk_combo_box_set_active(GTK_COMBO_BOX(Mcbo[MAGDISPU]), settings.general.units);
			gtk_combo_box_set_active(GTK_COMBO_BOX(Scbo[SPECDISPU]), settings.general.units);
			numItems = TTLDISPUNITS;
		break;
	}
	g_signal_handler_unblock(Tcbo[TRCDISPU], signals[DISPSGNL][TRACESIGNAL]);
	g_signal_handler_unblock(Mcbo[MAGDISPU], signals[DISPSGNL][MAGSIGNAL]);
	g_signal_handler_unblock(Scbo[SPECDISPU], signals[DISPSGNL][SPECSIGNAL]);
}
#endif

void setRadios(int type)
{
	switch(type)
	{
		case FILTERTYPE:
			g_signal_handler_block(MTForm[OFF], signals[FILTSGNL][MAGSIGNAL]);
			g_signal_handler_block(STForm[OFF], signals[FILTSGNL][SPECSIGNAL]);
			g_signal_handler_block(HTForm[OFF], signals[FILTSGNL][SPLITSIGNAL]);
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (MTForm[states.tForm.on]), TRUE);
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (STForm[states.tForm.on]), TRUE);
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (HTForm[states.tForm.on]), TRUE);
			g_signal_handler_unblock(MTForm[OFF], signals[FILTSGNL][MAGSIGNAL]);
			g_signal_handler_unblock(STForm[OFF], signals[FILTSGNL][SPECSIGNAL]);
			g_signal_handler_unblock(HTForm[OFF], signals[FILTSGNL][SPLITSIGNAL]);
		break;

		case DISPLAYTYPE:
//			g_signal_handler_block(Tcbo[TRCDISPU], signals[DISPSGNL][TRACESIGNAL]);
//			g_signal_handler_block(Mcbo[MAGDISPU], signals[DISPSGNL][MAGSIGNAL]);
//			g_signal_handler_block(Scbo[SPECDISPU], signals[DISPSGNL][SPECSIGNAL]);
//			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (Tdisp[settings.general.units]), TRUE);
//			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (Mdisp[settings.general.units]), TRUE);
//			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (Sdisp[settings.general.units]), TRUE);
//			g_signal_handler_unblock(Tcbo[TRCDISPU], signals[DISPSGNL][TRACESIGNAL]);
//			g_signal_handler_unblock(Mcbo[MAGDISPU], signals[DISPSGNL][MAGSIGNAL]);
//			g_signal_handler_unblock(Scbo[SPECDISPU], signals[DISPSGNL][SPECSIGNAL]);
		break;
		
		case MAGSCRTYPE:
			g_signal_handler_block(Mcbo[MAGTSCALE], signals[TRACESCALESGNL][MAGSIGNAL+MAXSCRNSGNLS*0]);
			g_signal_handler_block(Mcbo[MAGWSCALE], signals[WNDWSCALESGNL][MAGSIGNAL+MAXSCRNSGNLS*0]);
			g_signal_handler_block(Mover[OFF], signals[OVERLAYSGNL][MAGSIGNAL+MAXSCRNSGNLS*0]);
			g_signal_handler_block(Scbo[SPECWSCALE], signals[WNDWSCALESGNL][SPECSIGNAL+MAXSCRNSGNLS*0]);
			g_signal_handler_block(Sover[OFF], signals[OVERLAYSGNL][SPECSIGNAL+MAXSCRNSGNLS*0]);

			g_signal_handler_block(Hcbo[SPLITTSCALE], signals[TRACESCALESGNL][SPLITSIGNAL+MAXSCRNSGNLS*0]);
			g_signal_handler_block(Hcbo[SPLITWSCALE], signals[WNDWSCALESGNL][SPLITSIGNAL+MAXSCRNSGNLS*0]);
			g_signal_handler_block(Hover[OFF], signals[OVERLAYSGNL][SPLITSIGNAL+MAXSCRNSGNLS*0]);
			
			gtk_combo_box_set_active(GTK_COMBO_BOX(Mcbo[MAGTSCALE]), settings.magnify.Tamp);
			gtk_combo_box_set_active(GTK_COMBO_BOX(Mcbo[MAGWSCALE]), settings.magnify.Wamp);
			gtk_combo_box_set_active(GTK_COMBO_BOX(Scbo[SPECWSCALE]), settings.spectra.display.amp);
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (Mover[settings.magnify.overlay]), TRUE);
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (Sover[settings.spectra.display.overlay]), TRUE);

			gtk_entry_set_text(GTK_ENTRY(HTfixed[MAX]), gtk_entry_get_text(GTK_ENTRY(MTfixed[MAX])));
			gtk_entry_set_text(GTK_ENTRY(HTfixed[MIN]), gtk_entry_get_text(GTK_ENTRY(MTfixed[MIN])));
			gtk_combo_box_set_active(GTK_COMBO_BOX(Hcbo[SPLITTSCALE]), settings.magnify.Tamp);
			gtk_combo_box_set_active(GTK_COMBO_BOX(Hcbo[SPLITWSCALE]), settings.magnify.Wamp);
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (Hover[settings.magnify.overlay]), TRUE);
			
			g_signal_handler_unblock(Mcbo[MAGTSCALE], signals[TRACESCALESGNL][MAGSIGNAL+MAXSCRNSGNLS*0]);
			g_signal_handler_unblock(Mcbo[MAGWSCALE], signals[WNDWSCALESGNL][MAGSIGNAL+MAXSCRNSGNLS*0]);
			g_signal_handler_unblock(Mover[OFF], signals[OVERLAYSGNL][MAGSIGNAL+MAXSCRNSGNLS*0]);
			g_signal_handler_unblock(Scbo[SPECWSCALE], signals[WNDWSCALESGNL][SPECSIGNAL+MAXSCRNSGNLS*0]);
			g_signal_handler_unblock(Sover[OFF], signals[OVERLAYSGNL][SPECSIGNAL+MAXSCRNSGNLS*0]);
			g_signal_handler_unblock(Hcbo[SPLITTSCALE], signals[TRACESCALESGNL][SPLITSIGNAL+MAXSCRNSGNLS*0]);
			g_signal_handler_unblock(Hcbo[SPLITWSCALE], signals[WNDWSCALESGNL][SPLITSIGNAL+MAXSCRNSGNLS*0]);
			g_signal_handler_unblock(Hover[OFF], signals[OVERLAYSGNL][SPLITSIGNAL+MAXSCRNSGNLS*0]);
		break;			
	}
}

void change_menu(GtkNotebook *notebook, GtkWidget *page, guint new_page, int *o)
{	// callback when new notebook tab selected 
	// or when programmatically switching screens
	int		old_page = *o;
	dispMsg(UNDISPLAY, NULL);

	if (old_page != new_page)
		gtk_widget_hide(buttons[old_page]);

	setRadios(DISPLAYTYPE);									// make all DISPLAY types equal
	setRadios(MAGSCRTYPE);
	switch(new_page)
	{
		case TRACESCR:
			states.currentScreen = MAIN;
			gtk_widget_show(buttons[new_page]);
			makePixmap(new_page);
			states.reset.cursor = TRUE;
		break;

		case MAGNIFYSCR:
			states.currentScreen = MAIN;
			get_abs_mag_times();
			gtk_widget_show(buttons[new_page]);
			gtk_widget_queue_resize(DA[MAGNIFYDA]);			// force draw through configure event
			states.reset.cursor = TRUE;
		break;

		case SPECTRASCR:
			states.currentScreen = MAIN;
			gtk_widget_show(buttons[new_page]);
			makePixmap(new_page);
			states.reset.cursor = TRUE;
		break;

		case SPLITSCR:
			gtk_widget_show(buttons[SPLITSCR]);
			switch(states.split.panes)
			{
				case SPLIT2:
					states.currentScreen = SPLIT2;
					gtk_widget_hide(split3);
					gtk_widget_show(split2);
					if (settings.split.DA[states.split.panes][MID] == TRACESCR)
					{	//trace screen is not visible, draw it invisibly
						makePixmap(SPLITDA2M);
					}
					makePixmap(SPLITDA2T);
					states.reset.cursor=TRUE;
					makePixmap(SPLITDA2B);
				break;
		
				case SPLIT3:
					states.currentScreen = SPLIT3;
					gtk_widget_hide(split2);
					gtk_widget_show(split3);
					makePixmap(settings.split.DT[SPLIT3][TRACESCR]);
					makePixmap(settings.split.DT[SPLIT3][MAGNIFYSCR]);
					states.reset.cursor=TRUE;
					makePixmap(settings.split.DT[SPLIT3][SPECTRASCR]);
				break;
			}
		break;

		case HEADERSCR:
			states.currentScreen = MAIN;
			if (!states.trace.numSelected)
			{
				settings.headers.display = DISPLAYED;
			}
			else
			{
				settings.headers.display = SELECTED;
			}
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (HDRdisp[settings.headers.display]), TRUE);
			states.reset.cursor = TRUE;
			gtk_widget_show(buttons[new_page]);
			gdk_window_set_cursor(topLevel, NULL);
			makeHdrData(settings.headers.display);
		break;

		case LOGSCR:
			states.currentScreen = MAIN;
			gtk_widget_hide(logSW[LOGFILESW]);
			gtk_widget_hide(logSW[LOGTRCSW]);
			gtk_widget_show(logSW[settings.logs.dispType]);
			settings.logs.display = states.trace.numSelected ? SELECTED : DISPLAYED;
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (LOGdisp[settings.logs.display]), TRUE);
			states.reset.cursor = TRUE;
			gtk_widget_show(buttons[new_page]);
			gdk_window_set_cursor(topLevel, NULL);
			logClear();
			makeLOGData(settings.logs.dispType, settings.logs.display);
		break;
	}
	
	states.currentPage = new_page;
	memset(&states.user, 0, sizeof(states.user));
}

static char *makeFiltStr(char *tFormStr, int filter)
{
	char *filtName = settings.general.filter.filts[filter]->name;
	char *retStr = g_strdup_printf("%s : FILT(%s)", tFormStr, filtName);
	free(tFormStr);
	return (retStr);
}

static char *makeTFormStr(char *tFormStr, int tForm)
{
	char *retStr = g_strdup_printf("%s : %s", tFormStr, pqlTransforms[tForm]);
	free(tFormStr);
	return (retStr);
}

static void freeTFormData(DATAINFO *dataInfo)
{
	if (dataInfo->data)
		free(dataInfo->data);
	if (dataInfo->tFormStr)
		free(dataInfo->tFormStr);
	memset(dataInfo, 0, sizeof(DATAINFO));
}

static void makeTFormData(traceInfo *trcData, gboolean orig)
{
	float	*tformData;
	DATAINFO	*dataInfo = &trcData->data.tForm;
	
	if (orig && dataInfo)
	{
		freeTFormData(dataInfo);
	}
	
	tformData = (float *) trcData->data.tForm.data;

	if (!tformData)
	{	// copy the data from main to transform array
		DATAINFO	*dataInfo;
		int		i;
		gint32	*ptr = (gint32*) trcData->data.dataInfo.data;
		short	*sptr = (short*) trcData->data.dataInfo.data;
		float	*lptr = (float*) trcData->data.dataInfo.data;
		double	*dptr = (double*) trcData->data.dataInfo.data;
		int		numSamples = trcData->data.dataInfo.head.numSamples;
		int		dataFormat = trcData->data.head.data_form;
		int 	memSize = numSamples * dsizes[dataFormat];
		
		dataInfo = &trcData->data.tForm;
		trcData->data.tForm.data = malloc(memSize);
		tformData = (float *) trcData->data.tForm.data;
		dataInfo->head.numSamples = numSamples;
		dataInfo->head.data_form = MY_FLOAT;
		dataInfo->head.sampInt = trcData->data.head.sampInt;
		dataInfo->head.secsPsamp = (trcData->data.head.sampInt/ 1000000.0);
		dataInfo->tFormStr = g_strdup("Original");
		for (i = 0; i < numSamples; i++)
		{	// copy the data, converting to float, i.e., all t-forms happen in float-world
			DATAP(dataFormat, i, tformData[i]);
		}
	}
}

gboolean	idleFilter(void *nothing)
{	// called by when filter needs to be applied (via g_idle_add())
	applyFilter(states.currentPage);
	return FALSE;		// execute only once per call, i.e., don't call us again idly
}

void tFormDisp(GtkToggleButton *button, gpointer scr)
{
	int screen = GPOINTER_TO_INT(scr);
	
	states.tForm.on = !gtk_toggle_button_get_active(button);
	states.reset.reset[MAGNIFYSCR] = states.reset.reset[SPECTRASCR] = TRUE;
	states.split.resetPixmaps = TRUE;
	switch(screen)
	{
		case MAGNIFYSCR:
		case SPECTRASCR:
			makePixmap(screen);
		break;

		case SPLITSCR:
			switch(states.currentScreen)
			{
				case SPLIT2:
					if (settings.split.DA[states.split.panes][MID] == TRACESCR)
					{	//trace screen is not visible, draw it invisibly
						makePixmap(SPLITDA2M);
					}
					makePixmap(SPLITDA2T);
					makePixmap(SPLITDA2B);
					states.split.refreshPixmap[SPLIT3][TRACESCR] = TRUE;
					states.split.refreshPixmap[SPLIT3][MAGNIFYSCR] = TRUE;
					states.split.refreshPixmap[SPLIT3][SPECTRASCR] = TRUE;
				break;

				case SPLIT3:
					makePixmap(settings.split.DT[SPLIT3][TRACESCR]);
					makePixmap(settings.split.DT[SPLIT3][MAGNIFYSCR]);
					makePixmap(settings.split.DT[SPLIT3][SPECTRASCR]);
					states.split.refreshPixmap[SPLIT2][TRACESCR] = TRUE;
					states.split.refreshPixmap[SPLIT2][MAGNIFYSCR] = TRUE;
					states.split.refreshPixmap[SPLIT2][SPECTRASCR] = TRUE;
				break;
			}
		break;
	}
	setRadios(FILTERTYPE);
}

void applyFilter(int scr)
{	// callback on user selection transform ON|OFF, or from idleFilter()
	int 		i, succ=PQLXSUCCESS, numSamples;
	DATAINFO	*dataInfo;
	GSList 		*trcIter;
	traceInfo	*trcData;

	if (states.tForm.filter == NONE)		// no filter selected
	{
		states.tForm.on = FALSE;
		setRadios(FILTERTYPE);				// set all filter widgets equal
		return;
	}

	gdk_window_set_cursor(topLevel, hourCursor);
	while (g_main_context_iteration(NULL, FALSE));

	if (states.tForm.on)
	{
		if (!states.trace.numSelected)
		{
			dispMsg(DISPLAY, "No TRACES Selected");
			gdk_beep();
			states.tForm.on = FALSE;				// no filter has been applied
//			states.filter.newFilter[states.tForm.filter] = TRUE;			// reset for next application
			succ = FAILURE;
		}

		if (states.tForm.filter != settings.general.filter.AGCidx &&
			(!settings.general.filter.filts[states.tForm.filter]->poles[HIGHF] && 
			!settings.general.filter.filts[states.tForm.filter]->poles[LOWF]))
		{
			dispMsg(DISPLAY, "Filter Poles are 0, Filter Not Applied.  Please Define via Manage...");
			gdk_beep();
			states.tForm.on = FALSE;								// no filter has been applied
//			states.filter.newFilter[states.tForm.filter] = TRUE;	// reset for next application
			succ = FAILURE;
		}

		for (i = 0, trcIter =  (states.magnify.selection 
								? states.magnify.selection 
								: states.trace.selection);
			 trcIter && succ==PQLXSUCCESS;
			 trcIter = g_slist_next(trcIter), i++)
		{
			trcData = trcIter->data;
			numSamples = trcData->data.dataInfo.head.numSamples;

#if 0
    		if (!trcData->data.dataFilt)
			{	// make a list of DATAINFO elements for all filters
				for(j=0;j<settings.general.filter.numFilters;j++)
				{
					dataInfo = calloc(1, sizeof(DATAINFO));
					trcData->data.dataFilt = g_slist_append(trcData->data.dataFilt, dataInfo);
				}
			}
			
			dataInfo = g_slist_nth_data(trcData->data.dataFilt, states.tForm.filter);
			if (!dataInfo->data)
			{	// make the data buffer if it doesn't already exist
				char *dataBuf;
				if ((dataBuf = (char *) calloc(numSamples, sizeof(float))) == NULL) 
				{
		  			fprintf(stderr,"Out of memory, quitting.\n");
		  			gtk_main_quit();
				}
				dataInfo->data = dataBuf;
//				states.filter.newFilter[states.tForm.filter] = TRUE;
    		}
#endif

			makeTFormData(trcData, TRUE);
			dataInfo = &trcData->data.tForm;
			
			succ = PQLXSUCCESS;
			if (states.tForm.filter == settings.general.filter.AGCidx)
			{
				trcData->data.dataInfo.head.secsPsamp = (trcData->data.head.sampInt/ 1000000.0);
				trcData->data.dataInfo.head.numSamples = numSamples;
				trcData->data.dataInfo.head.data_form = trcData->data.head.data_form;
				succ = agc(dataInfo);
			}
			else
			{
				succ = _iir(&trcData->data.dataInfo, dataInfo, states.tForm.filter);
			}

			if (succ != PQLXSUCCESS)
			{
				dispMsg(DISPLAY, states.msg);
				gdk_beep();
				states.tForm.on = FALSE;				// no filter has been applied
			}
			else
			{
				dataInfo->tFormStr = makeFiltStr(dataInfo->tFormStr, states.tForm.filter);
			}
			trcData->magnify.selected = FALSE;
		}
  	}

	makeMAGSelectionList(FALSE);
	if (succ == PQLXSUCCESS)
	{
		states.reset.reset[MAGNIFYSCR] = states.reset.reset[SPECTRASCR] = TRUE;
		states.split.resetPixmaps = TRUE;
		switch(scr)
		{
			case MAGNIFYSCR:
			case SPECTRASCR:
				makePixmap(scr);
			break;
	
			case SPLITSCR:
				switch(states.currentScreen)
				{
					case SPLIT2:
						if (settings.split.DA[states.split.panes][MID] == TRACESCR)
						{	//trace screen is not visible, draw it invisibly
							makePixmap(SPLITDA2M);
						}
						makePixmap(SPLITDA2T);
						makePixmap(SPLITDA2B);
						states.split.refreshPixmap[SPLIT3][TRACESCR] = TRUE;
						states.split.refreshPixmap[SPLIT3][MAGNIFYSCR] = TRUE;
						states.split.refreshPixmap[SPLIT3][SPECTRASCR] = TRUE;
					break;
	
					case SPLIT3:
						makePixmap(settings.split.DT[SPLIT3][TRACESCR]);
						makePixmap(settings.split.DT[SPLIT3][MAGNIFYSCR]);
						makePixmap(settings.split.DT[SPLIT3][SPECTRASCR]);
						states.split.refreshPixmap[SPLIT2][TRACESCR] = TRUE;
						states.split.refreshPixmap[SPLIT2][MAGNIFYSCR] = TRUE;
						states.split.refreshPixmap[SPLIT2][SPECTRASCR] = TRUE;
					break;
				}
			break;
		}
	}
	setRadios(FILTERTYPE);						// set all filter widgets equal
	gdk_window_set_cursor(topLevel, NULL);
}

void newSplit(GtkToggleButton *button, gboolean *splits)
{	// callback for 2 or 3 split screens to display

	if (!gtk_toggle_button_get_active((GtkToggleButton *) button))
		return;				// only interested in the button that has been set, not the one that's unset
		
	gdk_window_set_cursor(topLevel, hourCursor);
	while (g_main_context_iteration(NULL, FALSE));

	if ((GtkWidget *) button == (GtkWidget *) Hsplit[SPLIT2])
		*splits = SPLIT2;
	else
		*splits = SPLIT3;

	switch(*splits)
	{
		case SPLIT2:
			gtk_widget_hide(HMidFrame);
			gtk_widget_hide(split3);
			gtk_widget_show(split2);
			states.currentScreen = SPLIT2;
			if (settings.split.DA[states.split.panes][MID] == TRACESCR)
			{	//trace screen is not visible, draw it invisibly
				makePixmap(SPLITDA2M);
			}
			gtk_combo_box_set_active(GTK_COMBO_BOX(Hcbo[SPLITTOP]), settings.split.DA[states.split.panes][TOP]);
			gtk_combo_box_set_active(GTK_COMBO_BOX(Hcbo[SPLITBOT]), settings.split.DA[states.split.panes][BOT]);
			makePixmap(SPLITDA2T);
			states.reset.cursor=TRUE;
			makePixmap(SPLITDA2B);
		break;

		case SPLIT3:
			gtk_widget_hide(split2);
			gtk_widget_show(split3);
			gtk_widget_show(HMidFrame);
			gtk_combo_box_set_active(GTK_COMBO_BOX(Hcbo[SPLITTOP]), settings.split.DA[states.split.panes][TOP]);
			gtk_combo_box_set_active(GTK_COMBO_BOX(Hcbo[SPLITMID]), settings.split.DA[states.split.panes][MID]);
			gtk_combo_box_set_active(GTK_COMBO_BOX(Hcbo[SPLITBOT]), settings.split.DA[states.split.panes][BOT]);
			states.currentScreen = SPLIT3;
			makePixmap(settings.split.DT[SPLIT3][TRACESCR]);
			makePixmap(settings.split.DT[SPLIT3][MAGNIFYSCR]);
			states.reset.cursor=TRUE;
			makePixmap(settings.split.DT[SPLIT3][SPECTRASCR]);
		break;
	}
}

void pqlComboSel(GtkComboBox *combo, gpointer which)
{
	int sel = gtk_combo_box_get_active(combo),
		scr = gtk_notebook_get_current_page((GtkNotebook *) data_screens),
		comboBox = GPOINTER_TO_INT(which);
		
	switch(scr)
	{
		case TRACESCR:
			switch(comboBox)
			{
				case TRCWSCALE:
					settings.trace.amp = sel;
				break;
				case TRCXAXIS:
					settings.trace.time = sel;
					states.reset.reset[TRACESCR] = TRUE;
				break;
#if 0
				case TRCDISPU:
					settings.general.units = sel;
				break;
#endif
			}
		break;
		case MAGNIFYSCR:
			switch(comboBox)
			{
				case MAGTSCALE:
					settings.magnify.Tamp = sel;
					switch (settings.magnify.Tamp)
					{
						case FIXEDS:
							gtk_widget_show(MTfixedB[MAX]);
							gtk_widget_show(MTfixedB[MIN]);
							gtk_widget_grab_focus(MTfixed[MAX]);
//							makePixmap(MAGNIFYSCR);		// re-plot using new setting
						break;

						default:
							gtk_widget_hide(MTfixedB[MAX]);
							gtk_widget_hide(MTfixedB[MIN]);
//							makePixmap(MAGNIFYSCR);		// re-plot using new setting
						break;
					}
				break;
				case MAGWSCALE:
					settings.magnify.Wamp = sel;
				break;
#if 0
				case MAGDISPU:
					settings.general.units = sel;
				break;
#endif
				case MAGZANCHOR:
					settings.magnify.anchor = sel;
				break;
				case MAGPICKING:
					settings.magnify.pick = sel;
				break;
			}
		break;
		case SPECTRASCR:
			switch(comboBox)
			{
				case SPECWSCALE:
					settings.spectra.display.amp = sel;
				break;
#if 0
				case SPECDISPU:
					settings.general.units = sel;
				break;
#endif
			}
		break;
		case SPLITSCR:
			switch(comboBox)
			{
				case SPLITTOP:
					settings.split.DA[states.split.panes][TOP] = sel;
				break;
				case SPLITMID:
					settings.split.DA[SPLIT3][MID] = sel;
				break;
				case SPLITBOT:
					settings.split.DA[states.split.panes][BOT] = sel;
				break;
				case SPLITTDISP:
					settings.split.traceDisp = sel;
					states.split.refreshPixmap[SPLIT2][TRACESCR] =
					states.split.refreshPixmap[SPLIT3][TRACESCR] = TRUE;
				break;
				case SPLITTSCALE:
					settings.magnify.Tamp = sel;
					switch (settings.magnify.Tamp)
					{
						case FIXEDS:
							gtk_widget_show(HTfixedB[MAX]);
							gtk_widget_show(HTfixedB[MIN]);
							gtk_widget_grab_focus(HTfixed[MAX]);
						break;
						default:
							gtk_widget_hide(HTfixedB[MAX]);
							gtk_widget_hide(HTfixedB[MIN]);
						break;
					}
					states.split.refreshPixmap[SPLIT2][MAGNIFYSCR] =
					states.split.refreshPixmap[SPLIT3][MAGNIFYSCR] = TRUE;
				break;
				case SPLITWSCALE:
					settings.magnify.Wamp = sel;
				break;
			}
		break;
	}

	// replot with new rules
	switch(scr)
	{
		case TRACESCR:
		case MAGNIFYSCR:
		case SPECTRASCR:
			makePixmap(scr);
		break;
		case SPLITSCR:
			settings.spectra.display.overlay = settings.magnify.overlay;
			settings.spectra.display.amp = settings.magnify.Wamp;
			reMag(NULL, GINT_TO_POINTER(scr));
		break;
	}
}

void radioB_chg(GtkToggleButton *button, gboolean *data)
{	// callback for all toggle buttons appearing on the screens
	int			screen;
	gboolean	prevOL=FALSE;
	
	screen = gtk_notebook_get_current_page((GtkNotebook *) data_screens);

	switch(screen)
	{
		case MAGNIFYSCR:
		case SPLITSCR:
			prevOL = settings.magnify.overlay;
		break;
	}
	*data = !gtk_toggle_button_get_active(button);

	if (settings.magnify.overlay &&
		prevOL != settings.magnify.overlay &&
		settings.magnify.Wamp==TRUE)	// Wamp=TRUE is scale by Trace
	{	// set the Amplitude Scaling to Window when switching to overlay
		g_signal_handler_block(Mcbo[MAGWSCALE], signals[WNDWSCALESGNL][MAGSIGNAL+MAXSCRNSGNLS*0]);
		g_signal_handler_block(Scbo[SPECWSCALE], signals[WNDWSCALESGNL][SPECSIGNAL+MAXSCRNSGNLS*0]);
		g_signal_handler_block(Hcbo[SPLITWSCALE], signals[WNDWSCALESGNL][SPLITSIGNAL+MAXSCRNSGNLS*0]);

		settings.magnify.Wamp = WSCALEWIND;
		gtk_combo_box_set_active(GTK_COMBO_BOX(Mcbo[MAGWSCALE]), settings.magnify.Wamp);
		gtk_combo_box_set_active(GTK_COMBO_BOX(Scbo[SPECWSCALE]), settings.spectra.display.amp);
		gtk_combo_box_set_active(GTK_COMBO_BOX(Hcbo[SPLITWSCALE]), settings.magnify.Wamp);

		g_signal_handler_unblock(Mcbo[MAGWSCALE], signals[WNDWSCALESGNL][MAGSIGNAL+MAXSCRNSGNLS*0]);
		g_signal_handler_unblock(Scbo[SPECWSCALE], signals[WNDWSCALESGNL][SPECSIGNAL+MAXSCRNSGNLS*0]);
		g_signal_handler_unblock(Hcbo[SPLITWSCALE], signals[WNDWSCALESGNL][SPLITSIGNAL+MAXSCRNSGNLS*0]);
	}
	
	// replot with new rules
	switch(screen)
	{
		case TRACESCR:
		case MAGNIFYSCR:
		case SPECTRASCR:
			makePixmap(screen);
		break;
		case SPLITSCR:
			settings.spectra.display.overlay = settings.magnify.overlay;
			settings.spectra.display.amp = settings.magnify.Wamp;
			reMag(NULL, GINT_TO_POINTER(screen));
		break;
	}
}

void hdr_chg(GtkWidget *button, int *header)
{
	int i;

	if (!gtk_toggle_button_get_active((GtkToggleButton *) button))
		return;				// only interested in the button that has been set, not the one that's unset

	for(i=0;i<3;i++)
	{
		if(button == HDRdisp[i])
		{
			break;
		}
	}
	*header = i;
	makeHdrData(*header);
}

static void logClear()
{
	GtkTextBuffer *buffer = states.logs.buffer;
	GtkTextIter iter, endIter;

	if (!buffer)
		return;
	
	gtk_text_buffer_get_iter_at_offset(buffer, &endIter, -1);
	gtk_text_buffer_get_iter_at_offset(buffer, &iter, 0);
	if (gtk_text_iter_get_offset(&endIter) > 0)
		gtk_text_buffer_delete(buffer, &iter, &endIter);
}

void log_chg(GtkWidget *button, gpointer t)
{
	int i, type = GPOINTER_TO_INT(t);

	if (!gtk_toggle_button_get_active((GtkToggleButton *) button))
		return;				// only interested in the button that has been set, not the one that's unset

	switch(type)
	{
		case LOGDISPBY:
			for(i=0;i<TTLLOGTYPES;i++)
			{
				if(button == LOGdispT[i])
				{
					break;
				}
			}
			gtk_widget_hide(logSW[settings.logs.dispType]);
			settings.logs.dispType = i;
			gtk_widget_show(logSW[settings.logs.dispType]);
		break;
		
		case LOGONDISP:
			for(i=0;i<TTLDISPLAYTYPES;i++)
			{
				if(button == LOGdisp[i])
				{
					break;
				}
			}
			settings.logs.display = i;
		break;
	}
	logClear();
	makeLOGData(settings.logs.dispType, settings.logs.display);
}

void setDispGain(int fromScr, int toScr)
{
	free(settings.general.display.gather.gainS[toScr]);
	settings.general.display.gather.gainS[toScr] = 
		g_strdup(settings.general.display.gather.gainS[fromScr]);
	gtk_entry_set_text(GTK_ENTRY(dispGain[toScr]), settings.general.display.gather.gainS[fromScr]);
	settings.general.display.gather.gain[toScr] = settings.general.display.gather.gain[fromScr];
}

void reDraw(GtkWidget *entry, gpointer s)
{	// display gain has changed, reset and redraw
	int		scr = GPOINTER_TO_INT(s);
	free(settings.general.display.gather.gainS[scr]);
	settings.general.display.gather.gainS[scr] = g_strdup(gtk_entry_get_text(GTK_ENTRY(entry)));
	sscanf(settings.general.display.gather.gainS[scr], "%f", &settings.general.display.gather.gain[scr]);
	states.reset.reset[scr] = TRUE;
	switch(scr)
	{
		case TRACESCR:
			states.reset.reset[MAGNIFYSCR] = TRUE;
			setDispGain(TRACESCR, MAGNIFYSCR);
		break;
		case MAGNIFYSCR:
			states.reset.reset[TRACESCR] = TRUE;
			setDispGain(MAGNIFYSCR, TRACESCR);
		break;
	}

	states.reset.cursor = TRUE;
	makePixmap(scr);
}

void reMag(GtkWidget *entry, gpointer s)
{	// on ENTER key in FIXED trace scale max and min entry widgets
	// as well as called programmatically
	
	int		scr = GPOINTER_TO_INT(s);
	
	// handle breeching of absolute bounds
	if (states.magnify.limits.selection.user.left < states.trace.GR[MAIN].user_xmin)
	{
		gdk_beep();
		states.magnify.limits.selection.user.left = states.trace.GR[MAIN].user_xmin;
	}
	if (states.magnify.limits.selection.user.right > states.trace.GR[MAIN].user_xmax)
	{
		gdk_beep();
		states.magnify.limits.selection.user.right = states.trace.GR[MAIN].user_xmax;
	}
	
	states.reset.reset[MAGNIFYSCR] = TRUE;
	states.reset.reset[SPECTRASCR] = TRUE;
	states.split.refreshPixmap[SPLIT2][MAGNIFYSCR] =
	states.split.refreshPixmap[SPLIT3][MAGNIFYSCR] = TRUE;
	states.split.refreshPixmap[SPLIT2][SPECTRASCR] =
	states.split.refreshPixmap[SPLIT3][SPECTRASCR] = TRUE;
	switch(scr)
	{
		case MAGNIFYSCR:
			makePixmap(MAGNIFYSCR);
		break;
		
		case SPLITSCR:
			makePixmap(settings.split.DT[states.split.panes][MAGNIFYSCR]);		// process MAG request
			switch(states.split.panes)
			{
				case SPLIT2:							// redraw SPLIT screens
					if (settings.split.DA[states.split.panes][MID] == TRACESCR)
					{	//trace screen is not visible, draw it invisibly
						makePixmap(SPLITDA2M);
					}
					makePixmap(SPLITDA2T);
					states.reset.cursor = TRUE;
					makePixmap(SPLITDA2B);
				break;

				case SPLIT3:							// redraw SPLIT screens
					makePixmap(SPLITDA3T);
					makePixmap(SPLITDA3M);
					states.reset.cursor = TRUE;
					makePixmap(SPLITDA3B);
				break;
			}
		break;
	}
}

static guint	msgID, msgContext;
void	raise_page(GtkButton *button, int *scr)
{	// called by Cancel, Continue, Restart, and Sort buttons from Controls panel
	if (ctrlWindow)
	{	// might not have been created yet
		gtk_widget_hide(ctrlWindow);
		gtk_grab_remove(ctrlWindow);
	}

	states.currentScreen = MAIN;
  	gtk_notebook_set_current_page((GtkNotebook *) data_screens, *scr);
	states.currentPage = *scr;
	switch(*scr)
	{
		case SPLITSCR:
			states.currentScreen = states.split.panes;
			switch(states.split.panes)
			{
				case SPLIT2:
					if (settings.split.DA[states.split.panes][MID] == TRACESCR)
					{	//trace screen is not visible, draw it invisibly
						makePixmap(SPLITDA2M);
					}
					makePixmap(SPLITDA2T);
					makePixmap(SPLITDA2B);
					break;
	
				case SPLIT3:
					makePixmap(SPLITDA3T);
					makePixmap(SPLITDA3M);
					makePixmap(SPLITDA3B);
					break;
			}
		break;

		case TRACESCR:
		case MAGNIFYSCR:
		case SPECTRASCR:
			makePixmap(*scr);
		break;

		case HEADERSCR:
			makeHdrData(settings.headers.display);
		break;
	}

	if (msgID)
	{
		gtk_statusbar_remove((GtkStatusbar *) statusBarCtrl, msgContext, msgID);
		msgID = 0;
	}

	return;
}

void save_defaults(GtkButton *button, plot_settings *save)
{
	char	msg[255];

	get_settings(NULL, NULL);
	make_defaults_file(save, DEFAULTSET);

	if(!msgContext)
		msgContext = gtk_statusbar_get_context_id((GtkStatusbar *) statusBarCtrl, "CTRLSCREENS");

	sprintf(msg, "Defaults File Saved");
	msgID = gtk_statusbar_push((GtkStatusbar *) statusBarCtrl, msgContext, msg);
}

static void setMagBounds(int type)
{
	double		diff, diffL=0, diffR=0, ratio, middle;
	traceInfo 	*trcData = states.trace.selection->data;

	switch(type)
	{
		case ZOOMOUT:
			diff = (states.magnify.limits.selection.user.right - states.magnify.limits.selection.user.left);
			diffL = (diff / (100/settings.magnify.zoomFactor)) * factors[settings.magnify.anchor*2];
			diffR = (diff / (100/settings.magnify.zoomFactor)) * factors[settings.magnify.anchor*2+1] * -1;
		break;
		case ZOOMIN:
			diff = (states.magnify.limits.selection.user.right - states.magnify.limits.selection.user.left);
			diffL = (diff / (100/settings.magnify.zoomFactor)) * factors[settings.magnify.anchor*2] * -1;
			diffR = (diff / (100/settings.magnify.zoomFactor)) * factors[settings.magnify.anchor*2+1];
		break;
	}

	switch(type)
	{
		case ZOOMABS:
			states.magnify.limits.selection.user.left = 
								PIX2USERX(&states.trace.GR[states.currentScreen], 
												states.magnify.limits.selection.pix.left);
			states.magnify.limits.selection.user.right = 
								PIX2USERX(&states.trace.GR[states.currentScreen],
												states.magnify.limits.selection.pix.right);
		break;
		case ZOOMX:
			ratio = (float) (states.magnify.limits.selection.pix.right - 			
									states.magnify.limits.selection.pix.left) /
					(float) (states.magnify.limits.margin[states.currentScreen].right - 
									states.magnify.limits.margin[states.currentScreen].left);
			ratio = 1. - ratio;
			diff = PIX2USERX(&trcData->magnify.GR[states.currentScreen], 
						states.magnify.limits.selection.pix.right) -
					PIX2USERX(&trcData->magnify.GR[states.currentScreen],
						states.magnify.limits.selection.pix.left);
			middle = PIX2USERX(&trcData->magnify.GR[states.currentScreen], 
							states.magnify.limits.selection.pix.right) - diff/2.;
							
			diff = (states.magnify.limits.selection.user.right -
						states.magnify.limits.selection.user.left);
			diff += ratio*diff;
			states.magnify.limits.selection.user.right = middle + diff/2.;
			states.magnify.limits.selection.user.left = middle - diff/2.;

			if (states.magnify.limits.selection.user.right > states.trace.relTimeLen)
				states.magnify.limits.selection.user.right = states.trace.relTimeLen;
			if (states.magnify.limits.selection.user.left < 0)
				states.magnify.limits.selection.user.left = 0;
		break;
		case ZOOMOUT:
		case ZOOMIN:
			states.magnify.limits.selection.user.left += diffL;
			states.magnify.limits.selection.user.right += diffR;
		break;
	}

	// handle breeching of absolute bounds
	if (states.magnify.limits.selection.user.left < states.trace.GR[MAIN].user_xmin)
	{
		states.magnify.limits.selection.user.left = states.trace.GR[MAIN].user_xmin;
	}
	if (states.magnify.limits.selection.user.right > states.trace.GR[MAIN].user_xmax)
	{
		states.magnify.limits.selection.user.right = states.trace.GR[MAIN].user_xmax;
	}
}

void snapMag(GtkWidget *widget, gpointer t)
{
	int		type = GPOINTER_TO_INT(t);
    int		leftFact=0, rightFact=0;
    traceInfo *trcData = states.trace.selection->data;

	if (!states.magnify.limits.selection.pix.left)
	{
		gdk_beep();
		return;		// only when vertical line(s) specified
	}

	gdk_window_set_cursor(topLevel, hourCursor);
	while (g_main_context_iteration(NULL, FALSE));

	switch(type)
	{
		case SNAP:
			switch(settings.magnify.anchor)
			{
				case LEFT:
					leftFact = 0;
					rightFact = 2;
					break;
		
				case RIGHT:
					leftFact = 2;
					rightFact = 0;
					break;
		
				case MIDDLE:
				case MOUSE:
					leftFact = 1;
					rightFact = 1;
					break;
			}
			states.magnify.limits.selection.user.left = 
					PIX2USERX(&trcData->magnify.GR[states.currentScreen], 
									states.magnify.limits.selection.pix.left) -
					leftFact * (states.magnify.limits.selection.user.length/2);
			if (states.magnify.limits.selection.user.left < states.trace.GR[MAIN].user_xmin)
			{		// left boundary breached?
				states.magnify.limits.selection.user.left = 0;
				states.magnify.limits.selection.user.right = (2.0/leftFact) * 
							PIX2USERX(&trcData->magnify.GR[states.currentScreen], 
									states.magnify.limits.selection.pix.left);
			}
			else
			{		// no breaching, set right boundary
				states.magnify.limits.selection.user.right = 
					PIX2USERX(&trcData->magnify.GR[states.currentScreen], 
									states.magnify.limits.selection.pix.left) +
					rightFact * (states.magnify.limits.selection.user.length/2);
			}
			if (states.magnify.limits.selection.user.right > states.trace.GR[MAIN].user_xmax)
			{	// right boundary breached?
				states.magnify.limits.selection.user.right = states.trace.GR[MAIN].user_xmax;
				states.magnify.limits.selection.user.left  = 
						states.magnify.limits.selection.user.right -
						2 * (states.magnify.limits.selection.user.right - 
						PIX2USERX(&trcData->magnify.GR[states.currentScreen], 
						states.magnify.limits.selection.pix.left));
			}
		break;

		case MAG:	// re-display using bounds on display
			states.magnify.limits.selection.user.left = 
							PIX2USERX(&trcData->magnify.GR[states.currentScreen], 
											states.magnify.limits.selection.pix.left);
			states.magnify.limits.selection.user.right = 
							PIX2USERX(&trcData->magnify.GR[states.currentScreen], 
											states.magnify.limits.selection.pix.right);
		break;
		
		case KEY:	// values set elsewhere, use 'em
		break;
	}
	states.magnify.limits.selection.pix.left = 0;		// reset

	switch(states.currentScreen)
	{
		case MAIN:
			states.reset.cursor = TRUE;
			reMag(NULL, GINT_TO_POINTER(MAGNIFYSCR));
		break;

		default:
			reMag(NULL, GINT_TO_POINTER(SPLITSCR));
		break;
	}
}

gboolean checkScrollBounds(double diffL, double diffR)
{
	gboolean	ret=FALSE;
	if (states.magnify.limits.selection.user.left + diffL < states.trace.GR[MAIN].user_xmin)
	{
		gdk_beep();
		states.magnify.limits.selection.user.left = states.trace.GR[MAIN].user_xmin;
		states.magnify.limits.selection.user.right = states.magnify.limits.selection.user.left + 
							states.magnify.limits.selection.user.length;
		ret=TRUE;
	}
	if (states.magnify.limits.selection.user.right + diffR > states.trace.GR[MAIN].user_xmax)
	{
		gdk_beep();
		states.magnify.limits.selection.user.right = states.trace.GR[MAIN].user_xmax;
		states.magnify.limits.selection.user.left = states.magnify.limits.selection.user.right - 
														states.magnify.limits.selection.user.length;
		ret=TRUE;
	}
	
	return ret;
}

gboolean scrollEvent (GtkWidget *widget, GdkEventScroll *event, gpointer s)
{	// process mouse scroll in display areas

	int			scr = GPOINTER_TO_INT(s);
	int 		zoomDir, scrollDir;
	int			tab, evt=NONE, min;
	float		jumpU;
	double		diff, diffL=0, diffR=0;
	double		zAnchor;
	static gboolean blocked=FALSE;
	static int	topTrace=0;
	traceInfo	*trcData = states.trace.selection->data;

	if (blocked)
		return FALSE;
	blocked = TRUE;

	switch(scr)
	{
		case TRACESCR:
		case MAGNIFYSCR:
		case SPECTRASCR:
			tab = scr;
		break;

		case SPLITDA3T:
		case SPLITDA2T:
			tab = SPLITSCR;
		break;

		case SPLITDA3M:
			tab = SPLITSCR;
		break;

		case SPLITDA3B:
		case SPLITDA2B:
			tab = SPLITSCR;
		break;

		default:
			blocked = FALSE;
			zAnchor = 0;
			return TRUE;		// paranoia check, this should never happen
		break;
	}

	switch(event->direction)
	{
		case GDK_SCROLL_UP:
			min = getMinDisp();
			if (min < 3)
			{
				gdk_beep();
				blocked = FALSE;
				zAnchor = 0;
				return TRUE;
			}
			zoomDir = 1;
			scrollDir = -1;
		break;
		case GDK_SCROLL_DOWN:
			zoomDir = -1;
			scrollDir = 1;
		break;
		default:
			blocked = FALSE;
			zAnchor = 0;
			return TRUE;
		break;
	}

	if (event->y < MAG_T_MARGIN)
	{	// on main x-axis, scroll the screen left/right
		jumpU = (float) (states.magnify.limits.selection.user.length *
							settings.magnify.scrollFactor/100.);
		diffL = diffR = jumpU * scrollDir;
		evt = SCROLL;
	}
	else
	{	// in plot region, zoom in/out
		evt = ZOOM;
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
				zAnchor = PIX2USERX(&trcData->magnify.GR[states.currentScreen], (int) event->x);
				diffL = (diff / (100/settings.magnify.zoomFactor)) *
						((zAnchor-states.magnify.limits.selection.user.left)/(diff/2.)) * zoomDir;
				diffR = (diff / (100/settings.magnify.zoomFactor)) *
						((states.magnify.limits.selection.user.right-zAnchor)/(diff/2.)) * zoomDir * -1;
			break;
		}
	}

	if (event->x < states.magnify.limits.margin[states.currentScreen].left)
	{	// scroll inside label?
		if (event->y < widget->allocation.height - MAG_B_MARGIN && 
			event->y > MAG_T_MARGIN &&
			settings.magnify.overlay)
		{
			evt = REPLOT;		// only inside label area in overlay mode
		}
	}

	switch(evt)
	{
		case REPLOT:						// bring trace to TOP
			topTrace += scrollDir;				
			topTrace = (topTrace >= states.trace.numSelected) 
						? 0 
						:	(topTrace < 0)
							? states.trace.numSelected - 1
							: topTrace;
			states.user.topRec.magnify = topTrace;
			mag_plot_files(widget, DAPixmap[scr], widget->allocation.width, 
							widget->allocation.height, states.user.topRec.magnify);

			gtk_widget_queue_draw_area(widget, 
						states.magnify.limits.margin[states.currentScreen].left,
						MAG_T_MARGIN, 
						states.magnify.limits.margin[states.currentScreen].length,
						widget->allocation.height - (MAG_T_MARGIN+MAG_B_MARGIN));
		break;

		case SCROLL:
			if (checkScrollBounds(diffL, diffR)==TRUE)
			{	// don't go past the ends
				reMag(NULL, GINT_TO_POINTER(tab));
				break;
			}
			//	fall-through!	
		case ZOOM:
			states.magnify.limits.selection.user.left += diffL;
			states.magnify.limits.selection.user.right += diffR;
			reMag(NULL, GINT_TO_POINTER(tab));
			while (gtk_events_pending())
				gtk_main_iteration ();
		break;
	}

	blocked = FALSE;
	return TRUE;
}

gboolean ctrlKeyEvent(GtkWidget *widget, GdkEventKey *event, gpointer s)
{
	int			scr = GPOINTER_TO_INT(s);
	guint		modifiers;
	int			ctrlKey;
	gboolean	retVal = TRUE;

	if (scr != MAGNIFYSCR)
		return retVal;
		
	modifiers = gtk_accelerator_get_default_mod_mask();
	ctrlKey = ((event->state & modifiers) == GDK_CONTROL_MASK) ? CTRL : NONE;
	
	switch (ctrlKey)
	{
		case CTRL:
			if (event->keyval == GDK_z &&
				event->type == GDK_KEY_RELEASE)
			{
				pqlTFormSel(NULL, GINT_TO_POINTER(TRUE));
				retVal = FALSE;			// event processed, do not process further
			}
		break;
	}
	
	return retVal;
}

gboolean keyEvent (GtkWidget *widget, GdkEventKey *event, gpointer s)
{	// process key press in display areas
	int			scr = GPOINTER_TO_INT(s);
	int			x, y, tab, min;
	int 		screen, zoomDir, scrollDir;
	float		jumpU;
	double		diff, diffL=0, diffR=0;
	double		zAnchor;	
	static	GdkWindow *gdkWindow;
	GdkModifierType state;
	static gboolean	blocked=FALSE;
	traceInfo	*trcData;
			
	if (blocked ||
		!states.trace.selection)
		return TRUE;
	blocked = TRUE;

	if (!gdkWindow)
		gdkWindow = gdk_get_default_root_window();

	trcData = states.trace.selection->data;
	switch(scr)
	{
		case TRACESCR:
		case MAGNIFYSCR:
		case SPECTRASCR:
			screen = scr;
			tab = scr;
		break;

		case SPLITDA3T:
		case SPLITDA2T:
			screen = settings.split.DA[states.split.panes][TOP];
			tab = SPLITSCR;
		break;

		case SPLITDA3M:
			screen = settings.split.DA[states.split.panes][MID];
			tab = SPLITSCR;
		break;

		case SPLITDA3B:
		case SPLITDA2B:
			screen = settings.split.DA[states.split.panes][BOT];
			tab = SPLITSCR;
		break;

		default:
			blocked = FALSE;
			return TRUE;		// paranoia check, this should never happen
		break;
	}

	if ((screen != MAGNIFYSCR) || 						// only mag screen
		(event->type == GDK_KEY_RELEASE)) 				// only on key release
	{
		switch(states.user.keyActive)
		{
			case ZKEY:
				if (states.magnify.limits.selection.pix.right)
				{
					snapMag(NULL, GINT_TO_POINTER(MAG));
				}
			break;
			case XKEY:
				if (states.magnify.limits.selection.pix.right)
				{
					setMagBounds(ZOOMX);
					snapMag(NULL, GINT_TO_POINTER(KEY));
				}
			break;
		}
		states.user.keyActive = NONE;
		blocked = FALSE;
		return TRUE;
	}

	switch(event->keyval)
	{
		case GDK_Up:
			min = getMinDisp();
			if (min < 3)
			{	// don't zoom in farther than possible
				gdk_beep();
				blocked = FALSE;
				return TRUE;
			}
		// fall-through!
		case GDK_Right:
			zoomDir = 1;
			scrollDir = 1;
		break;
		case GDK_Down:
		case GDK_Left:
			zoomDir = -1;
			scrollDir = -1;
		break;

		// all modifier keys (z, x, 0-9) are only registered here, plotting occurs elsewhere
		case GDK_z:
			states.user.keyActive = ZKEY;
			blocked = FALSE;
			return TRUE;
		break; 
		case GDK_x:
			states.user.keyActive = XKEY;
			blocked = FALSE;
			return TRUE;
		break;

		case GDK_1:
		case GDK_2:
		case GDK_3:
		case GDK_4:
		case GDK_5:
		case GDK_6:
		case GDK_7:
		case GDK_8:
		case GDK_9:
			states.magnify.pick.curPick = (event->keyval == GDK_0) ? 10 : (int) (event->keyval - GDK_0);
			blocked = FALSE;
			return TRUE;
		break;

		default:
			states.user.keyActive = NONE;
			blocked = FALSE;
			return TRUE;
		break;
	}

	switch(event->keyval)
	{
		case GDK_Up:
		case GDK_Down:
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
					gdk_window_get_pointer(gdkWindow, &x, &y, &state);
					zAnchor = PIX2USERX(&trcData->magnify.GR[states.currentScreen], (int) x);
					diffL = (diff / (100/settings.magnify.zoomFactor)) *
							((zAnchor-states.magnify.limits.selection.user.left)/(diff/2.)) * zoomDir;
					diffR = (diff / (100/settings.magnify.zoomFactor)) *
							((states.magnify.limits.selection.user.right-zAnchor)/(diff/2.)) * zoomDir * -1;
				break;
			}
		break;
		case GDK_Right:
		case GDK_Left:
			jumpU = (float) (states.magnify.limits.selection.user.length *
								settings.magnify.scrollFactor/100.);
			diffL = diffR = jumpU * scrollDir;
		break;
	}

	switch(event->keyval)
	{
		case GDK_Right:
		case GDK_Left:
			if (checkScrollBounds(diffL, diffR)==TRUE)
			{
				reMag(NULL, GINT_TO_POINTER(tab));
				break;
			}
			// fall-through!		
		case GDK_Up:
		case GDK_Down:
			states.magnify.limits.selection.user.left += diffL;
			states.magnify.limits.selection.user.right += diffR;
			reMag(NULL, GINT_TO_POINTER(tab));
		break;
	}

	blocked = FALSE;
	return TRUE;
}

gboolean mouseEvent(GtkWidget *widget, GdkEventButton *event, gpointer s)
{	// callback of all mouse events, all screens
	int		scr = GPOINTER_TO_INT(s);
	int		screen;
	guint	modifiers;

	if (states.user.keyActive != DKEY &&
		states.user.keyActive != ZKEY &&
		states.user.keyActive != XKEY )
	{
		modifiers = gtk_accelerator_get_default_mod_mask();
		states.user.keyActive = ((event->state & modifiers) == GDK_CONTROL_MASK) ? CTRL : NONE;
		if (states.user.keyActive == NONE)
			states.user.keyActive = ((event->state & modifiers) == GDK_SHIFT_MASK) ? SHIFT : NONE;
		if (states.user.keyActive == NONE)
			states.user.keyActive = ((event->state & modifiers) == (GDK_SHIFT_MASK + GDK_MOD1_MASK)) ? ALT : NONE;
	}
	
	switch(scr)
	{
		case TRACESCR:
		case MAGNIFYSCR:
		case SPECTRASCR:
			screen = scr;
		break;

		case SPLITDA3T:
		case SPLITDA2T:
			screen = settings.split.DA[states.split.panes][TOP];
		break;

		case SPLITDA3M:
			screen = settings.split.DA[states.split.panes][MID];
		break;

		case SPLITDA3B:
		case SPLITDA2B:
			screen = settings.split.DA[states.split.panes][BOT];
		break;

		default:
			return TRUE;		// paranoia check, this should never happen
	}

	doMouse(widget, event, DAPixmap[scr], screen);			// process the mouse event

	if (event->type == GDK_BUTTON_RELEASE)					// refresh screen(s) where necessary
	{
		switch(scr)
		{
			case MAGNIFYSCR:
			case SPECTRASCR:
			case TRACESCR:
				gdk_window_set_cursor(topLevel, hourCursor);
				while (g_main_context_iteration(NULL, FALSE));
				states.reset.cursor = TRUE;
				if (states.user.keyActive != DKEY &&
					states.user.keyActive != ZKEY &&
					states.user.keyActive != XKEY )
				{
					states.user.keyActive = NONE;
				}
				makePixmap(scr);
			break;

			case SPLITDA3T:
			case SPLITDA3M:
			case SPLITDA3B:
				makePixmap(scr);
				if ((states.user.event == MAGBOUNDARY && 
					 states.trace.limits.selection.pix[states.currentScreen].right) ||
					 states.user.event == XAXIS ||
					 states.user.event == ZOOM)
				{	// only replot the other screens when they need to be
					gdk_window_set_cursor(topLevel, hourCursor);
					while (g_main_context_iteration(NULL, FALSE));
					makePixmap(SPLITDA3T);
					makePixmap(SPLITDA3M);
					states.reset.cursor = TRUE;
					makePixmap(SPLITDA3B);
				}
			break;

			case SPLITDA2T:
			case SPLITDA2B:
				makePixmap(scr);
				if ((states.user.event == MAGBOUNDARY && 
					states.trace.limits.selection.pix[states.currentScreen].right) ||
					states.user.event == XAXIS ||
					states.user.event == ZOOM)
				{	// only replot the other screens when they need to be
					gdk_window_set_cursor(topLevel, hourCursor);
					while (g_main_context_iteration(NULL, FALSE));
					if (settings.split.DA[states.split.panes][MID] == TRACESCR)
					{	//trace screen is not visible, draw it invisibly
						makePixmap(SPLITDA2M);
					}
					makePixmap(SPLITDA2T);
					states.reset.cursor = TRUE;
					makePixmap(SPLITDA2B);
				}
			break;
		}
		
		switch(states.user.keyActive)
		{
			case ZKEY:
				if (states.magnify.limits.selection.pix.right)
				{
					snapMag(NULL, GINT_TO_POINTER(MAG));
				}
			break;		
			case XKEY:
				if (states.magnify.limits.selection.pix.right)
				{
					setMagBounds(ZOOMX);
					snapMag(NULL, GINT_TO_POINTER(KEY));
				}
			break;		
		}
	}
	
	return TRUE;
}

#define IN 0
#define OUT 1

static void getXY(GtkWidget *wi, int trcNum, int w, int h, int *x, int *y)
{
	int	diff, dispTraces=0;
	float trcH=0;
	
	switch(states.currentGraph)
	{
		case TRACESCR:
			dispTraces = g_slist_length(states.trace.traces[CURGRP]);
			trcH = (wi->allocation.height - TRC_B_MARGIN)/dispTraces;
		break;
		case MAGNIFYSCR:
			dispTraces = g_slist_length(states.trace.selection);
			trcH = (wi->allocation.height - (MAG_B_MARGIN + MAG_T_MARGIN))/dispTraces;
		break;
		case SPECTRASCR:
			dispTraces = g_slist_length(states.trace.selection);
			trcH = (wi->allocation.height - (MAG_B_MARGIN + MAG_T_MARGIN))/dispTraces;
		break;
	}
	gtk_window_get_position(GTK_WINDOW(topWindow), x, y);

	(*x) += wi->allocation.x + states.trace.plot.label_width[states.currentScreen];
	diff = (int) ((trcNum*2)/dispTraces) ? -h+30 : 5;
	(*y) += wi->allocation.y + (trcNum)*trcH + diff + trcH/2;
}

static void doHedrPopup(GtkWidget *widget, GdkEventMotion *event, int screen)
{
	static GtkTextBuffer *buffer;
	GtkTextIter iter, endIter;
	gint	x, y, trcNum;
	int		w=200, h, numSamps=0, margin=0;
	float	trcH=0, rms=0;
	double	max=0, min=0;
	gchar	*str;
	traceInfo *trc;
	GSList	*trcList=NULL;

	switch(screen)
	{
		case TRACESCR:
			trcList = states.trace.traces[CURGRP];
			trcH = states.trace.trace_depth[states.currentScreen];
			margin = 0;
		break;
		case MAGNIFYSCR:
			trcList = states.trace.selection;
			trcH = states.magnify.trace_depth[states.currentScreen] + MAG_TFORM_MARGIN;
			margin = MAG_T_MARGIN;
		break;
		case SPECTRASCR:
			trcList = states.trace.selection;
			trcH = states.spectra.trace_depth[states.currentScreen];
			margin = MAG_T_MARGIN;
		break;
	}
	trcNum = (event->y-margin) / trcH;
	trc = g_slist_nth_data(trcList, trcNum);
	if (!trc)
		return;
		
	switch(screen)
	{
		case TRACESCR:
			max = trc->data.dataInfo.amps.max;
			min = trc->data.dataInfo.amps.min;
			numSamps = trc->data.dataInfo.head.numSamples;
			rms = trc->trace.RMS;
		break;
		case MAGNIFYSCR:
		{
			double plot_length;
			max = trc->magnify.plot.maxAmp;
			min = trc->magnify.plot.minAmp;
			plot_length = trc->magnify.end - trc->magnify.begin;
			numSamps = (int) ((1000000. / trc->data.head.sampInt) * plot_length);
			rms = trc->magnify.RMS;
		}
		break;
	}

	if (!InfoWin)
	{
		static GtkWidget *InfoView;
		GtkWidget		*frame;
		InfoWin = gtk_window_new(GTK_WINDOW_POPUP);
		frame = gtk_frame_new(NULL);
		gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
		InfoView = gtk_text_view_new();
		gtk_container_add(GTK_CONTAINER(InfoWin), frame);
		gtk_container_add(GTK_CONTAINER(frame), InfoView);
		buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(InfoView));
		gtk_text_buffer_create_tag(buffer, "heading", "justification", GTK_JUSTIFY_CENTER, 
														"family", "monospace", 
														NULL);
		gtk_text_buffer_create_tag(buffer, "text", "family", "monospace", NULL);
	}
	
	gtk_text_buffer_get_iter_at_offset(buffer, &endIter, -1);
	gtk_text_buffer_get_iter_at_offset(buffer, &iter, 0);
	if (gtk_text_iter_get_offset(&endIter) > 0)
		gtk_text_buffer_delete(buffer, &iter, &endIter);

	switch (settings.general.display.mode)
	{
		case GATHERMODE:
			str = g_strdup_printf("Instrument Number: %s ", trc->data.head.sensorID);
			gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, str, -1, 
					"heading", NULL);	free(str	);
			str = g_strdup_printf("\n\n Event Number: %s \n", trc->data.head.channel);
			gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, str, -1, 
					"text", NULL);	free(str);
		break;
		
		case TRACEMODE:
			str = g_strdup_printf("Station: %s \n\n", trc->data.head.descr2);
			gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, str, -1, 
					"heading", NULL);	free(str	);
		break;
	}
	str = g_strdup_printf("    StartTime: %s \n", trc->data.head.descr1);
	gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, str, -1, "text", NULL);	free(str);
	str = g_strdup_printf("  Num Samples: %d \n", numSamps);
	gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, str, -1, "text", NULL);	free(str);
	str = g_strdup_printf("          Max: %.2f \n", max);
	gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, str, -1, "text", NULL);	free(str);
	str = g_strdup_printf("          Min: %.2f \n", min);
	gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, str, -1, "text", NULL);	free(str);
	{
		char	*sps;
		if (trc->data.head.sampInt < 1000000)
			sps = g_strdup_printf("%d", (int) (1000000./ trc->data.head.sampInt));
		else
			sps = g_strdup_printf("%f", (float) (1000000./ trc->data.head.sampInt));
		str = g_strdup_printf("    Samps/Sec: %s \n", sps);
		gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, str, -1, "text", NULL);	free(str);
		free(sps);
	}
	switch (settings.general.unitsConv)
	{
		case FALSE:
			str = g_strdup_printf("        Units: %s \n", unitTypes[trc->data.head.units]);
		break;
		case TRUE:
			str = g_strdup_printf("        Units: %s - Converted \n", unitTypes[trc->data.head.units]);
		break;
	}			
	gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, str, -1, "text", NULL);	free(str);
	str = g_strdup_printf("         Gain: %d \n", trc->data.head.gainConst);
	gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, str, -1, "text", NULL);	free(str);
	str = g_strdup_printf(" Scale Factor: %.2f \n", trc->data.head.scale_fac);
	gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, str, -1, "text", NULL);	free(str);
	str = g_strdup_printf("          RMS: %.1f \n", rms);
	gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, str, -1, "text", NULL);	free(str);
	str = g_strdup_printf("    File Type: %s \n", fileTypes[trc->data.fileType]);
	gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, str, -1, "text", NULL);	free(str);

	h = InfoWin->allocation.height;
	getXY(widget, trcNum, w, h, &x, &y);
	gtk_window_move(GTK_WINDOW(InfoWin), x, y);
	gtk_window_resize(GTK_WINDOW(InfoWin), w, 1);
	gtk_widget_show_all(InfoWin);
}

static void doSOHPopup(GtkWidget *widget, GdkEventMotion *event, int screen)
{
	static GtkTextBuffer *buffer;
	GtkTextIter iter, endIter;
	GSList	*iter1, *iter2;
	gint	x, y, trcNum;
	int		w=200, h, margin=0;
	float	trcH=0;
	gchar	*str;
	traceInfo *trc;
	GSList	*trcList=NULL;
	INSTSOH	*instSOH;

	switch(screen)
	{
		case TRACESCR:
			trcList = states.trace.traces[CURGRP];
			trcH = states.trace.trace_depth[states.currentScreen];
			margin = 0;
		break;
		case MAGNIFYSCR:
			trcList = states.trace.selection;
			trcH = states.magnify.trace_depth[states.currentScreen] + MAG_TFORM_MARGIN;
			margin = MAG_T_MARGIN;
		break;
		case SPECTRASCR:
			trcList = states.trace.selection;
			trcH = states.spectra.trace_depth[states.currentScreen];
			margin = MAG_T_MARGIN;
		break;
	}
	trcNum = (event->y - margin) / trcH;
	trc = g_slist_nth_data(trcList, trcNum);
	if (!trc)
		return;

	if (!(iter1 = g_slist_find_custom(states.data.instSOH, trc, (GCompareFunc) getInstSOH)))
		return;
	instSOH = iter1->data;		

	if (!SOHWin)
	{
		static GtkWidget *SOHView;
		GtkWidget		*frame, *scrollW;
		SOHWin = gtk_window_new(GTK_WINDOW_POPUP);
		frame = gtk_frame_new(NULL);
		gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
    		scrollW = gtk_scrolled_window_new(NULL, NULL);
  		gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW(scrollW), GTK_SHADOW_ETCHED_IN);
  		gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(scrollW),
				      GTK_POLICY_AUTOMATIC,
				      GTK_POLICY_AUTOMATIC);
		SOHView = gtk_text_view_new ();
		gtk_container_add(GTK_CONTAINER(scrollW), SOHView);
		gtk_container_add(GTK_CONTAINER(frame), scrollW);
		gtk_container_add(GTK_CONTAINER(SOHWin), frame);
		buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW(SOHView));
		gtk_text_buffer_create_tag (buffer, "heading", "justification", GTK_JUSTIFY_CENTER, 
														"family", "monospace", 
														NULL);
		gtk_text_buffer_create_tag (buffer, "text", "family", "monospace", NULL);
	}
	
	gtk_text_buffer_get_iter_at_offset(buffer, &endIter, -1);
	gtk_text_buffer_get_iter_at_offset(buffer, &iter, 0);
	if (gtk_text_iter_get_offset(&endIter) > 0)
		gtk_text_buffer_delete(buffer, &iter, &endIter);

	str = g_strdup_printf("Instrument Number: %s \n\n", trc->data.head.sensorID);
	gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, str, -1, "heading", NULL);	free(str);

	for(iter1 = instSOH->SOH; iter1; iter1 = g_slist_next(iter1))
	{
		SOHINFO	*sohInfo;
		sohInfo = iter1->data;
		for(iter2 = sohInfo->msgs; iter2; iter2 = g_slist_next(iter2))
		{
			SOHMSG *sohMsg;
			sohMsg = iter2->data;
			if (sohMsg->year)
				str = g_strdup_printf(" %04d-%03d/%02d:%02d:%02d  %s \n",
								sohMsg->year, sohMsg->jday, sohMsg->hour, 
								sohMsg->minute, sohMsg->second, sohMsg->msg);
			else
				str = g_strdup_printf(" %03d/%02d:%02d:%02d  %s \n",
								sohMsg->jday, sohMsg->hour, 
								sohMsg->minute, sohMsg->second, sohMsg->msg);
			gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, str, 
								-1, "text", NULL);
			if (strlen(str)*8.4 > w)
				w = (strlen(str)*8.4 <
						widget->allocation.width - 30 - 
							states.trace.plot.label_width[states.currentScreen])
					? strlen(str)*8.4
					: widget->allocation.width - 30 - 
							states.trace.plot.label_width[states.currentScreen];
			free(str);
		}
	}

	h = SOHWin->allocation.height;
	getXY(widget, trcNum, w, h, &x, &y);
	gtk_window_move(GTK_WINDOW(SOHWin), x, y);
	gtk_window_resize(GTK_WINDOW(SOHWin), w, 300);
	gtk_widget_show_all(SOHWin);
}

gboolean dragEvent(GtkWidget *widget, GdkEventMotion *event, gpointer s)
{
	int		scr = GPOINTER_TO_INT(s);
	int		screen, x, y;
	GdkModifierType state;
	guint	modifiers;
	static	GdkWindow *gdkWindow;

	if (!gdkWindow)
		gdkWindow = gdk_get_default_root_window();

	switch(scr)
	{
		case TRACESCR:
			if (states.currentScreen != MAIN)
				return TRUE;			// no dragging on SPLIT Trace
			screen = scr;
		break;

		case MAGNIFYSCR:
			screen = scr;
		break;

		case SPECTRASCR:
			screen = scr;
		break;

		case SPLITDA3T:
		case SPLITDA2T:
			screen = settings.split.DA[states.split.panes][TOP];
		break;

		case SPLITDA3M:
			screen = settings.split.DA[states.split.panes][MID];
		break;

		case SPLITDA3B:
		case SPLITDA2B:
			screen = settings.split.DA[states.split.panes][BOT];
		break;

		default:
			return TRUE;		// paranoia check, this should never happen
	}

	if ((event->x <= states.trace.plot.label_width[states.currentScreen]+7 &&
		event->y <= widget->allocation.height - TRC_B_MARGIN) &&
		(screen == TRACESCR || screen == MAGNIFYSCR))
	{	// mouse is in label area, plot an info screen and return
		if (event->x >= states.trace.plot.label_width[states.currentScreen]/2)
		{
			closePopUps(INFOPOPUP);
			doSOHPopup(widget, event, screen);
		}
		else
		{
			closePopUps(SOHPOPUP);
			doHedrPopup(widget, event, screen);
		}
		gdk_window_get_pointer(gdkWindow, &x, &y, &state);
		return TRUE;
	}
	closePopUps(ALLPOPUPS);

	if (states.user.mouseLeft == INACTIVE && 
		states.user.keyActive != DKEY)
	{	// no mouse-hover functionality on SPECTRASCR
		gdk_window_get_pointer(gdkWindow, &x, &y, &state);
		return TRUE;
	}

	modifiers = gtk_accelerator_get_default_mod_mask();
	if (states.user.keyActive != DKEY)
	{
		states.user.keyActive = NONE;
		if ((event->state & modifiers) == GDK_CONTROL_MASK)
		{
			states.user.keyActive = CTRL;
		}
		if ((event->state & modifiers) == GDK_SHIFT_MASK)
		{
			states.user.keyActive = SHIFT;
		}
	}

	doDrag(widget, event, DAPixmap[scr], screen);		// as often as possible

	makePixmap(scr);

	// even though we don't use the resulting information from this call, 
	// it needs to exist since calling it is an indication to the main_loop() 
	// that we are ready to receive the next mouse motion notify event,
	// i.e., while seemingly doing nothing, this call is very, very important!
	gdk_window_get_pointer(gdkWindow, &x, &y, &state);
	return TRUE;
}

gboolean selectT(GtkButton *button, gpointer c)
{	// called by Select All, Deselect All, and Remove buttons on TRACE screen
	int		cmd = GPOINTER_TO_INT(c);
	int 	i;
	gboolean	value=(cmd==SELECT ? TRUE : FALSE);
	GSList 	  *trcIter;
	traceInfo *trcData;

	if (!states.general.numTraces)
		return TRUE;		// don't do anything if there's no files

	for(trcIter = states.trace.traces[CURGRP], i=0;
		trcIter &&
		 (cmd == SELECT || cmd == DESELECT);
		trcIter = g_slist_next(trcIter), i++)
	{
		trcData = trcIter->data;
		trcData->trace.selected = value;
		if (button)	// reprint label only request is from user, i.e., not programmatically
			printLabel(DAPixmap[TRACESCR], trcData, i, states.trace.trace_depth[states.currentScreen], 0, TRACESCR);
	}
	if (button &&
		(cmd == SELECT || cmd == DESELECT))
	{
		gtk_widget_queue_draw_area(DA[TRACESCR], 4, 0, states.trace.plot.label_width[states.currentScreen],
			states.trace.trace_depth[states.currentScreen]*settings.trace.numPlots);
	}

	switch(cmd)
	{
		case SELECT:
			makeSelectionList(FALSE);
			states.split.resetPixmaps = TRUE;
		break;

		case DESELECT:
			makeSelectionList(FALSE);
			states.split.resetPixmaps = TRUE;
			// deselect any boundaries and points selected as well
			memset(&states.trace.point, 0, sizeof(states.trace.point));
			memset(&states.trace.limits.selection, 0, sizeof(states.trace.limits.selection));
			states.split.refreshPixmap[SPLIT2][TRACESCR] =
			states.split.refreshPixmap[SPLIT3][TRACESCR] =
			states.split.refreshPixmap[SPLIT2][MAGNIFYSCR] =
			states.split.refreshPixmap[SPLIT3][MAGNIFYSCR] =
			states.split.refreshPixmap[SPLIT2][SPECTRASCR] =
			states.split.refreshPixmap[SPLIT3][SPECTRASCR] = TRUE;
		break;

		case REMOVE:
			gdk_window_set_cursor(topLevel, hourCursor);
			while (g_main_context_iteration(NULL, FALSE));
			kill_all_traces(SELECTED);
			states.reset.reset[TRACESCR] = TRUE;
			makeDispList(INITGROUP);
		break;
	}

	if (button)
	{
		states.reset.cursor = TRUE;
		makePixmap(TRACESCR);
	}

	return TRUE;
}

gboolean mark(GtkButton *button, void *nothing)
{	// callback from TRACESCR MARK button
	static GtkWidget	*markDialog, *TmarkN;
	GtkWidget			*hbox, *label;
	GtkAdjustment 		*spinAdj;
	GSList				*trcIter;
	traceInfo			*trcData;

	if (!states.trace.numSelected)
	{
		dispMsg(DISPLAY, "No Traces selected, nothing written to MARK File");
		gdk_beep();
		return TRUE;
	}

	if (!settings.general.mark.fileN)
	{
		getFilename(NULL, GINT_TO_POINTER(MARKFTYPE));
	}

	if (!markDialog)
	{
		markDialog = gtk_dialog_new_with_buttons ("WRITE MARK File",
										GTK_WINDOW (topWindow),
										GTK_DIALOG_DESTROY_WITH_PARENT,
										GTK_STOCK_CANCEL, GTK_RESPONSE_NO,
										GTK_STOCK_OK, GTK_RESPONSE_YES,
										NULL);
		
		hbox = gtk_hbox_new(FALSE, 0);
		label = gtk_label_new("MARK File Number:");
		spinAdj = (GtkAdjustment *) gtk_adjustment_new(settings.general.mark.num, 
					1.0, MAXMARKS, 1.0, 1.0, 0.0);
		TmarkN = gtk_spin_button_new(spinAdj, 1.0, 0);
		gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);
		gtk_box_pack_start(GTK_BOX(hbox), TmarkN, FALSE, FALSE, 2);
		gtk_container_add(GTK_CONTAINER(GTK_DIALOG(markDialog)->vbox), hbox);
	}
	else
	{
		gtk_spin_button_set_value((GtkSpinButton *) TmarkN, settings.general.mark.num);
	}
	gtk_widget_show_all(markDialog);

	if (gtk_dialog_run(GTK_DIALOG(markDialog)) == GTK_RESPONSE_YES)
	{	// on OK
		FILE	*fp;
		char	markFile[255], msg[255];

		settings.general.mark.num = gtk_spin_button_get_value((GtkSpinButton *) TmarkN);
		sprintf(markFile, "%s.%d", settings.general.mark.fileN, settings.general.mark.num);
		if ((fp = fopen(markFile, "a")) == NULL)
		{
			sprintf(msg, "Unable to open MARK file %s", markFile);
			dispMsg(DISPLAY, msg);
		}
		else
		{
			for (trcIter = states.trace.selection;
				 trcIter;
				 trcIter = g_slist_next(trcIter))
			{	// print selected filename to markFile
				trcData = trcIter->data;
				fprintf(fp, "%s\n", trcData->data.fileName);
			}
			fclose(fp);
			sprintf(msg, "MARK File %s written", markFile);
			dispMsg(DISPLAY, msg);
		}
	}

	gtk_widget_hide(markDialog);
	
	return TRUE;
}	

void newStart(GtkTreeView *treeview, GtkTreePath *arg1, GtkTreeViewColumn *arg2, gpointer nil)
{	// called when header record double-clicked, i.e., start TRACE display from here
	char	*row;
	int		rowNum;

	row = gtk_tree_path_to_string(arg1);
	rowNum = atoi(row);
	switch (settings.headers.display)
	{
		case ALL:
			states.trace.startRec = rowNum;
		break;
		case DISPLAYED:
			states.trace.startRec += rowNum;
		break;
		case SELECTED:
		{
			traceInfo *trcData = g_slist_nth_data(states.trace.selection, rowNum);
			states.trace.startRec = trcData->data.position.current;
		}
		break;
	}
	g_free(row);

	makeDispList(INITGROUP);
	states.reset.reset[TRACESCR] = TRUE;
	states.general.plot.state = READDATA;
  	gtk_notebook_set_current_page((GtkNotebook *) data_screens, TRACESCR);
}

void writePicks(GtkWidget *widget, gpointer nil)
{
	int	i, j, k, pick, totPicks;
	FILE *fp;
	pointID point, points[2], P, S;
	pickInfo	picks[30];
	char	str[100], *outLabel;
	GtkWidget	*hbox, *label, *entry;
	static GtkWidget	*pickDialog, *MpickFN, *MpickKeys, 
						*MpickComment, *MpickKEYW, *MpickCOMW;
	GtkSizeGroup	*size_group;
	GSList	*trcIter;
	traceInfo	*trcData;

	if (!states.trace.numSelected)
	{
		dispMsg(DISPLAY, "No TRACES Selected, Nothing to Write");
		gdk_beep();
		return;
	}

	if (settings.magnify.pickKEYOn | settings.magnify.pickCOMOn | !settings.magnify.pickFile)
	{
		if (!settings.magnify.pickFile || !settings.magnify.pickFile[0])
		{
			getFilename(NULL, GINT_TO_POINTER(PICKFTYPE));
		}

		if (!pickDialog)
		{
			pickDialog = gtk_dialog_new_with_buttons ("WRITE Pick File",
											GTK_WINDOW (topWindow),
											GTK_DIALOG_DESTROY_WITH_PARENT,
											GTK_STOCK_OK, GTK_RESPONSE_YES,
											NULL);
			size_group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);

			hbox = gtk_hbox_new(FALSE, 0);
			label = gtk_label_new("Pick Filename:");
			MpickFN = entry = gtk_entry_new();
			gtk_entry_set_max_length(GTK_ENTRY(entry), 40);
			gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);
			gtk_box_pack_start(GTK_BOX(hbox), entry, FALSE, FALSE, 2);
			gtk_size_group_add_widget(size_group, label);
			gtk_container_add(GTK_CONTAINER(GTK_DIALOG(pickDialog)->vbox), hbox);
			gtk_entry_set_text(GTK_ENTRY(MpickFN), settings.magnify.pickFile);

			MpickKEYW = hbox = gtk_hbox_new(FALSE, 0);
			label = gtk_label_new("Pick KEYWORDS:");
			MpickKeys = entry = gtk_entry_new();
			gtk_entry_set_max_length(GTK_ENTRY(entry), 40);
			gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);
			gtk_box_pack_start(GTK_BOX(hbox), entry, FALSE, FALSE, 2);
			gtk_size_group_add_widget(size_group, label);
			gtk_container_add(GTK_CONTAINER(GTK_DIALOG(pickDialog)->vbox), hbox);
			
			MpickCOMW = hbox = gtk_hbox_new(FALSE, 0);
			label = gtk_label_new("Pick Commentary:");
			MpickComment = entry = gtk_entry_new();
			gtk_entry_set_max_length(GTK_ENTRY(entry), 80);
			gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);
			gtk_box_pack_start(GTK_BOX(hbox), entry, FALSE, FALSE, 2);
			gtk_size_group_add_widget(size_group, label);
			g_object_unref(size_group);
			gtk_container_add(GTK_CONTAINER(GTK_DIALOG(pickDialog)->vbox), hbox);

			gtk_widget_show_all(pickDialog);
		}

		if (settings.magnify.pickKEYOn)
		{	// get the KEYWORD(S)
			gtk_entry_set_text(GTK_ENTRY(MpickKeys), settings.magnify.pickKEY);
			gtk_widget_show(MpickKEYW);
		}
		else
		{
			gtk_entry_set_text(GTK_ENTRY(MpickKeys), "");
			gtk_widget_hide(MpickKEYW);
		}
		if (settings.magnify.pickCOMOn)
		{
			settings.magnify.pickComment[0] = 0;	//gtk_entry_set_text(GTK_ENTRY(MpickComment), "");
			gtk_entry_set_text(GTK_ENTRY(MpickComment), settings.magnify.pickComment);
			gtk_widget_show(MpickCOMW);
		}
		else
		{	// NO COMMENT!
			gtk_entry_set_text(GTK_ENTRY(MpickComment), "");
			gtk_widget_hide(MpickCOMW);
		}

		if (gtk_dialog_run (GTK_DIALOG (pickDialog)) == GTK_RESPONSE_YES)
		{	// on OK
			strcpy(settings.magnify.pickFile, gtk_entry_get_text(GTK_ENTRY(MpickFN)));
			strcpy(settings.magnify.pickKEY, gtk_entry_get_text(GTK_ENTRY(MpickKeys)));
			strcpy(settings.magnify.pickComment, gtk_entry_get_text(GTK_ENTRY(MpickComment)));
			gtk_widget_hide(pickDialog);
		}
	}

	if (!settings.magnify.pickFile[0])
	{
		dispMsg(DISPLAY, "No Pick File Specified, Nothing Written");
		gdk_beep();
		return;
	}

	if ((fp=fopen(settings.magnify.pickFile, "a"))==NULL)
	{
		dispMsg(DISPLAY, "Unable to Open Pick File %s, Cannot Write");
		gdk_beep();
		return;
	}

	for (trcIter = states.trace.selection, totPicks=0, i=0;
		 trcIter;
		 trcIter = g_slist_next(trcIter), totPicks=0, i++)
	{
		trcData = trcIter->data;
		outLabel = (strcmp(trcData->data.fileName, STNCOMPFN))
					? g_strdup(trcData->data.head.longFN)
					: g_strdup(trcData->data.head.descr2);
					
		if (settings.magnify.pickKEY[0])
			fprintf(fp, "%s\tKEYWORDS\t%s\n", outLabel, settings.magnify.pickKEY);

		if (settings.magnify.pickComment[0])
			fprintf(fp, "%s\tCOMMENT\t%s\n", outLabel, settings.magnify.pickComment);

		memset(&P, 0, sizeof(P));
		memset(&S, 0, sizeof(S));
		for(j=0;j<MAXPICKS && settings.magnify.pickTag[j][0]; ++j)
		{
			if (settings.magnify.pick == WSCALETRC)
			{
				pick = trcData->magnify.pick.trace.pick[j];
			}
			else
			{
				pick = states.magnify.pick.window.pick[j];
			}
			if (pick)
			{	// write only if a pick has been defined on screen 
				memset(&point, 0, sizeof(point));
				magPoint(i, 	// magnify screen record number
					states.magnify.limits.selection.user.left, 
					states.magnify.limits.selection.user.right,
					trcData->magnify.plot.increment, 
					pick,
					&point);

				picks[totPicks].pickN = j;
				picks[totPicks].pickE = FALSE;
				picks[totPicks++].pickT = PIX2USERX(&trcData->magnify.GR[states.currentScreen], pick);

				if (!strcmp(settings.magnify.pickTag[j], "P") || !strcmp(settings.magnify.pickTag[j], "p"))
					memcpy(&P, &point, sizeof(point));
				if (!strcmp(settings.magnify.pickTag[j], "S") || !strcmp(settings.magnify.pickTag[j], "s"))
					memcpy(&S, &point, sizeof(point));

				if (point.minAmp == point.maxAmp)
				{
					if (trcData->data.head.units != PQL_UNITS_VOLTS)
						sprintf(str, "%d", (int) point.minAmp);
					else
						sprintf(str, "%6.5f", point.minAmp);
				}
				else
				{
					if (trcData->data.head.units != PQL_UNITS_VOLTS)
						sprintf(str, "< %d ~ %d >", (int) point.minAmp, (int) point.maxAmp);
					else
						sprintf(str, "<%6.5f ~ %6.5f>", point.minAmp, point.maxAmp);
				}
				fprintf(fp, "%s\t%s\t%4d:%03d:%02d:%02d:%06.3f\t%s\n", 
						outLabel, settings.magnify.pickTag[j],
						point.timeVal.yr, julian(&point.timeVal), point.timeVal.hr, 
						point.timeVal.mn, point.timeVal.sec, str);

				for(k=0;k<2;k++)
				{
					if (settings.magnify.pick == WSCALETRC)
					{
						pick = trcData->magnify.pick.trace.errorPick[j][k];
					}
					else
					{
						pick = states.magnify.pick.window.errorPick[j][k];
					}
					if(pick)
					{
						memset(&point, 0, sizeof(point));
						magPoint(i, 	// magnify screen record number
							states.magnify.limits.selection.user.left, 
							states.magnify.limits.selection.user.right,
							trcData->magnify.plot.increment, 
							pick,
							&point);

						picks[totPicks].pickN = j;
						picks[totPicks].pickE = TRUE;
						picks[totPicks++].pickT = PIX2USERX(&trcData->magnify.GR[states.currentScreen], pick);

						if (point.minAmp == point.maxAmp)
						{
							if (trcData->data.head.units != PQL_UNITS_VOLTS)
								sprintf(str, "%d", (int) point.minAmp);
							else
								sprintf(str, "%6.5f", point.minAmp);
						}
						else
						{
							if (trcData->data.head.units != PQL_UNITS_VOLTS)
								sprintf(str, "< %d ~ %d >", (int) point.minAmp, (int) point.maxAmp);
							else
								sprintf(str, "<%6.5f ~ %6.5f>", point.minAmp, point.maxAmp);
						}
						fprintf(fp, "%s\te%s\t%4d:%03d:%02d:%02d:%06.3f\t%s\n", 
							outLabel, settings.magnify.pickTag[j],
							point.timeVal.yr, julian(&point.timeVal), point.timeVal.hr, 
							point.timeVal.mn, point.timeVal.sec, str);
					}
				}
			}
		}

		// save the pick information for possible later replotting
		if (!trcData->trace.picks)
		{	// first save?
			trcData->trace.picks = calloc(totPicks, sizeof(pickInfo));
		}
		else
		{	// 2nd or more save?
			trcData->trace.picks = realloc(trcData->trace.picks, 
										(trcData->trace.numPicks+totPicks)*sizeof(pickInfo));
		}
		for(k=0,j=trcData->trace.numPicks;j<totPicks+trcData->trace.numPicks; j++, k++)
		{
			trcData->trace.picks[j].pickN = picks[k].pickN;
			trcData->trace.picks[j].pickE = picks[k].pickE;
			trcData->trace.picks[j].pickT = picks[k].pickT;
			trcData->trace.picks[j].pickS = settings.magnify.pick;
		}
		trcData->trace.numPicks += totPicks;
			
		for (j=0;j<2;j++)
		{	// process the point selections
			memset(&points[j], 0, sizeof(points[j]));
			if (settings.magnify.pick == WSCALEWIND && i)
				break;		// plot points in pick.window mode only the first time when looping over traces

			if (settings.magnify.pick == WSCALETRC)
			{
				memcpy(&points[j], &trcData->magnify.point.trace.point[j], sizeof(pointID));
			}
			else
			{
				memcpy(&points[j], &states.magnify.point.window.point[j], sizeof(pointID));
			}
			if (points[j].epoch)
			{
				if (points[j].minAmp == points[j].maxAmp)
				{
					if (trcData->data.head.units != PQL_UNITS_VOLTS)
						sprintf(str, "%d", (int) points[j].minAmp);
					else
						sprintf(str, "%6.5f", points[j].minAmp);
				}
				else
				{
					if (trcData->data.head.units != PQL_UNITS_VOLTS)
						sprintf(str, "< %d ~ %d >", (int) points[j].minAmp, (int) points[j].maxAmp);
					else
						sprintf(str, "<%6.5f ~ %6.5f>", points[j].minAmp, points[j].maxAmp);
				}
				fprintf(fp, "%s\tP%d\t%4d:%03d:%02d:%02d:%06.3f\t%s\n", 
					outLabel, j+1,
					points[j].timeVal.yr, julian(&points[j].timeVal), points[j].timeVal.hr, 
					points[j].timeVal.mn, points[j].timeVal.sec, str);
			}
		}
		if (points[0].epoch && 
			points[1].epoch)
		{	// print peak-to-peak, if both points have been specified by user
			memset(&point, 0, sizeof(point));
			point.epoch = points[1].epoch - points[0].epoch;
			point.maxAmp = points[1].maxAmp - points[0].maxAmp;
			point.minAmp = points[1].minAmp - points[0].minAmp;
			if (point.minAmp == point.maxAmp)
			{
				if (trcData->data.head.units != PQL_UNITS_VOLTS)
					sprintf(str, "%d", (int) point.minAmp);
				else
					sprintf(str, "%6.5f", point.minAmp);
			}
			else
			{
				if (trcData->data.head.units != PQL_UNITS_VOLTS)
					sprintf(str, "< %d ~ %d >", (int) point.minAmp, (int) point.maxAmp);
				else
					sprintf(str, "<%6.5f ~ %6.5f>", point.minAmp, point.maxAmp);
			}
			fprintf(fp, "%s\t%s\t%8.3f\t%s\n", outLabel, "P2P", point.epoch, str);
		}
		if (P.epoch && S.epoch)
		{	// print P - S, if they both exist
			memset(&point, 0, sizeof(point));
			point.epoch = P.epoch - S.epoch;
			point.maxAmp = P.maxAmp - S.maxAmp;
			point.minAmp = P.minAmp - S.minAmp;
			if (point.minAmp == point.maxAmp)
			{
				if (trcData->data.head.units != PQL_UNITS_VOLTS)
					sprintf(str, "%d", (int) point.minAmp);
				else
					sprintf(str, "%6.5f", point.minAmp);
			}
			else
			{
				if (trcData->data.head.units != PQL_UNITS_VOLTS)
					sprintf(str, "< %d ~ %d >", (int) point.minAmp, (int) point.maxAmp);
				else
					sprintf(str, "<%6.5f ~ %6.5f>", point.minAmp, point.maxAmp);
			}
			fprintf(fp, "%s\t%s\t%8.3f\t%s\n", outLabel, "P-S", point.epoch, str);
		}
		free(outLabel);
	}
	fclose(fp);
	
	states.msg = g_strdup_printf("Pick File %s written.", settings.magnify.pickFile);
	makePixmap(MAGNIFYDA);
}

void print(GtkWidget *widget, gpointer s)
{
	int			scr = GPOINTER_TO_INT(s);
	gdk_window_set_cursor(topLevel, hourCursor);
	while (g_main_context_iteration(NULL, FALSE));

	if ((!states.trace.numSelected && states.currentPage != TRACESCR) ||
		(!states.general.numTraces))
	{	// don't print any blank screens
		dispMsg(DISPLAY, "No TRACES Displayed or Selected, Nothing to Print");
		gdk_window_set_cursor(topLevel, NULL);
		gdk_beep();
		return;
	}

	switch(settings.general.print.format)
	{
		case PRINT_FMT_PS:
			printPS(scr);
		break;
		case PRINT_FMT_PNG:
			printPNG(scr);
		break;
	}

	gdk_window_set_cursor(topLevel, NULL);
	gdk_beep();
}

void help(GtkButton *widget, gpointer s)
{
	int	scr = *((int*) s);
	if (!helpWindow)
	{
		make_help();
	}

	gtk_widget_show(helpWindow);
  	gtk_notebook_set_current_page((GtkNotebook *) help_screens, scr);
	gtk_grab_add(helpWindow);
	
	return;
}

void hideHelp(GtkButton *widget, gpointer nil)
{
	gtk_widget_hide(helpWindow);
	gtk_grab_remove(helpWindow);
}

#ifdef PQL_ONLY
void switchScreenPQL(int screen);
gboolean FKeyEvent(GtkWidget *widget, GdkEventKey *event, gpointer nil)
{
	guint	modifiers = gtk_accelerator_get_default_mod_mask();
	int		ctrlKey = ((event->state & modifiers) == GDK_CONTROL_MASK) ? CTRL : NONE;
	
	switch(event->keyval)
	{
		case GDK_F1:
			switch(ctrlKey)
			{
				case CTRL:
					help(NULL, &states.currentPage);
				break;
				default:
					switchScreenPQL((states.currentPage+1)%NUMSCREENS);
				break;
			}
		break;
	}
	return FALSE;
}
#endif

static void freeCG(XYZCOMPONENT *comp)
{
	g_hash_table_destroy(comp->comp);
}

static void uvwComp(char *key, TRC_N_DATA *trcNdata, gpointer f)
{
	traceInfo	*trcData = trcNdata->trc;
	int			success = GPOINTER_TO_INT(f);
	DATAINFO	*dataInfo = trcNdata->data;
	
	switch(success)
	{
		case TRUE:
			get_maxmin(trcData, dataInfo);
			dataInfo->tFormStr = makeTFormStr(dataInfo->tFormStr, PQLUVW);
		break;
		case FALSE:
			freeTFormData(dataInfo);
		break;
	}
}

static void uvw(char *key, XYZCOMPONENT *comp, gpointer f)
{
	int	func = GPOINTER_TO_INT(f);
	
	switch(func)
	{
		case FUNCUVW:
			if (g_hash_table_size(comp->comp) == 3)
				comp->success = xyzUVW(comp->comp, comp->sensor);
		break;
		
		case FUNCMAXMIN:
		{
			int	success = comp->success;
			g_hash_table_foreach(comp->comp, (GHFunc) uvwComp, GINT_TO_POINTER(success));
		}
		break;
	}
}

void pqlTFormSel(GtkComboBox *combo, gpointer flag)
{	// callback when transform selected
	gboolean 	orig = (gboolean) GPOINTER_TO_INT(flag), maxMin;
	traceInfo	*trcData;
	GSList		*trcIter;
	DATAINFO	*dataInfo;
	GHashTable	*CG=NULL;
	int			tForm = PQLTFORMS;
	
	switch(orig)
	{
		case TRUE:		// return trace to original state
			for (trcIter = states.trace.selection;
				 trcIter;
				 trcIter = g_slist_next(trcIter))
			{			// free any transformed data and clear the structure
				trcData = trcIter->data;
				freeTFormData(&trcData->data.tForm);
			}
			states.tForm.on = FALSE;
		break;
		
		case FALSE:
		{
			tForm = gtk_combo_box_get_active(combo);
			if (tForm == PQLTFORMS)
				return;

			maxMin = TRUE;
			if (tForm == PQLUVW)
			{
				CG = g_hash_table_new_full(g_str_hash, g_str_equal, 
							(GDestroyNotify)free, (GDestroyNotify)freeCG);
				maxMin = FALSE;		// this must be deferred till later for UVW() transform
			}

			states.tForm.on = TRUE;
			for (trcIter = (states.magnify.selection 
							? states.magnify.selection 
							: states.trace.selection);
				 trcIter;
				 trcIter = g_slist_next(trcIter))
			{
				trcData = trcIter->data;
				dataInfo = &trcData->data.tForm;
				if (!dataInfo->data)
					makeTFormData(trcData, FALSE);
				
				switch(tForm)
				{
					case PQLDEMEAN:
					{
						double mean = dataInfo->amps.mean ? dataInfo->amps.mean : trcData->data.dataInfo.amps.mean;
						demean(dataInfo, mean);
					}
					break;
					case PQLDETREND:
						detrend(dataInfo);
					break;
					case PQLREVPOL:
						polarityReverse(dataInfo);
					break;
					case PQLDIFFERENTIATE:
						dxdy(dataInfo);
						dataInfo->head.numSamples -= 1;
					break;
					case PQLINTEGRATE:
						integrate(dataInfo);
						dataInfo->head.numSamples -= 2;
					break;
					case PQLUVW:
					{	// just collect the traces into component groups
						TRC_N_DATA	*trcNdata;
						XYZCOMPONENT *comp;
						char		*CGkey;
						CGkey = g_strdup_printf("%s%s%s%.2s", trcData->data.head.network,
																	 trcData->data.head.station,
																	 trcData->data.head.location,
																	 trcData->data.head.channel);
						if ((comp = g_hash_table_lookup(CG, CGkey)) == NULL)
						{
							comp = calloc(1, sizeof(XYZCOMPONENT));
							comp->sensor = settings.general.tForm.uvwSensor;
							comp->success = FALSE;
							comp->comp = g_hash_table_new_full(g_str_hash, g_str_equal, 
												NULL, (GDestroyNotify)free);
							g_hash_table_insert(CG, CGkey, comp);
						}
						else
						{
							free(CGkey);
						}
						trcNdata = calloc(1, sizeof(TRC_N_DATA));
						trcNdata->trc = trcData;
						trcNdata->data = dataInfo;
						g_hash_table_insert(comp->comp, &trcData->data.head.channel[2], trcNdata);
					}
					break;
				}
				if (maxMin)
				{
					get_maxmin(trcData, dataInfo);
					dataInfo->tFormStr = makeTFormStr(dataInfo->tFormStr, tForm);
				}
				trcData->magnify.selected = FALSE;
			}
			// reset combo box to nothing
			gtk_combo_box_set_active(combo, PQLTFORMS);
		}
		break;
	}
	states.magnify.dirty = TRUE;
	setRadios(FILTERTYPE);
	
	// do any post trace file loop processing 
	switch (tForm)
	{
		case PQLUVW:	// process each component group
			g_hash_table_foreach(CG, (GHFunc) uvw, GINT_TO_POINTER(FUNCUVW));
			g_hash_table_foreach(CG, (GHFunc) uvw, GINT_TO_POINTER(FUNCMAXMIN));
			g_hash_table_destroy(CG);
		break;
	}

	makeMAGSelectionList(FALSE);
	switch(states.currentPage)
	{
		case MAGNIFYSCR:
		case SPECTRASCR:
			makePixmap(states.currentScr);
		break;

		case SPLITSCR:
			switch(states.currentScreen)
			{
				case SPLIT2:
					if (settings.split.DA[states.split.panes][MID] == TRACESCR)
					{	//trace screen is not visible, draw it invisibly
						makePixmap(SPLITDA2M);
					}
					makePixmap(SPLITDA2T);
					makePixmap(SPLITDA2B);
					states.split.refreshPixmap[SPLIT3][TRACESCR] = TRUE;
					states.split.refreshPixmap[SPLIT3][MAGNIFYSCR] = TRUE;
					states.split.refreshPixmap[SPLIT3][SPECTRASCR] = TRUE;
				break;

				case SPLIT3:
					makePixmap(settings.split.DT[SPLIT3][TRACESCR]);
					makePixmap(settings.split.DT[SPLIT3][MAGNIFYSCR]);
					makePixmap(settings.split.DT[SPLIT3][SPECTRASCR]);
					states.split.refreshPixmap[SPLIT2][TRACESCR] = TRUE;
					states.split.refreshPixmap[SPLIT2][MAGNIFYSCR] = TRUE;
					states.split.refreshPixmap[SPLIT2][SPECTRASCR] = TRUE;
				break;
			}
		break;
	}
	gdk_window_set_cursor(topLevel, NULL);
}

static gint getLogFile(LOGDATA *one, char *fileName)
{
	return (strcmp(one->fileName, fileName));
}

static LOGDATA *getLog(traceInfo *trc)
{
	LOGDATA *logData, *logDataRet=NULL;
	LOGTEXT	*logText;
	GSList	*textIter, *logIter;
	double	endT = trc->data.head.epochs + trc->data.head.length;
	
	logIter = g_slist_find_custom(states.logs.logs, trc->data.fileName, (GCompareFunc) getLogFile);
	if (!logIter)
		return NULL;
	logData = logIter->data;
	
	for (textIter = logData->text;
		textIter;
		textIter = g_slist_next(textIter))
	{
		logText = textIter->data;
		if (logText->startT >= trc->data.head.epochs &&
			logText->startT < endT)
		{
			if (!logDataRet)
				logDataRet = calloc(1, sizeof(LOGDATA));
			logDataRet->text = g_slist_append(logDataRet->text, logText);
		}
	}
	
	return logDataRet;
}

void logSelect(GtkTreeView *treeview, GtkTreePath *arg1, GtkTreeViewColumn *arg2, gpointer l)
{	// called when log entry double-clicked, i.e., display data in logVIEW widget
	int	logType = GPOINTER_TO_INT(l);
	char	*row;
	int		rowNum;
	LOGDATA *logData=NULL;
	LOGTEXT	*logText;
	GSList	*textIter;
	GtkTextBuffer **buffer = &states.logs.buffer;
	GtkTextIter iter, endIter;

	row = gtk_tree_path_to_string(arg1);
	rowNum = atoi(row);
	g_free(row);
	switch (logType)
	{
		case LOGFILE:
			logData = g_slist_nth_data(states.logs.logs, rowNum);
		break;
		case LOGTRC:
		{
			GSList *trcList=NULL;
			traceInfo *trc;
			switch(settings.logs.display)
			{
				case ALL:
					trcList = traces;
				break;
		
				case DISPLAYED:
					trcList = states.trace.traces[CURGRP];
				break;
		
				case SELECTED:
					trcList = states.trace.selection;
				break;
			}
			trc = g_slist_nth_data(trcList, rowNum);
			logData = getLog(trc);
		}
		break;
	}

	if (!*buffer)
		*buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(logTEXT));
	
	gtk_text_buffer_get_iter_at_offset(*buffer, &endIter, -1);
	gtk_text_buffer_get_iter_at_offset(*buffer, &iter, 0);
	if (gtk_text_iter_get_offset(&endIter) > 0)
		gtk_text_buffer_delete(*buffer, &iter, &endIter);

	for (textIter = logData->text;
		textIter;
		textIter = g_slist_next(textIter))
	{
		logText = textIter->data;
		gtk_text_buffer_insert(*buffer, &iter, logText->text, -1);
	}
	
	switch (logType)
	{
		case LOGTRC:
			g_slist_free(logData->text);
		break;
	}
}
