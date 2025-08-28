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

extern GtkTooltips *tips;
static char		toolTip[1000];
static GtkWidget	*sortPopup, *choiceList;
static GtkWidget	*sortChoice[MAXTTLSORTCRITERIA];
static GtkWidget	*sortCriteria[MAXTTLSORTCRITERIA];
static GtkWidget	*sortDirs[MAXTTLSORTCRITERIA];
static void newSort(GtkWidget *widget, void *nothing);
static void makeSortCombo(GtkWidget *vbox3, int element);

static void sortCritCBack(GtkWidget *widget, gpointer c)
{	// called upon selection of a sort criteria
	int 	critNum = GPOINTER_TO_INT(c);
	int		sortT = ctrl_settings.general.sort.selected;
	int 	i, selNum;
	char	*selStr;
	CRITITEM *crit;
	CRITDEF	*critDef;
	GSList	*critIter;
	SORTINFO	*sortInfo = g_slist_nth_data(ctrl_settings.general.sort.sorts, sortT);

	// find the string that's been selected
	selStr = g_strdup(gtk_combo_box_get_active_text((GtkComboBox *) widget));
	for(selNum=0;selNum<g_slist_length(sortInfo->criteria);selNum++)
	{	// find the criteria that's been selected
		crit = g_slist_nth_data(sortInfo->criteria, selNum);
		if (!strcmp(selStr, crit->desc))
			break;
	}
	crit->selected = TRUE;
	free(selStr);

	// create the CRITDEF user entry defining this selection
	critDef = calloc(1, sizeof(CRITDEF));
	critDef->num = selNum;
	critDef->dir = !gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(sortDirs[critNum]));
	sortInfo->user = g_slist_append(sortInfo->user, critDef);

	// remove all other text from the just-selected combo box
	g_signal_handler_block(sortCriteria[critNum], signals[SORTSGNL][critNum]);
	for (i = 0;
		 i < g_slist_length(sortInfo->criteria)-critNum;
		 i++)
	{
		gtk_combo_box_remove_text((GtkComboBox *) sortCriteria[critNum], 0);
	}
	// and add back the selection
	gtk_combo_box_append_text((GtkComboBox *) sortCriteria[critNum], crit->desc);
	gtk_combo_box_set_active(GTK_COMBO_BOX(sortCriteria[critNum]), 0);
	g_signal_handler_unblock(sortCriteria[critNum], signals[SORTSGNL][critNum]);

	if (crit->unique == TRUE)
	{	// if unique, don't allow more choices
		return;
	}

	makeSortCombo(choiceList, critNum+1);
	// construct next criteria option list
	for(critIter = sortInfo->criteria;
		critIter;
		critIter = g_slist_next(critIter))
	{	
		crit = critIter->data;
		if (crit->selected || crit->unique || !crit->desc[0])
			continue;	// don't offer choice if already picked or is unique or doesn't really exist
		gtk_combo_box_append_text((GtkComboBox *) sortCriteria[critNum+1], crit->desc);
	}

	// undone: don't create sort option when no more exist
	g_signal_handler_block(sortCriteria[critNum+1], signals[SORTSGNL][critNum+1]);
	gtk_combo_box_set_active(GTK_COMBO_BOX(sortCriteria[critNum+1]), -1);
	g_signal_handler_unblock(sortCriteria[critNum+1], signals[SORTSGNL][critNum+1]);
	gtk_widget_show_all(choiceList);

	return;
}

static void makeSortCombo(GtkWidget *vbox3, int element)
{
	GtkWidget	*hbox, *vbbox, *vbox2, *combo, *radioB, *radioB2, *label;
	static GtkSizeGroup	*size_group;
	char		*str;
	
	if (!size_group)
		size_group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	sortChoice[element] = hbox = gtk_hbox_new(FALSE, 0);
	str = g_strdup_printf("Criteria #%d: ", element+1);
	label = gtk_label_new(str);		free(str);
	sortCriteria[element] = combo = gtk_combo_box_new_text();
	signals[SORTSGNL][element] = 
			g_signal_connect(combo, "changed", G_CALLBACK(sortCritCBack), GINT_TO_POINTER(element));
	vbbox = gtk_vbutton_box_new();
	gtk_button_box_set_layout(GTK_BUTTON_BOX(vbbox), GTK_BUTTONBOX_SPREAD);
	gtk_container_add(GTK_CONTAINER(vbbox), combo);
	gtk_size_group_add_widget(size_group, combo);

	vbox2 = gtk_vbox_new(FALSE, 0);
	radioB = gtk_radio_button_new_with_label(NULL, "Ascending");
	gtk_box_pack_start(GTK_BOX(vbox2), radioB, FALSE, FALSE, 0);
	sortDirs[element] = radioB2 = gtk_radio_button_new_with_label(
							gtk_radio_button_get_group((GtkRadioButton *) radioB), "Descending");
	gtk_box_pack_start(GTK_BOX(vbox2), radioB2, FALSE, FALSE, 0);

	gtk_box_pack_start(GTK_BOX(sortChoice[element]), label, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(sortChoice[element]), vbbox, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(sortChoice[element]), vbox2, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox3), hbox, FALSE, FALSE, 2);
}

void sortList(GSList **list, int sortCrit)
{
	int i;
	GSList	*trcIter;
	traceInfo *trcData;

	SORTPARAMS sortParams;
	sortParams.type = SORTDIRECT;
	sortParams.sortCrit = sortCrit;
	*list = g_slist_sort_with_data(*list, (GCompareDataFunc) listSort, &sortParams);
	for(trcIter = *list, i=states.trace.startRec; 
		trcIter; 
		trcIter = g_slist_next(trcIter), i++)
	{	// set our current position number in the list
		trcData = trcIter->data;
		trcData->data.position.current = i;
	}
}

void sortTraceList()
{
	int			i;
	GSList		*trcIter;
	traceInfo	*trcData;
	SORTPARAMS	sortParams;
	SORTINFO	*sortInfo = g_slist_nth_data(settings.general.sort.sorts, settings.general.sort.selected);
	
	sortParams.type = SORTUSER;
	for (i=g_slist_length(sortInfo->user)-1; i>=0 ;i--)
	{	// for each criteria chosen by the user, sort in backwards order
		sortParams.sortCrit = i;
		traces = g_slist_sort_with_data(traces, (GCompareDataFunc) listSort, &sortParams);
	}
	
	for(trcIter = traces, i=0; 
		trcIter; 
		trcIter = g_slist_next(trcIter), i++)
	{	// set our current position number in the list
		trcData = trcIter->data;
		trcData->data.position.current = i;
	}
}

void sortTraces(GtkWidget *widget, gpointer s)
{	// called when Sort button on controls screen selected, and at startup if sorting defined
	gboolean	startUp = (gboolean) GPOINTER_TO_INT(s);

	selectT(NULL, GINT_TO_POINTER(DESELECT));	// clean up selection list
	sortTraceList();

	states.reset.reset[TRACESCR] = TRUE;
	switch(states.currentPage)
	{
		case NONE:		// nowhere to return to, wah!
		break;

		case HEADERSCR:	// return to header tab
			makeDispList(INITGROUP);
			raise_page(NULL, &states.currentPage);	// return to HEADERSCR
		break;

		default:		// return to Trace tab
			restartTrace(NULL, GINT_TO_POINTER(0));	// start traces over from new first
			if (!startUp)							// when called via SORT Button on controls panel
				raise_page(NULL, &OV[TRACESCR]);	// return to TRACESCR
		break;
	}
}

static void delSort(GtkWidget *widget, void *nothing)
{
	GSList	*iter;
	CRITDEF	*critDef;
	int		sortT = ctrl_settings.general.sort.selected, i;
	SORTINFO	*sortInfo = g_slist_nth_data(ctrl_settings.general.sort.sorts, sortT);
	
	// re-read the directions in case they've been changed
	for (iter = sortInfo->user, i=0;
		 iter;
		 iter = g_slist_next(iter), i++)
	{
		critDef = iter->data;
		critDef->dir = !gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(sortDirs[i]));
	}
	gtk_widget_destroy(sortPopup);
	sortPopup = NULL;
}

static gboolean destroySortControls(GtkWidget *widget, int *nothing)
{
	delSort(NULL, NULL);
	return TRUE;
}

void makeSortPopup(GtkWidget *widget, void *nothing)
{
	int			sortType = ctrl_settings.general.sort.selected;
	SORTINFO	*sortInfo = g_slist_nth_data(ctrl_settings.general.sort.sorts, sortType);
	int			i;
	GtkWidget	*vbox3, *vbox, *ctrlBox, *button, *actionB, *window;
	GSList		*iter, *critIter;
	CRITDEF		*critDef;
	CRITITEM	*crit;

	if (sortPopup)
		delSort(NULL, NULL);

	sortPopup = window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (window), "PQL II - Sort Criteria"); 
	gtk_container_set_border_width(GTK_CONTAINER(window), 2);
	gtk_window_set_position(GTK_WINDOW (window), GTK_WIN_POS_CENTER);
	g_signal_connect(window, "delete-event", G_CALLBACK(destroySortControls), NULL);

	ctrlBox = gtk_hbox_new(FALSE, 0);
	gtk_container_add (GTK_CONTAINER(window), ctrlBox);
	
	choiceList = vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add (GTK_CONTAINER(ctrlBox), vbox);

	vbox3 = gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), vbox3, FALSE, FALSE, 2);
	for (i=0;i<g_slist_length(sortInfo->user);i++)
	{
		makeSortCombo(vbox3, i);
	}
	
	actionB = gtk_hbutton_box_new();
	gtk_button_box_set_layout(GTK_BUTTON_BOX(actionB), GTK_BUTTONBOX_SPREAD);
	gtk_box_set_spacing(GTK_BOX(actionB), 2);
	gtk_box_pack_end(GTK_BOX(vbox), actionB, FALSE, FALSE, 2);

	button = gtk_button_new_with_label("New Sort");
	g_signal_connect(button, "clicked", G_CALLBACK(newSort), NULL);
	gtk_container_add(GTK_CONTAINER(actionB), button);
	strcpy(toolTip, "CLEAR Sort Criteria for NEW SORT");
	gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), button, toolTip, toolTip);
	
	button = gtk_button_new_with_label("Return");
	g_signal_connect(button, "clicked", G_CALLBACK(delSort), NULL);
	gtk_container_add(GTK_CONTAINER(actionB), button);
	strcpy(toolTip, "USE Defined Sort Criteria");
	gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), button, toolTip, toolTip);
	
	switch (g_slist_length(sortInfo->user))
	{
		case 0:		// nothing defined by user - set up first combox box of sort criteria possibilities
			makeSortCombo(vbox3, i);
			g_signal_handler_block((sortCriteria[0]), signals[SORTSGNL][0]);
			for(critIter = sortInfo->criteria;
				critIter;
				critIter = g_slist_next(critIter))
			{	
				crit = critIter->data;
				if (crit->desc[0])
					gtk_combo_box_append_text((GtkComboBox *) sortCriteria[0], crit->desc);
				crit->selected = FALSE;
			}
			gtk_combo_box_set_active(GTK_COMBO_BOX(sortCriteria[0]), -1);
			g_signal_handler_unblock((sortCriteria[0]), signals[SORTSGNL][0]);
		break;
		default:	// add the user's current definition
			for (iter = sortInfo->user, i = 0;
				 iter;
				 iter = g_slist_next(iter), i++)
			{
				critDef = iter->data;
				crit = g_slist_nth_data(sortInfo->criteria, critDef->num);
				g_signal_handler_block((sortCriteria[i]), signals[SORTSGNL][i]);
				gtk_combo_box_append_text(GTK_COMBO_BOX(sortCriteria[i]), crit->desc);
				gtk_combo_box_set_active(GTK_COMBO_BOX(sortCriteria[i]), 0);
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(sortDirs[i]), !critDef->dir);
				g_signal_handler_unblock((sortCriteria[i]), signals[SORTSGNL][i]);
			}
		break;
	}
	
	gtk_window_set_transient_for(GTK_WINDOW (sortPopup), GTK_WINDOW (ctrlWindow));
	gtk_widget_show_all(sortPopup);
}

static void newSort(GtkWidget *widget, void *nothing)
{
	int 		sortT;
	GSList		*critIter;
	SORTINFO	*sortInfo;

	sortT = gtk_combo_box_get_active(GTK_COMBO_BOX(sortType));
	sortInfo = g_slist_nth_data(ctrl_settings.general.sort.sorts, sortT);
	for (critIter = sortInfo->user;
		 critIter;
		 critIter = g_slist_next(critIter))
	{
		free(critIter->data);
	}
	g_slist_free(sortInfo->user);
	sortInfo->user = NULL;

	makeSortPopup(NULL, NULL);
	return;
}

