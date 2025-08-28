#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "pql_defines.h"
#include "pql_externs.h"
#include "gtk_pql_externs.h"

extern GtkTooltips *tips;
static char		toolTip[1000];

static void sortTypeSelect(GtkWidget *widget, gpointer nil)
{
	ctrl_settings.general.sort.selected = gtk_combo_box_get_active(GTK_COMBO_BOX(widget));
}

static gboolean destroyPqlControls(GtkWidget *widget, int *nothing)
{
	get_settings(NULL, NULL);
	raise_page(NULL, &states.currentPage);
	return TRUE;
}

gboolean setFG(GtkWidget *widget, gpointer s)
{
	int		scr = GPOINTER_TO_INT(s);
	gtk_color_button_get_color(GTK_COLOR_BUTTON (widget), &ctrl_settings.colors.fg[scr]);
	ctrl_settings.colors.fg[scr].pixel = FALSE;

	return TRUE;
}

gboolean setBG(GtkWidget *widget, gpointer s)
{
	int		scr = GPOINTER_TO_INT(s);
	gtk_color_button_get_color(GTK_COLOR_BUTTON (widget), &ctrl_settings.colors.bg[scr]);
	ctrl_settings.colors.bg[scr].pixel = FALSE;

	return TRUE;
}

gboolean setSB(GtkWidget *widget, gpointer s)
{
	int		scr = GPOINTER_TO_INT(s);
	gtk_color_button_get_color(GTK_COLOR_BUTTON (widget), &ctrl_settings.colors.sb[scr]);
	ctrl_settings.colors.sb[scr].pixel = FALSE;

	return TRUE;
}

gboolean setOL(GtkWidget *widget, gpointer i)
{
	int		idx = GPOINTER_TO_INT(i);

	gtk_color_button_get_color(GTK_COLOR_BUTTON (widget), &ctrl_settings.colors.ol[idx]);
	ctrl_settings.colors.ol[idx].pixel = FALSE;

	return TRUE;
}

gboolean setGP(GtkWidget *widget, gpointer nil)
{
	int	screen, colour, quit=FALSE;

	for (screen=0; screen < 2 && !quit; screen++)		// which radio button are we dealing with?
		for (colour=0; colour < MAXGAPC && !quit; colour++)
			if (widget == colorsGP[screen][colour])
					quit=TRUE;
	screen--; colour--; 

	gtk_color_button_get_color(GTK_COLOR_BUTTON (widget), &ctrl_settings.colors.gp[screen][colour]);
	ctrl_settings.colors.gp[screen][colour].pixel = FALSE;

	return TRUE;
}

gboolean setOP(GtkWidget *widget, gpointer nil)
{
	int	screen, colour, quit=FALSE;

	for (screen=0; screen < 2 && !quit; screen++)		// which radio button are we dealing with?
		for (colour=0; colour < MAXOLAPC && !quit; colour++)
			if (widget == colorsOP[screen][colour])
					quit=TRUE;
	screen--; colour--; 

	gtk_color_button_get_color(GTK_COLOR_BUTTON (widget), &ctrl_settings.colors.op[screen][colour]);
	ctrl_settings.colors.op[screen][colour].pixel = FALSE;
	memcpy(&ctrl_settings.colors.op[screen][1], &ctrl_settings.colors.op[screen][0], sizeof(GdkColor));
	ctrl_settings.colors.op[screen][1].red += (ctrl_settings.colors.op[screen][1].red > 57120) ? -7650 : 7650;
	ctrl_settings.colors.op[screen][1].green += (ctrl_settings.colors.op[screen][1].green > 57120) ? -7650 : 7650;
	ctrl_settings.colors.op[screen][1].blue += (ctrl_settings.colors.op[screen][1].blue > 57120) ? -7650 : 7650;

	return TRUE;
}

void	raiseControls(GtkButton *button, gpointer s)
{	// called by Controls button from each screen
	int		scr = GPOINTER_TO_INT(s);
	if (!ctrlWindow)
	{
		make_ctrlScreens();
	}

	set_controls();
	gtk_widget_show(ctrlWindow);
	gtk_widget_set_size_request(ctrlWindow, 0, 0);
	gtk_widget_set_size_request(ctrlWindow, -1, -1);
  	gtk_notebook_set_current_page((GtkNotebook *) control_screens, scr);

	return;
}

static GtkWidget	*CTraceModeB;

#ifdef PQL_ONLY
static GtkWidget	*CGatherModeB;
void	dispModeShow(GtkWidget *radio, gpointer nil)
{
	switch(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (radio)))
	{
		case TRUE:	// TRACEMODE
			gtk_widget_hide(CGatherModeB);
			gtk_widget_show(CTraceModeB);
			gtk_widget_set_sensitive(sortButton, TRUE);
		break;
		case FALSE:	// GATHERMODE
			gtk_widget_hide(CTraceModeB);
			gtk_widget_show(CGatherModeB);
			gtk_widget_set_sensitive(sortButton, FALSE);
		break;
	}
}
#endif

GtkWidget *make_controls(int type)
{
	GtkWidget	*controlBox, *control_box, *ctr, *label, *eBox, *hbox, *entry, *vbox;
	GtkWidget	*frame, *box, *radioB1, *radioB2, *controlFrame, 
				*radioB3, *actionB, *button, *combo, *box2;
	GtkWidget	*frame1, *box1, *sep;
	GtkAdjustment *spinAdj;
	static GtkSizeGroup	*size_group;

	char	str[25], pickTag[10];
	int		i, j, idx;
	struct timeval tv;
	struct timezone tz;

	control_box = gtk_event_box_new();

	switch(type)
	{
		case TRACECTRL:
			controlFrame = gtk_frame_new(NULL);
			controlBox = gtk_vbox_new(FALSE, 0);
			gtk_container_set_border_width(GTK_CONTAINER(controlBox), 5);
			gtk_container_add(GTK_CONTAINER(control_box), controlFrame);
			gtk_container_add(GTK_CONTAINER(controlFrame), controlBox);
	
			eBox = gtk_event_box_new();
			gtk_widget_modify_bg(eBox, GTK_STATE_NORMAL, &settings.colors.sb[type]);
			box = gtk_vbox_new(FALSE, 0);
			label = gtk_label_new(NULL);
	  		gtk_label_set_markup(GTK_LABEL(label), (const gchar *) 
					"<span weight=\"bold\" foreground=\"white\" font_desc=\"12\">TRACE SCREEN OPTIONS</span>");
			gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, 0);
			gtk_container_add(GTK_CONTAINER(eBox), box);
			gtk_box_pack_start(GTK_BOX(controlBox), eBox, FALSE, FALSE, 2);
			
			frame = gtk_frame_new("Display Mode");
			box = gtk_vbox_new(FALSE, 0);
	
			hbox = gtk_hbox_new(FALSE, 0);
			dispMode[TRACEMODE] = radioB1 = gtk_radio_button_new_with_label(NULL, "Trace mode");
			gtk_box_pack_start(GTK_BOX(hbox), radioB1, FALSE, FALSE, 2);
#ifdef PQL_ONLY
			dispMode[GATHERMODE] = radioB2 = gtk_radio_button_new_with_label(
							gtk_radio_button_get_group((GtkRadioButton *) radioB1), "Gather Mode");
			gtk_box_pack_start(GTK_BOX(hbox), radioB2, FALSE, FALSE, 2);
			g_signal_connect(dispMode[TRACEMODE], "toggled", G_CALLBACK(dispModeShow), NULL);
#endif
			gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 2);
			CTraceModeB = hbox = gtk_hbox_new(FALSE, 0);
			label = gtk_label_new("Number of Traces: ");
			spinAdj = (GtkAdjustment *) gtk_adjustment_new(settings.trace.numPlots, 1.0, MAXPLOTS, 
							1.0, 5.0, 0);
			Tnum_files = ctr = gtk_spin_button_new(spinAdj, 1.0, 0);
			gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);
			gtk_box_pack_start(GTK_BOX(hbox), ctr, FALSE, FALSE, 2);
			gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 2);
			gtk_misc_set_alignment(GTK_MISC (label), 1, .5);
			strcpy(toolTip, "Define Number of Traces to DISPLAY per Screen");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), ctr, toolTip, toolTip);

#ifdef PQL_ONLY
			size_group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);

			CGatherModeB = vbox = gtk_vbox_new(FALSE, 0);
			hbox = gtk_hbox_new(FALSE, 0);
			label = gtk_label_new("Start Time Spread: ");
			gtk_size_group_add_widget(size_group, label);
			gtk_misc_set_alignment(GTK_MISC (label), 1, .5);
			spinAdj = (GtkAdjustment *) gtk_adjustment_new((gdouble) 
						settings.general.display.gather.spread, 
						(gdouble) 0.1, (gdouble) 100.0, (gdouble) 0.1, 
						(gdouble) 1.0, (gdouble) 0.0);
			Tspread = ctr = gtk_spin_button_new(spinAdj, .1, 1);
			gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);
			gtk_box_pack_start(GTK_BOX(hbox), ctr, FALSE, FALSE, 2);
			strcpy(toolTip, "Define the Number of Seconds from 1st Start Time to Last Start Time Defining a Gather (Group of Traces)");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), ctr, toolTip, toolTip);
			gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 2);

			hbox = gtk_hbox_new(FALSE, 0);
			label = gtk_label_new("Shot Length: ");
			gtk_size_group_add_widget(size_group, label);
			gtk_misc_set_alignment(GTK_MISC (label), 1, .5);
			spinAdj = (GtkAdjustment *) gtk_adjustment_new((gdouble) 
						settings.general.input.shotLength, 
						(gdouble) 1.0, (gdouble) 10000.0, (gdouble) 1.0, 
						(gdouble) 1.0, (gdouble) 0.0);
			TshotLen = ctr = gtk_spin_button_new(spinAdj, 1, 0);
			gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);
			gtk_box_pack_start(GTK_BOX(hbox), ctr, FALSE, FALSE, 2);
			strcpy(toolTip, "For Input Option -t: Define the Shot Length (in Seconds)");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), ctr, toolTip, toolTip);
			gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 2);

			g_object_unref(size_group);
			gtk_box_pack_start(GTK_BOX(box), vbox, FALSE, FALSE, 2);
#endif

			gtk_container_add(GTK_CONTAINER(frame), box);
			gtk_box_pack_start(GTK_BOX(controlBox), frame, FALSE, FALSE, 2);
	
			frame = gtk_frame_new("Plotting");
			box = gtk_vbox_new(FALSE, 0);
			
			size_group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
#if 0
			hbox = gtk_hbox_new(FALSE, 0);
			label = gtk_label_new("Number of Traces: ");
			spinAdj = (GtkAdjustment *) gtk_adjustment_new(settings.trace.numPlots, 1.0, MAXPLOTS, 
							1.0, 5.0, 0.0);
			Tnum_files = ctr = gtk_spin_button_new(spinAdj, 1.0, 0);
			gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);
			gtk_box_pack_start(GTK_BOX(hbox), ctr, FALSE, FALSE, 2);
			gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 2);
			gtk_size_group_add_widget(size_group, label);
			gtk_misc_set_alignment(GTK_MISC (label), 1, .5);
			strcpy(toolTip, "Define Number of Traces to DISPLAY per Screen");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), ctr, toolTip, toolTip);
#endif			
			hbox = gtk_hbox_new(FALSE, 0);
			label = gtk_label_new("Magnify Interval: ");
			spinAdj = (GtkAdjustment *) gtk_adjustment_new(settings.magnify.length, 0.0, 
							25000.0, 100.0, 500.0, 0.0);
			TmagInt = ctr = gtk_spin_button_new(spinAdj, 100.0, 1);
			gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);
			gtk_box_pack_start(GTK_BOX(hbox), ctr, FALSE, FALSE, 2);
			gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 2);
			gtk_size_group_add_widget(size_group, label);
			gtk_misc_set_alignment(GTK_MISC (label), 1, .5);
			g_object_unref(size_group);
			strcpy(toolTip, "Define Size (in seconds) to MAGNIFY for Single Magnify Boundary, Default = 1/10 of Trace Length");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), ctr, toolTip, toolTip);
			
			size_group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
			gtk_size_group_add_widget(size_group, Tnum_files);
			gtk_size_group_add_widget(size_group, TmagInt);
			g_object_unref(size_group);

			gtk_container_add(GTK_CONTAINER(frame), box);
			gtk_box_pack_start(GTK_BOX(controlBox), frame, FALSE, FALSE, 2);
	
			frame = gtk_frame_new("Trace Label");
			box = gtk_vbox_new(FALSE, 0);
	
			hbox = gtk_hbox_new(FALSE, 0);
			Tlabel[LABEL_FILE] = radioB1 = gtk_radio_button_new_with_label(NULL, "Filename");
			gtk_box_pack_start(GTK_BOX(hbox), radioB1, FALSE, FALSE, 2);
			Tlabel[LABEL_FILE_SHORT] = radioB2 = gtk_radio_button_new_with_label(
							gtk_radio_button_get_group((GtkRadioButton *) radioB1), "Pathless");
			gtk_box_pack_start(GTK_BOX(hbox), radioB2, FALSE, FALSE, 2);
			Tlabel[LABEL_HEADER] = radioB3 = gtk_radio_button_new_with_label(
							gtk_radio_button_get_group((GtkRadioButton *) radioB1), "Header Info");
			gtk_box_pack_start(GTK_BOX(hbox), radioB3, FALSE, FALSE, 2);
			gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 2);
			strcpy(toolTip, "Trace Label is Full PATHNAME");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), radioB1, toolTip, toolTip);
			strcpy(toolTip, "Trace Label is FILENAME");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), radioB2, toolTip, toolTip);
			strcpy(toolTip, "Trace Label is HEADER Information");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), radioB3, toolTip, toolTip);
	
			hbox = gtk_hbox_new(FALSE, 0);
			label = gtk_label_new("Maximum Pixels/Filename: ");
			spinAdj = (GtkAdjustment *) gtk_adjustment_new(0, 0, 250.0, 50.0, 10.0, 0.0);
			TmaxPix = ctr = gtk_spin_button_new(spinAdj, 10.0, 0);
			gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);
			gtk_box_pack_start(GTK_BOX(hbox), ctr, FALSE, FALSE, 2);
			gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 2);
			strcpy(toolTip, "Display TRACE Label Using Maximum PIXELS");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), ctr, toolTip, toolTip);
			
			gtk_container_add(GTK_CONTAINER(frame), box);
			gtk_box_pack_start(GTK_BOX(controlBox), frame, FALSE, FALSE, 2);
	
			frame = gtk_frame_new("MARKing");
			box = gtk_vbox_new(FALSE, 0);

			hbox = gtk_hbox_new(FALSE, 0);
			label = gtk_label_new("Mark Base Filename: ");
			TmarkFN = entry = gtk_entry_new();
			gtk_entry_set_max_length(GTK_ENTRY(entry), 40);
			button = gtk_button_new_with_label("...");
			gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);
			gtk_box_pack_start(GTK_BOX(hbox), entry, FALSE, FALSE, 2);
			gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 2);
			gtk_container_add(GTK_CONTAINER(box), hbox);
			gtk_misc_set_alignment(GTK_MISC (label), 1, .5);
			strcpy(toolTip, "Define MARK Base Filename, see HELP for usage details");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), entry, toolTip, toolTip);
			g_signal_connect(button, "clicked", G_CALLBACK(getFilename), GINT_TO_POINTER(MARKFTYPE));

			gtk_container_add(GTK_CONTAINER(frame), box);
			gtk_box_pack_start(GTK_BOX(controlBox), frame, FALSE, FALSE, 2);
	
			size_group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
			frame = gtk_frame_new("Indicator Flags");
			box = gtk_vbox_new(FALSE, 0);
			
			label = gtk_label_new("Gap: ");
			hbox = gtk_hbox_new(FALSE, 0);
			gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);
			radioB1 = gtk_radio_button_new_with_label(NULL, "On");
			gtk_box_pack_start(GTK_BOX(hbox), radioB1, FALSE, FALSE, 2);
			Tgap = radioB2 = gtk_radio_button_new_with_label(
									gtk_radio_button_get_group((GtkRadioButton *) radioB1), "Off");
			gtk_box_pack_start(GTK_BOX(hbox), radioB2, FALSE, FALSE, 2);
			gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 2);
			gtk_size_group_add_widget(size_group, label);
			gtk_misc_set_alignment(GTK_MISC (label), 1, .5);
			strcpy(toolTip, "Display GAP Flags");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), radioB1, toolTip, toolTip);
			strcpy(toolTip, "Do NOT Display GAP Flags");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), radioB2, toolTip, toolTip);
			
			label = gtk_label_new("Overlap: ");
			hbox = gtk_hbox_new(FALSE, 0);
			gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);
			radioB1 = gtk_radio_button_new_with_label(NULL, "On");
			gtk_box_pack_start(GTK_BOX(hbox), radioB1, FALSE, FALSE, 2);
			Tolap = radioB2 = gtk_radio_button_new_with_label(
									gtk_radio_button_get_group((GtkRadioButton *) radioB1), "Off");
			gtk_box_pack_start(GTK_BOX(hbox), radioB2, FALSE, FALSE, 2);
			gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 2);
			gtk_size_group_add_widget(size_group, label);
			gtk_misc_set_alignment(GTK_MISC (label), 1, .5);
			strcpy(toolTip, "Display OVERLAP Flags");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), radioB1, toolTip, toolTip);
			strcpy(toolTip, "Do NOT Display OVERLAP Flags");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), radioB2, toolTip, toolTip);
			
			g_object_unref(size_group);

			gtk_container_add(GTK_CONTAINER(frame), box);
			gtk_box_pack_start(GTK_BOX(controlBox), frame, FALSE, FALSE, 2);

			frame = gtk_frame_new("Colors");
			vbox = gtk_vbox_new(FALSE, 0);
			hbox = gtk_hbox_new(FALSE, 0);
	
			size_group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);

			frame1 = gtk_frame_new("Foreground");
			colorsFG[TRACESCR] = gtk_color_button_new();
			gtk_color_button_set_title(GTK_COLOR_BUTTON (colorsFG[TRACESCR]), "Foreground");
			g_signal_connect(colorsFG[TRACESCR], "color-set", G_CALLBACK(setFG), GINT_TO_POINTER(TRACESCR));
			gtk_widget_set_size_request(colorsFG[TRACESCR], 60, 40);
			gtk_container_add(GTK_CONTAINER(frame1), colorsFG[TRACESCR]);
			gtk_box_pack_start(GTK_BOX(hbox), frame1, FALSE, FALSE, 2);
			gtk_size_group_add_widget(size_group, frame1);
			strcpy(toolTip, "Set the FOREGROUND Colour");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), colorsFG[TRACESCR], toolTip, toolTip);
	
			frame1 = gtk_frame_new("Background");
			colorsBG[TRACESCR] = gtk_color_button_new();
			gtk_color_button_set_title(GTK_COLOR_BUTTON (colorsBG[TRACESCR]), "Background");
			g_signal_connect(colorsBG[TRACESCR], "color-set", G_CALLBACK(setBG), GINT_TO_POINTER(TRACESCR));
			gtk_widget_set_size_request(colorsBG[TRACESCR], 60, 40);
			gtk_container_add(GTK_CONTAINER(frame1), colorsBG[TRACESCR]);
			gtk_box_pack_start(GTK_BOX(hbox), frame1, FALSE, FALSE, 2);
			gtk_size_group_add_widget(size_group, frame1);
			strcpy(toolTip, "Set the BACKGROUND Colour");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), colorsBG[TRACESCR], toolTip, toolTip);
	
			frame1 = gtk_frame_new("Sidebar");
			colorsSB[TRACESCR] = gtk_color_button_new();
			gtk_color_button_set_title(GTK_COLOR_BUTTON (colorsSB[TRACESCR]), "Sidebar");
			g_signal_connect(colorsSB[TRACESCR], "color-set", G_CALLBACK(setSB), GINT_TO_POINTER(TRACESCR));
			gtk_widget_set_size_request(colorsSB[TRACESCR], 60, 40);
			gtk_container_add(GTK_CONTAINER(frame1), colorsSB[TRACESCR]);
			gtk_box_pack_end(GTK_BOX(hbox), frame1, FALSE, FALSE, 2);
			gtk_size_group_add_widget(size_group, frame1);
			gtk_container_add(GTK_CONTAINER(vbox), hbox);
			strcpy(toolTip, "Set the SIDEBAR Colour");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), colorsSB[TRACESCR], toolTip, toolTip);

			hbox = gtk_hbox_new(FALSE, 0);
	
			frame1 = gtk_frame_new("Gap");
			colorsGP[TRACESCR][GC1] = gtk_color_button_new();
			gtk_color_button_set_title(GTK_COLOR_BUTTON (colorsGP[TRACESCR][GC1]), "Gap");
			g_signal_connect(colorsGP[TRACESCR][GC1], "color-set", G_CALLBACK(setGP), NULL);
			gtk_widget_set_size_request(colorsGP[TRACESCR][GC1], 60, 40);
			gtk_container_add(GTK_CONTAINER(frame1), colorsGP[TRACESCR][GC1]);
			gtk_box_pack_start(GTK_BOX(hbox), frame1, FALSE, FALSE, 2);
			gtk_size_group_add_widget(size_group, frame1);
			strcpy(toolTip, "Set the GAP Colour");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), colorsGP[TRACESCR][GC1], toolTip, toolTip);

			frame1 = gtk_frame_new("Overlap");
			colorsOP[TRACESCR][OL1] = gtk_color_button_new();
			gtk_color_button_set_title(GTK_COLOR_BUTTON (colorsOP[TRACESCR][OL1]), "Overlap");
			g_signal_connect(colorsOP[TRACESCR][OL1], "color-set", G_CALLBACK(setOP), NULL);
			gtk_widget_set_size_request(colorsGP[TRACESCR][OL1], 60, 40);
			gtk_container_add(GTK_CONTAINER(frame1), colorsOP[TRACESCR][OL1]);
			gtk_box_pack_start(GTK_BOX(hbox), frame1, FALSE, FALSE, 2);
			gtk_size_group_add_widget(size_group, frame1);
			strcpy(toolTip, "Set the OVERLAP Colour");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), colorsOP[TRACESCR][OL1], toolTip, toolTip);

			g_object_unref(size_group);

			gtk_container_add(GTK_CONTAINER(vbox), hbox);
			gtk_container_add(GTK_CONTAINER(frame), vbox);
			gtk_box_pack_start(GTK_BOX(controlBox), frame, FALSE, FALSE, 2);

			actionB = gtk_hbutton_box_new();
			gtk_button_box_set_layout(GTK_BUTTON_BOX(actionB), GTK_BUTTONBOX_SPREAD);
			gtk_box_set_spacing(GTK_BOX(actionB), 2);
	
			button = gtk_button_new_with_label("Continue");
			gtk_container_add(GTK_CONTAINER(actionB), button);
			g_signal_connect(button, "clicked", G_CALLBACK(get_settings), NULL);
			g_signal_connect(button, "clicked", G_CALLBACK(raise_page), &states.currentPage);  // &traceScr);
			strcpy(toolTip, "APPLY New Settings and RETURN to TRACE Screen");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), button, toolTip, toolTip);
			button = gtk_button_new_with_label("Restart");
			g_signal_connect(button, "clicked", G_CALLBACK(get_settings), NULL);
			g_signal_connect(button, "clicked", G_CALLBACK(restartTrace), GINT_TO_POINTER(0));
			g_signal_connect(button, "clicked", G_CALLBACK(raise_page), &OV[TRACESCR]);
			gtk_container_add(GTK_CONTAINER(actionB), button);
			gtk_box_pack_start(GTK_BOX(controlBox), actionB, FALSE, FALSE, 2);
			strcpy(toolTip, "APPLY New Settings and RETURN to TRACE Screen, Displaying from FIRST Trace");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), button, toolTip, toolTip);
	
			eBox = gtk_event_box_new();
			gtk_widget_modify_bg(eBox, GTK_STATE_NORMAL, &settings.colors.sb[type]);
			gtk_container_add(GTK_CONTAINER(controlBox), eBox);
	
		break;

		case MAGCTRL:
			controlFrame = gtk_frame_new(NULL);
			gtk_container_add(GTK_CONTAINER(control_box), controlFrame);
			controlBox = gtk_vbox_new(FALSE, 0);
			gtk_container_set_border_width(GTK_CONTAINER(controlBox), 5);
			gtk_container_add(GTK_CONTAINER(controlFrame), controlBox);
	
			eBox = gtk_event_box_new();
			gtk_widget_modify_bg(eBox, GTK_STATE_NORMAL, &settings.colors.sb[type]);
			box = gtk_vbox_new(FALSE, 0);
			label = gtk_label_new(NULL);
	  		gtk_label_set_markup(GTK_LABEL(label), 
					(const gchar *) "<span weight=\"bold\" foreground=\"white\" font_desc=\"12\">MAGNIFY SCREEN OPTIONS</span>");
			gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, 0);
			gtk_container_add(GTK_CONTAINER(eBox), box);
			gtk_box_pack_start(GTK_BOX(controlBox), eBox, FALSE, FALSE, 2);
			
			frame = gtk_frame_new("Zoom/Scrolling Factors");
			box = gtk_vbox_new(FALSE, 0);

			hbox = gtk_hbox_new(FALSE, 0);
			label = gtk_label_new("Zoom: ");
			spinAdj = (GtkAdjustment *) gtk_adjustment_new(settings.magnify.zoomFactor, 10, 30, 1, 1, 0);
			MZoomSpeed = gtk_spin_button_new(spinAdj, 5, 0);
			gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);
			gtk_box_pack_start(GTK_BOX(hbox), MZoomSpeed, FALSE, FALSE, 2);
			strcpy(toolTip, "Define SPEED of Zooming, HIGHER = FASTER (better for slower CPU's)");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), MZoomSpeed, toolTip, toolTip);
			
			label = gtk_label_new("	Scrolling: ");
			spinAdj = (GtkAdjustment *) gtk_adjustment_new(settings.magnify.scrollFactor, 5, 95, 1, 10, 0);
			MScrollFact = gtk_spin_button_new(spinAdj, 5, 0);
			gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);
			gtk_box_pack_start(GTK_BOX(hbox), MScrollFact, FALSE, FALSE, 2);
			strcpy(toolTip, "Define Scroll JUMP SIZE, as % of Selection");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), MScrollFact, toolTip, toolTip);

			gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 2);
			gtk_container_add(GTK_CONTAINER(frame), box);
			gtk_box_pack_start(GTK_BOX(controlBox), frame, FALSE, FALSE, 2);
			
			
			frame = gtk_frame_new("Y-Axis Units");
			box = gtk_vbox_new(FALSE, 0);

			hbox = gtk_hbox_new(FALSE, 0);
			yaxisConv = gtk_check_button_new_with_label("Convert Y-Axis Units using Scale and Gain");
			gtk_box_pack_start(GTK_BOX(hbox), yaxisConv, FALSE, FALSE, 2);
			
			gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 2);
			gtk_container_add(GTK_CONTAINER(frame), box);
			gtk_box_pack_start(GTK_BOX(controlBox), frame, FALSE, FALSE, 2);


			size_group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
			frame = gtk_frame_new("Picking");
			box = gtk_vbox_new(FALSE, 0);

			hbox = gtk_hbox_new(FALSE, 0);
			label = gtk_label_new("Pick Filename: ");
			MpickFN = entry = gtk_entry_new();
			gtk_entry_set_max_length(GTK_ENTRY(entry), 40);
			button = gtk_button_new_with_label("...");
			gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);
			gtk_box_pack_start(GTK_BOX(hbox), entry, FALSE, FALSE, 2);
			gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 2);
			gtk_container_add(GTK_CONTAINER(box), hbox);
			gtk_size_group_add_widget(size_group, label);
			gtk_misc_set_alignment(GTK_MISC (label), 1, .5);
			strcpy(toolTip, "Define Filename for Writing PICK Information");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), entry, toolTip, toolTip);
			g_signal_connect(button, "clicked", G_CALLBACK(getFilename), GINT_TO_POINTER(PICKFTYPE));

			frame1 = gtk_frame_new("Write Options");
			box1 = gtk_vbox_new(FALSE, 0);

			label = gtk_label_new("KEYWORDS: ");
			hbox = gtk_hbox_new(FALSE, 0);
			gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);
			radioB1 = gtk_radio_button_new_with_label(NULL, "On");
			gtk_box_pack_start(GTK_BOX(hbox), radioB1, FALSE, FALSE, 2);
			MpickKEYind = radioB2 = gtk_radio_button_new_with_label(
									gtk_radio_button_get_group((GtkRadioButton *) radioB1), "Off");
			gtk_box_pack_start(GTK_BOX(hbox), radioB2, FALSE, FALSE, 2);
			gtk_box_pack_start(GTK_BOX(box1), hbox, FALSE, FALSE, 2);
			gtk_size_group_add_widget(size_group, label);
			gtk_misc_set_alignment(GTK_MISC (label), 1, .5);
			strcpy(toolTip, "Write KEYWORDS to Pick File");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), radioB1, toolTip, toolTip);
			strcpy(toolTip, "NO KEYWORDS");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), radioB2, toolTip, toolTip);
			
			label = gtk_label_new("Commentary: ");
			hbox = gtk_hbox_new(FALSE, 0);
			gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);
			radioB1 = gtk_radio_button_new_with_label(NULL, "On");
			gtk_box_pack_start(GTK_BOX(hbox), radioB1, FALSE, FALSE, 2);
			MpickCOMind = radioB2 = gtk_radio_button_new_with_label(
									gtk_radio_button_get_group((GtkRadioButton *) radioB1), "Off");
			gtk_box_pack_start(GTK_BOX(hbox), radioB2, FALSE, FALSE, 2);
			gtk_box_pack_start(GTK_BOX(box1), hbox, FALSE, FALSE, 2);
			gtk_size_group_add_widget(size_group, label);
			gtk_misc_set_alignment(GTK_MISC (label), 1, .5);
			strcpy(toolTip, "Write COMMENT to Pick File");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), radioB1, toolTip, toolTip);
			strcpy(toolTip, "NO COMMENT");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), radioB2, toolTip, toolTip);
			
			gtk_container_add(GTK_CONTAINER(frame1), box1);
			gtk_container_add(GTK_CONTAINER(box), frame1);
			g_object_unref(size_group);

			frame1 = gtk_frame_new("Tag Names");
			box1 = gtk_vbox_new(FALSE, 0);

			size_group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
			for(i=0;i<MAXPICKS/2;i++)
			{
				box2 = gtk_hbox_new(FALSE, 0);

				sprintf(pickTag, "Pick #%d", i*2+1);
				label = gtk_label_new(pickTag);
				gtk_size_group_add_widget(size_group, label);
				MpickTag[i*2] = entry = gtk_entry_new();
				gtk_entry_set_width_chars(GTK_ENTRY(entry), 10);
				gtk_entry_set_max_length(GTK_ENTRY(entry), 10);
				gtk_box_pack_start(GTK_BOX(box2), label, FALSE, FALSE, 2);
				gtk_box_pack_start(GTK_BOX(box2), entry, FALSE, FALSE, 2);

				sprintf(pickTag, "Pick #%d", i*2+2);
				label = gtk_label_new(pickTag);
				gtk_size_group_add_widget(size_group, label);
				MpickTag[i*2+1] = entry = gtk_entry_new();
				gtk_entry_set_width_chars(GTK_ENTRY(entry), 10);
				gtk_entry_set_max_length(GTK_ENTRY(entry), 10);
				gtk_box_pack_start(GTK_BOX(box2), label, FALSE, FALSE, 2);
				gtk_box_pack_start(GTK_BOX(box2), entry, FALSE, FALSE, 2);

				gtk_container_add(GTK_CONTAINER(box1), box2);
			}
			g_object_unref(size_group);
			strcpy(toolTip, "Define PICK Tag Names");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), MpickTag[0], toolTip, toolTip);

			gtk_container_add(GTK_CONTAINER(frame1), box1);
			gtk_container_add(GTK_CONTAINER(box), frame1);

			gtk_container_add(GTK_CONTAINER(frame), box);
			gtk_box_pack_start(GTK_BOX(controlBox), frame, FALSE, FALSE, 2);

			size_group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);

			frame = gtk_frame_new("Indicator Flags");
			box = gtk_vbox_new(FALSE, 0);
			
			label = gtk_label_new("Gap: ");
			hbox = gtk_hbox_new(FALSE, 0);
			gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);
			radioB1 = gtk_radio_button_new_with_label(NULL, "On");
			gtk_box_pack_start(GTK_BOX(hbox), radioB1, FALSE, FALSE, 2);
			Mgap = radioB2 = gtk_radio_button_new_with_label(
									gtk_radio_button_get_group((GtkRadioButton *) radioB1), "Off");
			gtk_box_pack_start(GTK_BOX(hbox), radioB2, FALSE, FALSE, 2);
			gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 2);
			gtk_size_group_add_widget(size_group, label);
			gtk_misc_set_alignment(GTK_MISC (label), 1, .5);
			strcpy(toolTip, "Display GAP Flags");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), radioB1, toolTip, toolTip);
			strcpy(toolTip, "Do NOT Display GAP Flags");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), radioB2, toolTip, toolTip);
			
			label = gtk_label_new("Overlap: ");
			hbox = gtk_hbox_new(FALSE, 0);
			gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);
			radioB1 = gtk_radio_button_new_with_label(NULL, "On");
			gtk_box_pack_start(GTK_BOX(hbox), radioB1, FALSE, FALSE, 2);
			Molap = radioB2 = gtk_radio_button_new_with_label(
									gtk_radio_button_get_group((GtkRadioButton *) radioB1), "Off");
			gtk_box_pack_start(GTK_BOX(hbox), radioB2, FALSE, FALSE, 2);
			gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 2);
			gtk_size_group_add_widget(size_group, label);
			gtk_misc_set_alignment(GTK_MISC (label), 1, .5);
			g_object_unref(size_group);
			strcpy(toolTip, "Display OVERLAP Flags");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), radioB1, toolTip, toolTip);
			strcpy(toolTip, "Do NOT Display OVERLAP Flags");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), radioB2, toolTip, toolTip);
			
			gtk_container_add(GTK_CONTAINER(frame), box);
			gtk_box_pack_start(GTK_BOX(controlBox), frame, FALSE, FALSE, 2);

			size_group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
#if 0
			frame = gtk_frame_new("SideBar Display");
			box = gtk_vbox_new(FALSE, 0);
			
			label = gtk_label_new("Zoom Box: ");
			hbox = gtk_hbox_new(FALSE, 0);
			gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);
			radioB1 = gtk_radio_button_new_with_label(NULL, "Display");
			gtk_box_pack_start(GTK_BOX(hbox), radioB1, FALSE, FALSE, 2);
			MZDisp = radioB2 = gtk_radio_button_new_with_label(
									gtk_radio_button_get_group((GtkRadioButton *) radioB1), "Hide");
			gtk_box_pack_start(GTK_BOX(hbox), radioB2, FALSE, FALSE, 2);
			gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 2);
			gtk_size_group_add_widget(size_group, label);
			gtk_misc_set_alignment(GTK_MISC (label), 1, .5);
			strcpy(toolTip, "DISPLAY ZOOM Anchor Box on SideBar");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), radioB1, toolTip, toolTip);
			strcpy(toolTip, "HIDE ZOOM Anchor Box on SideBar (Makes SideBar Shorter)");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), radioB2, toolTip, toolTip);
			
			label = gtk_label_new("Pick Box: ");
			hbox = gtk_hbox_new(FALSE, 0);
			gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);
			radioB1 = gtk_radio_button_new_with_label(NULL, "Display");
			gtk_box_pack_start(GTK_BOX(hbox), radioB1, FALSE, FALSE, 2);
			MPDisp = radioB2 = gtk_radio_button_new_with_label(
									gtk_radio_button_get_group((GtkRadioButton *) radioB1), "Hide");
			gtk_box_pack_start(GTK_BOX(hbox), radioB2, FALSE, FALSE, 2);
			gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 2);
			gtk_size_group_add_widget(size_group, label);
			gtk_misc_set_alignment(GTK_MISC (label), 1, .5);
			g_object_unref(size_group);
			strcpy(toolTip, "DISPLAY Picking Box on SideBar");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), radioB1, toolTip, toolTip);
			strcpy(toolTip, "HIDE Picking Box on SideBar (Makes SideBar Shorter)");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), radioB2, toolTip, toolTip);
			
			gtk_container_add(GTK_CONTAINER(frame), box);
			gtk_box_pack_start(GTK_BOX(controlBox), frame, FALSE, FALSE, 2);
#endif
			frame = gtk_frame_new("Colors");
			vbox = gtk_vbox_new(FALSE, 0);
			hbox = gtk_hbox_new(FALSE, 0);
	
			size_group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);

			frame1 = gtk_frame_new("Foreground");
			colorsFG[MAGNIFYSCR] = gtk_color_button_new();
			gtk_color_button_set_title(GTK_COLOR_BUTTON (colorsFG[MAGNIFYSCR]), "Foreground");
			g_signal_connect(colorsFG[MAGNIFYSCR], "color-set", G_CALLBACK(setFG), GINT_TO_POINTER(MAGNIFYSCR));
			gtk_widget_set_size_request(colorsFG[MAGNIFYSCR],  60, 40);
			gtk_container_add(GTK_CONTAINER(frame1), colorsFG[MAGNIFYSCR]);
			gtk_box_pack_start(GTK_BOX(hbox), frame1, FALSE, FALSE, 2);
			gtk_size_group_add_widget(size_group, frame1);
			strcpy(toolTip, "Set the FOREGROUND Colour");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), colorsFG[MAGNIFYSCR], toolTip, toolTip);
	
			frame1 = gtk_frame_new("Background");
			colorsBG[MAGNIFYSCR] = gtk_color_button_new();
			gtk_color_button_set_title(GTK_COLOR_BUTTON (colorsBG[MAGNIFYSCR]), "Background");
			g_signal_connect(colorsBG[MAGNIFYSCR], "color-set", G_CALLBACK(setBG), GINT_TO_POINTER(MAGNIFYSCR));
			gtk_widget_set_size_request(colorsBG[MAGNIFYSCR],  60, 40);
			gtk_container_add(GTK_CONTAINER(frame1), colorsBG[MAGNIFYSCR]);
			gtk_box_pack_start(GTK_BOX(hbox), frame1, FALSE, FALSE, 2);
			gtk_size_group_add_widget(size_group, frame1);
			strcpy(toolTip, "Set the BACKGROUND Colour");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), colorsBG[MAGNIFYSCR], toolTip, toolTip);
	
			frame1 = gtk_frame_new("Sidebar");
			colorsSB[MAGNIFYSCR] = gtk_color_button_new();
			gtk_color_button_set_title(GTK_COLOR_BUTTON (colorsSB[MAGNIFYSCR]), "Sidebar");
			g_signal_connect(colorsSB[MAGNIFYSCR], "color-set", G_CALLBACK(setSB), GINT_TO_POINTER(MAGNIFYSCR));
			gtk_widget_set_size_request(colorsSB[MAGNIFYSCR],  60, 40);
			gtk_container_add(GTK_CONTAINER(frame1), colorsSB[MAGNIFYSCR]);
			gtk_box_pack_end(GTK_BOX(hbox), frame1, FALSE, FALSE, 2);
			gtk_size_group_add_widget(size_group, frame1);
			strcpy(toolTip, "Set the SIDEBAR Colour");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), colorsSB[MAGNIFYSCR], toolTip, toolTip);
	
			gtk_container_add(GTK_CONTAINER(vbox), hbox);

			hbox = gtk_hbox_new(FALSE, 0);
	
			frame1 = gtk_frame_new("Gap");
			colorsGP[MAGNIFYSCR][GC1] = gtk_color_button_new();
			gtk_color_button_set_title(GTK_COLOR_BUTTON (colorsGP[MAGNIFYSCR][GC1]), "Gap");
			g_signal_connect(colorsGP[MAGNIFYSCR][GC1], "color-set", G_CALLBACK(setGP), NULL);
			gtk_widget_set_size_request(colorsGP[MAGNIFYSCR][GC1], 60, 40);
			gtk_container_add(GTK_CONTAINER(frame1), colorsGP[MAGNIFYSCR][GC1]);
			gtk_box_pack_start(GTK_BOX(hbox), frame1, FALSE, FALSE, 2);
			gtk_size_group_add_widget(size_group, frame1);
			strcpy(toolTip, "Set the GAP Colour");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), colorsGP[MAGNIFYSCR][GC1], toolTip, toolTip);
	
			frame1 = gtk_frame_new("Overlap");
			colorsOP[MAGNIFYSCR][OL1] = gtk_color_button_new();
			gtk_color_button_set_title(GTK_COLOR_BUTTON (colorsOP[MAGNIFYSCR][OL1]), "Overlap");
			g_signal_connect(colorsOP[MAGNIFYSCR][OL1], "color-set", G_CALLBACK(setOP), NULL);
			gtk_widget_set_size_request(colorsOP[MAGNIFYSCR][OL1], 60, 40);
			gtk_container_add(GTK_CONTAINER(frame1), colorsOP[MAGNIFYSCR][OL1]);
			gtk_box_pack_start(GTK_BOX(hbox), frame1, FALSE, FALSE, 2);
			gtk_size_group_add_widget(size_group, frame1);
			strcpy(toolTip, "Set the OVERLAP Colour");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), colorsOP[MAGNIFYSCR][OL1], toolTip, toolTip);
			g_object_unref(size_group);

			gtk_container_add(GTK_CONTAINER(vbox), hbox);
			gtk_container_add(GTK_CONTAINER(frame), vbox);

			gtk_box_pack_start(GTK_BOX(controlBox), frame, FALSE, FALSE, 2);
	
			actionB = gtk_hbutton_box_new();
			gtk_button_box_set_layout(GTK_BUTTON_BOX(actionB), GTK_BUTTONBOX_SPREAD);
			gtk_box_set_spacing(GTK_BOX(actionB), 2);
	
			button = gtk_button_new_with_label("Continue");
			gtk_container_add(GTK_CONTAINER(actionB), button);
			g_signal_connect(button, "clicked", G_CALLBACK(get_settings), NULL);
			g_signal_connect(button, "clicked", G_CALLBACK(raise_page), &states.currentPage);  //&magScr);
			gtk_box_pack_start(GTK_BOX(controlBox), actionB, FALSE, FALSE, 2);
			strcpy(toolTip, "APPLY New Settings and RETURN to MAGNIFY Screen");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), button, toolTip, toolTip);
	
			eBox = gtk_event_box_new();
			gtk_widget_modify_bg(eBox, GTK_STATE_NORMAL, &settings.colors.sb[type]);
			gtk_container_add(GTK_CONTAINER(controlBox), eBox);
			
		break;

		case SPECCTRL:
			controlFrame = gtk_frame_new(NULL);
			gtk_container_add(GTK_CONTAINER(control_box), controlFrame);
			controlBox = gtk_vbox_new(FALSE, 0);
			gtk_container_set_border_width(GTK_CONTAINER(controlBox), 5);
			gtk_container_add(GTK_CONTAINER(controlFrame), controlBox);
	
			eBox = gtk_event_box_new();
			gtk_widget_modify_bg(eBox, GTK_STATE_NORMAL, &settings.colors.sb[type]);
			box = gtk_vbox_new(FALSE, 0);
			label = gtk_label_new(NULL);
	  		gtk_label_set_markup(GTK_LABEL(label), 
					(const gchar *) "<span weight=\"bold\" foreground=\"white\" font_desc=\"12\">SPECTRA SCREEN OPTIONS</span>");
			gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, 0);
			gtk_container_add(GTK_CONTAINER(eBox), box);
			gtk_box_pack_start(GTK_BOX(controlBox), eBox, FALSE, FALSE, 2);
			
			frame = gtk_frame_new("Amplitude");
			box = gtk_vbox_new(FALSE, 0);
			
			size_group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
			label = gtk_label_new("Scale: ");
			hbox = gtk_hbox_new(FALSE, 0);
			gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);
			radioB1 = gtk_radio_button_new_with_label(NULL, "Logarithmic");
			gtk_box_pack_start(GTK_BOX(hbox), radioB1, FALSE, FALSE, 2);
			Samp = radioB2 = gtk_radio_button_new_with_label(
									gtk_radio_button_get_group((GtkRadioButton *) radioB1), "Linear");
			gtk_box_pack_start(GTK_BOX(hbox), radioB2, FALSE, FALSE, 2);
			gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 2);
			gtk_size_group_add_widget(size_group, label);
			gtk_misc_set_alignment(GTK_MISC (label), 1, .5);
			strcpy(toolTip, "Define AMPLITUDE Scale as LOGARITHMIC");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), radioB1, toolTip, toolTip);
			strcpy(toolTip, "Define AMPLITUDE Scale as LINEAR");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), radioB2, toolTip, toolTip);
			
			hbox = gtk_hbox_new(FALSE, 0);
			label = gtk_label_new("Maximum: ");
			SyMax = entry = gtk_entry_new();
			gtk_entry_set_max_length(GTK_ENTRY(entry), 25);
			gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);
			gtk_box_pack_start(GTK_BOX(hbox), entry, FALSE, FALSE, 2);
			gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 2);
			gtk_size_group_add_widget(size_group, label);
			gtk_misc_set_alignment(GTK_MISC (label), 1, .5);
			strcpy(toolTip, "Define MAXIMUM AMPLITUDE to Display");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), entry, toolTip, toolTip);
	
			hbox = gtk_hbox_new(FALSE, 0);
			label = gtk_label_new("Decades: ");
			spinAdj = (GtkAdjustment *) gtk_adjustment_new(settings.spectra.display.decades, 0.1, 8.0, 1.0, 1.0, 0.0);
			Sdecades = ctr = gtk_spin_button_new(spinAdj, 1.0, 1);
			gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);
			gtk_box_pack_start(GTK_BOX(hbox), ctr, FALSE, FALSE, 2);
			gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 2);
			gtk_size_group_add_widget(size_group, label);
			gtk_misc_set_alignment(GTK_MISC (label), 1, .5);
			g_object_unref(size_group);
			strcpy(toolTip, "Define Number of AMPLITUDE DECADES to Display");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), ctr, toolTip, toolTip);
	
			gtk_container_add(GTK_CONTAINER(frame), box);
			gtk_box_pack_start(GTK_BOX(controlBox), frame, FALSE, FALSE, 2);
			
			frame = gtk_frame_new("Frequency");
			box = gtk_vbox_new(FALSE, 0);
			
			size_group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
			hbox = gtk_hbox_new(FALSE, 0);
			label = gtk_label_new("Scale: ");
			gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);
			radioB1 = gtk_radio_button_new_with_label(NULL, "Logarithmic");
			gtk_box_pack_start(GTK_BOX(hbox), radioB1, FALSE, FALSE, 2);
			Sfreq = radioB2 = gtk_radio_button_new_with_label(
									gtk_radio_button_get_group((GtkRadioButton *) radioB1), "Linear");
			gtk_box_pack_start(GTK_BOX(hbox), radioB2, FALSE, FALSE, 2);
			gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 2);
			gtk_size_group_add_widget(size_group, label);
			gtk_misc_set_alignment(GTK_MISC (label), 1, .5);
			strcpy(toolTip, "Define FREQUENCY Scale as LOGARITHMIC");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), radioB1, toolTip, toolTip);
			strcpy(toolTip, "Define FREQUENCY Scale as LINEAR");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), radioB2, toolTip, toolTip);
			
			hbox = gtk_hbox_new(FALSE, 0);
			label = gtk_label_new("Maximum (Hz): ");
			SxMax = entry = gtk_entry_new();
			gtk_entry_set_max_length(GTK_ENTRY(entry), 25);
			gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);
			gtk_box_pack_start(GTK_BOX(hbox), entry, FALSE, FALSE, 2);
			gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 2);
			gtk_size_group_add_widget(size_group, label);
			gtk_misc_set_alignment(GTK_MISC (label), 1, .5);
			strcpy(toolTip, "Define MAXIMUM FREQUENCY to Display");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), entry, toolTip, toolTip);
	
			hbox = gtk_hbox_new(FALSE, 0);
			label = gtk_label_new("Minimum (Hz): ");
			SxMin = entry = gtk_entry_new();
			gtk_entry_set_max_length(GTK_ENTRY(entry), 25);
			gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);
			gtk_box_pack_start(GTK_BOX(hbox), entry, FALSE, FALSE, 2);
			gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 2);
			gtk_size_group_add_widget(size_group, label);
			gtk_misc_set_alignment(GTK_MISC (label), 1, .5);
			g_object_unref(size_group);
			strcpy(toolTip, "Define MINIMUM FREQUENCY to Display");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), entry, toolTip, toolTip);
	
			gtk_container_add(GTK_CONTAINER(frame), box);
			gtk_box_pack_start(GTK_BOX(controlBox), frame, FALSE, FALSE, 2);
			
			frame = gtk_frame_new("Colors");
			hbox = gtk_hbox_new(FALSE, 0);
	
			frame1 = gtk_frame_new("Foreground");
			colorsFG[SPECTRASCR] = gtk_color_button_new();
			gtk_color_button_set_title(GTK_COLOR_BUTTON (colorsFG[SPECTRASCR]), "Foreground");
			g_signal_connect(colorsFG[SPECTRASCR], "color-set", G_CALLBACK(setFG), GINT_TO_POINTER(SPECTRASCR));
			gtk_widget_set_size_request(colorsFG[SPECTRASCR], 60, 40);
			gtk_container_add(GTK_CONTAINER(frame1), colorsFG[SPECTRASCR]);
			gtk_box_pack_start(GTK_BOX(hbox), frame1, FALSE, FALSE, 2);
			strcpy(toolTip, "Set the FOREGROUND Colour");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), colorsFG[SPECTRASCR], toolTip, toolTip);
	
			frame1 = gtk_frame_new("Background");
			colorsBG[SPECTRASCR] = gtk_color_button_new();
			gtk_color_button_set_title(GTK_COLOR_BUTTON (colorsBG[SPECTRASCR]), "Background");
			g_signal_connect(colorsBG[SPECTRASCR], "color-set", G_CALLBACK(setBG), GINT_TO_POINTER(SPECTRASCR));
			gtk_widget_set_size_request(colorsBG[SPECTRASCR], 60, 40);
			gtk_container_add(GTK_CONTAINER(frame1), colorsBG[SPECTRASCR]);
			gtk_box_pack_start(GTK_BOX(hbox), frame1, FALSE, FALSE, 2);
			strcpy(toolTip, "Set the BACKGROUND Colour");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), colorsBG[SPECTRASCR], toolTip, toolTip);
	
			frame1 = gtk_frame_new("Sidebar");
			colorsSB[SPECTRASCR] = gtk_color_button_new();
			gtk_color_button_set_title(GTK_COLOR_BUTTON (colorsSB[SPECTRASCR]), "Sidebar");
			g_signal_connect(colorsSB[SPECTRASCR], "color-set", G_CALLBACK(setSB), GINT_TO_POINTER(SPECTRASCR));
			gtk_widget_set_size_request(colorsSB[SPECTRASCR], 60, 40);
			gtk_container_add(GTK_CONTAINER(frame1), colorsSB[SPECTRASCR]);
			gtk_box_pack_end(GTK_BOX(hbox), frame1, FALSE, FALSE, 2);
			strcpy(toolTip, "Set the SIDEBAR Colour");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), colorsSB[SPECTRASCR], toolTip, toolTip);
	
			gtk_container_add(GTK_CONTAINER(frame), hbox);
			gtk_box_pack_start(GTK_BOX(controlBox), frame, FALSE, FALSE, 2);
	
			actionB = gtk_hbutton_box_new();
			gtk_button_box_set_layout(GTK_BUTTON_BOX(actionB), GTK_BUTTONBOX_SPREAD);
			gtk_box_set_spacing(GTK_BOX(actionB), 2);
	
			button = gtk_button_new_with_label("Continue");
			g_signal_connect(button, "clicked", G_CALLBACK(get_settings), NULL);
			g_signal_connect(button, "clicked", G_CALLBACK(raise_page), &states.currentPage);  
			strcpy(toolTip, "APPLY New Settings and RETURN to SPECTRA Screen");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), button, toolTip, toolTip);
			gtk_container_add(GTK_CONTAINER(actionB), button);
			gtk_box_pack_start(GTK_BOX(controlBox), actionB, FALSE, FALSE, 2);
	
			eBox = gtk_event_box_new();
			gtk_widget_modify_bg(eBox, GTK_STATE_NORMAL, &settings.colors.sb[type]);
			gtk_container_add(GTK_CONTAINER(controlBox), eBox);
			
		break;

		case SPLITCTRL:
			controlFrame = gtk_frame_new(NULL);
			gtk_container_add(GTK_CONTAINER(control_box), controlFrame);
			controlBox = gtk_vbox_new(FALSE, 0);
			gtk_container_set_border_width(GTK_CONTAINER(controlBox), 5);
			gtk_container_add(GTK_CONTAINER(controlFrame), controlBox);

			eBox = gtk_event_box_new();
			gtk_widget_modify_bg(eBox, GTK_STATE_NORMAL, &settings.colors.sb[type]);
			box = gtk_vbox_new(FALSE, 0);
			label = gtk_label_new(NULL);
	  		gtk_label_set_markup(GTK_LABEL(label), 
					(const gchar *) "<span weight=\"bold\" foreground=\"white\" font_desc=\"12\">SPLIT SCREEN OPTIONS</span>");
			gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, 0);
			gtk_container_add(GTK_CONTAINER(eBox), box);
			gtk_box_pack_start(GTK_BOX(controlBox), eBox, FALSE, FALSE, 2);
						
			sep = gtk_hseparator_new();
			gtk_widget_modify_fg(sep, GTK_STATE_NORMAL, &white);
			gtk_box_pack_start(GTK_BOX(controlBox), sep, FALSE, FALSE, 2);

#if 0
			frame1 = gtk_frame_new("2 Screens");
			hbox = gtk_hbox_new(FALSE, 4);
			
			frame = gtk_frame_new("Top");
			box = gtk_vbox_new(FALSE, 0);
			gtk_size_group_add_widget(size_group, box);
			Hradios[SPLIT2][TOP][TRACESCR] = radioB1 = gtk_radio_button_new_with_label(NULL, "Trace");
			gtk_container_add(GTK_CONTAINER(box), radioB1);
			Hradios[SPLIT2][TOP][MAGNIFYSCR] = radioB2 = 
					gtk_radio_button_new_with_label(gtk_radio_button_get_group((GtkRadioButton *) radioB1), "Magnify");
			gtk_container_add(GTK_CONTAINER(box), radioB2);
			Hradios[SPLIT2][TOP][SPECTRASCR] = radioB3 = 
					gtk_radio_button_new_with_label(gtk_radio_button_get_group((GtkRadioButton *) radioB1), "Spectra");
			gtk_container_add(GTK_CONTAINER(box), radioB3);
			gtk_container_add(GTK_CONTAINER(frame), box);
			gtk_container_add(GTK_CONTAINER(hbox), frame);

			frame = gtk_frame_new("Bottom");
			box = gtk_vbox_new(FALSE, 0);
			gtk_size_group_add_widget(size_group, box);
			Hradios[SPLIT2][BOT][TRACESCR] = radioB1 = gtk_radio_button_new_with_label(NULL, "Trace");
			gtk_container_add(GTK_CONTAINER(box), radioB1);
			Hradios[SPLIT2][BOT][MAGNIFYSCR] = radioB2 = 
					gtk_radio_button_new_with_label(gtk_radio_button_get_group((GtkRadioButton *) radioB1), "Magnify");
			gtk_container_add(GTK_CONTAINER(box), radioB2);
			Hradios[SPLIT2][BOT][SPECTRASCR] = radioB3 = 
					gtk_radio_button_new_with_label(gtk_radio_button_get_group((GtkRadioButton *) radioB1), "Spectra");
			gtk_container_add(GTK_CONTAINER(box), radioB3);
			gtk_container_add(GTK_CONTAINER(frame), box);
			gtk_container_add(GTK_CONTAINER(hbox), frame);
			
			gtk_container_add(GTK_CONTAINER(frame1), hbox);
			gtk_box_pack_start(GTK_BOX(controlBox), frame1, FALSE, FALSE, 2);

			frame1 = gtk_frame_new("3 Screens");
			hbox = gtk_hbox_new(FALSE, 4);

			frame = gtk_frame_new("Top");
			box = gtk_vbox_new(FALSE, 0);
			gtk_size_group_add_widget(size_group, box);
			Hradios[SPLIT3][TOP][TRACESCR] = radioB1 = gtk_radio_button_new_with_label(NULL, "Trace");
			gtk_container_add(GTK_CONTAINER(box), radioB1);
			Hradios[SPLIT3][TOP][MAGNIFYSCR] = radioB2 = 
					gtk_radio_button_new_with_label(gtk_radio_button_get_group((GtkRadioButton *) radioB1), "Magnify");
			gtk_container_add(GTK_CONTAINER(box), radioB2);
			Hradios[SPLIT3][TOP][SPECTRASCR] = radioB3 = 
					gtk_radio_button_new_with_label(gtk_radio_button_get_group((GtkRadioButton *) radioB1), "Spectra");
			gtk_container_add(GTK_CONTAINER(box), radioB3);
			gtk_container_add(GTK_CONTAINER(frame), box);
			gtk_container_add(GTK_CONTAINER(hbox), frame);

			frame = gtk_frame_new("Middle");
			box = gtk_vbox_new(FALSE, 0);
			gtk_size_group_add_widget(size_group, box);
			Hradios[SPLIT3][MID][TRACESCR] = radioB1 = gtk_radio_button_new_with_label(NULL, "Trace");
			gtk_container_add(GTK_CONTAINER(box), radioB1);
			Hradios[SPLIT3][MID][MAGNIFYSCR] = radioB2 = 
					gtk_radio_button_new_with_label(gtk_radio_button_get_group((GtkRadioButton *) radioB1), "Magnify");
			gtk_container_add(GTK_CONTAINER(box), radioB2);
			Hradios[SPLIT3][MID][SPECTRASCR] = radioB3 = 
					gtk_radio_button_new_with_label(gtk_radio_button_get_group((GtkRadioButton *) radioB1), "Spectra");
			gtk_container_add(GTK_CONTAINER(box), radioB3);
			gtk_container_add(GTK_CONTAINER(frame), box);
			gtk_container_add(GTK_CONTAINER(hbox), frame);

			frame = gtk_frame_new("Bottom");
			box = gtk_vbox_new(FALSE, 0);
			gtk_size_group_add_widget(size_group, box);
			Hradios[SPLIT3][BOT][TRACESCR] = radioB1 = gtk_radio_button_new_with_label(NULL, "Trace");
			gtk_container_add(GTK_CONTAINER(box), radioB1);
			Hradios[SPLIT3][BOT][MAGNIFYSCR] = radioB2 = 
					gtk_radio_button_new_with_label(gtk_radio_button_get_group((GtkRadioButton *) radioB1), "Magnify");
			gtk_container_add(GTK_CONTAINER(box), radioB2);
			Hradios[SPLIT3][BOT][SPECTRASCR] = radioB3 = 
					gtk_radio_button_new_with_label(gtk_radio_button_get_group((GtkRadioButton *) radioB1), "Spectra");
			gtk_container_add(GTK_CONTAINER(box), radioB3);
			gtk_container_add(GTK_CONTAINER(frame), box);
			gtk_container_add(GTK_CONTAINER(hbox), frame);
			
			gtk_container_add(GTK_CONTAINER(frame1), hbox);
			gtk_box_pack_start(GTK_BOX(controlBox), frame1, FALSE, FALSE, 2);
#endif
			frame = gtk_frame_new("Colors");
			hbox = gtk_hbox_new(FALSE, 0);
	
			frame1 = gtk_frame_new("Sidebar");
			colorsSB[SPLITSCR] = gtk_color_button_new();
			gtk_color_button_set_title(GTK_COLOR_BUTTON (colorsSB[SPLITSCR]), "Sidebar");
			g_signal_connect(colorsSB[SPLITSCR], "color-set", G_CALLBACK(setSB), GINT_TO_POINTER(SPLITSCR));
			gtk_widget_set_size_request(colorsSB[SPLITSCR], 60, 40);
			gtk_container_add(GTK_CONTAINER(frame1), colorsSB[SPLITSCR]);
			gtk_box_pack_start(GTK_BOX(hbox), frame1, FALSE, FALSE, 2);
			strcpy(toolTip, "Set the SIDEBAR Colour");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), colorsSB[SPLITSCR], toolTip, toolTip);
	
			gtk_container_add(GTK_CONTAINER(frame), hbox);
			gtk_box_pack_start(GTK_BOX(controlBox), frame, FALSE, FALSE, 2);

			actionB = gtk_hbutton_box_new();
			gtk_button_box_set_layout(GTK_BUTTON_BOX(actionB), GTK_BUTTONBOX_SPREAD);
			gtk_box_set_spacing(GTK_BOX(actionB), 2);
	
			button = gtk_button_new_with_label("Continue");
			g_signal_connect(button, "clicked", G_CALLBACK(get_settings), NULL);
			g_signal_connect(button, "clicked", G_CALLBACK(raise_page), &states.currentPage);
			gtk_container_add(GTK_CONTAINER(actionB), button);
			gtk_box_pack_start(GTK_BOX(controlBox), actionB, FALSE, FALSE, 2);
			strcpy(toolTip, "APPLY New Settings and RETURN to SPLIT Screen");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), button, toolTip, toolTip);
	
			eBox = gtk_event_box_new();
			gtk_widget_modify_bg(eBox, GTK_STATE_NORMAL, &settings.colors.sb[type]);
			gtk_container_add(GTK_CONTAINER(controlBox), eBox);
			
		break;

		case GENCTRL:
			controlFrame = gtk_frame_new(NULL);
			gtk_container_add(GTK_CONTAINER(control_box), controlFrame);
			controlBox = gtk_vbox_new(FALSE, 0);
			gtk_container_set_border_width(GTK_CONTAINER(controlBox), 5);
			gtk_container_add(GTK_CONTAINER(controlFrame), controlBox);
	
			eBox = gtk_event_box_new();
			gettimeofday(&tv, &tz);
			box = gtk_vbox_new(FALSE, 0);
			label = gtk_label_new(NULL);
	  		gtk_label_set_markup(GTK_LABEL(label), 
					(const gchar *) "<span weight=\"bold\" foreground=\"white\" font_desc=\"12\">GENERAL OPTIONS</span>");
			gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, 0);
			gtk_container_add(GTK_CONTAINER(eBox), box);
			gtk_box_pack_start(GTK_BOX(controlBox), eBox, FALSE, FALSE, 2);
						
			sep = gtk_hseparator_new();
			gtk_widget_modify_fg(sep, GTK_STATE_NORMAL, &white);
			gtk_box_pack_start(GTK_BOX(controlBox), sep, FALSE, FALSE, 2);
			
			frame = gtk_frame_new("SORT Options");
			box = gtk_vbox_new(FALSE, 3);
			gtk_container_add(GTK_CONTAINER(frame), box);
			gtk_box_pack_start(GTK_BOX(controlBox), frame, FALSE, FALSE, 2);
			
			frame = gtk_frame_new("Default SORT");
			hbox = gtk_hbox_new(FALSE, 0);
			size_group = gtk_size_group_new(GTK_SIZE_GROUP_BOTH);
			gtk_container_add(GTK_CONTAINER(frame), hbox);
			gtk_box_pack_start(GTK_BOX(box), frame, FALSE, FALSE, 2);

			sortDefault = combo = gtk_combo_box_new_text();
			gtk_widget_set_size_request(combo, 100, -1);
			gtk_size_group_add_widget(size_group, combo);
			gtk_box_pack_start(GTK_BOX(hbox), combo, FALSE, FALSE, 2);
			for(i=0;i<TTLSORTTYPES;i++)
			{
				gtk_combo_box_append_text(GTK_COMBO_BOX(combo), sortTypes[i]);
			}

			frame = gtk_frame_new("SORT Definitions");
			vbox = gtk_vbox_new(FALSE, 0);
			hbox = gtk_hbox_new(FALSE, 0);
			gtk_container_add(GTK_CONTAINER(vbox), hbox);
			gtk_box_pack_start(GTK_BOX(box), frame, FALSE, FALSE, 2);
			
			sortType = combo = gtk_combo_box_new_text();
			gtk_widget_set_size_request(combo, 100, -1);
			gtk_size_group_add_widget(size_group, combo);
			gtk_box_pack_start(GTK_BOX(hbox), combo, FALSE, FALSE, 2);
			for(i=0;i<TTLSORTTYPES;i++)
			{
				gtk_combo_box_append_text(GTK_COMBO_BOX(combo), sortTypes[i]);
			}
			g_signal_connect(combo, "changed", G_CALLBACK(sortTypeSelect), NULL);
			g_object_unref(size_group);
			
			actionB = gtk_hbutton_box_new();
			gtk_button_box_set_layout(GTK_BUTTON_BOX(actionB), GTK_BUTTONBOX_SPREAD);
			gtk_box_set_spacing(GTK_BOX(actionB), 2);

			button = gtk_button_new_with_label("Edit...");
			g_signal_connect(button, "clicked", G_CALLBACK(makeSortPopup), NULL);
			gtk_container_add(GTK_CONTAINER(actionB), button);
			strcpy(toolTip, "EDIT Sort Criteria for this Sort Type");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), button, toolTip, toolTip);

			sortButton = button = gtk_button_new_with_label("Sort");
			g_signal_connect(button, "clicked", G_CALLBACK(get_settings), NULL);
			g_signal_connect(button, "clicked", G_CALLBACK(sortTraces), GINT_TO_POINTER(FALSE));
			gtk_container_add(GTK_CONTAINER(actionB), button);
			strcpy(toolTip, "SORT Current Trace List using Current Sort Settings");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), button, toolTip, toolTip);
	
			gtk_container_add(GTK_CONTAINER(hbox), actionB);
			gtk_container_add(GTK_CONTAINER(frame), vbox);
	
			frame = gtk_frame_new("PRINT Format");
			box = gtk_vbox_new(FALSE, 3);
			TprtFrmt[PRINT_FMT_PS] = radioB1 = gtk_radio_button_new_with_label(NULL, "POSTSCRIPT");
			gtk_container_add(GTK_CONTAINER(box), radioB1);
			TprtFrmt[PRINT_FMT_PNG] = radioB2 = 
					gtk_radio_button_new_with_label(gtk_radio_button_get_group((GtkRadioButton *) radioB1), "PNG");
			gtk_container_add(GTK_CONTAINER(box), radioB2);
			gtk_container_add(GTK_CONTAINER(frame), box);
			gtk_box_pack_start(GTK_BOX(controlBox), frame, FALSE, FALSE, 2);

			frame = gtk_frame_new("Printing");
			box = gtk_vbox_new(FALSE, 0);

			size_group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
			hbox = gtk_hbox_new(FALSE, 0);
			label = gtk_label_new("Filename: ");
			TprtFnm = entry = gtk_entry_new();
			gtk_entry_set_max_length(GTK_ENTRY(entry), 95);
			button = gtk_button_new_with_label("...");
			gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);
			gtk_box_pack_start(GTK_BOX(hbox), entry, FALSE, FALSE, 2);
			gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 2);
			gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 2);
			gtk_size_group_add_widget(size_group, label);
			gtk_misc_set_alignment(GTK_MISC (label), 1, .5);
			strcpy(toolTip, "Specify Filename of Output File:\n\nPS: Leave Blank for Temporary File Usage when Using with Command\n\nPNG: Leave Blank to be Asked.");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), entry, toolTip, toolTip);
			g_signal_connect(button, "clicked", G_CALLBACK(getFilename), GINT_TO_POINTER(PRINTFTYPE));
	
			hbox = gtk_hbox_new(FALSE, 0);
			label = gtk_label_new("Print Command: ");
			TprtCmd = entry = gtk_entry_new();
			gtk_entry_set_max_length(GTK_ENTRY(entry), 25);
			gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);
			gtk_box_pack_start(GTK_BOX(hbox), entry, FALSE, FALSE, 2);
			gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 2);
			gtk_size_group_add_widget(size_group, label);
			gtk_misc_set_alignment(GTK_MISC (label), 1, .5);
			strcpy(toolTip, "PS Only:\nSpecify Command to PRINT.\nLeave Blank for File Output Only.");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), entry, toolTip, toolTip);
	
			label = gtk_label_new("Resolution: ");
			hbox = gtk_hbox_new(FALSE, 0);
			gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);
			TprtPRes1 = radioB1 = gtk_radio_button_new_with_label(NULL, "Low");
			gtk_box_pack_start(GTK_BOX(hbox), radioB1, FALSE, FALSE, 2);
			TprtPRes2 = radioB2 = gtk_radio_button_new_with_label(
									gtk_radio_button_get_group((GtkRadioButton *) radioB1), "High");
			gtk_box_pack_start(GTK_BOX(hbox), radioB2, FALSE, FALSE, 2);
			gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 2);
			gtk_size_group_add_widget(size_group, label);
			gtk_misc_set_alignment(GTK_MISC (label), 1, .5);
			strcpy(toolTip, "LOW Resolution - Better for Screen Display");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), radioB1, toolTip, toolTip);
			strcpy(toolTip, "HIGH Resolution - Better for Paper Print");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), radioB2, toolTip, toolTip);
			
			label = gtk_label_new("Paper Size: ");
			hbox = gtk_hbox_new(FALSE, 0);
			gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);
			TprtPSize1 = radioB1 = gtk_radio_button_new_with_label(NULL, "Letter");
			gtk_box_pack_start(GTK_BOX(hbox), radioB1, FALSE, FALSE, 2);
			TprtPSize2 = radioB2 = gtk_radio_button_new_with_label(
									gtk_radio_button_get_group((GtkRadioButton *) radioB1), "A4");
			gtk_box_pack_start(GTK_BOX(hbox), radioB2, FALSE, FALSE, 2);
			gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 2);
			gtk_size_group_add_widget(size_group, label);
			gtk_misc_set_alignment(GTK_MISC (label), 1, .5);
			strcpy(toolTip, "Paper Size: 8.5in x 11.5in");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), radioB1, toolTip, toolTip);
			strcpy(toolTip, "Paper Size: 210mm x 297mm");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), radioB2, toolTip, toolTip);
			g_object_unref(size_group);
			
			gtk_container_add(GTK_CONTAINER(frame), box);
			gtk_box_pack_start(GTK_BOX(controlBox), frame, FALSE, FALSE, 2);

			sep = gtk_hseparator_new();
			gtk_widget_modify_fg(sep, GTK_STATE_NORMAL, &white);
			gtk_box_pack_start(GTK_BOX(controlBox), sep, FALSE, FALSE, 2);

			frame = gtk_frame_new("Transform Options");
			box = gtk_vbox_new(FALSE, 3);
			gtk_container_add(GTK_CONTAINER(frame), box);
			gtk_box_pack_start(GTK_BOX(controlBox), frame, FALSE, FALSE, 2);
			
			frame = gtk_frame_new("UVW Sensors");
			gtk_box_pack_start(GTK_BOX(box), frame, FALSE, FALSE, 2);
			box = gtk_vbox_new(FALSE, 3);
			gtk_container_add(GTK_CONTAINER(frame), box);
			TFuvw[STS2] = radioB1 = gtk_radio_button_new_with_label(NULL, "STS-2");
			gtk_container_add(GTK_CONTAINER(box), radioB1);
			TFuvw[TRILLIUM] = radioB2 = 
					gtk_radio_button_new_with_label(gtk_radio_button_get_group((GtkRadioButton *) radioB1), "Trillium");
			gtk_container_add(GTK_CONTAINER(box), radioB2);

			actionB = gtk_hbutton_box_new();
			gtk_button_box_set_layout(GTK_BUTTON_BOX(actionB), GTK_BUTTONBOX_SPREAD);
			gtk_box_set_spacing(GTK_BOX(actionB), 2);
	
			button = gtk_button_new_with_label("Continue");
			g_signal_connect(button, "clicked", G_CALLBACK(get_settings), NULL);
			g_signal_connect(button, "clicked", G_CALLBACK(raise_page), &states.currentPage);
			gtk_container_add(GTK_CONTAINER(actionB), button);
			gtk_box_pack_start(GTK_BOX(controlBox), actionB, FALSE, FALSE, 2);
			strcpy(toolTip, "APPLY New Settings and RETURN to Current Screen");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), button, toolTip, toolTip);
	
			srand(tv.tv_usec);	// randomly select the background color
			j = (int) ((float) MAXOVERLAYCOLOURS * rand()/(RAND_MAX+1.0));
			gtk_widget_modify_bg(eBox, GTK_STATE_NORMAL, &settings.colors.ol[j]);

			eBox = gtk_event_box_new();
			gtk_widget_modify_bg(eBox, GTK_STATE_NORMAL, &settings.colors.ol[j]);
			gtk_container_add(GTK_CONTAINER(controlBox), eBox);
			
		break;

		case OVERLAYCTRL:
			controlFrame = gtk_frame_new(NULL);
			gtk_container_add(GTK_CONTAINER(control_box), controlFrame);
			controlBox = gtk_vbox_new(FALSE, 0);
			gtk_container_set_border_width(GTK_CONTAINER(controlBox), 5);
			gtk_container_add(GTK_CONTAINER(controlFrame), controlBox);
	
			eBox = gtk_event_box_new();
			gettimeofday(&tv, &tz);
			box = gtk_vbox_new(FALSE, 0);
			label = gtk_label_new(NULL);
	  		gtk_label_set_markup(GTK_LABEL(label), 
					(const gchar *) "<span weight=\"bold\" foreground=\"white\" font_desc=\"12\">OVERLAY COLOR OPTIONS</span>");
			gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, 0);
			gtk_container_add(GTK_CONTAINER(eBox), box);
			gtk_box_pack_start(GTK_BOX(controlBox), eBox, FALSE, FALSE, 2);
			
			frame = gtk_frame_new("Overlay Colors");
			vbox = gtk_vbox_new(FALSE, 0);
	
			for (i=0;i<MAXOVERLAYCOLOURS/5;i++)
			{
				hbox = gtk_hbox_new(FALSE, 0);
				for(j=0, idx=i*5;j<5;j++, idx++)
				{
					sprintf(str, "# %d", idx+1);
					frame1 = gtk_frame_new(str);
					colorsOL[idx] = gtk_color_button_new();
					sprintf(str, "Overlay Color # %d", idx+1);
					gtk_color_button_set_title(GTK_COLOR_BUTTON (colorsOL[idx]), str);
					g_signal_connect(colorsOL[idx], "color-set", G_CALLBACK(setOL), GINT_TO_POINTER(idx));
					gtk_container_add(GTK_CONTAINER(frame1), colorsOL[idx]);
					gtk_box_pack_start(GTK_BOX(hbox), frame1, FALSE, FALSE, 2);
				}
				gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 2);
			}
	
			gtk_container_add(GTK_CONTAINER(frame), vbox);
			gtk_box_pack_start(GTK_BOX(controlBox), frame, FALSE, FALSE, 2);
	
			actionB = gtk_hbutton_box_new();
			gtk_button_box_set_layout(GTK_BUTTON_BOX(actionB), GTK_BUTTONBOX_SPREAD);
			gtk_box_set_spacing(GTK_BOX(actionB), 2);
	
			button = gtk_button_new_with_label("Continue");
			g_signal_connect(button, "clicked", G_CALLBACK(get_settings), NULL);
			g_signal_connect(button, "clicked", G_CALLBACK(raise_page), &states.currentPage);
			gtk_container_add(GTK_CONTAINER(actionB), button);
			gtk_box_pack_start(GTK_BOX(controlBox), actionB, FALSE, FALSE, 2);
			strcpy(toolTip, "APPLY New Settings and RETURN to Current Screen");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), button, toolTip, toolTip);
	
			srand(tv.tv_usec);	// randomly select the background color
			j = (int) ((float) MAXOVERLAYCOLOURS * rand()/(RAND_MAX+1.0));
			gtk_widget_modify_bg(eBox, GTK_STATE_NORMAL, &settings.colors.ol[j]);

			eBox = gtk_event_box_new();
			gtk_widget_modify_bg(eBox, GTK_STATE_NORMAL, &settings.colors.ol[j]);
			gtk_container_add(GTK_CONTAINER(controlBox), eBox);
			
		break;

		case HEADERCTRL:
			controlFrame = gtk_frame_new(NULL);
			gtk_container_add(GTK_CONTAINER(control_box), controlFrame);
			controlBox = gtk_vbox_new(FALSE, 0);
			gtk_container_set_border_width(GTK_CONTAINER(controlBox), 5);
			gtk_container_add(GTK_CONTAINER(controlFrame), controlBox);
	
			eBox = gtk_event_box_new();
			gettimeofday(&tv, &tz);
			srand(tv.tv_usec);	// randomly select the background color
			j = (int) ((float) MAXOVERLAYCOLOURS * rand()/(RAND_MAX+1.0));
			gtk_widget_modify_bg(eBox, GTK_STATE_NORMAL, &settings.colors.ol[j]);

			box = gtk_vbox_new(FALSE, 0);
			label = gtk_label_new(NULL);
	  		gtk_label_set_markup(GTK_LABEL(label), 
					(const gchar *) "<span weight=\"bold\" foreground=\"white\" font_desc=\"12\">HEADER DISPLAY OPTIONS</span>");
			gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, 0);
			gtk_container_add(GTK_CONTAINER(eBox), box);
			gtk_box_pack_start(GTK_BOX(controlBox), eBox, FALSE, FALSE, 2);
			
			frame = gtk_frame_new("Display Header Fields");
			vbox = gtk_vbox_new(FALSE, 0);
			
			for(i=0;i<NUM_HDR_COLS;i++)
			{
				hdrToggles[i] = gtk_check_button_new_with_label(headers[i].desc);
				gtk_box_pack_start(GTK_BOX(vbox), hdrToggles[i], FALSE, FALSE, 2);
			}
			gtk_container_add(GTK_CONTAINER(frame), vbox);
			gtk_box_pack_start(GTK_BOX(controlBox), frame, FALSE, FALSE, 2);

			actionB = gtk_hbutton_box_new();
			gtk_button_box_set_layout(GTK_BUTTON_BOX(actionB), GTK_BUTTONBOX_SPREAD);
			gtk_box_set_spacing(GTK_BOX(actionB), 2);
	
			button = gtk_button_new_with_label("Continue");
			g_signal_connect(button, "clicked", G_CALLBACK(get_settings), NULL);
			g_signal_connect(button, "clicked", G_CALLBACK(raise_page), &states.currentPage);
			gtk_container_add(GTK_CONTAINER(actionB), button);
			gtk_box_pack_start(GTK_BOX(controlBox), actionB, FALSE, FALSE, 2);

			eBox = gtk_event_box_new();
			gtk_widget_modify_bg(eBox, GTK_STATE_NORMAL, &settings.colors.ol[j]);
			gtk_container_add(GTK_CONTAINER(controlBox), eBox);
		break;
	}

	return control_box;
}

GtkWidget *make_CtrlButtons(int type)
{
  GtkWidget *widget, *button, *menu_box;
  GtkWidget *actionB;
  int	scr;

  menu_box = gtk_event_box_new();

  widget = gtk_vbox_new(FALSE, 0);
  gtk_container_set_border_width(GTK_CONTAINER(widget), 5);
  gtk_container_add(GTK_CONTAINER(menu_box), widget);

  switch (type)
  {
	case TRACECTRL:
		scr = TRACESCR;
		
		actionB = gtk_vbutton_box_new();
  		gtk_box_pack_start(GTK_BOX(widget), actionB, FALSE, FALSE, 0);
		gtk_button_box_set_layout(GTK_BUTTON_BOX(actionB), GTK_BUTTONBOX_START);
		gtk_box_set_spacing(GTK_BOX(actionB), 2);

		button = gtk_button_new_with_label("Set Defaults");
		gtk_container_add(GTK_CONTAINER(actionB), button);
		g_signal_connect(button, "clicked", G_CALLBACK(save_defaults), &ctrl_settings);
		sprintf(toolTip, "WRITE PQL Settings to DEFAULTS File ($HOME/.pql/defaults.db), Re-Read on PQL StartUp");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), button, toolTip, toolTip);
		button = gtk_button_new_with_label("Restore Defaults");
		gtk_container_add(GTK_CONTAINER(actionB), button);
		g_signal_connect(button, "clicked", G_CALLBACK(restore_settings), NULL);
		strcpy(toolTip, "RE-READ PQL Settings from Default File");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), button, toolTip, toolTip);

		button = gtk_button_new_with_label("HELP");
		gtk_container_add(GTK_CONTAINER(actionB), button);
		g_signal_connect(button, "clicked", G_CALLBACK(help), &states.currentPage);
		strcpy(toolTip, "Display PQL Manual Pages");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), button, toolTip, toolTip);

		button = gtk_button_new_with_label("Cancel");
		gtk_container_add(GTK_CONTAINER(actionB), button);
		g_signal_connect(button, "clicked", G_CALLBACK(raise_page), &states.currentPage);
		strcpy(toolTip, "DO NOT APPLY Settings Changes");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), button, toolTip, toolTip);

#ifndef PQL_ONLY
		{
		void bugReport(GtkButton *button, gpointer nil);
		button = gtk_button_new_with_label("Bug Reports");
  		gtk_box_pack_end(GTK_BOX(widget), button, FALSE, FALSE, 0);
		g_signal_connect(button, "clicked", G_CALLBACK(bugReport), NULL);
		strcpy(toolTip, "Technical Support Information");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), button, toolTip, toolTip);
		}
#endif

	break;
  }

	gtk_widget_modify_bg(menu_box, GTK_STATE_NORMAL, &settings.colors.sb[scr]);

	return menu_box;
}

void make_ctrlScreens()
{
  GtkWidget *ctrlBox, *vbox;
  GtkWidget *frame;

  if (!ctrlWindow)
  {
		ctrlWindow = gtk_window_new (GTK_WINDOW_TOPLEVEL);
		gtk_window_set_title (GTK_WINDOW (ctrlWindow), "PQL II - Controls"); 
		gtk_container_set_border_width(GTK_CONTAINER(ctrlWindow), 2);
		gtk_window_set_position(GTK_WINDOW (ctrlWindow), GTK_WIN_POS_CENTER);
 		g_signal_connect(ctrlWindow, "delete-event", G_CALLBACK(destroyPqlControls), NULL);
		
		ctrlBox = gtk_hbox_new(FALSE, 0);
		gtk_container_add (GTK_CONTAINER(ctrlWindow), ctrlBox);

		controlButtons = make_CtrlButtons (TRACECTRL);
		gtk_box_pack_start(GTK_BOX(ctrlBox), controlButtons, FALSE, FALSE, 0);

		vbox = gtk_vbox_new(FALSE, 0);
		control_screens = gtk_notebook_new ();
		gtk_box_pack_start(GTK_BOX(vbox), control_screens, TRUE, TRUE, 0);
		statusBarCtrl = gtk_statusbar_new();
		gtk_box_pack_start(GTK_BOX(vbox), statusBarCtrl, FALSE, FALSE, 0);
		gtk_box_pack_start(GTK_BOX(ctrlBox), vbox, TRUE, TRUE, 0);

		frame = gtk_frame_new(NULL);
		gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
		controls[TRACECTRL] = make_controls(TRACECTRL);
		gtk_container_add(GTK_CONTAINER(frame), controls[TRACECTRL]);
		gtk_notebook_append_page(GTK_NOTEBOOK(control_screens), frame, gtk_label_new(" Trace "));

		frame = gtk_frame_new(NULL);
		gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
		controls[MAGCTRL] = make_controls(MAGCTRL);
		gtk_container_add(GTK_CONTAINER(frame), controls[MAGCTRL]);
		gtk_notebook_append_page(GTK_NOTEBOOK(control_screens), frame, gtk_label_new(" Magnify "));

		frame = gtk_frame_new(NULL);
		gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
		controls[SPECCTRL] = make_controls(SPECCTRL);
		gtk_container_add(GTK_CONTAINER(frame), controls[SPECCTRL]);
		gtk_notebook_append_page(GTK_NOTEBOOK(control_screens), frame, gtk_label_new(" Spectra "));

		frame = gtk_frame_new(NULL);
		gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
		controls[SPECCTRL] = make_controls(SPLITCTRL);
		gtk_container_add(GTK_CONTAINER(frame), controls[SPECCTRL]);
		gtk_notebook_append_page(GTK_NOTEBOOK(control_screens), frame, gtk_label_new(" Split "));

		frame = gtk_frame_new(NULL);
		gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
		controls[HEADERCTRL] = make_controls(HEADERCTRL);
		gtk_container_add(GTK_CONTAINER(frame), controls[HEADERCTRL]);
		gtk_notebook_append_page(GTK_NOTEBOOK(control_screens), frame, gtk_label_new(" Headers "));

		frame = gtk_frame_new(NULL);
		gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
		controls[GENCTRL] = make_controls(GENCTRL);
		gtk_container_add(GTK_CONTAINER(frame), controls[GENCTRL]);
		gtk_notebook_append_page(GTK_NOTEBOOK(control_screens), frame, gtk_label_new(" General "));

#if 0
		frame = gtk_frame_new(NULL);
		gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
		controls[SORTCTRL] = make_controls(SORTCTRL);
		gtk_container_add(GTK_CONTAINER(frame), controls[SORTCTRL]);
		gtk_notebook_append_page(GTK_NOTEBOOK(control_screens), frame, gtk_label_new(" Sort "));
#endif
		frame = gtk_frame_new(NULL);
		gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
		controls[OVERLAYCTRL] = make_controls(OVERLAYCTRL);
		gtk_container_add(GTK_CONTAINER(frame), controls[OVERLAYCTRL]);
		gtk_notebook_append_page(GTK_NOTEBOOK(control_screens), frame, gtk_label_new(" Overlay "));

		gtk_widget_show_all(ctrlWindow);
		gtk_window_set_transient_for(GTK_WINDOW (ctrlWindow), GTK_WINDOW (topWindow));

  }

  return;
}

