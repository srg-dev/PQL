#ifndef TAPESTRC_H
#define TAPESTRC_H

/*	TAPESTRC.H
	Created Nov 16, 1987 by Phil Davidson

	This module defines the data structures used
	for the transfering of information to tape.

	Modified 5/30/89 for new software formats
*/

struct pack_head_strc {			/* Packet Header Definition - 16b */
	char type[2];				/* Packet Type */
	char exper_num[1];		/* Experiment # in 2 digit BCD */
	char year[1];			/* Year in 2 digit BCD */
	char unit_id[2];		/* Staion Number in 4 digit BCD */
	char time[6];			/* 12 digit BCD DDDHHMMSSTHS */
	char bpp[2];			/* Bytes per Packet in 4 digit BCD */
	char seq_num[2];		/* Packet Sequence Number in 4 digit BCD */
};

struct channel_strc {			/* Channel Structure for Station & Channel Def - 146b */
	char number[2];			/* Channel number */
	char name[10];				/* Name */
	char azimuth[10];			/* Azimuth */
	char incline[10];			/* Inclination */
	char loc_x[10];				/* Location Coordinate X */
	char loc_y[10];				/* Location Coordinate Y */
	char loc_z[10];				/* Location Coordinate Z */
	char units_xy[4];			/* Location Units for X & Y */
	char units_z[4];			/* Location Units for Z */
	char preamp[4];				/* Preamp gain */
	char sens_mod[12];			/* Sensor Model Number */
	char sens_ser[12];			/* Sensor Serial Number */
	char cmmnt[40];				/* Comments */
	char volts[8] ;			/* volts per bit count  */
};

struct statn_chnl_def {			/* Station & Channel Definition - SC */
	struct pack_head_strc header;		/* Header Information */
	char ex_num[2];				/* Experiment Number */
	char ex_name[24];			/* Experiment Name */
	char ex_cmmmnt[40];			/* Experiment Comments */
	char st_num[4];				/* Station Number */
	char st_name[24];			/* Station Name */
	char st_cmmnt[40];			/* Comments on Station Set-up */
	char das_mod[12];			/* DAS Model Number */
	char das_ser[12];			/* DAS Serial Number */
	char ex_start[14];			/* Experiment Start Time */
	char clock_type[4];			/* Time Clock Type */
	char clock_ser[10];			/* Time Clock Serial Number */
	struct channel_strc channel[5];	        /* Channel Information */
	char reserved[92];			/* Reserved */
};

struct event_def {			/* Event Trigger Definition 162b */
	unsigned char 	trg_chan[16];	/* Trigger Channels */
	unsigned char 	min_chan[2];	/* Minimum Trigger Channels */
	unsigned char 	trg_wndw[8];	/* Trigger window */
	unsigned char 	pre_trg_len[8];	/* Pre-trigger Length */
	unsigned char 	pst_trg_len[8];	/* Post-trigger Length */
	unsigned char 	rec_len[8];	/* Record Length */
	unsigned char 	reserve[8];	/* Reserved */
	unsigned char 	sta_lng[8];	/* Length of Short Term Average */
	unsigned char 	lta_lng[8];	/* Length of Long Term Average */
	unsigned char 	mean_r[8];	/* Length of Average for mean removal */
	unsigned char 	trg_rat[8];	/* Trigger Ratio */
	unsigned char 	dtrg_rat[8];	/* Detrigger Ratio */
	unsigned char 	lta_hold[4];	/* LTA Hold Flag */
	unsigned char 	reserved[60];	/* Reserved */
};

struct timseq_def {			/* Time Trigger Definition 38b */
	unsigned char 	start_time[14];		/* Start Time YYYYDDDHHMMSS* */
	unsigned char 	rep_int[8];		/* Repeat Interval DDHHMMSS */
	unsigned char 	num_int[4];	    /* Number of Intervals */
	unsigned char 	pre_trg_len[8];		/* Pre-trigger Length */
	unsigned char 	rec_len[8];		/* Record Length */
	unsigned char	reserved[120] ;
};

struct timelist_def { 			/* TML trigger Def */
	unsigned char start_time1[14];
	unsigned char start_time2[14];
	unsigned char start_time3[14];
	unsigned char start_time4[14];
	unsigned char start_time5[14];
	unsigned char start_time6[14];
	unsigned char start_time7[14];
	unsigned char start_time8[14];
	unsigned char start_time9[14];
	unsigned char start_time10[14];
	unsigned char start_time11[14];
	unsigned char	rec_len[8] ;
};

struct level_def {			/* Level Trigger Definition 24b */
	unsigned char 	level[8];		/* Level */
	unsigned char 	pre_trg_len[8];		/* Pre-trigger Length */
	unsigned char 	rec_len[8];		/* Record Length */
	unsigned char	reserved[138] ;
};

struct rx_def {				/* Radio/External Trigger Definition 16b */
	unsigned char 	pre_trg_len[8];		/* Pre-trigger Length */
	unsigned char 	rec_len[8];		/* Record Length */
	unsigned char	reserved[146] ;
};

struct con_def {			/* Continuous Trigger Definition 8b */
	unsigned char 	rec_len[8];		/* Record Length */
	unsigned char	reserved[154] ;
};

struct crs_def {			/* Cross-trigger Definition 18b */
	unsigned char 	trg_stm[2];	/* Data stream to trigger of off */
	unsigned char 	pre_trg_len[8];		/* Pre-trigger Length */
	unsigned char 	rec_len[8];		/* Record Length */
	unsigned char	reserved[144] ;
};

struct strm_strc {				/* Stream Structure for use in Data Stream Def. - 230b */
	char number[2];				/* Data Stream Number */
	char name[24];				/* Data Stream Name */
	char channels[16];			/* Channels Included */
	char smp_rat[4];			/* Samples per Second */
	char dat_formt[2];			/* Data Format */
	char fltnum[8][2];			/* Filter Stage Numbers */
	char trg_typ[4];			/* Trigger Type */
	union            			/* Trigger Description */
	{
		struct event_def	event ;
		struct timseq_def	tims ;
		struct level_def	levl ;
		struct rx_def		radext ;
		struct con_def		cont ;
		struct crs_def		cross ;
	} trigger ;
};

struct d_strm_def {				/* Data Stream Definition - DS */
	struct pack_head_strc header;		/* Header Information */
	struct strm_strc stream[4];		/* Stream Information */
	char reserved[88];				/* Reserved */
};

struct filt_def {				/* Filter Definition - FD  */
	struct pack_head_strc header;		/* Header Information */
	char flt_num[2];			/* Filter Number */
	char dec_fct[2];			/* Decimation Factor */
	char num_coef[4];			/* Number of Coefficients */
	char delay[4];				/* Delay */
	char offset[4];				/* Offset Into List */
	char coef[124][8];			/* Filter Coefficients */
};

struct data_def {				/* Data Definition - DT */
	struct pack_head_strc header;		/* Header Information */
	char evnt_num[2];			/* Event Number in 4 digit BCD */
	char strm_num[1];			/* Data Stream Number in 2 digit BCD */
	char chn_num[1];			/* Channel Number in 2 digit BCD */
	char num_samples[2];			/* Number of Samples in 4 digit BCD */
	char data_format[2];			/* Data Format */
	char data[1000];				/* Data */
};

struct health_def {				/* State of Health Definition - SH */
	struct pack_head_strc header;		/* Header Information */
	char count[4];				/* # of bytes of info */
	char reserved[4];			/* Reserved */
	char info[1000];			/* Any Information */
};

typedef struct event_time_def {		/* Time Definition - EH and ET	*/
	char year[4] ;			/* year (ascii)			*/
	char day[3] ; 			/* day of year (ascii)		*/
	char hour[2] ; 			/* hour of trigger (ascii)	*/
	char min[2] ;			/* minute (ascii) 		*/
	char sec[2] ;			/* second (ascii)		*/
	char msec[3] ;			/* milliseconds (ascii)		*/
} EventTime;

struct event_trailer_def {		/* Pre-2.50 ET header		*/
	struct pack_head_strc header;	/* Header Information		*/
	char evnt_num[2] ;		/* Event Number in 4 digits BCD	*/
	char strm_num[1] ;		/* Stream Number		*/
	char reserve[5] ;		/* reserve			*/
	char trigger_time_msg[15];	/* text label			*/
	EventTime trigger_time;
	char reserved[969] ;
};

typedef struct event_trailer_new {	/* PASSCAL 2.50 - EH, ET header	*/
	struct pack_head_strc header;	/* Header Information		*/
	char evnt_num[2] ;			/* Event Number in 4 digits BCD	*/
	char strm_num[1] ;			/* Stream Number		*/
	char reserve[4] ;			/* reserve			*/
	char data_format[1];		/* data format in 2 digit BCD	*/
	char trigger_time_msg[33];	/* text label			*/
	char filler[7] ;			/* filler			*/
	char stream_name[24] ;		/* stream name from user	*/
	char sample_rate[4] ;		/* samples per second (ascii)	*/
	char trigger_type[4] ;		/* EVT, TIM, LEV, etc (ascii)	*/
	EventTime trigger_time;
	EventTime first_sample_time;
	EventTime detrigger_time;
	EventTime last_sample_time;
	char data_bit_weights[128];	/* (ascii)			*/
					/* volts/count;			*/
					/* 16 channels, 8 bytes/channel	*/
					/* <SP> if channel not in stream*/
    char true_bit_weights[128];
    char gain[16];
    char ADResolution[16];
    char FSA[16];
    char channelCode[64];
    char sensorFSA[16];
    char sensorVPU[96];
    char sensorUnits[16];
    char channelNumber[48];
    char reserved[156];
    char totalChannels[2];
    char comment[40];
    char digFiltList[16];
    char position[26];
    char RT120[80];
} EventTrailer;

typedef EventTrailer EventHeader;	/* EH header same as ET */
					/* EXCEPT		*/
					/* detrigger_time and	*/
					/* last_sample_time 	*/
					/* not supplied in EH	*/

struct calibration_def {		/* Calibration Definition - CD */
	struct pack_head_strc header ;	/* Header Information */
	char	start[14] ;		/* Start time DDDHHMMSS */
	char	rep_int[8] ;		/* Repeat Interval DDHHMMSS */
	char	num_int[4] ;		/* Number of Intervals  */
	char	length[8] ;		/* length of calibration in seconds */
	char	step[4] ;		/* step calibration ON/OFF    */
	char	step_period[8] ;	/* step period in seconds */
	char	step_size[8] ;		/* step size in seconds */
	char	step_ampl[8] ;		/* step amplitude in volts */
	char	step_output[4] ;	/* output to COIL/AMPLIFIER */
	char	freq[4] ;		/* freq calibration ON/OFF */
	char	freq_start[8] ;		/* start frequency in Hz */
	char	freq_stop[8] ;		/* stop frequency in Hz  */
	char	freq_ampl[8] ;		/* amplitude in volts  */
	char	freq_output[4] ;	/* output to COIL/AMPLIFIER */
	char	noise[4] ;		/* noise calibration ON/OFF */
	char	noise_ampl[8] ;		/* amplitude in volts */
	char	noise_output[4] ;	/* output to COIL/AMPLIFIER */
	char	reserve[894] ;
};

struct dstrm_def {		/* Data Stream Parameters Structure 214b */
	unsigned char 	number[2];		/* Data Stream Number */
	unsigned char 	name[24];		/* Data Stream Name */
	unsigned char 	ch_nums[16];		/* Data Stream Selected Channels Hex digit */
	unsigned char 	rate[4];		/* Data Stream Sample Rate */
	unsigned char 	dat_formt[2];		/* Data Stream Data Format */
	unsigned char 	trg_type[4];		/* Data Stream Trigger Number*/
	union {					/* Trigger Information */

		struct event_def 	evt;
		struct timseq_def tim;
		struct level_def 	lev;
		struct rx_def 		radext;
		struct con_def 	con;
		struct crs_def 	crs;
	} trigger;
};

struct smp_def {		/* User Sample Rate Definition Structure 20b */
	unsigned char 	rate_num[2];		/* Sample Rate Number */
	unsigned char 	num_stage[2];		/* Number of Stages */
	unsigned char 	fltnum[8][2];		/* 8 Filter Stage Numbers */
};

struct filter_def {		/* User Filter Definition Structure 4108b */
	unsigned char 	flt_num[2];		/* Filter Number */
	unsigned char 	dec_fct[2];		/* Decimation Factor */
	unsigned char 	num_coef[4];		/* Number of Coeficients */
	unsigned char 	delay[4];		/* Delay */
	unsigned char 	flt_coef[512][8];	/* Filter Coeficients */
};

struct wake_def {			/* Wake-up Sequence Definition 28b */
	unsigned char	seq_no[2] ;		/* Sequence # */
	unsigned char 	start_time[12];		/* Start Time YYYYDDDHHMM* */
	unsigned char 	pwr_dur[6];		/* Power-on Duration DDHHMM */
	unsigned char 	rep_int[6];		/* Repeat Interval DDHHMM */
	unsigned char 	num_int[2];		/* Number of Intervals */
};

struct mode_def {			/* Operating Mode Definition 4b */
	unsigned char 	pwr_stt[2];		/* Power State */
	unsigned char 	rec_mod[2];		/* Recording Mode */
};

struct wakeup_seq_def {	   /*  wake-up sequence packet description -- 9.3.6  */
	struct pack_head_strc header;		/* Header Information */
	struct mode_def mode;
	struct wake_def wake_info[8];       /* 244 bytes total  */
};

#endif /* TAPESTRC_H	*/
