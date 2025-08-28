#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pql_defines.h"
#include "pql_externs.h"
#include "gtk_pql_externs.h"

gboolean readData_finish(gpointer nil);
int	readTrace(traceInfo *);

static int		numThreads;		// total number of executing threads
static int		QLen;			// total files to read
static gboolean called;			
static GQueue	*traceQ;			// Q of files to read
static GStaticMutex QMutex = G_STATIC_MUTEX_INIT;		// mutex to read Q

enum {
	PBARINIT,
	PBARUPD,
	PBARDONE
};
	
typedef struct _GETTRACEARGS
{
	GThreadFunc returnFunc;
} GETTRACEARGS;

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
			sprintf(progText, "%d Traces", QLen);
			gtk_progress_bar_set_text(GTK_PROGRESS_BAR (progressBar), progText);
		break;
		
		case PBARUPD:
			prog = (double) ((double) (ttl)/(double) QLen);
			gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR (progressBar), prog);
			sprintf(progText, "(%d / %d) %3.1f%%", ttl, QLen, ((double) prog*100.));
			gtk_progress_bar_set_text(GTK_PROGRESS_BAR (progressBar), progText);
			ttl++;
		break;
		
		case PBARDONE:
//			gtk_widget_hide(progressBar);
		break;
	}
	
	return FALSE;
}

static traceInfo *getNextTrace()
{
	traceInfo *trcData;
	
	if (g_queue_is_empty(traceQ))
		return NULL;
	
	g_static_mutex_lock(&QMutex);
	trcData = g_queue_pop_head(traceQ);
//fprintf(stderr, "%d: trcData: %d\n", ++cnt, (int) trcData);
	g_static_mutex_unlock(&QMutex);
	return trcData;
}

static void _getTraces(GETTRACEARGS *args)
{
	traceInfo	*trcData;
	GThreadFunc retFunc = args->returnFunc;
	int	read;
	
	free(args);
	trcData = getNextTrace();
	while (trcData)
	{
		read = readTrace(trcData);
		switch(read)
		{
			case FAILURE:
				fprintf(stderr, "File %s cannot be read, removing...\n",
							trcData->data.fileName);
			break;
			case NOT_ENOUGH_DATA:
				switch(trcData->data.fileType)
				{
					case RT125A_DATA:
					case RT125_DATA:
						fprintf(stderr, "Shot contains no data, removing... ( %s )\n",
									trcData->data.head.shortFN);
					break;
					default:
						fprintf(stderr, "File %s contains no data, removing...\n",
									trcData->data.fileName);
					break;
				}
			break;
		}
		if (read != PQLXSUCCESS)
		{
  			kill_trace(trcData);
		}
		g_idle_add((GSourceFunc) PBarUpd, GINT_TO_POINTER(PBARUPD));
		trcData = getNextTrace();
	}
	(*(retFunc))(NULL);
}

static void finish()
{
	numThreads--;
	
	if (!numThreads)
	{	// all threads complete, return
		g_queue_free(traceQ);
		g_idle_add((GSourceFunc) PBarUpd, GINT_TO_POINTER(PBARDONE));
		g_idle_add((GSourceFunc) readData_finish, NULL);
		called = FALSE;
	}
}

void readTraces(GQueue *f)
{
	int	i, maxThreads;

//	if (called)
//		return;
	
	called = TRUE;
	QLen = g_queue_get_length(f);
	maxThreads = states.general.numCPUs == 1
					? 1
					: (QLen < 3*states.general.numCPUs) ? QLen : 3*states.general.numCPUs;
	
	traceQ = g_queue_copy(f);
	g_idle_add((GSourceFunc) PBarUpd, GINT_TO_POINTER(PBARINIT));
	
//fprintf(stderr, "qlen: %d\tmaxThreads: %d\n", QLen, maxThreads);
	for (i=0, numThreads = 0;
		 i < maxThreads;
		 i++, numThreads++)
	{
		GETTRACEARGS *getTraceArgs;
		getTraceArgs = calloc(1, sizeof(GETTRACEARGS));
		getTraceArgs->returnFunc = (GThreadFunc) finish;
		makeThread(ASYNC, (GThreadFunc) _getTraces, (void *) getTraceArgs, NULL);
	}
}

