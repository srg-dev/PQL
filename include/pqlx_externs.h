#ifndef	__PQLX_EXTERNS_H
#define	__PQLX_EXTERNS_H

#include "pqlx_pdf.h"
#include "gtk_pdf_externs.h"
#include "pqlx_stn.h"
#include "gtk_stn_externs.h"

#define XLOG	TRUE
#define XLIN	FALSE
#define YLOG	TRUE
#define YLIN	FALSE

extern GdkWindow	*topLevel;
extern GtkWidget	*topWindow;
extern GtkWidget	*topBox;
extern GtkWidget	*systems[NUM_SYSTEMS];
extern GtkWidget	*sysCbo[NUM_SYSTEMS];
extern gulong	sysCboSgnl[NUM_SYSTEMS];

// pointer arrays for context-sensitive widgets
extern GPtrArray	*stnSensitives[TTLSTNSENSARRAYS][TTLSTNSCREENS];
extern GPtrArray	*pdfSensitives[TTLPDFSENSARRAYS][TTLPDFSCREENS];

extern int	factors[8];
extern char	*systemNames[];
extern char	*months[];
extern GSList	*ntw, *ntwPDF;
extern GHashTable	*chnHash;
extern GdkColor		darkGrey, lightGrey, darkBlue, myWhite, myBlack, blue, red, lightGrey1, pink;
extern plot_settings settings;
extern plot_states states;
extern GdkGC	*GXC[3][NUMDATASCREENS];
extern PDFSTATES	pdfStates;
extern PDFSETTINGS	pdfSettings;
extern PQLXGDATA	pqlxGData;
extern PQLXSTATES	pqlxStates;
extern STNSTATES	stnStates;
extern STNSETTINGS	stnSettings;
extern GdkCursor	*hourCursor;
extern gboolean	debugOUTPUT;
extern char		*statStr[TTLPDFSTATS];
extern gint		reqID;

void switchScreenPQL(int screen);
void setupPQL(SOURCEINFO *);
void systemChg(GtkComboBox *combo, SOURCEINFO *sysInfo);
void window(graph *gr, double x1, double x2, double y1, double y2, int lx, int ly);
void viewport(graph *gr, int x1, int x2, int y1, int y2);
void kill_all_traces(int which);
void servers(GtkButton *button, gpointer nothing);
gint about(GtkButton *, gpointer system);
void initPDFsystem();
void initSTNsystem();
void PDFinitDetail(PDFREQ *pdfPtr, gboolean insert);
gboolean isPDFchn(char *channel);
void FNameRplc(char **strFrom);
void makeSTNDrawing(int scr);
void changeSTNTab(GtkNotebook *notebook, GtkWidget *page, guint new_page, void *nothing);
void dispMsgDialogue(char *msg);
void readSTNchnFilters(int system);
void trace_excerpt(traceInfo *from, traceInfo *to);
void srvrDefault(GtkWidget *, gpointer);
void readPDFsrvrs();
gboolean initDBConn(SRVRDB *srvrDB);
gboolean FKeyEvent(GtkWidget *widget, GdkEventKey *event, gpointer nil);
void bugReport(GtkButton *button, gpointer nil);
void changeTabSTN(GtkNotebook *notebook, GtkWidget *page, guint new_page, void *nothing);
void changeTabPDF(GtkNotebook *notebook, GtkWidget *page, guint new_page, void *nothing);
void plotPDF(PLOTPDFARGS *);
void plotSPECTRO(PLOTPDFARGS *plotPDFArgs);

// CMN library calls
void make_stationDialogue();
void setSystemStartup(int which);
void getSystemStartup();
void pdf_STATS(PDFREQ *pdfReq);
void setSensitive(int system, int array, int screen, gboolean snstv);
void setVisible(int system, int array, int scr, gboolean snstv);
int printPSHeadFoot2(char *fileName, int HF, int pWidth, int pHeight);
int print_ps_image2(GdkPixbuf *pixbuf);
void execAdmin(GtkButton *button, gpointer nil);
gboolean dirty(int system);
void printArr();

#include "util_externs.h"

#endif
