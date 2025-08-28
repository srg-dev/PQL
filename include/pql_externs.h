#ifndef	__PQL_EXTERNS_H
#define	__PQL_EXTERNS_H

extern char *progname;

// graphics contexts for each drawing
extern GdkGC	*GXC[3][NUMDATASCREENS];		
extern GdkGC	*OGXC[MAXOVERLAY], *GGXC[2][MAXGAPC];
extern GdkGC	*PGXC[2][MAXOLAPC];
extern GdkGC	*PRINTGC[3];

extern int		inc_margins[8];
extern int		factors[8];
extern int		dsizes[TTLDTYPES];

extern plot_settings settings, ctrl_settings;
extern plot_states states;
extern GSList 		*traces;			// data = traceInfo*
extern MPLX_MSEED	*mseed_opts;		// command-line options to multi-plex mini-seed file reading

extern void	dispMsg(int, char*);
//  font drawing globals
extern GtkWidget	*fontDA;
extern GdkGC	*fontGC;
extern char		fontFont[50];

extern CRITITEM	criteria[TTLPQLSORTCRITERIA],
					AHcriteria[TTLAHSORTCRITERIA],
					SEGYcriteria[TTLSEGYSORTCRITERIA],
					SACcriteria[TTLSACSORTCRITERIA];
extern char	*sortTypeStr[TTLSORTTYPES];
extern HDRITEM	headers[NUM_HDR_COLS];

extern GtkWidget	*progressBar, *statusBar;

void magnify_proc(GtkWidget *da, GdkPixmap *pixmap, int wind_width, int wind_height, gboolean init); 
int transform_proc(GtkWidget *da, GdkPixmap *pixmap, int wind_width, int wind_height, int restart);
void window(graph *gr, double x1, double x2, double y1, double y2, int lx, int ly);
void viewport(graph *gr, int x1, int x2, int y1, int y2);
int user_to_pix_x(graph *gr, double x);
int user_to_pix_y(graph *gr, double y);
int get_data_increment(traceInfo *trc, double secs, int pixwidth);
int	iir(traceInfo *trc, int active_filter);
void	get_maxmin_amp(traceInfo *);
void	get_maxmin(traceInfo *trc, DATAINFO *dataInfo);
void	mag_plot_files(GtkWidget *da, GdkPixmap *pixmap, int wind_width, int wind_height, int win_record); 
void	trans_plot_files(GtkWidget *da, GdkPixmap *pixmap, int wind_width, int wind_height, int win_record);
void	kill_trace(traceInfo*), make_defaults_file(plot_settings *, int), read_defaults_file(plot_settings *, int);
void	read_data();
void	kill_all_traces(int which), filesStartup();
void freeData(traceInfo *trc, int freeStruct);
void freeData2(traceInfo *trc);
void freeFiltData(traceInfo *trc);
void get_max_mins();
GSList *identifyTrace(char *, MPLX_MSEED*);
LOGDATA *makeLogs(char *fileName);
void get_abs_mag_times();
int agc(/*DATAINFO *fromData, */DATAINFO *toData);
int _iir(DATAINFO *fromData, DATAINFO *toData, int active_filter);
int dbl_request(DBLDI *dbldi);
gint insSort(traceInfo *one, traceInfo *two);
gint listSort(traceInfo *trc1, traceInfo *trc2, SORTPARAMS *);
void sortList(GSList **list, int sortCrit);
void sortTraceList();
gint getInstSOH(INSTSOH *one, traceInfo *trc);
gint insertInstSOH(INSTSOH *one, INSTSOH *two);
double RMS(int startPt, int endPt, traceInfo *trcData);
int parseMMLen(char *lenStr);
void sortInit();
void readHdr_1();
void readData_1();
void setDU(int dispScale);

#endif
