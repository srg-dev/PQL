#include "pql_defines.h"
#include "pql_externs.h"
#include <math.h>
#ifndef MAC
#include <values.h>
#endif

static double          minDP = 0;
static double          maxDP = 0;

void trace_point1(traceInfo *trcData, graph *gr, double begin)
{
  int            dataFormat;
  int            i;
  gint32         *ptr=NULL;
  short          *sptr=NULL;
  float	         *lptr=NULL;
  double		 *dptr=NULL;
  double         val=0;
  char			 *udata;
  float			temp_float;
  int			temp_int_2, temp_int, dataS;
  DATAINFO		*dataInfo;
  int			inc = trcData->trace.plot.increment;

  temp_float=((1000000.0/trcData->data.head.sampInt) * (begin-trcData->data.head.epochs));
  temp_int_2 = ((ceil)(temp_float)); // this is the number of data

  if (states.tForm.on && 
	  trcData->data.tForm.data && 
	  states.currentScreen == MAIN)
  {
	dataInfo = &trcData->data.tForm; 
//	g_slist_nth_data(trcData->data.dataFilt,states.filter.active);
	udata = dataInfo->data;
	dataS = sizeof(float);
  }
  else
  {
	udata = trcData->data.dataInfo.data;
	dataS = dsizes[trcData->data.head.data_form];
  }

  temp_int = temp_int_2 * dataS;
  udata = (char *) (((long) udata) + temp_int);

  if (!states.tForm.on && 
	  trcData->data.tForm.data) 
  {
	  dataFormat = trcData->data.head.data_form;
      switch (dataFormat)
	  {
		case BIT32:
			ptr = (gint32 *) udata;
		break;
		
      case BIT16:
			sptr = (short *) udata;
		break;
		
      case MY_FLOAT:
			lptr = (float *) udata;
		break;

      case MY_DOUBLE:
			dptr = (double *) udata;
		break;
	  }
  }
  else
  {  
      lptr = (float *) udata;
	  dataFormat = MY_FLOAT;
  }

  DATAP(dataFormat, 0, val);
  minDP = maxDP = val;
  for(i=1; i<inc;i++)
  {
	DATAP(dataFormat, i, val);
	if (val < minDP)
	{
		minDP = val;
		continue;
	}
	if (val > maxDP)
	{
		maxDP = val;
	}
  }

  return;
}


void tracePoint(int rec_no, int xPix, pointID *ptr)
{
 	graph	*gr;
	float	scale;
	traceInfo *trcData;
	GSList	*trcList = (states.currentScreen == MAIN 
							? states.trace.traces[CURGRP] 
							: states.trace.selection);
//	int		trcRec = (states.currentScreen == MAIN 
//							? rec_no+states.trace.startRec 
//							: rec_no);

//	trcData = g_slist_nth_data(trcList, trcRec);
	trcData = g_slist_nth_data(trcList, rec_no);
	if (!trcData)
		return;
	memset(ptr, 0, sizeof(pointID));
  	gr = &trcData->trace.GR[states.currentScreen];
	trace_point1(trcData, gr, PIX2USERX(gr, xPix));

	ptr->epoch = PIX2USERX(gr, xPix);
	passcal_etoh(&ptr->timeVal, ptr->epoch);

	ptr->minAmp = minDP;
	ptr->maxAmp = maxDP;
	if (trcData->data.fileType == SEGY_DATA &&
		trcData->data.head.units == PQL_UNITS_VOLTS)
	{	// convert to VOLTS if necessary
		scale = trcData->data.head.scale_fac / (float) trcData->data.head.gainConst;
		ptr->minAmp *= scale;
		ptr->maxAmp *= scale;
	}

	ptr->xVal = xPix;
	ptr->yVal = USER2PIXY(gr, (maxDP+minDP)/2);
}
