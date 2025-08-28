#ifndef __GTK_STN_EXTERNS_H
#define __GTK_STN_EXTERNS_H

extern GtkWidget	*STNscreens;		// notebook of STN tabs: MAIN
extern GtkWidget	*stnCtrlWindow;
extern GtkWidget	*stnEventWindow;

// Drawing areas
extern GtkWidget	*STNDA[TTLSTNSCREENS];

// Sidebars of Buttons - used for hiding() and showing()
extern GtkWidget	*STNbuttons[TTLSTNSCREENS];			// sidebars in their entirety

// common sidebar Widgets for all Tabs
extern GtkWidget	*stnYfixedB[TTLSTNSCREENS][2], *stnYfixed[TTLSTNSCREENS][2];
//					*stnXScale[TTLSTNSCREENS][MAXALIGNMENTS];
extern GtkWidget *stnXScales;
extern GtkWidget	*stnFltrCbos[TTLSTNSCREENS], *stnFilter[TTLSTNSCREENS][2];
extern GtkWidget	*stnUnits[TTLSTNSCREENS][2], *stnUnitsB[TTLSTNSCREENS];

// STNMAIN Widgets
extern GtkWidget	*chnFilter, *stnDisp[2], *stnProgBar, *stnCntLbl, *yScaleCbo[TOTSTNCHNGROUPS];
extern GtkWidget	*STNeventB[4], *stnShiftB[2];

// STNEVENT Widgets
extern GtkWidget	*stnOrder[TTLDISPORDERS], *stnAlign[MAXALIGNMENTS];
extern GtkWidget	*STNSnap, *STNMag, *STNZoomBox, *stnZAnchors;		//*STNAnchor[TOTMANCHORS], 

// EVENTs Dialog Widgets
extern GtkWidget	*stnEventScreens, *stnEvtUserTime[TTLSTNEVTTIMES], 
					*stnEvtLoc[TTLSTNEVTLOCS], *eventVIEW;

// STN Control Dialogue
extern GtkWidget	*stnColors[TTLSTNSCREENS][STNMAXCOLOURS];
// STNMAIN
extern GtkWidget	*stnSrvrDefault[2], *STNXdays;
extern GtkWidget	*stnStart, *STNyAxis[TTLSTNSCREENS];
extern GtkWidget	*STNlines, *STNdays, *stnControlScreens;
extern GtkWidget	*STNrplc[TTLREPLACE];
extern GtkWidget	*STNpql[TTLPQLDESTS];
extern GtkWidget	*STNunitsR[2], *STNunitsD;

// STNEVENT
extern GtkWidget	*STNevtEpiCtr[STNEVTEPIORDERS];
extern GtkWidget	*STNEvtCtrol;
extern GtkWidget	*STNevtModels;		// widget to list known models
extern GSList		*STNevtModelsR;		// list of radio buttons defining selectable models
extern GSList		*STNevtModelPhases;	// list holding phases per model, data = MODELPHASES
extern GSList		*STNevtAlignPhases;	// list holding phases per model for axis alignment, data = MODELPHASES
extern GtkWidget	*STNuserP[MAXPICKS], *STNuserPA;

// common STNEVENT & STNANALYSIS widgets
extern GtkWidget	*stnOverlay[TTLSTNSCREENS][2];

// STNANALYSIS
extern GtkWidget	*stnTForms, *AnalysesString, *EvtPhInfo;
extern GHashTable	*CG;

// STNSPECTRA
extern GtkWidget	*STNPopup[TTLPOPUPWIN], *STNSpecAmp, *STNSpecYMax, *STNSpecDecades;
extern GtkWidget	*STNSpecFreq, *STNSpecXMax, *STNSpecXMin, *STNColors[TTLPOPUPWIN][STNMAXPOPUPCOLOURS];
extern GtkWidget	*STNPsdYMax, *STNPsdYMin, *STNPsdHL, *STNPsdXMax, *STNPsdXMin;


// STN Stations button widgets
extern GtkWidget	*STNstnList, *ShiftDays, *statusBarSTN;

// Global State & Settings data structs
extern STNSTATES	stnStates;
extern STNSETTINGS	stnSettings, ctrlStnSettings;
extern GAsyncQueue	*drawQ;

// Instances and signals
extern GtkWidget	*stnWidgets[TTLSTNLOOKAHEADS];
extern gulong		stnSignals[TTLSTNLOOKAHEADS];
extern gulong		*stnSignals2[MAXSTNSIGNALTYPES];

// constants
extern char	*STNcmds[STNINIT+1];
extern char	*transforms[TTLSTNANALYSES], *zanchors[];
extern char	*STNxscales[MAXALIGNMENTS], *sensors[];

gboolean STN_cfg(GtkWidget *widget, GdkEventConfigure *event, gpointer scr);
gboolean STN_exp(GtkWidget *widget, GdkEventExpose *event, gpointer scr);
void	makeSTNDrawing(int scr);
void 	getSTNdata(GSList *stns, int *which);
gboolean 	getAllSTNdata();
void 	dispAdvance(GtkButton *button, gpointer dir);
void 	make_stn_gc(GtkWidget *widget);
void make_pdf_gc(GtkWidget *widget);
void 	stnDispChg(GtkToggleButton *button, gpointer disp);
void 	plotSTNtrc(GdkPixmap *pixmap, graph *gr, traceInfo *trc, ANADATA *anaData, 
					double begin, int inc, int recNo, int screen);
void 	stn_mag_plot(GdkPixmap *pixmap, graph *gr, traceInfo *trc, ANADATA *anaData, 
					int inc, int recNo, int screen);
int		getSTNDataInc(double secs, int pixwidth, double sampInt);
int		checkTrace(traceInfo *);
int		readTrace(traceInfo *);
gboolean chkCHN(char *chnNm);
void getSTNpdf();
gboolean dragEventSTN(GtkWidget *widget, GdkEventMotion *event, gpointer scr);
gboolean mouseEventSTN(GtkWidget *widget, GdkEventButton *event, gpointer scr);
void drawSTNData(GdkPixmap *pixmap);
void setChannels(GtkEntry *entry, gpointer nothing);
void clearSTNList(GSList **stnIter);
void pdfLBLplot(int);
void raiseStnControls(GtkButton *button, gpointer *specPSD);
void setStnCtrlSettings();
void getStnCtrlSettings();
void makeSTNGC(int screen, int context, GdkColor *fg, GdkColor *bg, GtkWidget *widget);
GList *stnCnt(GList *stnStart, gboolean dir);
void setDates(int cmd, int days);
void makeSTNList();
void stnChnFltrSelect(GtkEntry *entry, gpointer nothing);
void stnChnFltrSelectC(GtkComboBox *combo, int *screen);
gboolean chkRequest(int type);
void initSTNData();
void setSTNchnFiltCbos();
void read_STNdefaults(STNSETTINGS *base);
gboolean keyEventSTN(GtkWidget *widget, GdkEventKey *event, gpointer scr);
gboolean grabFocusSTN(GtkWidget *widget, GdkEvent *event, gpointer scr);
void strRplc(char *strTo, char *strFrom);
void save_STNdefaults(GtkButton *button, STNSETTINGS *save);
void write_STNdefaults(STNSETTINGS *base);
void restore_STNdefaults(GtkButton *widget, gpointer nothing);
gboolean freeStnChnData(GNode *node, gpointer nothing);
gint sortChannels(CHNDATA *one, CHNDATA *two);
void chnNameMk(CHNDATA *chnData, gpointer nothing);
gboolean _mkSTNDrawing(gpointer scr);
void stnFltrSelect(GtkComboBox *combo, gpointer screen);
void stnApplyFilter(GtkToggleButton *button, gpointer screen);
void setStnFiltCbos();
int iir(traceInfo *trc, int active_filter);
int _iir(DATAINFO *fromData, DATAINFO *dataInfo, int active_filter);
void freeAllSTNData();
void freeTrcData(GSList *trcIter);
gboolean	nextChannel(gpointer stage);
void stnRadio_chg(GtkToggleButton *radio, gpointer group);
gboolean	stnDataInstance(gpointer nothing);
PQLDATA *marryTrcs(GSList *trcsToMarry, double startE, double endE);
PQLDATA *cutTrc(PQLDATA *pqlIn, double startE, double endE);
void get_maxmin_amp(traceInfo *);
void stnRadioB3_chg(GtkWidget *button, int *group);
gboolean getUserMaxMin(char *mxmnS, double *mxmn);
void stnRefresh(GtkWidget *entry, gpointer scr);
void yaxisP();
void maxMin(CHNDATA *chnData);
gint sortTrcs(PQLDATA *pql1, PQLDATA *pql2);
void changeTabSTN(GtkNotebook *notebook, GtkWidget *page, guint new_page, gpointer nil);
void eventSelect(GtkButton *button, void *nothing);
gboolean readDrawQ(gpointer nothing);
void initSTNScreen(int initType);
void drawSTNEvent(GdkPixmap *pixmap);
void drawSTNana(GdkPixmap *pixmap);
void analyzeData(ANADATA *anaData, int tForm);
void stnTFormSel(GtkComboBox *combo, gpointer orig);
int	chnNum(char *ntw, char *stn, char *loc, char *chn);
gboolean mouseEventANA(GtkWidget *widget, GdkEventButton *event, gpointer scr);
gboolean dragEventANA(GtkWidget *widget, GdkEventMotion *event, gpointer scr);
int fft ( double real[], double imag[], int nfft, int direction);
void printSTNLabel(GdkPixmap *pmap, traceInfo *trc, int rec, int scr, gboolean selected, 
					gboolean overlay, ANAPOPUP *specPsd);
void initSTNFont();
gint dispShift(GtkButton *button, gpointer dir);
void dispMsgSTN(int action, char *msg);
gboolean selectSTNT(GtkButton *button, gpointer cmd);
void makeSTNSelectionList(int scr);
void stnYChnGroupSelectC(GtkComboBox *combo, gpointer screen);
void makeChnGrpScaleCbo();
void readChnGrpScales();
gint findGroup(YCHNGROUP *one, char *name);
char *getChnGrpScale(char *chnName, int screen, int which);
void setYScaleCbo(int which, int screen);
char *getChnGrpName(char *chnName, int screen);
gint getChnScale(YCHNSCALE *one, char *name);
gint getAutoScale(AUTOSCALE *one, char *channel);
void ANAsetDatas(GSList *);
void ANAreMag(GtkWidget *widget, gpointer type);
gboolean anaScrollEvent (GtkWidget *widget, GdkEventScroll *event, gpointer nil);
gboolean keyEventANA (GtkWidget *widget, GdkEventKey *event, gpointer nil);
void ANAPoint(PQLDATA *pqlData, int m_x, pointID *ptr);
void STNdispMsg(int action, char *msg);
void xAxisABSSTN(GdkPixmap *pmap, GdkGC *gxc, graph *grph, double start, int scr, gboolean labelDir);
double GDate2Epoch(double gDate);
void cutANA(GtkButton *button, gpointer nil);
char *getResp(ANADATA *anaData);
void stnPrint(GtkButton *button, gpointer screen);
void specPsdDo(int which);
void stn_spectra_plot(ANAPOPUP *specPsd, int win_record, int which);
void get_trans_data_stn(traceInfo *trcData, float *data);
int stnRemResp(traceInfo *trc, char *respFile);
void transform_dataPSD(traceInfo *trcData);
void makeSTNgc(GdkColor *fg, GdkColor *bg, GtkWidget *widget, GdkGC **ctxt);
void calcArrivals(gboolean new);
void clearArrivals(GSList *arrivalsList);
gint getStnArrivals(ARRIVALS *one, char *stn);
void makeModelsCtrls(GSList *models);
void modelChg(GtkWidget *radio, gpointer which);
void magNumPts(traceInfo *trcData, ANADATA *anaData);
gboolean reSetPopup(gpointer which);
void evtPhDisp(GtkWidget *widget, gint x1, gint y1);
void stnComboSel(GtkComboBox *combo, gpointer which);
gboolean ctrlKeyEventANA(GtkWidget *widget, GdkEventKey *event, gpointer s);
void particleMotion();
void specSetting(GtkToggleButton *radio, SET_ARGS *args);
void reDrawPopup(GtkToggleButton *radio, GtkWidget **da);
gboolean STN_popup_cfg(GtkWidget *widget, GdkEventConfigure *event, gpointer w);
gboolean STN_popup_exp(GtkWidget *widget, GdkEventExpose *event, gpointer w);
void destroyPopup(GtkWidget *da, gpointer w);
void closePopup(GtkButton *button, GtkWidget **topW);
void stnPopupPrint(GtkButton *button, ANAPOPUP *specPsd);
void makePOPUP(int which, int type, ANAPOPUP *specPsd);
void freePopupData(int which, ANAPOPUP *specPsd);
void makePopupDrawing(int which, ANAPOPUP *specPsd);
void stn_PM_plot(ANAPOPUP *PM);
GSList *getChnMeta(traceInfo *trc);
CHNMETA *getChnMetaD(traceInfo *trc);
float backAZ(float lat1, float lon1, float lat2, float lon2);
gboolean mouseEventPU(GtkWidget *widget, GdkEventButton *event, gpointer s);
gboolean dragEventPU(GtkWidget *widget, GdkEventMotion *event, gpointer s);
gboolean scrollPopup(GtkWidget *da, GdkEventScroll *event, gpointer nil);
void specPointPU(ANAPOPUP *popupD, int trcNum, int x, pointID *ptID);
SPECPT *spec_point(traceInfo *trcData, int s_x, graph *gr);

#endif
