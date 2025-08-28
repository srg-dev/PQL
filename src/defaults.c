#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#if defined LINUX || defined SOLARIS 
#include <values.h>
#endif
#include <sys/stat.h>

#include "pql_defines.h"
#include "pql_externs.h"
#include "gtk_pql_externs.h"
#include "dbl_iface.h"

#define	offsetS(a) ((int) ((long) &a - (long) &settings))
#define location(a) ((long) base + (int) a)

#define SORTTYPESEP	"|"
#define SORTCRITSEP	"$"
#define SORTDIRSEP	";"
#define SORTTYPENAMESEP	":"
enum {
	SORTENCRYPT,
	SORTDECRYPT
};

typedef struct _DFAULTS {
	char	desc[20];
	int		offset;
	int		type;
} DFAULTS;

static DFAULTS fileDefaults[] =
{
	{"FFLT", 0, MYSTRINGPTR},
	{"FMINC", 0, MYSTRINGPTR},
	{"FMEXC", 0, MYSTRINGPTR},
	{"FMLEN", 0, MYSTRINGPTR},
	{"FREPL", 0, MYBOOL},
	{"FDIR", 0, MYSTRINGPTR},
	{"END", 0, -1}
};
	
static DFAULTS defaults[] = 
{
	{"WSW", 0, MYINT},
	{"WSH", 0, MYINT},
	{"DSPMD", 0, MYINT},
	{"PP1", 0, MYINT},
	{"PP2", 0, MYINT},
	{"PP3", 0, MYINT},
	{"TN", 0, MYINT},
	{"TT", 0, MYINT},
	{"TA", 0, MYINT},
	{"TGF", 0, MYINT},
	{"TOF", 0, MYINT},
	{"TGS", 0, MYFLOAT},
	{"TDG", 0, MYFLOAT},
	{"TSL", 0, MYINT},
	{"MT", 0, MYINT},
	{"MW", 0, MYINT},
	{"MP", 0, MYINT},
	{"MPF", 0, MYSTRINGPTR},
	{"MPK", 0, MYINT},
	{"MPC", 0, MYINT},
	{"MZF", 0, MYINT},
	{"MSF", 0, MYINT},
	{"YUC", 0, MYBOOL},
	{"MP0", 0, MYSTRING},
	{"MP1", 0, MYSTRING},
	{"MP2", 0, MYSTRING},
	{"MP3", 0, MYSTRING},
	{"MP4", 0, MYSTRING},
	{"MP5", 0, MYSTRING},
	{"MP6", 0, MYSTRING},
	{"MP7", 0, MYSTRING},
	{"MP8", 0, MYSTRING},
	{"MP9", 0, MYSTRING},
	{"MA", 0, MYINT},
	{"MZB", 0, MYBOOL},
	{"MPB", 0, MYBOOL},
	{"MGF", 0, MYINT},
	{"MOF", 0, MYINT},
	{"MDG", 0, MYFLOAT},
	{"AGCW", 0, MYINT},
	{"AGCDW", 0, MYINT},
	{"AGCDT", 0, MYBOOL},
	{"AGCTH", 0, MYFLOAT},
	{"TFS", 0, MYINT},
	{"SA", 0, MYINT},
	{"SXS", 0, MYINT},
	{"SYS", 0, MYINT},
	{"SD", 0, MYFLOAT},
	{"SyMax", 0, MYFLOAT},
	{"SxMax", 0, MYFLOAT},
	{"SxMin", 0, MYFLOAT},
	{"H2T", 0, MYINT},
	{"H2B", 0, MYINT},
	{"H3T", 0, MYINT},
	{"H3M", 0, MYINT},
	{"H3B", 0, MYINT},
	{"HSP", 0, MYINT},
	{"HTD", 0, MYINT},
	{"PFMT", 0, MYINT},
	{"PFN", 0, MYSTRINGPTR},
	{"PC", 0, MYSTRINGPTR},
	{"PPS", 0, MYINT},
	{"PR", 0, MYINT},
	{"LT", 0, MYINT},
	{"HDR1", 0, MYBOOL},
	{"HDR2", 0, MYBOOL},
	{"HDR3", 0, MYBOOL},
	{"HDR4", 0, MYBOOL},
	{"HDR5", 0, MYBOOL},
	{"HDR6", 0, MYBOOL},
	{"HDR7", 0, MYBOOL},
	{"HDR8", 0, MYBOOL},
	{"HDR9", 0, MYBOOL},
	{"HDR10", 0, MYBOOL},
	{"HDR11", 0, MYBOOL},
	{"HDR12", 0, MYBOOL},
	{"HDR13", 0, MYBOOL},
	{"HDR14", 0, MYBOOL},
	{"HDR15", 0, MYBOOL},
	{"HDR16", 0, MYBOOL},
	{"HDR17", 0, MYBOOL},
#if 0
	{"SRT1",  0, MYSORT},
	{"SRT2",  0, MYSORT},
	{"SRT3",  0, MYSORT},
	{"SRT4",  0, MYSORT},
	{"SRT5",  0, MYSORT},
	{"SRT6",  0, MYSORT},
	{"SRT7",  0, MYSORT},
	{"SRT8",  0, MYSORT},
	{"SRT9",  0, MYSORT},
	{"SRT10",  0, MYSORT},
#endif
	{"SRTDEF", 0, MYINT},
	{"SRTSTR", 0, MYSTRINGPTR},
	{"TFG", 0, MYCOLOR},
	{"TBG", 0, MYCOLOR},
	{"TSB", 0, MYCOLOR},
	{"TG1", 0, MYCOLOR},
	{"TO1", 0, MYCOLOR},
	{"MFG", 0, MYCOLOR},
	{"MBG", 0, MYCOLOR},
	{"MSB", 0, MYCOLOR},
	{"MG1", 0, MYCOLOR},
	{"MO1", 0, MYCOLOR},
	{"SFG", 0, MYCOLOR},
	{"SBG", 0, MYCOLOR},
	{"SSB", 0, MYCOLOR},
	{"SPSB", 0, MYCOLOR},
	{"OV1", 0, MYCOLOR},
	{"OV2", 0, MYCOLOR},
	{"OV3", 0, MYCOLOR},
	{"OV4", 0, MYCOLOR},
	{"OV5", 0, MYCOLOR},
	{"OV6", 0, MYCOLOR},
	{"OV7", 0, MYCOLOR},
	{"OV8", 0, MYCOLOR},
	{"OV9", 0, MYCOLOR},
	{"OV10", 0, MYCOLOR},
	{"OV11", 0, MYCOLOR},
	{"OV12", 0, MYCOLOR},
	{"OV13", 0, MYCOLOR},
	{"OV14", 0, MYCOLOR},
	{"OV15", 0, MYCOLOR},
	{"OV16", 0, MYCOLOR},
	{"OV17", 0, MYCOLOR},
	{"OV18", 0, MYCOLOR},
	{"OV19", 0, MYCOLOR},
	{"OV20", 0, MYCOLOR},
	{"OV21", 0, MYCOLOR},
	{"OV22", 0, MYCOLOR},
	{"OV23", 0, MYCOLOR},
	{"OV24", 0, MYCOLOR},
	{"OV25", 0, MYCOLOR},
	{"OV26", 0, MYCOLOR},
	{"OV27", 0, MYCOLOR},
	{"OV28", 0, MYCOLOR},
	{"OV29", 0, MYCOLOR},
	{"OV30", 0, MYCOLOR},
	{"END", 0, -1}
};

int 
env_error(var_str) 
     char *var_str;
{
  
  fprintf(stderr,"ENV var %s not set.\n", var_str);
  exit(-1);
}

void set_defaults()
{
	traceInfo	trace;
	GHashTable	*dH;
	int			i, cnt;

	cnt=sizeof(defaults)/sizeof(DFAULTS);
	dH = g_hash_table_new_full(g_str_hash, g_str_equal, (GDestroyNotify)free, (GDestroyNotify)free);
	g_hash_table_insert(dH, g_strdup("WSW"), g_strdup_printf("%d", offsetS(settings.general.startUP.winDims[WIDTH])));
	g_hash_table_insert(dH, g_strdup("WSH"), g_strdup_printf("%d", offsetS(settings.general.startUP.winDims[HEIGHT])));
//#ifdef PQL_ONLY
	g_hash_table_insert(dH, g_strdup("DSPMD"), g_strdup_printf("%d", offsetS(settings.general.display.mode)));
//#endif
	g_hash_table_insert(dH, g_strdup("PP1"), g_strdup_printf("%d", offsetS(settings.general.startUP.panePos[0])));
	g_hash_table_insert(dH, g_strdup("PP2"), g_strdup_printf("%d", offsetS(settings.general.startUP.panePos[1])));
	g_hash_table_insert(dH, g_strdup("PP3"), g_strdup_printf("%d", offsetS(settings.general.startUP.panePos[2])));
	g_hash_table_insert(dH, g_strdup("TN"), g_strdup_printf("%d", offsetS(settings.trace.numPlots)));
	g_hash_table_insert(dH, g_strdup("TT"), g_strdup_printf("%d", offsetS(settings.trace.time)));
	g_hash_table_insert(dH, g_strdup("TA"), g_strdup_printf("%d", offsetS(settings.trace.amp)));
	g_hash_table_insert(dH, g_strdup("TGF"), g_strdup_printf("%d", offsetS(settings.trace.gFlag)));
	g_hash_table_insert(dH, g_strdup("TOF"), g_strdup_printf("%d", offsetS(settings.trace.oFlag)));
	g_hash_table_insert(dH, g_strdup("TDG"), g_strdup_printf("%d", offsetS(settings.general.display.gather.gain[TRACESCR])));
	g_hash_table_insert(dH, g_strdup("TGS"), g_strdup_printf("%d", offsetS(settings.general.display.gather.spread)));
	g_hash_table_insert(dH, g_strdup("TSL"), g_strdup_printf("%d", offsetS(settings.general.input.shotLength)));
	g_hash_table_insert(dH, g_strdup("MT"), g_strdup_printf("%d", offsetS(settings.magnify.Tamp)));
	g_hash_table_insert(dH, g_strdup("MW"), g_strdup_printf("%d", offsetS(settings.magnify.Wamp)));
	g_hash_table_insert(dH, g_strdup("MP"), g_strdup_printf("%d", offsetS(settings.magnify.pick)));
	g_hash_table_insert(dH, g_strdup("MPF"), g_strdup_printf("%d", offsetS(settings.magnify.pickFile)));
	g_hash_table_insert(dH, g_strdup("MPK"), g_strdup_printf("%d", offsetS(settings.magnify.pickKEYOn)));
	g_hash_table_insert(dH, g_strdup("MPC"), g_strdup_printf("%d", offsetS(settings.magnify.pickCOMOn)));
	g_hash_table_insert(dH, g_strdup("MZF"), g_strdup_printf("%d", offsetS(settings.magnify.zoomFactor)));
	g_hash_table_insert(dH, g_strdup("MSF"), g_strdup_printf("%d", offsetS(settings.magnify.scrollFactor)));
	g_hash_table_insert(dH, g_strdup("YUC"), g_strdup_printf("%d", offsetS(settings.general.unitsConv)));
	g_hash_table_insert(dH, g_strdup("MP0"), g_strdup_printf("%d", offsetS(settings.magnify.pickTag[0])));
	g_hash_table_insert(dH, g_strdup("MP1"), g_strdup_printf("%d", offsetS(settings.magnify.pickTag[1])));
	g_hash_table_insert(dH, g_strdup("MP2"), g_strdup_printf("%d", offsetS(settings.magnify.pickTag[2])));
	g_hash_table_insert(dH, g_strdup("MP3"), g_strdup_printf("%d", offsetS(settings.magnify.pickTag[3])));
	g_hash_table_insert(dH, g_strdup("MP4"), g_strdup_printf("%d", offsetS(settings.magnify.pickTag[4])));
	g_hash_table_insert(dH, g_strdup("MP5"), g_strdup_printf("%d", offsetS(settings.magnify.pickTag[5])));
	g_hash_table_insert(dH, g_strdup("MP6"), g_strdup_printf("%d", offsetS(settings.magnify.pickTag[6])));
	g_hash_table_insert(dH, g_strdup("MP7"), g_strdup_printf("%d", offsetS(settings.magnify.pickTag[7])));
	g_hash_table_insert(dH, g_strdup("MP8"), g_strdup_printf("%d", offsetS(settings.magnify.pickTag[8])));
	g_hash_table_insert(dH, g_strdup("MP9"), g_strdup_printf("%d", offsetS(settings.magnify.pickTag[9])));
	g_hash_table_insert(dH, g_strdup("MA"), g_strdup_printf("%d", offsetS(settings.magnify.anchor)));
	g_hash_table_insert(dH, g_strdup("MZB"), g_strdup_printf("%d", offsetS(settings.magnify.dispBox[ZOOMBOX])));
	g_hash_table_insert(dH, g_strdup("MPB"), g_strdup_printf("%d", offsetS(settings.magnify.dispBox[PICKBOX])));
	g_hash_table_insert(dH, g_strdup("MGF"), g_strdup_printf("%d", offsetS(settings.magnify.gFlag)));
	g_hash_table_insert(dH, g_strdup("MOF"), g_strdup_printf("%d", offsetS(settings.magnify.oFlag)));
	g_hash_table_insert(dH, g_strdup("MDG"), g_strdup_printf("%d", offsetS(settings.general.display.gather.gain[MAGNIFYSCR])));
	g_hash_table_insert(dH, g_strdup("TFS"), g_strdup_printf("%d", offsetS(settings.general.tForm.uvwSensor)));
	g_hash_table_insert(dH, g_strdup("AGCW"), g_strdup_printf("%d", offsetS(settings.general.filter.AGC.winLen)));
	g_hash_table_insert(dH, g_strdup("AGCDW"), g_strdup_printf("%d", offsetS(settings.general.filter.AGC.dwin)));
	g_hash_table_insert(dH, g_strdup("AGCDT"), g_strdup_printf("%d", offsetS(settings.general.filter.AGC.detect)));
	g_hash_table_insert(dH, g_strdup("AGCTH"), g_strdup_printf("%d", offsetS(settings.general.filter.AGC.threshold)));
	g_hash_table_insert(dH, g_strdup("SA"), g_strdup_printf("%d", offsetS(settings.spectra.display.amp)));
	g_hash_table_insert(dH, g_strdup("SXS"), g_strdup_printf("%d", offsetS(settings.spectra.display.xAxis)));
	g_hash_table_insert(dH, g_strdup("SYS"), g_strdup_printf("%d", offsetS(settings.spectra.display.yAxis)));
	g_hash_table_insert(dH, g_strdup("SD"), g_strdup_printf("%d", offsetS(settings.spectra.display.decades)));
	g_hash_table_insert(dH, g_strdup("SyMax"), g_strdup_printf("%d", offsetS(settings.spectra.display.yMax)));
	g_hash_table_insert(dH, g_strdup("SxMax"), g_strdup_printf("%d", offsetS(settings.spectra.display.xMax)));
	g_hash_table_insert(dH, g_strdup("SxMin"), g_strdup_printf("%d", offsetS(settings.spectra.display.xMin)));
	g_hash_table_insert(dH, g_strdup("H2T"), g_strdup_printf("%d", offsetS(settings.split.DA[SPLIT2][TOP])));
	g_hash_table_insert(dH, g_strdup("H2B"), g_strdup_printf("%d", offsetS(settings.split.DA[SPLIT2][BOT])));
	g_hash_table_insert(dH, g_strdup("H3T"), g_strdup_printf("%d", offsetS(settings.split.DA[SPLIT3][TOP])));
	g_hash_table_insert(dH, g_strdup("H3M"), g_strdup_printf("%d", offsetS(settings.split.DA[SPLIT3][MID])));
	g_hash_table_insert(dH, g_strdup("H3B"), g_strdup_printf("%d", offsetS(settings.split.DA[SPLIT3][BOT])));
	g_hash_table_insert(dH, g_strdup("HSP"), g_strdup_printf("%d", offsetS(settings.split.pane)));
	g_hash_table_insert(dH, g_strdup("HTD"), g_strdup_printf("%d", offsetS(settings.split.traceDisp)));
	g_hash_table_insert(dH, g_strdup("PFMT"), g_strdup_printf("%d", offsetS(settings.general.print.format)));
	g_hash_table_insert(dH, g_strdup("PFN"), g_strdup_printf("%d", offsetS(settings.general.print.fileN)));
	g_hash_table_insert(dH, g_strdup("PC"), g_strdup_printf("%d", offsetS(settings.general.print.command)));
	g_hash_table_insert(dH, g_strdup("PPS"), g_strdup_printf("%d", offsetS(settings.general.print.paperSize)));
	g_hash_table_insert(dH, g_strdup("PR"), g_strdup_printf("%d", offsetS(settings.general.print.paperRes)));
	g_hash_table_insert(dH, g_strdup("LT"), g_strdup_printf("%d", offsetS(settings.general.label.format)));
	g_hash_table_insert(dH, g_strdup("HDR1"), g_strdup_printf("%d", offsetS(settings.headers.dispHdr[0])));
	g_hash_table_insert(dH, g_strdup("HDR2"), g_strdup_printf("%d", offsetS(settings.headers.dispHdr[1])));
	g_hash_table_insert(dH, g_strdup("HDR3"), g_strdup_printf("%d", offsetS(settings.headers.dispHdr[2])));
	g_hash_table_insert(dH, g_strdup("HDR4"), g_strdup_printf("%d", offsetS(settings.headers.dispHdr[3])));
	g_hash_table_insert(dH, g_strdup("HDR5"), g_strdup_printf("%d", offsetS(settings.headers.dispHdr[4])));
	g_hash_table_insert(dH, g_strdup("HDR6"), g_strdup_printf("%d", offsetS(settings.headers.dispHdr[5])));
	g_hash_table_insert(dH, g_strdup("HDR7"), g_strdup_printf("%d", offsetS(settings.headers.dispHdr[6])));
	g_hash_table_insert(dH, g_strdup("HDR8"), g_strdup_printf("%d", offsetS(settings.headers.dispHdr[7])));
	g_hash_table_insert(dH, g_strdup("HDR9"), g_strdup_printf("%d", offsetS(settings.headers.dispHdr[8])));
	g_hash_table_insert(dH, g_strdup("HDR10"), g_strdup_printf("%d", offsetS(settings.headers.dispHdr[9])));
	g_hash_table_insert(dH, g_strdup("HDR11"), g_strdup_printf("%d", offsetS(settings.headers.dispHdr[10])));
	g_hash_table_insert(dH, g_strdup("HDR12"), g_strdup_printf("%d", offsetS(settings.headers.dispHdr[11])));
	g_hash_table_insert(dH, g_strdup("HDR13"), g_strdup_printf("%d", offsetS(settings.headers.dispHdr[12])));
	g_hash_table_insert(dH, g_strdup("HDR14"), g_strdup_printf("%d", offsetS(settings.headers.dispHdr[13])));
	g_hash_table_insert(dH, g_strdup("HDR15"), g_strdup_printf("%d", offsetS(settings.headers.dispHdr[14])));
	g_hash_table_insert(dH, g_strdup("HDR16"), g_strdup_printf("%d", offsetS(settings.headers.dispHdr[15])));
	g_hash_table_insert(dH, g_strdup("HDR17"), g_strdup_printf("%d", offsetS(settings.headers.dispHdr[16])));
	g_hash_table_insert(dH, g_strdup("SRTDEF"), g_strdup_printf("%d", offsetS(settings.general.sort.defaultSort)));
	g_hash_table_insert(dH, g_strdup("SRTSTR"), g_strdup_printf("%d", offsetS(settings.general.sort.str)));
#if 0
	g_hash_table_insert(dH, g_strdup("SRT1"), g_strdup_printf("%d", offsetS(settings.general.sort.criteria[0])));
	g_hash_table_insert(dH, g_strdup("SRT2"), g_strdup_printf("%d", offsetS(settings.general.sort.criteria[1])));
	g_hash_table_insert(dH, g_strdup("SRT3"), g_strdup_printf("%d", offsetS(settings.general.sort.criteria[2])));
	g_hash_table_insert(dH, g_strdup("SRT4"), g_strdup_printf("%d", offsetS(settings.general.sort.criteria[3])));
	g_hash_table_insert(dH, g_strdup("SRT5"), g_strdup_printf("%d", offsetS(settings.general.sort.criteria[4])));
	g_hash_table_insert(dH, g_strdup("SRT6"), g_strdup_printf("%d", offsetS(settings.general.sort.criteria[5])));
	g_hash_table_insert(dH, g_strdup("SRT7"), g_strdup_printf("%d", offsetS(settings.general.sort.criteria[6])));
	g_hash_table_insert(dH, g_strdup("SRT8"), g_strdup_printf("%d", offsetS(settings.general.sort.criteria[7])));
	g_hash_table_insert(dH, g_strdup("SRT9"), g_strdup_printf("%d", offsetS(settings.general.sort.criteria[8])));
	g_hash_table_insert(dH, g_strdup("SRT10"), g_strdup_printf("%d", offsetS(settings.general.sort.criteria[9])));
#endif
	g_hash_table_insert(dH, g_strdup("TFG"), g_strdup_printf("%d", offsetS(settings.colors.fg[0].red)));
	g_hash_table_insert(dH, g_strdup("TBG"), g_strdup_printf("%d", offsetS(settings.colors.bg[0].red)));
	g_hash_table_insert(dH, g_strdup("TSB"), g_strdup_printf("%d", offsetS(settings.colors.sb[0].red)));
	g_hash_table_insert(dH, g_strdup("TG1"), g_strdup_printf("%d", offsetS(settings.colors.gp[0][0].red)));
	g_hash_table_insert(dH, g_strdup("TO1"), g_strdup_printf("%d", offsetS(settings.colors.op[0][0].red)));
	g_hash_table_insert(dH, g_strdup("MFG"), g_strdup_printf("%d", offsetS(settings.colors.fg[1].red)));
	g_hash_table_insert(dH, g_strdup("MBG"), g_strdup_printf("%d", offsetS(settings.colors.bg[1].red)));
	g_hash_table_insert(dH, g_strdup("MSB"), g_strdup_printf("%d", offsetS(settings.colors.sb[1].red)));
	g_hash_table_insert(dH, g_strdup("MG1"), g_strdup_printf("%d", offsetS(settings.colors.gp[1][0].red)));
	g_hash_table_insert(dH, g_strdup("MO1"), g_strdup_printf("%d", offsetS(settings.colors.op[1][0].red)));
	g_hash_table_insert(dH, g_strdup("SFG"), g_strdup_printf("%d", offsetS(settings.colors.fg[2].red)));
	g_hash_table_insert(dH, g_strdup("SBG"), g_strdup_printf("%d", offsetS(settings.colors.bg[2].red)));
	g_hash_table_insert(dH, g_strdup("SSB"), g_strdup_printf("%d", offsetS(settings.colors.sb[2].red)));
	g_hash_table_insert(dH, g_strdup("SPSB"), g_strdup_printf("%d", offsetS(settings.colors.sb[3].red)));
	g_hash_table_insert(dH, g_strdup("OV1"), g_strdup_printf("%d", offsetS(settings.colors.ol[0].red)));
	g_hash_table_insert(dH, g_strdup("OV2"), g_strdup_printf("%d", offsetS(settings.colors.ol[1].red)));
	g_hash_table_insert(dH, g_strdup("OV3"), g_strdup_printf("%d", offsetS(settings.colors.ol[2].red)));
	g_hash_table_insert(dH, g_strdup("OV4"), g_strdup_printf("%d", offsetS(settings.colors.ol[3].red)));
	g_hash_table_insert(dH, g_strdup("OV5"), g_strdup_printf("%d", offsetS(settings.colors.ol[4].red)));
	g_hash_table_insert(dH, g_strdup("OV6"), g_strdup_printf("%d", offsetS(settings.colors.ol[5].red)));
	g_hash_table_insert(dH, g_strdup("OV7"), g_strdup_printf("%d", offsetS(settings.colors.ol[6].red)));
	g_hash_table_insert(dH, g_strdup("OV8"), g_strdup_printf("%d", offsetS(settings.colors.ol[7].red)));
	g_hash_table_insert(dH, g_strdup("OV9"), g_strdup_printf("%d", offsetS(settings.colors.ol[8].red)));
	g_hash_table_insert(dH, g_strdup("OV10"), g_strdup_printf("%d", offsetS(settings.colors.ol[9].red)));
	g_hash_table_insert(dH, g_strdup("OV11"), g_strdup_printf("%d", offsetS(settings.colors.ol[10].red)));
	g_hash_table_insert(dH, g_strdup("OV12"), g_strdup_printf("%d", offsetS(settings.colors.ol[11].red)));
	g_hash_table_insert(dH, g_strdup("OV13"), g_strdup_printf("%d", offsetS(settings.colors.ol[12].red)));
	g_hash_table_insert(dH, g_strdup("OV14"), g_strdup_printf("%d", offsetS(settings.colors.ol[13].red)));
	g_hash_table_insert(dH, g_strdup("OV15"), g_strdup_printf("%d", offsetS(settings.colors.ol[14].red)));
	g_hash_table_insert(dH, g_strdup("OV16"), g_strdup_printf("%d", offsetS(settings.colors.ol[15].red)));
	g_hash_table_insert(dH, g_strdup("OV17"), g_strdup_printf("%d", offsetS(settings.colors.ol[16].red)));
	g_hash_table_insert(dH, g_strdup("OV18"), g_strdup_printf("%d", offsetS(settings.colors.ol[17].red)));
	g_hash_table_insert(dH, g_strdup("OV19"), g_strdup_printf("%d", offsetS(settings.colors.ol[18].red)));
	g_hash_table_insert(dH, g_strdup("OV20"), g_strdup_printf("%d", offsetS(settings.colors.ol[19].red)));
	g_hash_table_insert(dH, g_strdup("OV21"), g_strdup_printf("%d", offsetS(settings.colors.ol[20].red)));
	g_hash_table_insert(dH, g_strdup("OV22"), g_strdup_printf("%d", offsetS(settings.colors.ol[21].red)));
	g_hash_table_insert(dH, g_strdup("OV23"), g_strdup_printf("%d", offsetS(settings.colors.ol[22].red)));
	g_hash_table_insert(dH, g_strdup("OV24"), g_strdup_printf("%d", offsetS(settings.colors.ol[23].red)));
	g_hash_table_insert(dH, g_strdup("OV25"), g_strdup_printf("%d", offsetS(settings.colors.ol[24].red)));
	g_hash_table_insert(dH, g_strdup("OV26"), g_strdup_printf("%d", offsetS(settings.colors.ol[25].red)));
	g_hash_table_insert(dH, g_strdup("OV27"), g_strdup_printf("%d", offsetS(settings.colors.ol[26].red)));
	g_hash_table_insert(dH, g_strdup("OV28"), g_strdup_printf("%d", offsetS(settings.colors.ol[27].red)));
	g_hash_table_insert(dH, g_strdup("OV29"), g_strdup_printf("%d", offsetS(settings.colors.ol[28].red)));
	g_hash_table_insert(dH, g_strdup("OV30"), g_strdup_printf("%d", offsetS(settings.colors.ol[29].red)));

	for(i=0;i<cnt-1;i++)
		defaults[i].offset = atoi(g_hash_table_lookup(dH, defaults[i].desc));
	g_hash_table_destroy(dH);

	cnt=sizeof(fileDefaults)/sizeof(DFAULTS);
	dH = g_hash_table_new_full(g_str_hash, g_str_equal, (GDestroyNotify)free, (GDestroyNotify)free);
	g_hash_table_insert(dH, g_strdup("FFLT"), g_strdup_printf("%d", offsetS(settings.general.input.file.filter)));
	g_hash_table_insert(dH, g_strdup("FMINC"), g_strdup_printf("%d", offsetS(settings.general.input.file.mseedInc)));
	g_hash_table_insert(dH, g_strdup("FMEXC"), g_strdup_printf("%d", offsetS(settings.general.input.file.mseedExc)));
	g_hash_table_insert(dH, g_strdup("FMLEN"), g_strdup_printf("%d", offsetS(settings.general.input.file.mseedLen)));
	g_hash_table_insert(dH, g_strdup("FREPL"), g_strdup_printf("%d", offsetS(settings.general.input.file.replace)));
	g_hash_table_insert(dH, g_strdup("FDIR"), g_strdup_printf("%d", offsetS(settings.general.input.file.directory)));

	for(i=0;i<cnt-1;i++)
		fileDefaults[i].offset = atoi(g_hash_table_lookup(dH, fileDefaults[i].desc));
	g_hash_table_destroy(dH);
	
	settings.general.startUP.winDims[WIDTH] = 1000;
	settings.general.startUP.winDims[HEIGHT] = 700;
	settings.general.display.mode = TRACEMODE;
	settings.general.startUP.panePos[0] = 350;
	settings.general.startUP.panePos[1] = 233;
	settings.general.startUP.panePos[2] = 466;
	settings.general.display.gather.gain[TRACESCR] = 1.0;
	settings.general.display.gather.gain[MAGNIFYSCR] = 1.0;
	settings.general.display.gather.gain[SPECTRASCR] = 1.0;
	settings.general.display.gather.spread = 1.0;
	settings.general.tForm.uvwSensor = STS2;
	settings.general.input.file.directory = g_strdup("");
	settings.general.input.file.filter = g_strdup("");
	settings.general.input.file.mseedInc = g_strdup("");
	settings.general.input.file.mseedExc = g_strdup("");
	settings.general.input.file.mseedLen = g_strdup("");
	settings.general.input.file.replace = TRUE;
	settings.trace.numPlots = 12;
	settings.trace.amp = WSCALETRC;
	settings.trace.time = RELATIVE;
	states.tForm.on = OFF;
	settings.trace.gFlag = ON;
	settings.trace.oFlag = ON;
	settings.magnify.Tamp = DISPS;
	settings.magnify.Wamp = WSCALETRC;
	settings.magnify.pick = WSCALEWIND;
	settings.magnify.pickKEYOn = OFF;
	settings.magnify.pickCOMOn = OFF;
	settings.magnify.overlay = OFF;
	settings.magnify.zoomFactor = 15;
	settings.magnify.scrollFactor = 60;
	settings.magnify.anchor = MIDDLE;
	settings.magnify.dispBox[ZOOMBOX] = ON;
	settings.magnify.dispBox[PICKBOX] = ON;
	settings.magnify.gFlag = ON;
	settings.magnify.oFlag = ON;
	settings.general.filter.AGC.winLen = 200;		// AGC Filter defaults
	settings.general.filter.AGC.dwin = 10;
	settings.general.filter.AGC.detect = FALSE;
	settings.general.filter.AGC.threshold = 25.0;
	settings.spectra.display.amp = WSCALETRC;
	settings.spectra.display.overlay = OFF;
	settings.spectra.display.yAxis = LIN;
	settings.spectra.display.xAxis = LOG;
	settings.spectra.display.decades = 3.5;
	settings.general.label.format = LABEL_HEADER;
	settings.general.label.length = 0;
	settings.general.print.format = PRINT_FMT_PNG;
	settings.general.print.paperSize = LETTER;
	settings.general.print.paperRes = HIGHRES;
	states.split.panes = SPLIT2;
	settings.split.pane = states.split.panes;
	settings.split.traceDisp = TFORMORIG;
	settings.split.DA[SPLIT2][TOP] = TRACESCR;
	settings.split.DA[SPLIT2][MID] = NONE;
	settings.split.DA[SPLIT2][BOT] = MAGNIFYSCR;
	settings.split.DA[SPLIT3][TOP] = TRACESCR;
	settings.split.DA[SPLIT3][MID] = MAGNIFYSCR;
	settings.split.DA[SPLIT3][BOT] = SPECTRASCR;
	settings.split.DT[SPLIT2][TRACESCR] = SPLITDA2T;
	settings.split.DT[SPLIT2][MAGNIFYSCR] = SPLITDA2B;
	settings.split.DT[SPLIT3][TRACESCR] = SPLITDA3T;
	settings.split.DT[SPLIT3][MAGNIFYSCR] = SPLITDA3M;
	settings.split.DT[SPLIT3][SPECTRASCR] = SPLITDA3B;

	settings.logs.dispType = LOGFILE;

	settings.general.sort.defaultSort = PQLSORT;
	settings.general.sort.selected = PQLSORT;
	settings.general.sort.str = g_strdup("");

	settings.colors.fg[TRACESCR].red = 10000;
	settings.colors.fg[TRACESCR].green = 10000;
	settings.colors.fg[TRACESCR].blue = 10000;
	settings.colors.bg[TRACESCR].red = 220*256;
	settings.colors.bg[TRACESCR].green = 211*256;
	settings.colors.bg[TRACESCR].blue = 237*256;
	settings.colors.sb[TRACESCR].red = 9*256;
	settings.colors.sb[TRACESCR].green = 60*256;
	settings.colors.sb[TRACESCR].blue = 94*256;
	settings.colors.gp[TRACESCR][0].red = 255*255;
	settings.colors.gp[TRACESCR][0].green = 255*255;
	settings.colors.gp[TRACESCR][0].blue = 27*255;
	settings.colors.op[TRACESCR][0].red = 253*255;
	settings.colors.op[TRACESCR][0].green = 64*255;
	settings.colors.op[TRACESCR][0].blue = 64*255;

	settings.colors.fg[MAGNIFYSCR].red = 10000;
	settings.colors.fg[MAGNIFYSCR].green = 10000;
	settings.colors.fg[MAGNIFYSCR].blue = 10000;
	settings.colors.bg[MAGNIFYSCR].red = 255*256;
	settings.colors.bg[MAGNIFYSCR].green = 217*256;
	settings.colors.bg[MAGNIFYSCR].blue = 241*256;
	settings.colors.sb[MAGNIFYSCR].red = 78*256;
	settings.colors.sb[MAGNIFYSCR].green = 16*256;
	settings.colors.sb[MAGNIFYSCR].blue = 83*256;
	settings.colors.gp[MAGNIFYSCR][0].red = 255*255;
	settings.colors.gp[MAGNIFYSCR][0].green = 215*255;
	settings.colors.gp[MAGNIFYSCR][0].blue = 0*255;
	settings.colors.op[MAGNIFYSCR][0].red = 253*255;
	settings.colors.op[MAGNIFYSCR][0].green = 64*255;
	settings.colors.op[MAGNIFYSCR][0].blue = 64*255;

	settings.colors.fg[SPECTRASCR].red = 10000;
	settings.colors.fg[SPECTRASCR].green = 10000;
	settings.colors.fg[SPECTRASCR].blue = 10000;
	settings.colors.bg[SPECTRASCR].red = 217*256;
	settings.colors.bg[SPECTRASCR].green = 242*256;
	settings.colors.bg[SPECTRASCR].blue = 217*256;
	settings.colors.sb[SPECTRASCR].red = 6580;
	settings.colors.sb[SPECTRASCR].green = 17920;
	settings.colors.sb[SPECTRASCR].blue = 25344;

	settings.colors.sb[SPLITSCR].red = 6580;
	settings.colors.sb[SPLITSCR].green = 17920;
	settings.colors.sb[SPLITSCR].blue = 25344;

	settings.colors.ol[0].red = 205*256;
	settings.colors.ol[0].green = 92*256;
	settings.colors.ol[0].blue = 92*256;
	settings.colors.ol[1].red = 85*256;
	settings.colors.ol[1].green = 107*256;
	settings.colors.ol[1].blue = 47*256;
	settings.colors.ol[2].red = 240*256;
	settings.colors.ol[2].green = 248*256;
	settings.colors.ol[2].blue = 100*256;
	settings.colors.ol[3].red = 188*256;
	settings.colors.ol[3].green = 143*256;
	settings.colors.ol[3].blue = 143*256;
	settings.colors.ol[4].red = 199*256;
	settings.colors.ol[4].green = 21*256;
	settings.colors.ol[4].blue = 133*256;
	settings.colors.ol[5].red = 255*256;
	settings.colors.ol[5].green = 105*256;
	settings.colors.ol[5].blue = 180*256;
	settings.colors.ol[6].red = 255*256;
	settings.colors.ol[6].green = 69*256;
	settings.colors.ol[6].blue = 0*256;
	settings.colors.ol[7].red = 34*256;
	settings.colors.ol[7].green = 139*256;
	settings.colors.ol[7].blue = 34*256;
	settings.colors.ol[8].red = 100*256;
	settings.colors.ol[8].green = 149*256;
	settings.colors.ol[8].blue = 237*256;
	settings.colors.ol[9].red = 139*256;
	settings.colors.ol[9].green = 69*256;
	settings.colors.ol[9].blue = 19*256;
	settings.colors.ol[10].red = 238*256;
	settings.colors.ol[10].green = 130*256;
	settings.colors.ol[10].blue = 238*256;
	settings.colors.ol[11].red = 255*256;
	settings.colors.ol[11].green = 20*256;
	settings.colors.ol[11].blue = 147*256;
	settings.colors.ol[12].red = 219*256;
	settings.colors.ol[12].green = 112*256;
	settings.colors.ol[12].blue = 147*256;
	settings.colors.ol[13].red = 0*256;
	settings.colors.ol[13].green = 250*256;
	settings.colors.ol[13].blue = 154*256;
	settings.colors.ol[14].red = 123*256;
	settings.colors.ol[14].green = 104*256;
	settings.colors.ol[14].blue = 238*256;
	settings.colors.ol[15].red = 244*256;
	settings.colors.ol[15].green = 164*256;
	settings.colors.ol[15].blue = 96*256;
	settings.colors.ol[16].red = 208*256;
	settings.colors.ol[16].green = 32*256;
	settings.colors.ol[16].blue = 144*256;
	settings.colors.ol[17].red = 173*256;
	settings.colors.ol[17].green = 255*256;
	settings.colors.ol[17].blue = 47*256;
	settings.colors.ol[18].red = 208*256;
	settings.colors.ol[18].green = 32*256;
	settings.colors.ol[18].blue = 144*256;
	settings.colors.ol[19].red = 124*256;
	settings.colors.ol[19].green = 252*256;
	settings.colors.ol[19].blue = 0*256;
	settings.colors.ol[20].red = 138*256;
	settings.colors.ol[20].green = 43*256;
	settings.colors.ol[20].blue = 226*256;
	settings.colors.ol[21].red = 165*256;
	settings.colors.ol[21].green = 42*256;
	settings.colors.ol[21].blue = 42*256;
	settings.colors.ol[22].red = 148*256;
	settings.colors.ol[22].green = 0*256;
	settings.colors.ol[22].blue = 211*256;
	settings.colors.ol[23].red = 250*256;
	settings.colors.ol[23].green = 250*256;
	settings.colors.ol[23].blue = 210*256;
	settings.colors.ol[24].red = 139*256;
	settings.colors.ol[24].green = 0*256;
	settings.colors.ol[24].blue = 0*256;
	settings.colors.ol[25].red = 60*256;
	settings.colors.ol[25].green = 179*256;
	settings.colors.ol[25].blue = 113*256;
	settings.colors.ol[26].red = 176*256;
	settings.colors.ol[26].green = 196*256;
	settings.colors.ol[26].blue = 222*256;
	settings.colors.ol[27].red = 205*256;
	settings.colors.ol[27].green = 51*256;
	settings.colors.ol[27].blue = 51*256;
	settings.colors.ol[28].red = 138*256;
	settings.colors.ol[28].green = 43*256;
	settings.colors.ol[28].blue = 226*256;
	settings.colors.ol[29].red = 205*256;
	settings.colors.ol[29].green = 205*256;
	settings.colors.ol[29].blue = 0*256;
	states.tForm.filter = NONE;
	states.general.numCPUs = get_ncpu_pqlx();

	for (i=0;i<MAXPICKS;i++)
	{	// default user pick names to sequential numbers
		sprintf(settings.magnify.pickTag[i], "%d", i+1);
	}

	for (i=0;i<TTLDTYPES;i++)
	{
		switch(i)
		{
			case BIT32:
				dsizes[i] = sizeof(gint32);
			break;
			case BIT16:
				dsizes[i] = sizeof(short);
			break;
			case MY_FLOAT:
				dsizes[i] = sizeof(float);
			break;
			case MY_DOUBLE:
				dsizes[i] = sizeof(double);
			break;
			case MY_CHAR:
				dsizes[i] = sizeof(char);
			break;
		}
	}
	
	sortInit();
	
	states.header.newCols = TRUE;
	for(i=0;i<NUM_HDR_COLS;i++)
	{
		settings.headers.dispHdr[i]=TRUE;
		switch(i)
		{
			case HDR_LFN:		// long filename
				headers[i].offset = offsetD(trace.data.head.longFN, trace);
			break;

			case HDR_SFN:		// short filename
				headers[i].offset = offsetD(trace.data.head.shortFN, trace);
			break;

			case HDR_NTW:		// network
				headers[i].offset = offsetD(trace.data.head.network, trace);
			break;

			case HDR_STN:		// station
				headers[i].offset = offsetD(trace.data.head.station, trace);
			break;

			case HDR_LOC:		// location ID
				headers[i].offset = offsetD(trace.data.head.location, trace);
			break;

			case HDR_CHN:		// channel
				headers[i].offset = offsetD(trace.data.head.channel, trace);
			break;

			case HDR_SID:		// sensor ID
				headers[i].offset = offsetD(trace.data.head.sensorID, trace);
			break;

			case HDR_TIME:		// start time
				headers[i].offset = offsetD(trace.data.head.epochs, trace);
			break;

			case HDR_LEN:		// length in seconds
				headers[i].offset = offsetD(trace.data.head.length, trace);
			break;

			case HDR_SPS:		// stored as msecs/sample, displayed as samp/sec
				headers[i].offset = offsetD(trace.data.head.sampInt, trace);
			break;

			case HDR_NDP:		// number of samples
				headers[i].offset = offsetD(trace.data.dataInfo.head.numSamples, trace);	//trace.trace.samples);
			break;

			case HDR_FMT:		// data format
				headers[i].offset = offsetD(trace.data.fileType, trace);
			break;

			case HDR_GAIN:		// gain
				headers[i].offset = offsetD(trace.data.head.gainConst, trace);
			break;

			case HDR_SCALE:		// scale
				headers[i].offset = offsetD(trace.data.head.scale_fac, trace);
			break;

			case HDR_AMPS:		// max/min amplitude
				headers[i].offset = offsetD(trace.data.dataInfo.amps, trace);
			break;

			case HDR_GAPS:		// number of gaps
				headers[i].offset = offsetD(trace.trace.numGaps, trace);
			break;

			case HDR_OLAPS:		// number of overlaps
				headers[i].offset = offsetD(trace.trace.numOlaps, trace);
			break;
		}
	}

	settings.general.myWordOrder = testByteOrder();
}

static int countChar(char *str, char *c)
{
	int	 i, ttlCnt;
	for (i=0, ttlCnt=0; str[i]; 
		 (ttlCnt+=((str[i]==c[0])?1:0)), i++);
	return ttlCnt;
}

static void sortStr(int func, plot_settings *base)
{
	SORTINFO	*sortInfo;
	CRITDEF		*critDef;
	char		**sorts=NULL, **crits=NULL;

	switch (func)
	{
		case SORTENCRYPT:		// convert the in-memory version to string version for saving
		{
			int			i, sortNum, ttlCrits, ttlSorts;
			GSList		*critIter, *sortIter;
			
			for (sortIter = base->general.sort.sorts, ttlSorts=0;
				 sortIter;
				 sortIter = g_slist_next(sortIter))
			{
				sortInfo = sortIter->data;
				if (!g_slist_length(sortInfo->user))
					continue;
				ttlSorts++;
			}
			ttlSorts += 1;		// 1 extra to null terminate
			sorts = calloc(ttlSorts, sizeof(char*));
			for (sortIter = base->general.sort.sorts, sortNum=0;
				 sortIter;
				 sortIter = g_slist_next(sortIter))
			{
				sortInfo = sortIter->data;
				if (!g_slist_length(sortInfo->user))
					continue;
				ttlCrits = g_slist_length(sortInfo->user)+3;	// 3 extra: 0=sortName, ttl-1=SORTTYPESEP, ttl=NULL
				crits = calloc(ttlCrits, sizeof(char*));
				crits[0] = g_strdup_printf("%s%s", sortTypeStr[sortInfo->sortNum], SORTTYPENAMESEP);
				for (critIter = sortInfo->user, i=1;
					 critIter;
					 critIter = g_slist_next(critIter), i++)
				{
					critDef = critIter->data;
					crits[i] = g_strdup_printf("%s%d%s%d", SORTCRITSEP, critDef->num,
															SORTDIRSEP, (int) critDef->dir);
				}
				crits[i] = g_strdup(SORTTYPESEP);
				sorts[sortNum] = g_strjoinv(NULL, crits);
				g_strfreev(crits);
				sortNum++;
			}
			base->general.sort.str = g_strjoinv(NULL, sorts);
			g_strfreev(sorts);
		}
		break;

		case SORTDECRYPT:		// convert the string version to in-memory version
		{
			int			i, j, critCnt, sortNum;
			char		**crit, **critDefs;
			
			sorts = g_strsplit(base->general.sort.str, SORTTYPESEP, 0);		// split sort types
			for (i=0;sorts[i];i++)
			{
				critCnt = countChar(sorts[i], SORTCRITSEP);
				if (critCnt == 0)
					continue;		// do nothing if no user prefs defined
				
				crit = g_strsplit(sorts[i], SORTTYPENAMESEP, 0);			// split name from crits
				for (sortNum=0;sortNum<TTLSORTTYPES;sortNum++)					// find the enum equivalent
				{
					if (!strcmp(sortTypeStr[sortNum], crit[0]))
						break;
				}
				if (sortNum == TTLSORTTYPES)	// not found
					continue;
				sortInfo = g_slist_nth_data(base->general.sort.sorts, sortNum);
				crits = g_strsplit(crit[1], SORTCRITSEP, 0);				// split the individual crits
				for (j=1;crits[j];j++)
				{
					critDefs = g_strsplit(crits[j], SORTDIRSEP, 0);
					critDef = calloc(1, sizeof(CRITDEF));
					critDef->num = atol(critDefs[0]);
					critDef->dir = (gboolean) atol(critDefs[1]);
					sortInfo->user = g_slist_append(sortInfo->user, critDef);
					g_strfreev(critDefs);
				}
				g_strfreev(crit);
				g_strfreev(crits);
			}
			g_strfreev(sorts);
		}
		break;
	}
}

void make_defaults_file(plot_settings *base, int which)
{
	int 	i, cnt;
	DBLDI	dbldi;
	DBLRECORDS dblrecs;
	char	*value;
	DFAULTS	*defPtr;

	switch (which)
	{
		case DEFAULTSET:
			cnt=sizeof(defaults)/sizeof(DFAULTS);
			sortStr(SORTENCRYPT, base);
			defPtr = defaults;
		break;
		
		case FILEDEFSET:
			cnt=sizeof(fileDefaults)/sizeof(DFAULTS);
			defPtr = fileDefaults;
		break;
	}
	
	memset(&dbldi, 0, sizeof(DBLDI));
	memset(&dblrecs, 0, sizeof(DBLRECORDS));
	dbldi.descr.tableData = DEFAULTST;
	dbldi.descr.dataEvent = WRITE_ALL;
	dbldi.data = &dblrecs;
	dblrecs.numRecs = cnt-1;
	if((dblrecs.recs = calloc(dblrecs.numRecs, sizeof(void *)))==NULL)
	{
		fprintf(stderr, "Out of Memory, cannot continue.\n");
		exit(-1);
	}
	
	for(i=0; i<dblrecs.numRecs ;i++, defPtr++)
	{
		dblrecs.recs[i] = calloc(1, sizeof(DEFAULTS_REC));
		strncpy(((DEFAULTS_REC *) dblrecs.recs[i])->name, defPtr->desc, strlen(defPtr->desc));
		value = ((DEFAULTS_REC *) dblrecs.recs[i])->value;
		switch(defPtr->type)
		{
			case MYBOOL:
				sprintf(value, "%d", *((gboolean *) location(defPtr->offset)));
			break;

			case MYSHORT:
				sprintf(value, "%d", *((guint16 *) location(defPtr->offset)));
			break;

			case MYINT:
				sprintf(value, "%d", *((int *) location(defPtr->offset)));
			break;

			case MYFLOAT:
				sprintf(value, "%f", *((float *) location(defPtr->offset)));
			break;

			case MYSTRING:
				sprintf(value, "%s", (char *) location(defPtr->offset));
			break;

			case MYSTRINGPTR:
				if ((char *) *((char **) location(defPtr->offset)))
					sprintf(value, "%s", (char *) *((char **) location(defPtr->offset)));
			break;

			case MYCOLOR:
				sprintf(value, "%d %d %d",  
								*((guint16 *) location(defPtr->offset)),
								*((guint16 *) location(defPtr->offset+sizeof(guint16))),
								*((guint16 *) location(defPtr->offset+2*sizeof(guint16))));
			break;
#if 0
			case MYSORT:
				sprintf(value, "%d %d",  
								*((int *) location(defPtr->offset)),
								*((int *) location(defPtr->offset+(TTLPQLSORTCRITERIA*sizeof(int)))));
			break;
#endif
		}
	}
	
	dbl_request(&dbldi);
	
	// free up memory
	for(i=0; i<dblrecs.numRecs ;i++)
	{
		free(dblrecs.recs[i]);
	}
	free(dblrecs.recs);
}

#define DEFAULTSFILE	".pqldfaults"
#ifndef WIN32
void read_old_defaults_file(plot_settings *base)
{
  int i;
  char *home, *getenv(), dfault[256], string[256], variable[30];
  FILE *fp=NULL;
  void set_defaults();

  set_defaults();
  home = getenv("HOME");

  if(home) 
  {
    strcpy(dfault, home);
	strcat(dfault, "/");
    strcat(dfault, DEFAULTSFILE);
    if((fp = fopen(dfault, "r")))
      while(fgets(string, 80, fp)) 
	  {
		sscanf(string, "%s", variable);
		for(i=0;defaults[i].type!=-1;++i)
	  		if (!strcmp(defaults[i].desc, variable))
	    		break;

		switch(defaults[i].type)
		{
			case MYBOOL:
				sscanf(string, "%*s %d\n", (gboolean *) location(defaults[i].offset));
			break;

			case MYSHORT:
				sscanf(string, "%*s %hd", (guint16 *) location(defaults[i].offset));
			break;

			case MYINT:
				sscanf(string, "%*s %d", (int *) location(defaults[i].offset));
			break;

			case MYFLOAT:
				sscanf(string, "%*s %f", (float *) location(defaults[i].offset));
			break;

			case MYSTRING:
				sscanf(string, "%*s %s", (char *) location(defaults[i].offset));
			break;

			case MYCOLOR:
				sscanf(string, "%*s %hd %hd %hd\n", ((guint16 *) location(defaults[i].offset)),
								((guint16 *) location(defaults[i].offset+sizeof(guint16))),
								((guint16 *) location(defaults[i].offset+2*sizeof(guint16))));
			break;

#if 0
			case MYSORT:
				sscanf(string, "%*s %d %d\n", ((int *) location(defaults[i].offset)),
								((int *) location(defaults[i].offset+(TTLPQLSORTCRITERIA*sizeof(int)))));
				if (*((int *) location(defaults[i].offset)))
					settings.general.sort.numCrit++;
			break;
#endif
		}

      }

    if (fp)	
      fclose(fp);
  }
}
#endif

void read_defaults_file(plot_settings *base, int which)
{
	int		i, j, cnt, ret;
	DBLDI	dbldi;
	DBLRECORDS dblrecs;
	gchar	*value;
	DFAULTS	*defPtr, *defPtr1;

	switch (which)
	{
		case DEFAULTSET:
			set_defaults();
			cnt=sizeof(defaults)/sizeof(DFAULTS);
			defPtr1 = defaults;
			
			// load the user-defined filters now
			readFilters();
		break;
		
		case FILEDEFSET:
			cnt=sizeof(fileDefaults)/sizeof(DFAULTS);
			defPtr1 = fileDefaults;
		break;
	}

	memset(&dbldi, 0, sizeof(DBLDI));
	memset(&dblrecs, 0, sizeof(DBLRECORDS));
	dbldi.descr.tableData = DEFAULTST;
	dbldi.descr.dataEvent = READ_ALL;
	dbldi.data = &dblrecs;
	ret = dbl_request(&dbldi);
	
	switch (ret)
	{
		case FAILURE:
			fprintf(stderr, "Cannot Read Local Settings File!\n");
			fprintf(stderr, "You must either:\n");
			fprintf(stderr, "\tExecute PQLX program 'convPQLXDBL' to Convert (all Settings Retained), or\n");
			fprintf(stderr, "\tCompletely remove directory and contents (all Settings LOST!) '~/.pql'\n");
			fprintf(stderr, "And then re-execute this program.\n\n");
			exit(-1);
		break;
		case FILEDNE:
			return;
		break;
	}

	if(dblrecs.numRecs) 
	{
      for(j=0;j<dblrecs.numRecs;j++)
	  {
		for(i=0;i<cnt;++i)
	  		if (!strcmp((defPtr1+i)->desc, ((DEFAULTS_REC *) (dblrecs.recs[j]))->name))
	    		break;
	    if (i==cnt)
	    	continue;
		defPtr = defPtr1+i;

		value = ((DEFAULTS_REC *) (dblrecs.recs[j]))->value;
		switch(defPtr->type)
		{
			case MYBOOL:
				sscanf(value, "%d", (gboolean *) location(defPtr->offset));
			break;

			case MYSHORT:
				sscanf(value, "%hd", (guint16 *) location(defPtr->offset));
			break;

			case MYINT:
				sscanf(value, "%d", (int *) location(defPtr->offset));
			break;

			case MYFLOAT:
				sscanf(value, "%f", (float *) location(defPtr->offset));
			break;

			case MYSTRING:
				sscanf(value, "%s", (char *) location(defPtr->offset));
			break;

			case MYSTRINGPTR:
				if (value &&
					strlen(value) &&
					isascii(value[0]))
				{	// handle empty string that have been previously saved
					*((char **) location(defPtr->offset)) = g_strdup(value);
				}
			break;

			case MYCOLOR:
				sscanf(value, "%hd %hd %hd", 
								((guint16 *) location(defPtr->offset)),
								((guint16 *) location(defPtr->offset+sizeof(guint16))),
								((guint16 *) location(defPtr->offset+2*sizeof(guint16))));
			break;

#if 0
			case MYSORT:
				sscanf(value, "%d %d", 
								((int *) location(defaults[i].offset)),
								((int *) location(defaults[i].offset+(TTLPQLSORTCRITERIA*sizeof(int)))));
				if (*((int *) location(defaults[i].offset)))
					settings.general.sort.numCrit++;
			break;
#endif
		}
		free(dblrecs.recs[j]);
      }
	  if (dblrecs.recs)
		free(dblrecs.recs);
	}
	
	switch (which)
	{
		case DEFAULTSET:
			// decrypt saved sort criteria
			sortStr(SORTDECRYPT, base);
		
			// 18-may-10 - these settings are no longer settable by the user, hard-coded
			settings.magnify.dispBox[ZOOMBOX] = ON;
			settings.magnify.dispBox[PICKBOX] = ON;
		
			// set any necessary states and/or other dependencies
			states.split.panes = settings.split.pane;
			if (settings.split.DA[SPLIT2][SPLITDA2T-SPLITDA2T] != TRACESCR &&
				settings.split.DA[SPLIT2][SPLITDA2B-SPLITDA2T] != TRACESCR)
				settings.split.DA[SPLIT2][SPLITDA2M-SPLITDA2T] = TRACESCR;
				
			// must set the selected sort to the default sort on startup
			settings.general.sort.selected = settings.general.sort.defaultSort;
			
			// special case to handle new zoomFactor setting range
			if (settings.magnify.zoomFactor > 30)
				settings.magnify.zoomFactor = 15;
			
			// set secondary overlap colors
			for (i=0;i<2;i++)
			{
				memcpy(&settings.colors.op[i][1], &settings.colors.op[i][0], sizeof(GdkColor));
				settings.colors.op[i][1].red += (settings.colors.op[i][1].red > 57120) ? -7650 : 7650;
				settings.colors.op[i][1].green += (settings.colors.op[i][1].green > 57120) ? -7650 : 7650;
				settings.colors.op[i][1].blue += (settings.colors.op[i][1].blue > 57120) ? -7650 : 7650;
			}
		
#ifndef PQL_ONLY
			settings.general.display.mode = TRACEMODE;		// GATHERMODE not allowed for pqlx
#endif
		break;
		
		case FILEDEFSET:
			settings.general.startUP.lastDir = g_strdup(settings.general.input.file.directory);
			settings.general.startUP.openReplace = settings.general.input.file.replace;
		break;
	}
	return;
}
