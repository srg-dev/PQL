#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pql_defines.h"
#include "pql_externs.h"
#include "gtk_pql_externs.h"

gboolean readData_finish(gpointer nil);
int     readTrace(traceInfo *);

static int              numThreads;             // total number of executing threads
static int              QLen;                   // total files to read
static gboolean         called;
static GQueue           *traceQ;                // Q of files to read

// Thread safety - replace old GStaticMutex with modern GMutex
static GMutex           traceQ_mutex;
static GMutex           counter_mutex;
static gboolean         mutexes_initialized = FALSE;

enum {
        PBARINIT,
        PBARUPD,
        PBARDONE
};

typedef struct _GETTRACEARGS
{
        GThreadFunc returnFunc;
} GETTRACEARGS;

// Initialize mutexes safely
static void ensure_trace_mutexes_initialized(void)
{
    static GMutex init_mutex;
    static gboolean init_lock_initialized = FALSE;
    
    if (!init_lock_initialized)
    {
        g_mutex_init(&init_mutex);
        init_lock_initialized = TRUE;
    }
    
    g_mutex_lock(&init_mutex);
    if (!mutexes_initialized)
    {
        g_mutex_init(&traceQ_mutex);
        g_mutex_init(&counter_mutex);
        mutexes_initialized = TRUE;
    }
    g_mutex_unlock(&init_mutex);
}

static gboolean PBarUpd(gpointer c)
{
        int             cmd = GPOINTER_TO_INT(c);
        double          prog;
        char            progText[56];
        static int      ttl;
        static GMutex   pbar_mutex;
        static gboolean pbar_init = FALSE;

        if (!pbar_init)
        {
            g_mutex_init(&pbar_mutex);
            pbar_init = TRUE;
        }

        g_mutex_lock(&pbar_mutex);

        switch (cmd)
        {
                case PBARINIT:
                        ttl = 1;
                        gtk_widget_show(progressBar);
                        gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR (progressBar), 0.0);
                        snprintf(progText, sizeof(progText), "%d Traces", QLen);
                        gtk_progress_bar_set_text(GTK_PROGRESS_BAR (progressBar), progText);
                break;

                case PBARUPD:
                        if (QLen > 0)  // Prevent division by zero
                        {
                            prog = (double) ((double) (ttl)/(double) QLen);
                            gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR (progressBar), prog);
                            snprintf(progText, sizeof(progText), "(%d / %d) %3.1f%%", ttl, QLen, ((double) prog*100.));
                            gtk_progress_bar_set_text(GTK_PROGRESS_BAR (progressBar), progText);
                        }
                        ttl++;
                break;

                case PBARDONE:
//                      gtk_widget_hide(progressBar);
                break;
        }

        g_mutex_unlock(&pbar_mutex);
        return FALSE;
}

static traceInfo *getNextTrace(void)
{
    traceInfo *trcData;

    ensure_trace_mutexes_initialized();
    
    g_mutex_lock(&traceQ_mutex);
    
    // CRITICAL FIX: Multiple safety checks
    if (traceQ && !g_queue_is_empty(traceQ))
    {
        trcData = g_queue_pop_head(traceQ);
    }
    else
    {
        trcData = NULL;
        // Debug info when queue becomes empty/invalid
        if (!traceQ)
            fprintf(stderr, "WARNING: traceQ is NULL in getNextTrace\n");
    }
    
    g_mutex_unlock(&traceQ_mutex);
    return trcData;
}

static void _getTraces(GETTRACEARGS *args)
{
        traceInfo       *trcData;
        GThreadFunc     retFunc = args->returnFunc;
        int             read;

        if (!args)
        {
            fprintf(stderr, "ERROR: NULL args passed to _getTraces\n");
            return;
        }

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
        
        if (retFunc)
            (*(retFunc))(NULL);
}

static void finish(void)
{
        ensure_trace_mutexes_initialized();
        
        g_mutex_lock(&counter_mutex);
        numThreads--;
        int remaining = numThreads;
        g_mutex_unlock(&counter_mutex);

        if (!remaining)
        {       // all threads complete, return
                g_mutex_lock(&traceQ_mutex);
                if (traceQ)
                {
                    g_queue_free(traceQ);
                    traceQ = NULL;
                }
                g_mutex_unlock(&traceQ_mutex);
                
                g_idle_add((GSourceFunc) PBarUpd, GINT_TO_POINTER(PBARDONE));
                g_idle_add((GSourceFunc) readData_finish, NULL);
                called = FALSE;
        }
}

void readTraces(GQueue *f)
{
        int     i, maxThreads;

        ensure_trace_mutexes_initialized();

        if (!f)
        {
            fprintf(stderr, "ERROR: NULL queue passed to readTraces\n");
            return;
        }

        called = TRUE;
        QLen = g_queue_get_length(f);
        
        if (QLen == 0)
        {
            fprintf(stderr, "WARNING: Empty trace queue\n");
            called = FALSE;
            return;
        }
        
        maxThreads = states.general.numCPUs == 1
                                        ? 1
                                        : (QLen < 3*states.general.numCPUs) ? QLen : 3*states.general.numCPUs;

        // CRITICAL FIX: Thread-safe queue initialization
        g_mutex_lock(&traceQ_mutex);
        if (traceQ)
        {
            // Clean up any existing queue
            g_queue_free(traceQ);
        }
        traceQ = g_queue_copy(f);
        if (!traceQ)
        {
            fprintf(stderr, "ERROR: Failed to copy trace queue\n");
            g_mutex_unlock(&traceQ_mutex);
            called = FALSE;
            return;
        }
        g_mutex_unlock(&traceQ_mutex);
        
        g_idle_add((GSourceFunc) PBarUpd, GINT_TO_POINTER(PBARINIT));

        // Initialize thread counter
        g_mutex_lock(&counter_mutex);
        numThreads = 0;
        g_mutex_unlock(&counter_mutex);

        //fprintf(stderr, "qlen: %d\tmaxThreads: %d\n", QLen, maxThreads);
        for (i = 0; i < maxThreads; i++)
        {
                GETTRACEARGS *getTraceArgs;
                getTraceArgs = calloc(1, sizeof(GETTRACEARGS));
                if (!getTraceArgs)
                {
                    fprintf(stderr, "ERROR: Failed to allocate getTraceArgs for thread %d\n", i);
                    break;
                }
                
                getTraceArgs->returnFunc = (GThreadFunc) finish;
                
                g_mutex_lock(&counter_mutex);
                numThreads++;
                g_mutex_unlock(&counter_mutex);
                
                makeThread(ASYNC, (GThreadFunc) _getTraces, (void *) getTraceArgs, NULL);
        }
}
