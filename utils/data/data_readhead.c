#include <string.h>
#include "nano2ah.h"
//#include "readhead.h"
#include "pql_defines.h"
#include "pql_externs.h"

void rmemcpy(buf, rbuf, size)
u_char *buf, *rbuf;
int size;
{
	register u_char *baddr, *raddr, *bufend, *rbufstart;

	bufend = buf+size;
	rbufstart = rbuf+size-1;

	for (baddr=buf, raddr=rbufstart; baddr<bufend; baddr++, raddr--)
		*baddr = *raddr;
}

void revmem(buf, size)
u_char *buf;
int size;
{
	u_char tmp[4];
	rmemcpy(tmp, buf, size);
	memcpy((char *)buf, (char *)tmp, size);
}

void get_char(char *str, MEMBUF *membuf, int len)
{
	(void) memccpy((void*)str, (void*)&membuf->str[membuf->pos], 0, len);
	str[len] = '\0';
	membuf->pos += len;
}

void get_uchar(str, membuf, len)
char *str;
MEMBUF *membuf;
int len;
{
	(void) memcpy(str, &membuf->str[membuf->pos], len);
	str[len] = '\0';
	membuf->pos += len;
}

void get_num(addr, membuf, size)
char *addr;
MEMBUF *membuf;
int size;
{
	rmemcpy(addr, &membuf->str[membuf->pos], size); 
	membuf->pos += size;
}

int read_header3(istream, trace)
FILE *istream;
NMXTRACE3 *trace;
{
	MEMBUF membuf;
	int num;

	num = fread((char *) &membuf.str[0], SIZECHAR, NANOHEADSIZE, istream);
	if (num != NANOHEADSIZE) return(FALSE);

	membuf.pos = 0;
	membuf.size = NANOHEADSIZE;

	get_num(&trace->usVersion, &membuf, SIZESHORT);
	if (trace->usVersion != 3) return(FALSE);

	get_num(&trace->usOffset, &membuf, SIZESHORT);

	get_num(&trace->stStnChn.usStnKey, &membuf, SIZESHORT);
	get_uchar(&trace->stStnChn.achStation[0] , &membuf, 5);
	get_uchar(&trace->stStnChn.achLocation[0] , &membuf, 2);
	get_uchar(&trace->stStnChn.achChannel[0] , &membuf, 3);

	get_num(&trace->stStnChn.usLongWordOrder, &membuf, SIZESHORT);
	get_num(&trace->stStnChn.usWordOrder, &membuf, SIZESHORT);
	get_uchar((u_char *) &trace->stStnChn.szNetworkID[0] , &membuf, 51);
	get_uchar(&trace->stStnChn.szSiteName[0] , &membuf, 61);
	get_uchar(&trace->stStnChn.szComment[0] , &membuf, 31);
	get_uchar(&trace->stStnChn.szSensorType[0] , &membuf, 51);
	get_uchar((u_char *) &trace->stStnChn.achDataFormat[0] , &membuf, 6);

	get_num(&trace->stStnChn.rLatitude, &membuf, SIZEFLOAT);
	get_num(&trace->stStnChn.rLongitude, &membuf, SIZEFLOAT);
	get_num(&trace->stStnChn.rElevation, &membuf, SIZEFLOAT);
	get_num(&trace->stStnChn.rDepth, &membuf, SIZEFLOAT);
	get_num(&trace->stStnChn.rAzimuth, &membuf, SIZEFLOAT);
	get_num(&trace->stStnChn.rDip, &membuf, SIZEFLOAT);

	get_num(&trace->stStnChn.rSensitivity, &membuf, SIZEFLOAT);
	get_num(&trace->stStnChn.rSensFreq, &membuf, SIZEFLOAT);
	get_num(&trace->stStnChn.ulRespKey, &membuf, SIZELONG);
	get_uchar(&trace->stStnChn.szSensUnits[0] , &membuf, 21);
	get_uchar(&trace->stStnChn.szCalibUnits[0] , &membuf, 21);
	get_num(&trace->stStnChn.rSampleRate, &membuf, SIZEFLOAT);
	get_num(&trace->stStnChn.rMaxClkDrift, &membuf, SIZEFLOAT);
	get_uchar(&trace->stStnChn.achChanFlags[0] , &membuf, 26);
	get_uchar(&trace->stStnChn.uchUpdateFlag, &membuf, 1);
	get_num(&trace->stStnChn.rtmUpdateTime1, &membuf, SIZEDOUBLE);
	get_num(&trace->stStnChn.rtmUpdateTime2, &membuf, SIZEDOUBLE);

	get_uchar(&trace->stStnChn.achGroupId[0] , &membuf, 4);
	get_uchar(&trace->stStnChn.uchFep, &membuf, 1);
	get_uchar(&trace->stStnChn.uchPort, &membuf, 1);
	get_uchar(&trace->stStnChn.uchChan, &membuf, 1);
	get_uchar(&trace->stStnChn.uchDataCode, &membuf, 1);
	get_num(&trace->stStnChn.rtmTimeCorrect, &membuf, SIZEDOUBLE);
	get_num(&trace->stStnChn.rSensorSensitivity, &membuf, SIZEFLOAT);
	get_num(&trace->stStnChn.rRd3Sensitivity, &membuf, SIZEFLOAT);
	get_num(&trace->stStnChn.usInstrumentID, &membuf, SIZESHORT);

	get_num(&trace->stStnChn.ulCommID, &membuf, SIZELONG);
	get_num(&trace->stStnChn.rtmLoadDate, &membuf, SIZEDOUBLE);

	get_num(&trace->stSeries.ulSeriesKey, &membuf, SIZELONG);
	get_uchar(&trace->stSeries.achStation[0] , &membuf, 5);
	get_uchar(&trace->stSeries.achLocation[0] , &membuf, 2);
	get_uchar(&trace->stSeries.achChannel[0] , &membuf, 3);
	get_num(&trace->stSeries.rtmStartTime1, &membuf, SIZEDOUBLE);
	get_num(&trace->stSeries.rtmEndTime1, &membuf, SIZEDOUBLE);
	get_num(&trace->stSeries.ulNumSamples, &membuf, SIZELONG);
	get_num(&trace->stSeries.IDCOffset, &membuf, SIZELONG);
	get_num(&trace->stSeries.IMaxAmplitude, &membuf, SIZELONG);
	get_num(&trace->stSeries.IMinAmplitude, &membuf, SIZELONG);

	get_uchar(&trace->stSeries.szFileName[0] ,&membuf, MAXFILENAMELENGTH);
	get_uchar(&trace->stSeries.szFormatVersion[0] , &membuf, 6);
	get_uchar(&trace->stSeries.szFormat[0] , &membuf, 6);

	return(TRUE);
}

#define NANOHEADSIZE1	134

int read_header1(istream, trace)
FILE *istream;
NMXTRACE1 *trace;
{
	MEMBUF membuf;
	int num;

	num = fread((char *) &membuf.str[0], SIZECHAR, NANOHEADSIZE1, istream);
	if (num != NANOHEADSIZE1) return(FALSE);

	membuf.pos = 0;
	membuf.size = NANOHEADSIZE1;

	get_num(&trace->ulDataOffset, &membuf, SIZELONG);
	get_num(&trace->ulBitMapOffset, &membuf, SIZELONG);
	get_num(&trace->ulNumSamples, &membuf, SIZELONG);
	get_uchar(&trace->uchDataType , &membuf, 1);
	get_uchar(&trace->achNetId[0], &membuf, 4);
	get_uchar(&trace->achSiteId[0], &membuf, 8);
	get_uchar(&trace->achGroupId[0], &membuf, 4);
	get_uchar(&trace->achChanID[0], &membuf, 4);
	get_num(&trace->fLatitude, &membuf, SIZEFLOAT);
	get_num(&trace->fLongitude, &membuf, SIZEFLOAT);
	get_num(&trace->fElevation, &membuf, SIZEFLOAT);
	get_uchar(&trace->uchFep, &membuf, 1);
	get_uchar(&trace->uchPort, &membuf, 1);
	get_uchar(&trace->uchChan, &membuf, 1);
	get_num(&trace->ulBaud, &membuf, SIZELONG);
	get_num(&trace->usSampleRate, &membuf, SIZESHORT);
	get_num(&trace->usDecimation, &membuf, SIZESHORT);
	get_uchar(&trace->uchDataCode, &membuf, 1);
	get_uchar(&trace->uchFormat, &membuf, 1);
	get_num(&trace->lTimeCorSecs, &membuf, SIZELONG);
	get_num(&trace->sTimeCorMs, &membuf, SIZESHORT);

	get_uchar(&trace->achKeyResponse[0], &membuf, 4);
	get_num(&trace->fRd3Sensitivity, &membuf, SIZEFLOAT);
	get_num(&trace->fInputResistance, &membuf, SIZEFLOAT);

	get_num(&trace->fSensitivity, &membuf, SIZEFLOAT);
	get_uchar(&trace->achSensorType[0], &membuf, 20);
	get_num(&trace->fDamping, &membuf, SIZEFLOAT);
	get_num(&trace->fExtResistance, &membuf, SIZEFLOAT);
	get_uchar(&trace->achUnits[0], &membuf, 20);
	get_num(&trace->ulTimeSecs, &membuf, SIZELONG);
	get_num(&trace->usTimeMs, &membuf, SIZESHORT);

	return(TRUE);
}

int read_rechdr(membuf, rechdr)
char *membuf;
RECHDR *rechdr;
{
	memcpy(&rechdr->seq_num[0], &membuf[0], 6);
	memcpy(&rechdr->DHI_1, &membuf[6], 1);
	memcpy(&rechdr->DHI_2, &membuf[7], 1);
	memcpy(&rechdr->stat_ID[0], &membuf[8], 5);
	memcpy(&rechdr->loc_ID[0], &membuf[13], 2);
	memcpy(&rechdr->chan_ID[0], &membuf[15], 3);
	memcpy(&rechdr->reserved1[0], &membuf[18], 2);
	rmemcpy(&rechdr->sec, &membuf[20], SIZELONG);
	rmemcpy(&rechdr->msec, &membuf[24], SIZESHORT);
	memcpy(&rechdr->reserved2[0], &membuf[26], 4);
	rmemcpy(&rechdr->sample_count, &membuf[30], SIZESHORT);
	rmemcpy(&rechdr->samsec, &membuf[32], SIZESHORT);
	rmemcpy(&rechdr->s_factor, &membuf[34], SIZESHORT);
	memcpy(&rechdr->activity_flag, &membuf[36], 1);
	memcpy(&rechdr->io_flag, &membuf[37], 1);
	memcpy(&rechdr->quality_flag, &membuf[38], 1);
	memcpy(&rechdr->n_blockettes, &membuf[39], 1);
	rmemcpy(&rechdr->time_cor, &membuf[40], SIZELONG);
	rmemcpy(&rechdr->d_offset, &membuf[44], SIZESHORT);
	rmemcpy(&rechdr->b_offset, &membuf[46], SIZESHORT);
	memcpy(&rechdr->reserved3[0], &membuf[48], 16);

	return(TRUE);
}

#include <stdio.h>
#include <sys/types.h>

int probe_header(FILE *istream)
{
	union short_union {
		u_short val;
		char buf[2];
	} s;

	s.buf[0] = getc(istream);
	s.buf[1] = getc(istream);

	ungetc(s.buf[1], istream);
	ungetc(s.buf[0], istream);

	switch (s.val) {
		case 0x8600:
			return(1);
		case 0x0300:
			return(3);
                default:
			return(EOF);
	}
}
