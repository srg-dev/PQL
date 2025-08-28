/* Public header file for NANO_H	*/ 

#ifndef NANO_H
#define NANO_H

#include <sys/types.h>

#define MAXFILENAMELENGTH		128

typedef struct NmxTraceHeader		/* Y File Version 3		*/
   {
   u_short	usVersion;
   u_short usOffset;
   struct STNCHN
   {
	u_short	usStnKey;		/* unique record ID		*/
	/* primary key							*/
	u_char	achStation[6];
	u_char	achLocation[3];
	u_char	achChannel[4];
	/* general information						*/
	u_short	usLongWordOrder;	/* longword byte order (binary)	*/
	u_short	usWordOrder;		/* word byte order (binary)	*/
	char	szNetworkID[52];	/* network description		*/
	u_char	szSiteName[62];		/* descriptive name of site	*/
	u_char	szComment[32];		/* channel instrument comment	*/
	u_char	szSensorType[52];	/* Geotech S13 (SEED instrument)*/
	char	achDataFormat[7];	/* data rec. format (i.e. Steim)*/
	/* location orientation						*/
	float	rLatitude;		/* plus or minus		*/
	float	rLongitude;		/* ditto			*/
	float	rElevation;		/* plus above sea level		*/
	float	rDepth;			/* depth below ground level	*/
	float	rAzimuth;		/* nothing (degrees clockwise)	*/
	float	rDip;			/* 90 is vertical, right way up	*/
	/* operational parameters					*/
	float	rSensitivity;		/* sensitivity (per bit) or gain*/
	float	rSensFreq;		/* freq for the above sens/gain	*/
	guint32	ulRespKey;		/* system response record ID	*/
	u_char	szSensUnits[22];	/* sens units (SEED resp units	*/
	u_char	szCalibUnits[22];	/* units of calibration input	*/
	float	rSampleRate;		/* sampling rate (samples/sec)	*/
	float	rMaxClkDrift;		/* clock drift (sec/sample)	*/
	char	achChanFlags[27];	/* channel flags (SEED defin)	*/
	u_char	uchUpdateFlag;		/* N or U to flag error updates	*/
	double	rtmUpdateTime1;		/* info effective start time	*/
	double	rtmUpdateTime2;		/* info effective end time	*/
	/* DACQS information (NMX internal variables)			*/
	char	achGroupId[5];		/* group id			*/
	u_char	uchFep;			/* Front end processor ID number*/
	u_char	uchPort;		/* Port Number			*/
	u_char	uchChan;		/* Channel number		*/
	u_char	uchDataCode;		/* Type of filter in RD3	*/
	double	rtmTimeCorrect;		/* Time correction (longsec)	*/
	float	rSensorSensitivity;	/* volts / nm / sec		*/
	float	rRd3Sensitivity;	/* Volts / bit			*/
	u_short	usInstrumentID;		/* instrument ID code		*/
	/* database conversion						*/
	guint32	ulCommID;		/* database comment		*/
	double	rtmLoadDate;
   } stStnChn;


   struct SERIES
   {
	guint32	ulSeriesKey;		/* unique series record ID num	*/
	/* primary key							*/
	u_char	achStation[6];
	u_char	achLocation[3];
	u_char	achChannel[4];
	/* contents							*/
	double	rtmStartTime1;
	double	rtmEndTime1;
	guint32	ulNumSamples;		/* # samples in this series	*/
	gint32	IDCOffset;		/* data DC offset		*/
	gint32	IMaxAmplitude;		/* maximum +ve amplitude	*/
	gint32	IMinAmplitude;		/* minimum -ve amplitude	*/
	/* stores							*/
	u_char	szFileName[MAXFILENAMELENGTH];	/* name of this file	*/
	u_char	szFormatVersion[7];	/* e.g. SEED, YFILE		*/
	u_char	szFormat[7];		/* e.g. 2.0			*/
	u_short	usMachineFormat;	/* byte order 0=little 1=big	*/
	guint32	ulFileLocation;		/* format dep. start of data	*/
	u_short	usFileSubLocation;	/* format dep. start of data	*/
	/* in memory use						*/
	struct TRACE	*pstTrace;	/* pointer to Trace struct	*/
	struct SERIES	*pstNextSeriesAll;	/* next SERIES global	*/
	struct SERIES	*pstNextSeriesStn;	/* next SERIES station	*/
	/* database convention						*/
	guint32	ulCommID;		/* comment			*/
	double	rtmLoadDate;
} stSeries;
} NMXTRACE3;

typedef struct NmxTraceHeader1 {
  guint32   ulDataOffset;	/* Data offset				*/
  guint32   ulBitMapOffset;	/* Data Error bitmap offset. (not used)	*/
  guint32   ulNumSamples;	/* No. of samples			*/
  u_char   uchDataType;		/* Data format (not used)		*/
  char    achNetId[5];		/* Network Id				*/
  char    achSiteId[9];		/* Site Id				*/
  char    achGroupId[5];	/* Group Id				*/
  char    achChanID[5];		/* Channel Id				*/
  float   fLatitude;		/* Site latitude			*/
  float   fLongitude;		/* Site longitude			*/
  float   fElevation;		/* Site elevation			*/
  u_char   uchFep;		/* Front End Processor number		*/
  u_char   uchPort;		/* Port Number				*/
  u_char   uchChan;		/* Channel Number			*/
  guint32   ulBaud;		/* Communications baud rate FEP <=> RD3	*/
  u_short  usSampleRate;	/* Data sample rate			*/
  u_short  usDecimation;	/* FEP filter decimation		*/
  u_char   uchDataCode;		/* Type of software filter in RD3	*/
  u_char   uchFormat;		/* Data transmission protocol		*/
  gint32    lTimeCorSecs;		/* Time correction (sec) applied to data*/
  short   sTimeCorMs;		/* Time correction (ms) applied to data	*/

  /* Channel Config							*/
  char    achKeyResponse[5];	/* Generic response			*/
  float   fRd3Sensitivity;	/* Volts/bit				*/
  float   fInputResistance;	/* Ohms					*/

  /* Taken from Sensor Config						*/
  float   fSensitivity;		/* Volts.seconds/metre			*/
  char    achSensorType[21];	/* Seismometer make			*/
  float   fDamping;		/* Critical damping			*/
  float   fExtResistance;	/* Critical damping resitance		*/
  char    achUnits[21];		/* Units of sensitivity			*/
  guint32   ulTimeSecs;		/* Time  of first data sample in	*/
				/* seconds since January 1, 1970	*/
				/* (includeing leap years,		*/
				/* excluding leap seconds)		*/
  u_short  usTimeMs;		/*Time (ms) of first data sample	*/
} NMXTRACE1;

typedef struct RecHdr {
	char seq_num[6];	/* offset 0, sequence number "######"	*/
	char DHI_1;		/* offset 6, 1st data header ('D' = 68)	*/
	char DHI_2;		/* offset 7, 2nd data header (' ' = 32)	*/
	char stat_ID[5];	/* offset 8, station ID "#####"		*/
	char loc_ID[2];		/* offset 13, location ID "##"		*/
	char chan_ID[3];	/* offset 15, channel ID "###"		*/
	char reserved1[2];	/* offset 18, reserved			*/
	gint32 sec;		/* offset 20, same as ulTimeSec		*/
	short msec;		/* offset 24, same as usTimeMs		*/
	char reserved2[4];	/* offset 26, reserved			*/
	u_short sample_count;	/* offset 30, num. of samples in record	*/
	short samsec;		/* offset 32, sample rate		*/
	short s_factor;		/* offset 34, sample rate multiplier	*/
	char activity_flag;	/* offset 36, activity flags		*/
	char io_flag;		/* offset 37, I/O flags			*/
	char quality_flag;	/* offset 38, data quality flags	*/
	char n_blockettes;	/* offset 39, # blockettes which follow	*/
	gint32 time_cor;		/* offset 40, time correction		*/
	u_short d_offset;	/* offset 44, start of data		*/
	u_short b_offset;	/* offset 46, start of first blockette	*/
	char reserved3[16];	/* offset 48, reserved			*/
} RECHDR;

#endif /*NANO_H	*/ 
