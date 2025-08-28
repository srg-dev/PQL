#ifndef __PQL_DATA_STRUCTS_H
#define __PQL_DATA_STRUCTS_H

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <sys/types.h>
#include <stdio.h>
#include "ptime.h"

#define	offsetD(a, b) ((int) ((long) &a - (long) &b))

typedef	struct {	// structure used to pass various values from other systems to PQL
	int		destSystem;		// which system should be invoked
	int		destTab;		// which tab of system should be invoked
	struct {
		int		scale;			// y-axis scale setting
		char	*fixed[2];		// y-axis fixed scale values
	} yaxis;
} SOURCEINFO;

typedef struct _SORTPARAMS
{
	int	type;				// DIRECTSORT | USERSORT
	int sortCrit;
} SORTPARAMS;
	
typedef struct _HDRITEM
{
	char	desc[255];
	int		offset;
	int		fieldType;
} HDRITEM;

typedef struct _CRITITEM
{
	char	*desc;			// text description
	int		unique;			// TRUE | FALSE
	int		selected;		// TRUE | FALSE
	int		offset;			// offset in bytes of sort field into dataInfo structure
	int		fieldType;		// type of field we're sorting on
} CRITITEM;

typedef struct _CRITDEF
{
	int			num;		// criteria number
	gboolean	dir;		// sort direction
} CRITDEF;

typedef struct _SORTINFO
{
	int		sortNum;		// enum of TTLSORTTYPES
	GSList	*criteria;		// list of sort items, data = critItem*, initialized in sortInit()
	GSList	*user;			// user-selected sort criteria to be applied, data = CRITDEF*
} SORTINFO;

typedef	struct {
	char	name[FNAMELEN];
	int		poles[2];
	float	cutoff[2];
} filter;

typedef struct {
	int length;
	int left;
	int right;
} bounds;

typedef struct {
	double length;
	double left;
	double right;
} coords;

typedef struct {
	double max;
	double min;
	double diff;
	double diffVolts;
	double mean;
} amplitudes;

typedef struct {
	double start;
	double end;
	double shift;
	double diff;
} times;

typedef struct {
	int		rec_num;
	double	epoch;
	struct ptime	timeVal;
	double	minAmp;
	double	maxAmp;
	double	freqVal;
	int		xVal;
	int		yVal;
} pointID;

typedef struct {
	pointID	point[2];
} points;

typedef struct {
	int	pick[MAXPICKS];				// picks 
	int errorPick[MAXPICKS][2];		// 2 error picks per pick
} picks;

typedef struct {
	int			pickN;				// pick number
	gboolean	pickE;				// error pick?
	double		pickT;				// epoch time of pick
	gboolean	pickS;				// scope of pick: WIND or TRACE
} pickInfo;

typedef struct {
	int bounds[2];
} glaps;

typedef struct _SPECPT
{
	int		trans_x, trans_y;
	double	trans_mouse_freq;
	double	trans_mouse_amp;
} SPECPT;
	
typedef struct _TIMEENTRY
{	// used to hold shot start time info supplied by option -t
	char *str;
	int	year, jday, hour, minute, second, msec;
	double	epochs;
} TIMEENTRY;

typedef struct _SOHMSG
{
	int 	year, jday, hour, minute, second;
	char	*msg;
} SOHMSG;
	
typedef struct _SOHINFO
{
	int		seqnum;					// sequence number
	GSList	*msgs;					// list of SOH messages, data = SOHMSG
} SOHINFO;

typedef struct _INSTSOH
{
	char *fileName;					
	char *instID;					// instrument ID
	GSList	*SOH;					// list of SOH records for instrument, data = SOHINFO
} INSTSOH;

typedef struct _SPECSETTINGS
{
	gboolean	amp,				// TRACE = TRUE, WINDOW = FALSE
				overlay;			// ON = TRUE,    OFF = FALSE 
	gboolean	yAxis, 				// LOG = TRUE,    LIN = FALSE
				xAxis; 				// LOG = TRUE,    LIN = FALSE
	float		xMax,
				xMin,
				yMax;
	float		decades;
	int			topTrc;
} SPECSETTINGS;

typedef struct _PSDSETTINGS
{
	gboolean	hlnm;				// display HLNM?
	int			yMin, yMax;
	float		xMin, xMax;
	gboolean	overlay;
} PSDSETTINGS;

typedef struct _settings
{
	struct {
		int			numPlots;
		gboolean	amp; 					// TRACE = TRUE,   WINDOW = FALSE
  		gboolean	time; 					// RELATIVE = TRUE, ABSOLUTE = FALSE
		gboolean	gFlag, oFlag;			// indicator flags ON|OFF
	} trace;

	struct {
		int		Tamp;						//  FIXEDS, DISPS, WINDS
		gboolean  Wamp,						//  TRACE = TRUE, WINDOW = FALSE
				overlay;					//  ON = TRUE,    OFF = FALSE 
		int		zoomFactor,
				scrollFactor;
		gboolean	gFlag, oFlag;			// indicator flags ON|OFF
		int			anchor;					// LEFT, MIDDLE, RIGHT, MOUSE
		float		length;
		char		*pickFile;
		gboolean	pickKEYOn, pickCOMOn;	// ON=TRUE
		char		pickKEY[41];
		char		pickComment[81];
		char		pickTag[MAXPICKS][11];	// pick TAG Names
		gboolean	pick;					// TRACE =TRUE, WINDOW=FALSE
		gboolean	dispBox[NUMBOXES];		// display boxes: ON|OFF
	} magnify;

	struct {
		SPECSETTINGS	display;			// display settings
		int		newFilter[NUMDATASCREENS];	// TRUE on filter change, else FALSE
	} spectra;
	
	struct {
		// what drawing type each DA holds:
		int		DA[2][NUMSPLITSCRNS];						//	DA[SPLIT2|SPLIT3][TOP|MID|BOT]
		// in which DA each drawing type is held within: 
		int		DT[2][NUMSPLITSCRNS];						//	DT[SPLIT2|SPLIT3][TRACE|MAG|SPECTRA]
		int		pane;										//  start-up pane - SPLIT2 | SPLIT3
		int		traceDisp;					// TRACE data display, 
	} split;

	struct {
		int			display;				// show ALL or DISPLAYED or SELECTED headers
		gboolean	dispHdr[NUM_HDR_COLS];	// which headers should be displayed 
	} headers;

	struct {
		int			dispType;				// show LOGS for LOGFILE | LOGTRACE
		int			display;				// show ALL or DISPLAYED or SELECTED logs
	} logs;

	struct {
		struct {
			int		mode;						// display mode - TRACEMODE | GATHERMODE
			struct {
				float	gain[3];				// display gain
				char	*gainS[3];				// TRACE, MAG, & SPECTRA screens
				float	spread;					// start time spread, in seconds
			} gather;
		} display;
		struct {
			char	*lastDir;					// data directory to display
			int		openReplace;
//			int		startUP;
			int		winDims[2];
			int		panePos[3];
		} startUP;
		struct {
			char	*last[MAXFTYPES];
		} dirs;
		struct {
			int	format;							// LABEL_FILE = 0, LABEL_FILE_SHORT = 1, LABEL_HEADER = 2
			int	length;							// pixel length, 0 = full name
		} label;
		int		unitsConv;						// FALSE = input, TRUE = convert 
		struct {
			int		format;						// one of PS, PNG
			char	*command;
			char	*fileN;
			gboolean		paperSize;			// paper size: LETTER | A4 
			gboolean		paperRes;			// HIGH = TRUE, LOW = FALSE
		} print;
		struct {
			int		defaultSort;				// default sort type, enum of TTLSORTTYPES
			int		selected;					// selected sort type, enum of TTLSORTTYPES
			char	*str;						// string saved to defaults file, (de)-tokenized in defaults.c
			GSList	*sorts;						// list of SORT definitions, data = SORTINFO *
			GHashTable *instOrder;				// hash table holding serial ID order from -i option
		} sort;
		struct {
			int			numFilters;				// number of user-defined filters
			filter		**filts;
			int			AGCidx;					// the filter number of the AGC filter
			struct {
				int	winLen;
				int	dwin;
				gboolean detect;
				float threshold;
			} AGC;
		} filter;
		struct {
			int		uvwSensor;					// which sensor-specific UVW() transform?
		} tForm;
		struct {
			char *fileN;						// base filename for MARK files
			int	num;							// mark number
		} mark;
		struct {
			GtkWidget	*DA;
			GdkGC		*GC;
			int			font;					// one of AXISF, MSGF, LABELF
			PangoContext *context[NUMDATASCREENS];
			PangoFontDescription *desc[NUMFONTTYPES];
		} font;
		struct {
			GSList		*shotTimes;				// list of start times provided via -t
			int			shotLength;				// length of shot - in seconds
			struct {
				char	*directory;				// data directory to display
				char	*filter;				// file display filter
				char	*mseedInc;				// mseed include channels
				char	*mseedExc;				// mseed exclude channels
				char	*mseedLen;				// mseed file length
				gboolean replace;				// replace files?
			} file;
		} input;
		gboolean	myWordOrder;				// endianness of machine we're running on
	} general;

	struct {
		GdkColor	fg[NUMDATASCREENS], 		// element pixel hijacked for my own purposes
					bg[NUMDATASCREENS], 		// background
					sb[NUMDATASCREENS],			// sidebar
					gp[2][MAXGAPC],				// gap colours for TRACE and MAGNIFY screens
					op[2][MAXOLAPC],			// overlap colours for TRACE and MAGNIFY
					ol[MAXOVERLAYCOLOURS];
	} colors;
} plot_settings;

typedef struct _states
{
	struct {
		GSList		*traces[TTLDISPGRPS];	// group of traces - PREVGRP, CURGRP, NEXTGRP
		struct {
			struct {
				coords user;				// left and right bounds in user coordinates
				bounds pix[NUMPLOTS];		// left and right bounds in pixel coordinates
			} selection;
			bounds margin[NUMPLOTS];		// left and right bounds of trace in pixels (MAIN|SPLIT2|SPLIT3|PRINT)
		} limits;
		amplitudes absAmp;					// absolute amplitudes of traces on display
		times	absTime;					// aboslute times of traces on display
		double	relTimeLen;					// largest time length (relative) for traces on display
		float	trace_depth[NUMPLOTS];		// MAIN | SPLIT2 | SPLIT3 | PRINT
		int 	numSelected;

		GSList	*selection;					// list of selected traces, data = traceInfo

		int 	startRec;
		graph	GR[NUMPLOTS];				// graph of x-axis
		struct {
			int		label_width[NUMPLOTS];
			int		dispScale;
			double	length[2];				// plot length, [ABSOLUTE|RELATIVE]
		} plot;
		struct {
			int		defGR;					// the GR responsible for defining this point
			points	window;					// 2 points per trace, only one trace at a time
		} point;
		gboolean	selectionChanged;		// list of selected traces changed?
	} trace;

	struct {
		gboolean	dirty;					// indicate if plot should be completely re-initiated
		struct {
			int		label_width[NUMPLOTS];
		} plot;
		amplitudes	absAmp;
		times		absTime;
		struct {
			struct {
				coords user;
				bounds pix;
			} selection;
			bounds margin[NUMPLOTS];
		} limits;
		float	trace_depth[NUMPLOTS];		// MAIN | SPLIT2 | SPLIT3 | PRINT
		graph	GR[NUMPLOTS];				// MAIN | SPLIT2 | SPLIT3 | PRINT
		graph	mainGR[NUMPLOTS];			// main x-Axis lookup
		struct {
			int		defGR;					// the GR that defined these points, needed to reverse engineer values
			points	window;					// 2 points only in window mode
		} point;
		struct {
			int		curPick;				// current pick we're operating on
			picks	window;					// pick info for window
			gboolean	reset;				// trace/window setting has changed
		} pick;
		gboolean	selectionChanged;		// list of selected traces changed?
		GSList		*selection;				// list of selected traces on MAGNIFY
	} magnify;

	struct {
		struct {
			struct {
				coords user;
				bounds pix;
			} selection;
			bounds margin[NUMPLOTS];
		} limits;
		float	trace_depth[NUMPLOTS];		// MAIN | SPLIT2 | SPLIT3 | PRINT
		graph	GR[NUMPLOTS];				// MAIN | SPLIT2 | SPLIT3 | PRINT
		struct {
			double	glob[TTLSPECGLOBALS];
			int		label_width[NUMPLOTS];
		} plot;
		struct {
			int		defGR;					// the GR that defined these points, needed to reverse engineer values
			pointID	window;					// 2 points per spectra, only one at a time
		} point;
	} spectra;

	struct {
		GSList	*instSOH;					// list of SOH records for instruments, data = INSTSOH
	} data;
	
	struct {
		int		panes;								// SPLIT2 = TRUE, SPLIT3 = FALSE
		gboolean realized[3];						// has split screen yet been realized?
		gboolean resetPixmaps;
		gboolean resetPixmap[2][NUMSPLITSCRNS];		// resetHPixmaps[SPLIT2|SPLIT3][TOP|MID|BOT]
		gboolean refreshPixmap[2][NUMDATASCREENS];	// refreshHPixmaps[SPLIT2|SPLIT3][TRACESCR|MAGNIFYSCR|SPECTRASCR]
	} split;

	struct {
		gboolean reset[3];					// reset for TRACE, MAG, and SPECTRA
		gboolean refresh[3];				// refresh for TRACE, MAG, and SPECTRA
		gboolean cursor;					// reset the cursor to normal (=TRUE)
	} reset;

	struct {
		int			event;					// my own defined mouse events, see gtk_mouse.c for use
		int			mouseActive;			// which mouse button is currently depressed
		gboolean	mouseLeft;				// DEPRESSED = ACTIVE = TRUE
		int			keyActive;				// which key is currently depressed, either NONE, CTRL, SHIFT, or ALT
		struct {
			int		startRec;				// beginning record of a DRAG event
			int		endRec;
			gboolean xAxis;					// dragging has begun on the xAxis
		} drag;
		struct {
			int		startX;					// beginning X pixel of a DRAG event
			int		startY;
		} mouseLoc;
		struct{
			int		magnify;				// in overlay mode, the current TOP record
			int		spectra;
		} topRec;
		pointID		*pointPtr;			// pointer to MAG and SPECTRA point structures, used by plot_extras
	} user;

	struct{
		int			filter;				// which user-defined filter currently in effect
		gboolean	on;						// tForm is (ON|OFF) - this is a universal setting
	} tForm;

	struct {
		gboolean	newCols;
	} header;
	
	struct {
		GtkTextBuffer *buffer;				// text buffer used to display log text
		gboolean	visible;				// if LOGS tab is visible or not
		GSList		*logs;					// data = LOGDATA*
	} logs;

	struct {
		int			numFiles;				// the number of files asked to be read in by the user
		GSList		*Files;					// singly-linked list of files to be read in
		GSList		*RTfiles;				// singly-linked list of raw reftek files to be read
	} input;
		
	int 	currentPage;
	int		currentScreen;					// on display: MAIN | SPLIT2 | SPLIT3
	int		currentScr;						// parameter to makePixmap()
	int 	currentGraph;					// currently being drawn: TRACE|MAG|SPECTRA
	int		currentDA;
	char	*msg;							// msg to output on status bar
	
	struct {
		int	numTraces;						// the current number of traces read in for display
		int sourceSystem;					// system passing data to PQL, most recently
		int	numCPUs;						// number of CPU's on the executing machine
		struct {
			int state;						// current state of drawing on MAIN tab
		} plot;
	} general;
} plot_states;

typedef struct _HEADINFO
{
  int		data_form, dataSize;
  gboolean	endianSwap;
  double	sampInt;				// msecs/sample
  int		year, day, hour, minute, second, m_secs;
  double	epochs;					// start time in epoch secs, format: epoch Seconds.msec
  double	epochStop;
  double	length;					// in seconds
  int		gainConst;
  float		scale_fac;
  int		units;				// one of PQL_UNITS
  char		*descr1, *descr2, *longFN, *shortFN;
  char		network[10], station[10], location[10], channel[10], sensorID[10];
  gint64	MSstartTime;			// start time in mini-seed format, only used for multi-plexed data reading
  void		*fileHdr;				// pointer to file format-specific header, valid only for AH, SAC, SEGY
} HEADINFO;

typedef struct {
	char		*tFormStr;				// string describing tform sequence
	char		*data;					// data buffer
	struct {
		int	numSamples;
		int	data_form;
		double	sampInt;
		double	secsPsamp;				// seconds/sample
	} head;
	amplitudes	amps;
} DATAINFO;

typedef	struct _TRACEINFO 
{
	struct {
		int original;					// original position at startup
		int current;					// current position in the list
	}	position;						// position # in our list
	char		*fileName;				// Filename of trace
  	int			fileType; 				// DATA TYPE
	HEADINFO	head;					// PQL header info
	DATAINFO	dataInfo;				// buffer of data and stats
	GSList		*dataFilt;				// singly-linked list of filtered data buffers: data=DATAINFO
	gboolean	sticky;					// TRUE = data is in memory only, and is not readable from file
	DATAINFO	tForm;					// transformed data
} TRACEINFO;

typedef struct _OVERLAP 
{
	int		bounds[2];						// pixel start and stop
	int 	len;							// length, in pixels
	char	*data;	
} OVERLAP;

typedef struct _traceInfo
{
	TRACEINFO	data;

	struct {
		int		numGaps, numOlaps, numPicks;	// number of gaps, overlaps, and saved picks
		int		*gaps;						// where are there gaps in the data?
		int		*overlaps;					// where are there overlaps in the data?
//		GSList	*gaps;						// list of gaps, data = glaps
		GSList	*olapData;					// list of overlaps, data = OVERLAP
		double	RMS;						// RMS of entire trace
		pickInfo *picks;					// save the pick information for replotting
		graph	GR[NUMPLOTS];
		gboolean selected;					// TRUE if trace is selected by user
		struct {
			int		increment;
			double	startTime, 
					endTime1, 
					endTime2, 
					offset;
			double	minAmp, maxAmp;
		} plot;
	} trace;

	struct {
		gboolean	selected;
		int		num_points_shown;
		double	begin, end, offset;
		char	*data_start;
		graph	GR[NUMPLOTS];
		struct {
			int 	increment;
			double	minAmp, maxAmp;
			PangoLayout *layout[2];
		} plot;
		amplitudes	amps;
		double		RMS;
		struct {
			picks	trace;					// pick info per trace
		} pick;
		struct {
			points	trace;					// 2 points per trace
		} point;
	} magnify;

	struct {
		float	ampMax;
		float	ampMin;
		double	avg;
		float	beg, end, fill_beg, fill_end;
		int		numPts;
		char	*data;
		float	*data_end;
		graph	GR[NUMPLOTS];
	} spectra;
	
	struct {
		double	epochS;						// epoch seconds start time of desired extract
		float	length;						// length in seconds
		char	*description;				// string to become data.fileName
		struct {
			char	*inetAddr;				// CWB internet address
			int		port;					// port number
			void	*mainReturn;			// where *CWBReturn() returns to (back to main)
		} CWB;
	} excerpt;
	
	struct {								// available only to data library
		int		blkSize;
		GSList	*packets;					// list of packet numbers for this trace, data = MMPACKET*
	} mplex;
	
} traceInfo;

#define CHNARGSEP ","
typedef struct {
	GSList	*incChans;					// list of channels to be included, data = char*
	GSList	*excChans;					// list of channels to exclude, data = char*
	int		length;						// length, in seconds, of trace to extract, 0 = entire trace
} MPLX_MSEED;

typedef struct _TRC_N_DATA
{
	traceInfo	*trc;
	DATAINFO	*data;
} TRC_N_DATA;

typedef struct _XYZCOMPONENT
{
	int			sensor;			// transform function is sensor-specific
	GHashTable	*comp;			// key = 'E|N|Z', data = TRC_N_DATA
	gboolean	success;
} XYZCOMPONENT;

typedef struct _LOGTEXT
{
	double	startT;
	char	*text;
} LOGTEXT;
	
typedef struct _LOGDATA
{
	char	*fileName, *inst, *station;
	double	startT, endT;		// in epochs
	GSList	*text;				// data = LOGTEXT*
} LOGDATA;

#include "util_externs.h"

#endif
