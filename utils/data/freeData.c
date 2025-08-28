#include <mysql.h>

#include "pqlx_defs.h"
#include "pql_defines.h"
#include "pqlx_stn.h"

void freeData2(traceInfo *);

void freeFiltData(traceInfo *trc)
{
	GSList		*dataFiltIter;
	DATAINFO	*dataInfo;
	
	dataFiltIter = trc->data.dataFilt;
	while(dataFiltIter)
	{
		dataInfo = dataFiltIter->data;
		if (dataInfo && dataInfo->data)
			free(dataInfo->data);
		dataFiltIter = g_slist_next(dataFiltIter);
	}
	g_slist_free(trc->data.dataFilt);
	trc->data.dataFilt = NULL;
}

void freeMyData(int type, void *data)
{
	switch(type)
	{
		case TRC1DATA:
		{
			TRC1	*trc1 = data;
			if (trc1->data.gaps)
				free(trc1->data.gaps);
			if (trc1->data.olaps)
				free(trc1->data.olaps);
		}
		break;

		case PSD1DATA:
		{
			PSD1	*psd1 = data;
			if (psd1->data.period)
				free(psd1->data.period);
			if (psd1->data.power)
				free(psd1->data.power);
			if (psd1->keys.conf.units)
				free (psd1->keys.conf.units);
		}
		break;

		case TRCINFO:
		{
			freeData2((traceInfo*)data);
		}
		break;

		case ANAINFO:
		{
			ANADATA	*anaData = data;
			GSList	*iter;
			if (anaData->original)
				free(anaData->original);
			anaData->original = NULL;
			if (anaData->dataInfo.data)
				free(anaData->dataInfo.data);
			anaData->original = NULL;
			for(iter=anaData->chnMETA;iter;iter=g_slist_next(iter))
				free(iter->data);
			if (anaData->chnMETA)
				g_slist_free(anaData->chnMETA);
			anaData->chnMETA = NULL;
			for(iter=anaData->userPicks;iter;iter = g_slist_next(iter))
				free(iter->data);
			if (anaData->userPicks)
				g_slist_free(anaData->userPicks);
			anaData->userPicks = NULL;
		}
		break;
	}

	return;
}
