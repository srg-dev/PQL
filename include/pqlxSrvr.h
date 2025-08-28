#ifndef	__PQLXSRVRINC
#define	__PQLXSRVRINC

#include "pql_defines.h"
#include "pqlx_defs.h"
#include "pqlx_externs.h"
#include "util_externs.h"

#define NTRCSSTARTEVENTPROC 100
#define MAX_DIR_PATH 2048		// maximal full path we support

#define RESPFILE 100	// don't want any collisions with data types
#define NOFILE	(-1)

#define RESPNTW		"B050F16"
#define RESPSTN		"B050F03"
#define RESPLOC		"B052F03"
#define RESPCHN		"B052F04"
#define	RESPMXLN	10
#define MAXSCANS	20

#define PDFNAME		"PDF"
#define DBMAINTPROG	"pqlxDBMaint"
#define POLLINTERVAL	(1*1000000)
#define PSDSIZE		((float)(1024.*1.1))
#define ONEGIG		(1024*1024*1024)
#define STNWARN		5
#define DISKSPACEWARN	1

enum {
	DIRSCAN,
	FILEINPUT,
	TTLIDENTIFYTYPES
} SCANTYPES;

enum {
	THR_SQR_WFR=1,
	THR_SQR_TR
};

typedef struct _PNGARGS
{
	int		threadNum;
	struct {
		char	*wwwDir;
		int		type;
		int		width, height; 
		int		R, G, B;
		gboolean border;
	} plot;
	union {
		struct {
			PDFPNG	*pdfPNG;
			gboolean plotHLNM;
		} PDF;
		
		struct {
			SPECPNG	*specPNG;
		} SPECTRO;
	} data;
} PNGARGS;

typedef struct _PQLXSRVR
{
	char		*pdfSrvr;	// server and dbName
	char		*srvrName;
	char		*dbName;
	int		identify;	// one of DIRSCAN, FILEINPUT
	int		numCPUS;		// number of CPUs to use
	char		*identFile;	// name of file containing trace names for re-analysis
	int		forkNum;		// fork number
	int		logKey;
	char		*execTime;
	gboolean	fork;		// whether or not the server should fork event processors, default = TRUE

	LOGIN	login;
	GSList	*pngWWW;		// PNGWWW Table Entries, per PNG type, data = PNGWWW*
	
	FILE *dbg;
} PQLXSRVRSTRUCT;

typedef struct _TIMERESTRICT
{
	gboolean	set;
	char	*start;
	char	*end;
} TIMERESTRICT;

int checkTrace(traceInfo *);
int chnID(SCAN *, char *, char *, char *, char *, gboolean);
void TIMESTR(char *);
void TIMESTRE(char *);
void getOut();
gboolean forkEventProcs(gboolean init, int numCPUs);
int forkEvents(LOGIN *login, int numProcs);
int pqlxEventProc(LOGIN *login, gboolean forked);
void process_xml_input(LOGIN *login, char *execTime);
int	findChn(TRC1 *trc1, SCAN *scanPtr);
void reAnalyze(char *dbName);
void outputScanStats(SCAN *scan, gboolean new);
void scanLockFile(char *srvr, int cmd);
gboolean scanIsLocked(char *srvr);
void waitForResume();
void pausable(int eventType);
gboolean trashDir();
void *restrictTime(TIMERESTRICT *tr);

// SRVR calls
//gboolean scanDir(SCAN *scnPtr, gboolean firstTime, int cutoff, gboolean *scanForked, char *inDir);
gboolean scanDir(SCAN *scnPtr, gboolean firstTime, gboolean *scanForked, char *inDir);

// ANALYSIS Library calls / EVENTQ event routines
int	an_trc(TRC1	*trc1);
int an_psd(int event, PSD1 *psd);

extern PQLXSRVRSTRUCT	pqlxSrvr;

#include "pqlxSrvrUtils.h"
#endif
