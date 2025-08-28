#ifndef __DBLINC_H
#define __DBLINC_H

int create_dbl_g(int dbfile);
int get_all_records_g(int dbfile, DBLRECORDS *dblrecs);
int add_record_g(int dbfile, void *record);
int remove_record_g(int dbfile, const char *key);
int get_record_g(int dbfile, void *record);

int dblg_defaults(DBLDI *dbldi);
int dblg_filter(DBLDI *dbldi);
#ifndef PQL_ONLY
int dblg_pdf(DBLDI *dbldi);
int dblg_pdfsrvr(DBLDI *dbldi);
int dblg_STN(DBLDI *dbldi);
#endif

#define	DFAULT_DB	"defaults.db"
#define	FILTER_DB	"filter.db"

#ifndef PQL_ONLY
#define PDF_PDF		"pdf.systemPDF.db"
#define	PDF_DB		"pdf.dfaults.db"
#define PDFSRVR_DB	"pdfsrvr.db"
#define STN_DB 		"stn.dfaults.db"
#define STN_CHFF_DB	"stn.chnfilts.db"
#define STN_CHNY_DB "stn.chnYscale.db"
#define STN_GRPY_DB "stn.grpYscale.db"

#define DBL_SEPARATOR	"%%%"
#endif

#endif
