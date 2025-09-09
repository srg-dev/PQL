#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pql_defines.h"
#include "pql_externs.h"
#include "gtk_pql_externs.h"

gboolean readHdr_finish(GSList *threadTraces);
GSList *readInstSOH(char *, int);

static int              numThreads, maxThreads;         // total number of executing threads
static int              QLen;                           // total files to read
static gboolean         called;
static GSList           *traceList;                     // list of all returned threadTraces
static GSList           **threadTraces;                 // per thread list of threadTraces
static GQueue           *fileQ;                         // Q of files to read

// Thread safety additions - static to maintain original interface
static GMutex           threadTraces_mutex;
static GMutex           counter_mutex;
static GMutex           traceList_mutex;
static gboolean         mutexes_initialized = FALSE;

enum {
        PBARINIT,
        PBARUPD,
        PBARDONE
};

typedef struct _GETTRACEARGS
{
        int                     threadNum;
        GSList          *threadQ;               // list to work from
        GThreadFunc returnFunc;
} GETTRACEARGS;

#define PBARMULTIPLE    10
#define MT_FACTOR               3

// Auto-initialize mutexes on first use
static void ensure_mutexes_initialized(void)
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
        g_mutex_init(&threadTraces_mutex);
        g_mutex_init(&counter_mutex);
        g_mutex_init(&traceList_mutex);
        mutexes_initialized = TRUE;
    }
    g_mutex_unlock(&init_mutex);
}

static gboolean PBarUpd(gpointer c)
{
        int             cmd = GPOINTER_TO_INT(c);
        double  prog;
        char    progText[56];
        static int ttl;

        switch (cmd)
        {
                case PBARINIT:
                        ttl = 1;
                        gtk_widget_show(progressBar);
                        gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR (progressBar), 0.0);
                        snprintf(progText, sizeof(progText), "%d Files", QLen);
                        gtk_progress_bar_set_text(GTK_PROGRESS_BAR (progressBar), progText);
                break;

                case PBARUPD:
                        if (!(ttl % PBARMULTIPLE))
                        {       // plot only every 10th when reading headers
                                prog = (double) ((double) (ttl)/(double) QLen);
                                gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR (progressBar), (prog));
                                snprintf(progText, sizeof(progText), "(%d / %d) %3.1f%%", ttl, QLen, ((double) ((prog)*100.)));
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

static void _getthreadTraces(GETTRACEARGS *args)
{
        PQLDATA *pqlIn;
        GSList  *readTraces, *threadList=NULL, *iter, *threadQ = args->threadQ;
        GThreadFunc retFunc = args->returnFunc;
        traceInfo       *trc;
        int             threadNum = args->threadNum;

        // CRITICAL FIX: Validate threadNum immediately
        if (threadNum < 0 || threadNum >= maxThreads)
        {
            fprintf(stderr, "FATAL: Invalid threadNum %d (maxThreads: %d)\n", threadNum, maxThreads);
            free(args);
            if (threadQ)
                g_slist_free(threadQ);
            
            // Decrement thread count since this thread is exiting
            ensure_mutexes_initialized();
            g_mutex_lock(&counter_mutex);
            numThreads--;
            g_mutex_unlock(&counter_mutex);
            return;
        }

        free(args);
        for (iter = threadQ;
                 iter;
                 iter = g_slist_next(iter))
        {
                pqlIn = iter->data;
                
                // Add safety check for pqlIn
                if (!pqlIn || !pqlIn->fileName)
                {
                    fprintf(stderr, "WARNING: NULL pqlIn or fileName, skipping\n");
                    continue;
                }
                
                readTraces = identifyTrace(pqlIn->fileName, mseed_opts);        // return list of individual Traces for input file
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
                        {       // Traces were identified
                                threadList = g_slist_concat(threadList, readTraces);
                                // and do any post-processing necessary
                                if (readTraces)  // Safety check
                                {
                                    trc = readTraces->data;         // check the file type using 1st trace returned
                                    if (trc)  // Additional safety check
                                    {
                                        switch(trc->data.fileType)
                                        {
                                                case RT125_DATA:
                                                        if (!g_slist_find_custom(states.data.instSOH, trc, (GCompareFunc) getInstSOH))
                                                        {
                                                                GSList  *SOHList;
                                                                INSTSOH *instSOH = calloc(1, sizeof(INSTSOH));
                                                                if (instSOH)  // Check allocation success
                                                                {
                                                                    instSOH->fileName = g_strdup(pqlIn->fileName);
                                                                    instSOH->instID = g_strdup(trc->data.head.sensorID);
                                                                    SOHList = readInstSOH(pqlIn->fileName, trc->data.fileType);
                                                                    instSOH->SOH = SOHList;
                                                                    states.data.instSOH = g_slist_insert_sorted(states.data.instSOH,
                                                                                                                    instSOH, (GCompareFunc) insertInstSOH);
                                                                }
                                                        }
                                                break;
                                                case RT125A_DATA:
                                                        if (!g_slist_find_custom(states.data.instSOH, trc, (GCompareFunc) getInstSOH))
                                                        {
                                                                GSList  *SOHList;
                                                                INSTSOH *instSOH = calloc(1, sizeof(INSTSOH));
                                                                if (instSOH)  // Check allocation success
                                                                {
                                                                    instSOH->fileName = g_strdup(pqlIn->fileName);
                                                                    instSOH->instID = g_strdup(trc->data.head.sensorID);
                                                                    SOHList = readInstSOH(pqlIn->fileName, trc->data.fileType);
                                                                    instSOH->SOH = SOHList;
                                                                    states.data.instSOH = g_slist_insert_sorted(states.data.instSOH,
                                                                                                                    instSOH, (GCompareFunc) insertInstSOH);
                                                                }
                                                        }
                                                break;
                                                case RT130_DATA:
                                                        if (!g_slist_find_custom(states.data.instSOH, trc, (GCompareFunc) getInstSOH))
                                                        {
                                                                GSList  *SOHList;
                                                                INSTSOH *instSOH = calloc(1, sizeof(INSTSOH));
                                                                if (instSOH)  // Check allocation success
                                                                {
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
                                                        }
                                                break;
                                                case MSEED_MPLEX_DATA:
                                                {
                                                        LOGDATA *logData;
                                                        logData = makeLogs(pqlIn->fileName);
                                                        if (logData)
                                                                states.logs.logs = g_slist_append(states.logs.logs, logData);
                                                }
                                                break;
                                        }
                                    }
                                }
                        }
                        break;
                }
                g_idle_add((GSourceFunc) PBarUpd, GINT_TO_POINTER(PBARUPD));
        }
        
        g_slist_free(threadQ);

        // CRITICAL FIX: Thread-safe assignment with bounds checking
        ensure_mutexes_initialized();
        g_mutex_lock(&threadTraces_mutex);
        
        // Double-check bounds before assignment (this was the segfault location)
        if (threadNum >= 0 && threadNum < maxThreads && threadTraces != NULL)
        {
            threadTraces[threadNum] = threadList;
        }
        else
        {
            fprintf(stderr, "FATAL ERROR: Cannot assign threadList - threadNum=%d, maxThreads=%d, threadTraces=%p\n", 
                   threadNum, maxThreads, (void*)threadTraces);
            // Emergency cleanup
            if (threadList)
                g_slist_free(threadList);
        }
        
        g_mutex_unlock(&threadTraces_mutex);

        (*(retFunc))(NULL);
}

static void finish(void)
{
    ensure_mutexes_initialized();
    
    g_mutex_lock(&counter_mutex);
    numThreads--;
    int remaining = numThreads;
    g_mutex_unlock(&counter_mutex);

    if (!remaining)
    {   // all threads complete, return list to caller (readHdr_1())
        int i;

        g_mutex_lock(&traceList_mutex);
        g_mutex_lock(&threadTraces_mutex);

        if (threadTraces)  // Safety check
        {
            for (i = 0; i < maxThreads; i++)
            {
                if (threadTraces[i])
                {
                    if (!traceList)
                    {
                        traceList = threadTraces[i];  // first non-NULL list
                    }
                    else
                    {
                        traceList = g_slist_concat(traceList, threadTraces[i]);
                    }
                    threadTraces[i] = NULL;  // avoid double free
                }
            }

            free(threadTraces);
            threadTraces = NULL;
        }

        g_mutex_unlock(&threadTraces_mutex);
        g_mutex_unlock(&traceList_mutex);

        g_idle_add((GSourceFunc) PBarUpd, GINT_TO_POINTER(PBARDONE));
        g_idle_add((GSourceFunc) readHdr_finish, traceList);
        called = FALSE;
    }
}

static GSList *getFiles(int numFiles)
{       // Keep original function name for compatibility
        GSList *files = NULL;
        PQLDATA *pqlIn;
        int             i;

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
        int     i, numFilesPThread;

        // Initialize mutexes on first use
        ensure_mutexes_initialized();

        called = TRUE;
        QLen = g_queue_get_length(f);
        maxThreads = (states.general.numCPUs == 1)
                                        ? MT_FACTOR
                                        : (QLen < MT_FACTOR * states.general.numCPUs)
                                                ? QLen
                                                : MT_FACTOR * states.general.numCPUs;

        // CRITICAL FIX: Safe allocation with validation
        g_mutex_lock(&threadTraces_mutex);
        threadTraces = calloc(maxThreads, sizeof(GSList *));
        if (!threadTraces)
        {
            fprintf(stderr, "FATAL: Failed to allocate threadTraces array for %d threads\n", maxThreads);
            g_mutex_unlock(&threadTraces_mutex);
            called = FALSE;
            return;
        }
        
        // Initialize all elements to NULL for safety
        for (i = 0; i < maxThreads; i++)
        {
            threadTraces[i] = NULL;
        }
        g_mutex_unlock(&threadTraces_mutex);

        fileQ = f;
        traceList = NULL;               // this should happen elsewhere, but we're defensive
        g_idle_add((GSourceFunc) PBarUpd, GINT_TO_POINTER(PBARINIT));

        numFilesPThread = QLen / maxThreads + 1;
        
        g_mutex_lock(&counter_mutex);
        numThreads = 0;
        g_mutex_unlock(&counter_mutex);
        
        for (i=0; i < maxThreads; i++)
        {
                GSList  *files = getFiles(numFilesPThread);
                if (!files)
                        break;

                GETTRACEARGS *getTraceArgs = calloc(1, sizeof(GETTRACEARGS));
                if (!getTraceArgs)
                {
                    fprintf(stderr, "ERROR: Failed to allocate thread args for thread %d\n", i);
                    g_slist_free(files);
                    break;
                }

                getTraceArgs->threadNum = i;
                getTraceArgs->threadQ = files;
                getTraceArgs->returnFunc = (GThreadFunc) finish;
                
                g_mutex_lock(&counter_mutex);
                numThreads++;
                g_mutex_unlock(&counter_mutex);
                
                makeThread(ASYNC, (GThreadFunc) _getthreadTraces, (void *) getTraceArgs, NULL);
        }
}
