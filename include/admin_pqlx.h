#ifndef _PQLXADMIN_H
#define _PQLXADMIN_H

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include "pql_version.h"
#include "pqlx_defs_head.h"
#include "db_iface_head.h"
#include "db_iface_admin.h"
#include "pqlx_defs.h"
#include "pqlx_types.h"

// some defines
#define	BUTTONW		85		// button width in pixels
#define BUTTONH		24		// button height
#define ACTIVESTR	"ACTIVE"
#define INACTIVESTR	"INACTIVE"

// some enums
enum {		// tabs
	A_INFO,
	A_CHAN,
	A_LOGS,
	TTLADMINTABS
};

enum {		// A_INFO tabs
	INFO_DESC,
	INFO_DATADIR,
	INFO_PSDCFG,
	INFO_PNG,
	TTLDBINFOTABS
};

enum {		// notebook enums
	NB_MAIN,
	NB_DEF,
	TTLNB
};

enum {		// PQLX server display views
	SRVR1,
	SRVR2,
	TTLSRVRVIEWS
};

enum {		// database information
	DBHOSTNAME,
	DBDBNAME,
	DBDESC,
	DBORG,
	DBTYPE,
	DBACCESS,
	DBDADMIN,
	DBDADMINEM,
	DBTADMIN,
	DBTADMINEM,
	DBPNGDIR,
	DBPNGDF,
	DBPNGHLNM,
	DBPNGNW,
	DBPNGNH,
	DBPNGNB,
	DBPNGNBC,
	DBPNGIW,
	DBPNGIH,
	DBPNGIB,
	DBPNGIBC,
	DBTTLDETAILS
};

enum {		// DB Types
	DBTYPEPROD,
	DBTYPESCRATCH,
	TTLDBTYPES
};

enum {		// DB Access
	DBACCPUB,
	DBACCPRIV,
	TTLDBACCESS
};

enum {		// actions
	DBACTIONCREATE,
	DBACTIONDELETE,
	DBACTIONSAVE,
	DBACTIONCANCEL,
	DBACTIONEDIT,
//	DBACTIONPWORD,
	DBACTIONREPSD,
	DBACTIONNEW,
	DBACTIONPREV,
	DBACTIONNEXT,
	DBACTIONLOGREF,
	DBACTIONCOPY,
	DBACTIONSRVRPAUSE,
	DBACTIONSRVRRESUME,
	TTLDBACTIONS
};

enum {
	PGMEXEC_SRVR,
	PGMEXEC_PQLX,
	TTLPGMEXECS
};

enum {				// display states
	DBCREATE,		// DB Create
	DBEDIT,			// DB Info Edit
	DBNOCONN,		// No DB Connection
	DBDISPLAY,		// DB Info Display
	TTLDBDISPSTATES
};

enum {				// display state types
	DISPSENS,		// sensitivity
	DISPVISIBLE,		// visibility
	DISPEDITABLE,	// editable
	TTLDISPTYPES
};

enum {				// data models
	DATADIRS,		// DATA DIRECTORIES
	CHANNELS,		// CHANNEL Details
	LOGEXEC,		// LOG EXEC Details
	LOGMSGS,		// LOG Messages
	EQEXEC,			// Event Q - executing
	EQQUED,			// Event Q - queued
	EQPEND,			// Event Q - pending
	SPSEPS,			// Sample Rate Epochs popup
	PSDCFG,			// PSD CONFIG 
	TTLDATAMODELS
};

enum {				// server execution states
	SRVREXEC_DONE,
	SRVREXEC_EXEC,
	SRVREXEC_PAUSE,
	SRVREXEC_CRASH,
	TTLSRVREXECSTATES
};

enum {				// independent buttons to manage
	BUTCOPYDB,
	BUTEXECEXEC,
	BUTPAUSEEXEC,
	BUTRESUMEEXEC,
	BUTEXECCFG,
	TTLINDBUTS
};

enum {
	CHN_TTL,
	CHN_STARTD,
	CHN_ENDD,
	TTLCHNSTATS
};
	
enum {		// data and PNG directory actions
	DD_ADD,
	DD_ENABLE,
	DD_DISABLE,
	DD_DELETE,
	PNG_ADD,
	IDFILE_ADD
};

enum {		// DATADIR model headers
	DD_DIR,
	DD_ED,
	DD_LS,
	DD_TT,
	DD_TR,
	DD_TTLHDRS
};

enum {		// CHANNELS model headers
	CH_NAME,
	CH_STARTDT,
	CH_ENDDT,
	CH_TTLTRC,
	CH_TTLPSD,
	CH_TTLSPSEPOCH,
	CH_RESP,
	CH_TTLHDRS
};

enum {		// SPSE model headers
	SPSE_SR,
	SPSE_STARTDT,
	SPSE_ENDDT,
	SPSE_TTLHDRS
};

enum {
	PSDCFGHDR_CHNGRP,
	PSDCFG_TTLHDRS
};

enum {
	PSDCFG_CONT,
	PSDCFG_TRIG,
	PSDCFG_TTLTYPES
};

enum {		// actions to PSDAction()
	PSDCFG_ADD,
	PSDCFG_TTLACTIONS
};

enum {
	PSDCFG_CHNGRP,
	PSDCFG_OVERLAP,
	PSDCFG_MINDB,
	PSDCFG_MAXDB,
	PSDCFG_DURATION,
	PSDCFG_MAXPER,
	PSDCFG_TYPE,
	PSDCONFTTLDETAILS
};

enum {
	LOGA_DISPLAY,
	LOGA_REFRESH,
	TTLLOGACTIONS
};

enum {		// LOGEXEC model headers
	LOGE_NAME,
	LOGE_TIME,
	LOGE_DONETIME,
	LOGE_DURATION,
	LOGE_STATUS,
	LOGE_TTLHDRS
};

enum {		// LOGMSG model headers
	LOGM_TYPE,
	LOGM_TIME,
	LOGM_MSG,
	LOGM_TTLHDRS
};

enum {
	EQ_TOTAL,
	EQ_TYPE,
	EQ_TTLHDRS
};

enum {
	PNGPLOT_PDF,
	PNGPLOT_SPECTRO,
	TTLPNGPLOTS
};

enum {
	NORMPNG,
	ICONPNG,
	TTLPNGTYPES
};

enum {
	PNGSIZE,
	PNGTEST,
	TTLPNGDISP
};

enum {
	FREE_DBINFO,
	FREE_DBLISTING
};

enum {		// parameters to setPolling
	POLL_LOG,
	POLL_EVENTQ,
	TTLPOLLTYPES
};

enum {
	SRVR_CFG_SHOW,
	SRVR_CFG_DESTROY
};

enum {
	FILE_SEL_DD,
	FILE_SEL_PNG,
	FILE_SEL_ID
};

enum {		// combo box specifier, arg to callback comboCBack()
	CBO_LOGMSGTYPE,
	TTLCBOTYPES
};

enum {
	TRSTARTT,
	TRENDT,
	TTLTRTIMES
};

enum {
	TRHOUR,
	TRMIN,
	TTLTRSEGS
};

#define NORMPNGMINW	290
#define NORMPNGMINH 220
#define ICONPNGMINW	170
#define ICONPNGMINH	145
#define ICONPNGMAXW	(THUMBSIZEMAXWPDF-1)
#define ICONPNGMAXH	(184)

typedef struct _TIME24
{	// hour and minute vs. a 24-hour clock
	int	time[TTLTRSEGS];
} TIME24;

typedef struct _SRVREXECCFG
{
	int			numCPU;					// number of CPU's to use when executing server
	gboolean 	dirScan;				// TRUE = directory scanned, FALSE = no directory scanning
	gboolean	refreshSysPNG;			// refresh SYS PDF's
	char		*identFile;				// provide identFile option to server execution
	struct {
		gboolean set;					// TRUE = execution is restricted by bounding hours 
		TIME24	bounds[TTLTRTIMES];		// bounding start and stop times
	} timeRestricted;
} SRVREXECCFG;

typedef struct _ADMINDATA
{
	struct {
		int		main;					// current main tab on display
		int		sub;						// any current sub-tab on display
	} tab;
	int		state;						// display state
	DBDEF	dbInfo;						// PQLX DB information
	int		sel[TTLDATAMODELS];			// selected Data Model Entry
	
	struct {
		int	maxCPU;						// maximum number of CPU's available on this machine
		int	logDisplay;					// display LOG, ERROR, or both log msg types
		SRVREXECCFG srvrCFG;				// server execution configuration settings
	} settings;

	struct {
		struct {
			struct {
				GSList	*list;			// list of PSD Confs when in Edit mode, data = PSDCONF*
				gboolean changed;		// did anything change?
			} psdConf;
		} edit;
		
		struct {
			GSList	*list;				// list of channels resulting from User Search
		} channels;
		
		struct {						// current server log being displayed (lower panel)
			int			srvrNum;
			SRVRLOG		*srvrLog;
			EVENTQLIST	EQList;			// list of current events on the EVENTQ
			gboolean polling[TTLPOLLTYPES];	// poll active?
		} log;
		
		struct {
			int 	which;				// which PNG selected for edit
			GSList	*statLines;			// stats lines, save them here...
			char	*pdfWWWDir;			// user-modified PNG output directory 
		} png;
	} states;
} ADMINDATA;

#endif
