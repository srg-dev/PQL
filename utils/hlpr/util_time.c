#include <time.h>
#include <sys/time.h>
#include "pql_defines.h"

static int      days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31, 31};

#define mod(a,b)	(a - ((int)(a/b)) * b)

//#define passcal_isleap(a)	((a % 4 == 0 && (a % 100 != 0 || a % 400 == 0)))

void	TIMESTR(char *timeStr)
{
	// use localtime
	time_t	tloc;
	struct tm	*timeN;

	time(&tloc);
#ifdef WIN32
	timeN = localtime(&tloc);
#else
	timeN = calloc(1, sizeof(struct tm));
	localtime_r(&tloc, timeN);
#endif

	sprintf(timeStr, "%d %03d %02d:%02d:%02d>>\t", 
			timeN->tm_year + 1900, 
			timeN->tm_yday+1, 
			timeN->tm_hour,
			timeN->tm_min, 
			timeN->tm_sec);
	
#ifndef WIN32
	free(timeN);
#endif
}

void	TIMESTRE(char *timeStr)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);

	sprintf(timeStr, "%d.%03d>>\t", (int) tv.tv_sec, (int) (tv.tv_usec/1000));
}

char *currentT()
{
	char *timeStr;
	time_t	tloc;
	struct tm	*timeN;

	time(&tloc);
#ifdef WIN32
	timeN = localtime(&tloc);
#else
	timeN = calloc(1, sizeof(struct tm));
	localtime_r(&tloc, timeN);
#endif

	timeStr = g_strdup_printf("%04d/%02d/%02d %02d:%02d:%02d", 
			timeN->tm_year + 1900, 
			timeN->tm_mon+1, 
			timeN->tm_mday, 
			timeN->tm_hour,
			timeN->tm_min, 
			timeN->tm_sec);

#ifndef WIN32
	free(timeN);
#endif
	return (timeStr);
}

void passcal_etoh(PTIME *t, double epoch)
/* fill the time structure *t with values
   equivalent to epochal time epoch    */
{
    int             diy;
    double          secleft;
    gint32          days;
    void            passcal_month_day();

    days = (gint32) ((double) epoch / (double) 86400.);
    secleft = mod(epoch, (double) 86400.0);  
    t->hr = t->mn = 0;
    t->sec = 0.;

    if (secleft) {		/* compute hours minutes seconds */
	if (secleft < 0) {	/* before 1970 */
	    days--;		/* subtract a day */
	    secleft += 86400.;	/* add a day */
	}
	t->hr = (int) (secleft / 3600.);
	secleft = mod(secleft, 3600.0);
	t->mn = (int) (secleft / 60.);
	t->sec = mod(secleft, 60.0);
    }

    if (days >= 0) {
	for (t->yr = 1970;; t->yr++) {
	    diy = passcal_isleap(t->yr) ? 366 : 365;
	    if (days < diy)
		break;
	    days -= diy;
	}
    }
    else {
	for (t->yr = 1969;; t->yr--) {
	    diy = passcal_isleap(t->yr) ? 366 : 365;
	    days += diy;
	    if (days >= 0)
		break;
	}
    }
    /* there was a round-off problem */
    if (t->hr == 23 && t->mn == 59 && t->sec == 60) {  
      t->sec = 0.0;
      t->mn = 0;
      t->hr = 0;
      days++;
    }

    days++;
    t->jday = days;
    passcal_month_day(t, (int) days);
    return;
}

void passcal_month_day(PTIME *t, int jul_day)
/* set month and day fields of time structure *t
   given the julian day jul_day               */
{
    int             i, dim, leap;

    leap = passcal_isleap(t->yr);
    t->day = jul_day;
    for (i = 0; i < 12; i++) {
	dim = days_in_month[i];
	if (leap && (i == 1))
	    dim++;
	if (t->day <= dim)
	    break;
	t->day -= dim;
    }
    t->mo = i + 1;
    return;
}

int julian(PTIME *t)
/* returns the julian day represented by *t */
{
    int             i, j, inc;

    j = 0;
    for (i = 0; i < t->mo - 1; i++) {
	inc = days_in_month[i];
	if ((i == 1) && passcal_isleap(t->yr))
	    inc++;
	j += inc;
    }
    j += t->day;
    return (j);
}

gint32 passcal_htoe(PTIME *t, gboolean convertJul)
{
    gint32            i, days;
    gint32            epoch;

    days = 0;
    if (t->yr > 1970) {
	for (i = 1970; i < t->yr; i++) {
	    days += 365;
	    if (passcal_isleap(i))
		days++;
	}
    }
    else if (t->yr < 1970) {
	for (i = t->yr; i < 1970; i++) {
	    days -= 365;
	    if (passcal_isleap(i))
		days--;
	}
    }
    switch(convertJul)
    {
    	case TRUE:
		    days += (gint32) (julian(t) - 1);
		break;
		case FALSE:
			days += (gint32) (t->day - 1);
		break;
	}
    epoch = (((gint32) days) * 86400.);
    epoch += (gint32) ((t->hr * 3600.) + (t->mn * 60.) + (t->sec));
    return (epoch);
}

static char daytab[2][13] = {
    {0,31,28,31,30,31,30,31,31,30,31,30,31},
    {0,31,29,31,30,31,30,31,31,30,31,30,31}};

int get_julday(int month, int day, int year)
{

   int julday;
   int i, leap;

	julday = day;

	leap = (year%4 == 0 && (year%100 != 0 || year%400 == 0));

	/* check range of dates */
	if((month < 1) || (month>12))
		return(-1);
	if((day <1) || (day > (daytab[leap][month])))
		return(-1);
	for(i =1;i<month;i++)
        	julday += daytab[leap][i];

	/* return yearday*/
	return (julday);
}

double getMidnight(double epoch)
{
	struct ptime date;

	passcal_etoh(&date, epoch);
	date.hr = date.mn = date.sec = 0;
	return passcal_htoe(&date, TRUE);
}

gboolean chkDateFormat(GDateYear year, GDateMonth month, GDateDay day)
{
	gboolean invalid = FALSE;
	if (year < 1900)
		invalid = TRUE;
	if (month < G_DATE_JANUARY ||
		month > G_DATE_DECEMBER)
		invalid = TRUE;
	if (day < 1 ||
		day > 31)
		invalid = TRUE;

	return(invalid);
}

