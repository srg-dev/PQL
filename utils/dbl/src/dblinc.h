#ifndef __DBLINC_H
#define __DBLINC_H

int create_dbl(int dbfile);
int get_all_records(int dbfile, DBLRECORDS *dblrecs);
int add_record(int dbfile, void *record);
int remove_record(int dbfile, const char *key);
int dbl_defaults(DBLDI *dbldi);
int dbl_filter(DBLDI *dbldi);
int get_record(int dbfile, void *record);

#ifndef PQL_ONLY
int dbl_pdf(DBLDI *dbldi);
int dbl_pdfsrvr(DBLDI *dbldi);
int dbl_STN(DBLDI *dbldi);
#endif

#define	DFAULT_DB	"defaults.db"
#define	FILTER_DB	"filter.db"

#ifndef PQL_ONLY
#define	PDF_DB		"pdf.dfaults.db"
#define PDFSRVR_DB	"pdfsrvr.db"
#define STN_DB 		"stn.dfaults.db"
#define STN_CHFF_DB	"stn.chnfilts.db"
#define STN_CHNY_DB "stn.chnYscale.db"
#define STN_GRPY_DB "stn.grpYscale.db"

#define DBL_SEPARATOR	"%%%"
#endif

#endif
