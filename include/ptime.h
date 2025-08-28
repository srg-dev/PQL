#ifndef __PTIME
#define __PTIME
typedef struct ptime {
	short yr;	/* year		*/
	short mo;	/* month	*/
	short day;	/* day		*/
	short jday;
	short hr;	/* hour		*/
	short mn;	/* minute	*/
	float sec;	/* second	*/
} PTIME;
#endif
