#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include "gdbm.h"
#include "pqlx_errors.h"
#include "dbl_iface.h"
#include "dblinc.h"

static gboolean	DBLVALID=TRUE;

static void initDBL()
{
	int	ret;

	if ((ret=create_dbl_g(FILTERST)) != PQLXSUCCESS)
	{
		fprintf(stderr,"Unable to make local database\n");
		fprintf(stderr,"\tPQL Filter Information will not be saved!\n");
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

	ret = get_all_records(FILTERST, dblrecs);

	return ret;
}
#endif

static int write_sngl(DBLRECORDS *dblrecs)
{
	int		ret=FAILURE;

	initDBL();

	if (!DBLVALID)
		return ret;
	ret = PQLXSUCCESS;

	add_record_g(FILTERST, dblrecs->recs[0]);

	return ret;
}

int dblg_filter(DBLDI *dbldi)
{
	int ret = FAILURE;
	switch(dbldi->descr.dataEvent)
	{
		case READ_ALL:
			ret = get_all_records_g(FILTERST, dbldi->data);
		break;

		case WRITE_SNGL:
			ret = write_sngl(dbldi->data);
		break;

		case DELETE_SNGL:
		{
			DBLRECORDS *dblrecs = dbldi->data;
			ret = remove_record_g(FILTERST, ((FILTER_REC *) (dblrecs->recs[0]))->name);
		}
		break;
	}

	return ret;
}
