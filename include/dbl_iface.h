#ifndef __DBLF_H
#define __DBLF_H

#include <time.h>
#include <glib.h>
#include "ptime.h"
#include "db_iface_defs.h"
#include "dbl_iface_defs.h"

// helper/subordinate data structures, NONE YET

// data structure holding data, 
// both returned from the DB and written to the DB
typedef struct _DATAL_INFO
{	// meta information for *data element
	int	tableData;	// the db table associated with the data
	int	dataEvent;	// INSERT_DATA | UPDATE_DATA
} DATAL_INFO;

// the main db interface data structure holding all subordinate data structures, 
// db_request() is called with this data structure having been filled accordingly
typedef struct _DBLDI
{	// Local Database Data Interface structure:
	// *data is ptr to in (data read from the db)
	// and out (data written to the db) data structures
	// which are specific to descr.table
	DATAL_INFO	descr;
	void 		*data;
	void		*data2;
} DBLDI;

// the generic data structure for DBL Records
typedef struct _DBLRECORDS
{
	int		numRecs;
	void	**recs;
} DBLRECORDS;

// data structures catering for PQL DEFAULTS values
typedef struct _DEFAULTS_REC
{
	char	name[100];
	char	value[255];
} DEFAULTS_REC;

// data structures catering for user-defined FILTERS
typedef struct _FILTER_REC
{
	char	name[FNAMELEN];
	int		poles[2];
	float	cutoff[2];
} FILTER_REC;

#ifndef PQL_ONLY
// data structures catering for user-defined PDF definitions
typedef struct _PDF_REC
{
	char	name[25];
	float	criteria[MAXNUMPDFREQCRIT];
} PDF_REC;

typedef struct _PDF
{
	int		numRecs;
	PDF_REC	*recs;
} PDF;

typedef struct _PDFSRVR_REC
{
	char	*name;
} PDFSRVR_REC;

// data structures catering for STN system settings
typedef struct _STN_CHN_FILTER
{
	char	*name;
} STN_CHN_FILTER;

typedef struct _STN_CHN_YSCALE
{
	char *name;
	char *loc, *chn;
	char *scale[2];
} STN_CHN_YSCALE;

typedef struct _STN_GRP_YSCALE
{
	char *name;
	GSList	*chns;				// list of channel scales, data = (char *) STN_CHN_YSCALE.name
} STN_GRP_YSCALE;
	
#endif

#endif
