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

	if ((ret=create_dbl_g(DEFAULTST)) != PQLXSUCCESS)
	{
		fprintf(stderr,"Unable to make local database\n");
		fprintf(stderr,"\tPQL Defaults information will not be saved!\n");
		DBLVALID = FALSE;
	}
	else
	{
		DBLVALID = TRUE;
	}
}

static int read_all(DBLRECORDS *dblrecs)
{
	int		ret;

	ret = get_all_records_g(DEFAULTST, dblrecs);

	if (ret != PQLXSUCCESS)
	{
		ret = FAILURE;
	}
	else
	{
		ret = PQLXSUCCESS;
	}
	return ret;
}

static int write_all(DBLRECORDS *dblrecs)
{
	int i, ret=FAILURE;

	initDBL();

	if (!DBLVALID)
		return ret;
	ret = PQLXSUCCESS;

	for(i=0;i<dblrecs->numRecs;i++)
	{
		add_record_g(DEFAULTST, dblrecs->recs[i]);
	}

	return ret;
}

int dblg_defaults(DBLDI *dbldi)
{
	int ret = FAILURE;
	switch(dbldi->descr.dataEvent)
	{
		case READ_ALL:
			ret = read_all(dbldi->data);
		break;

		case WRITE_ALL:
			ret = write_all(dbldi->data);
		break;
	}

	return ret;
}
