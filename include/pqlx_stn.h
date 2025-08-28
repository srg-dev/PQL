#ifndef	__PQLX_STN_INCLUDE
#define	__PQLX_STN_INCLUDE

//	defines related to STN system

enum {			// STNMAIN Display Modes
	STNCVG=0,	// coverage (n.b. this must be zero!)
	STNDTA,		// data
	TTLSTNMAINDISPLAYMODES
};
//#define TTLSTNEVTDIPSLAYMODES 2
//#define TTLSTNANLDIPSLAYMODES 2

enum {
	STNNOSRVR,		// not connected
	STNBUSY,		// busy retrieving data
	STNDRAW,		// busy drawing data
	STNCNCT,		// busy connected
	STNNOSEL,		// nothing selected, nothing to draw
	TTLSTNMSGS
};

enum {
	STNMSG,
	STNTEXT,
	STNFGBG,
	STNSBAR,
	STNGAPI,
	STNOLAP,
	STNARR,
	STNPICK,
	TOTALSTNCONTEXTS
};

enum {
	STNCURDT,		// STNDATA of current time period
	STNPREVDT,		// STNDATA of previous time period on display
	STNNEXTDT,		// STNDATA of next time period
	STNPREVSTN1,	// STNDATA of previous STN list
	STNNEXTSTN1,	// STNDATA of next STN list
	TTLSTNLOOKAHEADS,
	STNINIT,		// only used for switch(), dus, no array access
	STNREINIT
};
#define TTLPERIODSTNDATA	(STNNEXTDT+1)
#define STNPREVSTN	(STNPREVSTN1-STNPREVSTN1)	// = 0
#define STNNEXTSTN	(STNNEXTSTN1-STNPREVSTN1)	// = 1
#define TTLSTNSTNDATA	(STNNEXTSTN+1)

enum {			// these must be conceptually ordered same as above: STNCURDT, STNPREVDT, STNNEXTDT
	CURDTS,		// current date start	= 0
	CURDTE,		// current date end		= 1
	PRVDTS,		// prev date start		= 2
	PRVDTE,		// prev date end		= 3
	NXTDTS,		// next date start		= 4
	NXTDTE,		// next date end		= 5
	TTLSTNDATES
};

enum {
	STNSTATSPLT,
	STNPDFPLT,
	STNCHNMETAPLT,
	STNNOPLT,
	STNTRCSELPLT,
	STNTRCSELSTN,
	STNTRCSELCHN,
	STNTRCSELDAY,
	STNCHNSELECT,
	TTLSTNPLTS
};

enum {
	STNTRCPQLN,		// take data to PQL - start over
	STNTRCPQLA,		// take data to PQL - add
	STNTRCEVTN,		// take data to EVENT
	STNTRCEVTA,
	STNTRCANAN,		// take data to ANALYSIS
	STNTRCANAA,
	STNTRCDATA,		
	TTLSTNTRCACTIONS
};

#define MAXSTNXDAYS	(365*5)
#define MAXSTNDAYS	60
#define MAXSTNLINES	36
#define MAXTHREADSPCPU 2

enum {			// Control Panel defines
	STNMAINCTRL,
	STNANLCTRL,
	STNEVTCTRL,
	STNSPECCTRL,
	STNPSDCTRL,
	STNPMCTRL,
	STNTTLCTRLS
};

enum {		// STN colours
	STNFG,		// fore-ground
	STNBG,		// back-ground
	STNSB,		// side-bar
	STNGP,		// gap
	STNOP,		// overlap
	STNPA,		// predicted arrival
	STNUP,		// user picks
	STNMAXCOLOURS
};

enum {
	STNHL=STNGP,	// high/low noise model line colour
	STNPMC,			// PM compass lines
	STNPMSP,		// PM start point
	STNPMEP,		// PM end point
	STNPMEBA,		// PM event back azimuth
	STNPMLA,		// PM line azimuth
	STNMAXPOPUPCOLOURS
};

enum {
	STNUNITS,
	STNOVERLAY,
	STNXSCALE,
	STNZANCHOR,
	STNAMPSCALE,
	STNEPIORDER
};

enum {
	INIT1,
	INIT2,
	NEXT
};

enum {
	STNCHNFLTSGNL,
	STNFLTSGNL,
	STNFLTCBOSGNL,
	STNANATFORMSGNL,
	STNOVLYSGNL,
	STNXSCLSGNL,
	STNSRVRDFAULTSGNL,
	STNDBDFAULTSGNL,
	STNDISPCVGSGNL,
	STNDISPDATASGNL,
	STNYCHNGROUPSGNL,
	MAXSTNSIGNALTYPES
};

enum {
	STNDRAWINIT,
	STNDRAWREFRESH,
	STNDRAWDONE,
	STNDRAWNOTDONE,
	TTLSTNDRAWSTATES
};

enum {
	STNREDRAWREQ=1,
	STNREINITREQ,
	STNNOREQ
};

enum {					// radio button group defines
	STNWSCALE,
	TTLSTNRADIOS
};

// STNEVENT screen enums
enum {
	STNEVTEPINEAR1ST,	// plot nearest arrival to event first
	STNEVTEPIFAR1ST,	// plot farthest arrival to event first
	STNEVTEPIORDERS,
};

enum TRCDISPORDER {
	EVTDISPEVT,
	EVTDISPSTN,
	TTLDISPORDERS
};

#if 0
enum EPISORTORDER {
	EVTSORTNEAREST, 		
	EVTSORTFARTHEST			
};
#endif

enum EVTALIGNMENT {
	STNALIGNUSER=ABSOLUTE+1,
	STNALIGNPRDCT,
	MAXALIGNMENTS
};

enum {
	EVTEVTCAT,
	EVTEVTUSER,
	TTLEVTEVTTYPES
};

enum {
	ANATFORMS,
	ANADEMEAN,
	ANADIFFERENTIATE,
	ANAINTEGRATE,
	ANAREVPOL,
	ANADETREND,
	ANASPECTRA,
	ANAPSD,
	ANAUVW,
	ANADECONVOLVE,
	ANAPARTMOTION,
	TTLSTNANALYSES,
	ANAORIG,
	ANAFILTER
};

enum {
	ANAPLOT					// used for trcData->magnify.GR[] array
};

enum {
	STNCHNGRPMAIN,
//	STNCHNGRPEVENT,
	STNCHNGRPANALYSIS,
	TOTSTNCHNGROUPS,		// undone: until event and analysis implemented
};

enum {
	STNDRAGUP,
	STNDRAGDOWN
};

enum {
	STNSHIFTFORWARD,
	STNSHIFTBACKWARD
};

enum {
	STNCOUNTS,
	STNSENSITIVITY,
	TTLSTNUNITS
};

enum {
	YSCALEMANAGE,
	YSCALETRACE,
	YSCALEWINDOW,
	YSCALEFIXED,
	YSCALEFIXEDENTRIES
};
#define YSCALEAUTO "AUTO"

enum {
	STNSORTNAME,
	STNSORTTIME,
	STNSORTEPI
};

enum {
	STARTABS,
	STOPABS,
	TTLABSTIMES
};

enum {
	STN_POPUP_WIN_SINGLE,
	STN_POPUP_WIN_SEPARATE
};

enum {
	SPECTRA_WIN,
	PSD_WIN,
	PM_WIN,
	TTLPOPUPWIN
};

enum {
	PM_PLOT=TTLPOPUPWIN,
	PM_TRC,
	TTLPMDA
};

enum {
	PU_SPECTRA_DRAW_TOP,
	PU_SPECTRA_POINT,
	TTLPUACTIONS
};

enum {
	EVTCLEAR,
	EVTGET
};

typedef struct _INTERPTT
{	// struct holding interpolated TTIMES
	char *phaseName;
	float ttime;
} INTERPTT;

typedef struct _YCHNSCALE
{
	char	*name;
	char	*loc, *chn;
	char	*scale[2];
} YCHNSCALE;

typedef struct _YCHNGROUP
{
	char	*name;
	GSList	*chnScales;		// DATA = YCHNSCALE
} YCHNGROUP;

typedef struct _AUTOSCALE
{
	char	*name;
	GSList	*channels;
} AUTOSCALE;

typedef struct _TRCPLOT
{	// info for plotting trace indicators/data
	int		chnNum;
	double	time[TTLDATES];
	int		pixel[TTLDATES];
	int		bdry[TTLDATES];		// absolute pixel boundaries
	CHNDATA	*chnData;
	double	stats[MAXSTATS];
} TRCPLOT;

typedef struct _MODELPHASES
{
	int			modelKey;		// DB Key of model
	GtkWidget 	*widget;		// frame widget to display all phases
	GSList		*phasesR;		// list of radios buttons, data = GtkWidget*
} MODELPHASES;

typedef struct _SET_ARGS
{
	int		 setting;
	gboolean *val;
} SET_ARGS;

#define MODELSEP "`"
#define MODELSSEP ":"

#define STNTOPM	0
#define STNBOTM 30
#define STNRGTM 45
#define STNLFTM 70
#define YAXISW	10

#define STNTOPBD	(STNTOPM+(chnNum*stnStates.display.chnH)+1) 
#define STNBOTBD	(STNTOPM+((chnNum+1)*stnStates.display.chnH)-1)

#define EVTTOPM		30
#define EVTBOTM		TRC_B_MARGIN
#define EVTTOTALM	(EVTTOPM+EVTBOTM)
#define EVTTOPBD	(EVTTOPM+(i*chnH)+1) 
#define EVTBOTBD	(EVTTOPM+((i+1)*chnH)-1)

#define ANATOPMSG1	20
#define ANATOPMSG2	20
#define ANATOPMSG	(ANATOPMSG1 + ANATOPMSG2)
#define ANATOPXIS	25
#define ANATOPM		(ANATOPMSG1+ANATOPMSG2+ANATOPXIS)
#define ANABOTM		TRC_B_MARGIN
#define ANALFTM		(STNLFTM+10)
#define ANARGTM		10
#define ANATOTALM	(ANATOPM+ANABOTM)
#define ANATOPBD(x)	(x+(i*trcH)+1) 
#define ANABOTBD(x)	(x+((i+1)*trcH)-1)
#define SPECTOPM	25
#define PM_MARGIN	45
#define	DOTSIZE		2
#define	DOTSIZEF	2

#define TORAD(a)	((double)((a)*G_PI)/180.)
#define TODEG(a)	((double)((a)*180./G_PI))

typedef struct _STNCHN 
{	// structure holding NTW:STN and LOC:CHN pairs, see gtk_stn_data.c for usage 
	int		key;
	char	*name1;
	char	*name2;
	char	*shortName;
	char	*longName;
} STNCHN;

typedef struct _ANALYSIS
{
	TRACEINFO	orig;
	DATAINFO	mod;
} ANALYSIS;

typedef struct _STNSETTINGS
{
	int			ttlStns;					// total number of stations to traverse
	int			pqlDest;					// one of PQLTRC, PQLMAG, PQLSPLT2, PQLSPLT3
	struct {
		GDate	*dates[TTLDATES];			// absolute start and end dates of database
		int		yScale[TTLSTNSCREENS];		// y-axis scale: FIXEDS, DISPS, WINDS
		char	*yFixed[TTLSTNSCREENS][2];	// y-axis fixed scale values, as string
		gboolean yAxis[TTLSTNSCREENS];		// display y-axis?
		int		xScale;						// x-axis scale: ABSOLUTE | RELATIVE | STNALIGNUSER | STNALIGNPRDCT
		int		overlay[TTLSTNSCREENS];		// overlay
		int		units[TTLSTNSCREENS];		// display units: STNCOUNTS | STNSENSITIVITY
		gboolean unitsD;					// display the sidebar control?
	} display;

	struct {
		int			data;					// one of STNCVG | STNDTA
//		gboolean	yAxis;					// display yAxis? TRUE=NO
		int			lines;					// maximum number of lines for display
		int			days;					// number of days to display
		struct {
			CHN_FILTER	filters;
			int			active;				// which channel filter is active
		} chnFilter;
		struct {
			GSList	*scales;				// list of channel group scalings, data = YCHNSCALE		
			GSList	*groups;				// list of groups of channel scalings, data = YCHNGROUP
			char	*selected;				// saved group - reset at startup
		} yChnScale;
		int			shiftDays;				// number of days to shift display
	} main;
	
	struct {
		int 	dispOrder;					// display order of traces, EVTDISPSTN | EVTDISPEVT
		int 	epiOrder;					// order of epicentral sort
		char	*modelsSV;					// disk version of saved models and phases, data = char *
		GSList	*models;					// memory version of saved models as list, data = char *
		char	*modelSel;					// default selected model
		char	*alignPhase;				// phase for x-axis alignment
		char	userP[MAXPICKS][11];		// User Pick Names
		int		userPA;						// User Pick # to align X-Axis
	} event;

	struct {
		struct {
			char	*selected;					// saved group - reset at startup
		} yChnScale;
		int			anchor;						// zooming anchor
		struct {
			gboolean type[TTLPOPUPWIN];			// STN_SPECPSD_WIN_SINGLE | STN_SPEC_WIN_SEPARATE
			GdkColor colors[TTLPOPUPWIN][STNMAXPOPUPCOLOURS];	// colours - FG, BG, SB, HLNM
			GdkGC	*ctxt[TTLPOPUPWIN][STNMAXPOPUPCOLOURS][3];	// graphics contexts for drawing
			SPECSETTINGS	spectra;			// settings for Spectra display
			PSDSETTINGS		psd;				// settings for PSD display
			struct {
				int		width, height;			// width/height
				int		xPos, yPos;				// position on screen
			} dims[TTLPOPUPWIN];
		} popup;
	} analysis;
		
	struct {
		GdkGC	*ctxts[TTLSTNSCREENS][TOTALSTNCONTEXTS][3];		// graphics contexts for drawing
		GdkColor	colors[TTLSTNSCREENS][STNMAXCOLOURS];		// colours
	} drawing;

	struct {
		GtkWidget	*DA;
		GdkGC		*GC;
		int			font;						// one of AXISF, MSGF, LABELF
		PangoContext *context[NUMDATASCREENS];
		PangoFontDescription *desc[NUMFONTTYPES];
	} font;
	
	struct {
		gboolean startup;
		char	*server;
		char	*database;
		int		stnXDays;					// do not display stations having no data for stnXDays days
	} general;
} STNSETTINGS;

typedef struct _EVTDATA
{
	traceInfo	*trc;
	int			trcNum;						// trace number on display
//	float		*original;					// float version of original trace data
	struct {
		gboolean	selected;					// TRUE | FALSE
	} display;
} EVTDATA;

typedef struct _USERPICK
{
	int	pickNum;
	double time;
} USERPICK;

typedef struct _ANADATA
{
	traceInfo	*trc;						// point back to PQLDATA.trc, freed elsewhere
	GSList		*chnMETA;					// data = CHNMETA
	int			trcNum;						// trace number on display
	float		*original;					// float version of original trace data
	DATAINFO	dataInfo;					// modified/transformed data
	struct {
		gchar	*dispStr;					// transform string
		gboolean deconvolve;				// deconvolution applied?
	} tForm;
	struct {
		double	stats[2][MAXSTATS];			// [ABSOLUTE | RELATIVE] [MAX / MIN / MEAN]
		gboolean selected;					// channel is selected?
//		int		numGaps, numOlaps;
//		void	*trcPlot;					// plot parameters of trace
	} display;
	ARRIVALS	*arrivals;					// list of arrivals for selected EVENT
	GSList		*userPicks;					// list of user picks, data = USERPICK
} ANADATA;

typedef	struct _ANAPOPUPDATA
{	// values of individual traces' analysis
	traceInfo	*trc;
	struct {
		graph	gr;
	} spectra;
} ANAPOPUPDATA;

typedef struct _ANAPOPUP
{	// values for all of SpecPSD window
	int			which;						// which popup, needed for unions
	GtkWidget	*topW;						// top level window
	GtkWidget	*da;						// da to plot to, same as hash key
	GtkWidget	*sb;						// side-bar widget
	GdkPixmap	*pmap;						// pixmap to expose
	GtkWidget	*statusBar;					// PM status bar
	union {
		SPECSETTINGS	spec;				// SPECTRA display settings
		PSDSETTINGS		psd;				// PSD display settings
	} settings;
	union {
		struct {
			double	glob[TTLSPECGLOBALS];
			double	user_length;
			gboolean	overlay, amp;
			graph	gr;
		} spectra;
		struct {
			int			ptNum;
			GdkPoint	*line;
			GtkWidget	*da;				// da of traces
			GdkPixmap	*pmap;				// pmap of the traces	
		} PM;
	} plot;
	GSList		*data;						// list of transformed traces, data = ANAPOPUPDATA
} ANAPOPUP;

typedef struct _STNSTATES				
{	// structure maintaining display information
	struct {
		int		state;							// the state of the user's request, STNBUSY or STNDRAW
		int		type;							// what kind of request
		int		dataBUSY;						// TRUE if getAllSTNdata() is busy
		int		plotBUSY;						// TRUE if plotting data
		GDate	*endDate;						// user-specified endDate for data request
	} request;
		
	struct {
		gboolean	initiate;					// TRUE on first time draw of data
		int			dataState;					// STNDRAWINIT, STNDRAWREFRESH, STNDRAWDONE
		gboolean	dataRequest;				// data must be re-drawn
		gboolean	pdfBUSY;					// TRUE if pdf lookup thread, getSTNpdf(), is active
		gboolean	pdfRESET;					// TRUE if new request must be processed
		int			pdfCtr;
		int			ttlChn;						// total # channels on display
		int			chnH;						// channel height in pixels
		float		dayW;						// day width in pixels
		CHN_FLT_STATE chnFilter;				// Current channel filter selected
		struct {
			int		active;						// current list item selected
			YCHNGROUP *chnGroupScale;			// Current channel group scale selected
			GSList	*autoScale;					// list of CSD's having AUTO specifiers
		} yScaleGrp[TTLSTNSCREENS];
		struct {
			int		active;
			gboolean on;
		} dataFilter;
		struct {
			gboolean	valid;					// TRUE if bounding boxes are currently valid
			gboolean	mouseDown;				// TRUE = mouse button down
			gboolean	dragging;				// TRUE = current drag action is valid
			int			event;
			int			action;
			struct {
				GSList		*list;				// list of bounding boxes, data = GdkPoint[2] = active
				GdkPoint	*active;			// currently active bounding box -> GdkPoint[2]
				int			chnStart;			// starting channel number of selection
				int			curChan;			// current bounding channel of selection
			} box;
			int			keyActive;
		} user;
	} display;
	
	struct {
		PTIME	dateRng[TTLSTNDATES];			// date ranges for CURDT, PREVDT, and NEXTDT
		GList	*startStn, *endStn;				// ptrs START and END elements of stnList, (END = NEXT START!)
		GSList	*stns[TTLPERIODSTNDATA];		// singly-linked list of stations to plot, data=STNDATA
		int		stnStrtNum, stnCnt;				// starting number of station and count
		int		ttlChnCnt[TTLPERIODSTNDATA];	// total count of channels per screen
		GList	*stnList;						// Master list of stations to traverse
		GSList	*chnList;						// Channels of data on display, DATA = CHNDATA;
		GSList	*selections;					// selected channels, DATA = CHNDATA
		GSList	*pdfList;						// List of PDF plots
		graph	mainGR;							// GR of main screen
	} main;
	
	struct {	// NEXT & PREV station data (by station)
		GList	*startStn, *endStn;				// pointers to Starting and Ending Stns of stnList
		GSList	*stns;							// list of stations, data=STNDATA
		int		ttlChnCnt;						// total count of channels per screen
	} NP[TTLSTNSTNDATA];									

	struct {
		GSList		*models;					// list of known models, data = TTMODEL
		TTMODEL		*model;						// current selected model
		GSList		*events;					// list of EVENTS currently populating the EVENT dialog, data = EVENTINFO
		EVENTINFO	*event;						// current selected event for display/processing
		EVENTINFO	user;						// user-defined/supplied event
		GSList		*arrivals;					// list of arrivals for all stations on display, data = ARRIVALS
		int			curPick;					// current user pick selected to plot
		gint		evtPhInfoX, evtPhInfoY;		// screen location of EvtPhInfo popup
	} event;
	
	struct {
		GSList	*traces;						// ANALYSIS data on display, data = PQLDATA
		GSList	*selections;					// list of selected traces, data = PQLDATA
		struct {
			int		active;
			gboolean on;
		} dataFilter;
		struct {
			int		transform;					// current user transform request
			gboolean common;					// transform is same for all traces?
			gchar	*tFormStr;					// common transform display string
//			gboolean	zoomOut;				// zoomming out?
			GHashTable	*tForms;				// hash table of transforms per trace
			gboolean list;						// transform has been applied?
			double	left, right;				// zooming out bounds
		} tForm;
		struct {
			graph	gr[TTLGR];					// x-axis gr, MAINGR and LOCALGR
			graph	absGR;						// LOCALGR in absolute terms
			struct {
				struct {
					coords user;				// user coords of displayed data
					bounds pix;					// pixel coords of selection
				} selection;
				double times[TTLABSTIMES];		// absolute start and stop in display
			} limits;
			struct {
				points user;					// selected point, as offset
			} extra;
		} plot;
		struct {
			struct {
				GHashTable	*data;				// hash table holding popup data, data = ANAPOPUP
			} separate;
			struct {
				GtkWidget 	*win;				// common spectra/psd window - top level
				ANAPOPUP	*data;				// data currently on display
			} same;
		} popup[TTLPOPUPWIN];					// one for each popup (Spectra, PSD, PM)
	} analysis;
	
	struct {
		int	keyActive;							// which key is active - NONE, CTRL, SHIFT, or ALT
		gboolean mouseDown;
	} user;
	
	struct {
		gboolean	dirty[TTLSTNSCREENS][TTLSTNMAINDISPLAYMODES];	// TRUE = pixmap requires refreshing
		GdkPixmap	*main[TTLSTNMAINDISPLAYMODES];		// STNMAIN pixmaps to display
		GdkPixmap	*event[TTLPMAPTYPES];				// STNEVENT pixmaps to display
		GdkPixmap	*analysis[TTLPMAPTYPES];			// STNANALYSIS pixmaps to display
		GdkPixmap	*specPsdPrint;				// Spectra/PSD pixmap to print
	} pmap;

	int		screen;								// STNMAIN, STNEVENT, STNANALYSIS
	
	struct {
		GHashTable	*instIDs;	// hash table of instIDs, key = chnKey, data = char*
	} data;
	
} STNSTATES;

#endif
