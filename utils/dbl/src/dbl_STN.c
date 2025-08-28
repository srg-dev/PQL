#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include "gdbm.h"
#include "pqlx_errors.h"
#include "dbl_iface.h"
#include "dblinc.h"

static gboolean	DBLVALID=TRUE;

static void initDBL(DBLDI *dbldi)
{	// create the data store if not already
	int	ret;

	if ((ret=create_dbl(dbldi->descr.tableData)) != PQLXSUCCESS)
	{
		fprintf(stderr,"Unable to make local database\n");
		fprintf(stderr,"\tSTN Channel Filter Definitions will not be saved!\n");
		DBLVALID = FALSE;
	}
	else
	{
		DBLVALID = TRUE;
	}
}

#ifdef NOCOMP
static int read_all(DBLDI *dbldi)
{
	int		ret;

	ret = get_all_records(dbldi->descr.tableData, dblrecs);

	return ret;
}
#endif

static int write_sngl(DBLDI *dbldi)
{
	int		ret=FAILURE;
	DBLRECORDS	*dblrecs = dbldi->data;

	initDBL(dbldi);
	if (!DBLVALID)
		return ret;
	ret = PQLXSUCCESS;

	add_record(dbldi->descr.tableData, dblrecs->recs[0]);

	return ret;
}

static int write_all(DBLDI *dbldi)
{
	int i, ret=FAILURE;
	DBLRECORDS	*dblrecs = dbldi->data;

	initDBL(dbldi);

	if (!DBLVALID)
		return ret;
	ret = PQLXSUCCESS;

	for(i=0;i<dblrecs->numRecs;i++)
	{
		add_record(dbldi->descr.tableData, dblrecs->recs[i]);
	}

	return ret;
}

int dbl_STN(DBLDI *dbldi)
{
	int ret = FAILURE;
	switch(dbldi->descr.dataEvent)
	{
		case READ_ALL:
			ret = get_all_records(dbldi->descr.tableData, dbldi->data);
		break;

		case WRITE_ALL:
			ret = write_all(dbldi);
		break;

		case WRITE_SNGL:
			ret = write_sngl(dbldi);
		break;

		case DELETE_SNGL:
		{
			DBLRECORDS *dblrecs = dbldi->data;
			switch(dbldi->descr.tableData)
			{
				case STNCHNFLTRST:
					ret = remove_record(STNCHNFLTRST, ((STN_CHN_FILTER *) (dblrecs->recs[0]))->name);
				break;
				case STNCHNYSCALE:
					ret = remove_record(STNCHNYSCALE, ((STN_CHN_YSCALE *) (dblrecs->recs[0]))->name);
				break;
				case STNGRPYSCALE:
					ret = remove_record(STNGRPYSCALE, ((STN_GRP_YSCALE *) (dblrecs->recs[0]))->name);
				break;
			}
		}
		break;
	}

	return ret;
}
