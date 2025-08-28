#include <stdlib.h>
#include <string.h>
#include "pql_defines.h"
#include "pql_externs.h"
#include "gtk_pql_externs.h"

void get_trans_times(traceInfo *trcData, double user_x1, double user_x2, gboolean time, double absStart)
{
    float	length_of_trace;

    switch (time) 
	{
		case ABSOLUTE:
    		length_of_trace = trcData->data.dataInfo.head.numSamples / 
					(1000000. / trcData->data.head.sampInt);
    		if (((user_x1 + absStart) > 
    					(trcData->data.head.epochs + length_of_trace)) || 
					((user_x2 + absStart) < trcData->data.head.epochs)) 
			{
				trcData->spectra.beg = -1;
				break;	// transform window contains nothing for this trace
    		}

		    if ((user_x1 + absStart) < trcData->data.head.epochs) 
			{
				trcData->spectra.beg = 0;	// window starts before trace does
				trcData->spectra.fill_beg = 
					(trcData->data.head.epochs - (user_x1 + absStart));
		    }
		    else 
			{
				trcData->spectra.beg = 
					((user_x1 + absStart) - trcData->data.head.epochs);
				trcData->spectra.fill_beg = 0;	// window starts in middle of trace
		    }
		
		    if ((user_x2 + absStart) > (trcData->data.head.epochs + length_of_trace)) 
			{
				trcData->spectra.end = length_of_trace;	// window ends after trace 
				trcData->spectra.fill_end = (user_x2 + absStart) - 
						(trcData->data.head.epochs + length_of_trace);
		    }
		    else 
			{
				trcData->spectra.end = ((user_x2 + absStart) - trcData->data.head.epochs);
				trcData->spectra.fill_end = 0;	// window ends in middle of trace
		    }
		break;

		case RELATIVE:
		    trcData->spectra.fill_beg = 0;
		    length_of_trace = trcData->data.dataInfo.head.numSamples / 
					(1000000. / trcData->data.head.sampInt);
		
		    if (user_x1 > length_of_trace)
				trcData->spectra.beg = -1;
		    else
				trcData->spectra.beg = user_x1;
		
		    if (user_x2 > length_of_trace) 
			{
				trcData->spectra.end = length_of_trace;
				trcData->spectra.fill_end = user_x2 - length_of_trace;
		    }
		    else 
			{
				trcData->spectra.end = user_x2;
				trcData->spectra.fill_end = 0;
		    }
		break;
	}
	return;
}

void get_trans_data(traceInfo *trcData)
{
	int            num_samples, j, startSamp;
	float          *get_data, *read_flt;
	double         exp2(), pow(), sps;
	DATAINFO		*dataInfo;
	gint32	*ptr=(gint32*)trcData->data.dataInfo.data;
	short	*sptr=(short *)trcData->data.dataInfo.data;
	float	*lptr=(float*)trcData->data.dataInfo.data;
	double	*dptr=(double*)trcData->data.dataInfo.data;

    if (trcData->spectra.beg == -1)
      return;

    sps = (1000000. / trcData->data.head.sampInt);
    num_samples = ((trcData->spectra.end - trcData->spectra.beg) + 
					trcData->spectra.fill_beg) * sps;
    if (num_samples > trcData->data.dataInfo.head.numSamples)
    	num_samples = trcData->data.dataInfo.head.numSamples;

    for (j = 1; (int) pow((double) 2, (double) j) < num_samples; ++j);
    trcData->spectra.numPts = (int) pow((double) 2, (double) (j));
    
    if ((trcData->spectra.data = 
			(char *) calloc((unsigned) trcData->spectra.numPts + 2, sizeof(float))) == NULL) 
	{
		fprintf(stderr, "Out of MEMORY, cannot continue, exiting...\n");
		exit(-1);
    }
    
	get_data = (float *) trcData->spectra.data;
	trcData->spectra.avg = 0;
    
	startSamp = trcData->spectra.beg * sps;
    if (states.tForm.on && 
		trcData->data.tForm.data) 
	{
		dataInfo = &trcData->data.tForm;
		read_flt = (float *) dataInfo->data;
		for (j = 0; 
			 j < num_samples;
			 ++j)
		{
			get_data[j] = read_flt[startSamp+j];
			trcData->spectra.avg += (float) (get_data[j] / num_samples);
		}
	}
	else
	{
		for (j = 0; 
			 j < num_samples;
			 ++j)
		{
			DATAP(trcData->data.head.data_form, (startSamp+j), get_data[j]);
			trcData->spectra.avg += (float) (get_data[j] / num_samples);
		}
	}
   	trcData->spectra.data_end = (float *) (&get_data[j-1]); 

  	// demean the data
    for (j = 0; j < num_samples; ++j)
	{
		get_data[j] -= (float) trcData->spectra.avg;
	}
}

#define  PERCENT    .10

void taper(traceInfo *trcData)
{
    int		j, num_samples, ffl;
    float   *data_e, *data_s;
    double	cos(), sps, x;

	if (trcData->spectra.beg == -1 )
    	return;
   	sps = (1000000. / trcData->data.head.sampInt);
   	num_samples = ((trcData->spectra.end - trcData->spectra.beg) + 
					trcData->spectra.fill_beg + trcData->spectra.fill_end) * sps;
    if (num_samples > trcData->data.dataInfo.head.numSamples)
    	num_samples = trcData->data.dataInfo.head.numSamples;

	ffl = num_samples * PERCENT;
	data_e = trcData->spectra.data_end; 
	data_s = (float *) trcData->spectra.data; 
#if 0
	for (j = 0; j < ffl; ++j)
	{
    	value = .5 - .5 * cos((double) (3.1415926 * (float) (j) / (float) (ffl)));
    	data_s[j] *= value;
    	data_e[j*-1] *= value;
	}
#endif

	// use sin() taper
    x = (5*G_PI)/num_samples;
    for (j = 0; 
		 j < ffl; 
		 ++j, ++data_s, --data_e) 
	{
		*data_s *= sin(j*x);
		*data_e *= sin((num_samples-j)*x);
    }

#if 0
	{
		FILE *fp = fopen("/tmp/taper1.txt", "w");
		data_e = trcData->spectra.data_end; 
		data_s = (float *) trcData->spectra.data; 
		for (j = 0; 
			 j < ffl; 
			 ++j, ++data_s) 
		{
			fprintf(fp, "%g\n", *data_s);
		}
		fprintf(fp, "\n");		
		for (j = ffl-1; 
			 j >=0; 
			 --j, --data_e) 
		{
			fprintf(fp, "%g\n", *data_e);
		}
		fprintf(fp, "\n");		
		fclose(fp);
	}
#endif
}

SPECPT *spec_point(traceInfo *trcData, int s_x, graph *gr)
{
	double          x, xmin, ymin, ymax;
	float          *ptr, delta;
	int             skip, n_data, npts;
	int             pix_x0, pix_x1, pix_y0, pix_y1;
	double          exp();
	double          old_freq;
	double          old_amp;
	char			*udata;
	static SPECPT	specPt;

	memset(&specPt, 0, sizeof(SPECPT));

	npts = trcData->spectra.numPts;
	npts /= 2;
	udata = trcData->spectra.data;
	
	delta = (1./(2. * trcData->data.head.sampInt/1000000.)) / npts;
#if 0
-- replaced below using gr-> values directly, better
	xmin = states.spectra.plot.glob[SPEC_MINFREQ];
	ymin = states.spectra.plot.glob[SPEC_MINAMP];
	ymax = settings.spectra.display.yMax ? settings.spectra.display.yMax : FLT_MAX;
#endif
	
	xmin = PIX2USERX(gr, gr->pix_xmin);
	ymin = PIX2USERY(gr, gr->pix_ymax);
	ymax = PIX2USERY(gr, gr->pix_ymin);

	skip = (int) (xmin * 2 * npts * (trcData->data.head.sampInt / 1000000.));
	if (skip < 0)
		skip = 0;
	n_data = npts - skip;
	if (n_data < 2)
		return (&specPt);

	ptr = (float *) udata;
	ptr = (float *) (ptr + (skip * 2));
	
	x = xmin;
	
	old_freq = x;
	old_amp = (double) *ptr;
	
	pix_x0 = USER2PIXX(gr, x);
	
	if ((double) *ptr < ymin)
		pix_y0 = USER2PIXY(gr, ymin);
	else if ((double) *ptr > ymax)
		pix_y0 = USER2PIXY(gr, ymax);
	else
		pix_y0 = USER2PIXY(gr, (double) *ptr);
  
   // first line segment drawn is a special case, since the x
   // increment is not delta
  
	ptr = (float *) (ptr + 2);
	x += (double) delta;
	
	pix_x1 = USER2PIXX(gr, x);
	if ( /* (spec_plot_type == AMP) && */ ((double) *ptr < ymin))
		pix_y1 = USER2PIXY(gr, ymin);
	else if ((double) *ptr > ymax)
		pix_y1 = USER2PIXY(gr, ymax);
	else
		pix_y1 = USER2PIXY(gr, (double) *ptr);
  

	if (s_x <= pix_x0) 
	{
		specPt.trans_x = pix_x0;
		specPt.trans_y = pix_y0;
		
		specPt.trans_mouse_freq = old_freq;
		specPt.trans_mouse_amp = old_amp;
		return (&specPt);
	}      

	if (s_x <= pix_x1) 
	{
		if ((s_x - pix_x0 ) < (pix_x1 - s_x)) 
		{
			specPt.trans_x = pix_x0;
			specPt.trans_y = pix_y0;
			specPt.trans_mouse_freq = old_freq;
			specPt.trans_mouse_amp = old_amp;
			return (&specPt);
		}
		else 
		{
			specPt.trans_x = pix_x1;
			specPt.trans_y = pix_y1;
			specPt.trans_mouse_freq = x;
			specPt.trans_mouse_amp =(double) *ptr ;
			return (&specPt);
		}
	}

	pix_x0 = pix_x1;
	pix_y0 = pix_y1;
	n_data--;
	old_freq = x;
	old_amp = (double) *ptr;
	
	for (; n_data > 1; n_data--, pix_x0 = pix_x1, pix_y0 = pix_y1) 
	{
		ptr = (float *) (ptr + 2);
		x += (double) delta;
		pix_x1 = USER2PIXX(gr, x);
		if ( /* (spec_plot_type == AMP) && */ ((double) *ptr < ymin))
			pix_y1 = USER2PIXY(gr, ymin);
		else if ((double) *ptr > ymax)
			pix_y1 = USER2PIXY(gr, ymax);
		else
			pix_y1 = USER2PIXY(gr, (double) *ptr);

		if (s_x <= pix_x1) 
		{
			if ((s_x - pix_x0 ) < (pix_x1 - s_x)) 
			{
				specPt.trans_x = pix_x0;
				specPt.trans_y = pix_y0;
				
				specPt.trans_mouse_freq = old_freq;
				specPt.trans_mouse_amp = old_amp;
				return (&specPt);
			}
			else 
			{
				specPt.trans_x = pix_x1;
				specPt.trans_y = pix_y1;
				specPt.trans_mouse_freq = x;
				specPt.trans_mouse_amp =(double) *ptr ;
				
				return (&specPt);
			}
		}
	
		old_freq = x;
		old_amp = (double) *ptr;
		specPt.trans_mouse_freq = old_freq;
		specPt.trans_mouse_amp = old_amp;
	}
	return (&specPt);	
}

void specPoint(traceInfo *trcData, int s_x, pointID *ptr)
{
	SPECPT	*specPt;

	specPt = spec_point(trcData, s_x, &trcData->spectra.GR[states.currentScreen]);
	
	ptr->freqVal = specPt->trans_mouse_freq;
	ptr->minAmp = ptr->maxAmp = specPt->trans_mouse_amp;

	if (trcData->data.head.units == PQL_UNITS_VOLTS && 
		trcData->data.fileType == SEGY_DATA)
	{	// convert to VOLTS if necessary
		float	scale = trcData->data.head.scale_fac / (float) trcData->data.head.gainConst;
		ptr->minAmp *= scale;
		ptr->maxAmp *= scale;
	}

	ptr->xVal = specPt->trans_x;
	ptr->yVal = specPt->trans_y;
}

void transform_data(traceInfo *trcData, double minFreq, SPECSETTINGS *settings)
{
    int             j;
    float          *udata, delta, amp;
    double          sqrt(), pow(), /*minFreq,*/ maxFreq, curFreq, factor;

	if (trcData->spectra.beg == -1)
    	return;

	cfftr((float *) trcData->spectra.data, trcData->spectra.numPts);
	delta = trcData->data.head.sampInt / 1000000.;
	factor = ((double) (trcData->spectra.numPts * delta));
	
	trcData->spectra.ampMax = 0;
	
	maxFreq = settings->xMax ? settings->xMax : 
					(1./(2. * (trcData->data.head.sampInt / 1000000.)));

	for (j=0, udata=(float *) trcData->spectra.data; 
				j<=trcData->spectra.numPts; 
				j+=2)
	{
	    amp = (float) sqrt((double) ((double) (udata[j] * udata[j]) + (double) (udata[j+1] * udata[j+1])));
	//	    phase = atan2(*(data + 1), *data);  
	    udata[j] = amp * delta;
	//	    *(data + 1) = phase; 
	
		curFreq = ((double) j)/(2. * factor);
	
	    if ((curFreq > minFreq && curFreq < maxFreq) && 
		    	(udata[j] > trcData->spectra.ampMax))
		{	// what's the max for the displayed data?
	      trcData->spectra.ampMax = udata[j];
		}
	}			// end for(j=) 
}

void setSpecGlobals(double *global, gboolean first, traceInfo *trcData, SPECSETTINGS *settings)
{
	if (first)
	{
	    global[SPEC_MAXAMP] = G_MINDOUBLE;
    	global[SPEC_MINAMP] = G_MAXDOUBLE;
    	global[SPEC_MAXFREQ] = 0;
    }
    
	if (trcData->spectra.ampMax > global[SPEC_MAXAMP]) 
	{
	    global[SPEC_MAXAMP] = trcData->spectra.ampMax;
	}
		
	if ((trcData->spectra.ampMax/pow((double) 10., (double) settings->decades)) 
						< global[SPEC_MINAMP])
	{
	    global[SPEC_MINAMP] = trcData->spectra.ampMax / 
								pow((double) 10., (double) settings->decades);
	}
	
	if ((1./(2. * (trcData->data.head.sampInt / 1000000.))) > global[SPEC_MAXFREQ])
	{
	    global[SPEC_MAXFREQ] = 1./(2. * (trcData->data.head.sampInt / 1000000.));
	}
}
