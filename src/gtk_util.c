#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pql_defines.h"
#include "pql_externs.h"
#include "gtk_pql_externs.h"

enum {
	LABEL1,
	LABEL2,
	LABEL3
};

void make_gc(GtkWidget *widget, int scr)
{
	int i;

	GXC[NORMALGC][scr] = gdk_gc_new(widget->window);	// normal GC
  	gdk_gc_set_rgb_fg_color (GXC[NORMALGC][scr], &settings.colors.fg[scr]);
  	gdk_gc_set_rgb_bg_color (GXC[NORMALGC][scr], &settings.colors.bg[scr]);

	GXC[ABNORMALGC][scr] = gdk_gc_new(widget->window);	// abnormal GC
  	gdk_gc_set_rgb_bg_color (GXC[ABNORMALGC][scr], &settings.colors.fg[scr]);
  	gdk_gc_set_rgb_fg_color (GXC[ABNORMALGC][scr], &settings.colors.bg[scr]);

	GXC[INVGC][scr] = gdk_gc_new(widget->window);	// inversion GC
  	gdk_gc_set_rgb_bg_color (GXC[INVGC][scr], &settings.colors.bg[scr]);
  	gdk_gc_set_rgb_fg_color (GXC[INVGC][scr], &settings.colors.fg[scr]);
	gdk_gc_set_function (GXC[INVGC][scr], GDK_INVERT);

	if (scr < 2)
	{	
		for(i=0;i<MAXGAPC;i++)
		{		// set up GAP colours for TRACE and MAG screens only
			GGXC[scr][i] = gdk_gc_new(widget->window);	// normal GC
  			gdk_gc_set_rgb_fg_color (GGXC[scr][i], &settings.colors.gp[scr][i]);
  			gdk_gc_set_rgb_bg_color (GGXC[scr][i], &settings.colors.bg[scr]);
			settings.colors.gp[scr][i].pixel = TRUE;
		}
		for(i=0;i<MAXOLAPC;i++)
		{		// set up OVERLAP colours
			PGXC[scr][i] = gdk_gc_new(widget->window);	// normal GC
  			gdk_gc_set_rgb_fg_color (PGXC[scr][i], &settings.colors.op[scr][i]);
  			gdk_gc_set_rgb_bg_color (PGXC[scr][i], &settings.colors.bg[scr]);
			settings.colors.op[scr][i].pixel = TRUE;
		}
	}

	settings.colors.bg[scr].pixel = TRUE;
	settings.colors.fg[scr].pixel = TRUE;
	settings.colors.sb[scr].pixel = TRUE;

	switch(scr)
	{
		case TRACESCR:
			settings.general.font.desc[LABELF] = pango_font_description_from_string (LABELFONT);
			settings.general.font.desc[AXISF] = pango_font_description_from_string (AXISFONT);
			settings.general.font.desc[MSGF] = pango_font_description_from_string (MSGFONT);

			if (!PRINTGC[NORMALGC])
			{	// do this only once per program invocation, it never needs to change
				PRINTGC[NORMALGC] = gdk_gc_new(widget->window);	// PRINT GC - white background, black foreground
  				gdk_gc_set_rgb_fg_color (PRINTGC[NORMALGC], &black);
  				gdk_gc_set_rgb_bg_color (PRINTGC[NORMALGC], &white);

				PRINTGC[ABNORMALGC] = gdk_gc_new(widget->window);	// abnormal GC
  				gdk_gc_set_rgb_bg_color (PRINTGC[ABNORMALGC], &black);
  				gdk_gc_set_rgb_fg_color (PRINTGC[ABNORMALGC], &white);

				PRINTGC[INVGC] = gdk_gc_new(widget->window);	// inversion GC
  				gdk_gc_set_rgb_fg_color (PRINTGC[INVGC], &black);
  				gdk_gc_set_rgb_bg_color (PRINTGC[INVGC], &white);
				gdk_gc_set_function (PRINTGC[INVGC], GDK_INVERT);
				
				settings.colors.sb[SPLITSCR].pixel = TRUE;
			}
		break;

		default:
			if (!OGXC[0])
			{
				for(i=0;i<MAXOVERLAYCOLOURS;i++)
				{
					OGXC[i] = gdk_gc_new(widget->window);
					gdk_gc_set_rgb_fg_color(OGXC[i], &settings.colors.ol[i]);
					settings.colors.ol[i].pixel = TRUE;
				}
			}
		break;
	}
	return;
}

void fontDraw(GdkDrawable *pxmp, char *str, int x, int y, int gcType, int scr, int screen_rec, int labelRec)
{
	PangoLayout *layout=NULL, **layoutPtr;
	static int	scrOLD;
	int	freeLO;
	GdkGC	*gc;

	if (scr != SAMESCR)
	{
		scrOLD = scr;
	}
	else
	{
		scr = scrOLD;
	}

	switch(gcType)
	{
		case NORMALGC:
			if (states.currentScreen == PRINT)
			{
				settings.general.font.GC = PRINTGC[NORMALGC];
				gc = PRINTGC[ABNORMALGC];
			}
			else
			{
				settings.general.font.GC = GXC[NORMALGC][scr];
				gc = GXC[ABNORMALGC][scr];
			}
			gdk_draw_rectangle (pxmp,
					gc,
					TRUE,
					x-2, y-1,
					fontSIZE(str, WIDTH) + 4,
					fontSIZE(str, HEIGHT) + 1);
		break;	
		case INVGC:
			if (states.currentScreen == PRINT)
			{
				settings.general.font.GC = PRINTGC[ABNORMALGC];
				gc = PRINTGC[NORMALGC];
			}
			else
			{
				settings.general.font.GC = GXC[ABNORMALGC][scr];
				gc = GXC[NORMALGC][scr];
			}
			gdk_draw_rectangle (pxmp,
					gc,
					TRUE,
					x-2, y-1,
					fontSIZE(str, WIDTH) + 4,
					fontSIZE(str, HEIGHT) + 1);
		break;

		case OVERLAY:
			if (states.currentScreen == PRINT)
			{
				gc = PRINTGC[ABNORMALGC];
			}
			else
			{
				gc = GXC[ABNORMALGC][scr];
			}
			settings.general.font.GC = OGXC[screen_rec%30];
			gdk_draw_rectangle (pxmp,
					gc,
					TRUE,
					x-2, y-1,
					fontSIZE(str, WIDTH) + 4,
					fontSIZE(str, HEIGHT) + 1);
		break;
	}

	layoutPtr = &layout;		// local and temporary
	freeLO = TRUE;				// free the layout we create
	switch(scr)
	{
		case MAGNIFYSCR:
		{
			traceInfo	*trcData = g_slist_nth_data(states.trace.selection, screen_rec);
			if (screen_rec != -1 &&
				!(labelRec == LABEL3))
			{
				layoutPtr = &trcData->magnify.plot.layout[labelRec];	// save MAG labels, speed up zooming
				freeLO = FALSE;	// don't free the layout, re-use
			}
		}
		break;
	}

	if (!settings.general.font.context[states.currentGraph])	// reset to NULL on fg or bg change by user
		settings.general.font.context[states.currentGraph] = gtk_widget_get_pango_context (settings.general.font.DA);
	
	if (!*layoutPtr)
	{
		*layoutPtr = pango_layout_new (settings.general.font.context[states.currentGraph]);
		pango_layout_set_text (*layoutPtr, str, -1);
		pango_layout_set_font_description (*layoutPtr, settings.general.font.desc[settings.general.font.font]);
	}
	gdk_draw_layout(pxmp, settings.general.font.GC, x, y, *layoutPtr);

	if (freeLO)
		g_object_unref (*layoutPtr);

	return;
}

int
fontSIZE(char *str, int type)
{
	PangoLayout *layout;
	int	width, height, ret=0;

	if (!settings.general.font.context[states.currentGraph])
		settings.general.font.context[states.currentGraph] = gtk_widget_get_pango_context (settings.general.font.DA);
    layout = pango_layout_new (settings.general.font.context[states.currentGraph]);
    pango_layout_set_text (layout, str, -1);
	pango_layout_set_font_description (layout, settings.general.font.desc[settings.general.font.font]);
	pango_layout_get_pixel_size (layout, &width, &height);

	switch(type)
	{
		case HEIGHT:
			ret = height;
		break;
		case WIDTH:
			ret = width;
		break;
	}
    g_object_unref (layout);
	return(ret);
}

//int stringWidth(int rec_no, int trace_depth, int string)
int stringWidth(traceInfo *trcData, float trace_depth, int string)
{
	static int	height=0;
	int 	middle, string_y1, string_y2, labelF;
	int 	str_width=0, str_width1;
	char	incr[10];

	switch(string)
	{
		case LABEL:
			if (!height)		// set this only once, it never changes
    			height = fontSIZE("F", HEIGHT);
    		middle = trace_depth / 2;
    		string_y1 = middle - height + 1;
    		string_y2 = middle + 3;

			if (settings.general.display.mode == GATHERMODE)
			{
				labelF = LABEL_GATHERMODE;
			} else if ((settings.general.label.format==LABEL_HEADER) && 
						((string_y1 < 0) || ((string_y2 + height) > trace_depth)))
			{
				labelF = LABEL_FILE_SHORT;		// vertical space can't allow two lines, force only one
			}
			else
			{
				labelF = settings.general.label.format;
			}
			switch(labelF)
			{
				case LABEL_FILE:
					str_width = fontSIZE(trcData->data.head.longFN, WIDTH);
				break;
				case LABEL_FILE_SHORT:
					str_width = fontSIZE(trcData->data.head.shortFN, WIDTH);
				break;
				case LABEL_HEADER:
					str_width = fontSIZE(trcData->data.head.descr1, WIDTH);
					str_width1 = fontSIZE(trcData->data.head.descr2, WIDTH);
					if (str_width < str_width1)
						str_width = str_width1;
				break;
				case LABEL_GATHERMODE:
				{
					char *str = g_strdup_printf("%s:%s", trcData->data.head.sensorID,
									trcData->data.head.channel);
					int str_width1;
					str_width = fontSIZE(str, WIDTH);
					free(str);
					str_width1 = fontSIZE(str, WIDTH);
					free(str);
					if (str_width1 > str_width)
						str_width = str_width1;
				}
				break;
			}
		break;

		case INC:
			sprintf(incr, "%d", trcData->trace.plot.increment);
			str_width = fontSIZE(incr, WIDTH);
		break;
	}

	return(str_width);
}

void
printLabel(GdkPixmap *pixmap, traceInfo *trcData, int screen_rec, 
			float trace_depth, int trace_width, int scr)
{

	static int	height=0;
	int 	string_y1, string_y2, labelF, GCtype, string_y3=0;
	int 	extra, win_width, win_height, fontW;
	char	*dec=NULL;
	float	middle;

	gdk_drawable_get_size(pixmap, &win_width, &win_height);
	settings.general.font.font = LABELF;
	GCtype = NORMALGC;
	settings.general.font.DA = DA[scr];

	if (!height)		// set this only once, it never changes
    	height = fontSIZE("F", HEIGHT);
    middle = trace_depth / 2;
    string_y1 = middle - height + 1;
    string_y2 = middle + 3;

	extra = 0;
	switch (scr)
	{
		case TRACESCR:
			GCtype = trcData->trace.selected;
//   			dec = g_strdup_printf("%d", trcData->trace.plot.increment);
		break;

		case SPLITSCR:
//  			dec = g_strdup_printf("%d", trcData->trace.plot.increment);
			scr = TRACESCR;								// reset scr for call to fontDraw
		break;

		case MAGNIFYSCR:
			GCtype = trcData->magnify.selected;
			extra = MAG_T_MARGIN;
			if (!settings.magnify.overlay &&
				settings.general.display.mode == TRACEMODE) 
			{
				extra += screen_rec*MAG_TFORM_MARGIN;
			}
   			dec = g_strdup_printf("%d", trcData->magnify.plot.increment);
  			if (settings.magnify.overlay)
  			{
				GCtype = OVERLAY;
			}
		break;

		case SPECTRASCR:
			extra = MAG_T_MARGIN;
  			if (settings.spectra.display.overlay)
  			{
				GCtype = OVERLAY;
			}
		break;
	}

	if (settings.general.display.mode == GATHERMODE)
	{
		labelF = LABEL_GATHERMODE;
	} else if ((settings.general.label.format==LABEL_HEADER) && 
				((string_y1 < 0) || ((string_y2 + height) > trace_depth )))
	{
		labelF = LABEL_FILE_SHORT;		// vertical space can't allow two lines, force only one
	}
	else
	{
		labelF = settings.general.label.format;
	}

	if (scr == MAGNIFYSCR && 
		trace_width &&
		labelF != LABEL_GATHERMODE)
	{	// increment relevant only for MAG screen and when requested to print
		fontW = fontSIZE(dec, WIDTH);
		fontDraw(pixmap, dec, trace_width + (win_width - trace_width)/2 - fontW/2 + PQLINCXTRA/4,
					(trace_depth*screen_rec) + middle-height/2+1 + extra, 
					NORMALGC, scr, -1, -1);
		free(dec);
	}

	switch(labelF)
	{
		case LABEL_FILE:
			string_y2 = middle-height/2+1;
			fontDraw(pixmap, trcData->data.head.longFN, 4, 
							(trace_depth*screen_rec) + string_y2 + extra, GCtype, scr, 
							screen_rec, LABEL1);
		    string_y3 = string_y2 + height + 1;
		break;

		case LABEL_FILE_SHORT:
			string_y2 = middle-height/2+1;
			fontDraw(pixmap, trcData->data.head.shortFN, 4, 
							(trace_depth*screen_rec) + string_y2 + extra, GCtype, scr, 
							screen_rec, LABEL1);
		    string_y3 = string_y2 + height + 1;
		break;

		case LABEL_HEADER:
			fontDraw(pixmap, trcData->data.head.descr1, 4, 
							(trace_depth*screen_rec) + string_y1 + extra, GCtype, scr, 
							screen_rec, LABEL1);
			fontDraw(pixmap, trcData->data.head.descr2, 4, 
							(trace_depth*screen_rec) + string_y2 + extra, GCtype, scr, 
							screen_rec, LABEL2);
		    string_y3 = string_y2 + height + 1;
		break;
		
		case LABEL_GATHERMODE:
		{
			char *str = g_strdup_printf("%s:%s", trcData->data.head.sensorID,
									trcData->data.head.channel);
			string_y2 = middle-height/2+1;
			fontDraw(pixmap, str, 4, (trace_depth*screen_rec) + string_y2 + extra, 
						GCtype, scr, screen_rec, LABEL1);
		    string_y3 = string_y2 + height + 1;
			free(str);
		}
		break;
	}
	
	switch(scr)
	{
		case MAGNIFYSCR:
			if (labelF != LABEL_GATHERMODE)
			{
				char *rms = g_strdup_printf("RMS = %.1f", trcData->magnify.RMS);
				fontDraw(pixmap, rms, 4, (trace_depth*screen_rec) + string_y3 + extra, 
							GCtype, scr, screen_rec, LABEL3);
				free(rms);
			}
		break;
	}
}

void dispMsg(int action, char	*msg)
{
	static guint	msgID;
	static guint	msgContext;

	if(!msgContext)
		msgContext = gtk_statusbar_get_context_id((GtkStatusbar *) statusBar, "DATASCREENS");

	if (msgID)
		gtk_statusbar_remove((GtkStatusbar *) statusBar, msgContext, msgID);

	switch(action)
	{
		case DISPLAY:
			msgID = gtk_statusbar_push((GtkStatusbar *) statusBar, msgContext, msg);
		break;

		case UNDISPLAY:
			gtk_statusbar_pop((GtkStatusbar *) statusBar, msgContext);
		break;
	}

	return ;
}

#ifdef PQL_ONLY
gint about(GtkButton *button, gpointer nil)
{
	static GtkWidget *aboutD; 
	GtkWidget *label, *hbox;
	char	str[50];

	if (!aboutD)
	{
		aboutD = gtk_dialog_new_with_buttons("About PQL",
										GTK_WINDOW (topWindow),
										GTK_DIALOG_DESTROY_WITH_PARENT,
										GTK_STOCK_OK, GTK_RESPONSE_OK,
										NULL);

		label = gtk_label_new(NULL);
		gtk_label_set_markup(GTK_LABEL(label),
			(const gchar *) "<span font_desc=\"12\">\tPQL (PASSCAL Quick Look)\t\n</span>");
		gtk_container_add(GTK_CONTAINER(GTK_DIALOG(aboutD)->vbox), label);

		hbox = gtk_hbox_new(FALSE, 0);
		label = gtk_label_new(NULL);
		sprintf(str, "<span font_desc=\"11\">Version:	%s\n</span>", PROG_VERSION);
		gtk_label_set_markup(GTK_LABEL(label), str);
		gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
		gtk_container_add(GTK_CONTAINER(GTK_DIALOG(aboutD)->vbox), hbox);

		hbox = gtk_hbox_new(FALSE, 0);
		label = gtk_label_new(NULL);
		gtk_label_set_markup(GTK_LABEL(label),
			(const gchar *) "<span font_desc=\"10\">Authorship:</span>");
		gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
		gtk_container_add(GTK_CONTAINER(GTK_DIALOG(aboutD)->vbox), hbox);

		hbox = gtk_hbox_new(FALSE, 0);
		label = gtk_label_new(NULL);
		gtk_label_set_markup(GTK_LABEL(label),
			(const gchar *) "<span font_desc=\"10\">\tPQL I\n\t\tversion 1: Richard Boaz - 1991\n\t\tversion 2: Sid Hellman - 1993</span>");
		gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
		gtk_container_add(GTK_CONTAINER(GTK_DIALOG(aboutD)->vbox), hbox);

		hbox = gtk_hbox_new(FALSE, 0);
		label = gtk_label_new(NULL);
		gtk_label_set_markup(GTK_LABEL(label),
			(const gchar *) "<span font_desc=\"10\">\tPQL II\n\t\tversion 1: Richard Boaz - 2005\n</span>");
		gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
		gtk_container_add(GTK_CONTAINER(GTK_DIALOG(aboutD)->vbox), hbox);

		hbox = gtk_hbox_new(FALSE, 0);
		label = gtk_label_new(NULL);
		gtk_label_set_markup(GTK_LABEL(label),
			(const gchar *) "<span font_desc=\"10\">License:</span>");
		gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
		gtk_container_add(GTK_CONTAINER(GTK_DIALOG(aboutD)->vbox), hbox);

		hbox = gtk_hbox_new(FALSE, 0);
		label = gtk_label_new(NULL);
		gtk_label_set_markup(GTK_LABEL(label),
			(const gchar *) "<span font_desc=\"10\">\tCopyright (C):\tRichard I. Boaz - 2005</span>");
		gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
		gtk_container_add(GTK_CONTAINER(GTK_DIALOG(aboutD)->vbox), hbox);

		hbox = gtk_hbox_new(FALSE, 0);
		label = gtk_label_new(NULL);
		gtk_label_set_markup(GTK_LABEL(label),
			(const gchar *) "<span font_desc=\"10\">\tGNU General Public License, version 2\n</span>");
		gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
		gtk_container_add(GTK_CONTAINER(GTK_DIALOG(aboutD)->vbox), hbox);

		hbox = gtk_hbox_new(FALSE, 0);
		label = gtk_label_new(NULL);
		gtk_label_set_markup(GTK_LABEL(label),
			(const gchar *) "<span font_desc=\"10\">Comments, Suggestions, and Bug Reports to:\t\n</span>");
		gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
		gtk_container_add(GTK_CONTAINER(GTK_DIALOG(aboutD)->vbox), hbox);

		hbox = gtk_hbox_new(FALSE, 0);
		label = gtk_label_new(NULL);
		gtk_label_set_markup(GTK_LABEL(label),
			(const gchar *) "<span font_desc=\"10\">\temail: ivor.boaz@gmail.com\n\temail: passcal@passcal.nmt.edu\n</span>");
		gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
		gtk_container_add(GTK_CONTAINER(GTK_DIALOG(aboutD)->vbox), hbox);
	}
	gtk_widget_show_all(aboutD);

	gtk_dialog_run(GTK_DIALOG (aboutD));

	gtk_widget_hide(aboutD);

	return TRUE;
}
#endif

int getMinDisp()
{
	int 	min;
	GSList *trcIter;
	traceInfo *trcData;
	
	for (trcIter = states.trace.selection, min=100;
		 trcIter;
		 trcIter = g_slist_next(trcIter))
	{
		trcData = trcIter->data;
		if (trcData->magnify.begin == -1)
			continue;		// not interested in traces not on display (absolute time mode)

		if (trcData->magnify.num_points_shown < min)
			min = trcData->magnify.num_points_shown;
	}
	return min;
}

gint getInstSOH(INSTSOH *one, traceInfo *trc)
{
	int	ret;
	char *first, *second;

	first = g_strdup_printf("%s:%s", one->fileName, one->instID);
	second = g_strdup_printf("%s:%s", trc->data.fileName, trc->data.head.sensorID);
#if 0
	switch(trc->data.fileType)
	{
		case RT125_DATA:
		case RT125A_DATA:
			first = g_strdup_printf("%s:%s", one->fileName, one->instID);
			second = g_strdup_printf("%s:%s", trc->data.fileName, trc->data.head.sensorID);
		break;
		case RT130_DATA:
			first = g_strdup_printf("%s:%s:%s", one->fileName, one->instID, one->channel);
			second = g_strdup_printf("%s:%s:%s", trc->data.fileName, trc->data.head.sensorID, 
										trc->data.head.channel);
		break;
	}
#endif
	ret = strcmp(first, second);
	free(first); free(second);
	return (ret);
}

gint insertInstSOH(INSTSOH *one, INSTSOH *two)
{
	int	ret;
	char *first, *second;

	first = g_strdup_printf("%s:%s", one->fileName, one->instID);
	second = g_strdup_printf("%s:%s", two->fileName, two->instID);
#if 0
	switch((int) one->channel)
	{
		case 0:
			first = g_strdup_printf("%s:%s", one->fileName, one->instID);
			second = g_strdup_printf("%s:%s", two->fileName, two->instID);
		break;
		default:
			first = g_strdup_printf("%s:%s:%s", one->fileName, one->instID, one->channel);
			second = g_strdup_printf("%s:%s:%s", two->fileName, two->instID, two->channel);
		break;
	}
#endif
	ret = strcmp(first, second);
	free(first); free(second);
	return (ret);
}
