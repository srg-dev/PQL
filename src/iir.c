#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include "pql_defines.h"
#include "pql_externs.h"

int iir(traceInfo *trc, int active_filter)
{
	trc->data.dataInfo.head.data_form = trc->data.head.data_form;
	return(_iir(&trc->data.dataInfo, g_slist_nth_data(trc->data.dataFilt, active_filter), active_filter));
}

