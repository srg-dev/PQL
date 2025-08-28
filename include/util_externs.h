#ifndef	__UTIL_EXTERNS_H
#define __UTIL_EXTERNS_H

#include <cairo.h>

void	TIMESTR(char *timeStr);
char *	currentT();
void passcal_etoh(struct ptime *t, double epoch);
gint32 passcal_htoe(PTIME *t, gboolean convertJuls);
void passcal_month_day(struct ptime *t, int jul_day);
int get_julday(int month, int day, int year);
int julian(struct ptime *t);
void makeThread(gboolean callType, GThreadFunc toCall, void *args, GThreadFunc toReturn);
int globmatch (char *string, char *pattern);
double getMidnight(double);
int testByteOrder();
int get_ncpu_pqlx();
gboolean beepMe(gpointer nil);

#ifndef PQL_ONLY
// logging functions
gboolean loggingInit(char *procN, char *dbN, char *execTime, gboolean mainLoop, gboolean initCaching);
gboolean logging(int cmd);
LOGFILES *openLogFile(char *prefix, char *suffix, char *dbName, gchar *timeStr);
void getDbRange(GList *start, GList *end, int *dbBounds, int *perRecs);
int	getTotalDays(int *start, int *end);
void colorBar(GdkPixmap *pmap, int w, int h);
void colorBarS(GdkPixmap *pmap, int w, int h, SPECREQ *);
gboolean chkDateFormat(GDateYear year, GDateMonth month, GDateDay day);
#endif

#ifndef CLNT_ADMIN
enum {
	XAXISDAYSPDFHOUR,
	XAXISDAYSSPECTRO,
	TTLXAXISDAYSTYPES
};

typedef struct _XAXISDAYSARGS
{
	int			type;
	union {
		GdkPixmap	*pixmap;
		cairo_surface_t *surface;
	} plot;
	int			width, height;
	GdkGC		*gc;
	graph		*GR;
	int			start[2], end[2];
	double		startE;
} XAXISDAYSARGS;

#ifndef PQL_ONLY
#define COLORBW	15
#define ARROWW	(COLORBW-1)
#define ARROWH	(ARROWW*2+1)
typedef struct _CBARARGS
{
	int		w, h;
	int		dbBounds[TTLYBOUNDS];
	SPECREQ	*specReq;
} CBARARGS;

// helpful macros
// SECSPDIV() = seconds per PSD record of specReq
#define SECSPDIVSP(s)	(ONEDAYOFSECS/(s->data.numPSDday))
// TTLSECS() = total seconds on display of specReq
#define TTLSECSSP(s)	(s->plot.numDivs * SECSPDIVSP(s))

#endif

// declares not relevant for ADMIN client program
int fontSIZEP(char *str, int fontType, int type);
void fontDrawP(GdkDrawable *pxmp, char *str, int x, int y, int fontType, int gcid);
void fontDrawPGC(GdkDrawable *pxmp, char *str, int x, int y, int fType, GdkGC *gcl);
void fontDrawPRev(GdkDrawable *pxmp, char *str, int x, int y, int fontType, int gcid, int ogcid);
void fontDrawPR(GdkDrawable *pxmp, char *str, int x, int y, int fType, int gcid);
void initFontP();
void makeFONTGC(int which, GdkColor *fg, GdkColor *bg);
void FNameRplc(char **strFrom);
void freeMyData(int type, void *data);
void yaxisP(GdkPixmap *pixmp, GdkGC *grc, graph *grph, 
			double x0, double y0, double yn, 
			int tside, int nside, int ndivs, 
			double offset, int format, int font);
void xaxisP(GdkPixmap *pixmp, GdkGC *grc, graph *grph, 
			double x0, double y0, double xn, 
			int tside, int nside, int ndivs, double offset, 
			gboolean inv, double epoch, int font);
void xAxisDAYS(XAXISDAYSARGS *);
void xAxisTIME(XAXISDAYSARGS *);
void error_dialog(char *title, char *msg);

#ifndef PQL_ONLY
void xAxisDAYSCairo(XAXISDAYSARGS *axisArgs);
void plotPDFCairo(cairo_surface_t *surface, PDFREQ *pdfReq, int width, int height, int scr, int type, gboolean plotHLNM);
void plotSPECTROCairo(cairo_surface_t *surface, SPECREQ *specReq, int width, int height, int scr, int type);
void fontDrawPCairo(cairo_t *cr, char *str, int x, int y, int fType, gboolean rotate);
int fontSIZEPCairo(cairo_t *cr, char *str, int fType, int type);
cairo_surface_t *colorBarPDF(CBARARGS *);
cairo_surface_t *colorBarSPEC(CBARARGS *);
void xaxisPCairo(cairo_surface_t *surface, graph *grph, 
			double x0, double y0, double xn, 
			int tside, int nside, int ndivs, double offset, 
			gboolean inv, double epoch, int font);
void yaxisPCairo(cairo_surface_t *surface, graph *grph, 
			double x0, double y0, double yn, 
			int tside, int nside, int ndivs, 
			double offset, int format, int font);
void stnHLNM(GdkPixmap *pixmap, graph *grL, GdkColor *lineC);
void xAxisTIMECairo(XAXISDAYSARGS *axisArgs);
gint32 spectroEStart(SPECREQ *specReq);
#endif

void polarityReverse(DATAINFO *dataInfo);
void detrend(DATAINFO *dataInfo);
void dxdy(DATAINFO *dataInfo);
void integrate(DATAINFO *dataInfo);
void demean(DATAINFO *dataInfo, float mean);
gboolean xyzUVW(GHashTable *comp, int sensor);
void deconvolve(DATAINFO *dataInfo, traceInfo *trc, char *respFile);
#endif

extern GdkColor		darkGrey, lightGrey, lightGrey2, darkBlue, myWhite, myBlack, blue, red, lightGrey1, pink;
extern char	*compNm[TTLCOMPGROUPS][TTLCOMPONENTS];

#endif
