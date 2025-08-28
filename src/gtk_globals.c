#include "pql_defines.h"

#ifdef PQL_ONLY
// PQL-specific globals
GdkWindow	*topLevel;
GdkCursor	*hourCursor;
#endif

// Trace control widgets
GtkWidget	*Tnum_files, *TmagInt, *Tlabel[TTLLABELS], *TmaxPix;
GtkWidget	*Tgap, *Tolap, *TmarkFN;
GtkWidget	*dispMode[TTLPQLDISPLAYMODES], *Tspread, *TshotLen;
GtkWidget	*Tcbo[TTLTRCCBOS];

// Magnify control widgets
GtkWidget	*MZoomSpeed, *MScrollFact, *Mgap, *Molap, *MpickKEYind, *MpickCOMind;
GtkWidget	*MAnchor[TOTMANCHORS], *MpickFN, *MpickTag[MAXPICKS], *MpickKeys, *MpickComment, *Mpick;
GtkWidget	*Mover[2], *MmagUpd;	//*Mfilter[2], 
GtkWidget	*MZoomBox, *MPickBox;	//, *MZDisp, *MPDisp;
GtkWidget	*MTfixed[2], *MTfixedB[2];
GtkWidget	*Msnap, *Mmag;
GtkWidget	*Mcbo[TTLMAGCBOS], *MTForm[2], *yaxisConv;

// Spectra control widgets
GtkWidget	*Samp, *SyMax, *Sdecades, *Sfreq, *SxMax, *SxMin, *SspecUpd;
GtkWidget	*Sover[2], *STForm[2];
GtkWidget	*Scbo[TTLSPECCBOS];

// Common/Arrayed control widgets
GtkWidget	*dispGain[3], *dispGainB[3], *pqlTForms[TTLTFORMSCR];

// Split control widgets
GtkWidget	*Hradios[2][3][3]; 		// Hradios [SPLIT2|SPLIT3] [TOP|MID|BOT] [TRACESCR|MAGNIFYSCR|SPECTRASCR]
GtkWidget	*Hsplit[2], *HsplitBox;
GtkWidget	*splitBox, *split2, *split3;
GtkWidget	*Hpick;
GtkWidget	*HTfixedB[2];
GtkWidget	*HTfixed[2], *Hover[2];
GtkWidget	*Hsnap, *Hmag;
GtkWidget	*Hpanes[3];
GtkWidget	*Hcbo[TTLHCBOS], *HTForm[2], *HMidFrame;

// General control widgets
GtkWidget	*TFuvw[TTLSENSORS];

// Print control widgets
GtkWidget	*Tprt1, *Tprt2, *Tprt3, *TprtStr, *TprtFnm;
GtkWidget	*TprtCmd, *TprtColor, *TprtPSize1, *TprtPSize2;
GtkWidget	*TprtPRes1, *TprtPRes2;
GtkWidget	*TprtFrmt[TTLPRINTFMTS];

// Filter control widgets
GtkWidget	*fltrCbos[TOTFILTERCBOS], *fltrAGCTab;

// Statusbar widgets
GtkWidget	*statusBarCtrl;

GtkWidget	*topWindow, *ctrlWindow, *helpWindow;

// Button bars
GtkWidget	*buttons[NUMSCREENS];
GtkWidget	*controlButtons, *splitButtons, *split1Buttons, *split2Buttons, *split3Buttons, *helpButtons;

// Control widgets
GtkWidget	*controls[NUMCTRLSCRNS];

// Sort widgets
GtkWidget	*sortType, *sortDefault, *sortButton;

// Signals requiring blocking
//gulong		signals[MAXSIGNALTYPES][MAXSIGNALSCRNS];
gulong		*signals[MAXSIGNALTYPES];

// Notebook widgets
GtkWidget	*data_screens, *control_screens, *help_screens;

// color selectors
GtkWidget	*colorsFG[NUMDATASCREENS], *colorsBG[NUMDATASCREENS], *colorsSB[NUMDATASCREENS], 
			*colorsOL[MAXOVERLAYCOLOURS], *colorsGP[2][MAXGAPC], *colorsOP[2][MAXOLAPC];

// Drawing areas
GtkWidget	*DA[TOTALDA];

//  display pixmaps for each drawing area
GdkPixmap	*DAPixmap[TOTALDA];

// HEADER Screen widgets
GtkWidget	*hdrVIEW, *hdrSW;
GtkWidget	*hdrToggles[NUM_HDR_COLS];
GtkWidget	*HDRdisp[TTLDISPLAYTYPES];

// LOG Screen widgets
GtkWidget	*logSW[TTLLOGSW], *logVIEW[TTLLOGTYPES], *logTEXT;
GtkWidget	*LOGdispT[TTLLOGTYPES], *LOGdisp[TTLDISPLAYTYPES];

// some global values
char *fileTypes[] = {	"SEGY", "AH", "SAC", "Mini-SEED", 
						"DR100", "NANO", "RT130", "RT125", 
						"RT125A", "Mini-SEED"};
char *wscales[] = { "Trace", "Window" };
char *tscales[] = { "On Display", "Entire Trace", "Fixed" };
char *xscales[] = { "Relative", "Absolute" };
//char *dispunits[] = { "Volts", "Counts"};
char *unitTypes[] = {"Volts", "Counts", "Displacement", "Velocity", "Acceleration", "Unknown" };
char *hdtypes[] = { "Trace", "Magnify", "Spectra" };
char *pqlTransforms[TTLPQLANALYSES] = {"Transforms", "Demean", "De-Trend", "Polarity-Rev",  
										"dx/dy", "Integrate", "UVW"};
char *sensors[TTLSENSORS] = { "STS", "TRIL" };
char *sortTypes[TTLSORTTYPES] = {"PQL", "AH", "SEGY", "SAC"};

int		printRes[MAXRES] = {2, 1};
int		OV[MAXOVERLAYCOLOURS] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 
								10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 
								20, 21, 22, 23, 24, 25, 26, 27, 28, 29};

GdkColor	white = { 0, 0xffff, 0xffff, 0xffff };
GdkColor	black = { 0, 0x0, 0x0, 0x0 };
