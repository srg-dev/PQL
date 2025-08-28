#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>

#include "pqlx_defs.h"
#include "pql_defines.h"
#include "util_externs.h"

void demean(DATAINFO *dataInfo, float mean)
{	// demean in place
	float	*data = (float *) dataInfo->data;
	int		numPts = dataInfo->head.numSamples;
	int		i;
	
    for (i=0; i<numPts; i++) 
	{
		data[i] -= mean;
	}
}

void polarityReverse(DATAINFO *dataInfo)
{
	float	*data = (float *) dataInfo->data;
	int		numPts = dataInfo->head.numSamples;
	int 	i;
    for (i=0; i<numPts; i++) 
	{
		data[i] *= -1;
    }
}

void detrend(DATAINFO *dataInfo)
{
	float	*c = (float *) dataInfo->data;
	int		n = dataInfo->head.numSamples;
    int		i = 0;
    double	a = 0.0, b = 0.0, tsqsum = 0.0, ysum = 0.0, t = 0.0;

	for(i=0;i<n;i++)
	{
		ysum += c[i];
	}
	for(i=0;i<n;i++)
	{
		t = i - n/2 + 0.5;
		tsqsum += t*t;
		b += t*c[i];
	}
    b /= tsqsum;
    a = ysum/n - b*(n-1)/2.0;
	for(i=0;i<n;i++)
	{
		c[i] -= a + b*i;
	}
    return;
}

void dxdy(DATAINFO *dataInfo)
{	//	perform five-point (centered four-point) differentiation
	float	*data = (float *) dataInfo->data;
	int		number = dataInfo->head.numSamples;
	double	sampInt = dataInfo->head.sampInt;
	int 	j;
	float 	aj, ajm1, ajm2, ajp1, ajp2, dif[2], fac1, fac2;
	double	step;

	float *Array, *Output;
	float *const Dif = &dif[0] - 1;

	Array = Output = data;
	step = sampInt/(1000000.);

	// Do the endpoints first
	Dif[1] = (Array[2] - Array[0])/(2.*step);
	Dif[2] = (Array[number-1] - Array[number-3])/(2.*step);

	// Perform five-point (centered four-point) on interior points
	fac1 = 2./(3.*step);
	fac2 = 1./(12.*step);

	ajm2 = Array[0];
	ajm1 = Array[1];
	aj = Array[2];
	ajp1 = Array[3];

	for( j = 1; j <= (number - 4); j++ )
	{
		ajp2 = Array[j + 3];
		Output[j] = fac1*(ajp1 - ajm1) - fac2*(ajp2 - ajm2);
		ajm2 = ajm1;
		ajm1 = aj;
		aj = ajp1;
		ajp1 = ajp2;
	}

	// Store endpoints into array
	Output[0] = Dif[1];
	Output[number - 3] = Dif[2];
}

void integrate(DATAINFO *dataInfo)
{	// trapezoidal method
	float	*data = (float *) dataInfo->data;
	int		number = dataInfo->head.numSamples;
	double	sampInt = dataInfo->head.sampInt;
	int		jy;
	float	hstep, prtint, totint;
	double	step;
	float	*Array, *Output;

	Array = Output = data;
	step = sampInt/(1000000.);

    hstep = 0.5*step;
    for( totint = 0., jy = 0; jy < (number-1); jy++ )
    {
		prtint = hstep*(Array[jy] + Array[jy+1]);
		totint = totint + prtint;
		Output[jy] = totint;
	}
}

gboolean xyzUVW(GHashTable *comp, int sensor)
{
	int			i, j, npts, start[TTLCOMPONENTS], compGroup;
	traceInfo	*trc[TTLCOMPONENTS];
	TRC_N_DATA	*comps[TTLCOMPONENTS], *compGrp;
	DATAINFO	*tForm[TTLCOMPONENTS];
	double		w2, w3, w6, aux=0, auy, auz, avx=0, avy, avz, awx=0, awy, awz;
	double		epochStart=0, epochEnd=G_MAXDOUBLE;
	float		*data[TTLCOMPONENTS], x, y, z;
	gboolean	startTime;
	
	memset(start, 0, sizeof(start));
	// determine component group
	for(i=0;i<TTLCOMPGROUPS;i++)
	{
		compGrp = g_hash_table_lookup(comp, compNm[i][0]);
		if (compGrp)
		{
			switch(i)
			{
				case ONE23:
				case ONE2Z:
					compGrp = g_hash_table_lookup(comp, compNm[i][2]);
					if (!compGrp)
						continue;
				break;
			}
			break;
		}
	}
	if (i==TTLCOMPGROUPS)
		return FALSE;			// no valid component group found, return
	compGroup = i;

	for(i=0, startTime=FALSE;i<TTLCOMPONENTS;i++)
	{
		comps[i] = g_hash_table_lookup(comp, compNm[compGroup][i]);
		if (!comps[i])
			return FALSE;
		trc[i] = comps[i]->trc;			// not provided by STN data
		tForm[i] = comps[i]->data;
		data[i] = (float *) tForm[i]->data;
		if (trc[i])
		{	// determine max(startTime) and min(endTime)
			if (trc[i]->data.head.epochs > epochStart)
				epochStart = trc[i]->data.head.epochs;
			if (trc[i]->data.head.epochs + trc[i]->data.head.length < epochEnd)
				epochEnd = trc[i]->data.head.epochs + trc[i]->data.head.length;
			startTime = TRUE;
		}
	}

	if (startTime)
	{
		for(i=0;i<TTLCOMPONENTS;i++)
		{	// determine the common starting point for all component traces
			start[i] = (epochStart - trc[i]->data.head.epochs) * (1000000./trc[i]->data.head.sampInt);
			for(j=0;j<start[i];j++)
				data[i][j] = 0;		// zero out the beginning of the data not being converted
		}
		npts = (epochEnd - epochStart) * (1000000./trc[0]->data.head.sampInt) + 1;
	}
	else
	{
		npts = tForm[0]->head.numSamples;
	}
	
	w2 = sqrt(2.);
	w3 = sqrt(3.);
	w6 = sqrt(6.);
	
	switch (sensor)
	{
		case STS2:
			aux = -2./w6;
			avx = 1./w6;
			awx = 1./w6;
		break;
		case TRILLIUM:
			aux = 2./w6;
			avx = -1./w6;
			awx = -1./w6;
		break;
	}

	auy = 0.;
	avy = w3/w6;
	awy = -w3/w6;

	auz = w2/w6;
	avz = w2/w6;
	awz = w2/w6;
	
	for (j=0; j<npts; j++)
	{
		x = data[E][j+start[E]];
		y = data[N][j+start[N]];
		z = data[Z][j+start[Z]];
		data[E][j] = aux*x + auy*y + auz*z;
		data[N][j] = avx*x + avy*y + avz*z;
		data[Z][j] = awx*x + awy*y + awz*z;
	}
	
	for(i=0;i<TTLCOMPONENTS;i++)
	{	// zero out the end of the data not being converted
		for(j=npts;j<tForm[i]->head.numSamples;j++)
			data[i][j] = 0;
	}

	return TRUE;
}

#ifndef	PQL_ONLY

int convolve_resp(float data[], int npts, double delta,
	       char *net, char *sta, char *loc, char *chan,
	       int startstage, int stopstage,
	       char *units, time_t resptime, int usedelay,
	       double taperfreq[], int *prewhiten,
	       int deconvflag, char *respfilename,
	       int totalsensflag, int verbose);

void deconvolve(DATAINFO *dataInfo, traceInfo *trc, char *respFile)
{
	static GStaticMutex dconvMutex = G_STATIC_MUTEX_INIT;
	float		*data = (float *) dataInfo->data;
	double		delta, taperFreq[4] = { -1.0, -1.0, -1.0, -1.0 };
	int			startStage, stopStage, prewhiten;
	time_t		resptime;
	char		*resploc;

	if (!respFile)
		return;

	g_static_mutex_lock(&dconvMutex);
	
	delta = trc->data.head.sampInt / 1000000.;
	if (!strcmp(trc->data.head.location, "--") ||
		!strlen(trc->data.head.location))
	{
		resploc = g_strdup("*");
	} else 
	{
		resploc = g_strdup(trc->data.head.location);
	}
	startStage = -1;							// all stages
	stopStage = 0;							// all stages
	prewhiten = 0;							// no pre-whitening
	resptime = trc->data.head.epochs;
	convolve_resp(data, dataInfo->head.numSamples, delta, 
					trc->data.head.network, trc->data.head.station, 
					resploc, trc->data.head.channel, startStage, stopStage,
					"DEF", resptime, FALSE, taperFreq, &prewhiten, TRUE,
					respFile, TRUE, FALSE);
	free(resploc);
	
	g_static_mutex_unlock(&dconvMutex);
}
#endif
