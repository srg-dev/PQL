#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pql_defines.h"
#include "pql_externs.h"

static void setNewHdr(traceInfo *trc)
{
	struct ptime date;
	
//	trc->data.fileName = trc->excerpt.description;
	trc->data.head.longFN = trc->data.fileName;
	trc->data.head.shortFN = g_strdup(trc->data.fileName);

//	trc->data.dataInfo.data = NULL;
	trc->trace.numGaps = trc->trace.numOlaps = 0;
	trc->trace.gaps = trc->trace.overlaps = NULL;

	passcal_etoh(&date, trc->data.head.epochs);
	trc->data.head.year = date.yr;
	trc->data.head.day = date.jday;
	trc->data.head.hour = date.hr;
	trc->data.head.minute = date.mn;
	trc->data.head.second = date.sec;
	trc->data.head.m_secs = (trc->data.head.epochs - (int) trc->data.head.epochs)*1000.;
	trc->data.head.length = (trc->data.dataInfo.head.numSamples) *
							(trc->data.head.sampInt/1000000.) - (trc->data.head.sampInt/1000000.);
	trc->data.head.epochStop = trc->data.head.epochs + trc->data.head.length;

	trc->data.head.descr1 = g_strdup_printf("%i %03i %02i:%02i" ,
									trc->data.head.year, trc->data.head.day, 
									trc->data.head.hour, trc->data.head.minute);
	trc->data.head.descr2 = g_strdup_printf("%s.%s.%s.%s",
									trc->data.head.network, trc->data.head.station,
									trc->data.head.location, trc->data.head.channel);

	trc->data.dataFilt = NULL;
	trc->trace.olapData = NULL;
	trc->data.head.fileHdr = NULL;
}

static void manageGLaps(traceInfo *from, traceInfo *to, int startSamp, int endSamp)
{
	int		i;
	glaps	*glapPtr;
	GSList	*gapIter = NULL, *olapIter = NULL;
	gboolean inPlace=FALSE;
	
	if (from == to)
		inPlace = TRUE;
		
	for(i=0;i<from->trace.numGaps*2;i+=2)
	{
		if (from->trace.gaps[i+1] < startSamp)
			continue;								// gap is before selection, ignore 
		if (from->trace.gaps[i] > endSamp)
			break;									// gap is after selection, we're done
		glapPtr = calloc(1, sizeof(glaps));
		glapPtr->bounds[GLAPBEG] = from->trace.gaps[i] - startSamp;
		if (from->trace.gaps[i+1] > endSamp)
		{	// new gap ends at boundary
			glapPtr->bounds[GLAPEND] = endSamp - startSamp;
		}
		else
		{	// new gap ends at gap end
			glapPtr->bounds[GLAPEND] = from->trace.gaps[i+1] - startSamp;
		}
		gapIter = g_slist_append(gapIter, glapPtr);
	}		

	for(i=0;i<from->trace.numOlaps*2;i+=2)
	{
		if (from->trace.overlaps[i] < startSamp)
			continue;
		if (from->trace.overlaps[i] > endSamp)
			break;
		glapPtr = calloc(1, sizeof(glaps));
		glapPtr->bounds[GLAPBEG] = from->trace.overlaps[i] - startSamp;
		if (from->trace.overlaps[i+1] > endSamp)
		{
			// new overlap ends at boundary
			glapPtr->bounds[GLAPEND] = endSamp - startSamp;
		}
		else
		{
			// new overlap ends at overlap end
			glapPtr->bounds[GLAPEND] = from->trace.overlaps[i+1] - startSamp;
		}
		olapIter = g_slist_append(olapIter, glapPtr);
	}

	if (inPlace)
	{
		if (from->trace.gaps)
		{
			free(from->trace.gaps);
			from->trace.gaps = NULL;
		}
		if (from->trace.overlaps)
		{
			free(from->trace.overlaps);
			from->trace.overlaps = NULL;
		}
	}

	to->trace.numGaps = g_slist_length(gapIter);
	if (to->trace.numGaps)
	{
		to->trace.gaps = calloc(to->trace.numGaps, sizeof(int)*2);
		for(i=0;gapIter;gapIter=g_slist_next(gapIter), i+=2)
		{
			glapPtr = (gapIter)->data;
			to->trace.gaps[i] = glapPtr->bounds[GLAPBEG];
			to->trace.gaps[i+1] = glapPtr->bounds[GLAPEND];
			free(glapPtr);
		}
		g_slist_free(gapIter);
		gapIter = NULL;
	}

	to->trace.numOlaps = g_slist_length(olapIter);
	if (to->trace.numOlaps)
	{
		to->trace.overlaps = calloc(to->trace.numOlaps, sizeof(int)*2);
		for(i=0;olapIter;olapIter=g_slist_next(olapIter), i+=2)
		{
			glapPtr = (olapIter)->data;
			to->trace.overlaps[i] = glapPtr->bounds[GLAPBEG];
			to->trace.overlaps[i+1] = glapPtr->bounds[GLAPEND];
			free(glapPtr);
		}
		g_slist_free(olapIter);
	}
}

void trace_excerpt(traceInfo *from, traceInfo *to)
{
	int startSamp, sPs, numSamples, alloc;
	int	oneSample=to->data.head.sampInt;
	char *data;
	gboolean inPlace=FALSE;
	
	if (from == to)
		inPlace = TRUE;
		
	sPs = 1000000./from->data.head.sampInt;
	startSamp = (to->excerpt.epochS - to->data.head.epochs) * sPs;
	if (startSamp < 0)
	{	// to->excerpt.epochS is rounded to the minute, maybe down
		startSamp = 0;		
		to->excerpt.epochS = from->data.head.epochs;
	}
	
	numSamples = to->excerpt.length * sPs;
	alloc = numSamples * sizeof(dsizes[to->data.head.data_form]);
	if ((data = malloc(alloc))==NULL)
	{
		fprintf(stderr, "Unable to allocate %d bytes of data, necessarily quitting.\n", alloc);
		exit(-1);
	}
	if (numSamples > from->data.dataInfo.head.numSamples - startSamp + 1)
		numSamples = from->data.dataInfo.head.numSamples - startSamp + 1;
	memcpy(data, &from->data.dataInfo.data[startSamp*dsizes[from->data.head.data_form]], 
						numSamples * dsizes[from->data.head.data_form]);

	if (inPlace)
		free(from->data.dataInfo.data);
	to->data.dataInfo.data = data;

	to->data.head.epochs = to->excerpt.epochS;
	to->data.dataInfo.head.numSamples = numSamples;
	to->data.head.length = (numSamples)*(oneSample/1000000.) - (oneSample/1000000.);
	to->data.dataInfo.head.data_form = to->data.head.data_form;
	to->data.dataInfo.head.sampInt = to->data.head.sampInt;
	to->data.dataInfo.head.secsPsamp = (to->data.head.sampInt/1000000.);
	setNewHdr(to);
	
	manageGLaps(from, to, startSamp, startSamp + numSamples - 1);
	get_maxmin_amp(to);
}
