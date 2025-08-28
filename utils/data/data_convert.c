#include "nano2ah.h"
#include "pql_defines.h"
#include "pql_externs.h"

void get_null_head(ahhed *hed);

void convert_header3(ahhead, trace)
ahhed *ahhead;
NMXTRACE3 *trace;
{
	get_null_head(ahhead);

	strncpy(ahhead->station.code, 
		(char*) &trace->stSeries.szFileName[2], 3);

	ahhead->station.code[3] = '\0';

	strncpy(ahhead->station.chan, 
		(char*) trace->stStnChn.achChannel, 1);

	ahhead->station.chan[1] = '\0';

	strncpy(ahhead->station.stype, 
		(char*) &trace->stStnChn.szSensorType[4], 4);

	strncpy(ahhead->station.stype, (char *) &(trace->stStnChn.szSensorType[4]), 4);

	ahhead->station.stype[4] = '\0';

	ahhead->station.slat = trace->stStnChn.rLatitude;
	ahhead->station.slon = trace->stStnChn.rLongitude;
	ahhead->station.elev = trace->stStnChn.rElevation;
	ahhead->station.DS = 1.0;
	ahhead->station.A0 = 1.0;

        passcal_etoh(&(ahhead->record.abstime), trace->stSeries.rtmStartTime1);

	ahhead->record.type = FLOAT;
	ahhead->record.ndata = (gint32) trace->stSeries.ulNumSamples;
	ahhead->record.delta = 1.0 / trace->stStnChn.rSampleRate;
	ahhead->record.maxamp = trace->stSeries.IMaxAmplitude;

	strcpy(ahhead->record.log, "nano2ah;");
}


void convert_header1(ahhead, trace)
ahhed *ahhead;
NMXTRACE1 *trace;
{
	double epoch;

	get_null_head(ahhead);

	strncpy(ahhead->station.code, trace->achSiteId, 3);
	ahhead->station.code[3] = '\0';
	strncpy(ahhead->station.chan, trace->achChanID, 1);
	ahhead->station.chan[1] = '\0';
	strncpy(&ahhead->station.stype[0], &trace->achSensorType[4], 4);
	ahhead->station.stype[4] = '\0';

	ahhead->station.slat = trace->fLatitude;
	ahhead->station.slon = trace->fLongitude;
	ahhead->station.elev = trace->fElevation;
	ahhead->station.DS = 1.0;
	ahhead->station.A0 = 1.0;

        epoch = (double) trace->ulTimeSecs;
        epoch += ((double) trace->usTimeMs) / 1000.0;
        passcal_etoh(&(ahhead->record.abstime), epoch);

	ahhead->record.type = FLOAT;
	ahhead->record.ndata = (gint32) trace->ulNumSamples;
	ahhead->record.delta = 1.0 / (double) trace->usSampleRate;

	strcpy(ahhead->record.log, "nano2ah;");
}
