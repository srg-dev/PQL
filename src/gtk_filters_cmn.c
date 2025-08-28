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
#include "db_iface.h"
#include "dbl_iface.h"
#endif

enum {
	AGCWINLEN,
	AGCDWIN,
	AGCDTECT,
	AGCTHRESH,
	AGCTTL
};
static GtkWidget	*fltrVIEW, *FName, *fltrDialog[TTLFILTEREXTRA];
static GtkWidget	*cutoff[TTLPOLETYPES][TTLFILTEREXTRA],
				*poles[TTLPOLETYPES][TTLFILTEREXTRA];
static GtkWidget	*AGC[AGCTTL];

extern GtkTooltips *tips;
static GtkSizeGroup	*size_group;
static char		toolTip[1000];
static GtkWidget	*fltrSV, *fltrNEW, *fltrDEL;
static	gboolean	listChgd=FALSE;

gboolean stnIdleFilter(gpointer);
static void fltrInfo(GtkTreeView *treeview, gpointer nil);
static void fltrEdit(GtkTreeView *treeview, GtkTreePath *arg1, GtkTreeViewColumn *arg2, gpointer nil);
static void fltrCBack(GtkButton *button, gpointer t);

enum {
	FLTRNEW,
	FLTRSAVE,
	FLTRDEL,
	FLTRLSTDONE,
	FLTRCSTCX,
	FLTRCSTCONT,
	FLTRAGCCX,
	FLTRAGCDONE
};

enum {
	FLTRLIST,
	FLTRAGC
};

static void filtrBlk(int block)
{	// block and unblock the filter combo box selection callback
	int i;
	switch(block)
	{
		case BLOCKSIGS:		// block all filter combo signal callbacks
			for(i=0;i<TOTFILTERCBOS;i++)
				g_signal_handler_block(fltrCbos[i], signals[FILTCBOSGNL][i]);
		break;

		case UNBLOCKSIGS:
			for(i=0;i<TOTFILTERCBOS;i++)
				g_signal_handler_unblock(fltrCbos[i], signals[FILTCBOSGNL][i]);
		break;
	}
}

// store to hold the list of filters for the list dialogue
static GtkListStore *fltrStore;
static void add_fltr_hdr_columns(GtkTreeView *treeview)
{
	GtkCellRenderer *renderer;
	GtkTreeViewColumn	*col;
	
	renderer = gtk_cell_renderer_text_new();
	col = gtk_tree_view_column_new_with_attributes("Filters", renderer, "text", 0, NULL);
	gtk_tree_view_insert_column(treeview, col, 0);
}

static GtkWidget *makeFltrHdrModel()
{
	GtkWidget *treeview;

	fltrStore = gtk_list_store_new(1, G_TYPE_STRING);
	treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(fltrStore));

	return (treeview);
}

static void add_fltrs()
{	// add the filters to the dataStore for viewing
	int 		j, end;
	GtkTreeIter	iter;

	if (fltrStore)
		gtk_list_store_clear(fltrStore);

	end = FILTERCUSTOMLISTNUM;
	if (settings.general.display.mode == GATHERMODE)
		end--;		// display AGC filter only in GATHERMODE
		
	for(j=0;j<end;j++)
	{
		gtk_list_store_append(fltrStore, &iter);
		gtk_list_store_set(fltrStore, &iter, 0, settings.general.filter.filts[j]->name, -1);
	}
}

// dialogue defining a custom filter
static void make_filterCustom()
{
	GtkWidget	*frame1, *hbox, *table;
	GtkWidget	*vbox, *entry, *label, *button;
	int			i;
	char		str[10];

	fltrDialog[FILTERCUSTOM] = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(fltrDialog[FILTERCUSTOM]), " Custom Filter "); 
	gtk_container_set_border_width(GTK_CONTAINER(fltrDialog[FILTERCUSTOM]), 2);

	frame1 = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(frame1), GTK_SHADOW_IN);
	gtk_container_add(GTK_CONTAINER(fltrDialog[FILTERCUSTOM]), frame1);

	vbox = gtk_vbox_new(FALSE, 0);	// holding all widgets

	table = gtk_table_new(3, 3, FALSE);
	gtk_box_pack_start(GTK_BOX(vbox), table, TRUE, TRUE, 2);
	
	label = gtk_label_new(" Band ");
	gtk_table_attach_defaults(GTK_TABLE(table), label, 0, 1, 0, 1);
	label = gtk_label_new("Cutoff Freq (Hz)");
	gtk_table_attach_defaults(GTK_TABLE(table), label, 1, 2, 0, 1);
	label = gtk_label_new(" Poles ");
	gtk_table_attach_defaults(GTK_TABLE(table), label, 2, 3, 0, 1);
	label = gtk_label_new(" Low: ");
	gtk_table_attach_defaults(GTK_TABLE(table), label, 0, 1, 1, 2);
	label = gtk_label_new(" High: ");
	gtk_table_attach_defaults(GTK_TABLE(table), label, 0, 1, 2, 3);

	cutoff[LOWPOLE][FILTERCUSTOM] = entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(entry), 10);
	gtk_widget_set_size_request(entry, 20, -1);
	gtk_table_attach_defaults(GTK_TABLE(table), entry, 1, 2, 1, 2);
	strcpy(toolTip, "Define CUTOFF Frequency for LOW PASS Filter (Must be <= NYQUIST)");
	gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), entry, toolTip, toolTip);

	cutoff[HIGHPOLE][FILTERCUSTOM] = entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(entry), 10);
	gtk_widget_set_size_request(entry, 20, -1);
	gtk_table_attach_defaults(GTK_TABLE(table), entry, 1, 2, 2, 3);
	strcpy(toolTip, "Define CUTOFF Frequency for HIGH PASS Filter (Must be < LOW PASS Cutoff OR <= NYQUIST if LOW PASS CUTOFF not Specified)");
	gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), entry, toolTip, toolTip);

	poles[LOWPOLE][FILTERCUSTOM] = gtk_combo_box_new_text();
	poles[HIGHPOLE][FILTERCUSTOM] = gtk_combo_box_new_text();
	for (i = 0; i < NUMPOLES ; i++)
	{
		sprintf(str, "%d", i*2);
		gtk_combo_box_append_text(GTK_COMBO_BOX(poles[LOWPOLE][FILTERCUSTOM]), str);
		gtk_combo_box_append_text(GTK_COMBO_BOX(poles[HIGHPOLE][FILTERCUSTOM]), str);
	}
	gtk_widget_set_size_request(poles[LOWPOLE][FILTERCUSTOM], -1, -1);
	gtk_widget_set_size_request(poles[HIGHPOLE][FILTERCUSTOM], -1, -1);
	gtk_table_attach_defaults(GTK_TABLE(table), poles[LOWPOLE][FILTERCUSTOM], 2, 3, 1, 2);
	gtk_table_attach_defaults(GTK_TABLE(table), poles[HIGHPOLE][FILTERCUSTOM], 2, 3, 2, 3);

	// make the buttons
	hbox = gtk_hbutton_box_new();
	gtk_button_box_set_layout(GTK_BUTTON_BOX(hbox), GTK_BUTTONBOX_SPREAD);

	button = gtk_button_new_with_label("Continue");
	gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 2);
	strcpy(toolTip, "Return");
	gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), button, toolTip, toolTip);
	g_signal_connect(button, "clicked", G_CALLBACK(fltrCBack), GINT_TO_POINTER(FLTRCSTCONT));
	
	button = gtk_button_new_with_label("Cancel");
	gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 2);
	strcpy(toolTip, "Return");
	gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), button, toolTip, toolTip);
	g_signal_connect(button, "clicked", G_CALLBACK(fltrCBack), GINT_TO_POINTER(FLTRCSTCX));

	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 2);	

	gtk_container_add(GTK_CONTAINER(frame1), vbox);
	gtk_window_set_position(GTK_WINDOW(fltrDialog[FILTERCUSTOM]), GTK_WIN_POS_CENTER);
	gtk_window_set_transient_for((GtkWindow *) fltrDialog[FILTERCUSTOM], (GtkWindow *) topWindow);
	gtk_widget_show_all(fltrDialog[FILTERCUSTOM]);
}

static GtkWidget *make_tab(int which)
{
	GtkWidget	*frame1, *hbox1, *hbox, *fltrSW, *control_box;
	GtkWidget	*table;
	GtkWidget	*vbox, *vbox1, *entry, *box, *label, *button, *radioB1, *radioB2;
	int			i;
	char		str[10];

	control_box = gtk_event_box_new();
	frame1 = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME (frame1), GTK_SHADOW_IN);
	gtk_container_add(GTK_CONTAINER(control_box), frame1);

	switch(which)
	{
		case FLTRLIST:
			vbox1 = gtk_vbox_new(FALSE, 0);
			hbox1 = gtk_hbox_new(FALSE, 0);

			// make the filter list scrolled window
			fltrSW = gtk_scrolled_window_new(NULL, NULL);
			gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(fltrSW),
							   GTK_SHADOW_ETCHED_IN);
			gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(fltrSW),
						      GTK_POLICY_AUTOMATIC,
						      GTK_POLICY_AUTOMATIC);
			fltrVIEW = makeFltrHdrModel();
			g_signal_connect(fltrVIEW, "row-activated", G_CALLBACK(fltrEdit), NULL);
			g_signal_connect(fltrVIEW, "cursor-changed", G_CALLBACK(fltrInfo), NULL);
			gtk_container_add(GTK_CONTAINER(fltrSW), fltrVIEW);
			gtk_box_pack_start(GTK_BOX(hbox1), fltrSW, FALSE, FALSE, 2);
			add_fltr_hdr_columns(GTK_TREE_VIEW(fltrVIEW));
			gtk_widget_set_size_request(fltrSW, 165, -1);
			strcpy(toolTip, "Single-click to VIEW\nDouble-click to EDIT");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), fltrVIEW, toolTip, toolTip);

			vbox = gtk_vbox_new(FALSE, 0);	// holding all right-side widgets

			hbox = gtk_hbox_new(FALSE, 0);
			label = gtk_label_new("Name: ");
			FName = entry = gtk_entry_new();
			gtk_entry_set_max_length(GTK_ENTRY(entry), FNAMELEN);
			gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);
			gtk_box_pack_start(GTK_BOX(hbox), entry, FALSE, FALSE, 2);
			gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 2);

			table = gtk_table_new(3, 3, FALSE);
			gtk_box_pack_start(GTK_BOX(vbox), table, TRUE, TRUE, 2);
			
			label = gtk_label_new("Band");
			gtk_table_attach_defaults(GTK_TABLE(table), label, 0, 1, 0, 1);
			label = gtk_label_new("Cutoff Freq (Hz)");
			gtk_table_attach_defaults(GTK_TABLE(table), label, 1, 2, 0, 1);
			label = gtk_label_new("Poles");
			gtk_table_attach_defaults(GTK_TABLE(table), label, 2, 3, 0, 1);
			label = gtk_label_new("Low:");
			gtk_table_attach_defaults(GTK_TABLE(table), label, 0, 1, 1, 2);
			label = gtk_label_new("High:");
			gtk_table_attach_defaults(GTK_TABLE(table), label, 0, 1, 2, 3);

			cutoff[LOWPOLE][FILTERLIST] = entry = gtk_entry_new();
			gtk_entry_set_max_length(GTK_ENTRY(entry), 10);
			gtk_widget_set_size_request(entry, 20, -1);
			gtk_table_attach_defaults(GTK_TABLE(table), entry, 1, 2, 1, 2);

			cutoff[HIGHPOLE][FILTERLIST] = entry = gtk_entry_new();
			gtk_entry_set_max_length(GTK_ENTRY(entry), 10);
			gtk_widget_set_size_request(entry, 20, -1);
			gtk_table_attach_defaults(GTK_TABLE(table), entry, 1, 2, 2, 3);

			poles[LOWPOLE][FILTERLIST] = gtk_combo_box_new_text();
			poles[HIGHPOLE][FILTERLIST] = gtk_combo_box_new_text();
			for (i = 0; i < NUMPOLES ; i++)
			{
				sprintf(str, "%d", i*2);
				gtk_combo_box_append_text(GTK_COMBO_BOX(poles[LOWPOLE][FILTERLIST]), str);
				gtk_combo_box_append_text(GTK_COMBO_BOX(poles[HIGHPOLE][FILTERLIST]), str);
			}
			gtk_widget_set_size_request(poles[LOWPOLE][FILTERLIST], -1, -1);
			gtk_widget_set_size_request(poles[HIGHPOLE][FILTERLIST], -1, -1);
			gtk_table_attach_defaults(GTK_TABLE(table), poles[LOWPOLE][FILTERLIST], 2, 3, 1, 2);
			gtk_table_attach_defaults(GTK_TABLE(table), poles[HIGHPOLE][FILTERLIST], 2, 3, 2, 3);

			gtk_box_pack_start(GTK_BOX(hbox1), vbox, FALSE, FALSE, 2);
	
			// make the buttons
			hbox = gtk_hbutton_box_new();
			gtk_button_box_set_layout(GTK_BUTTON_BOX (hbox), GTK_BUTTONBOX_SPREAD);

			fltrNEW = button = gtk_button_new_with_label("NEW");
			gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 2);
			strcpy(toolTip, "Make NEW Filter Definition");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), button, toolTip, toolTip);
			g_signal_connect(button, "clicked", G_CALLBACK(fltrCBack), GINT_TO_POINTER(FLTRNEW));
	
			fltrSV = button = gtk_button_new_with_label("SAVE");
			gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 2);
			strcpy(toolTip, "SAVE Filter Definition");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), button, toolTip, toolTip);
			g_signal_connect(button, "clicked", G_CALLBACK(fltrCBack), GINT_TO_POINTER(FLTRSAVE));
	
			fltrDEL = button = gtk_button_new_with_label("DELETE");

			gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 2);
			strcpy(toolTip, "Remove Filter from List");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), button, toolTip, toolTip);
			g_signal_connect(button, "clicked", G_CALLBACK(fltrCBack), GINT_TO_POINTER(FLTRDEL));
	
			button = gtk_button_new_with_label("DONE");
			gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 2);
			strcpy(toolTip, "Return");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), button, toolTip, toolTip);
			g_signal_connect(button, "clicked", G_CALLBACK(fltrCBack), GINT_TO_POINTER(FLTRLSTDONE));

			gtk_box_pack_start(GTK_BOX(vbox1), hbox1, FALSE, FALSE, 2);	
			gtk_box_pack_start(GTK_BOX(vbox1), hbox, FALSE, FALSE, 2);	

			gtk_container_add(GTK_CONTAINER(frame1), vbox1);
		break;
		
		case FLTRAGC:
		{
			char *tmpStr;
			
			size_group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
			box = gtk_vbox_new(FALSE, 0);
			
			label = gtk_label_new("  Window Length: ");
			hbox = gtk_hbox_new(FALSE, 0);
			gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);
			AGC[AGCWINLEN] = entry = gtk_entry_new();
			gtk_box_pack_start(GTK_BOX(hbox), entry, FALSE, FALSE, 2);
			gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 2);
			gtk_size_group_add_widget(size_group, label);
			gtk_misc_set_alignment(GTK_MISC(label), 1, .5);
			tmpStr = g_strdup_printf("%d", settings.general.filter.AGC.winLen);
			gtk_entry_set_text(GTK_ENTRY(entry), tmpStr);
			free(tmpStr);
			strcpy(toolTip, "Number of Points in Moving Window");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), entry, toolTip, toolTip);
			
			label = gtk_label_new("  Window Delta: ");
			hbox = gtk_hbox_new(FALSE, 0);
			gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);
			AGC[AGCDWIN] = entry = gtk_entry_new();
			gtk_box_pack_start(GTK_BOX(hbox), entry, FALSE, FALSE, 2);
			gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 2);
			gtk_size_group_add_widget(size_group, label);
			gtk_misc_set_alignment(GTK_MISC(label), 1, .5);
			tmpStr = g_strdup_printf("%d", settings.general.filter.AGC.dwin);
			gtk_entry_set_text(GTK_ENTRY(entry), tmpStr);
			free(tmpStr);
			strcpy(toolTip, "Number of Points to Move Window");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), entry, toolTip, toolTip);

			label = gtk_label_new("  Detect Energy: ");
			hbox = gtk_hbox_new(FALSE, 0);
			gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);
			radioB1 = gtk_radio_button_new_with_label(NULL, "YES");
			gtk_box_pack_start(GTK_BOX(hbox), radioB1, FALSE, FALSE, 2);
			AGC[AGCDTECT] = radioB2 = gtk_radio_button_new_with_label(
									gtk_radio_button_get_group((GtkRadioButton *) radioB1), "NO");
			gtk_box_pack_start(GTK_BOX(hbox), radioB2, FALSE, FALSE, 2);
			gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 2);
			gtk_size_group_add_widget(size_group, label);
			gtk_misc_set_alignment(GTK_MISC(label), 1, .5);
			strcpy(toolTip, "Display OVERLAP Flags");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), radioB1, toolTip, toolTip);
			strcpy(toolTip, "Do NOT Display OVERLAP Flags");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), radioB2, toolTip, toolTip);
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radioB2), !settings.general.filter.AGC.detect);
			strcpy(toolTip, "Automatically Detect Energy\nIf No Energy Detected, No Gain Applied");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), radioB2, toolTip, toolTip);
			strcpy(toolTip, "Do Not Detect Energy");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), radioB1, toolTip, toolTip);
			
			label = gtk_label_new("  Threshold: ");
			hbox = gtk_hbox_new(FALSE, 0);
			gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);
			AGC[AGCTHRESH] = entry = gtk_entry_new();
			gtk_box_pack_start(GTK_BOX(hbox), entry, FALSE, FALSE, 2);
			gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 2);
			gtk_size_group_add_widget(size_group, label);
			gtk_misc_set_alignment(GTK_MISC(label), 1, .5);
			tmpStr = g_strdup_printf("%.1f", settings.general.filter.AGC.threshold);
			gtk_entry_set_text(GTK_ENTRY(entry), tmpStr);
			free(tmpStr);
			strcpy(toolTip, "Threshold Used in Detecting Energy");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), entry, toolTip, toolTip);

			g_object_unref(size_group);

			// make the button
			hbox = gtk_hbutton_box_new();
			gtk_button_box_set_layout(GTK_BUTTON_BOX(hbox), GTK_BUTTONBOX_SPREAD);

			button = gtk_button_new_with_label("CANCEL");
			gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 2);
			strcpy(toolTip, "Return");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), button, toolTip, toolTip);
			g_signal_connect(button, "clicked", G_CALLBACK(fltrCBack), GINT_TO_POINTER(FLTRAGCCX));
	
			button = gtk_button_new_with_label("DONE");
			gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 2);
			strcpy(toolTip, "SAVE Settings and Return");
			gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), button, toolTip, toolTip);
			g_signal_connect(button, "clicked", G_CALLBACK(fltrCBack), GINT_TO_POINTER(FLTRAGCDONE));

			gtk_container_add(GTK_CONTAINER(box), hbox);	
			gtk_container_add(GTK_CONTAINER(frame1), box);
		}
		break;
	}
	
	return (control_box);
}

// dialogue holding user-defined filters
static void make_filterList()
{
	GtkWidget	*frame, *notebook, *tab;
	GtkWidget	*vbox;

	fltrDialog[FILTERLIST] = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW (fltrDialog[FILTERLIST]), "Filter Management"); 
	gtk_container_set_border_width(GTK_CONTAINER(fltrDialog[FILTERLIST]), 2);
	gtk_window_set_position(GTK_WINDOW(fltrDialog[FILTERLIST]), GTK_WIN_POS_CENTER);

	vbox = gtk_vbox_new(FALSE, 0);
	notebook = gtk_notebook_new ();
	gtk_box_pack_start(GTK_BOX(vbox), notebook, TRUE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(fltrDialog[FILTERLIST]), vbox);

	frame = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
	tab = make_tab(FLTRLIST);
	gtk_container_add(GTK_CONTAINER(frame), tab);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), frame, gtk_label_new(" User Filters "));

	frame = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME (frame), GTK_SHADOW_IN);
	fltrAGCTab = tab = make_tab(FLTRAGC);
	gtk_container_add(GTK_CONTAINER(frame), tab);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), frame, gtk_label_new(" AGC Filter "));

	gtk_widget_show_all(fltrDialog[FILTERLIST]);
	gtk_widget_set_sensitive(fltrSV, FALSE);
	gtk_widget_set_sensitive(FName, FALSE);
	gtk_window_set_transient_for((GtkWindow *) fltrDialog[FILTERLIST], (GtkWindow *) topWindow);

	if (settings.general.display.mode == TRACEMODE)
		gtk_widget_set_sensitive(fltrAGCTab, FALSE);
}

static void make_filterDialogue(int type)
{
	if (!tips)
		tips = gtk_tooltips_new();
		
	switch(type)
	{
		case FILTERLIST:
			if (!fltrDialog[FILTERLIST])
				make_filterList();
			gtk_widget_set_sensitive(fltrSV, FALSE);
			gtk_widget_set_sensitive(FName, FALSE);
			gtk_widget_set_sensitive(fltrDEL, FALSE);
			add_fltrs();
		break;

		case FILTERCUSTOM:
			if (!fltrDialog[FILTERCUSTOM])
				make_filterCustom();
		break;
	}
}

void setFiltCbos()
{
	int		i, j;
	static int	prevNumCBOEntries=0;

	filtrBlk(BLOCKSIGS);
	for(i=0;i<TOTFILTERCBOS;i++)
	{
		// remove all previous entries
		for(j=prevNumCBOEntries; j >= 0 ; j--)
			gtk_combo_box_remove_text(GTK_COMBO_BOX(fltrCbos[i]), j);
		
		// make the new list
		gtk_combo_box_insert_text(GTK_COMBO_BOX(fltrCbos[i]), 0, "Filters");
		gtk_combo_box_set_active(GTK_COMBO_BOX(fltrCbos[i]), 0);
		for(j=0;j<settings.general.filter.numFilters;j++)
		{	// add the new user-defined filters
			gtk_combo_box_append_text(GTK_COMBO_BOX(fltrCbos[i]), settings.general.filter.filts[j]->name);
		}
		gtk_combo_box_append_text(GTK_COMBO_BOX(fltrCbos[i]), FLTEDITSTR);
	}
	filtrBlk(UNBLOCKSIGS);
	prevNumCBOEntries = settings.general.filter.numFilters + 2;
}

static	int filterSelected;

void displayFilter(int filterType, gboolean data)
{
	gchar	*tmpStr;
	filter	*filtPtr;

	if (!fltrDialog[filterType])
	{
		make_filterDialogue(filterType);
		gtk_window_set_transient_for(GTK_WINDOW(fltrDialog[filterType]), GTK_WINDOW(topWindow));
	}
	
	switch (filterType)
	{
		case FILTERCUSTOM:
			filterSelected = FILTERCUSTOMLISTNUM;		// the CUSTOM... selection
		break;
	}
	filtPtr = settings.general.filter.filts[filterSelected];
	
	switch(data)
	{
		case TRUE:
			if (filterType == FILTERLIST)
				gtk_entry_set_text(GTK_ENTRY(FName), filtPtr->name);
			gtk_combo_box_set_active(GTK_COMBO_BOX(poles[HIGHPOLE][filterType]), filtPtr->poles[HIGHF]/2);
			if (filtPtr->cutoff[HIGHF])
			{
				tmpStr = g_strdup_printf("%f", filtPtr->cutoff[HIGHF]);
				gtk_entry_set_text(GTK_ENTRY(cutoff[HIGHPOLE][filterType]), tmpStr);
				free(tmpStr);
			}

			gtk_combo_box_set_active(GTK_COMBO_BOX(poles[LOWPOLE][filterType]), filtPtr->poles[LOWF]/2);
			if (filtPtr->cutoff[LOWF])
			{
				tmpStr = g_strdup_printf("%f", filtPtr->cutoff[LOWF]);
				gtk_entry_set_text(GTK_ENTRY(cutoff[LOWPOLE][filterType]), tmpStr);
				free(tmpStr);
			}
		break;

		case FALSE:
			if (filterType == FILTERLIST)
				gtk_entry_set_text(GTK_ENTRY(FName), "");
			gtk_combo_box_set_active(GTK_COMBO_BOX(poles[HIGHPOLE][filterType]), 0);			// set the value
			gtk_entry_set_text(GTK_ENTRY(cutoff[HIGHPOLE][filterType]), "");
			gtk_combo_box_set_active(GTK_COMBO_BOX(poles[LOWPOLE][filterType]), 0);			// set the value
			gtk_entry_set_text(GTK_ENTRY(cutoff[LOWPOLE][filterType]), "");
		break;
	}
	gtk_widget_show(fltrDialog[filterType]);
	gtk_grab_add(fltrDialog[filterType]);

#if 0	
	switch (filterType)
	{
		case FILTERLIST:
			filterSelected = NOFILTERSELECTION;
		break;
	}
#endif
}

static void fltrInfo(GtkTreeView *treeview, gpointer nil)
{	// called when filter selected, read-only display
	char	*rowS;
	GtkTreePath *path;

	// we have to do some gymnastics to figure out which row it is
	gtk_tree_view_get_cursor(treeview, &path, NULL);
	rowS = gtk_tree_path_to_string(path);
	gtk_tree_path_free(path);
	filterSelected = atoi(rowS);
	g_free(rowS);
	displayFilter(FILTERLIST, TRUE);
	gtk_widget_set_sensitive(fltrSV, FALSE);
	gtk_widget_set_sensitive(fltrNEW, TRUE);
	gtk_widget_set_sensitive(fltrDEL, TRUE);
	gtk_widget_set_sensitive(FName, FALSE);
}

static void fltrEdit(GtkTreeView *treeview, GtkTreePath *arg1, GtkTreeViewColumn *arg2, gpointer nil)
{	// called when filter record double-clicked, make editable
	fltrInfo(treeview, NULL);
	gtk_widget_set_sensitive(fltrSV, TRUE);
}

gint sortFilts(FILTER_REC *one, FILTER_REC *two)
{
	return(strcmp(one->name, two->name));
}

void readFilters()
{
	DBLDI	dbldi;
	DBLRECORDS dblrecs;
	int		j;
	GSList	*filtLst=NULL, *filtIter;
	FILTER_REC *fltPtr;
	GSList	*trcIter;
	traceInfo *trcData;

	if (settings.general.filter.numFilters &&
		settings.general.filter.filts)
	{	// free previous allocations
		for(j=0;j<settings.general.filter.numFilters;j++)
		{
			if (settings.general.filter.filts[j])
				free(settings.general.filter.filts[j]);
		}
		free(settings.general.filter.filts);
		settings.general.filter.filts = NULL;
	}
	for (trcIter = traces; trcIter; trcIter = g_slist_next(trcIter))
	{
		trcData = trcIter->data;
		freeFiltData(trcData);
	}
	
	memset(&dbldi, 0, sizeof(DBLDI));
	memset(&dblrecs, 0, sizeof(DBLRECORDS));
	dbldi.descr.tableData = FILTERST;
	dbldi.descr.dataEvent = READ_ALL;
	dbldi.data = &dblrecs;
	dbl_request(&dbldi);

	for(j=0;j<dblrecs.numRecs;j++)
	{	// insert all returned into singly-linked list
		filtLst = g_slist_prepend(filtLst, dblrecs.recs[j]);
	}
	filtLst = g_slist_sort(filtLst, (GCompareFunc) sortFilts);
	if (dblrecs.numRecs)
		free(dblrecs.recs);

	settings.general.filter.numFilters = dblrecs.numRecs+1;			// one extra for Custom
	if (settings.general.display.mode == GATHERMODE)
	{
		settings.general.filter.numFilters++;						// one extra for AGC
	}
	settings.general.filter.filts = calloc(settings.general.filter.numFilters, sizeof(void *));

	for(filtIter = filtLst, j=0;
		filtIter;
		filtIter = g_slist_next(filtIter), j++)
	{
		fltPtr = (FILTER_REC *) (filtIter->data);
		settings.general.filter.filts[j] = calloc(1, sizeof(filter));
		strcpy(settings.general.filter.filts[j]->name, fltPtr->name);
		settings.general.filter.filts[j]->poles[HIGHF] = fltPtr->poles[HIGHF];
		settings.general.filter.filts[j]->cutoff[HIGHF] = fltPtr->cutoff[HIGHF];
		settings.general.filter.filts[j]->poles[LOWF] = fltPtr->poles[LOWF];
		settings.general.filter.filts[j]->cutoff[LOWF] = fltPtr->cutoff[LOWF];
		free(fltPtr);
	}
	g_slist_free(filtLst);
	
	if (settings.general.display.mode == GATHERMODE)
	{
		settings.general.filter.filts[j] = calloc(1, sizeof(filter));
		strcpy(settings.general.filter.filts[j]->name, FLTAGCSTR);
		settings.general.filter.AGCidx = j;
		j++;
	}
	else
		settings.general.filter.AGCidx = NONE;
	settings.general.filter.filts[j] = calloc(1, sizeof(filter));
	strcpy(settings.general.filter.filts[j]->name, FLTCUSTSTR);
	states.tForm.filter = NOFILTERSELECTION;
}

static void fltrCBack(GtkButton *button, gpointer t)
{	// called on all filter dialogue buttons
	int		i, type = GPOINTER_TO_INT(t);
	char		tmpStr[25];
	filter	*filtPtr = settings.general.filter.filts[filterSelected];

	switch(type)
	{
		case FLTRNEW:
			filterSelected = NOFILTERSELECTION;
			displayFilter(FILTERLIST, FALSE);
			gtk_widget_set_sensitive(fltrSV, TRUE);
			gtk_widget_set_sensitive(fltrNEW, FALSE);
			gtk_widget_set_sensitive(fltrDEL, FALSE);
			gtk_widget_set_sensitive(FName, TRUE);
			gtk_widget_grab_focus(FName);
		break;

		case FLTRSAVE:
		{
			DBLDI	dbldi;
			DBLRECORDS dblrecs;
			FILTER_REC dataRec;

			gtk_widget_set_sensitive(fltrSV, FALSE);
			gtk_widget_set_sensitive(fltrNEW, TRUE);
			gtk_widget_set_sensitive(FName, FALSE);

			memset(&dbldi, 0, sizeof(DBLDI));
			memset(&dblrecs, 0, sizeof(DBLRECORDS));
			dbldi.descr.tableData = FILTERST;
			dbldi.descr.dataEvent = WRITE_SNGL;
			dbldi.data = &dblrecs;
			dblrecs.recs = calloc(1, sizeof(void *));
			dblrecs.recs[0] = &dataRec;
			strcpy(dataRec.name, gtk_entry_get_text(GTK_ENTRY(FName)));
			dataRec.poles[HIGHF] = gtk_combo_box_get_active(GTK_COMBO_BOX(poles[HIGHPOLE][FILTERLIST]))*2;
			strcpy(tmpStr, gtk_entry_get_text(GTK_ENTRY(cutoff[HIGHPOLE][FILTERLIST])));
			sscanf(tmpStr, "%f", &dataRec.cutoff[HIGHF]);
			dataRec.poles[LOWF] = gtk_combo_box_get_active(GTK_COMBO_BOX(poles[LOWPOLE][FILTERLIST]))*2;
			strcpy(tmpStr, gtk_entry_get_text(GTK_ENTRY(cutoff[LOWPOLE][FILTERLIST])));
			sscanf(tmpStr, "%f", &dataRec.cutoff[LOWF]);

			dbl_request(&dbldi);
			free(dblrecs.recs);
			listChgd = TRUE;
			readFilters();
			add_fltrs();
			displayFilter(FILTERLIST, FALSE);
		}
		break;

		case FLTRDEL:
		{
			DBLDI	dbldi;
			DBLRECORDS dblrecs;
			FILTER_REC dataRec;

			memset(&dbldi, 0, sizeof(DBLDI));
			memset(&dblrecs, 0, sizeof(DBLRECORDS));
			dbldi.descr.tableData = FILTERST;
			dbldi.descr.dataEvent = DELETE_SNGL;
			dbldi.data = &dblrecs;
			dblrecs.recs = calloc(1, sizeof(void *));
			dblrecs.recs[0] = &dataRec;
			strcpy(dataRec.name, filtPtr->name);

			dbl_request(&dbldi);
			free(dblrecs.recs);
			listChgd = TRUE;
			readFilters();
			add_fltrs();
			displayFilter(FILTERLIST, FALSE);
		}
		break;

		case FLTRLSTDONE:
			gtk_grab_remove(fltrDialog[FILTERLIST]);
			gtk_widget_hide(fltrDialog[FILTERLIST]);
			if (listChgd)
			{
				setFiltCbos();
#ifndef PQL_ONLY
void setStnFiltCbos();
				setStnFiltCbos();
#endif
				listChgd=FALSE;
			}
			for(i=0;i<TOTFILTERCBOS;i++)
				gtk_combo_box_set_active((GtkComboBox *) fltrCbos[i], NOFILTERSELECTION);
			states.tForm.filter = NOFILTERSELECTION;
			if (states.tForm.on)
			{
				states.tForm.on = FALSE;
				switch(states.currentScreen)
				{
					case MAIN:
						states.reset.reset[states.currentScr] = TRUE;
						makePixmap(states.currentScr);
					break;
					
					case SPLIT2:
						states.split.resetPixmap[states.split.panes][SPLITDA2T-SPLITDA2T] = TRUE;
						states.split.resetPixmap[states.split.panes][SPLITDA2B-SPLITDA2T] = TRUE;
						makePixmap(SPLITDA2T);
						makePixmap(SPLITDA2B);
					break;

					case SPLIT3:
						states.split.resetPixmap[states.split.panes][SPLITDA3T-SPLITDA3T] = TRUE;
						states.split.resetPixmap[states.split.panes][SPLITDA3M-SPLITDA3T] = TRUE;
						states.split.resetPixmap[states.split.panes][SPLITDA3B-SPLITDA3T] = TRUE;
						makePixmap(SPLITDA3T);
						makePixmap(SPLITDA3M);
						makePixmap(SPLITDA3B);
					break;
				}
			}
		break;

		case FLTRCSTCONT:		// custom dialog, continue button
			// high-pass filter info
			filtPtr->poles[HIGHF] = gtk_combo_box_get_active(GTK_COMBO_BOX(poles[HIGHPOLE][FILTERCUSTOM]))*2;
			if (filtPtr->poles[HIGHF])
			{
				strcpy(tmpStr, gtk_entry_get_text(GTK_ENTRY(cutoff[HIGHPOLE][FILTERCUSTOM])));
				sscanf(tmpStr, "%f", &filtPtr->cutoff[HIGHF]);
			}
			// low-pass filter info
			filtPtr->poles[LOWF] = gtk_combo_box_get_active(GTK_COMBO_BOX(poles[LOWPOLE][FILTERCUSTOM]))*2;
			if (filtPtr->poles[LOWF])
			{
				strcpy(tmpStr, gtk_entry_get_text(GTK_ENTRY(cutoff[LOWPOLE][FILTERCUSTOM])));
				sscanf(tmpStr, "%f", &filtPtr->cutoff[LOWF]);
			}

			gtk_grab_remove(fltrDialog[FILTERCUSTOM]);
			gtk_widget_hide(fltrDialog[FILTERCUSTOM]);
#ifndef PQL_ONLY
			switch (pqlxStates.system)
			{
				case PQLSYSTEM:
					states.tForm.on = TRUE;
					g_idle_add((GSourceFunc) idleFilter, (gpointer) NULL);
				break;
				case STNSYSTEM:
					g_idle_add((GSourceFunc) stnIdleFilter, GINT_TO_POINTER(FILTERCUSTOMLISTNUM));
				break;
			}
#else
			states.tForm.on = TRUE;
			g_idle_add((GSourceFunc) idleFilter, (gpointer) NULL);
#endif
		break;


		case FLTRCSTCX:
			gtk_grab_remove(fltrDialog[FILTERCUSTOM]);
			gtk_widget_hide(fltrDialog[FILTERCUSTOM]);
		break;
		
		case FLTRAGCCX:
			gtk_grab_remove(fltrDialog[FILTERLIST]);
			gtk_widget_hide(fltrDialog[FILTERLIST]);
		break;
		
		case FLTRAGCDONE:
		{
			char	*tmpStr;
			
			for(i=0;i<TOTFILTERCBOS;i++)
				gtk_combo_box_set_active((GtkComboBox *) fltrCbos[i], NOFILTERSELECTION);
			states.tForm.filter = NOFILTERSELECTION;

			gtk_grab_remove(fltrDialog[FILTERLIST]);
			gtk_widget_hide(fltrDialog[FILTERLIST]);
			
			tmpStr = g_strdup(gtk_entry_get_text(GTK_ENTRY(AGC[AGCWINLEN])));
			sscanf(tmpStr, "%d", &settings.general.filter.AGC.winLen);
			free(tmpStr);
			tmpStr = g_strdup(gtk_entry_get_text(GTK_ENTRY(AGC[AGCDWIN])));
			sscanf(tmpStr, "%d", &settings.general.filter.AGC.dwin);
			free(tmpStr);
			tmpStr = g_strdup(gtk_entry_get_text(GTK_ENTRY(AGC[AGCTHRESH])));
			sscanf(tmpStr, "%f", &settings.general.filter.AGC.threshold);
			free(tmpStr);
			settings.general.filter.AGC.detect = 
				!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(AGC[AGCDTECT]));
		}
		break;
	}
}
