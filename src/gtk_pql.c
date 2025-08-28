#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "pql_defines.h"
#include "pql_externs.h"
#include "gtk_pql_externs.h"
#ifndef PQL_ONLY
#include "pqlx_externs.h"
#endif

GtkTooltips *tips;
static char	toolTip[1000];
static gboolean	lowRes;
static GtkWidget	*logNBTab;
static int			logScr;

void checkLogTab()
{
	if (states.logs.logs)
	{
		states.logs.visible = TRUE;
		gtk_widget_show(logNBTab);
	}
	else
	{
		states.logs.visible = FALSE;
		gtk_widget_hide(logNBTab);
	}
}

#ifdef PQL_ONLY
gboolean FKeyEvent(GtkWidget *widget, GdkEventKey *event, gpointer nil);
#endif

GtkWidget *makeSideBar(int type)
{
	GtkWidget *widget, *button, *menu_box, *eBox, *mouseBox;
	GtkWidget *actionB, *entry, *frame1, *vbox;
	GtkWidget *frame, *box, *radioB, *radioB1, *radioB2;
	GtkWidget *label, *sep, *hbox, *combo;
	static GtkSizeGroup	*size_group;
	GtkSizeGroup *size_group2;
	int	scr=0, i;
	struct timeval tv;
	struct timezone tz;

	menu_box = gtk_event_box_new();
	
	widget = gtk_vbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(widget), 5);
	gtk_container_add(GTK_CONTAINER(menu_box), widget);

  switch (type)
  {
	 case TRACESCR:
		gtk_widget_set_size_request(widget, BUTTONW, 0);
		scr = TRACESCR;
		size_group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);

  		label = gtk_label_new(NULL);
  		gtk_label_set_markup(GTK_LABEL(label),
				(const gchar *) "<span weight=\"bold\" foreground=\"white\" font_desc=\"18\">PQL II</span>");
  		gtk_box_pack_start(GTK_BOX(widget), label, FALSE, FALSE, 3);

  		sep = gtk_hseparator_new();
  		gtk_widget_modify_fg(sep, GTK_STATE_NORMAL, &white);
		gtk_box_pack_start(GTK_BOX(widget), sep, FALSE, FALSE, 7);

		button = gtk_button_new_with_label("TRACES");
		g_signal_connect((button), "clicked", G_CALLBACK(openTraces), NULL);
 		gtk_box_pack_start(GTK_BOX(widget), button, FALSE, FALSE, 1);
		strcpy(toolTip, "Open Trace files");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), button, toolTip, toolTip);

		button = gtk_button_new_with_label("Next");
		g_signal_connect((button), "clicked", G_CALLBACK(nextTrace), GINT_TO_POINTER(TRUE));
 		gtk_box_pack_start(GTK_BOX(widget), button, FALSE, FALSE, 1);
		strcpy(toolTip, "Display NEXT Screen of Trace Files");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), button, toolTip, toolTip);

		button = gtk_button_new_with_label("Previous");
 		gtk_box_pack_start(GTK_BOX(widget), button, FALSE, FALSE, 1);
		g_signal_connect(button, "clicked", G_CALLBACK(nextTrace), GINT_TO_POINTER(FALSE));
		strcpy(toolTip, "Display PREVIOUS Screen of Trace Files");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), button, toolTip, toolTip);

  		sep = gtk_hseparator_new();
  		gtk_widget_modify_fg(sep, GTK_STATE_NORMAL, &white);
		gtk_box_pack_start(GTK_BOX(widget), sep, FALSE, FALSE, 7);

		eBox = gtk_event_box_new();
		frame = gtk_frame_new("Window Scale");
		box = gtk_vbox_new(FALSE, 0);
		gtk_container_add(GTK_CONTAINER(frame), box);
		gtk_container_add(GTK_CONTAINER(eBox), frame);
		gtk_box_pack_start(GTK_BOX(widget), eBox, FALSE, FALSE, 2);

		Tcbo[TRCWSCALE] = combo = gtk_combo_box_new_text();
		gtk_box_pack_start(GTK_BOX(box), combo, FALSE, FALSE, 2);
		strcpy(toolTip, "Set AMPLITUDE Scale for Traces on Display");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), combo, toolTip, toolTip);
		for(i=0;i<TTLWSCALES;i++)
		{
			gtk_combo_box_append_text(GTK_COMBO_BOX(combo), wscales[i]);
		}
		gtk_combo_box_set_active(GTK_COMBO_BOX(combo), settings.trace.amp);
		g_signal_connect(combo, "changed", G_CALLBACK(pqlComboSel), GINT_TO_POINTER(TRCWSCALE));
		
		eBox = gtk_event_box_new();
		frame = gtk_frame_new("Time Axis");
		box = gtk_vbox_new(FALSE, 0);
		gtk_container_add(GTK_CONTAINER(frame), box);
		gtk_container_add(GTK_CONTAINER(eBox), frame);
		gtk_box_pack_start(GTK_BOX(widget), eBox, FALSE, FALSE, 2);

		Tcbo[TRCXAXIS] = combo = gtk_combo_box_new_text();
		gtk_box_pack_start(GTK_BOX(box), combo, FALSE, FALSE, 2);
		strcpy(toolTip, "Set Time Axis Scale");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), combo, toolTip, toolTip);
		for(i=0;i<TTLTIMESCALES;i++)
		{
			gtk_combo_box_append_text(GTK_COMBO_BOX(combo), xscales[i]);
		}
		gtk_combo_box_set_active(GTK_COMBO_BOX(combo), settings.trace.time);
		g_signal_connect(combo, "changed", G_CALLBACK(pqlComboSel), GINT_TO_POINTER(TRCXAXIS));

#if 0
		eBox = gtk_event_box_new();
		frame = gtk_frame_new("Display Units");
		box = gtk_vbox_new(FALSE, 0);
		gtk_container_add(GTK_CONTAINER(frame), box);
		gtk_container_add(GTK_CONTAINER(eBox), frame);
		gtk_box_pack_start(GTK_BOX(widget), eBox, FALSE, FALSE, 2);

		Tcbo[TRCDISPU] = combo = gtk_combo_box_new_text();
		gtk_box_pack_start(GTK_BOX(box), combo, FALSE, FALSE, 2);
		strcpy(toolTip, "Set Display Units");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), combo, toolTip, toolTip);
		for(i=0;i<TTLDISPUNITS;i++)
		{
			gtk_combo_box_append_text(GTK_COMBO_BOX(combo), dispunits[i]);
		}
		signals[DISPSGNL][TRACESIGNAL] = g_signal_connect(combo, "changed", 
										G_CALLBACK(pqlComboSel), GINT_TO_POINTER(TRCDISPU));
#endif

		dispGainB[scr] = eBox = gtk_event_box_new();
		frame = gtk_frame_new("Display Gain");
		box = gtk_vbox_new(FALSE, 0);
		dispGain[scr] = entry = gtk_entry_new();
		gtk_container_add(GTK_CONTAINER(box), entry);
		gtk_container_add(GTK_CONTAINER(frame), box);
		gtk_container_add(GTK_CONTAINER(eBox), frame);
		gtk_box_pack_start(GTK_BOX(widget), eBox, FALSE, FALSE, 2);
		strcpy(toolTip, "Display Gain to be Applied\nReal Number > 0 (e.g., 3.14)\n\t1.0 = automatic"); 
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), entry, toolTip, toolTip);

  		sep = gtk_hseparator_new();
  		gtk_widget_modify_fg(sep, GTK_STATE_NORMAL, &white);
		gtk_box_pack_start(GTK_BOX(widget), sep, FALSE, FALSE, 7);

		button = gtk_button_new_with_label("Remove");
 		gtk_box_pack_start(GTK_BOX(widget), button, FALSE, FALSE, 1);
		strcpy(toolTip, "DELETE Selected Traces from Input List");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), button, toolTip, toolTip);
  		g_signal_connect(button, "clicked", G_CALLBACK (selectT),  GINT_TO_POINTER(REMOVE));
		button = gtk_button_new_with_label("Select All");
 		gtk_box_pack_start(GTK_BOX(widget), button, FALSE, FALSE, 1);
  		g_signal_connect(button, "clicked", G_CALLBACK (selectT), GINT_TO_POINTER(SELECT));
		strcpy(toolTip, "SELECT ALL Traces");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), button, toolTip, toolTip);
		button = gtk_button_new_with_label("Deselect All");
 		gtk_box_pack_start(GTK_BOX(widget), button, FALSE, FALSE, 1);
  		g_signal_connect(button, "clicked", G_CALLBACK (selectT), GINT_TO_POINTER(DESELECT));
		strcpy(toolTip, "DE-SELECT ALL Traces");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), button, toolTip, toolTip);
		button = gtk_button_new_with_label("MARK");
 		gtk_box_pack_start(GTK_BOX(widget), button, FALSE, FALSE, 1);
  		g_signal_connect(button, "clicked", G_CALLBACK (mark), NULL);
		strcpy(toolTip, "Write Selected Traces to MARK File");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), button, toolTip, toolTip);

  		sep = gtk_hseparator_new();
  		gtk_widget_modify_fg(sep, GTK_STATE_NORMAL, &white);
		gtk_box_pack_start(GTK_BOX(widget), sep, FALSE, FALSE, 7);

		button = gtk_button_new_with_label("Print");
 		gtk_box_pack_start(GTK_BOX(widget), button, FALSE, FALSE, 1);
  		g_signal_connect(button, "clicked", G_CALLBACK (print), GINT_TO_POINTER(TRACESCR));
		strcpy(toolTip, "Create POSTSCRIPT File Ready for Printing");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), button, toolTip, toolTip);
		button = gtk_button_new_with_label("Controls");
 		gtk_box_pack_start(GTK_BOX(widget), button, FALSE, FALSE, 1);
		g_signal_connect(button, "clicked", G_CALLBACK(raiseControls), GINT_TO_POINTER(TRACECTRL));
		strcpy(toolTip, "View CONTROLS Panel and HELP.");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), button, toolTip, toolTip);

  		sep = gtk_hseparator_new();
  		gtk_widget_modify_fg(sep, GTK_STATE_NORMAL, &white);
		gtk_box_pack_start(GTK_BOX(widget), sep, FALSE, FALSE, 7);

		button = gtk_button_new_with_label("HELP");
 		gtk_box_pack_start(GTK_BOX(widget), button, FALSE, FALSE, 1);
		g_signal_connect(button, "clicked", G_CALLBACK(help), &states.currentPage);
		strcpy(toolTip, "Display PQL Manual Pages");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), button, toolTip, toolTip);

		button = gtk_button_new_with_label("About");
 		gtk_box_pack_start(GTK_BOX(widget), button, FALSE, FALSE, 1);
#ifdef PQL_ONLY
  		g_signal_connect(button, "clicked", G_CALLBACK (about), NULL);
#else
  		g_signal_connect(button, "clicked", G_CALLBACK (about), GINT_TO_POINTER(PQLSYSTEM));
#endif
		button = gtk_button_new_with_label("QUIT");
  		g_signal_connect(button, "clicked", G_CALLBACK (gtk_main_quit), NULL);
 		gtk_box_pack_start(GTK_BOX(widget), button, FALSE, FALSE, 1);
		strcpy(toolTip, "EXIT PQL");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), button, toolTip, toolTip);

		box = gtk_vbox_new(FALSE, 0);
		progressBar = gtk_progress_bar_new();
		gtk_widget_set_size_request(progressBar, BUTTONW, BUTTONH);
		mouseBox = gtk_event_box_new();
		label = gtk_label_new("mouse tips");
		gtk_container_add(GTK_CONTAINER(mouseBox), label);
		gtk_box_pack_end(GTK_BOX(box), mouseBox, FALSE, FALSE, 0);
#ifndef PQL_ONLY
		strcpy(toolTip, "F-Keys:\n"\
						" F1 - Go To Trace Viewer::Magnify\n"\
						" F2 - Go To PSD Viewer\n"\
						" F3 - Go To STN Viewer\n"\
						"\nWithin PLOT Region:\n"\
						" Magnify Boundary:\tCLICK\n"\
						" Data Point(s):\t\tCTRL+CLICK\n"\
						" Scan Data Points:\tCTRL+CLICK+DRAG\n"\
						" Make Trace First:\tSHIFT+CLICK\n\n"\
						"Within LABEL Region:\n"\
						" Select Trace:\t\tCLICK\n"\
						" Select Traces:\t\tCLICK+DRAG");
#else
		strcpy(toolTip, "F-Keys:\n"\
						" F1 - Go To PQL::Magnify\n"\
						"\nWithin PLOT Region:\n"\
						" Magnify Boundary:\tCLICK\n"\
						" Data Point(s):\t\tCTRL+CLICK\n"\
						" Scan Data Points:\tCTRL+CLICK+DRAG\n"\
						" Make Trace First:\tSHIFT+CLICK\n\n"\
						"Within LABEL Region:\n"\
						" Select Trace:\t\tCLICK\n"\
						" Select Traces:\t\tCLICK+DRAG");
#endif
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), mouseBox, toolTip, toolTip);
		gtk_box_pack_end(GTK_BOX(box), progressBar, FALSE, FALSE, 2);
		gtk_container_add(GTK_CONTAINER(widget), box);

  		// set some values
//  		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (Tscale), !settings.trace.amp);
//		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (Ttime), !settings.trace.time);
		settings.general.display.gather.gainS[scr] = g_strdup_printf("%.2f",
														settings.general.display.gather.gain[scr]);
		gtk_entry_set_text(GTK_ENTRY(dispGain[scr]), settings.general.display.gather.gainS[scr]);

		// attach the signals, can't do this before setting the values
//		g_signal_connect(Tscale, "toggled", G_CALLBACK(radioB_chg), &settings.trace.amp);
//		g_signal_connect(Ttime, "toggled", G_CALLBACK(radioB_chg), &settings.trace.time);
//		signals[DISPSGNL][TRACESIGNAL] = g_signal_connect(Tdisp[VOLTS], "toggled", 
//				G_CALLBACK(radioB_chg),	&settings.general.units);
		g_signal_connect(dispGain[scr],"activate", G_CALLBACK(reDraw), GINT_TO_POINTER(scr));

	break;

	case MAGNIFYSCR:
		scr = MAGNIFYSCR;
		
		if (!lowRes)
		{
	  		label = gtk_label_new(NULL);
  			gtk_label_set_markup(GTK_LABEL(label),
					(const gchar *) "<span weight=\"bold\" foreground=\"white\" font_desc=\"18\">PQL II</span>");
  			sep = gtk_hseparator_new();
  			gtk_widget_modify_fg(sep, GTK_STATE_NORMAL, &white);
  			gtk_box_pack_start(GTK_BOX(widget), label, FALSE, FALSE, 3);
			gtk_box_pack_start(GTK_BOX(widget), sep, FALSE, FALSE, 7);
		}

		eBox = gtk_event_box_new();
		frame = gtk_frame_new("Trace Scale");
		box = gtk_vbox_new(FALSE, 0);
		gtk_container_add(GTK_CONTAINER(frame), box);
		gtk_container_add(GTK_CONTAINER(eBox), frame);
		gtk_box_pack_start(GTK_BOX(widget), eBox, FALSE, FALSE, 2);
		
		Mcbo[MAGTSCALE] = combo = gtk_combo_box_new_text();
		gtk_box_pack_start(GTK_BOX(box), combo, FALSE, FALSE, 2);
		strcpy(toolTip, "Set Individual Trace Display Amplitude Scale");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), combo, toolTip, toolTip);
		for(i=0;i<3;i++)
		{
			gtk_combo_box_append_text(GTK_COMBO_BOX(combo), tscales[i]);
		}
		gtk_combo_box_set_active(GTK_COMBO_BOX(combo), settings.magnify.Tamp);
		signals[TRACESCALESGNL][MAGSIGNAL+MAXSCRNSGNLS*0] = 
				g_signal_connect(combo, "changed", G_CALLBACK(pqlComboSel), GINT_TO_POINTER(MAGTSCALE));

		size_group2 = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
		MTfixedB[MAX] = hbox = gtk_hbox_new(FALSE, 0);
		label = gtk_label_new("max:");
		gtk_size_group_add_widget(size_group2, label);
		MTfixed[MAX] = gtk_entry_new();
		g_signal_connect(MTfixed[MAX], "activate", G_CALLBACK(reMag), GINT_TO_POINTER(MAGNIFYSCR));
		gtk_entry_set_width_chars(GTK_ENTRY(MTfixed[MAX]), 5);
		gtk_container_add(GTK_CONTAINER(hbox), label);
		gtk_container_add(GTK_CONTAINER(hbox), MTfixed[MAX]);
		gtk_container_add(GTK_CONTAINER(box), hbox);
		strcpy(toolTip, "MAX: define as absolute (e.g., 1000) or as Percentage of MAX/MIN Mean (e.g., 50%) (BLANK = Real MAX)");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), MTfixed[MAX], toolTip, toolTip);

		MTfixedB[MIN] = hbox = gtk_hbox_new(FALSE, 0);
		label = gtk_label_new("min:");
		gtk_size_group_add_widget(size_group2, label);
		MTfixed[MIN] = gtk_entry_new();
		g_signal_connect(MTfixed[MIN], "activate", G_CALLBACK(reMag), GINT_TO_POINTER(MAGNIFYSCR));
		gtk_entry_set_width_chars(GTK_ENTRY(MTfixed[MIN]), 5);
		gtk_container_add(GTK_CONTAINER(hbox), label);
		gtk_container_add(GTK_CONTAINER(hbox), MTfixed[MIN]);
		gtk_container_add(GTK_CONTAINER(box), hbox);
		strcpy(toolTip, "MIN: define as absolute (e.g., 1000) or as Percentage of MAX/MIN Mean (e.g., 50%) (BLANK = Real MIN)");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), MTfixed[MIN], toolTip, toolTip);
		
		eBox = gtk_event_box_new();
		frame = gtk_frame_new("Window Scale");
		box = gtk_vbox_new(FALSE, 0);
		gtk_container_add(GTK_CONTAINER(frame), box);
		gtk_container_add(GTK_CONTAINER(eBox), frame);
		gtk_box_pack_start(GTK_BOX(widget), eBox, FALSE, FALSE, 2);
		
		Mcbo[MAGWSCALE] = combo = gtk_combo_box_new_text();
		gtk_box_pack_start(GTK_BOX(box), combo, FALSE, FALSE, 2);
		strcpy(toolTip, "Set Global Trace Display Amplitude Scale");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), combo, toolTip, toolTip);
		for(i=0;i<TTLWSCALES;i++)
		{
			gtk_combo_box_append_text(GTK_COMBO_BOX(combo), wscales[i]);
		}
		gtk_combo_box_set_active(GTK_COMBO_BOX(combo), settings.magnify.Wamp);
		signals[WNDWSCALESGNL][MAGSIGNAL+MAXSCRNSGNLS*0] = 
				g_signal_connect(combo, "changed", G_CALLBACK(pqlComboSel), GINT_TO_POINTER(MAGWSCALE));

#if 0
		eBox = gtk_event_box_new();
		frame = gtk_frame_new("Display Units");
		box = gtk_vbox_new(FALSE, 0);
		gtk_container_add(GTK_CONTAINER(frame), box);
		gtk_container_add(GTK_CONTAINER(eBox), frame);
		gtk_box_pack_start(GTK_BOX(widget), eBox, FALSE, FALSE, 2);

		Mcbo[MAGDISPU] = combo = gtk_combo_box_new_text();
		gtk_box_pack_start(GTK_BOX(box), combo, FALSE, FALSE, 2);
		strcpy(toolTip, "Set Display Units");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), combo, toolTip, toolTip);
		for(i=0;i<TTLDISPUNITS;i++)
		{
			gtk_combo_box_append_text(GTK_COMBO_BOX(combo), dispunits[i]);
		}
//		gtk_combo_box_set_active(GTK_COMBO_BOX(combo), settings.trace.time);
		signals[DISPSGNL][MAGSIGNAL] = g_signal_connect(combo, "changed", G_CALLBACK(pqlComboSel), GINT_TO_POINTER(MAGDISPU));
#endif

		eBox = gtk_event_box_new();
		frame = gtk_frame_new("Overlay");
		box = gtk_hbox_new(FALSE, 0);
		Mover[ON] = radioB1 = gtk_radio_button_new_with_label(NULL, "On");
		gtk_container_add(GTK_CONTAINER(box), radioB1);
		Mover[OFF] = radioB = gtk_radio_button_new_with_label(
								gtk_radio_button_get_group((GtkRadioButton *) radioB1), "Off");
		gtk_container_add(GTK_CONTAINER(box), radioB);
		gtk_container_add(GTK_CONTAINER(frame), box);
		gtk_container_add(GTK_CONTAINER(eBox), frame);
		gtk_box_pack_start(GTK_BOX(widget), eBox, FALSE, FALSE, 2);
		strcpy(toolTip, "DISPLAY Traces in OVERLAY Mode");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), radioB1, toolTip, toolTip);
		strcpy(toolTip, "DISPLAY Traces Individually");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), radioB, toolTip, toolTip);

		sep = gtk_hseparator_new();
		gtk_widget_modify_fg(sep, GTK_STATE_NORMAL, &white);
		gtk_box_pack_start(GTK_BOX(widget), sep, FALSE, FALSE, 7);

		eBox = gtk_event_box_new();
		frame = gtk_frame_new("Analysis");
		box = gtk_vbox_new(FALSE, 0);
		gtk_container_add(GTK_CONTAINER(frame), box);
		gtk_container_add(GTK_CONTAINER(eBox), frame);
		gtk_box_pack_start(GTK_BOX(widget), eBox, FALSE, FALSE, 2);

		button = gtk_button_new_with_label("Original");
		strcpy(toolTip, "Remove ALL Applied Transforms and Return Trace to ORIGINAL State\nHot-Key = CTRL-z");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), button, toolTip, toolTip);
		gtk_box_pack_start(GTK_BOX(box), button, FALSE, FALSE, 2);
		g_signal_connect(button, "clicked", G_CALLBACK(pqlTFormSel), GINT_TO_POINTER(TRUE));

		pqlTForms[MAGTFORM] = combo = gtk_combo_box_new_text();
		gtk_box_pack_start(GTK_BOX(box), combo, FALSE, FALSE, 2);
		strcpy(toolTip, "Apply Transform Function to ALL Traces (None Selected) or ONLY Selected Traces");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), combo, toolTip, toolTip);

		for(i=0;i<TTLPQLANALYSES;i++)
		{
			gtk_combo_box_append_text(GTK_COMBO_BOX(combo), pqlTransforms[i]);
		}			
		gtk_combo_box_set_active(GTK_COMBO_BOX(combo), PQLTFORMS);
		g_signal_connect(combo, "changed", G_CALLBACK(pqlTFormSel),  GINT_TO_POINTER(FALSE));

		fltrCbos[MAGFILTERCBO] = combo = gtk_combo_box_new_text();
		strcpy(toolTip, "Apply Filter Function to ALL Traces (None Selected) or ONLY Selected Traces");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), combo, toolTip, toolTip);
		gtk_widget_set_size_request(combo, 10, -1);
		gtk_container_add(GTK_CONTAINER(box), combo);

		hbox = gtk_hbox_new(FALSE, 0);
		gtk_container_add(GTK_CONTAINER(box), hbox);
		MTForm[ON] = radioB1 = gtk_radio_button_new_with_label(NULL, "On");
		gtk_container_add(GTK_CONTAINER(hbox), radioB1);
		MTForm[OFF] = radioB = gtk_radio_button_new_with_label(
								gtk_radio_button_get_group((GtkRadioButton *) radioB1), "Off");
		gtk_container_add(GTK_CONTAINER(hbox), radioB);
		strcpy(toolTip, "Display Transform Data");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), radioB1, toolTip, toolTip);
		strcpy(toolTip, "Display Original Data");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), radioB, toolTip, toolTip);

		dispGainB[scr] = eBox = gtk_event_box_new();
		frame = gtk_frame_new("Display Gain");
		box = gtk_vbox_new(FALSE, 0);
		dispGain[scr] = entry = gtk_entry_new();
		gtk_widget_set_size_request(entry, BUTTONW, -1);
		gtk_container_add(GTK_CONTAINER(box), entry);
		gtk_container_add(GTK_CONTAINER(frame), box);
		gtk_container_add(GTK_CONTAINER(eBox), frame);
		gtk_box_pack_start(GTK_BOX(widget), eBox, FALSE, FALSE, 2);
		strcpy(toolTip, "Display Gain to be Applied\nReal Number > 0 (e.g., 3.14159)"); 
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), entry, toolTip, toolTip);

		if (!lowRes)
		{
	  		sep = gtk_hseparator_new();
  			gtk_widget_modify_fg(sep, GTK_STATE_NORMAL, &white);
			gtk_box_pack_start(GTK_BOX(widget), sep, FALSE, FALSE, 7);
		}

		Msnap = button = gtk_button_new_with_label("SNAP");
		g_signal_connect(button, "clicked", G_CALLBACK(snapMag), GINT_TO_POINTER(SNAP));
		strcpy(toolTip, "MOVE (SNAP) Vertical Line to ZOOM Anchor Point");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), button, toolTip, toolTip);
		gtk_box_pack_start(GTK_BOX(widget), button, FALSE, FALSE, 2);

		Mmag = button = gtk_button_new_with_label("MAGNIFY");
		g_signal_connect(button, "clicked", G_CALLBACK(snapMag), GINT_TO_POINTER(MAG));
		strcpy(toolTip, "MAGNIFY current Double Boundary Selection");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), button, toolTip, toolTip);
		gtk_box_pack_start(GTK_BOX(widget), button, FALSE, FALSE, 2);

		MZoomBox = eBox = gtk_event_box_new();
		frame = gtk_frame_new("Zoom Anchor");
		box = gtk_vbox_new(FALSE, 0);
		gtk_container_add(GTK_CONTAINER(frame), box);
		gtk_container_add(GTK_CONTAINER(eBox), frame);
		gtk_box_pack_start(GTK_BOX(widget), eBox, FALSE, FALSE, 2);

		Mcbo[MAGZANCHOR] = combo = gtk_combo_box_new_text();
		gtk_box_pack_start(GTK_BOX(box), combo, FALSE, FALSE, 2);
		strcpy(toolTip, "Set Zooming Anchor Point");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), combo, toolTip, toolTip);
		for(i=0;i<TOTMANCHORS;i++)
		{
			gtk_combo_box_append_text(GTK_COMBO_BOX(combo), zanchors[i]);
		}
		gtk_combo_box_set_active(GTK_COMBO_BOX(combo), settings.magnify.anchor);
		g_signal_connect(combo, "changed", G_CALLBACK(pqlComboSel), GINT_TO_POINTER(MAGZANCHOR));

		MPickBox = eBox = gtk_event_box_new();
		frame = gtk_frame_new("Picking");
		box = gtk_vbox_new(FALSE, 0);
		gtk_container_add(GTK_CONTAINER(frame), box);
		gtk_container_add(GTK_CONTAINER(eBox), frame);
		gtk_box_pack_start(GTK_BOX(widget), eBox, FALSE, FALSE, 2);

		Mcbo[MAGPICKING] = combo = gtk_combo_box_new_text();
		gtk_box_pack_start(GTK_BOX(box), combo, FALSE, FALSE, 2);
		strcpy(toolTip, "Set Picking Scope");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), combo, toolTip, toolTip);
		for(i=0;i<TTLWSCALES;i++)
		{
			gtk_combo_box_append_text(GTK_COMBO_BOX(combo), wscales[i]);
		}
		gtk_combo_box_set_active(GTK_COMBO_BOX(combo), settings.magnify.pick);
		g_signal_connect(combo, "changed", G_CALLBACK(pqlComboSel), GINT_TO_POINTER(MAGPICKING));

		button = gtk_button_new_with_label("Write Picks");
		g_signal_connect(button, "clicked", G_CALLBACK(writePicks), NULL);
		gtk_box_pack_start(GTK_BOX(widget), button, FALSE, FALSE, 2);
		strcpy(toolTip, "WRITE Defined Picks to Pick File (Defined via CONTROLS PANEL)");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), button, toolTip, toolTip);
		if (!lowRes)
		{
	  		sep = gtk_hseparator_new();
	  		gtk_widget_modify_fg(sep, GTK_STATE_NORMAL, &white);
			gtk_box_pack_start(GTK_BOX(widget), sep, FALSE, FALSE, 7);
		}

		button = gtk_button_new_with_label("Print");
 		gtk_box_pack_start(GTK_BOX(widget), button, FALSE, FALSE, 1);
  		g_signal_connect(button, "clicked", G_CALLBACK (print), GINT_TO_POINTER(MAGNIFYSCR));
		strcpy(toolTip, "Create POSTSCRIPT File Ready for Printing");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), button, toolTip, toolTip);
		if (!lowRes)
		{
			button = gtk_button_new_with_label("Controls");
	 		gtk_box_pack_start(GTK_BOX(widget), button, FALSE, FALSE, 1);
			g_signal_connect(button, "clicked", G_CALLBACK(raiseControls), GINT_TO_POINTER(MAGCTRL));
			strcpy(toolTip, "View CONTROLS Panel");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), button, toolTip, toolTip);
			button = gtk_button_new_with_label("QUIT");
	  		g_signal_connect(button, "clicked", G_CALLBACK (gtk_main_quit), NULL);
	 		gtk_box_pack_start(GTK_BOX(widget), button, FALSE, FALSE, 1);
			strcpy(toolTip, "EXIT PQL");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), button, toolTip, toolTip);
		}

		box = gtk_vbox_new(FALSE, 0);
		mouseBox = gtk_event_box_new();
		label = gtk_label_new("mouse tips");
		gtk_container_add(GTK_CONTAINER(mouseBox), label);
		gtk_box_pack_end(GTK_BOX(box), mouseBox, FALSE, FALSE, 0);
#ifndef PQL_ONLY
		strcpy(toolTip, "F-Keys:\n"\
						" F1 - Go To Trace Viewer::Spectra\n"\
						" F2 - Go To PSD Viewer\n"\
						" F3 - Go To STN Viewer\n"\
						"\nWithin PLOT Region:\n Vertical Line:\t\tCLICK\n ZOOM In:\t\t\t"\
						"CLICK+DRAG RIGHT\n\t\t\t\tMouse Wheel UP\n\t\t\t\tArrow Key UP"\
						"\n\t\t\t\tZ-Key + LINES\n ZOOM Out:\t\tCLICK+DRAG LEFT\n\t\t\t\t"\
						"Mouse Wheel DOWN\n\t\t\t\tArrow Key DOWN\n\t\t\t\tX-Key + LINES"\
						"\n\t(Adjust ZOOM Speed via Controls Panel)\n\n Data Point(s):\t\t"\
						"CTRL+CLICK\n Scan Data Points:\tCTRL+CLICK+DRAG\n Define Pick:\t\t"\
						"PICK# SHIFT+CLICK\n Define Pick Error:\tPICK# SHIFT+ALT+CLICK\n\n"\
						"Within LABEL Region:\n"\
						" Select Trace:\t\tCLICK\n"\
						" In Overlay Mode:\n  Bring Trace to TOP:\tCLICK"\
						"\n\t\t\t\tMouse Wheel\n\nWithin Top AXIS Region:\n Jump Left:\t\t"\
						"CLICK (left of selection)\n Jump Right:\t\tCLICK (right of selection)\n"\
						" Move to Point:\t\tSHIFT+CLICK\n Scroll Window:\t\tCLICK+DRAG "\
						"(on selection)\n\t\t\t\tMouse Wheel\n\t\t\t\tArrow Key LEFT/RIGHT\n");
#else
		strcpy(toolTip, "F-Keys:\n"\
						" F1 - Go To PQL::Spectra\n"\
						"\nWithin PLOT Region:\n Vertical Line:\t\tCLICK\n ZOOM In:\t\t\t"\
						"CLICK+DRAG RIGHT\n\t\t\t\tMouse Wheel UP\n\t\t\t\tArrow Key UP"\
						"\n\t\t\t\tZ-Key + LINES\n ZOOM Out:\t\tCLICK+DRAG LEFT\n\t\t\t\t"\
						"Mouse Wheel DOWN\n\t\t\t\tArrow Key DOWN\n\t\t\t\tX-Key + LINES"\
						"\n\t(Adjust ZOOM Speed via Controls Panel)\n\n Data Point(s):\t\t"\
						"CTRL+CLICK\n Scan Data Points:\tCTRL+CLICK+DRAG\n Define Pick:\t\t"\
						"PICK# SHIFT+CLICK\n Define Pick Error:\tPICK# SHIFT+ALT+CLICK\n\n"\
						"Within LABEL Region:\n"\
						" Select Trace:\t\tCLICK\n"\
						" In Overlay Mode:\n  Bring Trace to TOP:\tCLICK"\
						"\n\t\t\t\tMouse Wheel\n\nWithin Top AXIS Region:\n Jump Left:\t\t"\
						"CLICK (left of selection)\n Jump Right:\t\tCLICK (right of selection)\n"\
						" Move to Point:\t\tSHIFT+CLICK\n Scroll Window:\t\tCLICK+DRAG "\
						"(on selection)\n\t\t\t\tMouse Wheel\n\t\t\t\tArrow Key LEFT/RIGHT\n");
#endif
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), mouseBox, toolTip, toolTip);
		gtk_container_add(GTK_CONTAINER(widget), box);

  		// set some values
  		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (Mover[settings.magnify.overlay]), TRUE);		
  		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (MTForm[OFF]), TRUE);
		settings.general.display.gather.gainS[scr] = g_strdup_printf("%.2f",
					settings.general.display.gather.gain[scr]);
		gtk_entry_set_text(GTK_ENTRY(dispGain[scr]), settings.general.display.gather.gainS[scr]);

		// attach the signals, don't want do this before setting the values
		signals[FILTSGNL][MAGSIGNAL] = g_signal_connect(MTForm[OFF], "toggled", G_CALLBACK(tFormDisp), GINT_TO_POINTER(MAGNIFYSCR));
		signals[OVERLAYSGNL][MAGSIGNAL+MAXSCRNSGNLS*0] = g_signal_connect(Mover[OFF], "toggled", 
															G_CALLBACK(radioB_chg), &settings.magnify.overlay);
		signals[FILTCBOSGNL][MAGFILTERCBO] = g_signal_connect(fltrCbos[MAGFILTERCBO], 
												"changed", G_CALLBACK(fltrSelect), NULL);
		g_signal_connect(dispGain[scr],"activate", G_CALLBACK(reDraw), GINT_TO_POINTER(scr));
	break;

	case SPECTRASCR:
		scr = SPECTRASCR;
		
  		label = gtk_label_new(NULL);
  		gtk_label_set_markup(GTK_LABEL(label),
				(const gchar *) "<span weight=\"bold\" foreground=\"white\" font_desc=\"18\">PQL II</span>");
  		sep = gtk_hseparator_new();
  		gtk_widget_modify_fg(sep, GTK_STATE_NORMAL, &white);
  		gtk_box_pack_start(GTK_BOX(widget), label, FALSE, FALSE, 3);
		gtk_box_pack_start(GTK_BOX(widget), sep, FALSE, FALSE, 7);

		eBox = gtk_event_box_new();
		frame = gtk_frame_new("Window Scale");
		box = gtk_vbox_new(FALSE, 0);
		gtk_container_add(GTK_CONTAINER(frame), box);
		gtk_container_add(GTK_CONTAINER(eBox), frame);
		gtk_box_pack_start(GTK_BOX(widget), eBox, FALSE, FALSE, 2);

		Scbo[SPECWSCALE] = combo = gtk_combo_box_new_text();
		gtk_box_pack_start(GTK_BOX(box), combo, FALSE, FALSE, 2);
		strcpy(toolTip, "Set AMPLITUDE Scale");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), combo, toolTip, toolTip);
		for(i=0;i<TTLWSCALES;i++)
		{
			gtk_combo_box_append_text(GTK_COMBO_BOX(combo), wscales[i]);
		}
		gtk_combo_box_set_active(GTK_COMBO_BOX(combo), settings.spectra.display.amp);
		signals[WNDWSCALESGNL][SPECSIGNAL+MAXSCRNSGNLS*0] = 
				g_signal_connect(combo, "changed", G_CALLBACK(pqlComboSel), GINT_TO_POINTER(SPECWSCALE));

#if 0
		eBox = gtk_event_box_new();
		frame = gtk_frame_new("Display Units");
		box = gtk_vbox_new(FALSE, 0);
		gtk_container_add(GTK_CONTAINER(frame), box);
		gtk_container_add(GTK_CONTAINER(eBox), frame);
		gtk_box_pack_start(GTK_BOX(widget), eBox, FALSE, FALSE, 2);

		Scbo[SPECDISPU] = combo = gtk_combo_box_new_text();
		gtk_box_pack_start(GTK_BOX(box), combo, FALSE, FALSE, 2);
		strcpy(toolTip, "Set Display Units");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), combo, toolTip, toolTip);
		for(i=0;i<TTLDISPUNITS;i++)
		{
			gtk_combo_box_append_text(GTK_COMBO_BOX(combo), dispunits[i]);
		}
//		gtk_combo_box_set_active(GTK_COMBO_BOX(combo), settings.trace.time);
		signals[DISPSGNL][SPECSIGNAL] = g_signal_connect(combo, "changed", G_CALLBACK(pqlComboSel), GINT_TO_POINTER(SPECDISPU));
#endif

		eBox = gtk_event_box_new();
		frame = gtk_frame_new("Overlay");
		box = gtk_hbox_new(FALSE, 0);
		Sover[ON] = radioB1 = gtk_radio_button_new_with_label(NULL, "On");
		gtk_container_add(GTK_CONTAINER(box), radioB1);
		Sover[OFF] = radioB = gtk_radio_button_new_with_label(gtk_radio_button_get_group((GtkRadioButton *) radioB1), "Off");
		gtk_container_add(GTK_CONTAINER(box), radioB);
		gtk_container_add(GTK_CONTAINER(frame), box);
		gtk_container_add(GTK_CONTAINER(eBox), frame);
		gtk_box_pack_start(GTK_BOX(widget), eBox, FALSE, FALSE, 2);
		strcpy(toolTip, "DISPLAY Spectras in OVERLAY Mode");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), radioB1, toolTip, toolTip);
		strcpy(toolTip, "DISPLAY Spectras Individually");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), radioB, toolTip, toolTip);

		sep = gtk_hseparator_new();
		gtk_widget_modify_fg(sep, GTK_STATE_NORMAL, &white);
		gtk_box_pack_start(GTK_BOX(widget), sep, FALSE, FALSE, 7);

		eBox = gtk_event_box_new();
		frame = gtk_frame_new("Analysis");
		box = gtk_vbox_new(FALSE, 0);
		gtk_container_add(GTK_CONTAINER(frame), box);
		gtk_container_add(GTK_CONTAINER(eBox), frame);
		gtk_box_pack_start(GTK_BOX(widget), eBox, FALSE, FALSE, 2);

		button = gtk_button_new_with_label("Original");
		gtk_box_pack_start(GTK_BOX(box), button, FALSE, FALSE, 2);
		g_signal_connect(button, "clicked", G_CALLBACK(pqlTFormSel), GINT_TO_POINTER(TRUE));

		pqlTForms[SPECTFORM] = combo = gtk_combo_box_new_text();
		gtk_box_pack_start(GTK_BOX(box), combo, FALSE, FALSE, 2);
		strcpy(toolTip, "Apply Transform Function to Traces");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), combo, toolTip, toolTip);

		for(i=0;i<TTLPQLANALYSES;i++)
		{
			gtk_combo_box_append_text(GTK_COMBO_BOX(combo), pqlTransforms[i]);
		}			
		gtk_combo_box_set_active(GTK_COMBO_BOX(combo), PQLTFORMS);
		g_signal_connect(combo, "changed", G_CALLBACK(pqlTFormSel),  GINT_TO_POINTER(FALSE));

		fltrCbos[SPECFILTERCBO] = combo = gtk_combo_box_new_text();
		gtk_widget_set_size_request(combo, 10, -1);
		gtk_container_add(GTK_CONTAINER(box), combo);

		hbox = gtk_hbox_new(FALSE, 0);
		gtk_container_add(GTK_CONTAINER(box), hbox);
		STForm[ON] = radioB1 = gtk_radio_button_new_with_label(NULL, "On");
		gtk_container_add(GTK_CONTAINER(hbox), radioB1);
		STForm[OFF] = radioB = gtk_radio_button_new_with_label(
								gtk_radio_button_get_group((GtkRadioButton *) radioB1), "Off");
		gtk_container_add(GTK_CONTAINER(hbox), radioB);
		strcpy(toolTip, "Display Transform Data");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), radioB1, toolTip, toolTip);
		strcpy(toolTip, "Display Original Data");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), radioB, toolTip, toolTip);

		dispGainB[scr] = eBox = gtk_event_box_new();
		frame = gtk_frame_new("Display Gain");
		box = gtk_vbox_new(FALSE, 0);
		dispGain[scr] = entry = gtk_entry_new();
		gtk_container_add(GTK_CONTAINER(box), entry);
		gtk_container_add(GTK_CONTAINER(frame), box);
		gtk_container_add(GTK_CONTAINER(eBox), frame);
		gtk_box_pack_start(GTK_BOX(widget), eBox, FALSE, FALSE, 2);
		strcpy(toolTip, "Display Gain to be Applied\nReal Number > 0 (e.g., 3.14159)"); 
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), entry, toolTip, toolTip);
		gtk_widget_set_size_request(entry, BUTTONW, -1);

  		sep = gtk_hseparator_new();
  		gtk_widget_modify_fg(sep, GTK_STATE_NORMAL, &white);
		gtk_box_pack_start(GTK_BOX(widget), sep, FALSE, FALSE, 7);

		button = gtk_button_new_with_label("Print");
 		gtk_box_pack_start(GTK_BOX(widget), button, FALSE, FALSE, 1);
  		g_signal_connect(button, "clicked", G_CALLBACK (print), GINT_TO_POINTER(SPECTRASCR));
		strcpy(toolTip, "Create POSTSCRIPT File Ready for Printing");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), button, toolTip, toolTip);
		button = gtk_button_new_with_label("Controls");
 		gtk_box_pack_start(GTK_BOX(widget), button, FALSE, FALSE, 1);
		g_signal_connect(button, "clicked", G_CALLBACK(raiseControls), GINT_TO_POINTER(SPECCTRL));
		strcpy(toolTip, "View CONTROLS Panel");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), button, toolTip, toolTip);
		button = gtk_button_new_with_label("QUIT");
  		g_signal_connect(button, "clicked", G_CALLBACK (gtk_main_quit), NULL);
 		gtk_box_pack_start(GTK_BOX(widget), button, FALSE, FALSE, 1);
		strcpy(toolTip, "EXIT PQL");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), button, toolTip, toolTip);

		box = gtk_vbox_new(FALSE, 0);
		mouseBox = gtk_event_box_new();
		label = gtk_label_new("mouse tips");
		gtk_container_add(GTK_CONTAINER(mouseBox), label);
		gtk_box_pack_end(GTK_BOX(box), mouseBox, FALSE, FALSE, 0);
#ifndef PQL_ONLY
		strcpy(toolTip, "F-Keys:\n"\
						" F1 - Go To Trace Viewer::Split\n"\
						" F2 - Go To PSD Viewer\n"\
						" F3 - Go To STN Viewer\n\n"\
						"Within PLOT Region:\n"
						" Frequency Cutoff:\tCLICK\n"
						" Data Point:\t\tCTRL+CLICK\n"
						" Scan Data Points:\tCTRL+CLICK+DRAG\n"
						" Re-Execute Transformation: SHIFT+CLICK\n\n"
						"On LABEL (in Overlay Mode):\n"
						" Bring Trace to TOP:\tCLICK\n");
#else
		strcpy(toolTip, "F-Keys:\n"\
						" F1 - Go To PQL::Split\n"\
						"Within PLOT Region:\n"
						" Frequency Cutoff:\tCLICK\n"
						" Data Point:\t\tCTRL+CLICK\n"
						" Scan Data Points:\tCTRL+CLICK+DRAG\n"
						" Re-Execute Transformation: SHIFT+CLICK\n\n"
						"On LABEL (in Overlay Mode):\n"
						" Bring Trace to TOP:\tCLICK\n");
#endif
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), mouseBox, toolTip, toolTip);
		gtk_container_add(GTK_CONTAINER(widget), box);

		// set the values 
  		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (Sover[settings.spectra.display.overlay]), TRUE);		
  		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (STForm[OFF]), TRUE);	
		settings.general.display.gather.gainS[scr] = g_strdup_printf("%.2f",
					settings.general.display.gather.gain[scr]);
		gtk_entry_set_text(GTK_ENTRY(dispGain[scr]), settings.general.display.gather.gainS[scr]);

		// attach the signals, can't do this before setting the values
		signals[OVERLAYSGNL][SPECSIGNAL+MAXSCRNSGNLS*0] = g_signal_connect(Sover[OFF], "toggled", 
													G_CALLBACK(radioB_chg), &settings.spectra.display.overlay);
		signals[FILTSGNL][SPECSIGNAL] = g_signal_connect(STForm[OFF], "toggled", 
													G_CALLBACK(tFormDisp), GINT_TO_POINTER(SPECTRASCR));		
		signals[FILTCBOSGNL][SPECFILTERCBO] = g_signal_connect(fltrCbos[SPECFILTERCBO], "changed", 
													G_CALLBACK(fltrSelect), NULL);
		g_signal_connect(dispGain[scr],"activate", G_CALLBACK(reDraw), GINT_TO_POINTER(scr));
	break;

	case SPLITSCR:
		gettimeofday(&tv, &tz);
		srand(tv.tv_usec);	// randomly select the background color
		scr = SPLITSCR;
		
  		label = gtk_label_new(NULL);
  		gtk_label_set_markup(GTK_LABEL(label),
				(const gchar *) "<span weight=\"bold\" foreground=\"white\" font_desc=\"18\">PQL II</span>");
  		sep = gtk_hseparator_new();
  		gtk_widget_modify_fg(sep, GTK_STATE_NORMAL, &white);
  		gtk_box_pack_start(GTK_BOX(widget), label, FALSE, FALSE, 3);
		gtk_box_pack_start(GTK_BOX(widget), sep, FALSE, FALSE, 7);

		HsplitBox = eBox = gtk_event_box_new();
		frame = gtk_frame_new("Screens");
		vbox = gtk_vbox_new(FALSE, 0);
		box = gtk_hbox_new(FALSE, 0);
		gtk_container_add(GTK_CONTAINER(vbox), box);
		gtk_container_add(GTK_CONTAINER(frame), vbox);
		gtk_container_add(GTK_CONTAINER(eBox), frame);
		gtk_box_pack_start(GTK_BOX(widget), eBox, FALSE, FALSE, 0);

		Hsplit[SPLIT2] = radioB1 = gtk_radio_button_new_with_label(NULL, "2");
		gtk_box_pack_start(GTK_BOX(box), radioB1, FALSE, FALSE, 10);
		Hsplit[SPLIT3] = radioB2 = gtk_radio_button_new_with_label(gtk_radio_button_get_group((GtkRadioButton *) radioB1), "3");
		gtk_box_pack_start(GTK_BOX(box), radioB2, FALSE, FALSE, 10);
		strcpy(toolTip, "DISPLAY 2 Split Screens");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), radioB1, toolTip, toolTip);
		strcpy(toolTip, "DISPLAY 3 Split Screens");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), radioB2, toolTip, toolTip);

		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (Hsplit[settings.split.pane]), TRUE);
		g_signal_connect(Hsplit[SPLIT3], "toggled", G_CALLBACK(newSplit), &states.split.panes);
		g_signal_connect(Hsplit[SPLIT2], "toggled", G_CALLBACK(newSplit), &states.split.panes);

		frame1 = gtk_frame_new("TOP");
		box = gtk_vbox_new(FALSE, 0);
		gtk_container_add(GTK_CONTAINER(frame1), box);
		gtk_container_add(GTK_CONTAINER(vbox), frame1);
		Hcbo[SPLITTOP] = combo = gtk_combo_box_new_text();
		gtk_box_pack_start(GTK_BOX(box), combo, FALSE, FALSE, 2);
		strcpy(toolTip, "Set TOP Display");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), combo, toolTip, toolTip);
		for(i=0;i<TTLHDTYPES;i++)
		{
			gtk_combo_box_append_text(GTK_COMBO_BOX(combo), hdtypes[i]);
		}
		gtk_combo_box_set_active(GTK_COMBO_BOX(combo), settings.split.DA[settings.split.pane][TOP]);
		g_signal_connect(combo, "changed", G_CALLBACK(pqlComboSel), GINT_TO_POINTER(SPLITTOP));

		HMidFrame = frame1 = gtk_frame_new("MIDDLE");
		box = gtk_vbox_new(FALSE, 0);
		gtk_container_add(GTK_CONTAINER(frame1), box);
		gtk_container_add(GTK_CONTAINER(vbox), frame1);
		Hcbo[SPLITMID] = combo = gtk_combo_box_new_text();
		gtk_box_pack_start(GTK_BOX(box), combo, FALSE, FALSE, 2);
		strcpy(toolTip, "Set TOP Display");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), combo, toolTip, toolTip);
		for(i=0;i<TTLHDTYPES;i++)
		{
			gtk_combo_box_append_text(GTK_COMBO_BOX(combo), hdtypes[i]);
		}
		gtk_combo_box_set_active(GTK_COMBO_BOX(combo), settings.split.DA[SPLIT3][MID]);
		g_signal_connect(combo, "changed", G_CALLBACK(pqlComboSel), GINT_TO_POINTER(SPLITMID));

		frame1 = gtk_frame_new("BOTTOM");
		box = gtk_vbox_new(FALSE, 0);
		gtk_container_add(GTK_CONTAINER(frame1), box);
		gtk_container_add(GTK_CONTAINER(vbox), frame1);
		Hcbo[SPLITBOT] = combo = gtk_combo_box_new_text();
		gtk_box_pack_start(GTK_BOX(box), combo, FALSE, FALSE, 2);
		strcpy(toolTip, "Set TOP Display");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), combo, toolTip, toolTip);
		for(i=0;i<TTLHDTYPES;i++)
		{
			gtk_combo_box_append_text(GTK_COMBO_BOX(combo), hdtypes[i]);
		}
		gtk_combo_box_set_active(GTK_COMBO_BOX(combo), settings.split.DA[settings.split.pane][BOT]);
		g_signal_connect(combo, "changed", G_CALLBACK(pqlComboSel), GINT_TO_POINTER(SPLITBOT));

		sep = gtk_hseparator_new();
  		gtk_widget_modify_fg(sep, GTK_STATE_NORMAL, &white);
		gtk_box_pack_start(GTK_BOX(widget), sep, FALSE, FALSE, 7);

		eBox = gtk_event_box_new();
		frame = gtk_frame_new("TRACE Display");
		box = gtk_vbox_new(FALSE, 0);
		gtk_container_add(GTK_CONTAINER(frame), box);
		gtk_container_add(GTK_CONTAINER(eBox), frame);
		gtk_box_pack_start(GTK_BOX(widget), eBox, FALSE, FALSE, 2);
		
		combo = gtk_combo_box_new_text();
		gtk_box_pack_start(GTK_BOX(box), combo, FALSE, FALSE, 2);
		strcpy(toolTip, "Define How TRACE Data is Displayed");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), combo, toolTip, toolTip);
		gtk_combo_box_append_text(GTK_COMBO_BOX(combo), "Original");
		gtk_combo_box_append_text(GTK_COMBO_BOX(combo), "Transformed");
		gtk_combo_box_set_active(GTK_COMBO_BOX(combo), settings.split.traceDisp);
		g_signal_connect(combo, "changed", G_CALLBACK(pqlComboSel), GINT_TO_POINTER(SPLITTDISP));

		eBox = gtk_event_box_new();
		frame = gtk_frame_new("Trace Scale");
		box = gtk_vbox_new(FALSE, 0);
		gtk_container_add(GTK_CONTAINER(frame), box);
		gtk_container_add(GTK_CONTAINER(eBox), frame);
		gtk_box_pack_start(GTK_BOX(widget), eBox, FALSE, FALSE, 2);
		
		Hcbo[SPLITTSCALE] = combo = gtk_combo_box_new_text();
		gtk_box_pack_start(GTK_BOX(box), combo, FALSE, FALSE, 2);
		strcpy(toolTip, "Set Individual Trace Display Amplitude Scale");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), combo, toolTip, toolTip);
		for(i=0;i<3;i++)
		{
			gtk_combo_box_append_text(GTK_COMBO_BOX(combo), tscales[i]);
		}
		gtk_combo_box_set_active(GTK_COMBO_BOX(combo), settings.magnify.Tamp);
		signals[TRACESCALESGNL][SPLITSIGNAL+MAXSCRNSGNLS*0] = 
				g_signal_connect(combo, "changed", G_CALLBACK(pqlComboSel), GINT_TO_POINTER(SPLITTSCALE));

		size_group2 = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
		HTfixedB[MAX] = hbox = gtk_hbox_new(FALSE, 0);
		label = gtk_label_new("max:");
		gtk_size_group_add_widget(size_group2, label);
		HTfixed[MAX] = entry = gtk_entry_new();
		g_signal_connect(entry, "activate", G_CALLBACK(reMag), GINT_TO_POINTER(SPLITSCR));
		gtk_entry_set_width_chars(GTK_ENTRY(entry), 5);
		gtk_container_add(GTK_CONTAINER(hbox), label);
		gtk_container_add(GTK_CONTAINER(hbox), entry);
		gtk_container_add(GTK_CONTAINER(box), hbox);
		strcpy(toolTip, "MAX: define as absolute (e.g., 1000) or as Percentage of MAX/MIN Mean (e.g., 50%) (BLANK = Real MAX)");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), entry, toolTip, toolTip);

		HTfixedB[MIN] = hbox = gtk_hbox_new(FALSE, 0);
		label = gtk_label_new("min:");
		gtk_size_group_add_widget(size_group2, label);
		HTfixed[MIN] = entry = gtk_entry_new();
		g_signal_connect(entry, "activate", G_CALLBACK(reMag), GINT_TO_POINTER(SPLITSCR));
		gtk_entry_set_width_chars(GTK_ENTRY(entry), 5);
		gtk_container_add(GTK_CONTAINER(hbox), label);
		gtk_container_add(GTK_CONTAINER(hbox), entry);
		gtk_container_add(GTK_CONTAINER(box), hbox);
		strcpy(toolTip, "MIN: define as absolute (e.g., 1000) or as Percentage of MAX/MIN Mean (e.g., 50%) (BLANK = Real MIN)");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), entry, toolTip, toolTip);
		
		eBox = gtk_event_box_new();
		frame = gtk_frame_new("Window Scale");
		box = gtk_vbox_new(FALSE, 0);
		gtk_container_add(GTK_CONTAINER(frame), box);
		gtk_container_add(GTK_CONTAINER(eBox), frame);
		gtk_box_pack_start(GTK_BOX(widget), eBox, FALSE, FALSE, 2);
		
		Hcbo[SPLITWSCALE] = combo = gtk_combo_box_new_text();
		gtk_box_pack_start(GTK_BOX(box), combo, FALSE, FALSE, 2);
		strcpy(toolTip, "Set Global Trace Display Amplitude Scale");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), combo, toolTip, toolTip);
		for(i=0;i<TTLWSCALES;i++)
		{
			gtk_combo_box_append_text(GTK_COMBO_BOX(combo), wscales[i]);
		}
		gtk_combo_box_set_active(GTK_COMBO_BOX(combo), settings.magnify.Wamp);
		signals[WNDWSCALESGNL][SPLITSIGNAL+MAXSCRNSGNLS*0] = 
				g_signal_connect(combo, "changed", G_CALLBACK(pqlComboSel), GINT_TO_POINTER(SPLITWSCALE));

		eBox = gtk_event_box_new();
		frame = gtk_frame_new("Overlay");
		box = gtk_hbox_new(FALSE, 0);
		Hover[ON] = radioB1 = gtk_radio_button_new_with_label(NULL, "On");
		gtk_container_add(GTK_CONTAINER(box), radioB1);
		Hover[OFF] = radioB = gtk_radio_button_new_with_label(
								gtk_radio_button_get_group((GtkRadioButton *) radioB1), "Off");
		gtk_container_add(GTK_CONTAINER(box), radioB);
		gtk_container_add(GTK_CONTAINER(frame), box);
		gtk_container_add(GTK_CONTAINER(eBox), frame);
		gtk_box_pack_start(GTK_BOX(widget), eBox, FALSE, FALSE, 2);
		strcpy(toolTip, "DISPLAY Traces in OVERLAY Mode");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), radioB1, toolTip, toolTip);
		strcpy(toolTip, "DISPLAY Traces Individually");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), radioB, toolTip, toolTip);

		sep = gtk_hseparator_new();
		gtk_widget_modify_fg(sep, GTK_STATE_NORMAL, &white);
		gtk_box_pack_start(GTK_BOX(widget), sep, FALSE, FALSE, 7);

		eBox = gtk_event_box_new();
		frame = gtk_frame_new("Analysis");
		box = gtk_vbox_new(FALSE, 0);
		gtk_container_add(GTK_CONTAINER(frame), box);
		gtk_container_add(GTK_CONTAINER(eBox), frame);
		gtk_box_pack_start(GTK_BOX(widget), eBox, FALSE, FALSE, 2);

		button = gtk_button_new_with_label("Original");
		gtk_box_pack_start(GTK_BOX(box), button, FALSE, FALSE, 2);
		g_signal_connect(button, "clicked", G_CALLBACK(pqlTFormSel), GINT_TO_POINTER(TRUE));

		pqlTForms[SPLITTFORM] = combo = gtk_combo_box_new_text();
		gtk_box_pack_start(GTK_BOX(box), combo, FALSE, FALSE, 2);
		strcpy(toolTip, "Apply Transform Function to Traces");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), combo, toolTip, toolTip);

		for(i=0;i<TTLPQLANALYSES;i++)
		{
			gtk_combo_box_append_text(GTK_COMBO_BOX(combo), pqlTransforms[i]);
		}			
		gtk_combo_box_set_active(GTK_COMBO_BOX(combo), PQLTFORMS);
		g_signal_connect(combo, "changed", G_CALLBACK(pqlTFormSel),  GINT_TO_POINTER(FALSE));

		fltrCbos[SPLTFILTERCBO] = combo = gtk_combo_box_new_text();
		gtk_widget_set_size_request(combo, 10, -1);
		gtk_container_add(GTK_CONTAINER(box), combo);

		hbox = gtk_hbox_new(FALSE, 0);
		gtk_container_add(GTK_CONTAINER(box), hbox);
		HTForm[ON] = radioB1 = gtk_radio_button_new_with_label(NULL, "On");
		gtk_container_add(GTK_CONTAINER(hbox), radioB1);
		HTForm[OFF] = radioB = gtk_radio_button_new_with_label(
								gtk_radio_button_get_group((GtkRadioButton *) radioB1), "Off");
		gtk_container_add(GTK_CONTAINER(hbox), radioB);
		strcpy(toolTip, "Display Transform Data");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), radioB1, toolTip, toolTip);
		strcpy(toolTip, "Display Original Data");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), radioB, toolTip, toolTip);

		sep = gtk_hseparator_new();
		gtk_widget_modify_fg(sep, GTK_STATE_NORMAL, &white);
		gtk_box_pack_start(GTK_BOX(widget), sep, FALSE, FALSE, 7);

		Hsnap = button = gtk_button_new_with_label("SNAP");
		g_signal_connect(button, "clicked", G_CALLBACK(snapMag), GINT_TO_POINTER(SNAP));
 		gtk_box_pack_start(GTK_BOX(widget), button, FALSE, FALSE, 1);
		strcpy(toolTip, "MOVE (SNAP) Vertical Line to ZOOM Anchor Point");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), button, toolTip, toolTip);

		Hmag = button = gtk_button_new_with_label("MAGNIFY");
		g_signal_connect(button, "clicked", G_CALLBACK(snapMag), GINT_TO_POINTER(MAG));
 		gtk_box_pack_start(GTK_BOX(widget), button, FALSE, FALSE, 1);
		strcpy(toolTip, "MAGNIFY current Double Boundary Selection");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), button, toolTip, toolTip);

		button = gtk_button_new_with_label("Write Picks");
		g_signal_connect(button, "clicked", G_CALLBACK(writePicks), NULL);
 		gtk_box_pack_start(GTK_BOX(widget), button, FALSE, FALSE, 1);
		strcpy(toolTip, "WRITE Defined Picks to Pick File (Defined via CONTROLS PANEL)");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), button, toolTip, toolTip);

  		sep = gtk_hseparator_new();
  		gtk_widget_modify_fg(sep, GTK_STATE_NORMAL, &white);
		gtk_box_pack_start(GTK_BOX(widget), sep, FALSE, FALSE, 7);

		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), button, toolTip, toolTip);
		button = gtk_button_new_with_label("Print");
 		gtk_box_pack_start(GTK_BOX(widget), button, FALSE, FALSE, 1);
  		g_signal_connect(button, "clicked", G_CALLBACK (print), GINT_TO_POINTER(SPLITSCR));
		strcpy(toolTip, "Create POSTSCRIPT File Ready for Printing");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), button, toolTip, toolTip);
		button = gtk_button_new_with_label("Controls");
 		gtk_box_pack_start(GTK_BOX(widget), button, FALSE, FALSE, 1);
		g_signal_connect(button, "clicked", G_CALLBACK(raiseControls), GINT_TO_POINTER(SPLITCTRL));
		strcpy(toolTip, "View CONTROLS Panel");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), button, toolTip, toolTip);
		button = gtk_button_new_with_label("QUIT");
  		g_signal_connect(button, "clicked", G_CALLBACK (gtk_main_quit), NULL);
 		gtk_box_pack_start(GTK_BOX(widget), button, FALSE, FALSE, 1);
		strcpy(toolTip, "EXIT PQL");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), button, toolTip, toolTip);

  		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (Hover[settings.magnify.overlay]), TRUE);		
  		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (HTForm[OFF]), TRUE);

		signals[FILTSGNL][SPLITSIGNAL] = g_signal_connect(HTForm[OFF], "toggled", 
												G_CALLBACK(tFormDisp), GINT_TO_POINTER(SPLITSCR));
		signals[FILTCBOSGNL][SPLTFILTERCBO] = g_signal_connect(fltrCbos[SPLTFILTERCBO], 
												"changed", G_CALLBACK(fltrSelect), NULL);
		signals[OVERLAYSGNL][SPLITSIGNAL+MAXSCRNSGNLS*0] = g_signal_connect(Hover[OFF], "toggled", 
															G_CALLBACK(radioB_chg), &settings.magnify.overlay);
	break;

	case HEADERSCR:
		gettimeofday(&tv, &tz);
		srand(tv.tv_usec);	// randomly select the sidebar color
		scr = (int) ((float) NUMDATASCREENS * rand()/(RAND_MAX+1.0));
		
  		label = gtk_label_new(NULL);
  		gtk_label_set_markup(GTK_LABEL(label),
				(const gchar *) "<span weight=\"bold\" foreground=\"white\" font_desc=\"18\">PQL II</span>");
  		sep = gtk_hseparator_new();
  		gtk_widget_modify_fg(sep, GTK_STATE_NORMAL, &white);
  		gtk_box_pack_start(GTK_BOX(widget), label, FALSE, FALSE, 3);
		gtk_box_pack_start(GTK_BOX(widget), sep, FALSE, FALSE, 7);

		actionB = gtk_vbutton_box_new();
  		gtk_box_pack_start(GTK_BOX(widget), actionB, FALSE, FALSE, 0);

		gtk_button_box_set_layout(GTK_BUTTON_BOX(actionB), GTK_BUTTONBOX_START);
		gtk_box_set_spacing(GTK_BOX(actionB), 2);

		eBox = gtk_event_box_new();
		frame = gtk_frame_new("On Display");
		box = gtk_vbox_new(FALSE, 0);
		HDRdisp[ALL] = radioB1 = gtk_radio_button_new_with_label(NULL, "All");
		gtk_container_add(GTK_CONTAINER(box), radioB1);
		g_signal_connect(radioB1, "toggled", G_CALLBACK(hdr_chg), &settings.headers.display);
		HDRdisp[DISPLAYED] = radioB = gtk_radio_button_new_with_label(
								gtk_radio_button_get_group((GtkRadioButton *) radioB1), "Displayed");
		gtk_container_add(GTK_CONTAINER(box), radioB);
		g_signal_connect(radioB, "toggled", G_CALLBACK(hdr_chg), &settings.headers.display);
		HDRdisp[SELECTED] = radioB = gtk_radio_button_new_with_label(
								gtk_radio_button_get_group((GtkRadioButton *) radioB1), "Selected");
		g_signal_connect(radioB, "toggled", G_CALLBACK(hdr_chg), &settings.headers.display);
		gtk_container_add(GTK_CONTAINER(box), radioB);
		gtk_container_add(GTK_CONTAINER(frame), box);
		gtk_container_add(GTK_CONTAINER(eBox), frame);
		gtk_box_pack_start(GTK_BOX(widget), eBox, FALSE, FALSE, 2);
		strcpy(toolTip, "DISPLAY Header Information of ALL, DISPLAYED, or SELECTED Traces");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), eBox, toolTip, toolTip);

  		sep = gtk_hseparator_new();
  		gtk_widget_modify_fg(sep, GTK_STATE_NORMAL, &white);
		gtk_box_pack_start(GTK_BOX(widget), sep, FALSE, FALSE, 7);

		button = gtk_button_new_with_label("Controls");
 		gtk_box_pack_start(GTK_BOX(widget), button, FALSE, FALSE, 1);
		g_signal_connect(button, "clicked", G_CALLBACK(raiseControls), GINT_TO_POINTER(HEADERCTRL));
		strcpy(toolTip, "View CONTROLS Panel");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), button, toolTip, toolTip);
		button = gtk_button_new_with_label("QUIT");
  		g_signal_connect(button, "clicked", G_CALLBACK (gtk_main_quit), NULL);
 		gtk_box_pack_start(GTK_BOX(widget), button, FALSE, FALSE, 1);
		strcpy(toolTip, "EXIT PQL");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), button, toolTip, toolTip);

		box = gtk_vbox_new(FALSE, 0);
		mouseBox = gtk_event_box_new();
		label = gtk_label_new("mouse tips");
		gtk_container_add(GTK_CONTAINER(mouseBox), label);
		gtk_box_pack_end(GTK_BOX(box), mouseBox, FALSE, FALSE, 0);
#ifndef PQL_ONLY
		strcpy(toolTip, "F-Keys:\n"\
						" F1 - Go To Trace Viewer::Trace or LOGS\n"\
						" F2 - Go To PSD Viewer\n"\
						" F3 - Go To STN Viewer\n"\
						"\nPlot Trace as 1st Record on TRACE Screen:\tDOUBLE-CLICK on Row");
#else
		strcpy(toolTip, "F-Keys:\n"\
						" F1 - Go To PQL::MAIN\n"\
						"\nPlot Trace as 1st Record on TRACE Screen:\tDOUBLE-CLICK on Row");
#endif
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), mouseBox, toolTip, toolTip);
		gtk_container_add(GTK_CONTAINER(widget), box);

		g_object_unref(size_group);

		break;
		
	case LOGSCR:
		gettimeofday(&tv, &tz);
		srand(tv.tv_usec);	// randomly select the sidebar color
		logScr = scr = (int) ((float) NUMDATASCREENS * rand()/(RAND_MAX+1.0));
		
  		label = gtk_label_new(NULL);
  		gtk_label_set_markup(GTK_LABEL(label),
				(const gchar *) "<span weight=\"bold\" foreground=\"white\" font_desc=\"18\">PQL II</span>");
  		sep = gtk_hseparator_new();
  		gtk_widget_modify_fg(sep, GTK_STATE_NORMAL, &white);
  		gtk_box_pack_start(GTK_BOX(widget), label, FALSE, FALSE, 3);
		gtk_box_pack_start(GTK_BOX(widget), sep, FALSE, FALSE, 7);

		actionB = gtk_vbutton_box_new();
  		gtk_box_pack_start(GTK_BOX(widget), actionB, FALSE, FALSE, 0);

		gtk_button_box_set_layout(GTK_BUTTON_BOX(actionB), GTK_BUTTONBOX_START);
		gtk_box_set_spacing(GTK_BOX(actionB), 2);

		eBox = gtk_event_box_new();
		frame = gtk_frame_new("Display By");
		box = gtk_vbox_new(FALSE, 0);
		LOGdispT[LOGFILE] = radioB1 = gtk_radio_button_new_with_label(NULL, "File");
		gtk_container_add(GTK_CONTAINER(box), radioB1);
		g_signal_connect(radioB1, "toggled", G_CALLBACK(log_chg), GINT_TO_POINTER(LOGDISPBY));
		LOGdispT[LOGTRC] = radioB = gtk_radio_button_new_with_label(
								gtk_radio_button_get_group((GtkRadioButton *) radioB1), "Trace");
		gtk_container_add(GTK_CONTAINER(box), radioB);
		g_signal_connect(radioB, "toggled", G_CALLBACK(log_chg), GINT_TO_POINTER(LOGDISPBY));
		gtk_container_add(GTK_CONTAINER(frame), box);
		gtk_container_add(GTK_CONTAINER(eBox), frame);
		gtk_box_pack_start(GTK_BOX(widget), eBox, FALSE, FALSE, 2);
		strcpy(toolTip, "DISPLAY LOG Information by Disk File or Trace");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), eBox, toolTip, toolTip);

		eBox = gtk_event_box_new();
		frame = gtk_frame_new("On Display");
		box = gtk_vbox_new(FALSE, 0);
		LOGdisp[ALL] = radioB1 = gtk_radio_button_new_with_label(NULL, "All");
		gtk_container_add(GTK_CONTAINER(box), radioB1);
		g_signal_connect(radioB1, "toggled", G_CALLBACK(log_chg), GINT_TO_POINTER(LOGONDISP));
		LOGdisp[DISPLAYED] = radioB = gtk_radio_button_new_with_label(
								gtk_radio_button_get_group((GtkRadioButton *) radioB1), "Displayed");
		gtk_container_add(GTK_CONTAINER(box), radioB);
		g_signal_connect(radioB, "toggled", G_CALLBACK(log_chg), GINT_TO_POINTER(LOGONDISP));
		LOGdisp[SELECTED] = radioB = gtk_radio_button_new_with_label(
								gtk_radio_button_get_group((GtkRadioButton *) radioB1), "Selected");
		g_signal_connect(radioB, "toggled", G_CALLBACK(log_chg), GINT_TO_POINTER(LOGONDISP));
		gtk_container_add(GTK_CONTAINER(box), radioB);
		gtk_container_add(GTK_CONTAINER(frame), box);
		gtk_container_add(GTK_CONTAINER(eBox), frame);
		gtk_box_pack_start(GTK_BOX(widget), eBox, FALSE, FALSE, 2);
		strcpy(toolTip, "DISPLAY LOG Information of ALL, DISPLAYED, or SELECTED Traces");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), eBox, toolTip, toolTip);

  		sep = gtk_hseparator_new();
  		gtk_widget_modify_fg(sep, GTK_STATE_NORMAL, &white);
		gtk_box_pack_start(GTK_BOX(widget), sep, FALSE, FALSE, 7);

		button = gtk_button_new_with_label("Controls");
 		gtk_box_pack_start(GTK_BOX(widget), button, FALSE, FALSE, 1);
		g_signal_connect(button, "clicked", G_CALLBACK(raiseControls), GINT_TO_POINTER(HEADERCTRL));
		strcpy(toolTip, "View CONTROLS Panel");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), button, toolTip, toolTip);
		button = gtk_button_new_with_label("QUIT");
  		g_signal_connect(button, "clicked", G_CALLBACK (gtk_main_quit), NULL);
 		gtk_box_pack_start(GTK_BOX(widget), button, FALSE, FALSE, 1);
		strcpy(toolTip, "EXIT PQL");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), button, toolTip, toolTip);

		box = gtk_vbox_new(FALSE, 0);
		mouseBox = gtk_event_box_new();
		label = gtk_label_new("mouse tips");
		gtk_container_add(GTK_CONTAINER(mouseBox), label);
		gtk_box_pack_end(GTK_BOX(box), mouseBox, FALSE, FALSE, 0);
#ifndef PQL_ONLY
		strcpy(toolTip, "F-Keys:\n"\
						" F1 - Go To Trace Viewer::Trace\n"\
						" F2 - Go To PSD Viewer\n"\
						" F3 - Go To STN Viewer\n"\
						"\nDisplay LOGS:\tDOUBLE-CLICK on Row");
#else
		strcpy(toolTip, "F-Keys:\n"\
						" F1 - Go To PQL::MAIN\n"\
						"\nDisplay LOGS:\tDOUBLE-CLICK on Row");
#endif
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), mouseBox, toolTip, toolTip);
		gtk_container_add(GTK_CONTAINER(widget), box);

		g_object_unref(size_group);
		break;
	}

	gtk_widget_modify_bg(menu_box, GTK_STATE_NORMAL, &settings.colors.sb[scr]);
	gtk_size_group_add_widget(size_group, widget);

	return menu_box;
}

static GtkWidget * makeSplit(int type)
{
	GtkWidget	*vpane=NULL, *vpane1, *vpane2, *frame, *da;
	
  switch(type)
  {
	case SPLITSCR2:

		Hpanes[0] = vpane = gtk_vpaned_new();

  		frame = gtk_frame_new(NULL);
  		gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
  		DA[SPLITDA2T] = da = gtk_drawing_area_new();
  		g_signal_connect(da, "configure_event", G_CALLBACK(pixmap_cfg), GINT_TO_POINTER(SPLITDA2T));
  		g_signal_connect(da, "expose_event", G_CALLBACK(pixmap_exp), GINT_TO_POINTER(SPLITDA2T));
		g_signal_connect(da, "scroll_event", G_CALLBACK(scrollEvent), GINT_TO_POINTER(SPLITDA2T));
  		g_signal_connect(da, "button_press_event", G_CALLBACK (mouseEvent), GINT_TO_POINTER(SPLITDA2T));
  		g_signal_connect(da, "button_release_event", G_CALLBACK (mouseEvent), GINT_TO_POINTER(SPLITDA2T));
  		g_signal_connect(da, "motion_notify_event", G_CALLBACK (dragEvent), GINT_TO_POINTER(SPLITDA2T));
  		g_signal_connect(da, "enter_notify_event", G_CALLBACK (grabFocus), &settings.split.DA[SPLIT2][TOP]);
  		g_signal_connect(da, "leave_notify_event", G_CALLBACK (grabFocus), &settings.split.DA[SPLIT2][TOP]);
		g_signal_connect(da, "key_press_event", G_CALLBACK (FKeyEvent), NULL);
  		g_signal_connect(da, "key_press_event", G_CALLBACK (keyEvent), GINT_TO_POINTER(SPLITDA2T));
		g_signal_connect(da, "key_release_event", G_CALLBACK (keyEvent), GINT_TO_POINTER(SPLITDA2T));
  		gtk_widget_set_events(da, gtk_widget_get_events (da) 
				  | GDK_KEY_PRESS_MASK
				  | GDK_KEY_RELEASE_MASK
				  | GDK_LEAVE_NOTIFY_MASK
				  | GDK_ENTER_NOTIFY_MASK
				  | GDK_BUTTON_PRESS_MASK
			      | GDK_POINTER_MOTION_MASK
			      | GDK_POINTER_MOTION_HINT_MASK
				  | GDK_BUTTON_RELEASE_MASK);

  		gtk_container_add(GTK_CONTAINER(frame), da);
		gtk_paned_pack1(GTK_PANED (vpane), frame, TRUE, TRUE);

  		frame = gtk_frame_new(NULL);
  		gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
  		DA[SPLITDA2B] = da = gtk_drawing_area_new();
  		g_signal_connect(da, "configure_event", G_CALLBACK(pixmap_cfg), GINT_TO_POINTER(SPLITDA2B));
  		g_signal_connect(da, "expose_event", G_CALLBACK(pixmap_exp), GINT_TO_POINTER(SPLITDA2B));
		g_signal_connect(da, "scroll_event", G_CALLBACK(scrollEvent), GINT_TO_POINTER(SPLITDA2B));
  		g_signal_connect(da, "button_press_event", G_CALLBACK (mouseEvent), GINT_TO_POINTER(SPLITDA2B));
  		g_signal_connect(da, "button_release_event", G_CALLBACK (mouseEvent), GINT_TO_POINTER(SPLITDA2B));
  		g_signal_connect(da, "motion_notify_event", G_CALLBACK (dragEvent), GINT_TO_POINTER(SPLITDA2B));
  		g_signal_connect(da, "enter_notify_event", G_CALLBACK (grabFocus), &settings.split.DA[SPLIT2][BOT]);
  		g_signal_connect(da, "leave_notify_event", G_CALLBACK (grabFocus), &settings.split.DA[SPLIT2][BOT]);
		g_signal_connect(da, "key_press_event", G_CALLBACK (FKeyEvent), NULL);
  		g_signal_connect(da, "key_press_event", G_CALLBACK (keyEvent), GINT_TO_POINTER(SPLITDA2B));
		g_signal_connect(da, "key_release_event", G_CALLBACK (keyEvent), GINT_TO_POINTER(SPLITDA2B));
  		gtk_widget_set_events(da, gtk_widget_get_events (da) 
				  | GDK_KEY_PRESS_MASK
				  | GDK_KEY_RELEASE_MASK
				  | GDK_LEAVE_NOTIFY_MASK
				  | GDK_ENTER_NOTIFY_MASK
				  | GDK_BUTTON_PRESS_MASK
			      | GDK_POINTER_MOTION_MASK
			      | GDK_POINTER_MOTION_HINT_MASK
				  | GDK_BUTTON_RELEASE_MASK);

  		gtk_container_add(GTK_CONTAINER(frame), da);
		gtk_paned_pack2(GTK_PANED (vpane), frame, TRUE, TRUE);

		break;

	case SPLITSCR3:

		Hpanes[2] = vpane1 = gtk_vpaned_new();
		Hpanes[1] = vpane2 = gtk_vpaned_new();

  		frame = gtk_frame_new(NULL);
  		gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
  		DA[SPLITDA3T] = da = gtk_drawing_area_new();
  		g_signal_connect(da, "configure_event", G_CALLBACK(pixmap_cfg), GINT_TO_POINTER(SPLITDA3T));
  		g_signal_connect(da, "expose_event", G_CALLBACK(pixmap_exp), GINT_TO_POINTER(SPLITDA3T));
		g_signal_connect(da, "scroll_event", G_CALLBACK(scrollEvent), GINT_TO_POINTER(SPLITDA3T));
  		g_signal_connect(da, "button_press_event", G_CALLBACK (mouseEvent), GINT_TO_POINTER(SPLITDA3T));
  		g_signal_connect(da, "button_release_event", G_CALLBACK (mouseEvent), GINT_TO_POINTER(SPLITDA3T));
  		g_signal_connect(da, "motion_notify_event", G_CALLBACK (dragEvent), GINT_TO_POINTER(SPLITDA3T));
  		g_signal_connect(da, "enter_notify_event", G_CALLBACK (grabFocus), &settings.split.DA[SPLIT3][TOP]);
  		g_signal_connect(da, "leave_notify_event", G_CALLBACK (grabFocus), &settings.split.DA[SPLIT3][TOP]);
		g_signal_connect(da, "key_press_event", G_CALLBACK (FKeyEvent), NULL);
  		g_signal_connect(da, "key_press_event", G_CALLBACK (keyEvent), GINT_TO_POINTER(SPLITDA3T));
		g_signal_connect(da, "key_release_event", G_CALLBACK (keyEvent), GINT_TO_POINTER(SPLITDA3T));
  		gtk_widget_set_events(da, gtk_widget_get_events (da) 
				  | GDK_KEY_PRESS_MASK
				  | GDK_KEY_RELEASE_MASK
				  | GDK_LEAVE_NOTIFY_MASK
				  | GDK_ENTER_NOTIFY_MASK
				  | GDK_BUTTON_PRESS_MASK
			      | GDK_POINTER_MOTION_MASK
			      | GDK_POINTER_MOTION_HINT_MASK
				  | GDK_BUTTON_RELEASE_MASK);

  		gtk_container_add(GTK_CONTAINER(frame), da);
		gtk_paned_pack1(GTK_PANED (vpane2), frame, FALSE, TRUE);

  		frame = gtk_frame_new(NULL);
  		gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
  		DA[SPLITDA3M] = da = gtk_drawing_area_new();
  		g_signal_connect(da, "configure_event", G_CALLBACK(pixmap_cfg), GINT_TO_POINTER(SPLITDA3M));
  		g_signal_connect(da, "expose_event", G_CALLBACK(pixmap_exp), GINT_TO_POINTER(SPLITDA3M));
		g_signal_connect(da, "scroll_event", G_CALLBACK(scrollEvent), GINT_TO_POINTER(SPLITDA3M));
  		g_signal_connect(da, "button_press_event", G_CALLBACK (mouseEvent), GINT_TO_POINTER(SPLITDA3M));
  		g_signal_connect(da, "button_release_event", G_CALLBACK (mouseEvent), GINT_TO_POINTER(SPLITDA3M));
  		g_signal_connect(da, "motion_notify_event", G_CALLBACK (dragEvent), GINT_TO_POINTER(SPLITDA3M));
  		g_signal_connect(da, "enter_notify_event", G_CALLBACK (grabFocus), &settings.split.DA[SPLIT3][MID]);
  		g_signal_connect(da, "leave_notify_event", G_CALLBACK (grabFocus), &settings.split.DA[SPLIT3][MID]);
		g_signal_connect(da, "key_press_event", G_CALLBACK (FKeyEvent), NULL);
  		g_signal_connect(da, "key_press_event", G_CALLBACK (keyEvent), GINT_TO_POINTER(SPLITDA3M));
		g_signal_connect(da, "key_release_event", G_CALLBACK (keyEvent), GINT_TO_POINTER(SPLITDA3M));
  		gtk_widget_set_events(da, gtk_widget_get_events (da) 
				  | GDK_KEY_PRESS_MASK
				  | GDK_KEY_RELEASE_MASK
				  | GDK_LEAVE_NOTIFY_MASK
				  | GDK_ENTER_NOTIFY_MASK
				  | GDK_BUTTON_PRESS_MASK
			      | GDK_POINTER_MOTION_MASK
			      | GDK_POINTER_MOTION_HINT_MASK
				  | GDK_BUTTON_RELEASE_MASK);

  		gtk_container_add(GTK_CONTAINER(frame), da);
		gtk_paned_pack2(GTK_PANED (vpane2), frame, TRUE, TRUE);

		gtk_paned_pack1(GTK_PANED (vpane1), vpane2, TRUE, TRUE);

  		frame = gtk_frame_new(NULL);
  		gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
  		DA[SPLITDA3B] = da = gtk_drawing_area_new();
  		g_signal_connect(da, "configure_event", G_CALLBACK(pixmap_cfg), GINT_TO_POINTER(SPLITDA3B));
  		g_signal_connect(da, "expose_event", G_CALLBACK(pixmap_exp), GINT_TO_POINTER(SPLITDA3B));
		g_signal_connect(da, "scroll_event", G_CALLBACK(scrollEvent), GINT_TO_POINTER(SPLITDA3B));
  		g_signal_connect(da, "button_press_event", G_CALLBACK (mouseEvent), GINT_TO_POINTER(SPLITDA3B));
  		g_signal_connect(da, "button_release_event", G_CALLBACK (mouseEvent), GINT_TO_POINTER(SPLITDA3B));
  		g_signal_connect(da, "motion_notify_event", G_CALLBACK (dragEvent), GINT_TO_POINTER(SPLITDA3B));
  		g_signal_connect(da, "enter_notify_event", G_CALLBACK (grabFocus), &settings.split.DA[SPLIT3][BOT]);
  		g_signal_connect(da, "leave_notify_event", G_CALLBACK (grabFocus), &settings.split.DA[SPLIT3][BOT]);
		g_signal_connect(da, "key_press_event", G_CALLBACK (FKeyEvent), NULL);
  		g_signal_connect(da, "key_press_event", G_CALLBACK (keyEvent), GINT_TO_POINTER(SPLITDA3B));
		g_signal_connect(da, "key_release_event", G_CALLBACK (keyEvent), GINT_TO_POINTER(SPLITDA3B));
  		gtk_widget_set_events(da, gtk_widget_get_events (da) 
				  | GDK_KEY_PRESS_MASK
				  | GDK_KEY_RELEASE_MASK
				  | GDK_LEAVE_NOTIFY_MASK
				  | GDK_ENTER_NOTIFY_MASK
				  | GDK_BUTTON_PRESS_MASK
			      | GDK_POINTER_MOTION_MASK
			      | GDK_POINTER_MOTION_HINT_MASK
				  | GDK_BUTTON_RELEASE_MASK);

  		gtk_container_add(GTK_CONTAINER(frame), da);
		gtk_paned_pack2(GTK_PANED (vpane1), frame, TRUE, TRUE);

		vpane = vpane1;
		break;
  }

  return vpane;
}

static GtkWidget *logViewer()
{
	static GtkWidget *view;

	view = gtk_text_view_new();
	gtk_text_view_set_editable(GTK_TEXT_VIEW(view), FALSE);
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(view), FALSE);
	gtk_text_view_set_pixels_below_lines(GTK_TEXT_VIEW(view), 2);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(view), GTK_WRAP_WORD);

	return view;
}

static void makeTab(int which, GtkNotebook *nb)
{
	GtkWidget *da, *sw;
	GtkWidget *frame;

	switch(which)
	{
		case TRACESCR:		// make the TRACE screen
			frame = gtk_frame_new(NULL);
			gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
			DA[TRACESCR] = da = gtk_drawing_area_new();
			gtk_container_add(GTK_CONTAINER(frame), da);
			signals[BTNPRSSGNL][TRACESIGNAL] = g_signal_connect(da, "button_press_event", G_CALLBACK (mouseEvent), GINT_TO_POINTER(TRACESCR));
			signals[BTNRLSSGNL][TRACESIGNAL] = g_signal_connect(da, "button_release_event", G_CALLBACK (mouseEvent), 
					GINT_TO_POINTER(TRACESCR));
			signals[MTNNTFYSGNL][TRACESIGNAL] = g_signal_connect(da, "motion_notify_event", G_CALLBACK (dragEvent), 
					GINT_TO_POINTER(TRACESCR));
			g_signal_connect(da, "enter_notify_event", G_CALLBACK (grabFocus), GINT_TO_POINTER(TRACESCR));
			g_signal_connect(da, "leave_notify_event", G_CALLBACK (grabFocus), GINT_TO_POINTER(TRACESCR));
			gtk_widget_set_events(da, gtk_widget_get_events (da) 
						  | GDK_LEAVE_NOTIFY_MASK
						  | GDK_ENTER_NOTIFY_MASK
						  | GDK_KEY_PRESS_MASK
						  | GDK_BUTTON_PRESS_MASK
						  | GDK_POINTER_MOTION_MASK
						  | GDK_POINTER_MOTION_HINT_MASK
						  | GDK_BUTTON_RELEASE_MASK);
			gtk_notebook_append_page(nb, frame, gtk_label_new(" Trace "));
		break;
		
		case MAGNIFYSCR:		// make the MAGNIFY screen
			frame = gtk_frame_new(NULL);
			gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
			DA[MAGNIFYSCR] = da = gtk_drawing_area_new();
			gtk_container_add(GTK_CONTAINER(frame), da);
			g_signal_connect(da, "configure_event", G_CALLBACK(pixmap_cfg), GINT_TO_POINTER(MAGNIFYSCR));
			g_signal_connect(da, "expose_event", G_CALLBACK(pixmap_exp), GINT_TO_POINTER(MAGNIFYSCR));
			g_signal_connect(da, "scroll_event", G_CALLBACK(scrollEvent), GINT_TO_POINTER(MAGNIFYSCR));
			g_signal_connect(da, "button_press_event", G_CALLBACK (mouseEvent), GINT_TO_POINTER(MAGNIFYSCR));
			g_signal_connect(da, "button_release_event", G_CALLBACK (mouseEvent), GINT_TO_POINTER(MAGNIFYSCR));
			g_signal_connect(da, "key_press_event", G_CALLBACK (FKeyEvent), NULL);
			g_signal_connect(da, "key_press_event", G_CALLBACK (keyEvent), GINT_TO_POINTER(MAGNIFYSCR));
			g_signal_connect(da, "key_release_event", G_CALLBACK (ctrlKeyEvent), GINT_TO_POINTER(MAGNIFYSCR));
			g_signal_connect(da, "key_release_event", G_CALLBACK (keyEvent), GINT_TO_POINTER(MAGNIFYSCR));
			g_signal_connect(da, "motion_notify_event", G_CALLBACK (dragEvent), GINT_TO_POINTER(MAGNIFYSCR));
			g_signal_connect(da, "enter_notify_event", G_CALLBACK (grabFocus), GINT_TO_POINTER(MAGNIFYSCR));
			g_signal_connect(da, "leave_notify_event", G_CALLBACK (grabFocus), GINT_TO_POINTER(MAGNIFYSCR));
			gtk_widget_set_events(da, gtk_widget_get_events (da) 
						  | GDK_LEAVE_NOTIFY_MASK
						  | GDK_ENTER_NOTIFY_MASK
						  | GDK_BUTTON_PRESS_MASK
						  | GDK_KEY_PRESS_MASK
						  | GDK_KEY_RELEASE_MASK
						  | GDK_POINTER_MOTION_MASK
						  | GDK_POINTER_MOTION_HINT_MASK
						  | GDK_BUTTON_RELEASE_MASK);
			gtk_notebook_append_page(nb, frame, gtk_label_new(" Magnify "));
		break;
		
		case SPECTRASCR:		// make the SPECTRA screen
			frame = gtk_frame_new(NULL);
			gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
			DA[SPECTRASCR] = da = gtk_drawing_area_new();
			gtk_container_add(GTK_CONTAINER(frame), da);
			g_signal_connect(da, "configure_event", G_CALLBACK(pixmap_cfg), GINT_TO_POINTER(SPECTRASCR));
			g_signal_connect(da, "expose_event", G_CALLBACK(pixmap_exp), GINT_TO_POINTER(SPECTRASCR));
			g_signal_connect(da, "button_press_event", G_CALLBACK (mouseEvent), GINT_TO_POINTER(SPECTRASCR));
			g_signal_connect(da, "button_release_event", G_CALLBACK (mouseEvent), GINT_TO_POINTER(SPECTRASCR));
			g_signal_connect(da, "motion_notify_event", G_CALLBACK (dragEvent), GINT_TO_POINTER(SPECTRASCR));
			g_signal_connect(da, "enter_notify_event", G_CALLBACK (grabFocus), GINT_TO_POINTER(SPECTRASCR));
			g_signal_connect(da, "leave_notify_event", G_CALLBACK (grabFocus), GINT_TO_POINTER(SPECTRASCR));
			gtk_widget_set_events(da, gtk_widget_get_events (da) 
						  | GDK_LEAVE_NOTIFY_MASK
						  | GDK_ENTER_NOTIFY_MASK
						  | GDK_KEY_PRESS_MASK
						  | GDK_BUTTON_PRESS_MASK
						  | GDK_POINTER_MOTION_MASK
						  | GDK_POINTER_MOTION_HINT_MASK
						  | GDK_BUTTON_RELEASE_MASK
						  | GDK_BUTTON_RELEASE_MASK);
			gtk_notebook_append_page(nb, frame, gtk_label_new(" Spectra "));
		break;
		
		case SPLITSCR:		// make the SPLIT screens
			splitBox = gtk_hbox_new(FALSE, 0);
			split2 = gtk_frame_new(NULL);
			gtk_frame_set_shadow_type(GTK_FRAME(split2), GTK_SHADOW_IN);
			frame = makeSplit(SPLITSCR2);
			gtk_container_add(GTK_CONTAINER(split2), frame);
			gtk_container_add(GTK_CONTAINER(splitBox), split2);
		
			split3 = gtk_frame_new(NULL);
			gtk_frame_set_shadow_type(GTK_FRAME(split3), GTK_SHADOW_IN);
			frame = makeSplit(SPLITSCR3);
			gtk_container_add(GTK_CONTAINER(split3), frame);
			gtk_container_add(GTK_CONTAINER(splitBox), split3);
		
			gtk_notebook_append_page(nb, splitBox, gtk_label_new(" Split "));
		break;
		
		case HEADERSCR:		// make the HEADER screen
			hdrSW = sw = gtk_scrolled_window_new(NULL, NULL);
			gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW(sw),
							   GTK_SHADOW_ETCHED_IN);
			gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(sw),
							  GTK_POLICY_AUTOMATIC,
							  GTK_POLICY_AUTOMATIC);
			hdrVIEW = makeHdrModel();
			g_signal_connect(hdrVIEW, "row-activated", G_CALLBACK (newStart), NULL);
			gtk_container_add(GTK_CONTAINER(sw), hdrVIEW);
			gtk_notebook_append_page(nb, sw, gtk_label_new(" Headers "));
		break;

		case LOGSCR:		// make the LOG screen
		{
			GtkWidget	*vbox, *tv, *vbox1, *label, *vpane, *labelBox;
			
			logNBTab = vpane = gtk_vpaned_new();
			vbox1 = gtk_vbox_new(TRUE, 0);
			gtk_paned_pack1(GTK_PANED (vpane), vbox1, TRUE, TRUE);

			logSW[LOGFILESW] = vbox = gtk_vbox_new(FALSE, 0);
			labelBox = gtk_event_box_new();
			gtk_widget_modify_bg(labelBox, GTK_STATE_NORMAL, &settings.colors.sb[logScr]);
			label = gtk_label_new(NULL);
			gtk_label_set_markup(GTK_LABEL(label),
					(const gchar *) "<span weight=\"bold\" foreground=\"white\" font_desc=\"11\">By Disk File</span>");
			sw = gtk_scrolled_window_new(NULL, NULL);
			gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(sw),
							   GTK_SHADOW_ETCHED_IN);
			gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw),
							  GTK_POLICY_AUTOMATIC,
							  GTK_POLICY_AUTOMATIC);
			logVIEW[LOGFILE] = tv = makeLOGModel(LOGFILE);
			g_signal_connect(logVIEW[LOGFILE], "row-activated", G_CALLBACK (logSelect), GINT_TO_POINTER(LOGFILE));
			gtk_container_add(GTK_CONTAINER(sw), tv);
			gtk_container_add(GTK_CONTAINER(labelBox), label);
			gtk_box_pack_start(GTK_BOX(vbox), labelBox, FALSE, FALSE, 1);
			gtk_box_pack_end(GTK_BOX(vbox), sw, TRUE, TRUE, 1);
			gtk_container_add(GTK_CONTAINER(vbox1), vbox);

			logSW[LOGTRCSW] = vbox = gtk_vbox_new(FALSE, 0);
			labelBox = gtk_event_box_new();
			gtk_widget_modify_bg(labelBox, GTK_STATE_NORMAL, &settings.colors.sb[logScr]);
			label = gtk_label_new(NULL);
			gtk_label_set_markup(GTK_LABEL(label),
					(const gchar *) "<span weight=\"bold\" foreground=\"white\" font_desc=\"11\">By Trace</span>");
			sw = gtk_scrolled_window_new(NULL, NULL);
			gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(sw),
							   GTK_SHADOW_ETCHED_IN);
			gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw),
							  GTK_POLICY_AUTOMATIC,
							  GTK_POLICY_AUTOMATIC);
			logVIEW[LOGTRC] = tv = makeLOGModel(LOGTRC);
			g_signal_connect(logVIEW[LOGTRC], "row-activated", G_CALLBACK (logSelect), GINT_TO_POINTER(LOGTRC));
			gtk_container_add(GTK_CONTAINER(sw), tv);
			gtk_container_add(GTK_CONTAINER(labelBox), label);
			gtk_box_pack_start(GTK_BOX(vbox), labelBox, FALSE, FALSE, 1);
			gtk_box_pack_end(GTK_BOX(vbox), sw, TRUE, TRUE, 1);
			gtk_container_add(GTK_CONTAINER(vbox1), vbox);

			vbox1 = gtk_vbox_new(FALSE, 0);
			logSW[LOGVIEWSW] = vbox = gtk_vbox_new(FALSE, 0);
			labelBox = gtk_event_box_new();
			gtk_widget_modify_bg(labelBox, GTK_STATE_NORMAL, &settings.colors.sb[logScr]);
			label = gtk_label_new(NULL);
			gtk_label_set_markup(GTK_LABEL(label),
					(const gchar *) "<span weight=\"bold\" foreground=\"white\" font_desc=\"11\">LOG</span>");
			sw = gtk_scrolled_window_new(NULL, NULL);
			gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(sw),
							   GTK_SHADOW_ETCHED_IN);
			gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw),
							  GTK_POLICY_AUTOMATIC,
							  GTK_POLICY_AUTOMATIC);
			logTEXT = logViewer();
			gtk_container_add(GTK_CONTAINER(sw), logTEXT);
			gtk_container_add(GTK_CONTAINER(labelBox), label);
			gtk_box_pack_start(GTK_BOX(vbox), labelBox, FALSE, FALSE, 1);
			gtk_box_pack_end(GTK_BOX(vbox), sw, TRUE, TRUE, 1);
			gtk_container_add(GTK_CONTAINER(vbox1), vbox);
			gtk_paned_pack2(GTK_PANED (vpane), vbox1, TRUE, TRUE);

			gtk_notebook_append_page(nb, logNBTab, gtk_label_new(" LOGS "));
			gtk_paned_set_position((GtkPaned *) vpane, 200);
		}
		break;
	}
}

void make_dataScreens()
{
	GtkWidget	*vbox, *hbox, *dataBox, *SB;
	int			i;
	
	tips = gtk_tooltips_new();

#ifndef PQL_ONLY
// PQLX-specific compile
	dataBox = systems[PQLSYSTEM] = gtk_hbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(topBox), systems[PQLSYSTEM]);

// PQLX-specific system combo box
	vbox = gtk_vbox_new(FALSE, 0);					// holding system pull-down and all sidebars
	sysCbo[PQLSYSTEM] = gtk_combo_box_new_text();
	for (i=0;i<NUM_SYSTEMS;i++)
	{
	  	gtk_combo_box_insert_text(GTK_COMBO_BOX(sysCbo[PQLSYSTEM]), i, systemNames[i]);
	}
	gtk_combo_box_set_active(GTK_COMBO_BOX(sysCbo[PQLSYSTEM]), -1);
	gtk_box_pack_start(GTK_BOX(vbox), sysCbo[PQLSYSTEM], FALSE, FALSE, 0);
	sysCboSgnl[PQLSYSTEM] = g_signal_connect(sysCbo[PQLSYSTEM], "changed", G_CALLBACK(systemChg), NULL);

#else
// PQL-specific compile
  {
	char	version[35];

	topWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	sprintf(version, "PQL II - version %s", PQL_VERSION);
	gtk_window_set_title(GTK_WINDOW (topWindow), version);
	gtk_window_set_position(GTK_WINDOW (topWindow), GTK_WIN_POS_CENTER);
  }

	gtk_widget_set_events(topWindow, gtk_widget_get_events (topWindow) 
								  | GDK_KEY_PRESS_MASK);
	g_signal_connect(topWindow, "key_press_event", G_CALLBACK (FKeyEvent), NULL);
	gtk_container_set_border_width(GTK_CONTAINER(topWindow), 2);
	g_signal_connect(topWindow, "destroy", G_CALLBACK (gtk_main_quit), NULL);

	dataBox = gtk_hbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(topWindow), dataBox);

	vbox = gtk_vbox_new(FALSE, 0);

#endif

	hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(dataBox), vbox, FALSE, FALSE, 0);

	// make all the SideBars for each display SCREEN
	for (i=0;i<NUMSCREENS;i++)
	{
		buttons[i] = SB = makeSideBar(i);
		gtk_box_pack_start(GTK_BOX(hbox), SB, FALSE, FALSE, 0);
	}

	// make the NOTEBOOK holding the data screens
	vbox = gtk_vbox_new(FALSE, 0);
	data_screens = gtk_notebook_new();
	gtk_box_pack_start(GTK_BOX(vbox), data_screens, TRUE, TRUE, 0);
	statusBar = gtk_statusbar_new();
	gtk_box_pack_end(GTK_BOX(vbox), statusBar, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(dataBox), vbox, TRUE, TRUE, 0);

	// make all the Data Screens in turn
	for (i=0;i<NUMSCREENS;i++)
		makeTab(i, (GtkNotebook *) data_screens);

	return;
}

static void connect_signals()
{
	signals[NTBKSGNL][TRACESIGNAL] = g_signal_connect(data_screens, "switch-page", G_CALLBACK(change_menu), &states.currentPage);
	g_signal_connect(DA[TRACESCR], "configure_event", G_CALLBACK(pixmap_cfg), GINT_TO_POINTER(TRACESCR));
	g_signal_connect(DA[TRACESCR], "expose_event", G_CALLBACK(pixmap_exp), GINT_TO_POINTER(TRACESCR));

}

void importTraces();
static void setPQLStart()
{
	states.currentPage = TRACESCR;				// initial state
	states.currentScreen = MAIN;
	states.reset.reset[TRACESCR] = TRUE;
	states.general.plot.state = NOTRACES;
}

static void _makeTraceBG()
{	// make the trace screen invisibly, i.e., do not display
	int oldPage, start;
	
	oldPage = states.currentPage;
	states.currentScreen=MAIN;
	states.currentPage = NONE;
	states.general.plot.state = DATADONE;		// data already in memory
	states.reset.reset[TRACESCR] = TRUE;
	makePixmap(TRACEDA);
	states.trace.numSelected = 0;
	if (states.general.numTraces < settings.trace.numPlots)
	{
		start = 0;
	}
	else
	{
		start = states.general.numTraces - settings.trace.numPlots;
	}
	states.trace.startRec = start;
	makeSelectionList(TRUE);
	states.currentPage = oldPage;
}

void switchScreenPQL(int screen)
{
	if (screen == LOGSCR &&
		!states.logs.visible)
		screen = TRACESCR;
	gtk_notebook_set_current_page((GtkNotebook *) data_screens, screen);
}

void setupPQL(SOURCEINFO *sysInfo)
{
	int		oldPane;
	
	if (states.input.numFiles)		
	{	// set-up required only when new files are being passed in
		importTraces();
		sortTraceList();
		states.tForm.on = FALSE;				// set filter OFF
		setRadios(FILTERTYPE);					// set all filter widgets equal
		switch (states.general.sourceSystem)
		{
			case STNSYSTEM:		// set Time Axis to ABSOLUTE
				gtk_combo_box_set_active(GTK_COMBO_BOX(Tcbo[TRCXAXIS]), ABSOLUTE);
			break;
		}

		switch(sysInfo->destTab)
		{
			case PQLTRC:
				if((states.currentPage != TRACESCR ||
					states.currentScreen != MAIN))
				{	// MAIN.TRACESCR not on display, do it programmatically
					gtk_notebook_set_current_page((GtkNotebook *) data_screens, TRACESCR);
				}
				else
				{	// MAIN.TRACESCR on display, re-plot
					states.general.plot.state = READDATA;
					gtk_widget_queue_resize(DA[TRACEDA]);	// use configure event
				}
			break;
			case PQLMAG:
				_makeTraceBG();
				states.trace.limits.selection.pix[MAIN].left = states.trace.limits.margin[MAIN].left;
				states.trace.limits.selection.pix[MAIN].right = states.trace.limits.margin[MAIN].right;
				states.reset.cursor = TRUE;
				states.magnify.selectionChanged = TRUE;
				settings.magnify.Tamp = sysInfo->yaxis.scale;
				if (sysInfo->yaxis.fixed[MAX])
					gtk_entry_set_text(GTK_ENTRY(MTfixed[MAX]), sysInfo->yaxis.fixed[MAX]);
				if (sysInfo->yaxis.fixed[MIN])
					gtk_entry_set_text(GTK_ENTRY(MTfixed[MIN]), sysInfo->yaxis.fixed[MIN]);
				if(states.currentPage != MAGNIFYSCR)
				{
					gtk_notebook_set_current_page((GtkNotebook *) data_screens, MAGNIFYSCR);
				}
				else
				{	
					setRadios(MAGSCRTYPE);
					reMag(NULL, GINT_TO_POINTER(MAGNIFYSCR));
				}
			break;
			case PQLSPLT2:
			case PQLSPLT3:
			{
				oldPane = states.split.panes;
				_makeTraceBG();
				states.split.panes = (sysInfo->destTab == PQLSPLT2) ? SPLIT2 : SPLIT3;
				states.trace.limits.selection.pix[MAIN].left = NONE;	// indication that this must be re-set!
				states.trace.limits.selection.pix[MAIN].right = NONE;	// see makePixmap()
				states.reset.cursor = TRUE;
				states.magnify.selectionChanged = TRUE;
				settings.magnify.Tamp = sysInfo->yaxis.scale;
				if (sysInfo->yaxis.fixed[MAX])
					gtk_entry_set_text(GTK_ENTRY(HTfixed[MAX]), sysInfo->yaxis.fixed[MAX]);
				if (sysInfo->yaxis.fixed[MIN])
					gtk_entry_set_text(GTK_ENTRY(HTfixed[MIN]), sysInfo->yaxis.fixed[MIN]);
				switch (oldPane-states.split.panes)
				{
					case 0:		// same as last call
						newSplit((GtkToggleButton *)Hsplit[states.split.panes], &states.split.panes);
						raise_page(NULL, &OV[SPLITSCR]);
					break;
					default:	// different, do it programmatically
						gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (Hsplit[states.split.panes]), TRUE);
					break;
				}
			}
			break;
		}
	}
}

void make_pql(gboolean low_res)
{
	int		i;
	
	lowRes = low_res;
	
	// set up signals array to manage necessary signal blocking
	signals[SORTSGNL] = calloc(TTLSORTTYPES * MAXTTLSORTCRITERIA, sizeof(gulong));
	signals[FILTSGNL] = calloc(MAXSCRNSGNLS, sizeof(gulong));
	signals[PICKSGNL] = calloc(MAXSCRNSGNLS, sizeof(gulong));
	signals[DISPSGNL] = calloc(MAXSCRNSGNLS, sizeof(gulong));
	signals[BTNPRSSGNL] = calloc(1, sizeof(gulong));
	signals[BTNRLSSGNL] = calloc(1, sizeof(gulong));
	signals[MTNNTFYSGNL] = calloc(1, sizeof(gulong));
	signals[NTBKSGNL] = calloc(1, sizeof(gulong));
	signals[FILTCBOSGNL] = calloc(TOTFILTERCBOS, sizeof(gulong));
	signals[TRACESCALESGNL] = calloc(TOTALDA, sizeof(gulong));// * 3);
	signals[WNDWSCALESGNL] = calloc(TOTALDA, sizeof(gulong));// * 2);
	signals[OVERLAYSGNL] = calloc(TOTALDA, sizeof(gulong));// * 2);

	// make the data window and children
	make_dataScreens();

	// realize all widgets
	gtk_accelerator_set_default_mod_mask(GDK_CONTROL_MASK|GDK_SHIFT_MASK|GDK_MOD1_MASK);
	gtk_widget_show_all(topWindow);
  
	gtk_widget_hide(logNBTab);
	states.logs.visible = FALSE;
	
	setFiltCbos();		// set up the filter combo boxes		

	// set the window size according to the user's settings
	gtk_window_resize (GTK_WINDOW (topWindow), 
				settings.general.startUP.winDims[WIDTH], 
				settings.general.startUP.winDims[HEIGHT]);

	// hide what we don't want to see
	for(i=1;i<NUMSCREENS;i++)
		gtk_widget_hide (buttons[i]);
	
	gtk_widget_hide (split3);
	gtk_widget_hide (progressBar);
	gtk_widget_hide (Mmag);
	gtk_widget_hide (Hmag);

	if (settings.magnify.Tamp != FIXEDS)
	{
		gtk_widget_hide(MTfixedB[MAX]);
		gtk_widget_hide(MTfixedB[MIN]);
		gtk_widget_hide(HTfixedB[MAX]);
		gtk_widget_hide(HTfixedB[MIN]);
	}
	
	if (settings.general.display.mode == TRACEMODE)
	{
		gtk_widget_hide(dispGainB[TRACESCR]);
		gtk_widget_hide(dispGainB[SPECTRASCR]);
	}

	if (!settings.magnify.dispBox[ZOOMBOX])
		gtk_widget_hide(MZoomBox);
	else
		gtk_widget_show(MZoomBox);

	if (!settings.magnify.dispBox[PICKBOX])
		gtk_widget_hide(MPickBox);
	else
		gtk_widget_show(MPickBox);

	if (settings.split.pane == SPLIT2)
		gtk_widget_hide(HMidFrame);
	
	connect_signals();
	setPQLStart();

#if 0
 	if (!strlen(settings.general.sort.str))
	{
		GtkWidget *dispWidget;
		char	*msg = "\nSORT Settings Have Changed!\n\n"
						"To make this annoying message go away:\n"
						"\t1) (Re)-Set Your Sort Options - \n"
						"\t\tFound in the Controls Panel::General Tab\n"
						"\t2) Select 'Set Defaults' on Controls Panel Side-Bar";
		
		dispWidget = gtk_message_dialog_new(NULL,	GTK_DIALOG_DESTROY_WITH_PARENT, 
													GTK_MESSAGE_INFO, GTK_BUTTONS_OK, 
													"%s", msg);
		gtk_window_set_position(GTK_WINDOW (dispWidget), GTK_WIN_POS_CENTER);
		gtk_dialog_run(GTK_DIALOG (dispWidget));
		gtk_widget_destroy(dispWidget);
	}
#endif

	return ;
}
