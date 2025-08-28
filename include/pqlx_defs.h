#ifndef __PQLX_DEFS_H
#define __PQLX_DEFS_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "pqlx_defs_head.h"

#define MINIMUM_PDB_VERSION 3
#define MINIMUM_DB_VERSION	34
#define PQLX_CLNT_DBVER_MIN 33
#define PQLX_CLNT_DBVER_MAX 34

#define XML_VERSION_MIN		4

#define XML_CHNMETA_VER		4
#define XML_EVENT_VER		1
#define ONEMIL				(1000000.0)

// environment variable names
#define ENVXML		"PQLXXML"
#define ENVBIN		"PQLXBIN"
#define ENVTRASH	"PQLXTRASH"
#define ENVLOG		"PQLXLOG"
#define ENVDBDEF	"PQLXDBDEF"

enum {		// the tabs of the PSD sub-system (all notebooks, all tabs)
	PDFMAIN,
	PDFDETAIL,
	PDFREQUEST,
	PDFMOVIE,
	TTLPDFTABS
};

enum {
	SPECTROMAIN=TTLPDFTABS,
	SPECTRODETREQ,
	TTLPDFSCREENS
};

#define SPECTROTAB(a)	(a-SPECTROMAIN)
#define TTLSPECTROTABS	(TTLPDFSCREENS-TTLPDFTABS)

enum {		// the tabs of the STN display sub-system
	STNMAIN,
	STNANALYSIS,
	TTLSTNSCREENS,
	STNSPECTRA=TTLSTNSCREENS,
	STNPSD
};

enum {
	STNICON,
	PDFPLOT,
	SPECTROPLOT,
	WWWPLOT
};

enum {
	PMAPVISIBLE,
	PMAPBASE,
	TTLPMAPTYPES
};

enum {
	NOTRACES,
	READHDRS,
	READDATA,
	DATAPLOT,
	DATADONE
};

enum {		// our known data types
	SEGY_DATA,	// these must be as ordered
	AH_DATA,	// or the global array dataTypes
	SAC_DATA,	// must also be changed
	MSEED_DATA,	// see db_globals.c
	DR100_DATA,
	NANO_DATA,
	RT130_DATA,
	RT125_DATA,
	RT125A_DATA,
	MSEED_MPLEX_DATA,
	CWB_DATA,
	TOTALDATATYPE
};

enum {
	PQL_UNITS_VOLTS,
	PQL_UNITS_COUNTS,
	PQL_UNITS_DISP,
	PQL_UNITS_VEL,
	PQL_UNITS_ACC,
	PQL_UNITS_UNKNWN,
	PQL_UNITS_TTL
};

#if 0
//old version
enum {
	MYVOLTS,
	MYCOUNTS,
	TTLDISPUNITS
};
#endif

enum {
	NTW,
	STN,
	LOC,
	CHN,
	NUMIDS
};

#define	MSEEDNLOC	"  "
#define RESPNLOC	"??"
#define PQLXNLOC	"--"
#define UNKNOWN		"XX"

enum {
	TRC1DATA,
	PSD1DATA,
	TRCINFO,
	ANAINFO,
	NUMDATAS
};

enum {		// PSDCFG.type column values
	BBAND_CONT,
	STMO_TRIG,
	TTLPSDCONFIGTYPES
};

enum {		// pdfReq.keys.pdfKey, when not a system PDF
	CUSTOMPDF = -1,
	SUBPDF = -2,
	HOURSPDF = -3
};
	
enum {		// the types of PDF Requests known to the system
	REQRANGE,
	REQFREQ,
	REQAGO,
	REQRANGE2,			// hour specifiers bound to start and stop days
	NUMREQTYPES
};

#define NOPSDDATA	-1
#define NOPSDBADSR	-2

enum {
	NOCHNPDF,
	BUSY,
	NODATA,
	NOSRVR,
	SRVRCONNECT,
	NOMOVIEPDF,
	BUSY_MOVIE,
	RESIZE_MOVIE,
	BAD_SR,
	NOSPECTRO,
	NOSPECTROPDF
};

enum {
	PQLSYSTEM,
	PDFSYSTEM,
	STNSYSTEM,
	NUM_SYSTEMS
};

enum {
	TRACEMODE,
	GATHERMODE,
	TTLPQLDISPLAYMODES
};

enum {
	ALLPOPUPS,
	INFOPOPUP,
	SOHPOPUP
};

enum {
	MAINGR,
	LOCALGR,
	TTLGR
};

enum {
	STNDATABUSYSENS,
	STNNOFILTERSENS,
	STNNOEVENTSENS,
	STNNOEVENTSENS1,
	TTLSTNSENSARRAYS
};

enum {
	PDFDATABUSYSENS,
	TTLPDFSENSARRAYS
};

enum {
	SORTDIRECT,
	SORTUSER
};
#define SORTSTARTTIME	8		/* must match array instance number of criteria[] (globals.c) */
#define SORTINSTID		7

enum {
	RELATIVE,
	ABSOLUTE,
	TTLTIMESCALES
};

enum {	// used in states.spectra.plot.glob array
	SPEC_MINAMP,
	SPEC_MAXAMP,
	SPEC_MINFREQ,
	SPEC_MAXFREQ,
	TTLSPECGLOBALS
};

enum {
	LIN,
	LOG,
};

enum {
	START,
	END
};

#define MAXPLOTS	500

#define MAXFREQ		10000
#define MINFREQ		1
#define MAXPOLES	12

enum {
    BIT32,
    BIT16,
    MY_FLOAT, 
	MY_DOUBLE,
	MY_CHAR,
	TTLDTYPES
};

#define MAIN_MARGIN		25
#define TRC_B_MARGIN	30
#define MAG_T_MARGIN	25
#define MAG_B_MARGIN	30
#define MAG_TFORM_MARGIN 10

enum {
	REPLACEFROM,
	REPLACETO,
	TTLREPLACE
};

enum {
	PRINT_FMT_PS,
	PRINT_FMT_PNG,
	TTLPRINTFMTS
};

enum {		// printer settings (settings.general.printe.file)
	PRINT_FILE,
	PRINT_W_USER,
	PRINT_PRINTER
};

enum {		// label display format (settings.general.label.format)
	LABEL_FILE,
	LABEL_FILE_SHORT,
	LABEL_HEADER,
	LABEL_GATHERMODE,
	TTLLABELS
};

#define	PQLDFAULTSFILE	"/.pqldfaults"

enum {		// which of data display is actually on display (states.currentScreen)
	SPLIT3,
	SPLIT2,
	MAIN,
	PRINT,
	NUMPLOTS
};

// defines of notebook tab values
enum {		// data display screens
	TRACESCR,
	MAGNIFYSCR,
	SPECTRASCR,
	SPLITSCR,
	HEADERSCR,
	LOGSCR,
	NUMSCREENS
};

enum {		// tabs containing t-form pull-down
	MAGTFORM,
	SPECTFORM,
	SPLITTFORM,
	TTLTFORMSCR
};

enum {		// callback args for Analysis widgets
	TFORMORIG,
	TFORMANA,
	TFORMFILT
};

enum {		// transforms known to PQL
	PQLTFORMS,
	PQLDEMEAN,
	PQLDETREND,
	PQLREVPOL,
	PQLDIFFERENTIATE,
	PQLINTEGRATE,
	PQLUVW,
	TTLPQLANALYSES,
	PQLORIG,
	PQLFILTER
};

#define FLTEDITSTR	"Edit..."
#define FLTCUSTSTR	"Custom..."
#define FLTAGCSTR	"AGC"
enum {		// filter combo headers
	FILTTITLE,
	FILTNORMAL,
	FILTCUSTOM,
	FILTEDIT,
	TTLFILTTYPES
};


enum {
	STS2,
	TRILLIUM,
	TTLSENSORS
};

enum {
	FUNCUVW,
	FUNCMAXMIN,
	FUNCFREE
};

enum {
	INITGROUP,
	PREVGROUP,
	NEXTGROUP
};

enum {
	PREVGRP,
	CURGRP,
	NEXTGRP,
	TTLDISPGRPS
};

#define NOFILTERSELECTION	(NONE)

// defines relating to all combo boxes
enum {
	WSCALETRC,
	WSCALEWIND,
	TTLWSCALES
};

enum {
	TRCWSCALE,
	TRCXAXIS,
	TRCDISPU,
	TTLTRCCBOS
};

enum {
	MAGTSCALE,
	MAGWSCALE,
	MAGDISPU,
	MAGZANCHOR,
	MAGPICKING,
	TTLMAGCBOS
};

enum {
	SPECWSCALE,
	SPECDISPU,
	TTLSPECCBOS
};

enum {
	TRACEH,
	MAGNIFYH,
	SPECTRAH,
	TTLHDTYPES
};

enum {
	SPLITTOP,
	SPLITBOT,
	SPLITMID,
	SPLITTDISP,
	SPLITTSCALE,
	SPLITWSCALE,
	TTLHCBOS
};

enum {
	MAGFILTERCBO,
	SPECFILTERCBO,
	SPLTFILTERCBO,
	TOTFILTERCBOS
};

enum {
	LABEL_ABOVE,
	LABEL_BELOW
};

enum {
	DEFAULTSET,
	FILEDEFSET
};

#define NUMDATASCREENS 4		// only TRACE, MAG, SPECTRA, and SPLIT
#define	SAMESCR	15

// defines of drawing areas and drawing area pixmaps
#define TRACEDA TRACESCR
#define MAGNIFYDA MAGNIFYSCR
#define SPECTRADA SPECTRASCR
// split screen drawing area defines, don't change these numbers!
#define SPLITDA3T	3
#define SPLITDA3M	4
#define SPLITDA3B	5
#define SPLITDA2T	6
#define SPLITDA2M	7
#define SPLITDA2B	8
#define TOTALDA	(SPLITDA2B+1)

#define SPLITSCR2	14
#define SPLITSCR3	8
#define SPLITSCR1	9

enum {		// control screens
	TRACECTRL,
	MAGCTRL,
	SPECCTRL,
	SPLITCTRL,
	HEADERCTRL,
	GENCTRL,
//	SORTCTRL,
	OVERLAYCTRL,
	NUMCTRLSCRNS
};
#define HELPCTRL	(NUMCTRLSCRNS+1)

enum {
	PQLSORT,
	AHSORT,
	SEGYSORT,
	SACSORT,
	TTLSORTTYPES
};

enum {		// AH sort fields
	AHINPUT,
	AHSTN,
	AHCHN,
	AHABSTIME,
	AHSLAT,
	AHSLON,
	AHELEV,
	AHELAT,
	AHELON,
	AHDEP,
	AHOT,
	TTLAHSORTCRITERIA
};

enum {		// SAC sort fields
	SACINPUT,
	SACKINST,
	SACKNETWK,
	SACKSTNM,
	SACKKHOLE,
	SACKCMPNM,
	SACNZYEAR,
	SACNZJDAY,
	SACNZHOUR,
	SACNZMIN,
	SACNZSEC,
	SACNZMSEC,
	SACSTLA,
	SACSTLO,
	SACSTEL,
	SACCMPAZ,
	SACCMPINC,
	SACO,
	SACKEVNM,
	SACEVLA,
	SACEVLO,
	SACEVEL,
	SACEVDP,
	SACMAG,
	SACNEVID,
	SACNORID,
	SACDIST,
	SACAZ,
	SACBAZ,
	SACGCARC,
	TTLSACSORTCRITERIA
};

enum {		// SEGY sort fields
	SEGYINPUT,
	SEGYSTN,
	SEGYCHN,
	SEGYSENSOR,
	SEGYINSTNO,
	SEGYCHNNUM,
	SEGYYEAR,
	SEGYJDAY,
	SEGYHOUR,
	SEGYMINUTE,
	SEGYSECOND,
	SEGYMILLISEC,
	SEGYLINESEQ,
	SEGYREELSEQ,
	SEGYEVTNUM,
	SEGYS2RDIST,
	SEGYRECELEV,
	SEGYSRCSURFELEV,
	SEGYSRCDEP,
	SEGYDTELEVREC,
	SEGYDTELEVSOURCE,
	SEGYSRCWATERDEP,
	SEGYRECWATERDEP,
	SEGYSRCLATY,
	SEGYSRCLONX,
	SEGYRECLATY,
	SEGYRECLONX,
	SEGYSRCUPHOLET,
	SEGYRECUPHOLET,
	SEGYTYEAR,
	SEGYTJDAY,
	SEGYTHOUR,
	SEGYTMINUTE,
	SEGYTSECOND,
	SEGYTMILLS,
	TTLSEGYSORTCRITERIA
};

enum {		// PQL sort fields
	INPUT,
	LONGFN,
	SHORTFN,
	NETWORK,
	STATION,
	LOCATION,
	CHANNEL,
	SENSOR,
	ABSSTART,
	FILETYPE,
	TTLPQLSORTCRITERIA	
};

#define MAXTTLSORTCRITERIA	TTLSEGYSORTCRITERIA

#define	BUTTONW		85		// button width in pixels
#define BUTTONH		24		// button height
enum {
	TOP,
	MID,
	BOT,
	NUMSPLITSCRNS
};

#define	NUMPOLES	7
#define NUMZOOMS	4

enum {
	ZOOMABS,
	ZOOMX,
	ZOOMOUT,
	ZOOMIN
};

enum {
	BACKWARDS,
	FORWARDS,
	INITIALIZE,
	PDFGREINIT
};

enum {
	HEIGHT,
	WIDTH,
	TTLDIMS
};

enum {
	FILTERTYPE,
	DISPLAYTYPE,
	MAGSCRTYPE,
	NRADIOS
};

enum {
	FILTERLIST,
	FILTERCUSTOM,
	TTLFILTEREXTRA
};

enum {
	LOWPOLE,
	HIGHPOLE,
	TTLPOLETYPES
};

enum {
	JAN,
	FEB,
	MAR,
	APR,
	MAY,
	JUN,
	JUL,
	AUG,
	SEP,
	OCT,
	NOV,
	DEC,
	TOTALMTHS
};

enum {
	SELECT,
	DESELECT,
	REMOVE
};

enum {
	JUMPL,
	MOVEL,
	MOVER,
	JUMPR
};
#define JUMPFACTOR .75
#define MOVEFACTOR .1

//#define LABELFONT	"Sans bold 9"
//#define AXISFONT	"Sans bold 10"
#define LABELFONT	"Sans 9"
#define AXISFONT	"Sans 9"
#define MSGFONT		"Sans 18"
#define PDFSTATSFONT "Sans 8"

#define PQLLABELXTRA	20
#define PQLINCXTRA		8

enum {
	NORMALGC,
	INVGC,
	ABNORMALGC,
	TTLGCTYPES
};

#define OVERLAY		(ABNORMALGC+1)
#define MAXOVERLAY	MAXPLOTS
#define MAXOVERLAYCOLOURS	30

#define TRACENONE	8
#define	MAGNONE		48
#define SPECNONE		64

#define	TMAXPIXMAPS	(TRACENONE+1)
#define	MMAXPIXMAPS	(MAGNONE+1)
#define	SMAXPIXMAPS	(SPECNONE+1)
#define	HMAXPIXMAPS 6
#define MAXMAXPIXMAPS (SMAXPIXMAPS)

#define	MAXMARKS	9

enum {		// PQL internal data types (defaults.c, sort.c, etc.)
	MYSTRING,
	MYINT,
	MYDOUBLE,
	SORTFILE,
	MYFLOAT,
	MYSHORT,
	MYCOLOR,
	MYSORT,
	MYAMPS,
	MYEPOCHS,
	MYSPS,
	MYFILETYPE,
	MYDIRECTORY,
	MYBOOL,
	MYSTRINGPTR,
	MYINSTSORT,
	MYPTIME,
	MYFLOATSHORT
};

#define ASC			TRUE
#define DESC		FALSE

enum {		// signal types
	SORTSGNL,
	FILTSGNL,
	PICKSGNL,
	DISPSGNL,
	BTNPRSSGNL,
	BTNRLSSGNL,
	MTNNTFYSGNL,
	NTBKSGNL,
	FILTCBOSGNL,
//	MAGKEYPRESSSGNL,
	TRACESCALESGNL,
	WNDWSCALESGNL,
	OVERLAYSGNL,
	MAXSIGNALTYPES
};
enum {		// signals identified per screen
	TRACESIGNAL,
	MAGSIGNAL,
	SPECSIGNAL,
	SPLITSIGNAL,
	MAXSCRNSGNLS
};

#define	MAXGAPC		1
#define	MAXOLAPC	2

enum {
	GC1,		// gap colours
	GC2,
	GC3
};
enum {
	OL1			// overlap colours
};

enum {		// event types used in states.user.event
	ZOOM,
	REPLOT,
	XAXIS,
	FREQCUTOFF,
	SELECTTR,
	MAGBOUNDARY,
	VLINE,
	SCROLL
};

enum {
	ALL,
	DISPLAYED,
	SELECTED,
	TTLDISPLAYTYPES
};

enum {
	ZANCHOR,
	YSCALE,
	HSCALE,
	ALIGNMENT,
	MAXRADIOGROUPS
};

enum {
	DISPS,
	WINDS,
	FIXEDS,
	CHNGROUPS,
	MAXYSCALES
};

enum {		// MAG screen anchor points
	LEFT,
	RIGHT,
	MIDDLE,
	MOUSE,
	TOTMANCHORS
};

enum {
	FNTLFTT,
	FNTRGTB
};

enum {
	ZOOMBOX,
	PICKBOX,
	NUMBOXES
};

enum {		// mouse modifier keys
	CTRL,
	SHIFT,
	ALT,
	DKEY,
	ZKEY,
	XKEY
};

enum {
	PICK0,
	PICK1,
	PICK2,
	PICK3,
	PICK4,
	PICK5,
	PICK6,
	PICK7,
	PICK8,
	PICK9,
	MAXPICKS	// this must be an even number (starting from 0)!
};

enum {
	PICKFTYPE,
	PRINTFTYPE,
	MARKFTYPE,
	MAXFTYPES
};

enum {
	SNAP,
	MAG,
	KEY
};

enum {
	HIGHF,
	LOWF,
};

// HEADER defines
enum 
{
	HDR_LFN,		// long filename
	HDR_SFN,		// short filename
	HDR_SID,		// sensor ID
	HDR_NTW,		// network
	HDR_STN,		// station
	HDR_LOC,		// location ID
	HDR_CHN,		// channel
	HDR_TIME,		// start time
	HDR_LEN,		// length in seconds
	HDR_SPS,		// samples/sec
	HDR_NDP,		// number of samples
	HDR_FMT,		// data format
	HDR_GAIN,		// gain
	HDR_SCALE,		// scale
	HDR_AMPS,		// max/min amplitude
	HDR_GAPS,		// number of gaps
	HDR_OLAPS,		// number of overlaps
	NUM_HDR_COLS	// total number of columns
};

// LOG Header Defines
enum {		// LOG Display Scrolled Windows
	LOGFILESW,
	LOGTRCSW,
	LOGVIEWSW,
	TTLLOGSW
};

enum {
	LOGDISPBY,
	LOGONDISP
};

enum {		// LOG Display Types
	LOGFILE,
	LOGTRC,
	TTLLOGTYPES
};

enum {
	LOGF_HDR_INST,
	LOGF_HDR_START,
	LOGF_HDR_END,
	LOGF_HDR_FILEN,
	NUM_LOGF_HDR_COLS
};

enum {
	LOGT_HDR_INST,
	LOGT_HDR_START,
	LOGT_HDR_END,
	LOGT_HDR_STATION,
	NUM_LOGT_HDR_COLS
};

enum {				// known paper sizes
	LETTER,
	A4,
	PAPERSIZES
};

enum {
	PRINTHEADER,
	PRINTFOOTER
};

enum {
	HIGHRES,
	LOWRES,
	MAXRES
};

enum {	// file types for printing
	USER,
	TEMP,
	BASE
};

enum {	// traceInfo copy options supplied to trcCopy() routine
	TRCSPECTRA,
	TRCPM,
	TTLTRCCOPYOPTIONS
};

#define PMARGIN 10
#define	LTRW	612		// LETTER width and height, in pixels
#define LTRH	792
#define A4W		594		// A4 width and height, in pixels
#define A4H		840

#define OLAPBEG (pInfo->GO.olapNum*2)
#define OLAPEND	(pInfo->GO.olapNum*2+1)
#define GAPBEG (pInfo->GO.gapNum*2)
#define GAPEND	(pInfo->GO.gapNum*2+1)

enum {
	PDFMSG,			// bg=darkGrey, fg=darkBlue
	PDFTEXT,		// bg=white, fg=black
	PDFHLNM,		// fg=lightGrey
	PDFTRCBG,		// fg=black, bg=lightGrey1
	PDFDASHLINE,	// dashed line
	PDFPINKLINE,	// pink double-dashed
	TOTALPDFCONTEXTS
};

enum {
	PDFFMSG,
	PDFFTEXT,
	PDFFTRC,
	WWWFTEXT,
	PDFSTATSTEXT,
	NUMFONTGCP
};

enum{
	STNFLBL=NUMFONTGCP,	// main
	STNFLBLA,		// analysis
	STNFLBLS,		// spectra
	STNFLBLP,		// psd
	STNFLBLPM,		// particle motion
	STNFLBLPMC,		// particle motion, N/E compass
	STNFLBLPMEBA,	// particle motion, Event BA
	NUMFONTGCS
};

enum {
	LABELF,
	AXISF,
	MSGF,
	STNLBLF,
	WWWLABELF1,
	WWWLABELF2,
	WWWLABELF3,
	WWWLABELF4,
	WWWLABELF5,
	WWWLABELF6,
	PDFSTATSF,
	NUMFONTTYPES
};

enum {						// defines for utility function stringWidth()
	LABEL,
	INC	
};

enum {
	POINT1,
	POINT2,
	TTLPOINTS
};

enum {
	GLAPBEG,
	GLAPEND
};

enum {
	ROW,
	COL
};

enum {
	DTLBACK,
	DTLFORW
};

enum {
	PQLTRC,
	PQLMAG,
	PQLSPLT2,
	PQLSPLT3,
	TTLPQLDESTS
};

enum {
	E,
	N,
	Z,
	TTLCOMPONENTS
};

enum {
	ENZ,
	ONE23,
	ONE2Z,
	TTLCOMPGROUPS
};

enum {
	GDATA_SYSPDFS,
	GDATA_BLINES
};

enum {		// identifier of Spectrogram dB Range, relative or fixed
	DBRELATIVE,
	DBFIXED,
	TTLDBSETTINGS
};

enum {
	SMALLLINE,
	LARGELINE,
	TTLLINESIZES
};
#define LEG_LINE_START	10
#define LEG_LINE_LEN	23

#define	WWWPDFW 550
#define WWWPDFH 425
#define	WWWPDFICOW 170
#define WWWPDFICOH 145
	
#define MAXNUMBERS	20
#define MMAXPDFROWS 3
#define MMAXPDFCOLS 3
#define DMAXPDFCOLS 2
#define DMAXPDFROWS 2
#define RMAXPDFCOLS 3
#define RMAXPDFROWS 3
#define MVMAXROWS	2
#define MVMAXCOLS	2
#define SPDFCOLS 1	
#define SSPECCOLS 1
#define SMAXCOLS (SPDFCOLS+SSPECCOLS)
#define SMAXROWS 3

// lifeguard defines
#define MISSING_CONFIG_VALUE -1
#define LIC_UNLIMITED	0

// time defines
#define ONEMNOFSECS		(60)
#define ONEHROFSECS		(ONEMNOFSECS*60)
#define ONEDAYOFSECS	(ONEHROFSECS*24)
#define BLANK	"nil"

#define STNCOMPFN	"STN Composite"
#define PQLXENDEDMSG "Process ENDED"
#define PQLXPAUSEMSG "Execution PAUSED"
#define PQLXRESUMEMSG "Execution RESUMED"

// logging utility -related defines
enum {	// logging commands, params to logging()
	LOG_OUTPUT,		// set writing to real output destinations
	LOG_FLUSH,		// guarantee that all messages have been handled
	LOG_SHUTDOWN,	// shutdown the logging operations
	TTLLOGCMDS
};

enum {	// logging commands, params to loggingRegister()
	LOG_WRITEFUNC,	// list of write functions
	LOG_ERRORFUNC,	// list of error msg processing functions
//	LOG_CLOSEFUNC,
	TTLLOGFUNCS
};

enum {	// server logging messages, param to logMsg()
	LOGMSG_PROG_STARTUP,
	LOGMSG_PROG_QUIT,
	LOGMSG_CONN_ATTEMPT,
	LOGMSG_CONN_SUCCESS,
	TTLSRVRLOGMSGS
};

#define FILTERCUSTOMLISTNUM (settings.general.filter.numFilters-1)

// terribly useful macros
#define strReplace(a, b, c, d) {char **strs = g_strsplit(a, c, -1); b = g_strjoinv(d, strs); g_strfreev(strs);}
#define g_date_copy(dt)	g_date_new_dmy(g_date_get_day(dt), g_date_get_month(dt), g_date_get_year(dt))
#define IFFREE(str)	{if (str) free(str), str = NULL;}
#define IFFREEGDATE(dt)	{if (dt) g_date_free(dt);}
#define IFFREEHT(ht)	{if (ht) g_hash_table_destroy(ht);}

#define SETHG(h)	{gdk_window_set_cursor(topLevel, h);\
						while (g_main_context_iteration(NULL, FALSE));}
#define FREESTRINGS(sa)	{int i=0;while(sa && (sa[i]!=NULL)) {IFFREE(sa[i]); i++;} IFFREE(sa); sa=NULL;}

// helpers extracting PSD Ref values from a SPECREQ struct
#define PSDREFPLOTS(a)	(((SPECTRODATA*)a->plot.start->data)->psdRef)
#define PSDREFPLOTE(a)	(((SPECTRODATA*)a->plot.end->data)->psdRef)
#define PSDREFABSS(a)	(((SPECTRODATA*)a->data.spectro->data)->psdRef)
#define PSDREFABSE(a)	(((SPECTRODATA*)((g_list_last)(a->data.spectro))->data)->psdRef)


// generic data extractor
// in order to use this, the names of the data pointers 
// must be set to point to the data array, and be exactly named, as:
// gint32	*ptr;	to integer (32-bit) data
// short	*sptr;	to short data
// float	*lptr;	to float data
// double	*dptr;	to double data
#define DATAP(b, c, d) {switch (b) { \
							case BIT32: d=ptr[c];break;		\
							case BIT16: d=sptr[c];break;		\
							case MY_FLOAT: d=lptr[c];break;	\
							case MY_DOUBLE: d=dptr[c];break;	\
							}	\
						}
						
#define DATAPSET(b, c, d) {switch (b) { \
							case BIT32: ptr[c]=d;break;		\
							case BIT16: sptr[c]=d;break;		\
							case MY_FLOAT: lptr[c]=d;break;	\
							case MY_DOUBLE: dptr[c]=d;break;	\
							}	\
						}
#ifndef	PQL_ONLY
//#include "pqlx_types.h"
#endif

#include <math.h>

// in-line macros to convert user coordinates to pixel coordinates and back
#define LOG10(x) ((double) (log(x)*.434294))
#define POW10(x) ((double) (exp(x*2.302585)))
#define USER2PIXX(gr, x)	((rint)(((((gr)->logx?LOG10((double)x):(double)x)-(gr)->user_xmin)/(gr)->user_xdif)*((double) (gr)->pix_xdif)+((double) (gr)->pix_xmin)))
#define PIX2USERX(gr, x) ((gr)->logx?POW10((((double)(x-(gr)->pix_xmin))/((double)(gr)->pix_xdif)*(double)((gr)->user_xdif)+(gr)->user_xmin)):(((double)(x-(gr)->pix_xmin))/((double)(gr)->pix_xdif)*(double)((gr)->user_xdif)+(gr)->user_xmin))
#define USER2PIXY(gr, y) ((int)(((double)(gr)->pix_ydif)-((double)(((gr)->logy?LOG10(y):y)-(gr)->user_ymin)/(gr)->user_ydif)*((double)(gr)->pix_ydif)+((double)(gr)->pix_ymin)))
#define PIX2USERY(gr, y) ((gr)->logy?POW10(((1.-(((double)y-(gr)->pix_ymin)/((double)(gr)->pix_ydif)))*(gr)->user_ydif+(gr)->user_ymin)):((1.-(((double)y-(gr)->pix_ymin)/((double)(gr)->pix_ydif)))*(gr)->user_ydif+(gr)->user_ymin))

#define CHOMP(str)	for(i=0;i<sizeof(str);i++) if (str[i]==' ') {str[i]=0;break;}

#define TOUPPER(a)	{ int i; for( i = 0; a[i]; i++) a[i] = toupper(a[i]);}

#endif
