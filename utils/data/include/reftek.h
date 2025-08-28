/*   ref2segy.h   */

#ifndef __REFTEK__H
#define __REFTEK__H

#include "tapestrc.h"

// RT-130 Defines
#define RT130_PKT_SIZE 1024
#define RT130_HEADER_SIZE	24
#define RT130_C0_MAX_SAMPS	892
#define RT130_C2_MAX_SAMPS	1561
#define DEFAULT_MSEED_BLOCKSIZE 1024
#define	AD		0x4144
#define	CD		0x4344
#define	DR		0x4452
#define	DS		0x4453
#define	DT		0x4454
#define	EH		0x4548
#define	ET		0x4554
#define	OM		0x4F4D
#define	SH		0x5348
#define	SC		0x5343
#define	FD		0x4644

#define SOHFN	"0/SOH.RT"
typedef struct _RT130_SOH_PKT
{
	unsigned char	header[RT130_HEADER_SIZE];
	unsigned char	information[1000];
} RT130_SOH_PKT;

typedef struct time_type {
	int year;	/* year         */
	int day;	/* day          */
	int hour;	/* hour         */
	int min;	/* minute       */
	int sec;	/* second       */
	int msec;	/* millisecond - NOTE, not always used (see clockview) */
	int usec;	/* microsecond  */
} RTTime;

typedef struct epoch_type {
	int	sec;
	int	usec;
} RTEpoch;

typedef struct _chanInfo {
	char *fileName;
	int strmNm, chnNm, chnNum;
	unsigned int data_form;
	int numSamps, gain;
} CHANINFO;

typedef struct _ehInfo {
	char 	*fileName;
	int		unit_id;
	float	sampsPsec;
	RTEpoch epoch;
} EHINFO;

#define RT130STREAMNM (sbcdhx(&block[18], 2)+1)
#define RT130CHANNELNM (sbcdhx(&block[19], 2)+1)
#define RT130CHANNELNUM ((sbcdhx(&block[18], 2))*3+(sbcdhx(&block[19],2)+1))
#define HDRCHANNELNUM(a) ((atoi(a)-1)*3+(atoi((strrchr(a,':')+1))))
#define RT130NUMSAMPS (sbcdhx(&block[20], 4))
#define RT130DATAFORM (block[23])
#define RT130UNITID (bch2int ((unsigned char *) &eh->header.unit_id[0], 4))
#define RT130CHGAIN(a, b) switch(a) { \
						case '1': b = 0x1; break; \
						case '2': b = 0x8; break; \
						case '3': b = 0x32; break; \
						case '4': b = 0x128; break; \
						case '5': b = 0x512; break; \
						case '6': b = 0x2048; break; \
						case '7': b = 0x8192; break; \
						case '8': b = 0x100; break; \
						case 'A': b = 12; break; \
						case 'B': b = 24; break; \
						case 'C': b = 36; break; \
						case 'D': b = 48; break; \
						case 'E': b = 64; break; \
						case 'F': b = 0x2; break; \
						case 'G': b = 0x4; break; \
						case 'H': b = 0x16; break; \
						case 'I': b = 0x64; break; \
						case 'J': b = 0x256; break; \
						}

// utility function declares
int sbcdhx(unsigned char *ch, int dgts);
int bch2int (unsigned char *ch, int n);
int ConvertRefType(char *str);
void ascii2bin(RTTime *btime, EventTime *atime);
void gmt2epoch (RTEpoch *epoch, RTTime *time, int leap_sec);
void rmemcpy();
int steim1 (char *, int *, int*, int, gboolean);
int steim2 (char *, int *, int*, int, gboolean);
int uncompress16 (char *, short *, int*, int);

// RT-125A Defines
enum {
	RT125AYRLOC,
	RT125AJDAYLOC1,
	RT125AJDAYLOC2,
	RT125AHRLOC,
	RT125AMNLOC,
	RT125ASECLOC,
	RT125AEVENTTIME_LEN
};

typedef struct _RT125A_SOH_PKT
{
	unsigned char	pageType;
	unsigned char	unitID[2];
	unsigned char	sequenceNum[2];
	unsigned char	flags;
	unsigned char	reserved;
	unsigned char	byteCnt[2];
	unsigned char	msgCnt;
	unsigned char	msgs[516];
	unsigned char	CRC[2];
} RT125A_SOH_PKT;

typedef struct _RT125A_DATA_PKT
{
	unsigned char	pageType;
	unsigned char	unitID[2];
	unsigned char	sequenceNum[2];
	unsigned char	flags;
	unsigned char	gain;
	unsigned char	eventNum[2];
	unsigned char	sampleCnt;
	unsigned char	eventTime[RT125AEVENTTIME_LEN];
	unsigned char	sampleRate[2];
	unsigned char	battery;
	unsigned char	data[507];
	unsigned char	CRC[2];
} RT125A_DATA_PKT;

typedef struct _RT125A_EVENT_ENTRY
{
	unsigned char	eventTime[RT125AEVENTTIME_LEN];
	unsigned char	action;
	unsigned char	parameter;
} RT125A_EVENT_ENTRY;
	
typedef struct _RT125A_EVENT
{
	unsigned char	pageType;
	unsigned char	unitID[2];
	unsigned char	sequenceNum[2];
	unsigned char	flags;
	unsigned char	reserved1;
	unsigned char	numEntries;
	unsigned char	totalEntries[2];
	unsigned char	reserved2[4];
	RT125A_EVENT_ENTRY	events[64];
	unsigned char	CRC[2];
} RT125A_EVENT;

typedef struct _RT125ATRACE {
	char *fileName;
//	char *eventTime;
	int unit_id;
	int eventNum;
	int sampsPsec;
	int year, jday, hour, minute, second;
	int numSamps, gain;
} RT125ATRACE;

#define RT125A_PKT_SIZE (sizeof(RT125A_DATA_PKT))
#define RT125A_MAX_PKT_RD 1000
// Page Types
#define PIC	0x00
#define PSH	0x01
#define PDT 0x03
#define PET 0x05
#define PER 0xFF

#define RT125AUNITID(a)	((int) (a[0]<<8) + (int) (a[1]) + 10000)
#define RT125ASPS(a)	((int) (a[0]<<8) + (int) (a[1]))
#define RT125AEVENTNUM(a)	((int) (a[0]<<8) + (int) (a[1]))
#define RT125AYEAR(a)	((int) (a[RT125AYRLOC]) + 1984)
#define RT125AJDAY(a)	((int) (a[RT125AJDAYLOC1]) * 100 + (int) (a[RT125AJDAYLOC2]) + 1)
#define RT125AHOUR(a)	((int) (a[RT125AHRLOC]))
#define RT125AMIN(a)	((int) (a[RT125AMNLOC]))
#define RT125ASEC(a)	((int) (a[RT125ASECLOC]))
#define RT125AGAIN(a, b) switch(a) { \
						case 0: b = 32; break; \
						case 1: b = 1; break; \
						case 2: b = 2; break; \
						case 3: b = 4; break; \
						case 4: b = 8; break; \
						case 5: b = 16; break; \
						case 6: b = 32; break; \
						case 7: b = 64; break; \
						case 8: b = 128; break; \
						case 9: b = 256; break; \
						}
#define RT125ASOHSEQN(a)	((int) (a[0]<<8) + (int) (a[1]))
#define RT125ASOHBCNT(a)	((int) (a[0]<<8) + (int) (a[1]))

// RT-125 Defines
typedef struct _RT125_SOH_PKT
{
	unsigned char	pageType;
	unsigned char	unitID[2];
	unsigned char	sequenceNum[2];
	unsigned char	flags;
	unsigned char	reserved;
	unsigned char	byteCnt[2];
	unsigned char	msgCnt;
	unsigned char	msgs[516];
	unsigned char	CRC[2];
} RT125_SOH_PKT;

typedef struct _RT125_DATA_PKT
{
	unsigned char	pageType;
	unsigned char	unitID[2];
	unsigned char	sequenceNum[2];
	unsigned char	flags;
	unsigned char	reserved;
	unsigned char	eventNum[2];
	unsigned char	sampleCnt;
	unsigned char	data[516];
	unsigned char	padding[2];
} RT125_DATA_PKT;

typedef struct _RT125_EXT_HEAD
{
	unsigned char eventTime[RT125AEVENTTIME_LEN];
	unsigned char sampleRate[2];
	unsigned char reserved;
} RT125_EXT_HEAD;
						
#define RT125_PKT_SIZE (sizeof(RT125_DATA_PKT))
#define RT125_MAX_PKT_RD 1000
#define RT125_EVENT_FIRST_PAGE		(1)
#define RT125_EVENT_LAST_PAGE		(1<<1)
#define RT125_EXTENDED_HEADER		(1<<2)

#define CR	0xD
#define LF	0xA

enum {
	LITTLE_END,
	BIG_END
};
int endian (void);

#endif /* REF2SEGY_H	*/
