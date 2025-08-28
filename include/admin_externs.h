#ifndef _ADMIN_EXTERN_H
#define _ADMIN_EXTERN_H

#include "util_externs.h"

extern GtkWidget	*topWindow;
extern GtkWidget	*statusBar;

// App Widgets
extern GtkWidget	*sideBar[TTLADMINTABS];
extern GtkWidget	*NBtabs[TTLADMINTABS];
extern GtkWidget	*dbInfo[DBTTLDETAILS];
extern GtkWidget	*stats[TTLPDFSTATS];
extern GtkWidget	*DBDefNB, *DBDefTabs[TTLDBINFOTABS];
extern GtkWidget	*buttons[TTLINDBUTS];
extern GtkWidget	*psdConf[PSDCONFTTLDETAILS];
extern GtkWidget	*pngPlot[TTLPNGPLOTS];
extern GtkWidget	*srvrCFGIDFile;
extern GtkWidget	*evtQdisp;
extern GtkWidget	*srvrFrame;
extern GtkWidget	*dbDelete;

// Channels Tab widgets
GtkWidget	*search, *chnStats[TTLCHNSTATS], *spsEwin;

// Data-Model Widgets
extern GtkWidget	*dataVIEW[TTLDATAMODELS];

// Pointer Arrays to Manage Display States
extern GPtrArray	*dispState[TTLDBDISPSTATES][TTLDISPTYPES][TTLDISPDIRS];

// LOG fp
extern FILE	*logFP;

// constant defs
extern char	*admin_tabs[TTLADMINTABS];
extern char	*info_tabs[TTLDBINFOTABS];
extern char	*dd_hdrs[DD_TTLHDRS];
extern char	*ch_hdrs[CH_TTLHDRS];
extern char	*loge_hdrs[LOGE_TTLHDRS];
extern char	*logm_hdrs[LOGM_TTLHDRS];
extern char	*eq_hdrs[EQ_TTLHDRS];
extern char	*spse_hdrs[SPSE_TTLHDRS];
extern char	*psdcfg_hdrs[PSDCFG_TTLHDRS];
extern char *dateFormats[TTLPDFDATEFORMATS];
extern char	*statStr[TTLPDFSTATS];
extern char	*types[TTLDBTYPES];
extern char	*PSDtypes[PSDCFG_TTLTYPES];
extern char	*accessTypes[TTLDBACCESS];
extern char	*onOff[2];
extern char	*logMsgTypes[TTLSRVRLOGTYPES];
extern char	*srvrStatus[TTLSRVREXECSTATES];
extern char	*eventQS[TTLEQSTATI];

extern GdkColor		white, black, ltblue, darkblue, green;

// globals
extern PQLXSTATES	pqlxStates;
extern ADMINDATA	admin;

// routine declares
void make_adminScreens();
void changeTab(GtkNotebook *notebook, GtkWidget *page, guint new_page, gpointer nil);
GtkWidget *makeDataModel(int model);
void addDataCols(int model, GtkTreeView *treeview);
void servers(GtkButton *button, gpointer nothing);
void setDispState(int which);
gboolean dispDBInfo(gpointer nil);
void dbAction(GtkButton *button, gpointer a);
void add_Data(int storeType, int srvrNum);
void clearStore(int storeType);
void freeLogInfo(SRVRLOG *srvrLog);
void freeExecInfo(SRVREXEC *srvrExec);
void freeDBInfo(int type, void *dbData);
void dispMsgDialog(char *msg);
void disconnectDB();
void connectDB(char *srvrNm, char *dbNm);
void DDAction(GtkButton *button, gpointer a);
void PSDAction(GtkButton *button, gpointer a);
gboolean getDataDir(int which);
void dataModelSel(GtkTreeView *treeview, gpointer w);
void freeDataDirInfo(DATADIR *dataDir);
void readPDFsrvrs();
void adminQuit(GtkButton *, gpointer nil);
void serverSelect(GtkButton *, gpointer nil);
gboolean pngDisp_cfg(GtkWidget *widget, GdkEventConfigure *event, gpointer w);
gboolean pngDisp_exp(GtkWidget *widget, GdkEventExpose *event, gpointer w);
void refreshSrvr(char *srvrNm);
void logDisplay(GtkTreeView *treeview, gpointer which);
void logSelect(GtkTreeView *treeview, GtkTreePath *arg1, GtkTreeViewColumn *arg2, gpointer nil);
void setPolling(int type, gboolean);
void scrollTV(int store, int pos);
void comboCBack(GtkComboBox *combo, gpointer t);
gboolean initDBConn(SRVRDB *srvrDB);
void setPassword();
void pollQuit();
void setSrvrExecState(int srvrNum);
gboolean chanSearch(GtkEntry *entry, GdkEventKey *event, gpointer nil);
void pgmExec(GtkButton *button, gpointer p);
void spsPopup(GtkTreeView *treeview, GtkTreePath *arg1, GtkTreeViewColumn *arg2, gpointer nil);
void dispInfo(GtkTreeView *treeview, gpointer w);
void getInfo(GtkTreeView *treeview, gpointer w);
void freeList(GSList *list);
void setSrvrCFGDefaults(SRVREXECCFG *srvrCFG);

#endif