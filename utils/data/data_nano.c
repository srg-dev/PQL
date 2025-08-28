#include <sys/stat.h>
#include <stdio.h>

#include <stdlib.h>
#include <rpc/rpc.h>
#include "nano2ah.h"
#include "pql_defines.h"
#include "pql_externs.h"

#ifdef SOLARIS2
#include "libgen.h"
#endif

#define CASE_SENSITIVE	0

#define entry(swit,count,type,store,size) p_entry \
	("-", (swit), CASE_SENSITIVE, (count), (type), (store), (size))

#define MAX_FILES 1024

long read_xdata2(FILE *istream, char **car, ahhed *ahhead);
int read_header3(FILE *, NMXTRACE3 *);
int read_header1(FILE *, NMXTRACE1 *);
int probe_header(FILE *istream);
void convert_header3(ahhed *, NMXTRACE3 *);
void convert_header1(ahhed *, NMXTRACE1 *);

//char	*directory;
extern char	*progname;

int get_nano_hedr(FILE *fp, traceInfo *trace)
{
  int	i, type, got_header;
  NMXTRACE1 trace1;
  NMXTRACE3 trace3;
  ahhed ahhead;
  char	*chr;

  rewind(fp);
  type = probe_header(fp);
  
  if (type == EOF)
  {
	return(FAILURE);
  }
  
  if (type == 1)
    got_header = read_header1(fp, &trace1);
  else // type == 3
    got_header = read_header3(fp, &trace3);
  
  if (got_header == FALSE) 
  {
    return(FAILURE);
  }
  
  if (type == 1) convert_header1 (&ahhead, &trace1);
  else convert_header3 (&ahhead, &trace3);
  
  if (ahhead.record.type != FLOAT) 
  {
    fprintf(stderr, "Only float data supported, cannot process file %s.", trace->data.fileName);
    return(FAILURE);
  }
  
  trace->data.fileType = NANO_DATA;
  trace->data.head.units = PQL_UNITS_VOLTS;
  trace->data.head.sampInt = (double) ((int) (ahhead.record.delta * 1000000.));
  trace->data.head.data_form = MY_FLOAT;
  trace->data.head.dataSize = sizeof(float);
  trace->data.head.year = ahhead.record.abstime.yr;
  trace->data.head.day = julian (&ahhead.record.abstime);
  trace->data.head.hour = ahhead.record.abstime.hr;
  trace->data.head.minute = ahhead.record.abstime.mn;
  trace->data.head.second = (int) ahhead.record.abstime.sec;
  trace->data.head.m_secs = (ahhead.record.abstime.sec - 
			  (int) ahhead.record.abstime.sec) * 1000.;
  trace->data.head.gainConst = 1.;
  trace->data.head.scale_fac = 1.;
  
  trace->data.head.descr1 = g_strdup_printf("%i, %03i %02i:%02i" ,
        ahhead.record.abstime.yr,
	    julian (&ahhead.record.abstime),
	    ahhead.record.abstime.hr,
	    ahhead.record.abstime.mn);

    g_strchug(ahhead.station.code); CHOMP(ahhead.station.code);
    g_strchug(ahhead.station.chan); CHOMP(ahhead.station.chan);  
	trace->data.head.descr2 = g_strdup_printf("ID: %s C: %s" , ahhead.station.code, ahhead.station.chan);
	strcpy(trace->data.head.sensorID, ahhead.station.code);
	strcpy(trace->data.head.channel, ahhead.station.chan);
    
	trace->data.head.longFN = trace->data.fileName;
	trace->data.head.shortFN = 
#ifndef WIN32
				g_strdup((chr = strrchr(trace->data.fileName,'/')) ? chr+1 : trace->data.fileName);
#else
				g_strdup((chr = strrchr(trace->data.fileName,'\\')) ? chr+1 : trace->data.fileName);
#endif
    
  return SUCCESS;
}

static int read_file(FILE *fp, traceInfo *trace)
{
  NMXTRACE1 trace1;
  NMXTRACE3 trace3;
  XDR xstream;
  int type, got_header;
  long data_recv;
  char           *mkdatspace();
  char           *c_ptr;
  float          oneSample;
//  float          *t_float, oneSample;
  ahhed ahhead;

  type = probe_header(fp);
  
  if (type == EOF) 
  {
    fprintf(stderr, "%s: EOF on file %s.\n", progname, trace->data.fileName);
	return(FAILURE);
  }
  
  if (type == 0) 
  {
    fprintf(stderr, "%s: file %s not data.\n", progname, trace->data.fileName);
	return(FAILURE);
  }
  
  if (type == 1)
    got_header = read_header1(fp, &trace1);
  else
    got_header = read_header3(fp, &trace3);
  
  if (got_header == FALSE) 
  {
    fprintf(stderr, "%s: can't read file header in %s.\n", progname, trace->data.fileName);
    return(FAILURE);
  }
  
  
  if (type == 1) 
	convert_header1 (&ahhead, &trace1);
  else 
	convert_header3 (&ahhead, &trace3);
  
  if (type == 1 && trace1.ulTimeSecs == 0) 
  {
    /* fill in the epoch time while getting data	*/
    data_recv = read_xdata2(fp, &c_ptr, &ahhead);

    trace->data.dataInfo.data = c_ptr;
    
    /* fill in number of samples read here		*/
    ahhead.record.ndata = data_recv;
    
    if (data_recv == FALSE) 
	{
      fprintf(stderr,"%s: %s error reading data; ignoring.\n", progname, trace->data.fileName);
      xdr_destroy (&xstream);
      fclose (fp);
      return (FAILURE);
    }
  }
  else
    return(FAILURE);
  
  /* save the number of data points */
  trace->data.dataInfo.head.numSamples = ahhead.record.ndata;
  oneSample = 1./(1000000./trace->data.head.sampInt);
  trace->data.head.length = (double) (ahhead.record.ndata * ahhead.record.delta - oneSample);
	trace->data.dataInfo.head.data_form = trace->data.head.data_form;
	trace->data.dataInfo.head.sampInt = trace->data.head.sampInt;
	trace->data.dataInfo.head.secsPsamp = (trace->data.head.sampInt/1000000.);
  return PQLXSUCCESS;
}

void verify_dir(directory)
char *directory;
{
	int mask;
	struct stat statbuf;

	/* stat directory					*/
	if (stat (directory, &statbuf) != 0) {
		/* create if necessary				*/
		/* error if can't create 			*/
		mask = umask(022) ^ 0xffffffff;
		if (mkdir(directory, mask) != 0) {
		   fprintf(stderr, "%s: can't create directory.\n", progname);
		   exit(1);
		}
	}
	else {
		/* make sure it's a directory			*/
		if ((statbuf.st_mode & S_IFDIR) == 0) {
		   fprintf(stderr, "%s: %s not a directory.\n",
			directory, progname);
		   exit(1);
		}
	}
}

char *strip_path (path)
  char *path;
{
  char *slash;

  slash = strrchr(path, '/');

  if (slash == NULL)
    return(path);
  else
    return(slash+1);
}

int get_nano_data(traceInfo *trace)
{
	FILE *fp;

	if ((fp=fopen(trace->data.fileName, "rb")) == NULL)
	{
		return(FAILURE);
	}

	if (!trace->data.head.year)
	{	// header not yet read, go get it first
		get_nano_hedr(fp, trace);
	}
	if(read_file(fp, trace)==FAILURE)
		return(FAILURE);

	fclose(fp);
	return(PQLXSUCCESS);
}

GSList *IDnano(FILE *fp, char *fileName)
{
  int	type, got_header, ret;
  NMXTRACE1 trace1;
  NMXTRACE3 trace3;
  ahhed ahhead;
  traceInfo	*trace;
  GSList	*traces=NULL;

  rewind(fp);
  type = probe_header(fp);
  
  if (type == EOF)
  {
	return(NULL);
  }
  
  if (type == 1)
    got_header = read_header1(fp, &trace1);
  else // type == 3
    got_header = read_header3(fp, &trace3);
  
  if (got_header == FALSE) 
  {
    return(NULL);
  }
  
  if (type == 1) convert_header1 (&ahhead, &trace1);
  else convert_header3 (&ahhead, &trace3);
  
  if (ahhead.record.type != FLOAT) 
  {
    fprintf(stderr, "Only float data supported, cannot process file %s.", fileName);
    return(NULL);
  }

  	trace = calloc(1, sizeof(traceInfo));
  	trace->data.fileName = g_strdup(fileName);
    trace->data.fileType = NANO_DATA;
	ret = get_nano_hedr(fp, trace);
	if (ret==PQLXSUCCESS)
		traces = g_slist_append(traces, trace);
	else
		freeData2(trace);
	rewind(fp);
  return traces;
}
