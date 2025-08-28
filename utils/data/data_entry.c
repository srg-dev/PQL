#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>           // stat(), etc
#include <dirent.h>
#include "pql_defines.h"
#include "pql_externs.h"
#include "reftek.h"

#define FILEMB			120
#define FILETOOLARGE	(FILEMB * 1024 * 1024)

int	check_segy(FILE *, traceInfo *), check_sac(FILE *, traceInfo *), 
	check_mseed(FILE *, traceInfo *), check_dr100(FILE *, traceInfo *),
	get_ah_hedr(FILE *, traceInfo *), get_nano_hedr(FILE *, traceInfo*);
int get_dr100_data(traceInfo *), get_segy_data(traceInfo *), get_mseed_data(traceInfo *),
	get_sac_data(traceInfo *), get_ah_data(traceInfo *), get_nano_data(traceInfo *),
	get_rt130_data(traceInfo *), get_rt125A_data(traceInfo *), get_rt125_data(traceInfo *),
	get_cwb_data(traceInfo *);
gboolean *IDmseed(FILE*, char*, MPLX_MSEED*, GSList**);
GSList	*IDsac(FILE*,char*), *IDnano(FILE*,char*),
		*IDdr100(FILE*,char*), *IDsegy(FILE*,char*), *IDah(FILE*,char*),
		*IDrt130(FILE*,char*), *IDrt125A(FILE*,char*), *IDrt125(FILE*,char*);
GSList	*get_rt125_SOH(FILE *), *get_rt125A_SOH(FILE *), *get_rt130_SOH(FILE *);
void trace_excerpt(traceInfo *from, traceInfo *to);

void freeData2(traceInfo *trc)
{
	GSList		*iter;
	OVERLAP		*olapData;

	if (trc->data.fileName)
		free(trc->data.fileName);
	if (trc->data.head.shortFN)
		free(trc->data.head.shortFN);
	if (trc->data.head.descr1)
		free(trc->data.head.descr1);
	if (trc->data.head.descr2)
		free(trc->data.head.descr2);
	if (trc->data.dataInfo.data)			// data array
		free(trc->data.dataInfo.data);
	trc->data.dataInfo.data = NULL;
	if (trc->data.head.fileHdr)
		free(trc->data.head.fileHdr);
	freeFiltData(trc);
	if (trc->trace.gaps)					// gap info
	{
		free(trc->trace.gaps);
		trc->trace.gaps = NULL;
	}
	if (trc->trace.overlaps)				// overlap info
	{
		free(trc->trace.overlaps);
		trc->trace.overlaps = NULL;
	}
	if (trc->trace.picks)					// pick info
	{
		free(trc->trace.picks);
		trc->trace.picks = NULL;
	}
	for(iter=trc->trace.olapData; iter; iter=g_slist_next(iter))
	{
		olapData = iter->data;
		free(olapData->data);
		free(olapData);
	}
	if (trc->trace.olapData)
	{
		g_slist_free(trc->trace.olapData);
		trc->trace.olapData = NULL;
	}
	// transform data elements
	if (trc->data.tForm.data)
		free(trc->data.tForm.data);
	if (trc->data.tForm.tFormStr)
		free(trc->data.tForm.tFormStr);
	memset(&trc->data.tForm, 0, sizeof(DATAINFO));
	
	memset(trc, 0, sizeof(traceInfo));
}

int checkTrace(traceInfo *trace)
{
	FILE *fp;

	if ((fp=fopen(trace->data.fileName, "rb")) == NULL)
	{
		return(FILE_DNE);
	}

//case MSEED_DATA:
	if ((check_mseed(fp, trace))!= -1) 
	{
		fclose(fp);
		return PQLXSUCCESS;
	}

//	case SAC_DATA: 
	if ((check_sac(fp, trace))!= -1) 
	{
		fclose(fp);
		return PQLXSUCCESS;
	}

//case SEGY_DATA:
	if((check_segy(fp, trace)) != -1) 
	{
		fclose(fp);
		return PQLXSUCCESS;
	}

#ifndef WIN32
//case AH_DATA:
	if ((get_ah_hedr(fp, trace))!= -1) 
	{
		fclose(fp);
		return PQLXSUCCESS;
	}

//case NANO_DATA:
	if ((get_nano_hedr(fp, trace))!= -1) 
	{
		fclose(fp);
		return PQLXSUCCESS;
	}

//	case DR100_DATA:	
	if((check_dr100(fp, trace)) != -1) 
	{
		fclose(fp);
		return PQLXSUCCESS;
	}
#endif

	fclose(fp);
	return (FAILURE);
}

int readTrace(traceInfo *trace)
{
	int ret=0;
	struct stat	file_stat;

	// call function to read data, but only if not already done
	// i.e., a read of data can happen elsewhere
	if (trace->data.dataInfo.data)
		return (PQLXSUCCESS);
	
    // stat() the entry to confirm its existence and size
    if (trace->data.fileType != CWB_DATA &&
    	stat(trace->data.fileName, &file_stat) == -1) 
	{	// if we can't stat the file, then we can't read it either, move on...
	    	return(FILE_DNE);
    }
	if (file_stat.st_size > (FILETOOLARGE))
	{	// file is > (FILETOOLARGE)Mb, i.e., of suspect quality and potentially damaging, so we reject it
		return(FILE_TOO_LARGE);
	}

	if (!trace->data.head.year &&
		trace->data.fileType != CWB_DATA)
	{	// header not yet read, go get it first
		ret = checkTrace(trace);
	}
	
	if (ret == FAILURE)
		return ret;
		
	switch(trace->data.fileType)
	{  
      case SEGY_DATA:
		ret = get_segy_data(trace);
		break;
	  
#ifndef WIN32
      case DR100_DATA:
		ret = get_dr100_data(trace);
		break;
	
      case AH_DATA:
		ret = get_ah_data(trace);
		break;

      case NANO_DATA:
		ret = get_nano_data(trace);
		break;
#endif

      case SAC_DATA:
		ret = get_sac_data(trace);
		break;
	  
      case MSEED_DATA:
      case MSEED_MPLEX_DATA:
		ret = get_mseed_data(trace);
		break;

	  case RT130_DATA:
		ret = get_rt130_data(trace);
		break;

	  case RT125A_DATA:
		ret = get_rt125A_data(trace);
		break;

	  case RT125_DATA:
		ret = get_rt125_data(trace);
		break;

#ifndef PQL_ONLY		
	  case CWB_DATA:
	  	ret = get_cwb_data(trace);
	  	break;
#endif
	}

	if (ret == PQLXSUCCESS &&
		trace->excerpt.epochS &&
		trace->excerpt.length &&
		trace->data.fileType != CWB_DATA)
	{
		trace_excerpt(trace, trace);
	}
	
	return ret;
}

GSList *identifyTrace(char *fileName, MPLX_MSEED *chnOptions)
{
	FILE *fp;
	GSList	*traces=NULL;

	if ((fp=fopen(fileName, "rb")) == NULL)
	{
		return((GSList *)FILE_DNE);
	}

//case MSEED_DATA:
	if ((IDmseed(fp, fileName, chnOptions, &traces))) 
	{
		fclose(fp);
		if (!traces)
			return ((GSList *) NO_CHANS);
		else
			return traces;
	}
//	case SAC_DATA: 
	if ((traces=IDsac(fp, fileName))) 
	{
		fclose(fp);
		return traces;
	}

//case SEGY_DATA:
	if((traces=IDsegy(fp, fileName))) 
	{
		fclose(fp);
		return traces;
	}

#ifndef WIN32
//case AH_DATA:
	if ((traces=IDah(fp, fileName))) 
	{
		fclose(fp);
		return traces;
	}

//case NANO_DATA:
	if ((traces=IDnano(fp, fileName))) 
	{
		fclose(fp);
		return traces;
	}

//	case DR100_DATA:	
	if((traces=IDdr100(fp, fileName))) 
	{
		fclose(fp);
		return traces;
	}
#endif

//case RT130_DATA:
	if ((traces=IDrt130(fp, fileName))) 
	{
		fclose(fp);
		return traces;
	}

//case RT125A_DATA:
	if ((traces=IDrt125A(fp, fileName))) 
	{
		fclose(fp);
		return traces;
	}

//case RT125_DATA:		// 125A format check must come before 125 format check!
	if ((traces=IDrt125(fp, fileName))) 
	{
		fclose(fp);
		return traces;
	}

	fclose(fp);
	return (NULL);
}

enum {
	FTDIR,
	FTFILE
};

static int fileType(char *name)
{	// is the input a file or a directory?
	// can't use extended field d_type from dirent.h, doesn't exist on SOLARIS
	DIR		*dp;
	int 	ret = FTFILE;
	
	if (!((dp=opendir(name))==NULL))
	{	// succeeded, it's a directory
		ret = FTDIR;
		closedir(dp);
	}

	return ret;
}

static char * getSOHFile(char *sohDir)
{
	DIR		*dp;
	struct dirent *ep;
	char	*sohFile=NULL;

	if ((dp=opendir(sohDir))==NULL)
	{	// failed
		return sohFile;
	}
	
	while((ep=readdir(dp)))
	{
		if (!strcmp(ep->d_name,".") || 
			!strcmp(ep->d_name, ".."))
			continue;	// don't read '.' and '..' directories

#ifdef WIN32
		sohFile = g_strdup_printf("%s\\%s", sohDir, ep->d_name);	
#else
		sohFile = g_strdup_printf("%s/%s", sohDir, ep->d_name);	
#endif
		switch(fileType(sohFile))
		{
			case FTDIR:		// it's another directory, ignore
				free(sohFile);
				sohFile = NULL;
				continue;
			break;
			case FTFILE:	// found a file, assume it's the one we want
				break;
			break;
		}
	}

	closedir(dp);
	return sohFile;
}

GSList *readInstSOH(char *fileName, int fileType)
{
	GSList	*SOHList=NULL;
	FILE *fp=NULL;
	
	switch(fileType)
	{
		case RT125_DATA:
			if ((fp=fopen(fileName, "rb")) == NULL)
			{
				return((GSList *)FILE_DNE);
			}
			SOHList = get_rt125_SOH(fp);
		break;
		case RT125A_DATA:
			if ((fp=fopen(fileName, "rb")) == NULL)
			{
				return((GSList *)FILE_DNE);
			}
			SOHList = get_rt125A_SOH(fp);
		break;

		case RT130_DATA:
		{
			char	*sohFile, *ptr, *sohDir;
#ifdef WIN32
			ptr = strrchr(fileName, '\\');
#else
			ptr = strrchr(fileName, '/');
#endif
			if (ptr)
			{
				ptr--;	*ptr = 0;
			}
			sohDir = g_strdup_printf("%s0", fileName);
			sohFile = getSOHFile(sohDir);
			if (!sohFile ||
				((fp=fopen(sohFile, "rb")) == NULL))
			{
				if (sohFile) free(sohFile);
				return((GSList *)FILE_DNE);
			}
			SOHList = get_rt130_SOH(fp);
			
			free(sohFile);
		}
		break;
	}
	
	fclose(fp);
	return(SOHList);
}
