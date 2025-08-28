#ifndef __DR100
#define __DR100

/*===========================================================================*/
/* SEED reader     |               dr100.h                 |     header file */
/*===========================================================================*/

/*
	Name:		dr100.h
	Purpose:	structure for header of a DR100 data file
	Usage:		#include "dr100.h"
	Input:		not applicable
	Output:		not applicable
	Warnings:	not applicable
	Errors:		not applicable
	Called by:	output_data
	Calls to:	none
	Algorithm:	not applicable
	Notes:
	Problems:	none known
	References:	O'Neill, D. (1987).  IRIS Interim Data Distribution Format
					(SAC ASCII), Version 1.0 (12 November 1987).  Incorporated
					Research Institutions for Seismology, 1616 North Fort Myer
					Drive, Suite 1440, Arlington, Virginia 22209.  11 pp.
			Tull, J. (1987).  SAC User's Manual, Version 10.2, October 7,
					1987.  Lawrence Livermore National Laboratory, L-205,
					Livermore, California 94550.  ??? pp.
	Language:	C, hopefully ANSI standard
	Author:		Peggy Hellweg
	Revisions:	 2/ 2/94
			 3/17/94	Remove #include <float.h> because brkseis20.berkeley.edu can't find it!!!
			10/26/98	Add integer header entries for no. of zeros (=71) and poles (=72) and real
					   header entries for the first 5 complex zeros (=71) and poles (=81) for
					   transducer response (from RDSEED program).
			 4/ 3/2000	Fix bug introduced by 10/26/98: remove rblank90.  Note: the DR100 files
					   produced by RDSEED are correct; RDSEED does not use any dr100_real_hdr
					   fields after rblank90, and output_dr100() only writes 128 values.
			 5/10/2000	Define INULL as (-32767-1) so it is a short, not a long.
					Define RNULL with F suffix so it is a float, not a double.
					Expand list of values for integer header clocktype (=252).
			 3/7/2001	Define PCGEOS_MAGIC_NUMBER to identify Ed Cranswick's PC-format (IEEE) files.
			 3/14/2001	Restore #include <float.h> and hope brkseis20.berkeley.edu can find it now.
					Let enclosing program #define INULL and RNULL.
			 4/11/2001	Undo #include <float.h> from 3/14/2001: IEEE FLT_MAX overflows when converted
					   to VAX format.  Programs that #include "dr100.h" should conditionally
					   #undef FLT_MAX #ifdef FLT_MAX before #include'ing "dr100.h"
*/

struct	dr100_int_hdr {

        short	ninthdr;					/*     1    Number of extra integer header blocks */
	short	naschdr; 					/*     2    Number of ASCII header blocks */
	short	iundef;						/*     3    "Undefined" integer value (INULL) */
        short	datatype;					/*     4    Data type: positive=real, negative=integer, */
								/*	      ABS = bytes/sample; except 1 = R*4, INULL = I*2 */
#define	DATATYPE_REAL	 1
#define	DATATYPE_INT	-1
#define	DATATYPE_REAL4	 4
#define	DATATYPE_INT2	-2
#define	DATATYPE_INT4	-4
        short	star;						/*     5    If 1, then (*) parameters are defined */
	short	iblank6, iblank7, iblank8, iblank9;		/*          Empty (header slots 6 through 9) */
	short	year, day, hour, minute, second, msec, usec;	/*  10..16  Zero time of file: year, day, hour, min, sec, msec, usec */
	short	tick1;						/*    17    Sample number of first tick mark (DR100) */
	short	tickthresh;					/*    18    Detection amplitude of tick mark (counts) */
	short	nticks;						/*    19    Number of tick marks detected */
	short	serialno;					/*    20    Serial number of recording unit */
	short	eventno;					/*    21    Event sequence number on tape */
	short	fileno;						/*    22    Sequence number of file on tape */
	short	iblank23, iblank24, iblank25, iblank26;		/*          Empty (header slots 23 through 26) */
	short	nfirchan;					/*    27    First active channel number recorded on unit */
	short	nactchan;					/*    28    Actual channel number as recorded on unit */
	short	ntotchan;					/*    29    Total number of channels recorded on unit */
	short	ntotcomp;					/*    30    Total number of components recorded on unit with */
								/*	      this component's station name */
	short	ndatablocks;					/*    31    Number of data blocks (excludes headers) */
	short	lastindex;					/*    32    Index (from 1) of last sample in last data block */
        short	blocksize;					/*    33    Block size (bytes) */
        short	programname;					/*    34    Playback program: 1=RDGeos, 2=AFTape, ... */
        short	programversion;					/*    35    Playback program version */
	short	programsubversion;				/*    36    Playback program sub-version */
	short	recordertype;					/*    37    Recorder type: 1-GEOS, 2=DR100, ... */
	short	recorderversion;				/*    38    Recorder version */
	short	recordersubversion;				/*    39    Recorder sub-version */
	short	sensorno;					/*    40    Sensor unit serial number */
	short	vertical;					/*    41    (*) Vertical orientation (degrees)*/
	short	horizontal;					/*    42    (*) Horizontal orientation (degrees)*/
	short	sensormodel[7];					/*  43..49  ASCII Sensor model (14 characters) */
	short	nlocnum;					/*    50    Location number (GEOS) */
	short	ntape;						/*    51    Experiment or tape number */
	short	ntrig;						/*    52    Trigger algorithm type */
	short	nsta;						/*    53    Trigger STA (tenths of seconds) */
	short	nlta;						/*    54    Trigger LTA (seconds) */
	short	nrat;						/*    55    Trigger ratio STA/LTA (2**) */
	short	ntrcomp;					/*    56    Trigger component */
	short	npreevent;					/*    57    Pre-event memory size (tenths of seconds) */
	short	npostevent;					/*    58    Post-trigger duration (seconds) */
	short	iblank59;					/*          Empty (header slot 59) */
	short	iblank60, iblank61, iblank62, iblank63, iblank64;/*         Empty (header slots 60 through 64) */
	short	iblank65, iblank66, iblank67, iblank68, iblank69;/*         Empty (header slots 65 through 69) */
	short	iblank70;					/*          Empty (header slot  70) */
	short	nzeros;						/*    71    No. of complex zeros in transducer response (from RDSEED) */
	short	npoles;						/*    72    No. of complex poles in transducer response (from RDSEED) */
	short	iblank73, iblank74;				/*          Empty (header slots 73 through 74) */
	short	iblank75, iblank76, iblank77, iblank78, iblank79;/*         Empty (header slots 75 through 79) */
	short	iblank80, iblank81, iblank82, iblank83, iblank84;/*         Empty (header slots 80 through 84) */
	short	iblank85, iblank86, iblank87, iblank88, iblank89;/*         Empty (header slots 85 through 89) */
	short	iblank90, iblank91, iblank92, iblank93, iblank94;/*         Empty (header slots 90 through 94) */
	short	iblank95, iblank96, iblank97, iblank98, iblank99;/*         Empty (header slots 95 through 99) */
	short	iblank100;					/*          Empty (header slot 100) */
	short	bugger[100];					/* 101..200 (*) ASCII bugger processing history (200 characters) */
	short	iblank201, iblank202, iblank203, iblank204;	/*          Empty (header slots 201 through 204) */
	short	iblank205, iblank206, iblank207;		/*          Empty (header slots 205 through 207) */
	short	ndir;						/*   208    Directory number (study I.D. no.) */
	short	nsdir;						/*   209    Sub-Directory # (or tape-set #) */
	short	filename[7];					/* 210..216 ASCII file name (14 characters) */
	short	studyname[3];					/* 217..219 ASCII study name (6 characters) */
	short	iblank220, iblank221, iblank222, iblank223;	/*          Empty (header slots 220 through 223) */
	short	iblank224, iblank225, iblank226, iblank227;	/*          Empty (header slots 224 through 227) */
	short	iblank228, iblank229, iblank230, iblank231;	/*          Empty (header slots 228 through 231) */
	short	iblank232, iblank233, iblank234, iblank235;	/*          Empty (header slots 232 through 235) */
	short	iblank236, iblank237, iblank238, iblank239;	/*          Empty (header slots 236 through 239) */
	short	iblank240, iblank241, iblank242, iblank243;	/*          Empty (header slots 240 through 243) */
	short	iblank244, iblank245, iblank246, iblank247;	/*          Empty (header slots 244 through 247) */
	short	iblank248, iblank249, iblank250, iblank251;	/*          Empty (header slots 248 through 251) */
	short	clocktype;					/*   252    Clock type: 0=None, 1=WWVB, 2=External(Master), */
								/*	      3=Manual, 4=OMEGA, 5=GOES, 6=GPS, INULL=Unknown */
	short	eventtype;					/*   253    Event type: 0=Continuous, 1=Trigger, 2=Preset, */
								/*	      3=Calibration, 4=Amplifier calibration, */
								/*	      5=Sensor calibration */
#define	EVENT_CONT	0
#define	EVENT_TRG	1
#define	EVENT_PRE	2
#define	EVENT_CAL	3
#define	EVENT_AMP	4
#define	EVENT_SENS	5
	short	motion;						/*   254    Motion type: 1=Acceleration, 2=Velocity, */
								/*	      3=Displacement, 50=Volumetric strain */
#define	MOTION_ACC	 1
#define	MOTION_VEL	 2
#define	MOTION_DISP	 3
#define	MOTION_VOL	50
	short	componentno;					/*   255    Component number (1-3=Acceleration, 4-6=Velocity, */
								/*	      7-9=Displacement, 1,4,7:Vertical) */
#define	COMPONENT_ACC	1
#define	COMPONENT_VEL	4
#define	COMPONENT_DISP	7
#define	COMPONENT_Z	0
#define	COMPONENT_N	1
#define	COMPONENT_E	2
	short	nsamples;					/*   256    Number of data samples (>32767, use I031/I032) */
};

struct  dr100_real_hdr {

        float	nrealhdr;					/*     1    Number of extra real header blocks */
	float	rundef;						/*     2    "Undefined" real value (RNULL) */

#define PCGEOS_MAGIC_NUMBER 0xF149F2CA				/* Ed Cranswick uses his own version of the PCGEOS playback       */
								/* program which writes pseuso-DR100 format files in the native   */
								/* PC data format (IEEE).  The only reliable way to identify      */
								/* these files is to look for the RNULL "Undefined Value" he used, */
								/* which has the hexadecimal code 0xF149F2CA.                     */

	float	rblank3, rblank4;				/*          Empty (header slots 3 through 4) */
        float	samplerate;					/*     5    Sample rate (no. per second) */
	float	samplelag;					/*     6    (*) Component sample lag (seconds) */
	float	rblank7, rblank8, rblank9;			/*          Empty (header slots 7 through 9) */
	float	rblank10, rblank11, rblank12, rblank13, rblank14;/*         Empty (header slots 10 through 14) */
	float	rblank15, rblank16, rblank17, rblank18, rblank19;/*         Empty (header slots 15 through 19) */
	float	rblank20, rblank21, rblank22, rblank23, rblank24;/*         Empty (header slots 20 through 24) */
	float	rblank25, rblank26, rblank27, rblank28, rblank29;/*         Empty (header slots 25 through 29) */
	float	rblank30, rblank31, rblank32, rblank33, rblank34;/*         Empty (header slots 30 through 34) */
	float	rblank35, rblank36, rblank37, rblank38;		/*          Empty (header slots 35 through 38) */
	float	transtype[1];					/*    39    (*) ASCII transducer type (4 characters): ACC, VEL, DISP, ... */
	float	latitude;					/*    40    (*) Latitude (degrees N) */
	float	xoffset;					/*    41    Local X coordinate (meters) */
	float	longitude;					/*    42    (*) Longitude (degrees E) */
	float	yoffset;					/*    43    Local Y coordinate (meters) */
	float	elevation;					/*    44    (*) Elevation (meters) */
	float	zoffset;					/*    45    Local Z coordinate (meters depth below surface) */
	float	digconst;					/*    46    (*) Digitizing constant (counts/volt) */
	float	aacorner;					/*    47    (*) Anti-alias filter corner frequency */
	float	aapoles;					/*    48    (*) Poles of AAF (roll-off = 6dB/octave/pole) */
	float	resonance;					/*    49    (*) Transducer natural frequency */
	float	damping;					/*    50    (*) Transducer damping coefficient (fraction of critical) */
	float	coilconst;					/*    51    (*) Transducer coil constant (volts/motion unit) */
	float	ampgain;					/*    52    (*) Amplifier gain (dB) */
	float	preampgain;					/*    53    (*) Preamplifier gain (dB) */
	float	rblank54;					/*          Empty (header slot 54) */
	float	rblank55, rblank56, rblank57, rblank58, rblank59;/*         Empty (header slots 55 through 59) */
	float	clockcorr;					/*    60    (*) Clock correction (seconds, subtract to get true time) */
	float	clocksec;					/*    61    Seconds since last clock correction */
	float	voltage;					/*    62    Voltage */
	float	trigratio;					/*    63    Desired trigger ratio STA/LTA (=2**I055) */
	float	sta;						/*    64    Actual value of STA at trigger */
	float	lta;						/*    65    Actual value of LTA at trigger */
	float	maxratio;					/*    66    Maximum value of STA/LTA during event */
	float	rblank67, rblank68, rblank69;			/*          Empty (header slots 67 through 69) */
	float	rblank70;					/*          Empty (header slot  70) */
	float	zeros[10];					/*  71..80  Complex zeros (real/imaginary pairs) of transducer response (from RDSEED) */
	float	poles[10];					/*  81..90  Complex poles (real/imaginary pairs) of transducer response (from RDSEED) */
	float	rblank91, rblank92, rblank93, rblank94;		/*          Empty (header slots 91 through 94) */
	float	rblank95, rblank96, rblank97, rblank98, rblank99;/*         Empty (header slots 95 through 99) */
       	float	rblank100, rblank101, rblank102;		/*          Empty (header slots 100 through 102) */
	float	agramhist;					/*   103    AGRAM history flag (data have been converted to "true" ground motion) */
	float	rblank104, rblank105, rblank106, rblank107;	/*          Empty (header slots 104 through 107) */
	float	rblank108, rblank109, rblank110, rblank111;	/*          Empty (header slots 108 through 111) */
	float	rblank112, rblank113, rblank114, rblank115;	/*          Empty (header slots 112 through 115) */
	float	rblank116, rblank117, rblank118, rblank119;	/*          Empty (header slots 116 through 119) */
	float	rblank120, rblank121, rblank122, rblank123;	/*          Empty (header slots 120 through 123) */
	float	rblank124, rblank125, rblank126, rblank127;	/*          Empty (header slots 124 through 127) */
	float	rblank128;					/*          Empty (header slot 128) */
};

/* True/false definitions */

#ifndef TRUE
#define TRUE ( 0 == 0 )
#endif
#ifndef FALSE
#define FALSE ( 0 != 0 )
#endif

/* Define integer and real null values */

#ifndef INULL
#define INULL (-32767-1) /* -32768 is long, -32767-1 is short */
#endif

#ifndef RNULL
#ifndef FLT_MAX
#define FLT_MAX 1e37F    /* Watch out: w/o F suffix, RNULL is double */
#endif
#define RNULL (-FLT_MAX)
#endif

#endif /* #ifndef __DR100 */
