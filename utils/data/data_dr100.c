#include <stdlib.h>
#include <string.h>
#include "pql_defines.h"
#include "pql_externs.h"
#include "dr100.h"
#include "libvfbb.h"

#ifdef SOLARIS2
#include "libgen.h"
#endif

// header locations of DR100 data, for both int and real headers
// the list here represents only those fields PQL currently cares about
#define DR_NDATABLKS	30
#define DR_LSTIDX	31
#define DR_BLKSIZE	32
#define DR_NSAMPLES	255
#define DR_YEAR		9
#define DR_JDAY		10
#define DR_HOUR		11
#define DR_MINUTE	12
#define DR_SECOND	13
#define DR_MSEC		14
#define DR_USED		15
#define DR_COMPONENTNO	254
#define DR_SERIALNO		39
#define DR_SAMPLERATE	4	
#define DR_IUNDEF	2
#define DR_DATATYPE	3

int
get_dr100_hedr(FILE *fp, traceInfo *trace)
{

/*******************************************************************************

this procedure reads in the trace->data.headr and corresponding data for each trace to be
plotted onto the main screen.  each trace->data.headr and array for data is allocated
each time the trace->data.headr and data are read in.  before reading in new trace->data.headrs and
data (via the next or prev buttons on main screen) the space used to store the
old trace->data.headrs and data is freed.

*******************************************************************************/

	int		nfile, ihdr_no=1, int_hdr[256], rhdr_no=1, error_no;
	int		nsize;
	float	rhdr[128], oneSample;
	char	*chr;
	struct ptime date;
	short	inull;
	short	dtype;

	rewind(fp);
	memset(int_hdr, 0, sizeof(int_hdr)); 
	memset(rhdr, 0, sizeof(rhdr));

	nfile = strlen(trace->data.fileName);
	if ((error_no = bbihdr(trace->data.fileName, &nfile, &ihdr_no, int_hdr)))
	{
		fprintf(stderr, "Error reading INT header for file %s, error: %d\n", 
						trace->data.fileName, error_no);
		return FAILURE;
	}

	if ((error_no=bbrhdr(trace->data.fileName, &nfile, &rhdr_no, rhdr)))
	{
		fprintf(stderr, "Error reading REAL header for file %s\n", 
						trace->data.fileName);
		return FAILURE;
	}

     inull = int_hdr[DR_IUNDEF];

  	 trace->data.head.units = PQL_UNITS_COUNTS;
     dtype = (int_hdr[DR_DATATYPE] == inull) ? 0 : int_hdr[DR_DATATYPE] ;
     switch ( dtype ) 
     {
		case 0:
     	case -2: 
			nsize    = 256;		// = (blksize/datasize)
         	trace->data.head.data_form = BIT16;
			trace->data.head.dataSize = sizeof(short);
		break;

     	case -1:
     	case -4: 
			nsize    = 128;
         		trace->data.head.data_form = BIT32;
			trace->data.head.dataSize = sizeof(int);
   		break;

     	case 1:
     	case 4: 
			nsize    = 128;
         	trace->data.head.data_form = MY_FLOAT;
			trace->data.head.dataSize = sizeof(float);
   		break;

     	default: 
       		nsize    = 0;
   		break;
     }
     trace->data.dataInfo.head.numSamples = 
         ( int_hdr[DR_NDATABLKS] == inull ) && 
         ( int_hdr[DR_LSTIDX] == inull ) ?
         int_hdr[DR_NSAMPLES] : (int_hdr[DR_NDATABLKS]-1) * nsize + int_hdr[DR_LSTIDX] ;

	trace->data.head.sampInt = 1000000/rhdr[DR_SAMPLERATE];
	oneSample = 1./(1000000./trace->data.head.sampInt);
	trace->data.head.length = (double) (trace->data.dataInfo.head.numSamples * 
			(float) (trace->data.head.sampInt/1000000.) - oneSample);

    trace->data.head.year = int_hdr[DR_YEAR];
	if (trace->data.head.year <= 99)
	{	// convert 2-char year value
		if (trace->data.head.year>=69)
			trace->data.head.year += 1900;
		else
			trace->data.head.year += 2000;
	}
        trace->data.head.day = int_hdr[DR_JDAY];
        trace->data.head.hour = int_hdr[DR_HOUR];
        trace->data.head.minute = int_hdr[DR_MINUTE];
        trace->data.head.second = int_hdr[DR_SECOND];
        trace->data.head.m_secs = int_hdr[DR_MSEC];
        trace->data.head.gainConst = 1.0;
        trace->data.head.scale_fac = 1.0;

        trace->data.head.descr1 = g_strdup_printf("%i, %03i %02i:%02i" ,
	            int_hdr[DR_YEAR],
	            int_hdr[DR_JDAY],
	            int_hdr[DR_HOUR],
	            int_hdr[DR_MINUTE]);
            
    trace->data.head.descr2 = g_strdup_printf("ID: %d C: %d" , int_hdr[DR_SERIALNO], 
    												int_hdr[DR_COMPONENTNO]);

    sprintf(trace->data.head.sensorID, "%d", int_hdr[DR_SERIALNO]);
    sprintf(trace->data.head.channel, "%d", int_hdr[DR_COMPONENTNO]);

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
//	passcal_month_day(&date, date.day);
	trace->data.head.epochs = passcal_htoe(&date, FALSE) + trace->data.head.m_secs/1000.;

        return(0);
}

int get_dr100_data(traceInfo *trace)
{

/*******************************************************************************

this procedure reads in the trace->data.headr and corresponding data for each trace to be
plotted onto the main screen.    each trace->data.headr and array for data is allocated
each time the trace->data.headr and data are read in.    before reading in new trace->data.headrs and
data (via the next or prev buttons on main screen) the space used to store the
old trace->data.headrs and data is freed.

******************************************************************************/

    int	    no_read;
	struct dr100_int_hdr int_hdr;    // DR100 Integer trace->data.headr structure
	struct dr100_real_hdr real_hdr;  // DR100 Real trace->data.headr structure
	FILE *fp;

	if ((fp=fopen(trace->data.fileName, "rb")) == NULL)
	{
		return(FAILURE);
	}

	if (!trace->data.head.year)
	{	// header not yet read, go get it first
		get_dr100_hedr(fp, trace);
		rewind(fp);
	}

	fseek(fp, sizeof(int_hdr) + sizeof(real_hdr), SEEK_SET);

	if ((trace->data.dataInfo.data = (char *)
				calloc((unsigned)trace->data.dataInfo.head.numSamples, trace->data.head.dataSize)) == NULL) 
	{
		return(FAILURE);
	}
	if ((no_read = fread(trace->data.dataInfo.data, trace->data.head.dataSize, trace->data.dataInfo.head.numSamples, fp)) 
					!= trace->data.dataInfo.head.numSamples) 
	{
		free(trace->data.dataInfo.data);
		return(FAILURE);
	}

	if (trace->data.head.endianSwap)
	{	// convert from LITTLE_ENDIAN to BIG, but only if required
		switch(trace->data.head.data_form)
		{
			case BIT32:
			case MY_FLOAT:
				from_vax_i4(trace->data.dataInfo.data, trace->data.dataInfo.data, &trace->data.dataInfo.head.numSamples);
				break;
			case BIT16:
				from_vax_i2(trace->data.dataInfo.data, trace->data.dataInfo.data, &trace->data.dataInfo.head.numSamples);
				break;
		}
	}

	fclose(fp);
	trace->data.dataInfo.head.data_form = trace->data.head.data_form;
	trace->data.dataInfo.head.sampInt = trace->data.head.sampInt;
	trace->data.dataInfo.head.secsPsamp = (trace->data.head.sampInt/1000000.);
	return(PQLXSUCCESS);
}

int check_dr100(FILE *fp, traceInfo *trace)
{
	int		ret=FAILURE, i;
	struct dr100_int_hdr vax_int_hdr;         // DR100 Integer trace->data.headr structure
	struct dr100_int_hdr int_hdr;             // DR100 Integer trace->data.headr structure
	gboolean endianSwap;

	rewind(fp);
    if (fread(&vax_int_hdr, sizeof(struct dr100_int_hdr), 1, fp) == 0) 
    {	// read INT trace->data.headr
        return(FAILURE);
    }

    // try to verify without reversing the bytes
    memcpy(&int_hdr, &vax_int_hdr, sizeof(struct dr100_int_hdr));

	for(i=0, endianSwap=FALSE;i<2;i++)
	{
		if (i)		
		{	// try again after converting from VAX to UNIX ints
			endianSwap = TRUE;
		    	from_vax_ihdr(&vax_int_hdr, &int_hdr);
	    }

    	if (!((int_hdr.year == 0 || int_hdr.year > 2100) ||
    		(int_hdr.day < 0 || int_hdr.day > 366) || 
			(int_hdr.hour < 0 || int_hdr.hour > 23) ||
			(int_hdr.minute < 0 || int_hdr.minute > 59) ||
			(int_hdr.second < 0 || int_hdr.second > 60))) 
    	{		// file is DR100
			ret = PQLXSUCCESS;
			break;
    	}
    }

    if (ret==PQLXSUCCESS)
    {
		trace->data.fileType = DR100_DATA;
		trace->data.head.endianSwap = endianSwap;
		ret = get_dr100_hedr(fp, trace);
    }

	return ret;
}

GSList *IDdr100(FILE *fp, char *fileName)
{
	int		ret=FAILURE, i;
	struct dr100_int_hdr vax_int_hdr;         // DR100 Integer trace->data.headr structure
	struct dr100_int_hdr int_hdr;             // DR100 Integer trace->data.headr structure
	gboolean endianSwap;
	traceInfo *trace;
	GSList *traces=NULL;

	rewind(fp);
    if (fread(&vax_int_hdr, sizeof(struct dr100_int_hdr), 1, fp) == 0) 
    {
    	return(NULL);
    }

    // try to verify without reversing the bytes
    memcpy(&int_hdr, &vax_int_hdr, sizeof(struct dr100_int_hdr));

	for(i=0, endianSwap=FALSE;i<2;i++)
	{
		if (i)		
		{	// try again after converting from VAX to UNIX ints
			endianSwap = TRUE;
	    	from_vax_ihdr(&vax_int_hdr, &int_hdr);
	    }

    	if (!((int_hdr.year == 0 || int_hdr.year > 2100) ||
    		(int_hdr.day < 0 || int_hdr.day > 366) || 
			(int_hdr.hour < 0 || int_hdr.hour > 23) ||
			(int_hdr.minute < 0 || int_hdr.minute > 59) ||
			(int_hdr.second < 0 || int_hdr.second > 60))) 
    	{		// file is DR100
			ret = PQLXSUCCESS;
			break;
    	}
    }

    if (ret==PQLXSUCCESS)
    {
	    	trace = calloc(1, sizeof(traceInfo));
		trace->data.fileType = DR100_DATA;
		trace->data.head.endianSwap = endianSwap;
		trace->data.fileName = g_strdup(fileName);
		ret = get_dr100_hedr(fp, trace);
		if (ret == PQLXSUCCESS)
			traces = g_slist_append(traces, trace);
		else
			freeData2(trace);
    }

	return traces;
}
