#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "pql_defines.h"
#include "pql_externs.h"

#ifdef SOLARIS2
#include "libgen.h"
#endif

/* /usr/include/suds@ -> /local/passcal/prog/jrw/suds/include/ */
#include  "suds.h"


/* die routine often used when calling st_error */
void die(in)
   INTV  in;
{
   exit(in);
}

int
get_suds_hedr(fp_st, rec_no)
     SUDS_STREAM    *fp_st;
     int		rec_no;
{

  /********************************************************************************
    THIS ONLY READS ONE WAVEFORM STRUCT PER FILE - LATER VERSION WILL DO BELOW
    this procedure opens each SUDS file, finds each occurance of a waveform structure
    that contatins data, loads the data[], head[], sample[] and sample_rate[] arrays
    appropraitely, composes trace names using the PASSCAL segy file name conventions
    (hh.mm.ss.das#.C) and returns a list of these names to replace those in Files[]
    in name_traces and the number of traces found.
    
    ********************************************************************************/

  int                 i,j,k, numin;
  long	        julday();
  INT2		   *short_ptr, *tmp_s;
  INT4              yr, mo, da, hr, mn, jd, *long_ptr, *tmp_l;
  FLOAT8            sc;
  CHAR             *suds_struct_ptr;
  long		   *data_ptr;

  
  /*  When multiple traces is enabled - change 20 to MAXFILE (or something)  */
  SUDS_WAVEFORM    *waveform;
  SUDS_RECORDER    *recorder;
  SUDS_SIGNAL_PATH *signal_path;
  
  i=j=0;
  while(numin=st_get(&suds_struct_ptr,fp_st)!=EOF) {
    switch(type_of_structure(suds_struct_ptr)) {
    case WAVEFORMS:     waveform=(SUDS_WAVEFORM *)suds_struct_ptr;
      break;
    case RECORDERS:     recorder=(SUDS_RECORDER *)suds_struct_ptr;
      break;
    case SIGNAL_PATHS:  signal_path=(SUDS_SIGNAL_PATH *)suds_struct_ptr;
      break;
    default:  st_free(suds_struct_ptr);
    }
  }
  
  
  
  
  /* Fill in PQL header info  */
  if ((data[rec_no]->data.head = (struct generic_head *)
       calloc(sizeof(struct generic_head), sizeof(char))) == NULL) {

    free_all_structures(waveform, recorder, signal_path);
    
    
    return (-1);
  }
  /*	data[rec_no]->data.head->data_form = get_size_of(recorder->storage_type) == 4 ? BIT32 : BIT16; */
  switch (type_of_data(waveform))  {
  case IN4:
    data[rec_no]->data.head->data_form = BIT32;
    break;
  case IN2:
    data[rec_no]->data.head->data_form = BIT16; 
    break; 
  case FL4:
    data[rec_no]->data.head->data_form = MY_FLOAT; 
    break; 
  default:
    data[rec_no]->data.head->data_form = BIT32; 
    break; 
  }

  /*   if (waveform->from_time != 0.0)  */
  /*     decode_mstime(waveform->from_time, &yr, &mo, &da, &hr, &mn, &sc); */
  /*   else */
  /*     decode_mstime(waveform->nominal_time, &yr, &mo, &da, &hr, &mn, &sc);*/
  /*   decode_mstime(waveform->nominal_time, &yr, &mo, &da, &hr, &mn, &sc); */
  
  if (waveform->from_time != MINTIME)
    decode_mstime(waveform->from_time, &yr, &mo, &da, &hr, &mn, &sc);
  else
    decode_mstime(waveform->nominal_time, &yr, &mo, &da, &hr, &mn, &sc);

  jd = get_julday(mo,da,yr);
  data[rec_no]->data.head->year = (short) yr;
  data[rec_no]->data.head->day = (short) jd;
  data[rec_no]->data.head->hour = (short) hr;
  data[rec_no]->data.head->minute = (short) mn;
  data[rec_no]->data.head->second = (short) (int) sc;
  /*   data[rec_no]->data.head->m_secs = (short) ( (sc - data[rec_no]->data.head->second) * */
  /*   1000); */
  data[rec_no]->data.head->m_secs = (short)((rint)( ( (sc - data[rec_no]->data.head->second) * 1000.0)));
  data[rec_no]->data.head->gainConst = (short) recorder->gain;
  data[rec_no]->data.head->scale_fac = (float) recorder->conv_2_mvolts;
  /* force the calculation of mean by setting it to zero  */
  data[rec_no]->data.head->mean = 0.0;
  data[rec_no]->data.head->max = (float) waveform->max_val_data;
  data[rec_no]->data.head->min = (float) waveform->min_val_data;
  

  if (((int) xv_get(P_label_trace, PANEL_VALUE)) == LABEL_HEADER) {
    
    sprintf((data[rec_no]->data.head->descr1), "%i, %03i %02i:%02i",
	    data[rec_no]->data.head->year,
	    data[rec_no]->data.head->day,
	    data[rec_no]->data.head->hour,
	    data[rec_no]->data.head->minute);
    /*
       sprintf((data[rec_no]->data.head->descr2), "ID: %5.5s, C: %i",
       recorder->serial_number,
       signal_path->channel_number);
       */
    sprintf((data[rec_no]->data.head->descr2), "ID: %5.5s",
	    recorder->serial_number);

  }
  else {
    
    strncpy(data[rec_no]->data.head->descr1,data[rec_no + startRec]->data.fileName, 200);
    
    if ( ((int) xv_get(P_label_trace, PANEL_VALUE)) == LABEL_FILE_SHORT) {
      
      
#ifdef SOLARIS2
      strncpy(data[rec_no]->data.head->descr1,basename(data[rec_no + startRec]->data.fileName), 200);
#endif
      
    } 
    
    sprintf((data[rec_no]->data.head->descr2),"\0");
    

  }
  
  sample_rate[rec_no] = 1e6/waveform->nom_dig_rate;
  
  data[rec_no]->trace.samples = waveform->data_length;

  /*   st_free(waveform); */
  /*   st_free(recorder); */
  /*   st_free(signal_path); */

  free_all_structures(waveform, recorder, signal_path);

  return (0);
  
}

int
get_suds_data(fp_st, rec_no)
     SUDS_STREAM    *fp_st;
     int		rec_no;

{

  /********************************************************************************
    THIS ONLY READS ONE WAVEFORM STRUCT PER FILE - LATER VERSION WILL DO BELOW
    this procedure opens each SUDS file, finds each occurance of a waveform structure
    that contatins data, loads the data[], head[], sample[] and sample_rate[] arrays
    appropraitely, composes trace names using the PASSCAL segy file name conventions
    (hh.mm.ss.das#.C) and returns a list of these names to replace those in Files[]
    in name_traces and the number of traces found.

    ********************************************************************************/

  INT2 *t_s_p; /* tmp short pointer */
  
  int                 i,j,k, numin;
  long	        julday();
  INT2		   *short_ptr, *tmp_s;
  INT4              yr, mo, da, hr, mn, jd, *long_ptr, *tmp_l;
  FLOAT8            sc;
  CHAR             *suds_struct_ptr;
  long		   *data_ptr;
  /*  When multiple traces is enabled - change 20 to MAXFILE (or something)  */
  SUDS_WAVEFORM    *waveform;
  SUDS_RECORDER    *recorder;
  SUDS_SIGNAL_PATH *signal_path;
  
  i=j=0;
  while(numin=st_get(&suds_struct_ptr,fp_st)!=EOF) {
    switch(type_of_structure(suds_struct_ptr)) {
    case WAVEFORMS:     waveform=(SUDS_WAVEFORM *)suds_struct_ptr;
      break;
    case RECORDERS:     recorder=(SUDS_RECORDER *)suds_struct_ptr;
      break;
    case SIGNAL_PATHS:  signal_path=(SUDS_SIGNAL_PATH *)suds_struct_ptr;
      break;
    default:  st_free(suds_struct_ptr);
    }
  }
  
  


  /* Fill in PQL header info  */
  if ((data[rec_no]->data.head = (struct generic_head *)
       calloc(sizeof(struct generic_head), sizeof(char))) == NULL) {
    free_all_structures(waveform, recorder, signal_path);
    return (-1);
  }
  /*	data[rec_no]->data.head->data_form = get_size_of(recorder->storage_type) == 4 ? BIT32 : BIT16; */
  switch (type_of_data(waveform))  {
  case IN4:
    data[rec_no]->data.head->data_form = BIT32;
    break;
  case IN2:
    data[rec_no]->data.head->data_form = BIT16; 
    break; 
  case FL4:
    data[rec_no]->data.head->data_form = MY_FLOAT; 
    break; 
  default:
    data[rec_no]->data.head->data_form = BIT32; 
    break; 
  }
  /*  
     decode_mstime(waveform->from_time, &yr, &mo, &da, &hr, &mn, &sc);
     if (waveform->from_time != 0.0) 
     decode_mstime(waveform->from_time, &yr, &mo, &da, &hr, &mn, &sc);
     else
     decode_mstime(waveform->nominal_time, &yr, &mo, &da, &hr, &mn, &sc);
     decode_mstime(waveform->nominal_time, &yr, &mo, &da, &hr, &mn, &sc);
     */
  
  if (waveform->from_time != MINTIME)
    decode_mstime(waveform->from_time, &yr, &mo, &da, &hr, &mn, &sc);
  else
    decode_mstime(waveform->nominal_time, &yr, &mo, &da, &hr, &mn, &sc);


  jd = get_julday(mo,da,yr);
  data[rec_no]->data.head->year = (short) yr;
  data[rec_no]->data.head->day = (short) jd;
  data[rec_no]->data.head->hour = (short) hr;
  data[rec_no]->data.head->minute = (short) mn;
  data[rec_no]->data.head->second = (short) (int) sc;
  data[rec_no]->data.head->m_secs = (short)((rint)( ( (sc - data[rec_no]->data.head->second) * 1000.0)));
  data[rec_no]->data.head->gainConst = (short) recorder->gain;
  data[rec_no]->data.head->scale_fac = (float) recorder->conv_2_mvolts;
  /* force the calculation of mean by setting it to zero  */
  data[rec_no]->data.head->mean = 0.0;
  data[rec_no]->data.head->max = (float) waveform->max_val_data;
  data[rec_no]->data.head->min = (float) waveform->min_val_data;
  
  
  if (((int) xv_get(P_label_trace, PANEL_VALUE)) == LABEL_HEADER) {
    
    sprintf((data[rec_no]->data.head->descr1), "%i, %03i %02i:%02i",
	    data[rec_no]->data.head->year,
	    data[rec_no]->data.head->day,
	    data[rec_no]->data.head->hour,
	    data[rec_no]->data.head->minute);
    /*
       sprintf((data[rec_no]->data.head->descr2), "ID: %5.5s, C: %i",
       recorder->serial_number,
       signal_path->channel_number);
       */
    sprintf((data[rec_no]->data.head->descr2), "ID: %5.5s",
	    recorder->serial_number);
    
  }
  else {
    
    strncpy(data[rec_no]->data.head->descr1, data[rec_no + startRec]->data.fileName, 200);
    sprintf((data[rec_no]->data.head->descr2), "\0");
  }
  
  sample_rate[rec_no] = 1e6/waveform->nom_dig_rate;
  
  data[rec_no]->trace.samples = waveform->data_length;
  size_of_data[rec_no]->data.data = data[rec_no]->trace.samples;
  
  data_ptr = (long *) pointer_to_data(waveform);
  /*     data[rec_no]->data.data =  (char *) data_ptr; */

  switch (data[rec_no]->data.head->data_form)  {
  case BIT32:
    if ((data[rec_no]->data.data = (char *) calloc((unsigned) data[rec_no]->trace.samples, 4)) == NULL) {
      free_all_structures(waveform, recorder, signal_path);
      return(-1);
    }
    memcpy(data[rec_no]->data.data, data_ptr, data[rec_no]->trace.samples*4);
    /*
       data[rec_no]->data.data =  (char *) data_ptr;
       
       
       tmp_l = long_ptr;
       for (i=0; i< data[rec_no]->trace.samples; i++, tmp_l++) {
       *tmp_l = (long) data_ptr[i];
       printf("%d\n", data[rec_no]->data.data[i]); 
       }
       data[rec_no]->data.data = (char *) long_ptr;
       for(i=0; i<data[rec_no]->trace.samples; i++)  
       printf("%d\n", data[rec_no]->data.data[i]);
       */
    
    break;
    
  case BIT16:
    
    if ((data[rec_no]->data.data = (char *) calloc((unsigned) data[rec_no]->trace.samples, 2)) == NULL) {
      free_all_structures(waveform, recorder, signal_path);
      return(-1);
    }
    
    memcpy(data[rec_no]->data.data, data_ptr, data[rec_no]->trace.samples*2);
    
    /*
       tmp_s = short_ptr;
       for (i=0; i< data[rec_no]->trace.samples; i++, tmp_s++) { 
       *tmp_s = (short) data_ptr[i];
       t_s_p = data_ptr + i;
       t_s_p++;
       tmp_s++;
       i++;
       *tmp_s = *t_s_p;
       }
       data[rec_no]->data.data = (char *) short_ptr;
       
       data[rec_no]->data.data =  (char *) data_ptr;
       */
     break;
  }
  
  /*   st_free(waveform); */
  /*   st_free(recorder); */
  /*   st_free(signal_path); */
  
  free_all_structures(waveform, recorder, signal_path);

	data[rec_no]->data.dataInfo.head.data_form = data[rec_no]->data.head.data_form;
	data[rec_no]->data.dataInfo.head.sampInt = data[rec_no]->data.head.sampInt;
	data[rec_no]->data.dataInfo.head.secsPsamp = (data[rec_no]->data.head.sampInt/1000000.);

	return (0);
}

int free_all_structures(waveform, recorder, signal_path)
     SUDS_WAVEFORM    *waveform;
     SUDS_RECORDER    *recorder;
     SUDS_SIGNAL_PATH *signal_path;
{

  st_free(waveform);
  st_free(recorder);
  st_free(signal_path);

  return SUCCESS;
}

