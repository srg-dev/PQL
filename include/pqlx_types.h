#ifndef __PQLX_TYPES_H
#define __PQLX_TYPES_H

#include <glib.h>

typedef struct _NSLCDATA
{
	char	name[10];
	char	fullname[50];
	char	alias[20];
	int		id;
} NSLCDATA;

typedef struct _CHN_FILTER
{
	int			numFilters;			// number of user-defined channel filters
	char		**filts;
} CHN_FILTER;

typedef struct _CHN_FLT_STATE
{
	char	*filt;					// channel filter en force
	int		active;
} CHN_FLT_STATE;

typedef struct _SRVRDB
{
	char *name[2];
} SRVRDB;

typedef struct _DBRPLCSTR
{	// structure holding file pathname replacement strings, per db
	char	*fromStr;
	char	*toStr;
} DBRPLCSTR;

typedef struct _PQLX_STATES
{
	int startupSys;					// which system to invoke at startup
	int	system;						// current system on display
	int pqlDest;					// where traces should end up

	struct {
		SERVERDBS	*server;		// current selected server		
		DBLST		*dbInfo;		// current selected DB
		GSList		*srvrDB;		// list of displayed servers, data = SERVERDBS*
	} servers;

	struct {
		gboolean	state;			// SRVRNOCONNECT, SRVRCONNECTING, SRVRCONNECTED
		char		*srvrNm, *dbNm;	// server and dbName of current connection
		gboolean	editPermit;		// Edit DB info?
	} conn;

#if 0
functionality suspended
search for function FNameRplc to re-instate in all pqlx systems source directories
	struct {
		struct {
			DBRPLCSTR	*replace;	// replacement string for file pathname
			GHashTable	*rplc;		// hash table of replacement strings, key = dbName
		} fname;
	} user;
#endif
	
	struct {
		int			numCPUs;
		gboolean	myWordOrder;
	} machine;

} PQLXSTATES;

typedef struct _PQLXGDATA
{
	GSList	*systemPDFs;		// data = SYSPDFDEF
	GSList	*systemBLINEs;		// data = BLINEDEF
} PQLXGDATA;

typedef struct _LOGFILES
{
	char *fileName;
	FILE *fp;
} LOGFILES;

#endif
