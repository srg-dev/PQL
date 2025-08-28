#ifndef __DBIF_H
#define __DBIF_H

#include <time.h>
#include <glib.h>
#include "ptime.h"
#include "db_iface_defs.h"
#include "db_iface_head.h"

// helper/subordinate data structures
typedef struct _SRVRSTATE
{
	struct {
		int	cntInfo[TTLSRVRSTT];
	} data;
} SRVRSTATE;

// the PSD Configuration Parameters, corresponds to PSDCFG table
typedef struct _PSDCONF
{
	char	*chnGroup;
    int 	last_stage;
    int		min_power, max_power;
	int		overlap;
    int		sections;
    int		steps;
    int		hourly_data;
    int		psd_dur;  		// duration of one PSD, in seconds
    int		db_offset;		// Added to db before output
    float	max_period;		// center period of smoothing bins
    float	bin_width;		// width of smoothing bins, fractional octave
    float	gap_thresh;		// max allowed gap, in sample intervals
    char    *units;			// Units of instrument response.
	int		type;			// type of PSD analysis - BBAND_CONT | STMO_TRIG
	struct {
		int		cmdFlag;	// one of DU_NONE, DU_NEW, DU_UPD, see ADMIN_DBSAVE for usage
		gboolean data;		// there exists analyzed data?
	} admin;
} PSDCONF;

typedef struct _PDFBOUNDS
{
	struct {
		int	chnKey;
	} keys;
	struct {
		MYTIME	bounds[TTLCHNBOUNDS];
	} data;
} PDFBOUNDS;
		
// SCAN table data structure
typedef struct _SCAN
{
	struct {
		int		dirKey;
		char	directory[MAX_DIR_STR];
		unsigned long long	scanTime;
		int		scanType;
	} keys;
	struct {
		int		nTrace;			// number of traces found
		int		nResp;			// number of response files found
		int		nNtw;			// number of NEW networks identified
		int		nStn;			// etc.
		int		nLoc;
		int		nChn;
	} data;
} SCAN;
	
typedef struct _SCANS
{
	struct {
		GSList 	*scans;			// list of directories to scan, data=SCAN
	} data;
} SCANS;

// TRC Table data struct
typedef struct _TRC
{
	struct {
		int	trcKey;
		int	chni;
		struct ptime	trcTime;
	} keys;
} TRCKEY;

typedef struct _TRCNEXT
{
	struct {
		int	chnKey;
		int	trcKey;
	} keys;
	struct {
		int		trcKey;
		char	*filename;
	} data;		
} TRCNEXT;

// TRC1 event data struct
typedef struct _TRC1
{
	struct {
		int		chnKey;
		int		fileType;			// MSEED | SAC | AH | SEGY | DR100 
		char	directory[MAX_DIR_STR];
		char	filename[MAX_FILE_STR];
	} keys;
	struct {
		MYTIME	sTime, eTime;						// start and end times
		int		nSamps, nGaps, nOlaps;				// totals
		int		*gaps;								// gaps[i]=startPt#, gaps[i+1]=endPt#
		int		*olaps;								// overlaps, as for gaps
		double	stats[MAXSTATS];					// MAX/MIN/MEAN
		double	sampInt;							// sample interval - msecs/sample
	} data;
} TRC1;

typedef struct _RESPFILE
{
	struct {
		int	respKey;
		int chnKey;
	} keys;
	struct {
		char	*fileName;
	} data;
} RESPFILE;

typedef struct _SYSPDFDEF
{
	int		pdfKey;
	char 	*pdfName;
	char		*pdfDescr;
	int		type, dayStart, dayEnd, year;
} SYSPDFDEF;

typedef struct _SYSPDFS
{
	struct {
		GSList	*sysPDF;				// list of System PDF's, data = SYSPDFDEF
	} data;
} SYSPDFS;

typedef struct _BLINEPT
{
	float	period;
	float	power;
} BLINEPT;

typedef struct _BLINEPTS
{
	int		numPts;					// number of points = size of array
	BLINEPT	*points;				// dynamic array, element type = BLINEPT
} BLINEPTS;

typedef struct _BLINEDEF
{
	int		 blineKey;				// primary DB key
	char	 *name, *descr;
	gboolean global;					// TRUE = global bline, i.e., across all DB's
	gboolean dir;
	gboolean active;
	int		 chnKey;
	BLINEPTS lineDef;
	int		 R, G, B;				// plot color
} BLINEDEF;

typedef struct _SYSBLINES
{
	struct {
		GSList *sysBLINE;			// list of Base-Lines, data = BLINEDEF
	} data;	
} SYSBLINES;

// PSD1 event data struct (calculate PSD's, event types PSD1 and PSD2)
typedef struct _PSD1
{
	struct {
		int		chnKey, trcKey, trcKey2;			// hold both the channel and trace keys
		char	ntw[11], stn[11], loc[11], chn[11];
		int		year, jday;							// year/jday of the trace being analyzed
		int		fileType;							// trace format
		char	filename[2][MAX_DIR_STR + MAX_FILE_STR];
		RESPFILE	resp;
		PSDCONF	conf;
		MYTIME	psdTime;							// start time of last computed PSD for trace
		gboolean overlapPSD;						// should overlap PSD be created?
	} keys;
	struct {
		int		nPeriods;							// number of periods being saved
		float	*period;							// array of freq/periods
		int		*power;								// array of powers
		MYTIME	sTime, eTime;						// start and end times of PSD
		int		psdKey;
	} data;
} PSD1;

typedef struct _PDFPNG
{
	struct {
		int chnKey;
	} keys;
	struct {
		char	*ntw, *stn, *loc, *chn;				// channel name
		GSList	*pdfReqs;							// list of system PDF's, data = PDFREQ
	} data;
} PDFPNG;

typedef struct _PSDDATA
{
	float	period;
	int		power;
} PSDDATA;

typedef struct _SPECTRODATA
{
	int		psdRef;
	GSList	*psdData;								// data = PSDDATA*
} SPECTRODATA;
	
typedef struct _SPECREQ
{
	struct {
		int		pdfType;							// SYSTEM, BASELINE
		int 	pdfKey;								// PDFCFG key, or CUSTOMPDF for user request
		int		chnKey;								// channel instance db key
		char	*sysPDFB[TTLDATES];					// System PDF Date boundaries
		float	criteria[MAXNUMPDFREQCRIT];			// request criteria specifiers - user only
		int		reqType;						
	} keys;
	struct {
		char	*pdfName;							// name of PDF
		char	*name;								// fully qualified NTW.STN.LOC.CHN
		int		numDivs;							// absolute total of PSD/time divisions
		int		numPSDs;							// the number of PSD's included in this extract
		int		numRecs;							// the number of data points returned (= numPeriods * numPSDs)
		int		numPeriods;							// the number of periods (y-axis range)
		int		numPSDday;							// number of PSD's per day
		GDate	*dateBounds[TTLDATES];				// date bounds of DB data
		float	yBounds[TTLYBOUNDS];				// y-axis bounds
		int		dbBounds[TTLYBOUNDS];				// dB boundaries
		GList	*spectro;							// list of PSD's, data = SPECTRODATA*
		int		dateFmt;							// Spectro Plot Date Format
	} data;
	struct {
		int		dBRange;							// DBRELATIVE || DBFIXED
		GDate	*dateBounds[TTLDATES];				// date bounds of display data
		int		bounds[TTLXCOORDS];					// user value of zoom bounds
		int		dbBounds[TTLYBOUNDS];				// db bounds on display
		GList	*start, *end;						// start and stop PSD list record, of data.spectro list
		guchar	*dataMap;							// the data map to magnify and plot
		int		numPSDs;							// total PSDs for this plot
		int		numDivs;							// absolute total of PSD/time divisions
		gboolean replot;							// replot required flag
		GHashTable	*gaps;							// hash table of columns that are PSD gaps
	} plot;
	gboolean	busy;								// TRUE = db request outstanding
} SPECREQ;

typedef struct _SPECPNG
{
	struct {
		int chnKey;
	} keys;
	struct {
		char	*ntw, *stn, *loc, *chn;				// channel name
		GSList	*specReqs;							// list of system SPECTRO's, data = SPECREQ
	} data;
} SPECPNG;

#if 0
typedef struct _PDFSTATSDATA
{
	struct {
		int 	chnKey;
//		int		ttlPeriods;
		GSList	*hits;								// per/pow hits, data = PDFHITS
	} keys;
	struct {
		GSList 	*stats;								// period stats, data = PDFSTATS
	} data;
} PDFSTATSDATA;
#endif

typedef struct _TRCLST
{	// list of traces and their statistics
	char	*trcName;								// name of trace
	MYTIME	times[TTLTIMES];						// start and stop time of trace
	double	stats[MAXSTATS];						// MAX/MIN/MEAN
	int		event;									// event causing crash
	double	sampInt;
} TRCLST;

typedef struct _PSDTRCD
{	// list of traces of a PDF
	int		reqID;
	struct {
		int	chnKey;
	} keys;
	struct {
		int		numRecs;
		GSList	*trcList;							// data = TRCLIST
	} data;
} PSDTRCD;

typedef struct _PSDHRD
{	// list of dates and times of PSDs of a PDF
	int		reqID;
	struct {
		int chnKey;
	} keys;
	struct {
		int		numRecs;
		MYTIME	**hrList;
		int		length;
	} data;
} PSDHRD;

typedef struct _SYSLST
{	// list of channels
	int		chnKey;
	char	*chnName;					// name of channel
	int		event;
} SYSLST;

typedef struct _CRASHD
{	// list of trace files having crashed the server during processing
	struct {
		int	which;
	} keys;
	
	struct {
		GSList	*crashList;	// data = TRCLST* or SYSLST*, i.e., key.which
	} data;
} CRASHD;

typedef struct _GLAPS
{	// structure holding gap and overlap info returned from DB
	int		start, end;
} GLAPS;

typedef struct _TRCDATA
{
	struct {
		int	trcKey;
	} keys;

	struct {
		char	*name;
		MYTIME	dates[TTLDATES];
		double	stats[MAXSTATS];			// MAX/MIN/MEAN
		GSList	*gaps;
		GSList	*olaps;
	} data;

	struct {
		int		data;						// display coverage indicator or actual data
		int		pixelPos[TTLDATES];			// defining the pixel stop and start points, X|Y
		void	*trc;
		gboolean plot;						// display trace?
		gboolean lastDay;					// special case flag: TRUE=trace is last day of dataset
	} display;
} TRCDATA;

typedef struct _CHNMETA
{
	MYTIME	dates[TTLDATES];
	char	*siteName;
	double	coords[TTLCOORDS];
	float	location[TTLLOCATIONS];
	double	sensitivity;
	char	*units;
	char	*instID;
	double	sampInt;
} CHNMETA;

typedef struct _INSTIDS
{
	struct {
		GHashTable *instIDs;		// hash table of instIDs, key=chnKey, data = char *
	} data;
} INSTIDS;
		
// Data structures of STN/CHN-specific information, used by STN system
// structure holding channel info
typedef struct _CHNDATA
{
	struct {
		int	chnKey;							// channel instance db key
	} keys;
	
	struct {
		char	*name;						// name of channel, LOC.CHN
		char	*longName;					// NTW.STN.LOC.CHN name version
		GSList	*traceFiles;				// list of trace files for this channel/time period, data=TRCDATA
		int		trcCnt;
		double	sampInt;					// sample interval - msecs/sample
		PDFREQ	*pdfReq;
		GSList	*chnMETA;					// list of channel's meta-data information, data=CHNMETA
	} data;

	struct {
		double	stats[2][MAXSTATS];			// [ABSOLUTE | RELATIVE] [MAX / MIN / MEAN]
		int		numGaps, numOlaps;
		TRCDATA	*trcData;					// structure holding all traces as a single instance
		void	*trcPlot;					// plot parameters of trace
		gboolean selected;					// channel is selected?
	} display;
} CHNDATA;

typedef struct _PDFPERIOD
{
	struct {
		int		chnKey;
		int		psdKey;
	} keys;
	
	struct {
		int	ttlPeriods;						// total number of periods
	} data;
} PDFPERIOD;

// structure holding station info
typedef struct _STNDATA
{											
	struct {
		int			stnKey;
		GSList		*chnKeys;				// channels being plotted, data=((int) channel Key)
		MYTIME		dates[TTLDATES];		// start and end dates of information
	} keys;

	struct {
		gchar		*name;					// NTW.STN pair name, set in makeSTNList, not by db
		int			numChannels;
		GSList		*chnInfo;				// list of channels for this station, data=CHNDATA
	}data;
	
	struct {
		int			chnCntPre;				// number of channels displayed before this station
		void		*pixelPos;				// pixel position of station label
	} display;

} STNDATA;

typedef struct _INSTRUMENT
{
	int		chnKey;
	char	*instID;
} INSTRUMENT;

typedef struct _INSTRUMENTS
{
	struct {
		GSList *chnKeys;					// list of channel keys, data = int *
	} keys;
	
	struct {
		GSList *instruments;				// list of instruments, data = INSTRUMENT
	} data;
} INSTRUMENTS;

// structure holding station info
typedef struct _SCRNDATA
{											
	struct {
		GSList		*chnKeys;				// channels being plotted per screen, data=((int) channel Key)
		MYTIME		dates[TTLDATES];		// start and end dates of information
	} keys;

	struct {
		GHashTable	*chnInfo;				// hash table of channel info for all stations per screen, data=CHNDATA
	}data;
	
} SCRNDATA;

typedef struct _DATEBNDRY	
{
	struct {
		MYTIME		dates[TTLDATES];
	} data;
} DATEBNDRY;

typedef struct _REPSD
{
	GDate	*dtBounds[TTLDATES];
	GSList	*channels;				// data = CHNSTATS*
}
REPSD;

// structure holding xml input info
typedef struct _XMLINFO
{
	struct {
		GSList *info;
	} data;
} XMLINFO;

// structures holding Travel-Time Table info
typedef struct _TTNAME
{
	char *name;
	int	key;	
} TTNAME;

typedef struct _TTIME
{
	int	phaseKey;
	char *phaseName;
	float depth;
	float distance;
	float ttime;
} TTIME;

typedef struct _TTMODEL
{
	char	*name;
	int		key;
	GSList	*phases;			// data = TTNAME
} TTMODEL;

typedef struct _TTINFO
{
	struct {
		TTMODEL	*model;
	} keys;
	struct {
		GSList	*ttimes;		// data = TTIME
	} data;		
} TTINFO;

typedef struct _ARRIVALS
{
	struct {
		int		modelKey;		// key of model
		float	depth;			// depth of earthquake
		float	distance;		// distance to station
		char	*stn;			// station name
	} keys;
	struct {
		GSList	*phases;		// data = (GSList*), one per phase, where data = TTIME
		GSList	*arrivals;		// data = INTERPTT, used only gui-side
	} data;
} ARRIVALS;

// structure holding EVENT information
typedef struct _EVENTINFO
{
	int		eventKey;					// db primary key
	int		eventID, originID;
	char	*description;
	char	*catalog;
	char	*timeStamp;
	char	*magType;
	int		dtime[TTLSTNEVTTIMES];
	float	loc[TTLSTNEVTLOCS];
	double	eventTE;
} EVENTINFO;

typedef struct _SEISEVENTS
{
	struct {
		int day[TTLSTNEVTTIMES];		// day to retrieve
	} keys;
	
	struct {
		GSList *events;					// data = EVENTINFO
	} data;
} SEISEVENTS;

// structure holding database maintenance request info
typedef struct _DBMAINT
{
	struct {
		char	*tableName;
	} keys;
	
	struct {
		GSList	*channels;
	} data;
} DBMAINT;
#endif
