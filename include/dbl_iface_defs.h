#ifndef	__DBL_IFACE_DEFS_H
#define	__DBL_IFACE_DEFS_H

enum{	// DBLDI.DATAL_INFO.dataEvent types
	READ_ALL,
	WRITE_ALL,
	READ_SNGL,
	WRITE_SNGL,
	DELETE_SNGL
};

// table/data request combinations (DATA_INFO.tableData), 
// links DATA_INFO.data to the proper database table
// as well as the information to be written/extracted
enum {
	DEFAULTST,		// PQL defaults
	FILTERST,		// PQL filter definitions
#ifndef PQL_ONLY
	PDFST,			// user-defined PDF's - not yet implemented
	PDFSRVRST,		// PDF Database Servers
	PDFDFAULTST,	// PDF defaults
	STNCHNFLTRST,	// STN Channel Filter list
	STNDFAULTST,	// STN defaults
	STNCHNYSCALE,	// STN Channel Group Scaling
	STNGRPYSCALE,	// STN YScale Group Definitions
#endif
	NLTABLES
};

#define DBL_NO_RECORD "NO_RECORD"
#define FNAMELEN 60
#define STNCHNFLEN 20

#endif
