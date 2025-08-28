#ifndef	__GTK_PDF_EXTERNS_H
#define	__GTK_PDF_EXTERNS_H

extern GtkWidget	*PSDnb;						// notebook holding all PSD notebooks
extern GtkWidget	*PSDbooks[TTLPSDNB];		// notebooks of PSD Viewer tabs
extern GtkWidget	*PSDCTRLnb;					// notebook holding all PSD Control notebooks
extern GtkWidget	*PSDCTRLbooks[TTLPSDNB];	// notebooks of PSD Control tabs: PDF, SPECTRO

// Dialogues
extern GtkWidget	*reqWndw, *movieWndw;
extern GtkWidget	*pdfCtrlWindow;		// controls dialogue

// Drawing areas
extern GtkWidget	*MPDFDA[MMAXPDFROWS][MMAXPDFCOLS];
extern GtkWidget	*DPDFDA[DMAXPDFROWS][DMAXPDFCOLS];
extern GtkWidget	*RPDFDA[RMAXPDFROWS][RMAXPDFCOLS];
extern GtkWidget	*MVPDFDA[TTLMOVIESCREENS];
extern GtkWidget	*SPMAINDA[SMAXROWS][SMAXCOLS];
extern GtkWidget	*SPREQDA[SMAXROWS][SMAXCOLS];

// PDFMAIN Screen defines - see gtk_pdf_globals.c for descriptions
extern GtkWidget 	*Mrowlbl[NUMIDS][MMAXPDFROWS],
					*Mcollbl[MMAXPDFCOLS];
extern GtkWidget	*pMdisp[NUMPDFMAINDISPLAYMODES];
extern GtkWidget	*md1NSL, *md1CHNG, *md1PDF1, *md1PDF2, *md1PDF3;
extern GtkWidget	*md2PDF, *md2NSL1, *md2NSL2, *md2NSL3, *md2CHNG;
extern GtkWidget	*md3CHN1, *md3CHN2, *md3CHN3, *md3PDF1, *md3PDF2, *md3PDF3;
extern GtkWidget	*md4PDF, *chnCntLbl;
extern GtkWidget	*statusBarPDF;

// PDFDETAIL Screen widgets
extern GtkWidget	*dHrMode, *dTrc;
extern GtkWidget	*pdfStart, *CWBServer, *CWBPort;
extern GtkWidget	*pdfDtailCnt;
extern GtkWidget	*pdfTrcW[MAXTRCEXTRACTS];

// PDFREQUEST Screen Widgets
extern GtkWidget	*rdCHN;
extern GtkWidget	*reqBy[NUMREQTYPES], *rYR[2], *rDY[2][2], *rYRF,
					*rHR[2][2], *rDYA[2], *rFREQ[2], *rFREQF, *rAGOF, *rMDF;

// PDFMOVIE Screen Widgets
extern GtkWidget	*rdCHNmv;
extern GtkWidget	*mDY[PDFREQBOUNDS][PDFREQDAYS], *mHR[PDFREQBOUNDS][PDFREQHOURS], *mINT;
extern GtkWidget	*mCmd[TTLMOVIESCREENS][TTLPDFMCMDBUTTONS], *ctrlBarEB[TTLMOVIESCREENS];
extern GtkWidget	*mvDisp[TTLMOVIEDISPLAYS];	//, *mvDispB[TTLMOVIEDISPLAYS];
extern GtkWidget	*mCUM[2], *mFR[TTLMOVIESCREENS], *mDT[TTLMOVIESCREENS][2];
extern GtkWidget	*movieDisp, *movieDispCell;

// Requestor Dialog containers
extern GtkWidget	*reqDispCell[TTLPDFREQTYPES], *reqMenu[TTLPDFREQTYPES];

// SPECTROMAIN Screen Widgets
extern GtkWidget	*SPpdf, *SPchnFilt, *SPchnCnt;

// SPECTRODETREQ Screen Widgets
extern GtkWidget	*pdfPane[TTLSPECTROTABS];
extern GtkWidget	*pdfDisp[TTLSPECTROTABS][2];

// common widgets to all screens
extern GtkWidget	*axisBnds[TTLPDFSCREENS][TTLPERBOUNDS];
extern GtkWidget	*pdfStats[TTLPDFSCREENS][TTLPDFSTATS];	// which PDF statistics to plot

// Controls Dialogue Widgets
extern GtkWidget	*pdfColors[TTLPDFSCREENS];
extern GtkWidget	*ctrlBar;
extern GtkWidget	*pdfSrvrDefault[2];
extern GtkWidget	*pdfDateFormat;
extern GtkWidget	*fdUnit;
extern GtkWidget	*spZoomPct;
extern GtkWidget	*blineOptions, *blPct;
extern GSList	*PSDblineCtrlR;

// Sidebars of Buttons - used for hiding() and showing()
extern GtkWidget	*PSDsb[TTLPDFSCREENS];
extern GtkWidget	*MButtons[NUMPDFMAINDISPLAYMODES];
extern GtkWidget	*PDFeventB[1];				// event boxes, required for setting bg color
extern GtkWidget	*chnFilterPDF;

// Combo Box data holders and iterators
extern GtkTreeStore *storeNSL, *storeCHN, *storeCHNG, *storePDF;
extern GtkTreeIter	 nslNI, nslSI, nslLI;				// NTW.STN.LOC menu iterators
extern GtkTreeIter	 nslcNI, nslcSI, nslcLI, nslcCI;	// NTW.STN.LOC.CHN menu iterators
extern GtkTreeIter	 chnGI;								// CHN Group iterator
extern GtkTreeIter	 pdf1I, pdf2I;						// PDF types iterator

// Signal management
extern gulong	pdfSignals[MAXPDFSIGNALTYPES];

extern PDFSTATES	pdfStates;
extern PDFSETTINGS	pdfSettings, ctrlPdfSettings;
extern GQueue	*resizeQ[TTLMOVIESCREENS];

// constants
extern const int scrSizes[TTLPDFSCREENS][2];			// number of rows/cols for each screen
extern char		*pdfNBNames[TTLPDFTABS];
extern char		*spectroNBNames[TTLSPECTROTABS];
extern char		*dateFormats[TTLPDFDATEFORMATS];
extern char		*reqDispCells[TTLREQDISPCELLS], *movieDispCells[TTLMOVIEDISPCELLS];
extern char		*spectroDispCells[NUMSPLITSCRNS];
extern char		*zoomScope[TTLZOOMSCOPES];

// function declares
gboolean PDF_cfg(GtkWidget *widget, GdkEventConfigure *event, gpointer);
gboolean PDF_exp(GtkWidget *widget, GdkEventExpose *event, gpointer);
void MDispChg(GtkToggleButton *, gpointer);
void RReqChg(GtkToggleButton *, gpointer);
void changeTabPDF(GtkNotebook *, GtkWidget *, guint, gpointer which);
gboolean mouseEventPDF(GtkWidget *widget, GdkEventButton *event, gpointer);
gboolean dragEventPDF(GtkWidget *widget, GdkEventMotion *event, gpointer);
void cboSelect(GtkComboBox *combo, gpointer);
void makeDrawing(int scr, int row, int col);
void make_pdf_gc(GtkWidget *widget);
gboolean clearAllReqs(GtkButton *button, gpointer screen);
gboolean plotRequest(GtkButton *button, gpointer s);
void make_ReqDialog(gboolean);
void clearReqEntries(GtkButton *button, void *nothing);
void cancelPDFReq(GtkWidget *widget, gpointer dstroy);
void requestPDF(GtkButton *button, void *nothing);
void set_reqEntries(int screen, gpointer req);
gboolean get_reqEntries(int screen, gpointer req);
gboolean	idlePDF(gpointer);
gboolean	idleTMP(IDXTMPARGS *args);
int	yrEnd(int yr);
void servers(GtkButton *button, gpointer nothing);
void freeDetail(gboolean ALL);
void manage_PDF_signals(int);
void DDispMv(GtkToggleButton *button, gpointer);
void pdfReqCopy(PDFREQ **dest, PDFREQ *pdfReq);
void specReqCopy(SPECREQ **dest, SPECREQ *specReq);
void pdfPrint(GtkButton *, gpointer);
void get_chnName(int tab, gpointer req);
void pdfSave(GtkButton *button, void *nothing);
void nextPDFs(GtkButton *button, gpointer);
void initPDFFont();
void make_PDF_stationDialogue(GtkButton *button, gpointer nil);
void freePDF(PDFREQ *pdfReq, gboolean);
void freeSpectro(SPECREQ *specReq);
void pdfChnFltrSelectC(GtkComboBox *combo, gpointer);
void setPDFchnFiltCbos();
void makeChnFltList();
GThreadFunc get_pdf_traces(gpointer);
void radioOption(GtkToggleButton *button, gpointer w);
void write_PDFdefaults(PDFSETTINGS *base);
void read_PDFdefaults(PDFSETTINGS *base);
void getPdfCtrlSettings();
void setPdfCtrlSettings();
void restore_PDFdefaults(GtkButton *widget, gpointer nothing);
void save_PDFdefaults(GtkButton *button, PDFSETTINGS *save);
void raisePdfControls(GtkButton *button, gpointer s);
void freeTrc(void *data, gpointer w);
void dispMsgPDF(int action, char *msg);
gboolean grabFocus(GtkWidget *widget, GdkEvent *event, gpointer nil);
void pdfRadioB3_chg(GtkWidget *radio, gpointer);
void make_MovieDialog(gboolean full);
gboolean movieRequest(GtkButton *button, gpointer);
void moviePDF(GtkButton *button, void *nothing);
void freeFilm(PDFFILM *film);
void pdfMoviePlay(GtkButton *button, gpointer);
void pdfMFrameRate(GtkRange *range, gpointer);
void newMvDisp(GtkToggleButton *button, gboolean *disp);
void pdfMovieSave(GtkButton *button, gpointer);
gboolean resizeMovie(FILMRESIZE *filmR);
void movieCmd(int screen, int cmd, int state);
void clearMovieEntries(GtkButton *button, void *nothing);
char *ChnName(int chni_pk);
void setPeriodBounds(int chnKey, PDFXAXIS *axis);
void make_PeriodScaleDialog(GtkToggleButton *button, gpointer s);
gint getPeriodBounds(XAXISBOUNDSDEF *xAxis, char *channel);
void axisToggle(GtkToggleButton *button, gpointer s);
void write_PDFdefault(int which);
void clearDispList();
void pdfComboSel(GtkComboBox *combo, gpointer w);
void alignSpectro(GtkButton *button, gpointer nil);
void setCursor(int row, int col, gboolean state);
void resetDrawings();
void makeBLINECtrls(int cmd, GSList *blines);
void pdfPrep(int prepType, PDFREQ *pdfReq, PLOTPDFARGS *, int screen, int row, int col);

// spectro calls
gboolean mouseEventSPECTRO(GtkWidget *widget, GdkEventButton *event, gpointer);
gboolean mouseEventSPECTROPDF(GtkWidget *da, GdkEventButton *event, gpointer s);
gboolean dragEventSPECTRO(GtkWidget *da, GdkEventMotion *event, gpointer s);
gboolean scrollEventSPECTRO(GtkWidget *widget, GdkEventScroll *event, gpointer s);
void spectroPDF(gpointer r);
void spectroPrint(GtkButton *button, gpointer);
gboolean _makeDrawing(gpointer r);

#endif
