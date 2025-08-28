#ifndef	__GTK_PQL_EXTERNS_H
#define	__GTK_PQL_EXTERNS_H

extern GdkWindow	*topLevel;
extern GtkWidget	*topWindow;

// Trace control widgets
extern GtkWidget	*Tnum_files, *TmagInt, *Tlabel[TTLLABELS], *TmaxPix;
extern GtkWidget	*Tgap, *Tolap, *TmarkFN;
extern GtkWidget	*dispMode[TTLPQLDISPLAYMODES], *Tspread, *TshotLen;
extern GtkWidget	*Tcbo[TTLTRCCBOS];

// Magnify control widgets
extern GtkWidget	*MZoomSpeed, *MScrollFact, *Mgap, *Molap, *MpickKEYind, *MpickCOMind;
extern GtkWidget	*MAnchor[TOTMANCHORS], *MpickFN, *MpickTag[MAXPICKS], *MpickKeys, *MpickComment, *Mpick;
extern GtkWidget	*MmagUpd, *Mover[2];	//*Mfilter[2], 
extern GtkWidget	*MZoomBox, *MPickBox;	//, *MZDisp, *MPDisp;
extern GtkWidget	*MTfixed[2], *MTfixedB[2];
extern GtkWidget	*Msnap, *Mmag;
extern GtkWidget	*Mcbo[TTLMAGCBOS], *MTForm[2], *yaxisConv;

// Spectra control widgets
extern GtkWidget	*Samp, *SyMax, *Sdecades, *Sfreq, *SxMax, *SxMin, *SspecUpd;
extern GtkWidget	*Sover[2], *STForm[2];
extern GtkWidget	*Scbo[TTLSPECCBOS];

// Common/Arrayed control widgets
extern GtkWidget	*dispGain[3], *dispGainB[3], *pqlTForms[TTLTFORMSCR];

// Split control widgets
extern GtkWidget	*Hradios[2][3][3]; 			// Hradios[SPLIT2|SPLIT3][TOP|MID|BOT][TRACESCR|MAGNIFYSCR|SPECTRASCR]
extern GtkWidget	*Hsplit[2], *HsplitBox;
extern GtkWidget	*splitBox, *split2, *split3;
extern GtkWidget	*Hpick;
extern GtkWidget	*HTfixedB[2];
extern GtkWidget	*HTfixed[2], *Hover[2];
extern GtkWidget	*Hsnap, *Hmag;
extern GtkWidget	*Hpanes[3];
extern GtkWidget	*Hcbo[TTLHCBOS], *HTForm[2], *HMidFrame;

// General control widgets
extern GtkWidget	*TFuvw[TTLSENSORS];

// Print control widget
extern GtkWidget	*Tprt1, *Tprt2, *Tprt3, *TprtStr, *TprtFnm, *TprtCmd, *TprtColor, *TprtPSize1, *TprtPSize2;
extern GtkWidget	*TprtPRes1, *TprtPRes2;
extern GtkWidget	*TprtFrmt[TTLPRINTFMTS];

// Filter control widgets
extern GtkWidget	*fltrCbos[TOTFILTERCBOS], *fltrAGCTab;

// Statusbar widgets
extern GtkWidget	*statusBarCtrl;

extern GtkWidget	*ctrlWindow, *helpWindow;

// Button bars
extern GtkWidget	*buttons[NUMSCREENS];
extern GtkWidget	*controlButtons, *splitButtons, *split1Buttons, *split2Buttons, *split3Buttons, *helpButtons;

// Control widgets
extern GtkWidget	*controls[NUMCTRLSCRNS];

// Sort widgets
extern GtkWidget	*sortType, *sortDefault, *sortButton;
extern gulong		*signals[MAXSIGNALTYPES];

// Notebook widgets
extern GtkWidget	*data_screens, *control_screens, *help_screens;

// color selectors
extern GtkWidget	*colorsFG[NUMDATASCREENS], *colorsBG[NUMDATASCREENS], *colorsSB[NUMDATASCREENS], 
					*colorsOL[MAXOVERLAYCOLOURS], *colorsGP[2][MAXGAPC], *colorsOP[2][MAXOLAPC];

// drawing areas
extern GtkWidget	*DA[TOTALDA];

//  pixmaps on display
extern GdkPixmap	*DAPixmap[TOTALDA];

// HEADER Screen widgets
extern GtkWidget	*hdrVIEW, *hdrSW;
extern GtkWidget	*hdrToggles[NUM_HDR_COLS];
extern GtkWidget	*HDRdisp[TTLDISPLAYTYPES];

// LOG Screen widgets
extern GtkWidget	*logSW[TTLLOGSW], *logVIEW[TTLLOGTYPES], *logTEXT;
extern GtkWidget	*LOGdispT[TTLLOGTYPES], *LOGdisp[TTLDISPLAYTYPES];

// CURSOR widget
extern GdkCursor	*hourCursor;

extern char *fileTypes[], *unitTypes[], *wscales[], *xscales[], /**dispunits[],*/ 
			*tscales[], *zanchors[], *hdtypes[], *pqlTransforms[], *sensors[], *sortTypes[];

extern int	printRes[MAXRES];
extern int	OV[MAXOVERLAYCOLOURS];

void	plot_files(GtkWidget *, GdkPixmap *, int, int, gboolean);
void	plot_extras(GtkWidget *, GdkPixmap *, int, int, int, int, int);
void	save_defaults(GtkButton *, plot_settings *);
gboolean	pixmap_cfg(GtkWidget *, GdkEventConfigure *, gpointer);
gboolean	pixmap_exp(GtkWidget *, GdkEventExpose *, gpointer);
gboolean idlePixmap(gpointer);
void	change_menu(GtkNotebook *, GtkWidget *, guint, int *);
void	radioB_chg(GtkToggleButton *, gboolean *);
void	newSplit(GtkToggleButton *, gboolean *);
void	hdr_chg(GtkWidget *, int *);
void	raise_page(GtkButton *, int *);
void	raiseControls(GtkButton *, gpointer);
void	help(GtkButton *, gpointer);
void	hideHelp(GtkButton *, void *);
void	make_gc(GtkWidget *, int);
void	makePixmap(int);
void	set_controls();
void	get_settings(GtkButton *, int *);
void	restore_settings(GtkButton*, int*);
gint	nextTrace(GtkButton*, gpointer);
gint	restartTrace(GtkButton*, gpointer);
void	splitCfg(GtkWidget *, int *);
void	reMag(GtkWidget *, gpointer);
void	reDraw(GtkWidget *entry, gpointer);
//void	radioB3_chg(GtkWidget *, int *);
void pqlComboSel(GtkComboBox *combo, gpointer which);
int	fontSIZE(char *, int);
gboolean	grabFocus(GtkWidget *, GdkEvent *, gpointer nil);
gboolean	mouseEvent(GtkWidget *, GdkEventButton *, gpointer);
gboolean	keyEvent(GtkWidget *, GdkEventKey *, gpointer);
gboolean	dragEvent(GtkWidget *, GdkEventMotion *, gpointer);
void	doMouse (GtkWidget *, GdkEventButton *, GdkPixmap *, int);
void doDrag (GtkWidget *, GdkEventMotion *, GdkPixmap *, int);
gboolean	magZoom(GtkWidget *, GdkEventMotion *, int *);
gboolean	selectT(GtkButton *, gpointer cmd);
gboolean	mark(GtkButton *, void *);
void	makeSelectionList(gboolean selectAll);
gboolean setFG(GtkWidget *, gpointer);
gboolean setBG(GtkWidget *, gpointer);
gboolean setSB(GtkWidget *, gpointer);
gboolean setOL(GtkWidget *, gpointer);
gboolean setGP(GtkWidget *, gpointer);
gboolean setOP(GtkWidget *, gpointer);
GtkWidget *	makeHdrModel();
void	makeHdrData(int);
void newStart(GtkTreeView *treeview, GtkTreePath *arg1, GtkTreeViewColumn *arg2, gpointer );
void printLabel(GdkPixmap *pixmap, traceInfo *trc, int screen_rec, float trace_depth, int trace_width, int scr);
void make_ctrlScreens();
void make_help();
void sortTraces(GtkWidget *widget, gpointer startup);
void tracePoint(int rec_no, int xPix, pointID *ptr);
void magPoint(int mag_rec_no, double begin, double end, int inc, int m_x, pointID *ptr);
void specPoint(traceInfo *trcData, int s_x, pointID *ptr);
int stringWidth(traceInfo *trcData, float trace_depth, int string);
void xaxis(), yaxis();
void fontDraw(GdkDrawable *pxmp, char *str, int x, int y, int gcType, int scr, int screen_rec, int labelRec);
int get_data_increment();
void get_mag_times(), get_mag_max_mins();
gboolean	idleFilter(void *);
void applyFilter(int);
void setFilter();
void setFiltCbos();
void readFilters();
void fltrSelect(GtkComboBox *combo, gpointer nil);
void setRadios(int type);
gint openTraces(GtkButton *, gpointer);
void writePicks(GtkWidget *, void *);
void snapMag(GtkWidget *, gpointer);
void print(GtkWidget *, gpointer);
gint about(GtkButton *, gpointer system);
gint getFilename(GtkButton *, gpointer);
gboolean scrollEvent (GtkWidget *widget, GdkEventScroll *event, gpointer);
int getMinDisp();
gboolean getUserMaxMin(char *mxmnS, double *mxmn);
void xAxisABS(GdkPixmap *pmap, GdkGC *gxc, graph *grph, double start, int scr, gboolean labelDir);
void printPS(int scr);
void printPNG(int scr);
void plot_dispInfo(int dArea, GdkPixmap *pPixmap, int imageh, int imagew, int format);
void makeDispList(int cmd);
void setDispGain(int fromScr, int toScr);
void	transform_data(traceInfo *trcData, double minFreq, SPECSETTINGS *settings);
void	get_trans_data(traceInfo *trcData);
void	taper(traceInfo *trcData);
void	get_trans_times(traceInfo *trcData, double user_x1, double user_x2, gboolean time, double absStart);
traceInfo *trcCopy(traceInfo *trc, int what);
void setSpecGlobals(double *global, gboolean first, traceInfo *trcData, SPECSETTINGS *settings);
void cfftr( float *x, int n );
void pqlTFormSel(GtkComboBox *combo, gpointer flag);
void tFormDisp(GtkToggleButton *button, gpointer scr);
void makeMAGSelectionList(gboolean selectAll);
gboolean ctrlKeyEvent(GtkWidget *widget, GdkEventKey *event, gpointer);
GtkWidget *makeLOGModel(int which);
void log_chg(GtkWidget *button, gpointer t);
void logSelect(GtkTreeView *treeview, GtkTreePath *arg1, GtkTreeViewColumn *arg2, gpointer l);
void makeLOGData(int dispType, int display);
void checkLogTab();
void makeSortPopup(GtkWidget *widget, void *nothing);
gboolean _makePixmap(gpointer s);

//#ifdef PQL_ONLY
void	dispModeShow(GtkWidget *radio, gpointer nil);
//#endif

extern GdkColor	white;
extern GdkColor	black;

#endif
