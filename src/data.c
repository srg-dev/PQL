#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "pql_defines.h"
#include "pql_externs.h"
#include "gtk_pql_externs.h"

static	int	prev_numPlots, prev_start_rec;

static GSList *cutTrace(traceInfo *trc);
int	readTrace(traceInfo *);
int checkTrace(traceInfo *);
void readFiles(GQueue *f);
void readTraces(GQueue *f);
static void	makeList();
gboolean _openTraces(gpointer nil);

void readHdr_2(GSList *readTraces)
{
	int	file_num = g_slist_length(traces) + 1;
	GSList *tracesIter;
	traceInfo	*trc;

	if (!settings.general.input.shotTimes)
	{	// take the traces as returned
		for(tracesIter = readTraces; 
			tracesIter; 
			tracesIter = g_slist_next(tracesIter), file_num++)
		{
			((traceInfo *) (tracesIter->data))->data.position.original = file_num;
			traces = g_slist_prepend(traces, tracesIter->data);
		}
	} 
	else 
	{	// traces must be cut up as per input list of start times
		GSList	*cutTraces, *cutTracesIter;
		int		read;
		for(tracesIter = readTraces; 
			tracesIter; 
			tracesIter = g_slist_next(tracesIter))
		{
			trc = tracesIter->data;
			read = readTrace(trc);			// read the data to get the length
			switch(read)
			{
				case FAILURE:
					fprintf(stderr, "File %s cannot be read, removing...\n", 
								trc->data.fileName);
				break;
				case NOT_ENOUGH_DATA:
					switch(trc->data.fileType)
					{
						case RT125A_DATA:
						case RT125_DATA:
							fprintf(stderr, "Shot contains no data, removing... ( %s )\n", 
										trc->data.head.shortFN);
						break;
						default:
							fprintf(stderr, "File %s contains no data, removing...\n", 
										trc->data.fileName);
						break;
					}
				break;
			}
			if (read != PQLXSUCCESS)
			{	// file no good, proceed to next
				freeData2(trc);
				free(trc);
				continue;
			}
			freeData(trc, FALSE);
			cutTraces = cutTrace(trc);
			for(cutTracesIter = cutTraces; 
				cutTracesIter; 
				cutTracesIter = g_slist_next(cutTracesIter), file_num++)
			{
				((traceInfo *) (cutTracesIter->data))->data.position.original = file_num;
				traces = g_slist_prepend(traces, cutTracesIter->data);
			}
			freeData2(trc);
			free(trc);
			if (g_slist_length(cutTraces))
			{
//				numTracesIn += g_slist_length(cutTraces);
				g_slist_free(cutTraces);
			}
			else
			{	// no qualifying excerpts
				continue;
			}
		}
	}
}

void readHdr_3()
{
	PQLDATA	*pqlIn;
	GSList	*fileIter;
	
 	for (fileIter = states.input.Files; 
		 fileIter; 
		 fileIter = g_slist_next(fileIter))
	{	// clear the list of input files, we don't need it anymore
		// (and, more importantly, downstream logic keys off of empty states.input.Files list)
		pqlIn = fileIter->data;
		if (pqlIn->fileName)
			free(pqlIn->fileName);
	}
	g_slist_free(states.input.Files);
	states.input.Files=NULL;
	states.input.numFiles = 0;
}


gboolean readHdr_finish(GSList *tracesIn)
{
	int		numTracesIn = 0;
	
	if (tracesIn)
	{
		numTracesIn = g_slist_length(tracesIn);
		readHdr_2(tracesIn);
		g_slist_free(tracesIn);
	}
	else
	{	// no valid files found, try again...
		static GtkWidget *dialog=NULL; 
		GtkWidget *label;
		readHdr_3();
		if (!dialog)
		{
			dialog = gtk_dialog_new_with_buttons ("OPEN",
											GTK_WINDOW (topWindow),
											GTK_DIALOG_DESTROY_WITH_PARENT,
											GTK_STOCK_OK, GTK_RESPONSE_OK,
											NULL);

			label = gtk_label_new(NULL);
			gtk_label_set_markup(GTK_LABEL(label), 
				(const gchar *) "<span font_desc=\"12\">\n\tNO TRACE Files Identified for Display\t\n"\
								"\tSee Console for Specific Error Messages\t\n</span>");

			gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), label);
			gtk_widget_show_all(dialog);
		}
		else
		{
			gtk_widget_show(dialog);
		}
		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_hide(dialog);
		g_idle_add((GSourceFunc) _openTraces, NULL);
		return FALSE;
	}

	readHdr_3();
	
	states.general.numTraces += numTracesIn;
	states.reset.cursor = TRUE;
	checkLogTab();
	if (criteria[0].desc[0] != 'S' &&	// != "Sort Order"
		settings.general.display.mode == TRACEMODE)
	{
		sortTraces(NULL, GINT_TO_POINTER(TRUE));
	}

	states.trace.startRec = 0;		// start plotting from trace #1
	makeDispList(INITGROUP);
	states.general.plot.state = READDATA;
	g_idle_add((GSourceFunc) _makePixmap, GINT_TO_POINTER(TRACESCR));
	
	return FALSE;
}

void readHdr_1()
{
  	int		i, j;
	GSList	*fileIter, *tracesIn=NULL;
	PQLDATA	*pqlIn;
	static GQueue	*fileQ;

	if (fileQ)
		g_queue_free(fileQ);
	fileQ = g_queue_new();

  	for (i = j = 0, fileIter = states.input.Files; 
  		fileIter; 
  		fileIter = g_slist_next(fileIter), i++)
	{
		pqlIn = fileIter->data;
		g_queue_push_head(fileQ, pqlIn);
	}
	
	if (g_queue_get_length(fileQ))
	{
		states.reset.cursor = TRUE;
		states.general.plot.state = READHDRS;
		g_idle_add((GSourceFunc) _makePixmap, GINT_TO_POINTER(TRACEDA));
		readFiles(fileQ);						// multi-threaded, see thread_file.c for details
	}
	else	// either traces from elsewhere, or none read in...
	{
		states.reset.cursor = TRUE;
		states.general.plot.state = DATADONE;
		g_idle_add((GSourceFunc) _makePixmap, GINT_TO_POINTER(TRACEDA));
		readHdr_finish(tracesIn);
	}
}

void importTraces()
{
	GSList	*pqlIter, *tracesIn=NULL;
	PQLDATA	*pqlIn;
	traceInfo	*trc;
	int	fileNum = g_slist_length(traces) + 1, numTracesIn;

  	for (pqlIter = states.input.Files; 
  		 pqlIter; 
  		 pqlIter = g_slist_next(pqlIter))
	{
		pqlIn = pqlIter->data;
		if (pqlIn->trc)
		{	// trace is being handed to PQL, assign and continue
			trc = pqlIn->trc;
			trc->data.position.original = fileNum++;
			tracesIn = g_slist_prepend(tracesIn, trc);
			switch (states.general.sourceSystem)
			{
				case STNSYSTEM:
					// trc header and data already in memory
					// nothing to do except get next record
				break;
				case PDFSYSTEM:
					// trc pre-defined, but we need to read the header info
					checkTrace(trc);
				break;
			}
		}
	}
	numTracesIn = g_slist_length(tracesIn);
	readHdr_2(tracesIn);
	g_slist_free(tracesIn);
	readHdr_3();
	states.general.numTraces += numTracesIn;
	states.reset.cursor = TRUE;
	states.trace.startRec = 0;		// start plotting from trace #1
	makeDispList(INITGROUP);
	states.general.plot.state = READDATA;
}	

gboolean readData_finish(gpointer nil)
{
	prev_numPlots = settings.trace.numPlots;
	prev_start_rec = states.trace.startRec;

	states.general.plot.state = DATAPLOT;
	g_idle_add((GSourceFunc) _makePixmap, GINT_TO_POINTER(TRACESCR));

	return FALSE;
}

void readData_1()
{

/*

read in corresponding data for each trace to be
plotted onto the main screen.  each array for data is allocated
each time the data are read in.  before reading in new 
data (via the next or prev buttons on main screen) the space used to store the
old data is freed.

*/

	traceInfo	*trcData;
	GSList		*trcIter;
	static GQueue	*traceQ;

	if (traceQ)
		g_queue_free(traceQ);
	traceQ = g_queue_new();

	makeList();

	states.trace.plot.dispScale = 0;
	for(trcIter = states.trace.traces[CURGRP];
		trcIter;
		trcIter = g_slist_next(trcIter))
	{
		trcData = trcIter->data;
#if 0
		switch (trcData->data.fileType) 
		{
			case SEGY_DATA:
				states.trace.plot.dispScale |= (MYVOLTS+1);
				states.trace.plot.dispScale |= (MYCOUNTS+1);
			break;

			case AH_DATA:
				states.trace.plot.dispScale |= (MYVOLTS+1);
			break;

			case SAC_DATA:
				states.trace.plot.dispScale |= (MYVOLTS+1);
			break;

			case MSEED_DATA:
			case CWB_DATA:
				states.trace.plot.dispScale |= (MYCOUNTS+1);
			break;

			case NANO_DATA:
				states.trace.plot.dispScale |= (MYVOLTS+1);
			break;

			case DR100_DATA:
				states.trace.plot.dispScale |= (MYCOUNTS+1);
			break;
		}
#endif

		if (trcData->data.dataInfo.data)
			continue;	// read it in only if it doesn't exist
		
		g_queue_push_head(traceQ, trcData);
	}
	
	if (g_queue_get_length(traceQ))
	{
		readTraces(traceQ);			// multi-threaded, see thread_data.c for details
	}
	else
	{
		readData_finish(NULL);
	}
	
	return;
}

#ifdef PQL_ONLY
void freeFiltData(traceInfo *trc)
{
	GSList		*dataFiltIter;
	DATAINFO	*dataInfo;
	
	dataFiltIter = trc->data.dataFilt;
	while(dataFiltIter)
	{
		dataInfo = dataFiltIter->data;
		if (dataInfo->data)
			free(dataInfo->data);
		dataFiltIter = g_slist_next(dataFiltIter);
	}
	g_slist_free(trc->data.dataFilt);
	trc->data.dataFilt = NULL;
}
#endif

void freeData(traceInfo *trc, int freeStruct)
{
	GSList *iter;
	OVERLAP		*olapData;

	switch(freeStruct)
	{
		case FALSE:	// free only the data structures
    		if (trc->data.dataInfo.data)
				free(trc->data.dataInfo.data);
			trc->data.dataInfo.data = NULL;
			freeFiltData(trc);
			if (trc->spectra.data)
			{
				free(trc->spectra.data);
				trc->spectra.data=NULL;
			}
			if (trc->trace.gaps)					// gap info
			{
				free(trc->trace.gaps);
				trc->trace.gaps = NULL;
			}
			if (trc->trace.overlaps)				// overlap info
			{
				free(trc->trace.overlaps);
				trc->trace.overlaps = NULL;
			}
			for(iter=trc->trace.olapData; iter; iter=g_slist_next(iter))
			{
				olapData = iter->data;
				free(olapData->data);
				free(olapData);
			}
			if (trc->trace.olapData)
			{
				g_slist_free(trc->trace.olapData);
				trc->trace.olapData = NULL;
			}
			// transform data elements
			if (trc->data.tForm.data)
				free(trc->data.tForm.data);
			if (trc->data.tForm.tFormStr)
				free(trc->data.tForm.tFormStr);
			memset(&trc->data.tForm, 0, sizeof(DATAINFO));
		break;

		case TRUE:	// free the entire structure's contents and the structure itself
			traces = g_slist_remove(traces, trc);
//			if (settings.general.display.mode == GATHERMODE)		// bug #274
				states.trace.traces[CURGRP] = g_slist_remove(states.trace.traces[CURGRP], trc);
			freeData2(trc);
			free(trc);
		break;
	}
}

static void	makeList()
{
	int i;
	traceInfo *trcData;
	
	for (i=prev_start_rec; 
		 i < prev_numPlots+prev_start_rec &&
		 i < states.general.numTraces; 
		 i++)
	{
		if ((i< states.trace.startRec) ||		// entry is before, remove data
			(i> states.trace.startRec + settings.trace.numPlots-1))		// entry is after
		{
			trcData = g_slist_nth_data(traces, i);
			if (trcData &&
				!trcData->data.sticky)
			{
//fprintf(stderr, "deleting rec %d\n", i);
				freeData(trcData, FALSE);
			}
		}
	}
	
	return;
}

void kill_trace(traceInfo *trcData)
{
	if (trcData)
	{
		freeData(trcData, TRUE);
	}

	states.general.numTraces = g_slist_length(traces);;
}

void kill_all_traces(int which)
{
	int		i;
	GSList *trcIter;
	traceInfo *trcData;

	switch(which)
	{
		case ALL:
			for (trcIter = traces;
				 trcIter;
				 trcIter = traces)
			{
				trcData = trcIter->data;
				freeData(trcData, TRUE);
			}
			states.general.numTraces= g_slist_length(traces);
			g_slist_free(states.trace.selection);
			states.trace.selection = NULL;
			
		{	// free up all SOH info
			GSList *iter1, *iter2, *iter3;
			INSTSOH	*instSOH;
			SOHINFO	*SOHinfo;
			SOHMSG	*SOHmsg;
			LOGDATA	*logData;
			
			for(iter1 = states.data.instSOH;
				iter1;
				iter1 = g_slist_next(iter1))
			{
				instSOH = iter1->data;
				for(iter2 = instSOH->SOH;
					iter2;
					iter2 = g_slist_next(iter2))
				{
					SOHinfo = iter2->data;
					for(iter3 = SOHinfo->msgs;
						iter3;
						iter3 = g_slist_next(iter3))
					{
						SOHmsg = iter3->data;
						free(SOHmsg->msg);
						free(SOHmsg);
					}
					g_slist_free(SOHinfo->msgs);
					free(SOHinfo);
				}
				g_slist_free(instSOH->SOH);
				free(instSOH);
			}
			g_slist_free(states.data.instSOH);
			states.data.instSOH = NULL;
			
			// delete all LOG files
			for(iter1 = states.logs.logs;
				iter1;
				iter1 = g_slist_next(iter1))
			{
				logData = iter1->data;
				for(iter2 = logData->text;
					iter2;
					iter2 = g_slist_next(iter2))
				{
					free(iter2->data);
				}
				g_slist_free(logData->text);
			}
			g_slist_free(states.logs.logs);
			states.logs.logs = NULL;
		}
		break;
		
		case SELECTED:
			for (trcIter = states.trace.selection;
				 trcIter;
				 trcIter = g_slist_next(trcIter))
			{	// remove all selected traces from both lists
				trcData = trcIter->data;
				freeData(trcData, TRUE);
			}
			g_slist_free(states.trace.selection);
			states.trace.selection = NULL;
			states.trace.numSelected= 0;
			states.general.numTraces= g_slist_length(traces);
			if (states.trace.startRec >= states.general.numTraces)
				states.trace.startRec = states.general.numTraces - settings.trace.numPlots;
			for (trcIter = traces, i=0;
				 trcIter;
				 trcIter = g_slist_next(trcIter))
			{	// renumber our position holder
				trcData = trcIter->data;
				trcData->data.position.current = i++;
			}
		break;
	}
}

static GQueue	*prevList;

static void dispList(int which)
{
	static traceInfo	*trc;
	GSList	*iter;
	
	switch(settings.general.display.mode)
	{
		case TRACEMODE:
		{
			int startRec=0, i;
			switch(which)
			{		
				case CURGRP:
					startRec = states.trace.startRec;
				break;
		
				case PREVGRP:
					if (states.trace.startRec == 0)
					{
						startRec = -1;
						break;
					}
					startRec = (states.trace.startRec - settings.trace.numPlots > 0)
								? states.trace.startRec - settings.trace.numPlots
								: 0;
				break;
				
				case NEXTGRP:
					if (states.trace.startRec + 
						settings.trace.numPlots > 
							states.general.numTraces)
					{
						startRec = -1;
						break;
					}
					startRec = states.trace.startRec + settings.trace.numPlots;
				break;
			}
			states.trace.traces[which] = NULL;
			if (startRec == -1)
				break;

			for(iter = g_slist_nth(traces, startRec), i=0;
				iter && i < settings.trace.numPlots;
				iter = g_slist_next(iter), i++)
			{
				trc = iter->data;
				states.trace.traces[which] = g_slist_append(states.trace.traces[which], trc);
			}
		}
		break;
		
		case GATHERMODE:
		{
			int	startRec;
			double	epochS;
			states.trace.traces[which] = NULL;
			switch(which)
			{		
				case CURGRP:
#if 0
fprintf(stderr, "CURGRP: pre-sort list\n");
	{
		GSList *trcIter;
		fprintf(stderr, "\n");
		for(trcIter = traces;
			trcIter;
			trcIter = g_slist_next(trcIter))
		{
			trc = trcIter->data;
			fprintf(stderr, "trcList: sensor: %s\tepochs: %f\n", 
					trc->data.head.sensorID, trc->data.head.epochs);
		}
	}
#endif
					sortList(&traces, SORTSTARTTIME);
					startRec = 0;
					trc = g_slist_nth_data(traces, startRec);
					epochS = trc->data.head.epochs;
					for(iter = g_slist_nth(traces, startRec);
						iter;
						iter = g_slist_next(iter))
					{
						trc = iter->data;
//fprintf(stderr, "trc: %s\tstart: %f\ttrc: %f\tdiff: %f\n", 
//trc->data.head.descr1, epochS, trc->data.head.epochs,trc->data.head.epochs - epochS);
						if (trc->data.head.epochs - epochS > settings.general.display.gather.spread)
							break;
						states.trace.traces[which] = g_slist_append(states.trace.traces[which], trc);
					}
					sortList(&states.trace.traces[which], SORTINSTID);
//					states.trace.startRec += g_slist_length(states.trace.traces[which]);
//					startRec = g_slist_length(states.trace.traces[which]);
				break;
				
				case NEXTGRP:
//fprintf(stderr, "NEXTGRP: startRec: %d\n", states.trace.startRec);
					if (states.trace.startRec + 
						g_slist_length(states.trace.traces[CURGRP]) >= 
							states.general.numTraces)
					{	// at the end of the list
						break;
					}
					startRec = states.trace.startRec + g_slist_length(states.trace.traces[CURGRP]);
					trc = g_slist_nth_data(traces, startRec);
					epochS = trc->data.head.epochs;
					for(iter = g_slist_nth(traces, startRec);
						iter;
						iter = g_slist_next(iter))
					{
						trc = iter->data;
//fprintf(stderr, "trc: %s\tstart: %f\ttrc: %f\tdiff: %f\n", 
//trc->data.head.descr1, epochS, trc->data.head.epochs,trc->data.head.epochs - epochS);
						if (trc->data.head.epochs - epochS > settings.general.display.gather.spread)
							break;
						states.trace.traces[which] = g_slist_append(states.trace.traces[which], trc);
					}
					sortList(&states.trace.traces[which], SORTINSTID);
				break;
				
				case PREVGRP:
					if (g_queue_get_length(prevList))
						states.trace.traces[which] = g_queue_pop_head(prevList);
				break;
			}
		}		
		break;
	}
}

void makeDispList(int cmd)
{
	switch(cmd)
	{
		case INITGROUP:
			if (!prevList)
				prevList = g_queue_new();
			else
			{
				g_queue_free(prevList);
				prevList = g_queue_new();
			}
			
			if (states.trace.traces[PREVGRP])
				g_slist_free(states.trace.traces[PREVGRP]);
			if (states.trace.traces[CURGRP])
				g_slist_free(states.trace.traces[CURGRP]);
			if (states.trace.traces[NEXTGRP])
				g_slist_free(states.trace.traces[NEXTGRP]);

			dispList(CURGRP);

			dispList(PREVGRP);

			dispList(NEXTGRP);
		break;
		
		case PREVGROUP:
			states.trace.startRec -= g_slist_length(states.trace.traces[PREVGRP]);
			if (states.trace.startRec < 0)
				states.trace.startRec = 0;
			if (states.trace.traces[NEXTGRP])
				g_slist_free(states.trace.traces[NEXTGRP]);
			states.trace.traces[NEXTGRP] = states.trace.traces[CURGRP];
			states.trace.traces[CURGRP] = states.trace.traces[PREVGRP];

			dispList(PREVGRP);
		break;
		
		case NEXTGROUP:
			states.trace.startRec += g_slist_length(states.trace.traces[CURGRP]);
			if(settings.general.display.mode == GATHERMODE)
			{
				g_queue_push_head(prevList, states.trace.traces[PREVGRP]);
			}
			states.trace.traces[PREVGRP] = states.trace.traces[CURGRP];
			states.trace.traces[CURGRP] = states.trace.traces[NEXTGRP];

			dispList(NEXTGRP);
		break;
	}
}

static GSList *cutTrace(traceInfo *trc)
{
	GSList *traces=NULL, *timesIter;
	TIMEENTRY	*shotTime;
	traceInfo	*cutTrc;
	
	for(timesIter = settings.general.input.shotTimes;
		timesIter;
		timesIter = g_slist_next(timesIter))
	{
		shotTime = timesIter->data;
		if (shotTime->epochs >= trc->data.head.epochs &&
			shotTime->epochs+settings.general.input.shotLength <= 
				trc->data.head.epochs+trc->data.head.length)
		{
			cutTrc = calloc(1, sizeof(traceInfo));
			memcpy(cutTrc, trc, sizeof(traceInfo));
			// remove the max/min
			cutTrc->data.dataInfo.amps.max = 0; 
			cutTrc->data.dataInfo.amps.min = 0;
			cutTrc->data.dataInfo.data = NULL;
			cutTrc->data.fileName = g_strdup(trc->data.fileName);
			cutTrc->data.head.shortFN = NULL;
			cutTrc->data.head.descr1 = NULL;
			cutTrc->data.head.descr2 = NULL;
			cutTrc->data.head.epochs = cutTrc->excerpt.epochS = shotTime->epochs;
			cutTrc->data.head.length = cutTrc->excerpt.length = settings.general.input.shotLength;
//fprintf(stderr, "excerpt trc: sensor: %s\tepochs: %f\n", 
//cutTrc->data.head.sensorID, cutTrc->data.head.epochs);
			traces = g_slist_append(traces, cutTrc);
		}
		if (shotTime->epochs > trc->data.head.epochs + trc->data.head.length)
			break;
	}
	return (traces);
}

traceInfo *trcCopy(traceInfo *trc, int what)
{
	traceInfo *trcN = calloc(1, sizeof(traceInfo));
	
	// copy the header - always
	memcpy(&trcN->data.head, &trc->data.head, sizeof(HEADINFO));
	trcN->data.head.descr1 = g_strdup(trc->data.head.descr1);
	trcN->data.head.descr2 = trc->data.head.descr2 ? g_strdup(trc->data.head.descr2) : g_strdup("");
	trcN->data.head.longFN = trc->data.head.longFN ? g_strdup(trc->data.head.longFN) : g_strdup("");
	trcN->data.head.shortFN = trc->data.head.shortFN ? g_strdup(trc->data.head.shortFN) : g_strdup("");
	
	switch(what)
	{
		case TRCSPECTRA:	// STN - Spectra and PSD
			trcN->spectra.ampMax = trc->spectra.ampMax;
			trcN->spectra.ampMin = trc->spectra.ampMin;
			trcN->spectra.avg = trc->spectra.avg;
			trcN->spectra.beg =  trc->spectra.beg;
			trcN->spectra.end = trc->spectra.end; 
			trcN->spectra.fill_beg = trc->spectra.fill_beg;
			trcN->spectra.fill_end = trc->spectra.fill_end;
			trcN->spectra.numPts = trc->spectra.numPts;
			trcN->spectra.data = trc->spectra.data;
			trcN->spectra.data_end = trc->spectra.data_end;
		break;
		
		case TRCPM:			// STN - Particle Motion
		{
			int	nSamps = trc->data.dataInfo.head.numSamples,
				format = trc->data.dataInfo.head.data_form;
			memcpy(&trcN->data.dataInfo, &trc->data.dataInfo, sizeof(DATAINFO));
			trcN->data.dataInfo.data = malloc(nSamps * dsizes[format]);
			memcpy(trcN->data.dataInfo.data, trc->data.dataInfo.data, nSamps * dsizes[format]);
		}
		break;
	}

	return trcN;
}
