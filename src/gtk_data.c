#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pql_defines.h"
#include "pql_externs.h"
#include "gtk_pql_externs.h"

gboolean getUserMaxMin(char *mxmnS, double *mxmn)
{
	// return converted user-specification
	// TRUE = as a percentage
	// FALSE = as specified by user
	gboolean	ret = FALSE;
	if (strchr(mxmnS, '%'))
	{	// make fixed MAX as a percentage
		sscanf(mxmnS, "%lf%%", mxmn);
		ret = TRUE;
	}
	else
	{	// user-specified MAX
		if (strlen(mxmnS))
		{	// set only if a value is specified
			sscanf(mxmnS, "%lf", mxmn);
		}
	}
	return ret;
}

static void check_colors(GtkWidget *widget, int reset)
{
	int i, j;

	for (i=0;i<NUMDATASCREENS-1;i++)
	{
		if (ctrl_settings.colors.fg[i].pixel != settings.colors.fg[i].pixel || 
			ctrl_settings.colors.bg[i].pixel != settings.colors.bg[i].pixel || 
			reset)
		{	// has fore- or background colours changed for any screen?
			if (GXC[NORMALGC][i])
			{
				g_object_unref(GXC[NORMALGC][i]);
				g_object_unref(GXC[ABNORMALGC][i]);
				g_object_unref(GXC[INVGC][i]);
				GXC[NORMALGC][i] = NULL;
			}
			states.split.resetPixmaps = TRUE;
			settings.general.font.context[i] = NULL;
			states.reset.refresh[i] = TRUE;
			states.split.refreshPixmap[SPLIT2][i] =
			states.split.refreshPixmap[SPLIT3][i] = TRUE;
		}

		if (i<2)		// check for the gap colours
		{	// check for gap colour changes on TRACE and MAG screens
			for (j=0;j<MAXGAPC;j++)
			{
				if (ctrl_settings.colors.gp[i][j].pixel != settings.colors.gp[i][j].pixel || reset)
				{
					if (GGXC[i][j])
					{
						g_object_unref(GGXC[i][j]);
						GGXC[i][j] = gdk_gc_new(gtk_widget_get_parent_window(GTK_WIDGET (widget)));
						gdk_gc_set_rgb_fg_color(GGXC[i][j], &ctrl_settings.colors.gp[i][j]);
					}
					ctrl_settings.colors.gp[i][j].pixel = TRUE;
					states.reset.refresh[i] = TRUE;
					states.split.refreshPixmap[SPLIT2][i] =
					states.split.refreshPixmap[SPLIT3][i] = TRUE;
				}
			}
		}

		if (i<2)		// check for the overlap colours
		{
			for (j=0;j<MAXOLAPC;j++)
			{	// check for overlap colour changes on TRACE and MAG screens
				if (ctrl_settings.colors.op[i][j].pixel != settings.colors.op[i][j].pixel || reset)
				{
					if (PGXC[i][j])
					{
						g_object_unref(PGXC[i][j]);
						PGXC[i][j] = gdk_gc_new(gtk_widget_get_parent_window(GTK_WIDGET (widget)));
						gdk_gc_set_rgb_fg_color(PGXC[i][j], &ctrl_settings.colors.op[i][j]);
					}
					ctrl_settings.colors.op[i][j].pixel = TRUE;
					states.reset.refresh[i] = TRUE;
					states.split.refreshPixmap[SPLIT2][i] =
					states.split.refreshPixmap[SPLIT3][i] = TRUE;
				}
			}
		}
	}

	for (i=0;i<MAXOVERLAYCOLOURS;i++)
	{
		if (ctrl_settings.colors.ol[i].pixel != settings.colors.ol[i].pixel || reset)
		{
			if (OGXC[i])
			{
				g_object_unref(OGXC[i]);
				OGXC[i] = gdk_gc_new(gtk_widget_get_parent_window(GTK_WIDGET (widget)));
				gdk_gc_set_rgb_fg_color(OGXC[i], &ctrl_settings.colors.ol[i]);
			}

			ctrl_settings.colors.ol[i].pixel = TRUE;

			states.reset.reset[MAGNIFYSCR] = TRUE;
			states.reset.refresh[SPECTRASCR] = TRUE;
			settings.general.font.context[MAGNIFYSCR] = NULL;
			states.split.refreshPixmap[SPLIT2][MAGNIFYSCR] =
			states.split.refreshPixmap[SPLIT3][MAGNIFYSCR] = TRUE;
			states.split.refreshPixmap[SPLIT2][SPECTRASCR] =
			states.split.refreshPixmap[SPLIT3][SPECTRASCR] = TRUE;
		}
	}

	for (i=0;i<NUMDATASCREENS;i++)		// set sidebar values
	{
		if (ctrl_settings.colors.sb[i].pixel != settings.colors.sb[i].pixel || reset)
		{
			gtk_widget_modify_bg(buttons[i], GTK_STATE_NORMAL, &ctrl_settings.colors.sb[i]);
		}
	}
}

void set_controls()
{
	GtkWidget	*radioB=NULL;
	char		tmpStr[50];
	int			i;

	memcpy(&ctrl_settings, &settings, sizeof(plot_settings));

	// TRACE control options
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON 
			(dispMode[ctrl_settings.general.display.mode]),	TRUE);
#ifdef PQL_ONLY
	dispModeShow(dispMode[TRACEMODE], NULL);
	gtk_spin_button_set_value((GtkSpinButton *) Tspread,
								ctrl_settings.general.display.gather.spread);
	gtk_spin_button_set_value((GtkSpinButton *) TshotLen,
								ctrl_settings.general.input.shotLength);
#endif
	gtk_spin_button_set_value((GtkSpinButton *) Tnum_files, ctrl_settings.trace.numPlots);
	gtk_spin_button_set_value((GtkSpinButton *) TmagInt, ctrl_settings.magnify.length);

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (Tlabel[ctrl_settings.general.label.format]), TRUE);			

	gtk_spin_button_set_value((GtkSpinButton *) TmaxPix, ctrl_settings.general.label.length);
	
	if (ctrl_settings.general.mark.fileN)
		gtk_entry_set_text(GTK_ENTRY(TmarkFN), ctrl_settings.general.mark.fileN);

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (Tgap), !ctrl_settings.trace.gFlag);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (Tolap), !ctrl_settings.trace.oFlag);

	// MAGNIFY control options
	gtk_spin_button_set_value((GtkSpinButton *) MZoomSpeed, ctrl_settings.magnify.zoomFactor);
	gtk_spin_button_set_value((GtkSpinButton *) MScrollFact, ctrl_settings.magnify.scrollFactor);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (yaxisConv), ctrl_settings.general.unitsConv);

	if (ctrl_settings.magnify.pickFile)
		gtk_entry_set_text(GTK_ENTRY(MpickFN), ctrl_settings.magnify.pickFile);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (MpickKEYind), !ctrl_settings.magnify.pickKEYOn);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (MpickCOMind), !ctrl_settings.magnify.pickCOMOn);

	for(i=0;i<MAXPICKS;i++)
	{
		gtk_entry_set_text(GTK_ENTRY(MpickTag[i]), ctrl_settings.magnify.pickTag[i]);
	}
		
//	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (MZDisp), !ctrl_settings.magnify.dispBox[ZOOMBOX]);
//	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (MPDisp), !ctrl_settings.magnify.dispBox[PICKBOX]);

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (Mgap), !ctrl_settings.magnify.gFlag);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (Molap), !ctrl_settings.magnify.oFlag);

	// SPECTRA control options
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (Samp), !ctrl_settings.spectra.display.yAxis);
	
	sprintf(tmpStr, "%f", ctrl_settings.spectra.display.yMax);
	gtk_entry_set_text(GTK_ENTRY(SyMax), tmpStr);

	gtk_spin_button_set_value((GtkSpinButton *) Sdecades, ctrl_settings.spectra.display.decades);

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (Sfreq), !ctrl_settings.spectra.display.xAxis);
	
	sprintf(tmpStr, "%f", ctrl_settings.spectra.display.xMax);
	gtk_entry_set_text(GTK_ENTRY(SxMax), tmpStr);

	sprintf(tmpStr, "%f", ctrl_settings.spectra.display.xMin);
	gtk_entry_set_text(GTK_ENTRY(SxMin), tmpStr);

	// SPLIT control options
#if 0
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (Hradios[SPLIT2][TOP][settings.split.DA[SPLIT2][TOP]]), TRUE);		
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (Hradios[SPLIT2][BOT][settings.split.DA[SPLIT2][BOT]]), TRUE);		
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (Hradios[SPLIT3][TOP][settings.split.DA[SPLIT3][TOP]]), TRUE);		
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (Hradios[SPLIT3][MID][settings.split.DA[SPLIT3][MID]]), TRUE);		
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (Hradios[SPLIT3][BOT][settings.split.DA[SPLIT3][BOT]]), TRUE);		
#endif

	gtk_color_button_set_color(GTK_COLOR_BUTTON (colorsSB[SPLITSCR]), &ctrl_settings.colors.sb[SPLITSCR]);

	// FILTER controls
	// now found in gtk_filters.c - no longer a part of the controls panel
	
	// GENERAL controls
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (TFuvw[settings.general.tForm.uvwSensor]), TRUE);

	// PRINT control options
	
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (TprtFrmt[ctrl_settings.general.print.format]), TRUE);
	if (ctrl_settings.general.print.fileN)
		gtk_entry_set_text(GTK_ENTRY(TprtFnm), ctrl_settings.general.print.fileN);
	if (ctrl_settings.general.print.command)
		gtk_entry_set_text(GTK_ENTRY(TprtCmd), ctrl_settings.general.print.command);
	switch(ctrl_settings.general.print.paperSize)
	{
		case LETTER:
			radioB = TprtPSize1;
			break;
		case A4:
			radioB = TprtPSize2;
			break;
	}
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (radioB), TRUE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (TprtPRes2), !ctrl_settings.general.print.paperRes);

	// SORT control options
	gtk_combo_box_set_active(GTK_COMBO_BOX(sortDefault), ctrl_settings.general.sort.defaultSort);
	gtk_combo_box_set_active(GTK_COMBO_BOX(sortType), ctrl_settings.general.sort.selected);
			
	//undone: make setting of color buttons more efficient: only set upon a change since setting is retained between
	//control screen invocations
	for (i=0;i<NUMDATASCREENS-1;i++)
	{
		gtk_color_button_set_color(GTK_COLOR_BUTTON (colorsSB[i]), &ctrl_settings.colors.sb[i]);
		gtk_color_button_set_color(GTK_COLOR_BUTTON (colorsFG[i]), &ctrl_settings.colors.fg[i]);
		gtk_color_button_set_color(GTK_COLOR_BUTTON (colorsBG[i]), &ctrl_settings.colors.bg[i]);
		if (i<2) 
		{	// set gap and overlap for TRACE and MAG screens only
			gtk_color_button_set_color(GTK_COLOR_BUTTON (colorsGP[i][0]), &ctrl_settings.colors.gp[i][0]);
			gtk_color_button_set_color(GTK_COLOR_BUTTON (colorsOP[i][0]), &ctrl_settings.colors.op[i][0]);
		}
	}

	// OVERLAY control options
	for (i=0;i<MAXOVERLAYCOLOURS;i++)
	{
		gtk_color_button_set_color(GTK_COLOR_BUTTON (colorsOL[i]), &ctrl_settings.colors.ol[i]);
	}

	// set up the HEADER screen options
	for (i=0;i<NUM_HDR_COLS;i++)
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (hdrToggles[i]), settings.headers.dispHdr[i]);
	}

	if (settings.general.display.mode == GATHERMODE)
		gtk_widget_set_sensitive(sortButton, FALSE);
	else
		gtk_widget_set_sensitive(sortButton, TRUE);
		
	return;
}

void get_settings(GtkButton *button, int *nothing)
{
	char	tmpStr[50];
	int		h, i;
	gboolean	reinit = FALSE;

	// WINDOW option
	ctrl_settings.general.startUP.winDims[WIDTH] = topWindow->allocation.width;
	ctrl_settings.general.startUP.winDims[HEIGHT] = topWindow->allocation.height;
	if (states.split.realized[1])
	{
		ctrl_settings.general.startUP.panePos[0] = gtk_paned_get_position((GtkPaned *) Hpanes[0]);
		ctrl_settings.general.startUP.panePos[1] = gtk_paned_get_position((GtkPaned *) Hpanes[1]);
		ctrl_settings.general.startUP.panePos[2] = gtk_paned_get_position((GtkPaned *) Hpanes[2]);
		ctrl_settings.split.pane = states.split.panes;
	}

#ifdef PQL_ONLY
	// TRACE control options
	for(i=0;i<TTLPQLDISPLAYMODES;i++)
	{
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (dispMode[i])))
		{
			ctrl_settings.general.display.mode = i;
			break;
		}
	}
	switch(ctrl_settings.general.display.mode)
	{
		case TRACEMODE:
			gtk_widget_hide(dispGainB[TRACESCR]);
//			gtk_widget_hide(dispGainB[MAGNIFYSCR]);
			gtk_widget_hide(dispGainB[SPECTRASCR]);
			if (fltrAGCTab)
				gtk_widget_set_sensitive(fltrAGCTab, FALSE);
		break;
		case GATHERMODE:
			if (fltrAGCTab)
				gtk_widget_set_sensitive(fltrAGCTab, TRUE);
			gtk_widget_show(dispGainB[TRACESCR]);
//			gtk_widget_show(dispGainB[MAGNIFYSCR]);
			gtk_widget_show(dispGainB[SPECTRASCR]);
		break;
	}
	ctrl_settings.general.display.gather.spread = gtk_spin_button_get_value((GtkSpinButton *) Tspread); 
	ctrl_settings.general.input.shotLength = gtk_spin_button_get_value((GtkSpinButton *) TshotLen); 
#endif
	ctrl_settings.trace.numPlots = gtk_spin_button_get_value((GtkSpinButton *) Tnum_files); 
	ctrl_settings.magnify.length = gtk_spin_button_get_value((GtkSpinButton *) TmagInt); 
	if (ctrl_settings.magnify.length == 0)
		ctrl_settings.magnify.length = states.trace.relTimeLen / 10.;		// reset to 10% 

	for(i=0;i<TTLLABELS;i++)
	{
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (Tlabel[i])) == TRUE)
		{
			ctrl_settings.general.label.format = i;
			break;
		}
	}

	ctrl_settings.general.label.length = gtk_spin_button_get_value((GtkSpinButton *) TmaxPix); 
	
	if (ctrl_settings.general.mark.fileN)
		free(ctrl_settings.general.mark.fileN);
	ctrl_settings.general.mark.fileN = g_strdup(gtk_entry_get_text(GTK_ENTRY(TmarkFN)));
	ctrl_settings.trace.gFlag = !gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (Tgap));
	ctrl_settings.trace.oFlag = !gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (Tolap));

	// MAGNIFY control options
	ctrl_settings.magnify.zoomFactor = gtk_spin_button_get_value((GtkSpinButton *) MZoomSpeed); 
	ctrl_settings.magnify.scrollFactor = gtk_spin_button_get_value((GtkSpinButton *) MScrollFact);
	ctrl_settings.general.unitsConv = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (yaxisConv));

	if (ctrl_settings.magnify.pickFile)
		free(ctrl_settings.magnify.pickFile);
	ctrl_settings.magnify.pickFile = g_strdup(gtk_entry_get_text(GTK_ENTRY(MpickFN)));
	ctrl_settings.magnify.pickKEYOn = !gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (MpickKEYind));
	ctrl_settings.magnify.pickCOMOn = !gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (MpickCOMind));

	for(i=0;i<MAXPICKS;i++)
	{
		strcpy(ctrl_settings.magnify.pickTag[i], gtk_entry_get_text(GTK_ENTRY(MpickTag[i])));
	}
		
//	ctrl_settings.magnify.dispBox[ZOOMBOX] = !gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (MZDisp));
//	ctrl_settings.magnify.dispBox[PICKBOX] = !gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (MPDisp));

	if (!ctrl_settings.magnify.dispBox[ZOOMBOX])
		gtk_widget_hide(MZoomBox);
	else
		gtk_widget_show(MZoomBox);

	if (!ctrl_settings.magnify.dispBox[PICKBOX])
		gtk_widget_hide(MPickBox);
	else
		gtk_widget_show(MPickBox);
	
	ctrl_settings.magnify.gFlag = !gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (Mgap));
	ctrl_settings.magnify.oFlag = !gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (Molap));

	// SPECTRA control options
	ctrl_settings.spectra.display.yAxis = !gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (Samp));
	
	strcpy(tmpStr, gtk_entry_get_text(GTK_ENTRY(SyMax)));
	if (strlen(tmpStr))
		sscanf(tmpStr, "%f", &ctrl_settings.spectra.display.yMax);
	else
		ctrl_settings.spectra.display.yMax = 0;

	ctrl_settings.spectra.display.decades = gtk_spin_button_get_value((GtkSpinButton *) Sdecades);

	ctrl_settings.spectra.display.xAxis = !gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (Sfreq));
	
	strcpy(tmpStr, gtk_entry_get_text(GTK_ENTRY(SxMax)));
	if (strlen(tmpStr))
		sscanf(tmpStr, "%f", &ctrl_settings.spectra.display.xMax);
	else
		ctrl_settings.spectra.display.xMax = 0;

	strcpy(tmpStr, gtk_entry_get_text(GTK_ENTRY(SxMin)));
	if (strlen(tmpStr))
		sscanf(tmpStr, "%f", &ctrl_settings.spectra.display.xMin);
	else
		ctrl_settings.spectra.display.xMin = 0;

	// SPLIT control options
	// if user has specified SPLIT2 MAG & SPECTRA, make TRACE our invisible screen
	if (ctrl_settings.split.DA[SPLIT2][SPLITDA2T-SPLITDA2T] != TRACESCR &&
		ctrl_settings.split.DA[SPLIT2][SPLITDA2B-SPLITDA2T] != TRACESCR)
		ctrl_settings.split.DA[SPLIT2][SPLITDA2M-SPLITDA2T] = TRACESCR;
	else
		ctrl_settings.split.DA[SPLIT2][SPLITDA2M-SPLITDA2T] = NONE;
		
	// FILTER controls
	// now found in gtk_filters.c - no longer a part of the controls panel

	// GENERAL control options
	for (i=0; i< TTLSENSORS; i++)
	{
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (TFuvw[i])))
			ctrl_settings.general.tForm.uvwSensor = i;
	}

	// PRINT control options
	for(i=0;i<TTLPRINTFMTS;i++)
	{
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (TprtFrmt[i])))
			ctrl_settings.general.print.format = i;
	}
	if (ctrl_settings.general.print.fileN)
		free(ctrl_settings.general.print.fileN);
	ctrl_settings.general.print.fileN = g_strdup(gtk_entry_get_text(GTK_ENTRY(TprtFnm)));
	if ((ctrl_settings.general.print.command))
		free(ctrl_settings.general.print.command);
	ctrl_settings.general.print.command = g_strdup(gtk_entry_get_text(GTK_ENTRY(TprtCmd)));

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (TprtPSize1)))
		ctrl_settings.general.print.paperSize = LETTER;
	else
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (TprtPSize2)))			
			ctrl_settings.general.print.paperSize = A4;
	ctrl_settings.general.print.paperRes = !gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (TprtPRes2));

	// COLOR control options
	check_colors(GTK_WIDGET (button), FALSE);                                       

	// SORT control options
	ctrl_settings.general.sort.defaultSort = gtk_combo_box_get_active(GTK_COMBO_BOX(sortDefault));

	// HEADER control options
	for (i=0;i<NUM_HDR_COLS;i++)
	{
		ctrl_settings.headers.dispHdr[i] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (hdrToggles[i]));
	}

	// check to see if anything changed forcing new pixmaps
	if (settings.trace.numPlots != ctrl_settings.trace.numPlots ||
		settings.general.label.format != ctrl_settings.general.label.format ||
		settings.general.label.length != ctrl_settings.general.label.length)
	{
		states.general.plot.state = READDATA;
	}

	if (settings.trace.gFlag != ctrl_settings.trace.gFlag ||
		settings.trace.oFlag != ctrl_settings.trace.oFlag) 
	{
		states.reset.refresh[TRACESCR] = TRUE;				// soft reset
		states.split.refreshPixmap[SPLIT2][TRACESCR] =
		states.split.refreshPixmap[SPLIT3][TRACESCR] = TRUE;
	}

	if (settings.spectra.display.decades != ctrl_settings.spectra.display.decades ||
		settings.spectra.display.yMax != ctrl_settings.spectra.display.yMax ||
		settings.spectra.display.yAxis != ctrl_settings.spectra.display.yAxis ||
		settings.spectra.display.xAxis != ctrl_settings.spectra.display.xAxis)
	{	// re-draw, do not recompute
		states.reset.refresh[SPECTRASCR] = TRUE;
		states.split.refreshPixmap[SPLIT2][SPECTRASCR] =
		states.split.refreshPixmap[SPLIT3][SPECTRASCR] = TRUE;
	}
	if (settings.spectra.display.xMax != ctrl_settings.spectra.display.xMax ||
		settings.spectra.display.xMin != ctrl_settings.spectra.display.xMin) 
	{	// re-draw and recompute
		states.reset.reset[SPECTRASCR] = TRUE;
	}

	for (i=0;i<NUM_HDR_COLS;i++)
	{
		if (settings.headers.dispHdr[i] != ctrl_settings.headers.dispHdr[i])
		{
			states.header.newCols = TRUE;
			break;		// only need to know if we need to remake
		}
	}

	if ((settings.magnify.gFlag != ctrl_settings.magnify.gFlag) ||
		(settings.magnify.oFlag != ctrl_settings.magnify.oFlag)) 
	{
		states.reset.refresh[MAGNIFYSCR] = TRUE;
		states.split.refreshPixmap[SPLIT2][MAGNIFYSCR] =
		states.split.refreshPixmap[SPLIT3][MAGNIFYSCR] = TRUE;
	}

	for(h=0;h<2;h++)
	{
		for(i=0;i<NUMDATASCREENS-1;i++)
		{
			if (ctrl_settings.split.DA[h][i] != settings.split.DA[h][i])
				states.split.refreshPixmap[h][i] = TRUE;
		}
	}

	if (ctrl_settings.trace.numPlots != settings.trace.numPlots)
		reinit = TRUE;

	memcpy(&settings, &ctrl_settings, sizeof(plot_settings));
	memcpy(&ctrl_settings, &settings, sizeof(plot_settings));

	if (reinit)
		makeDispList(INITGROUP);
	return;
}

void restore_settings(GtkButton *widget, int *nothing)
{
	int	save=states.split.panes;
	read_defaults_file(&settings, DEFAULTSET);		// bug! this resets too many values, e.g., states.split.panes
	check_colors(GTK_WIDGET (widget), TRUE);
	set_controls();
	states.split.panes = save;
	return;
}

static void add_log_columns(GtkTreeView *treeview, int logType)
{
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	
	switch(logType)
	{
		case LOGFILE:
			renderer = gtk_cell_renderer_text_new();
			column = gtk_tree_view_column_new_with_attributes("NTW.STN.LOC.CHN", renderer, "text", 0, NULL);
			gtk_tree_view_insert_column(treeview, column, 0);
			renderer = gtk_cell_renderer_text_new();
			column = gtk_tree_view_column_new_with_attributes("Start Time", renderer, "text", 1, NULL);
			gtk_tree_view_insert_column(treeview, column, 1);
			renderer = gtk_cell_renderer_text_new();
			column = gtk_tree_view_column_new_with_attributes("Stop Time", renderer, "text", 2, NULL);
			gtk_tree_view_insert_column(treeview, column, 2);
			renderer = gtk_cell_renderer_text_new();
			column = gtk_tree_view_column_new_with_attributes("FileName", renderer, "text", 3, NULL);
			gtk_tree_view_insert_column(treeview, column, 3);
		break;
		case LOGTRC:
			renderer = gtk_cell_renderer_text_new();
			column = gtk_tree_view_column_new_with_attributes("NTW.STN.LOC.CHN", renderer, "text", 0, NULL);
			gtk_tree_view_insert_column(treeview, column, 0);
			renderer = gtk_cell_renderer_text_new();
			column = gtk_tree_view_column_new_with_attributes("Start Time", renderer, "text", 1, NULL);
			gtk_tree_view_insert_column(treeview, column, 1);
			renderer = gtk_cell_renderer_text_new();
			column = gtk_tree_view_column_new_with_attributes("Stop Time", renderer, "text", 2, NULL);
			gtk_tree_view_insert_column(treeview, column, 2);
			renderer = gtk_cell_renderer_text_new();
			column = gtk_tree_view_column_new_with_attributes("FileName", renderer, "text", 3, NULL);
			gtk_tree_view_insert_column(treeview, column, 3);
		break;
	}
}

static void add_hdr_columns(GtkTreeView *treeview)
{
	int i, j, colsRemaining;
	GtkCellRenderer *renderer;
	static GtkTreeViewColumn *column[NUM_HDR_COLS];
	static int numCols;
	
	colsRemaining = numCols;
	for (i=j=0;i<NUM_HDR_COLS;i++)
	{
		char colTitle[50];

		if (column[j])
			strcpy(colTitle, gtk_tree_view_column_get_title(column[j]));
		else
			strcpy(colTitle, "forced-insert");

		switch(settings.headers.dispHdr[i])
		{
			case TRUE:
				if (!strcmp(colTitle, headers[i].desc))
				{	// column should be displayed and already is, do nothing and proceed to next column
					colsRemaining--;
					j++;
				}
				else
				{	// column should be displayed and isn't, insert
					if (colsRemaining)
					{
						memmove(&column[j+1], &column[j], sizeof(GtkTreeViewColumn *) * colsRemaining);
					}
					renderer = gtk_cell_renderer_text_new();
					column[j] = gtk_tree_view_column_new_with_attributes(headers[i].desc, renderer, "text", i, NULL);
					gtk_tree_view_insert_column(treeview, column[j], j);
					numCols++;
					j++;
				}
				break;

			case FALSE:
				if (!strcmp(colTitle, headers[i].desc))
				{	// column should NOT be displayed and already is, remove it
					gtk_tree_view_remove_column(treeview, column[j]);
					memmove(&column[j], &column[j+1], sizeof(GtkTreeViewColumn *) * (colsRemaining-1));
					column[colsRemaining-1+j] = NULL;
					numCols--;
					colsRemaining--;
				}
				else
				{	// column should NOT be displayed and isn't, do nothing and proceed to next
//					
				}
				break;
		}
	}
}

GtkListStore *headerStore, *logStore[TTLLOGTYPES];

GtkWidget *makeHdrModel()
{
	GtkWidget *treeview;

	headerStore = gtk_list_store_new(NUM_HDR_COLS, 
						G_TYPE_STRING, 
						G_TYPE_STRING,
						G_TYPE_STRING,
						G_TYPE_STRING,
						G_TYPE_STRING,
						G_TYPE_STRING,
						G_TYPE_STRING,
						G_TYPE_STRING,
						G_TYPE_STRING,
						G_TYPE_STRING,
						G_TYPE_STRING,
						G_TYPE_STRING,
						G_TYPE_STRING,
						G_TYPE_STRING,
						G_TYPE_STRING,
						G_TYPE_STRING,
						G_TYPE_STRING);

	treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL (headerStore));

	return (treeview);
}

GtkWidget *makeLOGModel(int which)
{
	GtkWidget *treeview;
	GtkListStore *store;

	switch (which)
	{
		case LOGFILE:
			store = logStore[which] = gtk_list_store_new(NUM_LOGF_HDR_COLS, 
						G_TYPE_STRING, 
						G_TYPE_STRING,
						G_TYPE_STRING,
						G_TYPE_STRING);
		break;
		case LOGTRC:
			store = logStore[which] = gtk_list_store_new(NUM_LOGT_HDR_COLS, 
						G_TYPE_STRING, 
						G_TYPE_STRING,
						G_TYPE_STRING,
						G_TYPE_STRING);
		break;
	}

	treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL (store));

	return (treeview);
}

static gboolean validFile(char *fileName, GSList *trcList)
{
	GSList	*iter;
	traceInfo	*trc;
	gboolean	valid;
	
	for(iter = trcList, valid = FALSE;
		iter && valid == FALSE;
		iter = g_slist_next(iter))
	{
		trc = iter->data;
		if (!strcmp(trc->data.fileName, fileName))
			valid = TRUE;
	}
	return valid;
}

static gboolean validTrace(traceInfo *trc)
{
	GSList	*iter;
	LOGDATA	*logData;
	gboolean	valid;
	
	for(iter = states.logs.logs, valid = FALSE;
		iter && valid == FALSE;
		iter = g_slist_next(iter))
	{
		logData = iter->data;
		if (!strcmp(trc->data.fileName, logData->fileName))
			valid = TRUE;
	}
	return valid;
}

void makeLOGData(int dispType, int display)
{
	GtkListStore 	*store;
	GtkTreeIter		iter;
	GSList			*trcList;
	struct ptime	date;
	char			*colStr;
	static gboolean first[TTLLOGTYPES] = { TRUE, TRUE };
	
	store = logStore[dispType];
	
	if (store)
		gtk_list_store_clear(store);

	if (first[dispType])
	{
		add_log_columns(GTK_TREE_VIEW (logVIEW[dispType]), dispType);
		first[dispType] = FALSE;
	}

	switch(display)
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

	switch (dispType)                                                               
	{
		case LOGFILE:
		{                                                            
			GSList	*logIter;
			LOGDATA *logData;
			for (logIter = states.logs.logs;
				 logIter;
				 logIter = g_slist_next(logIter))
			{
				logData = logIter->data;
				if (!validFile(logData->fileName, trcList))
					continue;

				gtk_list_store_append(store, &iter);
				colStr = g_strdup(logData->station);
				gtk_list_store_set(store, &iter, 0, colStr, -1);

				passcal_etoh(&date, (double) logData->startT);
				colStr = g_strdup_printf("%i %03i %02i:%02i:%02i" , 
									date.yr, date.jday,	date.hr, 
									date.mn, (int) date.sec);
				gtk_list_store_set(store, &iter, 1, colStr, -1);

				passcal_etoh(&date, (double) logData->endT);
				colStr = g_strdup_printf("%i %03i %02i:%02i:%02i" , 
									date.yr, date.jday,	date.hr, 
									date.mn, (int) date.sec);
				gtk_list_store_set(store, &iter, 2, colStr, -1);

				colStr = g_strdup(logData->fileName);
				gtk_list_store_set(store, &iter, 3, colStr, -1);
			}
		}
		break;

		case LOGTRC:
		{
			GSList	*trcIter;
			traceInfo *trc;
			for (trcIter = trcList;
				trcIter;
				trcIter = g_slist_next(trcIter))
			{
				trc = trcIter->data;
				if (!validTrace(trc))
					continue;

				gtk_list_store_append(store, &iter);
				colStr = g_strdup(trc->data.head.descr2);
				gtk_list_store_set(store, &iter, 0, colStr, -1);

				passcal_etoh(&date, (double) trc->data.head.epochs);
				colStr = g_strdup_printf("%i %03i %02i:%02i:%02i" , 
									date.yr, date.jday,	date.hr, 
									date.mn, (int) date.sec);
				gtk_list_store_set(store, &iter, 1, colStr, -1);

				passcal_etoh(&date, (double) (trc->data.head.epochs + trc->data.head.length));
				colStr = g_strdup_printf("%i %03i %02i:%02i:%02i" , 
									date.yr, date.jday,	date.hr,         
									date.mn, (int) date.sec);
				gtk_list_store_set(store, &iter, 2, colStr, -1);

				colStr = g_strdup(trc->data.fileName);
				gtk_list_store_set(store, &iter, 3, colStr, -1);
			}
		}
		break;
	}
}

#define location(a)	((long) trcData + (int) a)

void makeHdrData(int hdrDisp)
{
	GtkTreeIter	iter;
	struct ptime	date;
	int		msec, i, j, end=0, start;
	long	len;
	char	colStr[255], format[20];
	double	*double_ptr;
	traceInfo	*trcData;
	GSList		*trcList=NULL;

	if (headerStore)
		gtk_list_store_clear(headerStore);

	if (states.header.newCols)
	{
		add_hdr_columns(GTK_TREE_VIEW (hdrVIEW));
		states.header.newCols = FALSE;
	}

	switch(hdrDisp)
	{
		case ALL:
			trcList = traces;
			start = 0;
			end = states.general.numTraces;
		break;

		case DISPLAYED:
			trcList = states.trace.traces[CURGRP];
			start = 0;
			end = g_slist_length(trcList);
		break;

		case SELECTED:
			trcList = states.trace.selection;
			start = 0;
			end = states.trace.numSelected;
		break;
	}

	for(i=start; (i < end); ++i)
	{
		trcData = g_slist_nth_data(trcList, i);
		if (!trcData)
			break;

		gtk_list_store_append(headerStore, &iter);
		for (j=0;j<NUM_HDR_COLS;j++)
		{
			if (!settings.headers.dispHdr[j])
				continue;		// print only the columns we're interested in, we're only saving time, we could do this

			switch(headers[j].fieldType)
			{

				case MYDIRECTORY:
#ifdef WIN32
					(len = (long) (strrchr((char *) *((char **) location(headers[j].offset)), '\\')));
#else
					(len = (long) (strrchr((char *) *((char **) location(headers[j].offset)), '/')));
#endif
					len =  len	? len - (long) ((char *) *((char **) location(headers[j].offset))) 
									: 0;
					if (len)
					{
						strncpy(colStr, (char *) *((char **) location(headers[j].offset)), (size_t) len);
						colStr[len]=0;
					}
					else
					{
						strcpy(colStr, ".");
					}
				break;
				
				case MYSTRING:
					strcpy(colStr, (char *) location(headers[j].offset));
				break;

				case MYSTRINGPTR:
					strcpy(colStr, (char *) *((char **) location(headers[j].offset)));
				break;

				case MYFLOAT:
					sprintf(colStr, "%6.4e", *((float *) location(headers[j].offset)));
				break;

				case MYSHORT:
					sprintf(colStr, "%d", *((short *) location(headers[j].offset)));
				break;

				case MYINT:
					sprintf(colStr, "%d", *((int *) location(headers[j].offset)));
				break;

				case MYDOUBLE:
					sprintf(colStr, "%8.3f", *((double *) location(headers[j].offset)));
				break;

				case MYAMPS:
					if (trcData->data.head.units == PQL_UNITS_VOLTS || 
						trcData->data.head.data_form == MY_FLOAT ||
						trcData->data.head.data_form == MY_DOUBLE)
					{
						strcpy(format, "%9.4f / %9.4f");
						sprintf(colStr, format,
								*((double *) (location(headers[j].offset)+sizeof(double))),
								*((double *) location(headers[j].offset))); 
					}
					else
					{
						strcpy(format, "%d / %d");
						sprintf(colStr, format,
								(int) *((double *) (location(headers[j].offset)+sizeof(double))),
								(int) *((double *) location(headers[j].offset))); 
					}
				break;

				case MYEPOCHS:
					double_ptr = (double *) location(headers[j].offset);
					passcal_etoh(&date, *(double_ptr));
					msec = ((float) (*(double_ptr) - (int) *(double_ptr))*1000);
					sprintf(colStr,"%i %03i %02i:%02i:%02i.%03i" , 
							date.yr, date.jday,	date.hr, 
							date.mn, (int) date.sec, (int) msec);
				break;

				case MYFILETYPE:
					switch(*((int *) location(headers[j].offset)))
					{
						case SEGY_DATA:
							sprintf(colStr, "SEGY");
						break;

						case AH_DATA:
							sprintf(colStr, "AH");
						break;

						case SAC_DATA:
							sprintf(colStr, "SAC");
						break;

						case MSEED_DATA:
						case MSEED_MPLEX_DATA:
							sprintf(colStr, "mini-SEED");
						break;

						case NANO_DATA:
							sprintf(colStr, "NANO");
						break;

						case DR100_DATA:
							sprintf(colStr, "DR100");
						break;

						case RT130_DATA:
							sprintf(colStr, "RT130");
						break;

						case RT125A_DATA:
							sprintf(colStr, "RT125A");
						break;

						case RT125_DATA:
							sprintf(colStr, "RT125");
						break;
					}
				break;

				case MYSPS:
				{
					double_ptr = (double *) location(headers[j].offset);
					if (*(double_ptr) <= 1000000)
						sprintf(colStr, "%d", (int) ((1000000./ *(double_ptr))+.5));
					else
						sprintf(colStr, "%f", (double) (1000000./ *(double_ptr)));
				}
				break;
			}	// end switch(headers[j].fieldType)
			gtk_list_store_set(headerStore, &iter, j, colStr, -1);
		}	// end each column
	}		// end each row

	if (hdrDisp == ALL)
	{	// when displaying all, make the 1st trace record on display 
		// in TRACE screen the 1st record on display here
		char row[5];
		GtkTreePath *path;
		sprintf(row, "%d", states.trace.startRec);
		path = gtk_tree_path_new_from_string(row);
		gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW (hdrVIEW), path, NULL, TRUE, 0.0, 0.0);
		gtk_tree_view_set_cursor(GTK_TREE_VIEW (hdrVIEW), path, NULL, FALSE);
		gtk_tree_path_free (path);
	}

	dispMsg(UNDISPLAY, NULL);
}

