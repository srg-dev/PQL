#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "pql_defines.h"
#include "pql_externs.h"
#include "reftek.h"

int sbcdhx(unsigned char *ch, int dgts)
{
  int             num, i;
  
  for (i = num =0; i < dgts; i += 2) {
    num = 10 * (10 * num + *ch / 16) + (*ch % 16);
    ch++;
  }

  return (num);
}

int bch2int (unsigned char *ch, int n)
{
  int i, num, nn;

  num = 0;
  nn = n / 2;
  for (i = 0; i < (n / 2); i++) {
    /*   High nibble   */
    num += ((ch[i] & 0xF0) >> 4) * (int) pow (16.0, (double) (nn + 1));
    /*   Low nibble   */
    num += (ch[i] & 0x0F) * (int) pow (16.0, (double) nn);
    /*   Assume all DAS numbers 9000 and over are new   */
    if (i == 0 && num <= 35225) 
    {
      num = sbcdhx (ch, 4);
      break;
    }
    nn -= 2;
  }

  return (num);

}

int ConvertRefType(char *str)
{
    unsigned short t_code, *code;
	static int byteOrder = -1;
	
	if (byteOrder == -1)
		byteOrder = testByteOrder();

    if (byteOrder == MY_BIG_ENDIAN)
		memcpy(&t_code,&str[0],2);
    else
		rmemcpy(&t_code,&str[0],2); /* reverse memory copy (byte swap) */
    code = &t_code;
    return (int) *code;
}

void ascii2bin(RTTime *btime, EventTime *atime)
{
     btime->year = 0;
     btime->day = 0;
     btime->hour = 0;
     btime->min = 0;
     btime->sec = 0;
     btime->msec = 0;
     btime->usec = 0;
     if(atime->year[0] == ' ')
     {
       btime->year = 1970;
       btime->day = 1;
       btime->hour = 0;
       btime->min = 0;
       btime->sec = 0;
       btime->usec = 0;
       return;
     }
     sscanf(atime->year, "%4d", &(btime->year));
     sscanf(atime->day, "%3d", &(btime->day));
     sscanf(atime->hour, "%2d", &(btime->hour));
     sscanf(atime->min, "%2d", &(btime->min));
     sscanf(atime->sec, "%2d", &(btime->sec));
     sscanf(atime->msec, "%3d", &(btime->msec));

     return;
}

void gmt2epoch (RTEpoch *epoch, RTTime *time, int leap_sec)
{
	int epoch_year=1970;
	int years_since, leap_years, days, sec;

	/* check for valid range	*/
	if (time->year < epoch_year)
		time->year = epoch_year;

	if (time->day < 1)
		time->day = 1;

	years_since = time->year - epoch_year;

	leap_years = (years_since + 1) / 4;

	/* need to subtract one, since range of julian day (1-365/366)	*/
	days = years_since*365 + time->day + leap_years - 1;

	sec = days*86400 + time->hour*3600 + time->min*60 + time->sec;
	epoch->sec = sec + leap_sec;

	/* 
		This is a carry over from a ref2segy's usage of this 
			patch 97.350 paulf
	*/
	if (time->msec >0)
		epoch->usec = time->msec*1000;
	else
		epoch->usec = time->usec;
}
