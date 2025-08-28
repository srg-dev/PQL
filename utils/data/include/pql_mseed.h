#ifndef _PQL_MSEED_H
#define _PQL_MSEED_H

#define	CLOSEMSR(msfp, msr)				ms_readmsr_r(msfp, msr, NULL, 0, NULL, NULL, 0, 0, 0)
#define READMSRDATA(msfp, msr, file)	ms_readmsr_r(msfp, msr, file, 0, NULL, NULL, 1, TRUE, 0)
#define READMSRDATA2(msfp, msr, file, offset)	ms_readmsr_r(msfp, msr, file, 0, &offset, NULL, 1, TRUE, 0)
#define READMSTGDATA(mstg, file)		ms_readtraces(mstg, file, -1, -1.0, -1.0, 0, 1, TRUE, 0)
#define READMSRNDATA(msfp, msr, file)	ms_readmsr_r(msfp, msr, file, 0, NULL, NULL, 1, FALSE, 0)
#define READMSRNDATA2(msfp, msr, file, offset)	ms_readmsr_r(msfp, msr, file, 0, &offset, NULL, 1, FALSE, 0)
#define READMSTGNDATA(mstg, file)		ms_readtraces(mstg, file, -1, -1.0, -1.0, 0, 1, FALSE, 0)

#define ISVALIDENCODING(a) ((a==0  || \
							 a==1  || \
							 a==3  || \
							 a==10 || \
							 a==11 || \
							 a==30 || \
							 a==32 || \
							 a==4  || \
							 a==5  || \
							 a==12 || \
							 a==13 || \
							 a==14) \
							? TRUE \
							: FALSE)

#define SAMPINT(a) ((a==0)	\
					? 0		\
					: 	((a+.005)<1)	\
						? ((double)10000000000./(double)((int)(a*10000)))	\
						: ((double)100000000./(double)((int)(a*100+.5))))

#define GAPSTART	(numGaps*2)
#define GAPTERM		(numGaps*2+1)
#define OLAPSTART	(numOlaps*2)
#define OLAPTERM	(numOlaps*2+1)

#define HDR_OFFSET	48
#define MSEED_DETECT_ALL_RECLEN
#define TOOMANYPOINTS	50000000

typedef struct _MM_TRCINFO
{
	char	*channelName;
	char	*fileName;
	char	*network, *station, *location, *channel;
	int		blkSize;
	hptime_t	startTime;
	double	sampRate;
	int		sampleCnt;
	int8_t	encoding;
	GSList	*packets;
} MM_TRCINFO;

typedef struct _MMPACKET
{
	int	index;
	int sampleCnt;
	hptime_t startTime;
} MMPACKET;

typedef struct _MMINDEX
{
	char	*channelName;
	char	*network, *station, *location, *channel;
	hptime_t startTime;
	double	sampRate;
	int		sampleCnt;
	int8_t	encoding;
	GSList	*packets;
} MMINDEX;

typedef struct _MMFILE
{
	char	*fileName;
	int		blkSize;		// = 0 means file has multiple block sizes, <> 0 means absolute block size
	GSList	*channels;
} MMFILE;

unsigned short swap_2byte(unsigned short nValue);
guint32 swap_4byte(guint32 nValue);

#endif
