#include "pql_defines.h"
#include "pql_externs.h"

char		*progname;
char    *compNm[TTLCOMPGROUPS][TTLCOMPONENTS];

// graphics contexts for each drawing
GdkGC	*GXC[3][NUMDATASCREENS];		// base gc for each: GXC[NORMALGC|INVGC|ABNORMAL][TR|MAG|SPEC]
GdkGC	*OGXC[MAXOVERLAY];
GdkGC	*GGXC[2][MAXGAPC];				// GAP gc for each: [TRACE|MAG][GAP#]
GdkGC	*PGXC[2][MAXOLAPC];				// OVERLAP gc
GdkGC	*PRINTGC[3];					// white background for print

int		inc_margins[8] = { 50, 40, 35, 25, 20, 18, 20, 18};
int		factors[8] = { 0, 2, 2, 0, 1, 1, 1, 1};
int		dsizes[TTLDTYPES];
char	*zanchors[TOTMANCHORS] = {"Left", "Right", "Middle", "Mouse"};

plot_settings settings, ctrl_settings;
plot_states	states;
GSList 		*traces;			// data = traceInfo*
MPLX_MSEED	*mseed_opts;		// command-line options to multi-plex mini-seed file reading
void	dispMsg(int, char*);

CRITITEM	criteria[TTLPQLSORTCRITERIA] =
{
	{"Input Order", 		TRUE,	FALSE,	0,	MYINT},			// cannot set offset field here,
	{"Long File Name", 		FALSE,	FALSE,	0,	MYSTRINGPTR},	// see set_defaults()
	{"Short File Name", 	FALSE,	FALSE,	0,	MYSTRINGPTR},
	{"Network Name", 		FALSE,	FALSE,	0,	MYSTRING},
	{"Station Name", 		FALSE,	FALSE,	0,	MYSTRING},
	{"Location Identifier", FALSE,	FALSE,	0,	MYSTRING},
	{"Channel Name", 		FALSE,	FALSE,	0,	MYSTRING},
	{"Sensor ID", 			FALSE,	FALSE,	0,	MYSTRING},
	{"Absolute Start Time", FALSE,	FALSE,	0, 	MYDOUBLE},
	{"File Type", 			FALSE,	FALSE,	0,	MYINT}
};
CRITITEM	AHcriteria[TTLAHSORTCRITERIA] =
{
	{"Input Order", 		TRUE,	FALSE,	0,	MYINT},			// cannot set offset field here,
	{"Station", 			FALSE,	FALSE,	0,	MYSTRING},		// see set_defaults()
	{"Channel", 			FALSE,	FALSE,	0,	MYSTRING},
	{"Absolute Start Time", FALSE,	FALSE,	0, 	MYPTIME},
	{"Station LAT", 		FALSE,	FALSE,	0,	MYFLOAT},
	{"Station LON", 		FALSE,	FALSE,	0,	MYFLOAT},
	{"Station ELEV", 		FALSE,	FALSE,	0,	MYFLOAT},
	{"Event LAT", 			FALSE,	FALSE,	0,	MYFLOAT},
	{"Event LON", 			FALSE,	FALSE,	0,	MYFLOAT},
	{"Event DEPTH", 		FALSE,	FALSE,	0,	MYFLOAT},
	{"Event Origin Time", 	FALSE,	FALSE,	0,	MYPTIME}
};
CRITITEM	SEGYcriteria[TTLSEGYSORTCRITERIA] =
{
	{"Input Order", 		TRUE,	FALSE,	0,	MYINT},			// cannot set offset field here,
	{"Station Name", 		FALSE,	FALSE,	0,	MYSTRING},		// see set_defaults()
	{"Channel Name", 		FALSE,	FALSE,	0,	MYSTRING},
	{"Sensor", 				FALSE,	FALSE,	0,	MYSTRING},
	{"Instrument Number", 	FALSE,	FALSE,	0,	MYSHORT},
	{"Channel Number", 		FALSE,	FALSE,	0,	MYINT},
	{"Year",		 		FALSE,	FALSE,	0,	MYSHORT},
	{"Julian Day",			FALSE,	FALSE,	0,	MYSHORT},
	{"Hour",		 		FALSE,	FALSE,	0,	MYSHORT},
	{"Minute",		 		FALSE,	FALSE,	0,	MYSHORT},
	{"Second",		 		FALSE,	FALSE,	0,	MYSHORT},
	{"Millisecond",	 		FALSE,	FALSE,	0,	MYSHORT},
	{"Line Sequence", 		FALSE,	FALSE,	0,	MYINT},
	{"Reel Sequence", 		FALSE,	FALSE,	0,	MYINT},
	{"Event Number", 		FALSE,	FALSE,	0,	MYINT},
	{"SRC 2 REC Distance", 	FALSE,	FALSE,	0,	MYINT},
	{"REC Elevation",		FALSE,	FALSE,	0,	MYINT},
	{"SRC Surface Elevation", FALSE,	FALSE,	0,	MYINT},
	{"SRC Depth",		 	FALSE,	FALSE,	0,	MYINT},
	{"SRC Elevation Datum",	FALSE,	FALSE,	0,	MYINT},
	{"REC Elevation Datum",	FALSE,	FALSE,	0,	MYINT},
	{"SRC Water Depth", 	FALSE,	FALSE,	0,	MYINT},
	{"REC Water Depth", 	FALSE,	FALSE,	0,	MYINT},
	{"SRC LAT",			 	FALSE,	FALSE,	0,	MYINT},
	{"SRC LON",			 	FALSE,	FALSE,	0,	MYINT},
	{"REC LAT",			 	FALSE,	FALSE,	0,	MYINT},
	{"REC LON",			 	FALSE,	FALSE,	0,	MYINT},
	{"SRC Uphole Time",	 	FALSE,	FALSE,	0,	MYINT},
	{"REC Uphole Time",	 	FALSE,	FALSE,	0,	MYINT},
	{"Trigger Year", 		FALSE,	FALSE,	0,	MYSHORT},
	{"Trigger Julian Day",	FALSE,	FALSE,	0,	MYSHORT},
	{"Trigger Hour", 		FALSE,	FALSE,	0,	MYSHORT},
	{"Trigger Minute", 		FALSE,	FALSE,	0,	MYSHORT},
	{"Trigger Second",		FALSE,	FALSE,	0,	MYSHORT},
	{"Trigger Milliseconds",FALSE,	FALSE,	0,	MYSHORT}
};
CRITITEM	SACcriteria[TTLSACSORTCRITERIA] =
{
	{"Input Order", 		TRUE,	FALSE,	0,	MYINT},			// cannot set offset field here,
	{"Instrument Name",		FALSE,	FALSE,	0,	MYSTRING},		// see set_defaults()
	{"Network Name",		FALSE,	FALSE,	0,	MYSTRING},
	{"Station Name",		FALSE,	FALSE,	0,	MYSTRING},
	{"Location Name", 		FALSE,	FALSE,	0,	MYSTRING},
	{"Channel Name", 		FALSE,	FALSE,	0,	MYSTRING},
	{"Year",		 		FALSE,	FALSE,	0,	MYINT},
	{"Julian Day",	 		FALSE,	FALSE,	0,	MYINT},
	{"Hour",		 		FALSE,	FALSE,	0,	MYINT},
	{"Minute",		 		FALSE,	FALSE,	0,	MYINT},
	{"Second",		 		FALSE,	FALSE,	0,	MYINT},
	{"Millisecond",	 		FALSE,	FALSE,	0,	MYINT},
	{"Station LAT",		 	FALSE,	FALSE,	0,	MYFLOAT},
	{"Station LON",		 	FALSE,	FALSE,	0,	MYFLOAT},
	{"Station Elevation",	FALSE,	FALSE,	0,	MYFLOAT},
	{"Channel Azimuth", 	FALSE,	FALSE,	0,	MYFLOAT},
	{"Channel Incident Angle", 	FALSE,	FALSE,	0,	MYFLOAT},
	{"Event Origin Time",	FALSE,	FALSE,	0,	MYFLOAT},
	{"Event Name",			FALSE,	FALSE,	0, 	MYSTRING},
	{"Event LAT",		 	FALSE,	FALSE,	0,	MYFLOAT},
	{"Event LON",		 	FALSE,	FALSE,	0,	MYFLOAT},
	{"Event Elevation",		FALSE,	FALSE,	0,	MYFLOAT},
	{"Event Depth",		 	FALSE,	FALSE,	0,	MYFLOAT},
	{"Event Magnitude",	 	FALSE,	FALSE,	0,	MYFLOAT},
	{"Event ID",	 		FALSE,	FALSE,	0,	MYINT},
	{"Event Origin ID", 	FALSE,	FALSE,	0,	MYINT},
	{"Event Distance", 		FALSE,	FALSE,	0,	MYFLOAT},
	{"Event Azimuth", 		FALSE,	FALSE,	0,	MYFLOAT},
	{"Event Back-Azimuth", 	FALSE,	FALSE,	0,	MYFLOAT},
	{"Event Great Circle Arc", 	FALSE,	FALSE,	0,	MYFLOAT}
};

HDRITEM	headers[NUM_HDR_COLS] =
{
	{"Directory",	0,	MYDIRECTORY},		// cannot set offset field here, see set_defaults()
	{"Filename",	0, 	MYSTRINGPTR},
	{"Sensor ID", 	0,	MYSTRING},
	{"Network", 	0,	MYSTRING},
	{"Station", 	0,	MYSTRING},
	{"Location", 	0,	MYSTRING},
	{"Channel", 	0,	MYSTRING},
	{"Start Time", 	0, 	MYEPOCHS},
	{"Length", 		0, 	MYDOUBLE},
	{"SPS", 		0, 	MYSPS},
	{"Samples", 	0, 	MYINT},
	{"File Type", 	0, 	MYFILETYPE},
	{"Gain", 		0, 	MYSHORT},
	{"Scale", 		0, 	MYFLOAT},
	{"Amplitudes", 	0, 	MYAMPS},
	{"Gaps", 		0, 	MYINT},
	{"Overlaps", 	0,	MYINT}
};
char	*sortTypeStr[TTLSORTTYPES] = {"PQL", "AH", "SEGY", "SAC"};

GtkWidget	*progressBar, *statusBar;
