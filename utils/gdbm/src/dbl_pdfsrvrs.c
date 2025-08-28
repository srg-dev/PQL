#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include "gdbm.h"
#include "pqlx_errors.h"
#include "dbl_iface.h"
#include "dblinc.h"

static gboolean	DBLVALID=TRUE;

static void initDBL()
{	// create the data store, previous contents are destoryed!
	int	ret;

	if ((ret=create_dbl_g(PDFSRVRST)) != PQLXSUCCESS)
	{
		fprintf(stderr,"Unable to make local database\n");
		fprintf(stderr,"\tPQLX PDF Server Information will not be saved!\n");
		DBLVALID = FALSE;
	}
	else
	{
		DBLVALID = TRUE;
	}
}

#ifdef NOCOMP
static int read_all(DBLRECORDS *dblrecs)
{
	int		ret;

	ret = get_all_records_g(PDFSRVRST, dblrecs);

	return ret;
}

static int write_sngl(DBLRECORDS *dblrecs)
{
	int		ret=FAILURE;

	initDBL();

	if (!DBLVALID)
		return ret;
	ret = PQLXSUCCESS;

	add_record_g(PDFSRVRST, dblrecs->recs[0]);

	return ret;
}
#endif

static int write_all(DBLRECORDS *dblrecs)
{
	int i, ret=FAILURE;

	initDBL();

	if (!DBLVALID)
		return ret;
	ret = PQLXSUCCESS;

	for(i=0;i<dblrecs->numRecs;i++)
	{
		add_record_g(PDFSRVRST, dblrecs->recs[i]);
	}

	return ret;
}

int dblg_pdfsrvr(DBLDI *dbldi)
{
	int ret = FAILURE;
	switch(dbldi->descr.dataEvent)
	{
		case READ_ALL:
			ret = get_all_records_g(PDFSRVRST, dbldi->data);
		break;

		case WRITE_ALL:
			ret = write_all(dbldi->data);
		break;

		case DELETE_SNGL:
		{
			DBLRECORDS *dblrecs = dbldi->data;
			ret = remove_record_g(PDFSRVRST, ((PDFSRVR_REC *) (dblrecs->recs[0]))->name);
		}
		break;
	}

	return ret;
}
