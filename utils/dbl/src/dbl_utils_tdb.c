#include <glib.h>

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "tdb.h"
#include "pqlx_defs.h"
#include "pqlx_errors.h"
#include "dbl_iface.h"
#include "dblinc.h"

static const char * WORK_DIR = ".pql";

/*******************************************************************************
    Utility functions
*******************************************************************************/
#ifdef NOCOMP
static void strupper(char * s)
{
    while (*s)
    {
        *s = toupper(*s);
        ++s;
    }
}
#endif

static char *get_db_name(int dbfile)
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

static int	testFile(char *dbFN)
{
	int	ret;
	switch (g_file_test(dbFN, G_FILE_TEST_EXISTS))
	{
		case TRUE:
			ret = FAILURE;
		break;
		case FALSE:
			ret = FILEDNE;
		break;
	}
	return ret;
}

int remove_record(int dbfile, const char *key)
{
    TDB_CONTEXT *dbf;
	TDB_DATA data;
	int ret=PQLXSUCCESS;
	char	*dbFN = get_db_name(dbfile);

    if ((dbf=tdb_open(dbFN, 0, TDB_NOLOCK, O_RDWR, 0644)) == NULL)
    {
		return (testFile(dbFN));
    }
    
    // remove record from database
	data.dptr = (unsigned char *) g_strdup(key);
	data.dsize = strlen(key);
    if ((ret=tdb_delete(dbf,data)) != 0)
	{
		fprintf(stderr, "Unable to remove key %s from the local data store.\n", key);
		fprintf(stderr, "\tTDB Error (code=%d): %s\n", tdb_error(dbf), tdb_errorstr(dbf));
		ret = FAILURE;
	}
    
	free(data.dptr);
    tdb_close(dbf);
	return ret;
}

int get_record(int dbfile, void *record)
{
    TDB_CONTEXT *dbf;
	TDB_DATA	key, data;
	int ret=PQLXSUCCESS;
	char	*keyName=NULL;
	const	char *str;
	char	*dbFN = get_db_name(dbfile);

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

    if ((dbf=tdb_open(dbFN, 0, TDB_NOLOCK, O_RDWR, 0644)) == NULL)
    {
		return (testFile(dbFN));
    }
    
	key.dptr = (unsigned char *) g_strdup(keyName);
	key.dsize = strlen(keyName);
	free(keyName);
	data = tdb_fetch(dbf, key);

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
				str = (char *) data.dptr;	// handle compiler warning
				strncpy(rec->value, str, data.dsize);
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
    tdb_close(dbf);
	return ret;
}

int get_all_records(int dbfile, DBLRECORDS *dblrecs)
{
    TDB_CONTEXT *dbf;
	TDB_DATA	key, nextkey, data;
	int		i, ret=PQLXSUCCESS;
	const	char *str;
	char	*dbFN = get_db_name(dbfile);

    if ((dbf=tdb_open(dbFN, 0, TDB_NOLOCK, O_RDWR, 0644)) == NULL)
    {
		return (testFile(dbFN));
    }

	// count how many total records first
	for(	i=0,key=tdb_firstkey(dbf);			// get the first record
			key.dptr;							// until no more left
			nextkey=tdb_nextkey(dbf,key),free(key.dptr),key=nextkey,i++);	// advance all
	dblrecs->numRecs = i;
	if((dblrecs->recs = calloc(dblrecs->numRecs, sizeof(void *)))==NULL)
	{
		fprintf(stderr, "Out of Memory, cannot continue.\n");
		exit(-1);
	}

	for(i=0,key=tdb_firstkey(dbf);
		key.dptr;
		nextkey=tdb_nextkey(dbf,key),free(key.dptr),key=nextkey,i++)
	{
		data=tdb_fetch(dbf, key);
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
				str = (char *) key.dptr;	// handle compiler warning
				strncpy(rec->name, str, key.dsize);
				str = (char *) data.dptr;	// handle compiler warning
				strncpy(rec->value, str, data.dsize);
			}
			break;
	
			case FILTERST:
			{
				char *string;
				FILTER_REC *rec;
				dblrecs->recs[i] = rec = calloc(1, sizeof(FILTER_REC));
				str = (char *) key.dptr;	// handle compiler warning
				strncpy(rec->name, str, key.dsize);
				str = (char *) data.dptr;	// handle compiler warning
				string = g_strndup(str, data.dsize);
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
				str = (char *) key.dptr;	// handle compiler warning
				rec->name = g_strndup(str, key.dsize);
			}
			break;

			case STNCHNFLTRST:
			{
				STN_CHN_FILTER *rec;
				dblrecs->recs[i] = rec = calloc(1, sizeof(STN_CHN_FILTER));
				str = (char *) key.dptr;	// handle compiler warning
				rec->name = g_strndup(str, key.dsize);
			}
			break;

			case STNCHNYSCALE:
			{
				char	*string, loc[STNCHNFLEN], chn[STNCHNFLEN], scale[2][STNCHNFLEN];
				STN_CHN_YSCALE *rec;
				memset(scale, 0, sizeof(scale));
				dblrecs->recs[i] = rec = calloc(1, sizeof(STN_CHN_YSCALE));
				str = (char *) key.dptr;	// handle compiler warning
				rec->name = g_strndup(str, key.dsize);
				str = (char *) data.dptr;	// handle compiler warning
				string = g_strndup(str, data.dsize);
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
				str = (char *) key.dptr;	// handle compiler warning
				rec->name = g_strndup(str, key.dsize);
				str = (char *) data.dptr;	// handle compiler warning
				string = g_strndup(str, data.dsize);
				for(j=0;j<strlen(string);j++)
				{
					str = (char *) &data.dptr[j];	// handle compiler warning
					if (!strncmp(str, DBL_SEPARATOR, strlen(DBL_SEPARATOR)))
						numScales ++;
				}
				
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
		IFFREE(data.dptr);
    }
    
    tdb_close(dbf);
	return ret;
}

int add_record(int dbfile, void *record)
{
    TDB_CONTEXT *dbf;
	TDB_DATA	key, data;
	int	ret=PQLXSUCCESS;
	const char *str;
	char	*dbFN = get_db_name(dbfile);

	memset(&key, 0, sizeof(TDB_DATA));
	memset(&data, 0, sizeof(TDB_DATA));
    if ((dbf=tdb_open(dbFN, 0, TDB_NOLOCK, O_RDWR, 0644)) == NULL)
    {
		return (testFile(dbFN));
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
			key.dptr = (unsigned char *) g_strdup(rec->name);
			key.dsize = strlen(rec->name);
			data.dptr = (unsigned char *) g_strdup(rec->value);
			data.dsize = strlen(rec->value);
		}
		break;
	
		case FILTERST:
		{
			FILTER_REC *rec = record;
			key.dptr = (unsigned char *) g_strdup(rec->name);
			key.dsize = strlen(rec->name);
			data.dptr = (unsigned char *) g_strdup_printf("%d %f %d %f", 
							rec->poles[HIGHF], rec->cutoff[HIGHF],
							rec->poles[LOWF], rec->cutoff[LOWF]);
			str = (char *) data.dptr;	// handle compiler warning
			data.dsize = strlen(str);
		}
		break;

#ifndef PQL_ONLY
		case PDFST:
		break;

		case PDFSRVRST:
		{
			PDFSRVR_REC *rec = record;
			key.dptr = (unsigned char *) g_strdup(rec->name);
			key.dsize = strlen(rec->name);
			data.dptr = (unsigned char *) g_strdup(rec->name);
			data.dsize = strlen(rec->name);
		}
		break;
		
		case STNCHNFLTRST:
		{
			STN_CHN_FILTER *rec = record;
			key.dptr = (unsigned char *) g_strdup(rec->name);
			key.dsize = strlen(rec->name);
			data.dptr = (unsigned char *) g_strdup(rec->name);
			data.dsize = strlen(rec->name);
		}
		break;
		
		case STNCHNYSCALE:
		{
			STN_CHN_YSCALE *rec = record;
			key.dptr = (unsigned char *) g_strdup(rec->name);
			key.dsize = strlen(rec->name);
			data.dptr = (unsigned char *) g_strdup_printf("%s %s %s %s", rec->loc, rec->chn, rec->scale[MAX], rec->scale[MIN]);
			str = (char *) data.dptr;	// handle compiler warning
			data.dsize = strlen(str);
		}
		break;
			
		case STNGRPYSCALE:
		{
			char	chnKeyW[1024], *chnIns, *chnWPtr;
			GSList	*iter;
			STN_GRP_YSCALE *rec = record;
			key.dptr = (unsigned char *) g_strdup(rec->name);
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
			data.dptr = (unsigned char *) g_strdup(chnKeyW);
			str = (char *) data.dptr;	// handle compiler warning
			data.dsize = strlen(str);
		}
		break;
#endif
	}

    // write record to database
	ret = tdb_store(dbf, key, data, TDB_MODIFY);
	if (ret != TDB_SUCCESS)
	{
		ret = tdb_store(dbf, key, data, TDB_INSERT);
		if (ret != TDB_SUCCESS)
		{
			fprintf(stderr, "Unable to add key '%s' to the local data store.\n", key.dptr);
			fprintf(stderr, "\tTDB Error (code=%d): %s\n", tdb_error(dbf), tdb_errorstr(dbf));
			ret = FAILURE;
		}
	}
    
	free(key.dptr);
	free(data.dptr);
    tdb_close(dbf);
	return ret;
}

int create_dbl(int dbfile)
{
    // create database
    TDB_CONTEXT *dbf;
	int ret=PQLXSUCCESS;
	char	*dbFN = get_db_name(dbfile);

    if ((dbf=tdb_open(dbFN, 0, TDB_DEFAULT, O_RDWR | O_CREAT, 0644)) == NULL)
   	{
		fprintf(stderr, "Unable to Create the local DB (filename = '%s').\n", dbFN);
   	    return FAILURE;
   	}
    
    // close database
    tdb_close(dbf);
	return ret;
}
