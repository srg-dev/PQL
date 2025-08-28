#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pql_defines.h"
#include "pql_externs.h"
#include "gtk_pql_externs.h"

gboolean readHdr_finish(GSList *threadTraces);
GSList *readInstSOH(char *, int);

static int		numThreads, maxThreads;		// total number of executing threads
static int		QLen;						// total files to read
static gboolean called;			
static GSList	*traceList;					// list of all returned threadTraces
static GSList	**threadTraces;				// per thread list of threadTraces
static GQueue	*fileQ;						// Q of files to read
//static GStaticMutex QMutex = G_STATIC_MUTEX_INIT;		// mutex to read Q

enum {
	PBARINIT,
	PBARUPD,
	PBARDONE
};
	
typedef struct _GETTRACEARGS
{
	int			threadNum;
	GSList		*threadQ;		// list to work from
	GThreadFunc returnFunc;
} GETTRACEARGS;

#define PBARMULTIPLE	10
#define MT_FACTOR		3

static gboolean PBarUpd(gpointer c)
{
	int		cmd = GPOINTER_TO_INT(c);
	double	prog;
	char	progText[56];
	static int ttl;

	switch (cmd)
	{
		case PBARINIT:
			ttl = 1;
			gtk_widget_show(progressBar);
			gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR (progressBar), 0.0);
			sprintf(progText, "%d Files", QLen);
			gtk_progress_bar_set_text(GTK_PROGRESS_BAR (progressBar), progText);
		break;
		
		case PBARUPD:
			if (!(ttl % PBARMULTIPLE))
			{	// plot only every 10th when reading headers
				prog = (double) ((double) (ttl)/(double) QLen);
				gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR (progressBar), (prog));
				sprintf(progText, "(%d / %d) %3.1f%%", ttl, QLen, ((double) ((prog)*100.)));
				gtk_progress_bar_set_text(GTK_PROGRESS_BAR (progressBar), progText);
			}
			ttl++;
		break;
		
		case PBARDONE:
			gtk_widget_hide(progressBar);
		break;
	}
	
	return FALSE;
}

#if 0
static PQLDATA *getNextFile()
{
	PQLDATA *pqlIn;
	
	g_static_mutex_lock(&QMutex);
	pqlIn = g_queue_pop_head(fileQ);
	g_static_mutex_unlock(&QMutex);
	return pqlIn;
}
#endif

static void _getthreadTraces(GETTRACEARGS *args)
{
	PQLDATA	*pqlIn;
	GSList	*readTraces, *threadList=NULL, *iter, *threadQ = args->threadQ;
	GThreadFunc retFunc = args->returnFunc;
	traceInfo	*trc;
	int		threadNum = args->threadNum;
	
	free(args);
	for (iter = threadQ;
		 iter;
		 iter = g_slist_next(iter))
	{
		pqlIn = iter->data;
		readTraces = identifyTrace(pqlIn->fileName, mseed_opts);	// return list of individual Traces for input file
		switch ((long) readTraces)
		{
			case 0:
				fprintf(stderr, "File is not Recognized, ignored (%s)\n", pqlIn->fileName);
			break;
			case FILE_DNE:
				fprintf(stderr, "File Does Not Exist, removing from input list (%s)\n", 
						pqlIn->fileName);
			break;
			case NO_CHANS:
				fprintf(stderr, "File Recognized as Multiplex Mini-Seed, "\
								"however no valid channels found for display (%s)\n", 
								pqlIn->fileName);
			break;
			
			default:
			{	// Traces were identified
				threadList = g_slist_concat(threadList, readTraces);
				// and do any post-processing necessary
				trc = readTraces->data;		// check the file type using 1st trace returned
				switch(trc->data.fileType)
				{
					case RT125_DATA:
						if (!g_slist_find_custom(states.data.instSOH, trc, (GCompareFunc) getInstSOH))
						{
							GSList	*SOHList;
							INSTSOH	*instSOH = calloc(1, sizeof(INSTSOH));
							instSOH->fileName = g_strdup(pqlIn->fileName);
							instSOH->instID = g_strdup(trc->data.head.sensorID);
							SOHList = readInstSOH(pqlIn->fileName, trc->data.fileType);
							instSOH->SOH = SOHList;
							states.data.instSOH = g_slist_insert_sorted(states.data.instSOH, 
														instSOH, (GCompareFunc) insertInstSOH);
						}
					break;
					case RT125A_DATA:
						if (!g_slist_find_custom(states.data.instSOH, trc, (GCompareFunc) getInstSOH))
						{
							GSList	*SOHList;
							INSTSOH	*instSOH = calloc(1, sizeof(INSTSOH));
							instSOH->fileName = g_strdup(pqlIn->fileName);
							instSOH->instID = g_strdup(trc->data.head.sensorID);
							SOHList = readInstSOH(pqlIn->fileName, trc->data.fileType);
							instSOH->SOH = SOHList;
							states.data.instSOH = g_slist_insert_sorted(states.data.instSOH, 
														instSOH, (GCompareFunc) insertInstSOH);
						}
					break;
					case RT130_DATA:
						if (!g_slist_find_custom(states.data.instSOH, trc, (GCompareFunc) getInstSOH))
						{
							GSList	*SOHList;
							INSTSOH	*instSOH = calloc(1, sizeof(INSTSOH));
							instSOH->fileName = g_strdup(pqlIn->fileName);
							instSOH->instID = g_strdup(trc->data.head.sensorID);
							SOHList = readInstSOH(pqlIn->fileName, trc->data.fileType);
							if (SOHList != ((GSList *)FILE_DNE))
								instSOH->SOH = SOHList;
							else
								instSOH->SOH = NULL;
							states.data.instSOH = g_slist_insert_sorted(states.data.instSOH, 
														instSOH, (GCompareFunc) insertInstSOH);
						}
					break;
					case MSEED_MPLEX_DATA:
					{
						LOGDATA	*logData;
						logData = makeLogs(pqlIn->fileName);
						if (logData)
							states.logs.logs = g_slist_append(states.logs.logs, logData);
					}
					break;
				}
			}
			break;
		}
		g_idle_add((GSourceFunc) PBarUpd, GINT_TO_POINTER(PBARUPD));
	}
	g_slist_free(threadQ);
	threadTraces[threadNum] = threadList;
	(*(retFunc))(NULL);
}

static void finish()
{
	numThreads--;
	
	if (!numThreads)
	{	// all threads complete, return list to caller (readHdr_1())
		int		i;

		for (i=0;i<maxThreads;i++)
		{
			if (threadTraces[i])
				traceList = g_slist_concat(traceList, threadTraces[i]);
		}
		free(threadTraces);

		g_idle_add((GSourceFunc) PBarUpd, GINT_TO_POINTER(PBARDONE));
		g_idle_add((GSourceFunc) readHdr_finish, traceList);
		called = FALSE;
	}
}

static GSList *getFiles(int numFiles)
{	// create individual lists of files to read for each thread
	// this replaces the previous method that required a lock around
	// each file, now avoided
	GSList *files = NULL;
	PQLDATA *pqlIn;
	int		i;
	
	for (i=0; i < numFiles; i++)
	{
		pqlIn = g_queue_pop_head(fileQ);
		if (!pqlIn)
			break;
		files = g_slist_prepend(files, pqlIn);
	}
	return (files);		
}

void readFiles(GQueue *f)
{
	int	i, numFilesPThread;

	called = TRUE;
	QLen = g_queue_get_length(f);
	maxThreads = (states.general.numCPUs == 1)
					? MT_FACTOR
					: (QLen < MT_FACTOR * states.general.numCPUs) 
						? QLen 
						: MT_FACTOR * states.general.numCPUs;

	threadTraces = calloc(maxThreads, sizeof(GSList *));
	fileQ = f;
	traceList = NULL;		// this should happen elsewhere, but we're defensive
	g_idle_add((GSourceFunc) PBarUpd, GINT_TO_POINTER(PBARINIT));
	
	numFilesPThread = QLen / maxThreads + 1;
	for (i=0, numThreads = 0;
		 i < maxThreads;
		 i++, numThreads++)
	{
		GSList	*files = getFiles(numFilesPThread);
		if (!files)
			break;
		
		GETTRACEARGS *getTraceArgs = calloc(1, sizeof(GETTRACEARGS));
		
		getTraceArgs->threadNum = i;
		getTraceArgs->threadQ = files;
		getTraceArgs->returnFunc = (GThreadFunc) finish;
		makeThread(ASYNC, (GThreadFunc) _getthreadTraces, (void *) getTraceArgs, NULL);
	}
}
