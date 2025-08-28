#include <glib.h>

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "gdbm.h"
#include "pqlx_defs.h"
#include "pqlx_errors.h"
#include "dbl_iface.h"
#include "dblinc.h"

#ifndef WIN32
static const char * WORK_DIR = ".pqlg";
#else
static const char * WORK_DIR = ".pql";
#endif

/*******************************************************************************
    Utility functions
*******************************************************************************/

static char * get_db_name_g(int dbfile)
{
    static char db_name[NLTABLES][1024];
	gchar	*filename=NULL;
	char	*home_dir;

    if (db_name[dbfile][0] != '/')
    {
		switch(dbfile)
		{
			case DEFAULTST:
				filename = g_strdup(DFAULT_DB);
			break;
			case FILTERST:
				filename = g_strdup(FILTER_DB);
			break;
#ifndef PQL_ONLY
//			case PDFST:
//				filename = g_strdup(PDF_DB);
//			break;
			case PDFDFAULTST:
				filename = g_strdup(PDF_DB);
			break;
			case PDFSRVRST:
				filename = g_strdup(PDFSRVR_DB);
			break;
			case STNCHNFLTRST:
				filename = g_strdup(STN_CHFF_DB);
			break;
			case STNDFAULTST:
				filename = g_strdup(STN_DB);
			break;
			case STNCHNYSCALE:
				filename = g_strdup(STN_CHNY_DB);
			break;
			case STNGRPYSCALE:
				filename = g_strdup(STN_GRPY_DB);
			break;
#endif
		}

        // find home directory
        home_dir = g_strdup(g_get_home_dir());

        // create a directory
        snprintf(db_name[dbfile],1024,"%s/%s",home_dir,WORK_DIR);
#ifdef WIN32
        mkdir(db_name[dbfile]);
#else
        mkdir(db_name[dbfile],0777);
#endif

        // generate the file name
        snprintf(db_name[dbfile],1024,"%s/%s/%s",home_dir,WORK_DIR,filename);
		free(filename);
		free(home_dir);
    }

    return db_name[dbfile];
}


int remove_record_g(int dbfile, const char *key)
{
    GDBM_FILE dbf;
	datum data;
	int ret=PQLXSUCCESS;

    if ((dbf=gdbm_open(get_db_name_g(dbfile), 512, GDBM_WRITER, 0666, 0)) == NULL)
    {
        return FAILURE;
    }
    
    // remove record from database
	data.dptr = g_strdup(key);
	data.dsize = strlen(key);
    if ((ret=gdbm_delete(dbf,data)) != 0)
	{
		fprintf(stderr, "Unable to remove key %s from the local data store.\n", key);
		fprintf(stderr, "\tGDBM Error (code=%d): %s\n", gdbm_errno, gdbm_strerror(gdbm_errno));
		ret = FAILURE;
	}
    
	free(data.dptr);
    gdbm_close(dbf);
	return ret;
}

int get_record_g(int dbfile, void *record)
{
    GDBM_FILE dbf;
	datum	key, data;
	int ret=PQLXSUCCESS;
	char	*keyName=NULL;

    switch(dbfile)
    {
		case DEFAULTST:
#ifndef PQL_ONLY
		case STNDFAULTST:
		case PDFDFAULTST:
#endif
		{
			DEFAULTS_REC *rec = record;
			keyName = g_strdup(rec->name);
			strcpy(rec->value, DBL_NO_RECORD);
		}
		break;
	}

    if ((dbf=gdbm_open(get_db_name_g(dbfile), 512, GDBM_READER, 0666, 0)) == NULL)
    {
        return FAILURE;
    }
    
	key.dptr = g_strdup(keyName);
	key.dsize = strlen(keyName);
	free(keyName);
	data = gdbm_fetch(dbf, key);

	if (data.dptr)
	{
		switch(dbfile)
		{
			case DEFAULTST:
#ifndef PQL_ONLY
			case STNDFAULTST:
			case PDFDFAULTST:
#endif
			{
				DEFAULTS_REC *rec = record;
				memset(rec->value, 0, sizeof(rec->value));
				strncpy(rec->value, data.dptr, data.dsize);
			}
			break;
#if 0	
			case FILTERST:
			{
				FILTER_REC *rec = record;
				memset(rec, 0, sizeof(FILTER_REC));
				strncpy(rec->name, key.dptr, key.dsize);
				sscanf(data.dptr, "%d %f %d %f", 
							&rec->poles[HIGHF], &rec->cutoff[HIGHF],
							&rec->poles[LOWF], &rec->cutoff[LOWF]);
			}
			break;

#ifndef PQL_ONLY
			case PDFST:
			break;

			case PDFSRVRST:
			{
				PDFSRVR_REC *rec;
				dblrecs->recs[i] = rec = malloc(sizeof(PDFSRVR_REC));
				memset(rec, 0, sizeof(PDFSRVR_REC));
				rec->name = g_strndup(key.dptr, key.dsize);
			}
			break;

			case STNCHNFLTRST:
			{
				STN_CHN_FILTER *rec;
				dblrecs->recs[i] = rec = malloc(sizeof(STN_CHN_FILTER));
				memset(rec, 0, sizeof(STN_CHN_FILTER));
				rec->name = g_strndup(key.dptr, key.dsize);
			}
			break;
#endif
#endif
		}
		free(data.dptr);
	}
    
	free(key.dptr);
    gdbm_close(dbf);
	return ret;
}

int get_all_records_g(int dbfile, DBLRECORDS *dblrecs)
{
    GDBM_FILE dbf;
	datum	key, nextkey, data;
	int		i, ret=PQLXSUCCESS;

    if ((dbf=gdbm_open(get_db_name_g(dbfile), 512, GDBM_READER, 0666, 0)) == NULL)
    {
        return FAILURE;
    }

	// count how many total records first
	for(	i=0,key=gdbm_firstkey(dbf);			// get the first record
			key.dptr;							// until no more left
			nextkey=gdbm_nextkey(dbf,key),free(key.dptr),key=nextkey,i++);	// advance all
	dblrecs->numRecs = i;
	if((dblrecs->recs = calloc(dblrecs->numRecs, sizeof(void *)))==NULL)
	{
		fprintf(stderr, "Out of Memory, cannot continue.\n");
		exit(-1);
	}

	for(i=0,key=gdbm_firstkey(dbf);
		key.dptr;
		nextkey=gdbm_nextkey(dbf,key),free(key.dptr),key=nextkey,i++)
	{
		data=gdbm_fetch(dbf, key);
		switch(dbfile)
		{
			case DEFAULTST:
#ifndef PQL_ONLY
			case STNDFAULTST:
			case PDFDFAULTST:
#endif
			{
				DEFAULTS_REC *rec;
				dblrecs->recs[i] = calloc(1, sizeof(DEFAULTS_REC));
				rec = dblrecs->recs[i];
				strncpy(rec->name, key.dptr, key.dsize);
				strncpy(rec->value, data.dptr, data.dsize);
			}
			break;
	
			case FILTERST:
			{
				char *string;
				FILTER_REC *rec;
				dblrecs->recs[i] = rec = calloc(1, sizeof(FILTER_REC));
				strncpy(rec->name, key.dptr, key.dsize);
				string = g_strndup(data.dptr, data.dsize);
				sscanf(string, "%d %f %d %f", 
							&rec->poles[HIGHF], &rec->cutoff[HIGHF],
							&rec->poles[LOWF], &rec->cutoff[LOWF]);
				free(string);
			}
			break;

#ifndef PQL_ONLY
			case PDFST:
			break;

			case PDFSRVRST:
			{
				PDFSRVR_REC *rec;
				dblrecs->recs[i] = rec = calloc(1, sizeof(PDFSRVR_REC));
				rec->name = g_strndup(key.dptr, key.dsize);
			}
			break;

			case STNCHNFLTRST:
			{
				STN_CHN_FILTER *rec;
				dblrecs->recs[i] = rec = calloc(1, sizeof(STN_CHN_FILTER));
				rec->name = g_strndup(key.dptr, key.dsize);
			}
			break;

			case STNCHNYSCALE:
			{
				char	*string, loc[STNCHNFLEN], chn[STNCHNFLEN], scale[2][STNCHNFLEN];
				STN_CHN_YSCALE *rec;
				memset(scale, 0, sizeof(scale));
				dblrecs->recs[i] = rec = calloc(1, sizeof(STN_CHN_YSCALE));
				rec->name = g_strndup(key.dptr, key.dsize);
				string = g_strndup(data.dptr, data.dsize);
				sscanf(string, "%s %s %s %s", loc, chn, scale[MAX], scale[MIN]);
				free(string);
				rec->loc = g_strdup(loc);
				rec->chn = g_strdup(chn);
				rec->scale[MAX] = g_strdup(scale[MAX]);	if (!strcmp(rec->scale[MAX], BLANK)) rec->scale[MAX] = g_strdup("");
				rec->scale[MIN] = g_strdup(scale[MIN]);	if (!strcmp(rec->scale[MIN], BLANK)) rec->scale[MIN] = g_strdup("");
			}
			break;
			
			case STNGRPYSCALE:
			{
				char *strPtr, name[STNCHNFLEN], *nm, *string;
				int	 numScales=1, j;
				STN_GRP_YSCALE	*rec;
				dblrecs->recs[i] = rec = calloc(1, sizeof(STN_GRP_YSCALE));
				rec->name = g_strndup(key.dptr, key.dsize);
				string = g_strndup(data.dptr, data.dsize);
				for(j=0;j<strlen(string);j++)
					if (!strncmp(&data.dptr[j], DBL_SEPARATOR, strlen(DBL_SEPARATOR)))
						numScales ++;
				
				strPtr = string;		
				for(j=0;j<numScales;j++)
				{
					sscanf(strPtr, "%[^%]s", name);
					nm = g_strdup(name);
					rec->chns = g_slist_append(rec->chns, nm);
					strPtr += strlen(name) + strlen(DBL_SEPARATOR);
				}
				free(string);
			}
			break;
#endif
		}
		free(data.dptr);
    }
    
    gdbm_close(dbf);
	return ret;
}

int add_record_g(int dbfile, void *record)
{
    GDBM_FILE dbf;
	datum	key, data;
	int	ret=PQLXSUCCESS;

	memset(&key, 0, sizeof(datum));
	memset(&data, 0, sizeof(datum));
    if ((dbf=gdbm_open(get_db_name_g(dbfile), 512, GDBM_WRITER, 0666, 0)) == NULL)
    {
        return FAILURE;
    }

	switch(dbfile)
	{
		case DEFAULTST:
#ifndef PQL_ONLY
		case STNDFAULTST:
		case PDFDFAULTST:
#endif
		{
			DEFAULTS_REC *rec = record;
			key.dptr = g_strdup(rec->name);
			key.dsize = strlen(rec->name);
			data.dptr = g_strdup(rec->value);
			data.dsize = strlen(rec->value);
		}
		break;
	
		case FILTERST:
		{
			FILTER_REC *rec = record;
			key.dptr = g_strdup(rec->name);
			key.dsize = strlen(rec->name);
			data.dptr = g_strdup_printf("%d %f %d %f", 
							rec->poles[HIGHF], rec->cutoff[HIGHF],
							rec->poles[LOWF], rec->cutoff[LOWF]);
			data.dsize = strlen(data.dptr);
		}
		break;

#ifndef PQL_ONLY
		case PDFST:
		break;

		case PDFSRVRST:
		{
			PDFSRVR_REC *rec = record;
			key.dptr = g_strdup(rec->name);
			key.dsize = strlen(rec->name);
			data.dptr = g_strdup(rec->name);
			data.dsize = strlen(rec->name);
		}
		break;
		
		case STNCHNFLTRST:
		{
			STN_CHN_FILTER *rec = record;
			key.dptr = g_strdup(rec->name);
			key.dsize = strlen(rec->name);
			data.dptr = g_strdup(rec->name);
			data.dsize = strlen(rec->name);
		}
		break;
		
		case STNCHNYSCALE:
		{
			STN_CHN_YSCALE *rec = record;
			key.dptr = g_strdup(rec->name);
			key.dsize = strlen(rec->name);
			data.dptr = g_strdup_printf("%s %s %s %s", rec->loc, rec->chn, rec->scale[MAX], rec->scale[MIN]);
			data.dsize = strlen(data.dptr);
		}
		break;
			
		case STNGRPYSCALE:
		{
			char	chnKeyW[1024], *chnIns, *chnWPtr;
			GSList	*iter;
			STN_GRP_YSCALE *rec = record;
			key.dptr = g_strdup(rec->name);
			key.dsize = strlen(rec->name);

			iter = rec->chns;
			chnIns = g_strdup_printf("%s", (char *) iter->data);
			chnWPtr = g_stpcpy(chnKeyW, chnIns);
			free(chnIns);
			for(iter=g_slist_next(iter);iter;iter=g_slist_next(iter))
			{
				chnIns = g_strdup_printf("%s%s", DBL_SEPARATOR, (char *) iter->data);
				chnWPtr = g_stpcpy(chnWPtr, chnIns);
				free(chnIns);
			}
			data.dptr = g_strdup(chnKeyW);
			data.dsize = strlen(data.dptr);
		}
		break;
#endif
	}

    // write record to database
	ret = gdbm_store(dbf, key, data, GDBM_REPLACE);
	if (ret != GDBM_NO_ERROR)
	{
		fprintf(stderr, "Unable to add key '%s' to the local data store.\n", key.dptr);
		fprintf(stderr, "\tGDBM Error (code=%d): %s\n", gdbm_errno, gdbm_strerror(gdbm_errno));
		ret = FAILURE;
	}
    
	free(key.dptr);
	free(data.dptr);
    gdbm_close(dbf);
	return ret;
}

int create_dbl_g(int dbfile)
{
    // create database
    GDBM_FILE dbf;
	int ret=PQLXSUCCESS;

    if ((dbf=gdbm_open(get_db_name_g(dbfile), 512, GDBM_WRCREAT, 0666, 0)) == NULL)
   	{
   	    return FAILURE;
   	}
    
    // close database
    gdbm_close(dbf);
	return ret;
}
