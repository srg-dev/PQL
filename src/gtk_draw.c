#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pql_defines.h"
#include "pql_externs.h"
#include "gtk_pql_externs.h"

static GdkPixmap	*Tpm[TMAXPIXMAPS];		// trace screen backing store pixmaps
static GdkPixmap	*Mpm[MMAXPIXMAPS];		// mag screen
static GdkPixmap	*Spm[SMAXPIXMAPS];		// spectra screen
static GdkPixmap	*Hpm[HMAXPIXMAPS];		// split screen

gboolean _makePixmap(gpointer s)
{
	int scr = GPOINTER_TO_INT(s);
	makePixmap(scr);
	return FALSE;
}

void makeNONE(GdkPixmap *pxmp, int type, int w, int h)
{
	char	msg[100];

	settings.general.font.font = MSGF;
	msg[0] = 0;
	switch(type)
	{
		case TRACESCR:
			switch(states.currentScreen)
			{
				case MAIN:
					switch (states.general.plot.state)
					{
						case NOTRACES:
							strcpy(msg, "No TRACES Selected for Viewing");
						break;
						case READHDRS:
							strcpy(msg, "Reading Headers...");
						break;
						case READDATA:
							strcpy(msg, "Reading Traces...");
						break;
						case DATAPLOT:
							strcpy(msg, "Plotting...");
						break;
					}
				break;

				default:
					strcpy(msg, "No TRACES Selected for Viewing");
				break;
			}
		break;

		case MAGNIFYSCR:
			strcpy(msg, "No TRACES Selected for Magnification");
		break;

		case SPECTRASCR:
			strcpy(msg, "No TRACES Selected for Transformation");
		break;
	}

	fontDraw(pxmp, msg, 
				w/2 - fontSIZE(msg, WIDTH)/2, 
				h/2 - fontSIZE(msg, HEIGHT)/2, 
				NORMALGC, type, -1, -1);

	return;
}

void clearPixmaps()
{
	int i;
	for(i=0;i<TMAXPIXMAPS;i++)
	{
		if (Tpm[i])
			g_object_unref(Tpm[i]);
		Tpm[i] = NULL;
	}
	for(i=0;i<MMAXPIXMAPS;i++)
	{
		if (Mpm[i])
			g_object_unref(Mpm[i]);
		Mpm[i] = NULL;
	}
	for(i=0;i<SMAXPIXMAPS;i++)
	{
		if (Spm[i])
			g_object_unref(Spm[i]);
		Spm[i] = NULL;
	}
	for(i=0;i<HMAXPIXMAPS;i++)
	{
		if (Hpm[i])
			g_object_unref(Hpm[i]);
		Hpm[i] = NULL;
	}
}
	
GdkPixmap * tracePixmap(GtkWidget *da)
{
	int pixmapID, i, width=da->allocation.width, height=da->allocation.height;
	static int oldpmID;
	GSList	*trcIter;
	traceInfo *trcData;

	states.currentGraph = TRACESCR;
	if (!GXC[NORMALGC][TRACESCR])
		make_gc(da, TRACESCR);

	settings.general.font.DA = da;
	settings.general.font.GC = GXC[NORMALGC][TRACESCR];
	
	switch (states.general.plot.state)
	{
		case DATADONE:
			if (!states.general.numTraces)
				states.general.plot.state = NOTRACES;
		break;
	}
	
	switch (states.general.plot.state)
	{
		case NOTRACES:
		case READHDRS:
			if (topLevel)
			{
				states.general.plot.state == NOTRACES
					? gdk_window_set_cursor(topLevel, NULL)
					: gdk_window_set_cursor(topLevel, hourCursor);
			}
			states.reset.cursor = FALSE;
			clearPixmaps();
			Tpm[TRACENONE] = gdk_pixmap_new(da->window, 
								da->allocation.width, da->allocation.height, -1);
			gdk_draw_rectangle(Tpm[TRACENONE], GXC[ABNORMALGC][TRACESCR], TRUE, 0, 0, width, height);	
			makeNONE(Tpm[TRACENONE], TRACESCR, width, height);	
			states.reset.reset[TRACESCR] = TRUE;
			dispMsg(UNDISPLAY, NULL);
			return Tpm[TRACENONE];
		break;
		
		case READDATA:
			gdk_window_set_cursor(topLevel, hourCursor);
			states.reset.cursor = FALSE;
			clearPixmaps();
			Tpm[TRACENONE] = gdk_pixmap_new(da->window, 
								da->allocation.width, da->allocation.height, -1);
			gdk_draw_rectangle (Tpm[TRACENONE], GXC[ABNORMALGC][TRACESCR], TRUE, 0, 0, width, height);	
			makeNONE(Tpm[TRACENONE], TRACESCR, width, height);	
	
			readData_1();						// read all the data
			states.reset.reset[TRACESCR] = TRUE;
			dispMsg(UNDISPLAY, NULL);
			return Tpm[TRACENONE];
		break;
		
		case DATAPLOT:
			gdk_window_set_cursor(topLevel, hourCursor);
			states.reset.cursor = FALSE;
			clearPixmaps();
			Tpm[TRACENONE] = gdk_pixmap_new(da->window, 
								da->allocation.width, da->allocation.height, -1);
			gdk_draw_rectangle (Tpm[TRACENONE], GXC[ABNORMALGC][TRACESCR], TRUE, 0, 0, width, height);	
			makeNONE(Tpm[TRACENONE], TRACESCR, width, height);	
	
			states.general.plot.state = DATADONE;
			g_idle_add((GSourceFunc) _makePixmap, GINT_TO_POINTER(TRACESCR));
			states.reset.reset[TRACESCR] = TRUE;
			dispMsg(UNDISPLAY, NULL);
			return Tpm[TRACENONE];
		break;
	}
	
	// block the mouse signals for the TRACE screen
	g_signal_handler_block(DA[TRACESCR], signals[BTNPRSSGNL][TRACESIGNAL]);
	g_signal_handler_block(DA[TRACESCR], signals[BTNRLSSGNL][TRACESIGNAL]);
	g_signal_handler_block(DA[TRACESCR], signals[MTNNTFYSGNL][TRACESIGNAL]);
	g_signal_handler_block(data_screens, signals[NTBKSGNL][TRACESIGNAL]);

	if (states.reset.reset[TRACESCR])			// have we been told to reconfigure?
	{
		get_max_mins();
		gdk_window_set_cursor(topLevel, hourCursor);
		states.reset.cursor = TRUE;
		selectT(NULL, GINT_TO_POINTER(DESELECT));	// clean up selection list
		states.trace.limits.selection.pix[states.currentScreen].left = 
		states.trace.limits.selection.pix[states.currentScreen].right = 0;
		states.trace.limits.selection.user.left = 
		states.trace.limits.selection.user.right = 0;

//		setDU(states.trace.plot.dispScale);
		
		// reset everything (that needs to be reset)
		memset(&states.magnify, 0, sizeof(states.magnify));
		memset(&states.spectra, 0, sizeof(states.spectra));
		memset(&states.trace.GR, 0, sizeof(states.trace.GR));
		memset(&states.trace.point, 0, sizeof(states.trace.point));
		memset(&states.trace.plot, 0, sizeof(states.trace.plot));
		memset(&states.trace.limits, 0, sizeof(states.trace.limits));
		states.tForm.on = OFF;

		// update required on all other screens
		states.reset.refresh[TRACESCR] = TRUE;
		states.reset.reset[MAGNIFYSCR] = 
		states.reset.reset[SPECTRASCR] = 
		states.split.resetPixmaps = TRUE;
		states.split.refreshPixmap[SPLIT2][TRACESCR] = 
		states.split.refreshPixmap[SPLIT3][TRACESCR] = TRUE;
		states.split.refreshPixmap[SPLIT2][MAGNIFYSCR] = 
		states.split.refreshPixmap[SPLIT3][MAGNIFYSCR] = TRUE;
		states.split.refreshPixmap[SPLIT2][SPECTRASCR] = 
		states.split.refreshPixmap[SPLIT3][SPECTRASCR] = TRUE;

		setRadios(FILTERTYPE);
	}

	if (states.reset.refresh[TRACESCR])
	{
		clearPixmaps();
	}

	// define unique pixmapID for each possible display combination
	pixmapID =	settings.trace.amp + 
				settings.trace.time*2; 
				//+ settings.general.units*4;

	if (pixmapID != oldpmID && states.trace.selectionChanged)
	{	// an attempt to keep the pixmaps in sync with definition of current selections 
		// when selections occur across different pixmaps
		if (Tpm[pixmapID])
		{
			for (trcIter = g_slist_nth(traces, states.trace.startRec), i=0;
				 trcIter &&
				 i < settings.trace.numPlots;
				 trcIter = g_slist_next(trcIter), i++)
			{
				trcData = trcIter->data;
				printLabel(Tpm[pixmapID], trcData, i, states.trace.trace_depth[states.currentScreen], 
							USER2PIXX(&trcData->trace.GR[states.currentScreen], 
											(double) trcData->trace.plot.endTime1), TRACESCR);
			}
		}
		states.trace.selectionChanged = FALSE;
	}	
	if (!Tpm[pixmapID])
	{
		Tpm[pixmapID] = gdk_pixmap_new(da->window, width, height, -1);
		gdk_draw_rectangle (Tpm[pixmapID], GXC[ABNORMALGC][TRACESCR], TRUE, 0, 0, width, height);

		DAPixmap[TRACEDA] = Tpm[pixmapID];
		plot_files(da, Tpm[pixmapID], width, height, FALSE);

		if (settings.magnify.length == 0)
			settings.magnify.length = states.trace.relTimeLen/10.;
	}

	if (oldpmID != pixmapID)
	{	// split update required
		states.split.refreshPixmap[SPLIT2][TRACESCR] = 
		states.split.refreshPixmap[SPLIT3][TRACESCR] = TRUE;
		states.reset.reset[MAGNIFYSCR] = TRUE;
		memset(&states.trace.point, 0, sizeof(states.trace.point));
		oldpmID = pixmapID;
	}

	plot_extras(da, Tpm[pixmapID], pixmapID, TRACESCR, 
					states.reset.reset[TRACESCR] | states.reset.refresh[TRACESCR],
					width, height);
	states.reset.reset[TRACESCR] = FALSE;
	states.reset.refresh[TRACESCR] = FALSE;

	gtk_widget_hide(progressBar);
	g_signal_handler_unblock(DA[TRACESCR], signals[BTNPRSSGNL][TRACESIGNAL]);
	g_signal_handler_unblock(DA[TRACESCR], signals[BTNRLSSGNL][TRACESIGNAL]);
	g_signal_handler_unblock(DA[TRACESCR], signals[MTNNTFYSGNL][TRACESIGNAL]);
	g_signal_handler_unblock(data_screens, signals[NTBKSGNL][TRACESIGNAL]);

	gtk_widget_hide(progressBar);
	return Tpm[pixmapID];
}

GdkPixmap * magPixmap(GtkWidget *da)
{
	int i, pixmapID, width=da->allocation.width, height=da->allocation.height;
	static int oldpmID;
	gboolean init = FALSE;
	GSList	*trcIter;
	traceInfo	*trcData;

	states.currentGraph = MAGNIFYSCR;
	pixmapID =	settings.magnify.Wamp + 
				settings.magnify.overlay*2 + 
				//settings.general.units*4 + 
				states.tForm.on*8 +
				settings.magnify.Tamp*16;

	if (oldpmID != pixmapID)
	{
		init = TRUE;
	}
	
	if (!states.trace.limits.selection.pix[states.currentScreen].left || 
		!states.trace.numSelected)
	{
		pixmapID = MAGNONE;
		states.reset.reset[MAGNIFYSCR] = TRUE;
	}

	if (!GXC[NORMALGC][MAGNIFYSCR])
		make_gc(da, MAGNIFYSCR);

	settings.general.font.DA = da;
	settings.general.font.GC = GXC[NORMALGC][MAGNIFYSCR];
	
	// have we been told to reconfigure?
	if (states.reset.reset[MAGNIFYSCR] || 
		states.reset.refresh[MAGNIFYSCR])		
	{
		gtk_widget_hide(Mmag);
		gtk_widget_show(Msnap);
		for(i=0;i<MMAXPIXMAPS;i++)
		{
			if (Mpm[i])
				g_object_unref(Mpm[i]);
			Mpm[i] = NULL;
		}
		oldpmID = -1;
		states.user.topRec.magnify = states.trace.numSelected-1;
		if (states.magnify.selectionChanged)
		{	// don't do this if the list of traces hasn't changed
			for (trcIter = states.trace.selection, i=0;
				 trcIter;
				 trcIter = g_slist_next(trcIter), i++)
			{
				trcData = trcIter->data;
				for(i=0;i<trcData->trace.numPicks;i++)
				{
					trcData->trace.picks[i].pickS = WSCALETRC;
				}
			}
			states.magnify.selectionChanged = FALSE;
			init = TRUE;
		}
	}

	if (states.magnify.pick.reset || 
		states.reset.reset[MAGNIFYSCR])
	{
		memset(&states.magnify.point.window, 0, sizeof(states.magnify.point.window));
		memset(&states.magnify.plot, 0, sizeof(states.magnify.plot));
	}
	
	if ((states.magnify.dirty))	// ||		 ((states.filter.active==-1) ? FALSE : states.filter.newFilter[states.filter.active])))
	{
		if (Mpm[pixmapID])
		{
			g_object_unref(Mpm[pixmapID]);
			Mpm[pixmapID] = NULL;
		}
		states.magnify.dirty = FALSE;
		init = TRUE;
	}

	if (!Mpm[pixmapID])
	{
		Mpm[pixmapID] = gdk_pixmap_new(da->window, width, height, -1);
		gdk_draw_rectangle (Mpm[pixmapID],
					GXC[ABNORMALGC][MAGNIFYSCR],
					TRUE,
					0, 0, width, height);

		switch(pixmapID)
		{
			case MAGNONE:
				settings.general.font.DA = da;
				makeNONE(Mpm[pixmapID], MAGNIFYSCR, width, height);
			break;

			default:
				magnify_proc(da, Mpm[pixmapID], width, height, init);
			break;
		}
	}

	plot_extras(da, Mpm[pixmapID], pixmapID, MAGNIFYSCR, states.reset.reset[MAGNIFYSCR], width, height);
	if (oldpmID != pixmapID)
	{
		states.split.refreshPixmap[SPLIT2][MAGNIFYSCR] = 
		states.split.refreshPixmap[SPLIT3][MAGNIFYSCR] = TRUE;
		oldpmID = pixmapID;
	}

	states.reset.reset[MAGNIFYSCR] = states.reset.refresh[MAGNIFYSCR] = FALSE;
	states.magnify.pick.reset = FALSE;
	return Mpm[pixmapID];
}

GdkPixmap * specPixmap(GtkWidget *da)
{
	int i, pixmapID, succ, width=da->allocation.width, height=da->allocation.height;
	static int oldpmID;

	states.currentGraph = SPECTRASCR;
	pixmapID =	settings.spectra.display.amp + 
				settings.spectra.display.overlay*2 + 
				//settings.general.units*4 + 
				states.tForm.on*8 + 
				settings.spectra.display.yAxis*16 + 
				settings.spectra.display.xAxis*32;

	if (!states.trace.limits.selection.pix[states.currentScreen].left || 
		!states.trace.numSelected)
	{
		pixmapID = SPECNONE;
		states.reset.reset[SPECTRASCR] = TRUE;
	}

	if (!GXC[NORMALGC][SPECTRASCR])
		make_gc(da, SPECTRASCR);

	settings.general.font.DA = da;
	settings.general.font.GC = GXC[NORMALGC][SPECTRASCR];

	if (states.reset.reset[SPECTRASCR])		// have we been told to reconfigure?
	{
		states.user.topRec.spectra = states.trace.numSelected-1;
		memset(&states.spectra, 0, sizeof(states.spectra));
		oldpmID = -1;
	}

	if (states.reset.refresh[SPECTRASCR] || 
		states.reset.reset[SPECTRASCR])
	{
		for(i=0;i<SMAXPIXMAPS;i++)
		{
			if (Spm[i])
				g_object_unref(Spm[i]);
			Spm[i] = NULL;
		}
	}

	if (!Spm[pixmapID])
	{
		Spm[pixmapID] = gdk_pixmap_new(da->window, width, height, -1);
		gdk_draw_rectangle (Spm[pixmapID],GXC[ABNORMALGC][SPECTRASCR], TRUE, 0, 0, width, height);

		switch(pixmapID)
		{
			case SPECNONE:
				settings.general.font.DA = da;
				makeNONE(Spm[pixmapID], SPECTRASCR, width, height);
			break;

			default:
				succ = transform_proc(da, Spm[pixmapID], width, height,	states.reset.reset[SPECTRASCR]); 
				if (succ != PQLXSUCCESS)
				{
					pixmapID = oldpmID;
				}
			break;
		}
	}

	plot_extras(da, Spm[pixmapID], pixmapID, SPECTRASCR, states.reset.reset[SPECTRASCR], width, height);

	if (oldpmID != pixmapID)
	{	// undone: fix spectra refresh without assuming where new pixmap has been created
		states.split.refreshPixmap[SPLIT2][SPECTRASCR] = 
		states.split.refreshPixmap[SPLIT3][SPECTRASCR] = TRUE;	
		memset(&states.spectra.point, 0, sizeof(states.spectra.point));
		oldpmID = pixmapID;
	}

	states.reset.reset[SPECTRASCR] = 
	states.reset.refresh[SPECTRASCR] = 
	settings.spectra.newFilter[states.currentScreen] = FALSE;
	return Spm[pixmapID];
}

GdkPixmap * splitPixmap(GtkWidget *da, int scr)
{
	int	i, pixmapID, screen;
	int width=da->allocation.width, height=da->allocation.height;
	GSList *trcIter;
	traceInfo *trcData;

	screen = settings.split.DA[states.split.panes][scr];
	pixmapID = states.split.panes*3 + screen;

	if (!GXC[NORMALGC][screen])
		make_gc(da, screen);

	settings.general.font.DA = da;
	settings.general.font.GC = GXC[NORMALGC][screen];

	if (states.split.resetPixmaps)
	{
		for(i=0;i<HMAXPIXMAPS;i++)
		{
			if (Hpm[i])
				g_object_unref(Hpm[i]);
			Hpm[i] = NULL;
		}
	}

	// do we require a reset of the drawing area
	if (states.split.resetPixmap[states.split.panes][scr] ||
		states.split.refreshPixmap[states.split.panes][screen] ||
		states.magnify.pick.reset)
	{
		if (Hpm[pixmapID])
			g_object_unref(Hpm[pixmapID]);
		Hpm[pixmapID] = NULL;
		states.split.resetPixmaps = TRUE;
	}

	if (!Hpm[pixmapID])
	{
		Hpm[pixmapID] = gdk_pixmap_new(da->window, width, height, -1);
		if (!GXC[NORMALGC][screen])
			make_gc(da, screen);

		gdk_draw_rectangle (Hpm[pixmapID], GXC[ABNORMALGC][screen], TRUE, 0, 0, width, height);
	
		switch(screen)
		{
			case TRACESCR:
				states.currentGraph = TRACESCR;
				if (!states.trace.numSelected)
				{
					makeNONE(Hpm[pixmapID], TRACESCR, width, height);
				}
				else
				{
					memset(&states.trace.plot, 0, sizeof(states.trace.plot));
					memset(&states.trace.point.window, 0, sizeof(states.trace.point.window));
					plot_files(da, Hpm[pixmapID], width, height, TRUE);
				}
			break;

			case MAGNIFYSCR:
				states.currentGraph = MAGNIFYSCR;
				if (!states.trace.limits.selection.pix[states.currentScreen].left || 
					!states.trace.numSelected)
				{
					makeNONE(Hpm[pixmapID], MAGNIFYSCR, width, height);
				}
				else
				{
					gtk_widget_hide(Hmag);
					gtk_widget_show(Hsnap);
					memset(&states.magnify.point.window, 0, sizeof(states.magnify.point.window));
					memset(&states.magnify.plot, 0, sizeof(states.magnify.plot));
					if (states.magnify.selectionChanged)
					{	// don't do this if the list of traces hasn't changed
						for (trcIter = states.trace.selection, i=0;
							 trcIter;
							 trcIter = g_slist_next(trcIter), i++)
						{
							trcData = trcIter->data;
							for(i=0;i<trcData->trace.numPicks;i++)
							{
								trcData->trace.picks[i].pickS = WSCALETRC;
							}
						}
						states.magnify.selectionChanged = FALSE;
					}
					magnify_proc(da, Hpm[pixmapID], width, height, TRUE);
				}
			break;

			case SPECTRASCR:
				states.currentGraph = SPECTRASCR;
				if (!states.trace.limits.selection.pix[states.currentScreen].left || 
					!states.trace.numSelected)
				{
					makeNONE(Hpm[pixmapID], SPECTRASCR, width, height);
				}
				else
				{
					transform_proc(da, Hpm[pixmapID], width, height,
							(states.reset.reset[SPECTRASCR] || 					// reset on new
							settings.spectra.newFilter[states.currentScreen])); // reset on new filter
					if (states.reset.reset[SPECTRASCR] || 
						settings.spectra.newFilter[states.currentScreen])
					{
						states.reset.reset[SPECTRASCR] = FALSE;
						states.reset.refresh[SPECTRASCR] = TRUE;
					}
				}
			break;
		}
	}

	if (states.trace.numSelected)		// only plot extras when there's actually something on display
		plot_extras(da, Hpm[pixmapID], pixmapID, screen, 
					states.split.resetPixmaps ||
					states.split.resetPixmap[states.split.panes][scr] ||
					states.split.refreshPixmap[states.split.panes][screen],
					width, height);
	states.split.resetPixmaps = FALSE;
	states.split.resetPixmap[states.split.panes][scr] = FALSE;
	states.split.refreshPixmap[states.split.panes][screen] = FALSE;
	states.magnify.pick.reset = FALSE;

	return Hpm[pixmapID];
}

void	makePixmap(int scr)
{
	gboolean	expose=TRUE;
	static gboolean	prepare2M = FALSE;
	
	switch(scr)
	{
		case TRACEDA:
			DAPixmap[scr] = tracePixmap(DA[scr]);
		break;

		case MAGNIFYDA:
			// handle case when called the first time, 
			// possibly before DA has been rendered, thus, disallow drawing
			if (DA[scr]->window != NULL)						
				DAPixmap[scr] = magPixmap(DA[scr]);
		break;

		case SPECTRADA:
			if (DA[scr]->window != NULL)						
				DAPixmap[scr] = specPixmap(DA[scr]);
		break;

		case SPLITDA3T:
			if (DA[scr]->window != NULL)						
			{
				DAPixmap[scr] = splitPixmap(DA[scr], TOP);
				if (states.trace.limits.selection.pix[MAIN].left == NONE)
				{	// must reset these values when coming from STN
					states.trace.limits.selection.pix[states.currentScreen].left = 
							states.trace.limits.margin[states.currentScreen].left;
					states.trace.limits.selection.pix[states.currentScreen].right = 
							states.trace.limits.margin[states.currentScreen].right;
				}
			}
		break;

		case SPLITDA3M:
			if (DA[scr]->window != NULL)
			{
				if (!states.split.realized[0])
				{	// start-up: set the position of the split screens
					gtk_paned_set_position((GtkPaned *) Hpanes[1], settings.general.startUP.panePos[1]);
					gtk_paned_set_position((GtkPaned *) Hpanes[2], settings.general.startUP.panePos[2]);
				}
				DAPixmap[scr] = splitPixmap(DA[scr], MID);
				if (states.trace.limits.selection.pix[MAIN].left == NONE)
				{
					states.trace.limits.selection.pix[states.currentScreen].left = 
							states.trace.limits.margin[states.currentScreen].left;
					states.trace.limits.selection.pix[states.currentScreen].right = 
							states.trace.limits.margin[states.currentScreen].right;
				}
			}
		break;

		case SPLITDA3B:
			if (DA[scr]->window != NULL)
			{
				if (!states.split.realized[1])
				{	// start-up: set the position of the split screens
					gtk_paned_set_position((GtkPaned *) Hpanes[1], settings.general.startUP.panePos[1]);
					gtk_paned_set_position((GtkPaned *) Hpanes[2], settings.general.startUP.panePos[2]);
				}
				DAPixmap[scr] = splitPixmap(DA[scr], BOT);
				if (states.trace.limits.selection.pix[MAIN].left == NONE)
				{
					states.trace.limits.selection.pix[states.currentScreen].left = 
							states.trace.limits.margin[states.currentScreen].left;
					states.trace.limits.selection.pix[states.currentScreen].right = 
							states.trace.limits.margin[states.currentScreen].right;
				}
			}
		break;

		case SPLITDA2T:
			if (DA[scr]->window != NULL)
			{
				if (!states.split.realized[2])
				{	// start-up: set the position of the split screens
					gtk_paned_set_position((GtkPaned *) Hpanes[0], settings.general.startUP.panePos[0]);
				}
				if (states.trace.limits.selection.pix[MAIN].left == NONE)
				{
					states.trace.limits.selection.pix[states.currentScreen].left = 
							states.trace.limits.margin[states.currentScreen].left;
					states.trace.limits.selection.pix[states.currentScreen].right = 
							states.trace.limits.margin[states.currentScreen].right;
				}
				if (prepare2M)
				{
					prepare2M = FALSE;
					DAPixmap[SPLITDA2M] = splitPixmap(DA[SPLITDA2T], MID);
					states.split.resetPixmap[states.split.panes][TOP] = TRUE;
					states.split.resetPixmap[states.split.panes][BOT] = TRUE;
				}
				DAPixmap[scr] = splitPixmap(DA[scr], TOP);
			}
		break;

		case SPLITDA2B:
			if (DA[scr]->window != NULL)
			{
				if (states.trace.limits.selection.pix[MAIN].left == NONE)
				{
					states.trace.limits.selection.pix[states.currentScreen].left = 
							states.trace.limits.margin[states.currentScreen].left;
					states.trace.limits.selection.pix[states.currentScreen].right = 
							states.trace.limits.margin[states.currentScreen].right;
				}
				if (prepare2M)
				{
					prepare2M = FALSE;
					DAPixmap[SPLITDA2M] = splitPixmap(DA[SPLITDA2B], MID);
					states.split.resetPixmap[states.split.panes][TOP] = TRUE;
					states.split.resetPixmap[states.split.panes][BOT] = TRUE;
				}
				DAPixmap[scr] = splitPixmap(DA[scr], BOT);
			}
		break;

		case SPLITDA2M:
			if (DA[SPLITDA2T]->window != NULL &&
				settings.split.DA[states.split.panes][MID] == TRACESCR)
			{
				DAPixmap[scr] = splitPixmap(DA[SPLITDA2T], MID);
				states.split.resetPixmap[states.split.panes][TOP] = TRUE;
				states.split.resetPixmap[states.split.panes][BOT] = TRUE;
			}
			else
			{
				if (settings.split.DA[states.split.panes][MID] == TRACESCR)
					prepare2M = TRUE;
			}
			expose = FALSE;
		break;
	}

	if (expose &&			// don't expose any invisible pixmaps
		DA[scr] &&			// and only if there's a widget
		DA[scr]->window)	// and a window
		gtk_widget_queue_draw_area(DA[scr], 0, 0, DA[scr]->allocation.width, DA[scr]->allocation.height);
	else
	{
		if (DA[scr] &&
			!DA[scr]->window)
		{	// force a configure event
			gtk_widget_queue_resize(DA[scr]);
		}
	}
	return;
}
