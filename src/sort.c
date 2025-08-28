#include <string.h>
#include "pql_defines.h"
#include "pql_externs.h"
#include "ahhead.h"
#include "sac.h"
#include "segy.h"

#define loc(a)	((long) a + (int) offset)

#define VALIDFILE(a, b)	((a==AH_DATA && b==AHSORT) ? TRUE : \
						 (a==SEGY_DATA && b==SEGYSORT) ? TRUE : \
						 (a==SAC_DATA && b==SACSORT) ? TRUE : FALSE)
		
gint listSort(traceInfo *trc1, traceInfo *trc2, SORTPARAMS *sortParams)
{
	int 		type=0, offset=0, sortCrit=sortParams->sortCrit;
	gboolean 	dir=TRUE;
	int			retVal=0, dirVal;
	CRITITEM	*crit;
	CRITDEF		*critDef;
	SORTINFO	*sortInfo;
	void		*dataPtr1, *dataPtr2;
	
	switch(sortParams->type)
	{
		case SORTUSER:
			sortInfo = g_slist_nth_data(settings.general.sort.sorts, settings.general.sort.selected);
			critDef = g_slist_nth_data(sortInfo->user, sortCrit);
			crit = g_slist_nth_data(sortInfo->criteria, critDef->num);
			type = crit->fieldType;
			offset = crit->offset;
			dir = critDef->dir;
			switch(settings.general.sort.selected)
			{	// set the appropriate header base address
				case NONE:
				case PQLSORT:
					dataPtr1 = trc1;
					dataPtr2 = trc2;
				break;
				case AHSORT:
				case SEGYSORT:
				case SACSORT:
					dataPtr1 = trc1->data.head.fileHdr;
					dataPtr2 = trc2->data.head.fileHdr;
					// handle case when file is not of requested type
					if (!dataPtr1 || !VALIDFILE(trc1->data.fileType, settings.general.sort.selected))
						return -1;
					if (!dataPtr2 || !VALIDFILE(trc2->data.fileType, settings.general.sort.selected))
						return 1;
				break;
			}
		break;
		
		case SORTDIRECT:
			sortInfo = g_slist_nth_data(settings.general.sort.sorts, PQLSORT);
			critDef = g_slist_nth_data(sortInfo->user, sortCrit);
			crit = g_slist_nth_data(sortInfo->criteria, critDef->num);
			type = crit->fieldType;
			offset = crit->offset;
			dir = TRUE;
			dataPtr1 = trc1;
			dataPtr2 = trc2;
		break;
	}
	dirVal = dir ? 1 : -1;		// TRUE = ascending
		
	switch (type)
	{
		case MYSTRINGPTR:
			retVal = dirVal * (strcmp((char *) *((char **) loc(dataPtr1)), (char *) *((char **) loc(dataPtr2))));
		break;

		case MYSTRING:
			retVal = dirVal * (strcmp((char *) loc(dataPtr1), (char *) loc(dataPtr2)));
		break;

		case MYINT:
			retVal = dirVal * (*((int *) loc(dataPtr1)) - *((int *) loc(dataPtr2)));
		break;

		case MYDOUBLE:
			retVal = dirVal * (*((double *) loc(dataPtr1)) - *((double *) loc(dataPtr2)));
		break;
			
		case MYFLOAT:
			retVal = dirVal * (*((float *) loc(dataPtr1)) - *((float *) loc(dataPtr2)));
		break;
			
		case MYPTIME:
		{
			PTIME *time1 = ((PTIME *) loc(dataPtr1)),
				  *time2 = ((PTIME *) loc(dataPtr2));
			float epoch1 = (float) ((gint32) passcal_htoe(time1, (time1->mo ? TRUE : FALSE)) + (float) (time1->sec - (int) time1->sec)),
				  epoch2 = (float) ((gint32) passcal_htoe(time2, (time2->mo ? TRUE : FALSE)) + (float) (time2->sec - (int) time2->sec));
			
			retVal = dirVal * ((int) (epoch1*1000.) - (int) (epoch2*1000.));
		}
		break;
			
		case MYINSTSORT:
		{	// sort type for list of data provided as input by user 
			int *ins1 = g_hash_table_lookup(settings.general.sort.instOrder, (char *) loc(dataPtr1)),
				*ins2 = g_hash_table_lookup(settings.general.sort.instOrder, (char *) loc(dataPtr2));
			int insert1, insert2;
				
			if (!ins1)
			{	// insert item doesn't exist in our lookup table, put to head
				retVal = -1;
				break;
			}
				
			if (!ins2)
			{
				retVal = 1;
				break;
			}
				
			insert1 = *ins1;
			insert2 = *ins2;
			retVal = dir
					? insert1 - insert2
					: (insert1 - insert2)*-1;
		}
		break;
	}
	return retVal;
}

void sortInit()
{
	// initialize the offset locations of the header values available for sorting
	// offset used in above routine for positioning to proper location for comparison
	int		i;
	traceInfo	trace;
	ahhed	ahHead;
	struct sac	sacHead;
	SEGYHEAD	segyHead;
	SORTINFO	*sortInfo;
	
	for (i=0;i<TTLPQLSORTCRITERIA;i++)
	{
		switch(i)
		{
			case INPUT:
				criteria[i].offset = offsetD(trace.data.position.original, trace);
			break;
			case LONGFN:
				criteria[i].offset = offsetD(trace.data.head.longFN, trace);
			break;
			case SHORTFN:
				criteria[i].offset = offsetD(trace.data.head.shortFN, trace);
			break;
			case NETWORK:
				criteria[i].offset = offsetD(trace.data.head.network, trace);
			break;
			case STATION:
				criteria[i].offset = offsetD(trace.data.head.station, trace);
			break;
			case LOCATION:
				criteria[i].offset = offsetD(trace.data.head.location, trace);
			break;
			case CHANNEL:
				criteria[i].offset = offsetD(trace.data.head.channel, trace);
			break;
			case SENSOR:
				criteria[i].offset = offsetD(trace.data.head.sensorID, trace);
			break;
			case ABSSTART:
				criteria[i].offset = offsetD(trace.data.head.epochs, trace);
			break;
			case FILETYPE:
				criteria[i].offset = offsetD(trace.data.fileType, trace);
			break;
		}
	}

	for (i=0;i<TTLAHSORTCRITERIA;i++)
	{
		switch(i)
		{
			case AHINPUT:
				AHcriteria[i].offset = offsetD(trace.data.position.original, trace);
			break;
			case AHSTN:
				AHcriteria[i].offset = offsetD(ahHead.station.code, ahHead);
			break;
			case AHCHN:
				AHcriteria[i].offset = offsetD(ahHead.station.chan, ahHead);
			break;
			case AHABSTIME:
				AHcriteria[i].offset = offsetD(ahHead.record.abstime, ahHead);
			break;
			case AHSLAT:
				AHcriteria[i].offset = offsetD(ahHead.station.slat, ahHead);
			break;
			case AHSLON:
				AHcriteria[i].offset = offsetD(ahHead.station.slon, ahHead);
			break;
			case AHELEV:
				AHcriteria[i].offset = offsetD(ahHead.station.elev, ahHead);
			break;
			case AHELAT:
				AHcriteria[i].offset = offsetD(ahHead.event.lat, ahHead);
			break;
			case AHELON:
				AHcriteria[i].offset = offsetD(ahHead.event.lon, ahHead);
			break;
			case AHDEP:
				AHcriteria[i].offset = offsetD(ahHead.event.dep, ahHead);
			break;
			case AHOT:
				AHcriteria[i].offset = offsetD(ahHead.event.ot, ahHead);
			break;
		}
	}

	for (i=0;i<TTLSACSORTCRITERIA;i++)
	{
		switch(i)
		{
			case SACINPUT:
				SACcriteria[i].offset = offsetD(trace.data.position.original, trace);
			break;
			case SACKINST:
				SACcriteria[i].offset = offsetD(sacHead.kinst[0], sacHead);
			break;
			case SACKNETWK:
				SACcriteria[i].offset = offsetD(sacHead.knetwk[0], sacHead);
			break;
			case SACKSTNM:
				SACcriteria[i].offset = offsetD(sacHead.kstnm[0], sacHead);
			break;
			case SACKKHOLE:
				SACcriteria[i].offset = offsetD(sacHead.khole[0], sacHead);
			break;
			case SACKCMPNM:
				SACcriteria[i].offset = offsetD(sacHead.kcmpnm[0], sacHead);
			break;
			case SACNZYEAR:
				SACcriteria[i].offset = offsetD(sacHead.nzyear, sacHead);
			break;
			case SACNZJDAY:
				SACcriteria[i].offset = offsetD(sacHead.nzjday, sacHead);
			break;
			case SACNZHOUR:
				SACcriteria[i].offset = offsetD(sacHead.nzhour, sacHead);
			break;
			case SACNZMIN:
				SACcriteria[i].offset = offsetD(sacHead.nzmin, sacHead);
			break;
			case SACNZSEC:
				SACcriteria[i].offset = offsetD(sacHead.nzsec, sacHead);
			break;
			case SACNZMSEC:
				SACcriteria[i].offset = offsetD(sacHead.nzmsec, sacHead);
			break;
			case SACSTLA:
				SACcriteria[i].offset = offsetD(sacHead.stla, sacHead);
			break;
			case SACSTLO:
				SACcriteria[i].offset = offsetD(sacHead.stlo, sacHead);
			break;
			case SACSTEL:
				SACcriteria[i].offset = offsetD(sacHead.stel, sacHead);
			break;
			case SACCMPAZ:
				SACcriteria[i].offset = offsetD(sacHead.cmpaz, sacHead);
			break;
			case SACCMPINC:
				SACcriteria[i].offset = offsetD(sacHead.cmpinc, sacHead);
			break;
			case SACO:
				SACcriteria[i].offset = offsetD(sacHead.o, sacHead);
			break;
			case SACKEVNM:
				SACcriteria[i].offset = offsetD(sacHead.kevnm[0], sacHead);
			break;
			case SACEVLA:
				SACcriteria[i].offset = offsetD(sacHead.evla, sacHead);
			break;
			case SACEVLO:
				SACcriteria[i].offset = offsetD(sacHead.evlo, sacHead);
			break;
			case SACEVEL:
				SACcriteria[i].offset = offsetD(sacHead.evel, sacHead);
			break;
			case SACEVDP:
				SACcriteria[i].offset = offsetD(sacHead.evdp, sacHead);
			break;
			case SACMAG:
				SACcriteria[i].offset = offsetD(sacHead.mag, sacHead);
			break;
			case SACNEVID:
				SACcriteria[i].offset = offsetD(sacHead.nevid, sacHead);
			break;
			case SACNORID:
				SACcriteria[i].offset = offsetD(sacHead.norid, sacHead);
			break;
			case SACDIST:
				SACcriteria[i].offset = offsetD(sacHead.dist, sacHead);
			break;
			case SACAZ:
				SACcriteria[i].offset = offsetD(sacHead.az, sacHead);
			break;
			case SACBAZ:
				SACcriteria[i].offset = offsetD(sacHead.baz, sacHead);
			break;
			case SACGCARC:
				SACcriteria[i].offset = offsetD(sacHead.gcarc, sacHead);
			break;
		}
	}

	for (i=0;i<TTLSEGYSORTCRITERIA;i++)
	{
		switch(i)
		{
			case SACINPUT:
				SEGYcriteria[i].offset = offsetD(trace.data.position.original, trace);
			break;
			case SEGYSTN:
				SEGYcriteria[i].offset = offsetD(segyHead.station_name[0], segyHead);
			break;
			case SEGYCHN:
				SEGYcriteria[i].offset = offsetD(segyHead.channel_name[0], segyHead);
			break;
			case SEGYSENSOR:
				SEGYcriteria[i].offset = offsetD(segyHead.sensor_serial[0], segyHead);
			break;
			case SEGYINSTNO:
				SEGYcriteria[i].offset = offsetD(segyHead.inst_no, segyHead);
			break;
			case SEGYCHNNUM:
				SEGYcriteria[i].offset = offsetD(segyHead.channel_number, segyHead);
			break;
			case SEGYYEAR:
				SEGYcriteria[i].offset = offsetD(segyHead.year, segyHead);
			break;
			case SEGYJDAY:
				SEGYcriteria[i].offset = offsetD(segyHead.day, segyHead);
			break;
			case SEGYHOUR:
				SEGYcriteria[i].offset = offsetD(segyHead.hour, segyHead);
			break;
			case SEGYMINUTE:
				SEGYcriteria[i].offset = offsetD(segyHead.minute, segyHead);
			break;
			case SEGYSECOND:
				SEGYcriteria[i].offset = offsetD(segyHead.second, segyHead);
			break;
			case SEGYMILLISEC:
				SEGYcriteria[i].offset = offsetD(segyHead.m_secs, segyHead);
			break;
			case SEGYLINESEQ:
				SEGYcriteria[i].offset = offsetD(segyHead.lineSeq, segyHead);
			break;
			case SEGYREELSEQ:
				SEGYcriteria[i].offset = offsetD(segyHead.reelSeq, segyHead);
			break;
			case SEGYEVTNUM:
				SEGYcriteria[i].offset = offsetD(segyHead.event_number, segyHead);
			break;
			case SEGYS2RDIST:
				SEGYcriteria[i].offset = offsetD(segyHead.sourceToRecDist, segyHead);
			break;
			case SEGYRECELEV:
				SEGYcriteria[i].offset = offsetD(segyHead.recElevation, segyHead);
			break;
			case SEGYSRCSURFELEV:
				SEGYcriteria[i].offset = offsetD(segyHead.sourceSurfaceElevation, segyHead);
			break;
			case SEGYSRCDEP:
				SEGYcriteria[i].offset = offsetD(segyHead.sourceDepth, segyHead);
			break;
			case SEGYDTELEVREC:
				SEGYcriteria[i].offset = offsetD(segyHead.datumElevRec, segyHead);
			break;
			case SEGYDTELEVSOURCE:
				SEGYcriteria[i].offset = offsetD(segyHead.datumElevSource, segyHead);
			break;
			case SEGYSRCWATERDEP:
				SEGYcriteria[i].offset = offsetD(segyHead.sourceWaterDepth, segyHead);
			break;
			case SEGYRECWATERDEP:
				SEGYcriteria[i].offset = offsetD(segyHead.recWaterDepth, segyHead);
			break;
			case SEGYSRCLATY:
				SEGYcriteria[i].offset = offsetD(segyHead.sourceLatOrY, segyHead);
			break;
			case SEGYSRCLONX:
				SEGYcriteria[i].offset = offsetD(segyHead.sourceLongOrX, segyHead);
			break;
			case SEGYRECLATY:
				SEGYcriteria[i].offset = offsetD(segyHead.recLatOrY, segyHead);
			break;
			case SEGYRECLONX:
				SEGYcriteria[i].offset = offsetD(segyHead.recLongOrX, segyHead);
			break;
			case SEGYSRCUPHOLET:
				SEGYcriteria[i].offset = offsetD(segyHead.sourceUpholeTime, segyHead);
			break;
			case SEGYRECUPHOLET:
				SEGYcriteria[i].offset = offsetD(segyHead.recUpholeTime, segyHead);
			break;
			case SEGYTYEAR:
				SEGYcriteria[i].offset = offsetD(segyHead.trigyear, segyHead);
			break;
			case SEGYTJDAY:
				SEGYcriteria[i].offset = offsetD(segyHead.trigday, segyHead);
			break;
			case SEGYTHOUR:
				SEGYcriteria[i].offset = offsetD(segyHead.trighour, segyHead);
			break;
			case SEGYTMINUTE:
				SEGYcriteria[i].offset = offsetD(segyHead.trigminute, segyHead);
			break;
			case SEGYTSECOND:
				SEGYcriteria[i].offset = offsetD(segyHead.trigsecond, segyHead);
			break;
			case SEGYTMILLS:
				SEGYcriteria[i].offset = offsetD(segyHead.trigmills, segyHead);
			break;
		}
	}

	// insert the above static sort info's to our global linked list
	// these must be insert to the linked list the same order as enum TTLPQLSORTS
	// PQLSORT
	sortInfo = calloc(1, sizeof(SORTINFO));
	sortInfo->sortNum = PQLSORT;
	for (i=0;i<TTLPQLSORTCRITERIA;i++)
	{
		sortInfo->criteria = g_slist_append(sortInfo->criteria, &criteria[i]);
	}
	settings.general.sort.sorts = g_slist_append(settings.general.sort.sorts, sortInfo);

	// AHSORT
	sortInfo = calloc(1, sizeof(SORTINFO));
	sortInfo->sortNum = AHSORT;
	for (i=0;i<TTLAHSORTCRITERIA;i++)
	{
		sortInfo->criteria = g_slist_append(sortInfo->criteria, &AHcriteria[i]);
	}
	settings.general.sort.sorts = g_slist_append(settings.general.sort.sorts, sortInfo);

	// SEGYSORT
	sortInfo = calloc(1, sizeof(SORTINFO));
	sortInfo->sortNum = SEGYSORT;
	for (i=0;i<TTLSEGYSORTCRITERIA;i++)
	{
		sortInfo->criteria = g_slist_append(sortInfo->criteria, &SEGYcriteria[i]);
	}
	settings.general.sort.sorts = g_slist_append(settings.general.sort.sorts, sortInfo);

	// SACSORT
	sortInfo = calloc(1, sizeof(SORTINFO));
	sortInfo->sortNum = SACSORT;
	for (i=0;i<TTLSACSORTCRITERIA;i++)
	{
		sortInfo->criteria = g_slist_append(sortInfo->criteria, &SACcriteria[i]);
	}
	settings.general.sort.sorts = g_slist_append(settings.general.sort.sorts, sortInfo);
}

