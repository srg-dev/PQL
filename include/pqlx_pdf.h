#ifndef	__PQLX_PDF_INCLUDE
#define	__PQLX_PDF_INCLUDE

//	defines related to PDF system

#include "pqlx_pdf_head.h"

enum {
	PDFNB,
	SPECTRONB,
	TTLPSDNB
};

enum {		// PDFMAIN Display Modes
	PDFSTND,		// by Station
	PDFPDFD,		// by PDF
	PDFBOTHD,		// by Both
	PDFGROUP,		// by group of channels
	NUMPDFMAINDISPLAYMODES
};

enum {		// the buttons specifc to each PDFMAIN display modes
	MBUT1,
	MBUT2,
	MBUT3
};

enum {		// the four types of combo boxes on PDFMAIN screen
	CBONSL,		// NTW.STN.LOC
	CBONSLC,	// NTW.STN.LOC.CHN
	CBOCHNG,	// CHN Groups
	CBOPDF		// PDFs
};

enum {		// the four displays for the DETAIL screen - 
	NW,
	SW,
	NE,
	SE,
	ALLCELLS
};

enum {
	UL,
	UM,
	UR,
	ML,
	MM,
	MR,
	LL,
	LM,
	LR,
	TTLREQDISPCELLS
};

enum {
	ULM,
	URM,
	LLM,
	LRM,
	TTLMOVIEDISPCELLS
};

enum {
	XAXISBNDS
};

enum {
	PDFREQPDF,
	PDFREQSPECTRO,
	TTLPDFREQTYPES
};

enum {		// PDF Request Screen defines
	PDFREQSTART,		// range start - year, day, hour params
	PDFREQEND,			// range end - year, day, hour params
	PDFREQBOUNDS
};

enum {
	PDFREQYR,
	PDFREQMO,
	PDFREQDY,
	PDFREQDAYS
};

enum {
	PDFREQHR,
	PDFREQMN,
	PDFREQHOURS
};

enum {
	PDFREQNUMEXTRACT,	// number of days to extract
	PDFREQFREQUENCY		// how often to extract them
};

enum {
	YEARH,
	JDAYH
};

enum {
	HORIZ,
	VERT
};

enum {
	TRACECNT,
	VALIDTRC,
	TOTALTRCCNTS
};

enum {
	STATE_NOSRVR,
	STATE_NOPDF,
	STATE_BUSY1,
	STATE_BUSY2,
	STATE_BUSY3,
	STATE_FINISH,
	STATE_NOPSD,
	STATE_SRVRCNCT,
	STATE_NOMOVIEPDF,
	STATE_MVRESIZE,
	STATE_BADSR,
	STATE_NOSPECTRO,
	TTLSTATES
};

enum {
	PDFALLSELECT,
	PDFALLTOGGLE,
	TTLPDFPRINTCMDS
};

enum {
	PDFCHNFLTSGNL,
	PDFSRVRDFAULTSGNL,
	PDFDBDFAULTSGNL,
	SPECTROCHNFLTSGNL,
	MAXPDFSIGNALTYPES
};

enum {	// data points
	X,
	Y
};

enum {
	PMW,	// pixmap width
	PMH,	// pixmap height
	PMM,	// pixmap top margin
	TTLPMD
};

enum {
	INCLUSIVE,
	EXCLUSIVE
};

enum {
	DBTRCLIST,
	PQLTRCLIST
};

enum {
	PDFDTRCNOPLOT,
	PDFDTRCENTIRE,
	PDFDTRCPSD,
	MAXTRCEXTRACTS
};

enum {
	TRCEXTRACT
};

enum {
	FDUNIT_PER,
	FDUNIT_FREQ,
	TTLFDUNITS
};

enum {
	MVDISP1,			// single movie display cell
	MVDISP2,			// 4 movie display cells
	TTLMOVIEDISPLAYS
};

enum {	// Movie Cells
	PDFM1,
	PDFM2,
	PDFM3,
	PDFM4,
	PDFM5,
	TTLMOVIESCREENS
};

enum {	// movie commands
	PDFM_BACKWARD,
	PDFM_PAUSEB,
	PDFM_FORWARD,
	PDFM_PAUSEF,
	TTLPDFMCMDBUTTONS,
	PDFM_BEGIN = TTLPDFMCMDBUTTONS,
	PDFM_END,
	TTLPDFMCMDS,
	PDFM_NONE
};

enum {
	PDFM_PAUSE,
	PDFM_START,
	PDFM_CANCEL
};

enum {
	PDFM_FRAMENUM,
	PDFM_TTLFRAMES,
	TTLPDFFRAMECOUNTERS
};

enum {		// radio button identifiers, param to radioOption()
	RDPDFHR,
	RSPDF
};

enum {
	COLPDF,
	COLSPEC,
	TTLSPECCOLS
};

enum {
	ZOOMSINGLE,
	ZOOMALL,
	TTLZOOMSCOPES
};

enum {
	PDFCBOZOOMSCOPE
};

enum {
	PDF_P_DB,
	PDF_P_DISP
};

enum {
	BLINE_MK_CTRL,
	BLINE_MK_LIST,
	BLINE_MK_LISTDEL
};

#define	SPECTROPDF(rowNum)	{\
	makeThread(ASYNC, (GThreadFunc) spectroPDF, GINT_TO_POINTER(rowNum), NULL);\
}

#define REFNUM(scr, row, col)	(scr*MAXNUMBERS + row*scrSizes[scr][COL] + col)
#define ROWNUM(a)	(a / scrSizes[screen][COL])
#define COLNUM(a)	(a % scrSizes[screen][COL])

#define	PDFLABELFONT	"Sans 11"
#define	WWWLABELFONT1	"Sans 14"
#define WWWLABELFONT2	"Sans 13"
#define WWWLABELFONT3	"Sans 12"
#define WWWLABELFONT4	"Sans 11"
#define WWWLABELFONT5	"Sans 9"
#define WWWLABELFONT6	"Sans 7"

#define PDFMAXFRAMERATE 30
#define PDFMINFRAMERATE -30

#define XAXISBNDSTRSEP	"|"
#define XAXISBNDITEMSEP "`"
#define XAXISBNDITEMSEPC '`'

#define BLINEPCTDEF	15
typedef struct _BLINECTRL
{
	gulong		id;					// signal handler id
	GtkWidget	*widg;				// controls screen check box widget
	BLINEDEF		*blineDef;			// BLINEDEF
	gboolean		plot;				// to be plotted?
} BLINECTRL;

typedef struct _XAXISBOUNDSDEF
{
	char	*chnGrp;					// 2-letter channel group identifier
	char	*bounds[TTLPERBOUNDS];		// string version of bounds, for display only
	PDFXAXIS	xAxis;					// float version of bounds, internal use
} XAXISBOUNDSDEF;

typedef struct _FILMRESIZE
{
	int screen;
	int	frameNo;
} FILMRESIZE;

typedef struct _PDFFILMFRAME
{
	long	movieID;			// movie ID = PDFFILM.movieID
	int		frameNo;			// frame number, PDFFILM.frames hash table key
	PDFREQ	*pdfReq;			// PDFREQ data
	GdkPixmap	*pmap;			// picture of frame
} PDFFILMFRAME;

typedef struct _PDFFILM
{
	long		movieID;		// stnStates.movie.movies hash table key = DA address
	int			screen;			// screen playing the movie
	GDate		julian[2];		// start and stop date of film
	int			interval;		// number of days in each frame
	gboolean	cumulative;		// is movie cumulative?
	int			frameNo;		// current frame displayed
	int			lastFrame;		// total number of frames
	int			cmd;			// current PLAY command
	GHashTable	*frames;		// data = PDFFILMFRAME 
} PDFFILM;

typedef struct _TRACEARGS
{
	GdkPixmap	*pixmap;			// pixmap to draw to
	int			dims[TTLPMD];		// trace plot dimensions
	gboolean	callType;			// SYNC | ASYNC
	gboolean	(*returnFunc)(gpointer);		// where to return control if ASYNC
} TRACEARGS;

typedef struct _IDXTMPARGS
{
	int		reqType;
	int		table;
} IDXTMPARGS;

typedef	struct	_PDFSETTINGS
{
	struct {
		int 		display;		// PDFSTND, PDFPDFD, PDFBOTHD, PDFGROUP
		CHN_FILTER	chnFilter;
		struct {
			char	*string;		// string version saved to disk
			GSList	*defs;			// list version in memory, data = XAXISBOUNDSDEF
			gboolean	active;		// ON | OFF
		} xAxis;
		struct {
			int	blineKey;		// selected baseline
			int pct;				// selected percent
		} bline;
	} main;
	
	struct {
		int			trcExtract;		// TRC extract option (NOPLOT, ENTIRE, PSD)
		gboolean	pdfHrMode;		// pdf HR selection is INCLUSIVE | EXCLUSIVE
		struct {
			char	*inetAddr;		// i-net address of CWB server
			int		port;			// port
		} CWB;
	} detail;

	struct {
		gboolean	display;					// MVDISP1 | MVDISP2
		int			frameRate[TTLMOVIESCREENS];	// frame rate
		gboolean	repeat[TTLMOVIESCREENS];	// repeat play?
		gboolean	cumulative;					// request is cumulative?
	} movie;
	
	struct {
		struct {
			gboolean	active;					// ON | OFF
		} xAxis;
		gboolean	PDF;						// PDF Display
	} spmain;
	
	struct {
		struct {
			gboolean	active;					// ON | OFF
		} xAxis;
		gboolean	PDF;						// PDF Display
		struct {
			int	scrollPct;						// Zoom Scroll %
			int	scope;							// ZOOMALL | ZOOMSINGLE
		} zoom;
	} spreq;
	
	struct {	
		gboolean	statistics[TTLPDFSCREENS][TTLPDFSTATS];	// which PDF statistics to plot
		int			dateFormat;					// PDF Plot Date Format
		int			fdUnits;					// freq domain plot units - FDUNIT_PER | FDUNIT_FREQ
	} common;
	
	struct {
		GdkGC	*ctxts[TOTALPDFCONTEXTS][TTLGCTYPES];
		GdkColor sbC[TTLPDFSCREENS];	// sidebar colors
		GdkColor ctrlBar;				// PDFMOVIE control bar color
	} drawing;
	
	struct {
		gboolean	startup;		// TRUE if pqlx should start in PDF system
		char		*server;
		char		*database;
		struct {
			int	format;				// print output format
		} print;
	} general;
} PDFSETTINGS;

typedef struct _PDFSTATES
{
	struct {
		int			plotW[MMAXPDFROWS][MMAXPDFCOLS][TTLDIMS];
		graph		gr[NUMPDFMAINDISPLAYMODES][MMAXPDFROWS][MMAXPDFCOLS];
		GdkPixmap	*pmap[NUMPDFMAINDISPLAYMODES][MMAXPDFROWS][MMAXPDFCOLS];
		PDFREQ		pdfReq[NUMPDFMAINDISPLAYMODES][MMAXPDFROWS][MMAXPDFCOLS];
		float		perB[NUMPDFMAINDISPLAYMODES][MMAXPDFROWS][MMAXPDFCOLS][TTLPERBOUNDS];
		struct {
			GSList	*list;								// main list of all channels
			GSList	*chnList;							// list of actual channels to display
			CHN_FLT_STATE chnFilter;					// channel filter limiting our main list
			int		pdfKey;								// System PDF to display for group
			int		pdfType;							// PDF Type: SYSTEM, BASELINE
		} group;			
	} main;

	struct {
		gboolean	subPDF[DMAXPDFROWS][DMAXPDFCOLS];	// TRUE= a sub-PDF is currently defined
		int			plotW[DMAXPDFROWS][DMAXPDFCOLS][TTLDIMS];	// drawing dimensions
		graph		gr[DMAXPDFROWS][DMAXPDFCOLS];		// graphing systems
		float		perB[DMAXPDFROWS][DMAXPDFCOLS][TTLPERBOUNDS];	// frequency boundary definitions
		GdkPoint	bbox[ALLCELLS][TTLPOINTS];			// start and stop drag positions
		gboolean	mouseDown;
		PDFREQ		*pdfReq[ALLCELLS];					// PDF request details
		PDFREQ		subReq;								// PDF sub-request specifications
		PSDTRCD		psdTrc;								// sub-PDF Select Trace list
		int			traceCnt[TOTALTRCCNTS];				// how many traces (index=(TRACECNT|VALIDTRC))
		PSDHRD		psdHr;								// sub-PDF Select start times
		GList		*disp, *dispList;					// current detail on display and total list
		GSList		*trcList;							// traces to PQL
		int			idleCnt;							// keep track of outstanding sub-select requests
	} detail;

	struct {
		struct {
			int			chnKey;
			int			row, col;
			int			reqType;							// REQRANGE, REQFREQ, REQAGO
		} req;
		int			plotW[RMAXPDFROWS][RMAXPDFCOLS][TTLDIMS];
		graph		gr[RMAXPDFROWS][RMAXPDFCOLS];
		float		perB[RMAXPDFROWS][RMAXPDFCOLS][TTLPERBOUNDS];
		PDFREQ		pdfReq[RMAXPDFROWS][RMAXPDFCOLS];
	} request;

	struct {
		int			screen;								// screen of current request
		int			plotW[TTLMOVIESCREENS][TTLDIMS];	// drawing dimensions
		GHashTable	*films[TTLMOVIESCREENS];			// hash table holding each DA movie, data = PDFFILM
		int			dbReq[TTLMOVIESCREENS];				// counter of frames to and from DB requested
		int			resize[TTLMOVIESCREENS];			// resize in progress?
		int			state[TTLMOVIESCREENS];				// which direction movie is playing
		int			count[TTLMOVIESCREENS][TTLPDFFRAMECOUNTERS];	// count of movie frames
	} movie;
	
	struct {
		int			row, col;
		SPECREQ		specReq[SMAXROWS][SSPECCOLS];		// the spectrogram data
		graph		specGR[SMAXROWS][SSPECCOLS];		// the gr of spectrogram
		PDFREQ		pdfReq[SMAXROWS][SPDFCOLS];			// the equivalent PDF
		graph		pdfGR[SMAXROWS][SPDFCOLS];			// the gr of PDF
		float		perB[SMAXROWS][SMAXCOLS][TTLPERBOUNDS];
		struct {
			GSList	*list;								// main list of all channels
			GSList	*chnList;							// list of actual channels to display
			CHN_FLT_STATE chnFilter;					// channel filter limiting our main list
			int		pdfKey;								// System PDF to display for group
			int		pdfType;							// PDF Type: SYSTEM, BASELINE
		} group;			
	} spmain;

	struct {
		int			row, col;
		SPECREQ		specReq[SMAXROWS][SSPECCOLS];		// the spectrogram data
		graph		specGR[SMAXROWS][SSPECCOLS];		// the gr of data ON DISPLAY
		PDFREQ		pdfReq[SMAXROWS][SPDFCOLS];			// the equivalent PDF
		graph		pdfGR[SMAXROWS][SPDFCOLS];			// the gr of PDF
		float		perB[SMAXROWS][SMAXCOLS][TTLPERBOUNDS];
		int			cntr;								// counter to decide dest cell
	} spdetreq;

	struct {	// pixmaps for each Drawing Area
		GdkPixmap	*main[NUMPDFMAINDISPLAYMODES][MMAXPDFROWS][MMAXPDFCOLS];
		GdkPixmap	*detail[DMAXPDFROWS][DMAXPDFCOLS];
		GdkPixmap	*request[RMAXPDFROWS][RMAXPDFCOLS];
		GdkPixmap	*movie[TTLMOVIESCREENS];
		GdkPixmap	*spmain[SMAXROWS][SMAXCOLS];
		GdkPixmap	*spreq[SMAXROWS][SMAXCOLS];
		int			state[TTLPDFSCREENS][MMAXPDFROWS][MMAXPDFCOLS];		// MOVIE screen not used here
		int			mState[TTLMOVIESCREENS];
	} pmap;

	int		system;			// PDFNB, SPECTRONB
	int		screen;			// PDFMAIN, PDFDETAIL, PDFREQUEST, PDFSPECTRO, PDFMOVIE
} PDFSTATES;
	
#endif
