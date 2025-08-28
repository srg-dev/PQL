#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pql_defines.h"
#include "pql_externs.h"
#include "sac.h"
#define LITTLE 0
#define BIG 1

#ifdef SOLARIS2
#include "libgen.h"
#endif

#ifdef LINUX
#include <byteswap.h>
#endif

guint32 swap_4byte(guint32 nValue);

void
swapFloats (unsigned int *data, int num)
{
  int i;

#ifndef LINUX
  typedef union {
    float f;
    char c[4];
  } number ;

  number little_endian;
  number big_endian;
#endif

  for (i = 0; i < num; i++) {
#ifdef LINUX
    data[i] = bswap_32 (data[i]);
#else
    little_endian.f = (float) data[i];
    big_endian.c[0] = little_endian.c[3];
    big_endian.c[1] = little_endian.c[2];
    big_endian.c[2] = little_endian.c[1];
    big_endian.c[3] = little_endian.c[0];
    data[i] = (gint32) big_endian.f;
#endif
  }
}
	
void swapData(gint32 *ln32_dp, int npts)
{
	int	i;
	for (i=0;i<npts;i++)
	{
		ln32_dp[i] = swap_4byte(ln32_dp[i]);
	}
}

int get_sac_hedr(FILE *fp, traceInfo *trace)
{
  double real_int_epoch;
  int    m_sec;
  int    milli_sec;
  struct ptime    date;
  struct sac      sac_head;
  int             i;
  char	*chr;
  float	oneSample;
  
  rewind(fp);

  if ((fread(&sac_head, 1, sizeof(sac_head), fp)) != sizeof(sac_head)) 
  {
    return(FAILURE);
  }

  if (trace->data.head.endianSwap)
  {
	swapData((gint32 *) &sac_head, 110);
  }

  if (sac_head.delta == 0)
  {
	return(FAILURE);
  }

  trace->data.dataInfo.head.numSamples = sac_head.npts;
  trace->data.head.sampInt = (double) ((int) ((sac_head.delta+.0000001) * 1000000.));	// handle rounding errors
  oneSample = 1./(1000000./trace->data.head.sampInt);
  trace->data.head.length = (double) (sac_head.npts * sac_head.delta - oneSample);
  trace->data.head.data_form = MY_FLOAT;
  trace->data.head.dataSize = sizeof(float);
  
  switch (sac_head.idep)
  {
	  case IUNKN:
	  	  trace->data.head.units = PQL_UNITS_UNKNWN;
	  break;
	  case IDISP:
	  	  trace->data.head.units = PQL_UNITS_DISP;
	  break;
	  case IVEL:
	  	  trace->data.head.units = PQL_UNITS_VEL;
	  break;
	  case IVOLTS:
	  	  trace->data.head.units = PQL_UNITS_VOLTS;
	  break;
	  case IACC:
	  	  trace->data.head.units = PQL_UNITS_ACC;
	  break;
  }
  
  date.yr = sac_head.nzyear;
//  passcal_month_day(&date, sac_head.nzjday);	// convert jday to month/day for later use
  date.day = sac_head.nzjday;
  date.hr = sac_head.nzhour;
  date.mn = sac_head.nzmin;
  date.sec = sac_head.nzsec;
  m_sec = sac_head.nzmsec;   
  real_int_epoch = passcal_htoe(&date, FALSE) + (int)sac_head.b;	// add the seconds into the epoch time 
  
  milli_sec = (sac_head.b - (int)sac_head.b) * 1000;	// add the m_sec into the epoch time 
  m_sec += milli_sec;
  if (m_sec > 1000) 
  {		// carry the extra
    real_int_epoch += 1.0;
    m_sec -= 1000;
  }
  if (m_sec < 0)
  {
	real_int_epoch -= 1.0;
	m_sec += 1000;
  }
  passcal_etoh(&date, real_int_epoch);
  
  trace->data.head.year = date.yr;
  trace->data.head.day = date.jday;	
  trace->data.head.hour = date.hr;
  trace->data.head.minute = date.mn;
  trace->data.head.second = date.sec;
  trace->data.head.m_secs = m_sec;
  trace->data.head.epochs = real_int_epoch + trace->data.head.m_secs/1000.;
  
  trace->data.head.gainConst = 1.;
  trace->data.head.scale_fac = 1.;

    trace->data.head.descr1 = g_strdup_printf("%i %03i %02i:%02i" ,
	    (int) sac_head.nzyear,
	    (int) sac_head.nzjday,
	    (int) sac_head.nzhour,
	    (int) sac_head.nzmin);

    g_strchug(sac_head.kstnm); CHOMP(sac_head.kstnm);
    g_strchug(sac_head.kcmpnm); CHOMP(sac_head.kcmpnm);  
    g_strchug(sac_head.knetwk); CHOMP(sac_head.knetwk);
    g_strchug(sac_head.khole); CHOMP(sac_head.khole);
   	trace->data.head.descr2 = g_strdup_printf("ID: %s C: %s", sac_head.kstnm, sac_head.kcmpnm);
	strncpy(trace->data.head.sensorID, sac_head.kstnm, sizeof(trace->data.head.sensorID)-1);
	strncpy(trace->data.head.station, sac_head.kstnm, sizeof(trace->data.head.sensorID)-1);
	strncpy(trace->data.head.channel, sac_head.kcmpnm, sizeof(trace->data.head.channel)-1);
	if (strcmp(sac_head.knetwk, "-12345"))
		strncpy(trace->data.head.network, sac_head.knetwk, sizeof(trace->data.head.network)-1);
	if (strcmp(sac_head.khole, "-12345"))
		strncpy(trace->data.head.location, sac_head.khole, sizeof(trace->data.head.location)-1);

	trace->data.head.longFN = trace->data.fileName;
	trace->data.head.shortFN = 
#ifndef WIN32
				g_strdup((chr = strrchr(trace->data.fileName,'/')) ? chr+1 : trace->data.fileName);
#else
				g_strdup((chr = strrchr(trace->data.fileName,'\\')) ? chr+1 : trace->data.fileName);
#endif
	
	trace->data.head.fileHdr = calloc(1, sizeof(struct sac));
	memcpy(trace->data.head.fileHdr, &sac_head, sizeof(struct sac));

	return PQLXSUCCESS;
}

int
get_sac_data(traceInfo *trace)
{
	char	    dummy[3];
	struct sac      sac_head;
	int	npts;
	FILE *fp;

	if ((fp=fopen(trace->data.fileName, "rb")) == NULL)
	{
		return(FAILURE);
	}

	if ((fread(&sac_head, 1, sizeof(sac_head), fp)) != sizeof(sac_head)) 
	{
		fprintf(stderr, "%s:\tProblem reading header, cannot process.\n", trace->data.head.longFN);
    	return(FAILURE);
	}

	if (trace->data.head.endianSwap)
	{
		swapData((gint32 *) &sac_head, 110);
	}

	if (sac_head.iftype != ITIME) 
	{
    	free(trace->data.dataInfo.data);
		fprintf(stderr, "%s:\tOnly FLOAT data type supported.\n", trace->data.head.longFN);
    	return (FAILURE);
	}
  
  if ((trace->data.dataInfo.data = (char *) calloc((unsigned) trace->data.dataInfo.head.numSamples, 
  													trace->data.head.dataSize)) == NULL) 
  {
	fprintf(stderr, "%s:\tUnable to allocate memory, exiting.\n", 
				trace->data.head.longFN);
	exit(FAILURE);
  }
  
  if ((npts = fread(trace->data.dataInfo.data, trace->data.head.dataSize, trace->data.dataInfo.head.numSamples, fp)) != 
				  trace->data.dataInfo.head.numSamples) 
  {	
    fprintf(stderr, "%s:\tRead %d points (Expected %d)\n", 
				trace->data.head.longFN, npts, sac_head.npts);
	trace->data.dataInfo.head.numSamples = npts;
  }
  
  if (fread(dummy, 1, 2, fp) != 0) 
  {
	fprintf(stderr, "%s:\tUnread data points in file, possible problem with header.\n", 
				trace->data.head.longFN);
  }

  if (trace->data.head.endianSwap)
  {
//undone: figure out how to decide when SAC files should have data bytes swapped
// it is not always true that if the header requires swapping then the data does as well
// until we know how to do this, we will assume that if the header requires swapping so does the data!
	swapData((gint32 *) trace->data.dataInfo.data, trace->data.dataInfo.head.numSamples);
  }

  trace->data.dataInfo.head.data_form = trace->data.head.data_form;
  trace->data.dataInfo.head.sampInt = trace->data.head.sampInt;
  trace->data.dataInfo.head.secsPsamp = (trace->data.head.sampInt/1000000.);

  fclose(fp);
  return (PQLXSUCCESS);
}

static int check_endian(struct sac *sac_head)
{
	int ret = FAILURE;
	
  do {
	  if (!(sac_head->nzyear < 0 ||
  		sac_head->nzyear < 100 ||
  		sac_head->nzjday < 0 ||
  		sac_head->nzjday > 366 ||
  		sac_head->nzhour < 0 ||
  		sac_head->nzhour > 23 ||
  		sac_head->nzmin < 0 ||
  		sac_head->nzmin > 59 ||
  		sac_head->nzsec < 0 ||
  		sac_head->nzsec > 59 ||
  		sac_head->nzmsec < 0 ||
  		sac_head->nzmsec > 999) && (sac_head->unused16==-12345)) 
	  {
		 ret = FALSE;
		 break;
	  }
  
	sac_head->nzyear = swap_4byte(sac_head->nzyear);
	sac_head->nzjday = swap_4byte(sac_head->nzjday);
	sac_head->nzhour = swap_4byte(sac_head->nzhour);
	sac_head->nzmin = swap_4byte(sac_head->nzmin);
	sac_head->nzsec = swap_4byte(sac_head->nzsec);
	sac_head->nzmsec = swap_4byte(sac_head->nzmsec);
	sac_head->unused16 = swap_4byte(sac_head->unused16);

	  if (!(sac_head->nzyear < 0 ||
  		sac_head->nzyear < 100 ||
  		sac_head->nzjday < 0 ||
  		sac_head->nzjday > 366 ||
  		sac_head->nzhour < 0 ||
  		sac_head->nzhour > 23 ||
  		sac_head->nzmin < 0 ||
  		sac_head->nzmin > 59 ||
  		sac_head->nzsec < 0 ||
  		sac_head->nzsec > 59 ||
  		sac_head->nzmsec < 0 ||
  		sac_head->nzmsec > 999) && (sac_head->unused16==-12345)) 
	  {
		 ret = TRUE;
	  }
	  break;
  } while(1);
  return ret;
}
  
int	check_sac(FILE *fp, traceInfo *trace)
{
  int			ret = FAILURE;
  int			endianSwap;
  struct sac    sac_head;

  rewind(fp);

  if ((fread(&sac_head, 1, sizeof(sac_head), fp)) != sizeof(sac_head)) 
  {
    return(FAILURE);
  }
  
  ret = endianSwap = check_endian(&sac_head);
  
  if (endianSwap != FAILURE)
  {	// file is SAC
	trace->data.fileType = SAC_DATA;
	trace->data.head.endianSwap = endianSwap;
	ret = get_sac_hedr(fp, trace);
	rewind(fp);
  }

  return(ret);
}

GSList *IDsac(FILE *fp, char *fileName)
{
  int			ret = FAILURE, endianSwap;
  struct sac	sac_head;
  traceInfo		*trace;
  GSList		*traces=NULL;

  rewind(fp);

  if ((fread(&sac_head, 1, sizeof(sac_head), fp)) != sizeof(sac_head)) 
  {
  	return(NULL);
  }

   ret = endianSwap = check_endian(&sac_head);
  
  if (endianSwap != FAILURE)
  {
  	trace = calloc(1, sizeof(traceInfo));
  	trace->data.fileName = g_strdup(fileName);
	trace->data.fileType = SAC_DATA;
	trace->data.head.endianSwap = ret;
	ret = get_sac_hedr(fp, trace);
	if (ret==PQLXSUCCESS)
		traces = g_slist_append(traces, trace);
	else
		freeData2(trace);
	rewind(fp);
  }

  return traces;
}
