#include <stdlib.h>
#include <stdio.h>
#ifndef TTIME
#include <string.h>
#endif /* TTIME */
#include <rpc/rpc.h>
#include <math.h>
#include "ahhead.h"
#include "pql_defines.h"
#include "pql_externs.h"

#ifdef SOLARIS2
#include "libgen.h"
#endif

int	xdr_gethead();

int get_ah_hedr(FILE *fp, traceInfo *trace)
{
  struct ptime    date;

  long            err;
  XDR             xdr_datafile;
  int             xdr_getdata(), i;
  char           *mkdatspace(), *chr;
  ahhed           ah_head;
  float			oneSample;

  rewind(fp);
  xdrstdio_create(&xdr_datafile, fp, XDR_DECODE);
  
  if ((err = ((long) xdr_gethead(&ah_head, &xdr_datafile))) != 1) {
    return(FAILURE);
  }

  if (	(ah_head.record.abstime.yr == 0) || 
	    (ah_head.record.delta == 0) ||
		(ah_head.record.abstime.hr < 0 || ah_head.record.abstime.hr > 23) ||
		(ah_head.record.abstime.mn < 0 || ah_head.record.abstime.mn > 59) ||
		(ah_head.record.abstime.sec < 0 || ah_head.record.abstime.sec > 60)) 
  {		// file fails criteria for AH
	return(FAILURE);
  }
  
  	trace->data.fileType = AH_DATA;
  	trace->data.head.units = PQL_UNITS_VOLTS;
    trace->data.dataInfo.head.numSamples = ah_head.record.ndata;
    trace->data.head.sampInt = (double) ((int) ((ah_head.record.delta+.0000001) * 1000000.));	// handle rounding errors
	oneSample = 1./(1000000./trace->data.head.sampInt);
	trace->data.head.length = (double) (ah_head.record.ndata * ah_head.record.delta - oneSample);
    trace->data.head.data_form = MY_FLOAT;
	trace->data.head.dataSize = sizeof(float);
    trace->data.head.year = ah_head.record.abstime.yr;
    trace->data.head.day = julian (&ah_head.record.abstime);
    trace->data.head.hour = ah_head.record.abstime.hr;
    trace->data.head.minute = ah_head.record.abstime.mn;
    trace->data.head.second = (int) ah_head.record.abstime.sec;
    trace->data.head.m_secs = rint((ah_head.record.abstime.sec - (int) ah_head.record.abstime.sec) * 1000.);
    trace->data.head.gainConst = 1.;
    trace->data.head.scale_fac = 1.;
    
    trace->data.head.descr1 = g_strdup_printf("%i %03i %02i:%02i",
	      ah_head.record.abstime.yr,
	      julian (&ah_head.record.abstime),
	      ah_head.record.abstime.hr,
	      ah_head.record.abstime.mn);
      
    g_strchug(ah_head.station.code); CHOMP(ah_head.station.code);
    g_strchug(ah_head.station.chan); CHOMP(ah_head.station.chan);  
	trace->data.head.descr2 = g_strdup_printf("ID: %s C: %s" , 
									ah_head.station.code, ah_head.station.chan);
	strcpy(trace->data.head.station, ah_head.station.code);
	strcpy(trace->data.head.channel, ah_head.station.chan);

	trace->data.head.longFN = trace->data.fileName;
	trace->data.head.shortFN = 
#ifndef WIN32
			g_strdup((chr = strrchr(trace->data.fileName,'/')) ? chr+1 : trace->data.fileName);
#else
			g_strdup((chr = strrchr(trace->data.fileName,'\\')) ? chr+1 : trace->data.fileName);
#endif
	
	date.yr = trace->data.head.year;
	date.day = trace->data.head.day;
	date.hr = trace->data.head.hour;
	date.mn = trace->data.head.minute;
	date.sec = trace->data.head.second;
	trace->data.head.epochs = passcal_htoe(&date, FALSE) + trace->data.head.m_secs/1000.;
	
	trace->data.head.fileHdr = calloc(1, sizeof(ahhed));
	memcpy(trace->data.head.fileHdr, &ah_head, sizeof(ahhed));
    
  return PQLXSUCCESS;

}

int get_ah_data(traceInfo *trace)
{
    long            err;
    XDR             xdr_datafile;
    int             xdr_getdata();
    char           *mkdatspace(), dummy[3];
    ahhed           ah_head;
	FILE *fp;

	if ((fp=fopen(trace->data.fileName, "rb")) == NULL)
	{
		return(FAILURE);
	}

	if (!trace->data.head.year)
	{	// header not yet read, go get it first
		get_ah_hedr(fp, trace);
		rewind(fp);
	}

    xdrstdio_create(&xdr_datafile, fp, XDR_DECODE);

    if ((err = ((long) xdr_gethead(&ah_head, &xdr_datafile))) != 1) {
        return(FAILURE);
    }

    trace->data.dataInfo.data = mkdatspace(&ah_head);
    trace->data.dataInfo.head.numSamples = ((ah_head.record).ndata);

    if ((err = xdr_getdata(&ah_head, trace->data.dataInfo.data, &xdr_datafile)) != ah_head.record.ndata) 
	{
		free(trace->data.dataInfo.data);
		fprintf(stderr, "%s:\t Unable to read data file.\n", trace->data.fileName);
		return (FAILURE);
    }

    if (fread(dummy, 1, 2, fp) != 0) {
        free(trace->data.dataInfo.data);
        return(FAILURE);
    }

    if (ah_head.record.type != FLOAT) 
	{
        free(trace->data.dataInfo.data);
		fprintf(stderr, "%s:\t Only float data supported.\n", trace->data.fileName);
		return (FAILURE);
    }

	fclose(fp);
	trace->data.dataInfo.head.data_form = trace->data.head.data_form;
	trace->data.dataInfo.head.sampInt = trace->data.head.sampInt;
	trace->data.dataInfo.head.secsPsamp = (trace->data.head.sampInt/1000000.);
    return (PQLXSUCCESS);
}

GSList *IDah(FILE *fp, char *fileName)
{
  long            err;
  XDR             xdr_datafile;
  int             xdr_getdata(), ret;
  char           *mkdatspace();
  ahhed           ah_head;
  traceInfo		*trace;
  GSList		*traces=NULL;

  rewind(fp);
  xdrstdio_create(&xdr_datafile, fp, XDR_DECODE);
  
  if ((err = ((long) xdr_gethead(&ah_head, &xdr_datafile))) != 1) {
    return(NULL);
  }

  if (	(ah_head.record.abstime.yr == 0) || 
	    (ah_head.record.delta == 0) ||
		(ah_head.record.abstime.hr < 0 || ah_head.record.abstime.hr > 23) ||
		(ah_head.record.abstime.mn < 0 || ah_head.record.abstime.mn > 59) ||
		(ah_head.record.abstime.sec < 0 || ah_head.record.abstime.sec > 60)) 
  {		// file fails criteria for AH
	return(NULL);
  }
  
  	trace = calloc(1, sizeof(traceInfo));
  	trace->data.fileName = g_strdup(fileName);
	trace->data.fileType = AH_DATA;
	ret = get_ah_hedr(fp, trace);
	if (ret==PQLXSUCCESS)
		traces = g_slist_append(traces, trace);
	else
		freeData2(trace);
	rewind(fp);

  return traces;
}
  
