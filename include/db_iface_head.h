#ifndef _DBIF_HEAD_H
#define _DBIF_HEAD_H

// main data structure holding calling arguments
typedef struct _SQL_INFO
{
	int			sqr;			// the display cell (or thread) making this call
	gboolean	callType;		// SYNC | ASYNC, call is synchronous or not?
	gboolean	threaded;		// call is being made from within a thread?
	int			dataEvent;		// see db_iface_defs.h for valid values
	void 		*returnFunc;	// return function address for ASYNC calls
	struct {
		int		connNum;		// internal use for db_() routines only
		int		errorNO;		// error returned from call
		int		cnt;			// number of request attempts
	} call;
} SQL_INFO;

// data structure holding EVENTQ table information
typedef struct _EVNT_INFO
{	// server main loop executes against this
	int	id;			// the eventq id
	int	type;		// the event type, this db value must match
					// what is defined above for event types
	int	typeKey;	// = EVENTQ.typeKey
	int typeKey2;	// = EVENTQ.typeKey2
	int chnKey;		// = EVENTQ.chni_fk
	int	errorNO;
} EVNT_INFO;

typedef struct _DATA_INFO
{	// meta information for *data element
	int	tableData;	// the db table associated with the data
} DATA_INFO;

// the main db interface data structure holding all subordinate data structures, 
// db_request() is called with this data structure having been filled accordingly
typedef struct _SQLDI
{	// SQL Data Interface structure:
	// *data is ptr to input (data read from the db)
	// and output (data written to the db) data structures 
	// these are specific to the event.type or descr.table
	SQL_INFO	args;
	EVNT_INFO	event;
	DATA_INFO	descr;
	void 		*data;
	void		*data2;
} SQLDI;

// LOGIN structure used for connecting
typedef	struct	_LOGIN
{
	struct {
		char	hostname[65];
		char	username[25];
		char	password[25];
		char	dbname[25];
		int		stnXDays;
		char	pgmName[50];
		int		numCPUs;
	} keys;
	struct {
		int		numChn;
		int		logKey;
		int		dbVersion;
		int		pdbVersion;
		gboolean initialExecution;
		GSList	*models;	// list of known models defined in PQLXMETA, data = TTMODEL
	} data;
} LOGIN;

// IDS table data struct
typedef struct _IDS
{
	struct {
		int		idKey;
		char	type[4];
	} keys;
	struct {
		char	name[10];
		char	alias[55];
	} data;
} IDS;

// CHNI Table data struct
typedef struct _CHNI
{
	struct {
		int		chniKey;
		int		idKeys[4];
		char	*name;
	} keys;
	void*	data;		// no data for this table
} CHNI;

typedef struct _SPSEPOCH
{
	int		sampInt;
	char	*dates[TTLTIMES];
} SPSEPOCH;

// Channel Information for ADMIN
typedef struct _CHNSTATS
{
	int		chnKey;
	char	*name;
	char	*dataDate[TTLTIMES];	// STARTTIME | ENDTIME
	int		nTrace;
	int		nPSD;
	int		nSPSepoch;
	char	*response;
	double	sampInt;
	GSList	*spsEpochs;				// data = SPSEPOCH*
} CHNSTATS;

// Data structures for PDF server and databases
typedef struct _PNGWWW
{
	int		type;					// PDF or SPECTRO
	char	*name;
	char	*dir;					// output directory
	gboolean border, Iborder, HLNM;	// border and HLNM: (ON|OFF)
	int		width, height, Iwidth, Iheight;	// dimensions
	int		r, g, b, Ir, Ig, Ib;	// border color
	gboolean stats[TTLPDFSTATS];	// PDF statistics to plot
	int		dateFmt;
} PNGWWW;

typedef struct _EQREC
{
	char	*descr;
	int 	count;
	int		status;
	char 	*error;
} EQREC;

typedef struct _EVENTQLIST
{
	GSList	*events;		// data = EQREC*
} EVENTQLIST;

typedef struct _LOGMSG
{
	struct {
		int		logKey;
		char	*dbName;
	} keys;
	
	struct {
		int	 type;
		char *time;
		char *msg;
	} data;
} LOGMSG;

typedef struct _SRVRLOG
{
	struct {
		int	execKey;
		LOGMSG	*lastLog;		// needed for LOGMSGTBLNEXT
	} keys;
	
	struct {
		char	*pgmName;
		char	*time[TTLTIMES];	// START and END times of execution
		char	*duration;
		gboolean	complete;
		gboolean	updated[TTLSRVRLOGTYPES];
		GSList	*msgs[TTLSRVRLOGTYPES];		// data = LOGMSG*
	} data;
} SRVRLOG;

typedef struct _SRVREXEC
{
	struct {
		GSList	*execs;		// data = SRVRLOG*
	} data;
} SRVREXEC;

typedef struct _NEWEXEC
{
	struct {
		int	lastExec;
	} keys;
	struct {
		gboolean started;
	} data;
} NEWEXEC;

typedef struct _DATADIR
{
	int		pk;
	char	*directory;
	char	*lastFile;
	char	*status;
	int		statusI;
	int		ttlTrace;
	int		ttlResp;
	struct {
		int		cmdFlag;	// one of DD_NONE, DD_NEW, DD_UPD, see ADMIN_DBSAVE for usage
	} admin;
} DATADIR;
	
typedef struct _DBLST
{
	char	*hostName;
	char	*dbName;
	char	*organisation;
	char	*description;
	char	*techAdminName;
	char	*techAdminEmail;
	char	*dataAdminName;
	char	*dataAdminEmail;
	char	*type;				// string versions
	char	*access;
	int		typeI, accessI;		// int versions
	char	*adminPWord;
	char	*hint;
	int		dbVersion;
} DBLST;

typedef struct _DBDEF
{
	DBLST	dbListing;
	GSList	*pngWWW;			// data = PNGWWW*
	GSList	*dataDirs;			// data = DATADIR*
	GSList	*psdConfs;			// data = PSDCONF*
	GSList	*chnInfo;			// data = CHNSTATS*
	SRVREXEC srvrExec;			// Log Executions
} DBDEF;
	
typedef struct _SERVERDBS
{
	struct {
		char	*srvrName;
		char	*dbName;
		GSList	*dbsList;		// data = DBLST*
	} data;
} SERVERDBS;

typedef struct _PDFDATA
{
	float	period;
	int		power;
	double	prob;
} PDFDATA;

typedef struct _PDFHITSDATA
{
	float	period;
	int		power;
	int		hits;
} PDFHITSDATA;

typedef struct _PDFHITS
{
	int		pdfKey;
	int		ttlPeriods;
	int		ttlHits;				// total number of hits per period
	int		ttlRows;				// total number of rows allocated for *hits
	PDFHITSDATA	*hits;				// array of per/pow hits
} PDFHITS;

typedef struct _PDFSTATS1
{	// all statistics for each period
	float	period;
	int		statV[TTLPDFSTATS];
} PDFSTATS1;

typedef struct _PDFSTATS
{
	int		pdfKey;
	int		ttlPeriods;
	PDFSTATS1	*stats;				// array of period statistics
} PDFSTATS;

typedef struct _PDFSTATSL1
{
	float	period;
	int		power;
} PDFSTATSL1;

typedef struct _PDFSTATSL
{	// holding the line defined by per/pow combos for a given statistic
	int		statistic;				// which statistic this line represents
	int		numPts;					// number of points in the line
	PDFSTATSL1	*line;
} PDFSTATSL;

typedef struct _MYTIME
{
	double	epochS;
	PTIME	date;
	short	m_secs;
} MYTIME;

typedef struct _PDFREQ
{
	int		reqID;								// unique ID per PDFREQ request
	struct {
		int		pdfType;						// SYSTEM, BASELINE
		int 	pdfKey;							// PDF DB key, or CUSTOMPDF for user request
		int		chnKey;							// channel instance db key
		char	*sysPDFB[TTLDATES];				// System PDF Date boundaries
		int		yBounds[TTLYBOUNDS];			// y-axis bounds
		gboolean	statistics[TTLPDFSTATS];	// which statistics to display
		int 	reqType;						// REQRANGE, REQAGO, REQFREQ, REQRANGE2
		float	criteria[MAXNUMPDFREQCRIT];		// request criteria specifiers
		gboolean	tmpFilled;					// has DB tmp table been filled? (see db_sql.h for info)
		gboolean	fillTmp;					// should we fill the tmp tables ourselves?
		int		type;							// type of PSD analysis - BBAND_CONT | STMO_TRIG
		struct {
			int	bline_fk;						// baseline pk
			int	pct;							// percent points
		} bline;
	} keys;
	struct {
		char	*pdfName;						// name of system PDF
		char	*name;							// fully qualified NTW.STN.LOC.CHN
		int		numPSDs;						// the number of PSD's included in this extract
		int		numRecs;						// the number of probabilities returned
		PDFDATA	*pdf;							// the period, power, probability combo
		float	*pdfM;							// the magnified probs, 1-to-1 relationship to display PDF
		MYTIME	sTime, eTime;					// start and end date of returned PDF
		PDFHITS	*hits;							// period/power combo of bin hits for this PDF
		PDFSTATS *stats;						// computed period statistics - non-System PDFs
		GSList 	*statLines;						// lines of statistics, data = PDFSTATSL
		int		dateFmt;						// PDF Plot Date Format
		int		ttlEpochs;						// number of SPS epochs for this PDF
	} data;
	gboolean busy;								// TRUE = db request outstanding
} PDFREQ;

#endif
