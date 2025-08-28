#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "pql_defines.h"
#include "pql_externs.h"
#include "gtk_pql_externs.h"
#include "db_iface.h"
#include "dbl_iface.h"

void displayFilter(int, gboolean);

void fltrSelect(GtkComboBox *combo, gpointer nil)
{	// callback when filter combo box item selected by user
	int 		i, filtType, active=gtk_combo_box_get_active(combo);

	if (active == FILTTITLE)		// active == 0 is our title, to be ignored
		return;

	states.tForm.filter = active-1;
	filtType = FILTNORMAL;
	if (states.tForm.filter == FILTERCUSTOMLISTNUM)
		filtType = FILTCUSTOM;
	if (states.tForm.filter == settings.general.filter.numFilters)
		filtType = FILTEDIT;

	switch (filtType)
	{
		case FILTEDIT:		// EDIT... selected
			states.tForm.on = OFF;
			makePixmap(MAGNIFYSCR);
			setRadios(FILTERTYPE);
			displayFilter(FILTERLIST, FALSE);
			return;
		break;
		case FILTCUSTOM:		// CUSTOM... selected
			displayFilter(FILTERCUSTOM, TRUE);
		break;
		case FILTNORMAL:
			states.tForm.on = TRUE;
			g_idle_add((GSourceFunc) idleFilter, (gpointer) NULL);
		break;
	}

	for(i=0;i<TOTFILTERCBOS;i++)
	{	// set filters the same on all other screens
		gtk_combo_box_set_active((GtkComboBox *) fltrCbos[i], FILTTITLE);
	}
}
