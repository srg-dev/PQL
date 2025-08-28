/*
	The following routines plot both linear and logarithmic axes on the
   Sun workstation using X routines.
	Originally written by Peter Crames as part of the Graphpac package.
	Modified 1/88 by rbd for use by X11 on the Sun.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pql_defines.h"
#include "pql_externs.h"
#include "gtk_pql_externs.h"

/*	xaxis will draw an x axis from (x0, y0) to (xn, y0) with major, [middle,
   and minor] tick marks. A major tick mark is the biggest in size and is also
   numbered. A middle tick mark is middle in size and is not numbered. A minor
   tick mark is smallest in size and is also not numbered. The function will
   find the length of the axis, xylen, which equals xn-x0. Then the largest
   power of 10 smaller than xylen will be found.  For example, if xylen = 253,
   then the power of 10 will be p = 100.  If xylen = .003, then p = .001. Major
   tick marks (with numbering) will be drawn at x0, xn, and every multiple of p
   between x0 and xn. For example, if the x axis is from 314 to 860, xylen
   equals 546, and p equals 100. There will be major tick marks at 314, 400,
   500, 600, 700, 800 and 860. The distance between each major tick mark is
   divided into ndivs divisions. If ndivs <= 0, ndivs will be set to the default
   of 10 divisions between major tick marks.  Minor tick marks will be drawn at
   every multiple of p/ndivs between x0 and xn that is not a major tick mark. If
   ndivs is even, the tick mark half way between each major tick mark will be a
   middle tick mark instead of a minor tick mark.  For example, if the x axis is
   from 147 to 312, and ndivs = 4, there will be major tick marks at 147, 200,
   300 and 312. There will be middle tick marks at 150 and 250. There will be
   minor tick marks at 175, 225, and 275. If ndivs = 1, there will be no middle
   or minor tick marks.
   	tside and nside determine which side of the x axis tick marks and
   numbers will appear. 1 is above the axis, -1 is below the axis, and 0 is not
   at all. For example, xaxis(pxw, grph, 215., 50., 3490., 1, -1, 8, 0.) will
   draw an x axis from (215., 50.) to (3490., 50.) with tick marks above the
   axis and numbers below the axis.
	offset is the label offset. This number will be added to the value of
   the coordinate at major tick marks when computing the label. Label offsets
   are not currently supported for logarithmic axes.
	grph is a pointer to the graph which describes the mapping between the
   user's coordinates and the pixels of pxw. The graph structure is defined in
   defines.h, and the mapping routines are contained in user_pw.c.
 */

void	g_xmajt(), g_xmidt(), g_xmint();
void	g_ymajt(), g_ymidt(), g_ymint();
gboolean	g_eq();

double          p[77] = {1e-38, 1e-37, 1e-36, 1e-35, 1e-34, 1e-33, 1e-32, 1e-31, 1e-30,
    1e-29, 1e-28, 1e-27, 1e-26, 1e-25, 1e-24, 1e-23, 1e-22, 1e-21,
    1e-20, 1e-19, 1e-18, 1e-17, 1e-16, 1e-15, 1e-14, 1e-13, 1e-12,
    1e-11, 1e-10, 1e-9, 1e-8, 1e-7, 1e-6, 1e-5, 1e-4, 1e-3, 1e-2,
    1e-1, 1e0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9, 1e10,
    1e11, 1e12, 1e13, 1e14, 1e15, 1e16, 1e17, 1e18, 1e19, 1e20, 1e21,
    1e22, 1e23, 1e24, 1e25, 1e26, 1e27, 1e28, 1e29, 1e30, 1e31, 1e32,
1e33, 1e34, 1e35, 1e36, 1e37, 1e38};
double          eps;
int             pwr, ts, ns;

GdkPixmap		*pixmap;
GdkGC          *grct;
graph          *locgrph;
int				frmt;

int             majtick = 7;	/* tick sizes in pixels */
int             midtick = 4;
int             mintick = 3;

int				pix_xmax;	// right-most x pixel
int             strwidth;	// character size in pixels
int             charwidth;	// character size in pixels
static int      charheight;

static double   lastnum;	// position of last printed tick label
static double   nextnum;	// position of next tick label to be printed
static double   label_offset;
static double	startTime;

void xaxis(pixmp, grc, grph, x0, y0, xn, tside, nside, ndivs, offset, inv, epoch)
    GdkPixmap      *pixmp;
    GdkGC          *grc;
    graph          *grph;
    double          x0, y0, xn;
    int             tside, nside, ndivs;
    double          offset;
	gboolean		inv;	// invert the output string, used by spectra to plot Period in seconds
	double			epoch;	// if !0, use as absolute start time and print major tick as a time specification
{
    double          xylen, ptr, cnt, divcon, mincon, maxcon;
    double          true_xmin, true_xmax;
    int             pix_x0, pix_x1, pix_y0, last_pix_x0=0, height;
    double          exp();

    grct = grc;
	pixmap = pixmp;
    locgrph = grph;
	startTime = epoch;
	gdk_drawable_get_size(pixmp, &pix_xmax, &height);

    /* allow for floating point error in axis limits */
    if (locgrph->logx) {
	true_xmin = exp(2.302585 * locgrph->user_xmin);
	true_xmax = exp(2.302585 * locgrph->user_xmax);
    }
    else {
	true_xmin = locgrph->user_xmin;
	true_xmax = locgrph->user_xmax;
    }
    mincon = (true_xmin < 0.) ? 1.01 :.99;
    maxcon = (true_xmax < 0.) ?.99 : 1.01;

    if (x0 < true_xmin * mincon || xn > true_xmax * maxcon) {
/*		msg("xaxis: x axis outside xy window", 1, INVERSE);  */
		fprintf(stderr, "xaxis: x axis outside xy window\n");
		return;
//		exit(-1);
    }
    if (x0 > xn) {
/*		msg("xaxis: end point greater than start point", 1, INVERSE);  */
		fprintf(stderr, "xaxis: start point greater than end point\n");
		return;
//		exit(-1);
    }
    if ((tside != -1 && tside != 0 && tside != 1) || (nside != -1 && nside != 0 && nside != 1)) {
/*		msg("xaxis: tside and nside must be -1, 0 or 1", 1, INVERSE);  */
		fprintf(stderr, "xaxis: tside and nside must be -1, 0 or 1\n");
		return;
//		exit(-1);
    }

    lastnum = true_xmin;
    if (ndivs <= 1)
	ndivs = 10;		/* set default ndivs */

    if (locgrph->logx) {

	/* find smallest power of 10 greater than starting point */
	for (pwr = 76; p[pwr] > x0; pwr--);
	label_offset = 0.;
    }
    else {

	/* find greatest power of 10 less than axis length */
	xylen = xn - x0;
	for (pwr = 76; p[pwr] >= xylen; pwr--);
	label_offset = offset;
    }

    /* ptr will move from x0 through xn */
    ptr = 0.;

    /* cnt will count the minor tick marks between major tick marks */
    cnt = ((locgrph->logx == 0) ? ((double) ndivs) : 0.);

    divcon = p[pwr] / (double) ndivs;	/* minor tick mark every divcon units */
    eps =.5 * p[pwr - 2];	/* used to compare doubles for equality in g_eq */
    ts = tside;			/* ts and ns are global so that they          */
    ns = nside;			/* needn't be passed to the tick functions    */

    if (x0 + label_offset != 0.) {
	while (1) {
	    if (locgrph->logx)
		cnt += 1.;

	    /* step ptr by p[pwr] until it passes x0 */
	    if (x0 + label_offset > 0.) {
		ptr += p[pwr];
		if (ptr > x0 + label_offset)
		    break;
	    }
	    else {
		ptr -= p[pwr];
		if (ptr < x0 + label_offset)
		    break;
	    }
	}
	if (!(locgrph->logx)) {

	    /* (if log axis, ptr and cnt are already set) */
	    if (x0 + label_offset < 0.)
		cnt = 0.;
	    while (1) {

		/*
		 * now move ptr in the opposite direction by steps of divcon
		 * until it passes x0 again. ptr will then equal the tick
		 * mark just before or just after x0
		 */
		if (x0 + label_offset > 0.) {
		    ptr -= divcon;
		    cnt -= 1.;
		    if (ptr - eps < x0 + label_offset)
			break;
		}
		else {
		    ptr += divcon;
		    cnt += 1.;
		    if (ptr + eps > x0 + label_offset)
			break;
		}
	    }
	    if (x0 + label_offset > 0.) {

		/*
		 * now move ptr in the original direction by divcon
		 */
		ptr += divcon;
		cnt += 1.;
	    }
	}
    }
    ptr -= label_offset;

    // draw axis line and major tick mark at start point
    pix_x0 = USER2PIXX(locgrph, x0);
    pix_x1 = USER2PIXX(locgrph, xn);
    pix_y0 = USER2PIXY(locgrph, y0);
    gdk_draw_line(pixmap, grct, pix_x0, pix_y0, pix_x1, pix_y0);
    g_xmajt(x0, pix_x0, pix_y0, inv, TRUE);
	if (startTime) last_pix_x0 = pix_x0 - strwidth/2;		// only used when plotting absolute time

    if (locgrph->logx) 
	{
		while (ptr <= xn + eps) 
		{
		    pix_x0 = USER2PIXX(locgrph, ptr);
		    if (cnt == 10.) 
			{
				g_xmajt(ptr, pix_x0, pix_y0, inv, TRUE);
				cnt = 1.;
				pwr++;
		    }
		    else
				g_xmidt(pix_x0, pix_y0);
		    ptr += p[pwr];
		    cnt += 1.;
		}
    }
    else 
	{
	while (ptr <= xn + eps) 
	{
	    pix_x0 = USER2PIXX(locgrph, ptr);
		if (!startTime)
		{	// process even tick distribution
	    	if (g_eq(cnt, (double) ndivs,.1)) 
			{	// at major tick mark
				g_xmajt(ptr, pix_x0, pix_y0, inv, TRUE);
				cnt = 1.;
	    	}
	    	else 
	    	{	// output middle tick mark if ndivs even and halfway between major tick marks
				if (g_eq(cnt, (double) ndivs *.5,.1))
			    	g_xmidt(pix_x0, pix_y0);
				else
			    	g_xmint(pix_x0, pix_y0);
				cnt += 1.;
	    	}
		}
		else
		{	// process time tick distribution
			if (pix_x0 >= last_pix_x0 + strwidth*1.3)
			{
				g_xmajt(ptr, pix_x0, pix_y0, FALSE, TRUE);
				last_pix_x0 = pix_x0;
			}
			else
			{
				if (pix_x0 < 0)
				{
//					fprintf(stderr, "internal error: please report (ptr=%g, xn=%g, divcon=%g)\n", ptr, xn, divcon);
					break;
				}
				else
				{
		    		g_xmidt(pix_x0, pix_y0);
				}
			}
		}
	    ptr += divcon;
	}
    }
    g_xmajt(xn, pix_x1, pix_y0, inv, !inv);
}

void g_xmajt(pt, xpt, ypt, inv, prt)
/* major tick x axis */
    double          pt;
    int             xpt, ypt, prt;
	gboolean		inv;
{
    char            outpt[20];
    int             len, hz_offset, pix_x0, pix_y0=0;

    // plot tick 
	settings.general.font.font = AXISF;

    if (ts == 1)
		gdk_draw_line(pixmap, grct, xpt, ypt, xpt, ypt - majtick);
    else 
		if (ts == -1)
			gdk_draw_line(pixmap, grct, xpt, ypt, xpt, ypt + majtick);

    if (!ns || !prt)
		return;

    /* create label */
    if (g_eq(0., pt, p[pwr - 5]))
		pt = 0.;

    if ((int) pt/1000000 > 0 && !startTime)
		sprintf(outpt, "%.4g", pt + label_offset);
    else 
	{
		memset(outpt, 0, 20);
		if (startTime)
		{	// print the absolute time
			static	int		day;
			struct ptime	date;
			int				msec, jday;
			double			startT;
			startT = startTime + pt;
			passcal_etoh(&date, startT);
			msec = (((float) (startT - (int) startT))*1000.0);
			jday = get_julday(date.mo, date.day, date.yr);
			if (day != jday)
			{
				sprintf(outpt, "%03d %02d:%02d:%02d.%03d", jday, date.hr, date.mn, (int) date.sec, msec);
				day = jday;
			}
			else
			{
				sprintf(outpt, "%02d:%02d:%02d.%03d", date.hr, date.mn, (int) date.sec, msec);
			}
		}
		else
		{	// just print the point
			sprintf(outpt, "%10.8f", pt);
		}
		for(len=strlen(outpt)-1;len > 0; len--) 
		{
	    	if(outpt[len] == '0')
				outpt[len] = 0;
	    	else
				break;
		}
    }
    /*
     * determine horizontal offset necessary for centering label
     * above/beneath tick mark; if new label will overlap the previous label,
     * don't print it
     */
    for (len = 0; outpt[len] != '\0'; len++);
    if (outpt[len - 1] == '.') 
	{
		outpt[len - 1] = '\0';
		len--;
    }
	if (!charheight)
		charheight = fontSIZE(outpt, HEIGHT);
	g_strstrip(outpt);
	strwidth = fontSIZE(outpt, WIDTH);
    hz_offset = strwidth / 2;
    pix_x0 = xpt - hz_offset;
    nextnum = PIX2USERX(locgrph, pix_x0);

    if (nextnum > lastnum) 
    {	// print tick label
    	if (pix_x0 + strwidth > pix_xmax)
    		pix_x0 = pix_xmax - strwidth - 1;
		if ((ns == 1) && (ts != 1))
		    pix_y0 = ypt - charheight - 1;	// majtick - 10;
		else if ((ns == 1) && (ts == 1))
		    pix_y0 = ypt - 2 * majtick;
		else if ((ns == -1) && (ts != -1))
		    pix_y0 = ypt + 2; //charheight + majtick - 15;
		else if ((ns == -1) && (ts == -1))
		    pix_y0 = ypt + charheight + 2 * majtick;
	
		fontDraw(pixmap, outpt, pix_x0, pix_y0, NORMALGC, SAMESCR, -1, -1);
		lastnum = PIX2USERX(locgrph, xpt + hz_offset);
    } 
}

void g_xmidt(xpt, ypt)
/* middle tick x axis */
    int             xpt, ypt;
{
    if (ts == 1)
		gdk_draw_line(pixmap, grct, xpt, ypt, xpt, ypt - midtick);
    else 
		if (ts == -1)
			gdk_draw_line(pixmap, grct, xpt, ypt, xpt, ypt + midtick);
}

void g_xmint(xpt, ypt)
/* minor tick x axis */
    int             xpt, ypt;
{
    if (ts == 1)
		gdk_draw_line(pixmap, grct, xpt, ypt, xpt, ypt - midtick);
    else 
		if (ts == -1)
			gdk_draw_line(pixmap, grct, xpt, ypt, xpt, ypt + midtick);
}

/*
  yaxis will draw a y axis from (x0, y0) to (x0, yn). The rules for major,
  middle, and minor tick marks are the same as for the xaxis function. For tside
  and nside, 1 is to the right of the axis, -1 is to the left of the axis, and 0
  is not at all.
*/
#define MIDM	0
#define BELOW   1
#define ABOVE   2

void yaxis(pixmp, grc, grph, x0, y0, yn, tside, nside, ndivs, offset, format)
    GdkPixmap      *pixmp;
    GdkGC          *grc;
    graph          *grph;
    double          x0, y0, yn;
    int             tside, nside, ndivs, format;
    double          offset;
{
    double          xylen, ptr, cnt, divcon, mincon, maxcon;
    double          true_ymin, true_ymax;
    int             pix_x0, pix_y0, pix_y1;
    double          exp();

    grct = grc;
	pixmap = pixmp;
    locgrph = grph;
    frmt = format;

    /* allow for floating point error in axis limits */
    if (locgrph->logy) {
	true_ymin = exp(2.302585 * locgrph->user_ymin);
	true_ymax = exp(2.302585 * locgrph->user_ymax);
    }
    else {
	true_ymin = locgrph->user_ymin;
	true_ymax = locgrph->user_ymax;
    }
    mincon = (true_ymin < 0.) ? 1.01 :.99;
    maxcon = (true_ymax < 0.) ?.99 : 1.01;

    if (y0 < true_ymin * mincon || yn > true_ymax * maxcon) {
		fprintf(stderr, "yaxis: y axis outside xy window\n");
		return;
//		exit(-1);
    }
    if (y0 > yn) {
		fprintf(stderr, "yaxis: start point greater than end point\n");
		return;
//		exit(-1);
    }
    if ((tside != -1 && tside != 0 && tside != 1) || (nside != -1 && nside != 0 && nside != 1)) {
		fprintf(stderr, "yaxis: tside and nside must be -1, 0 or 1\n");
		return;
//		exit(-1);
    }

    lastnum = true_ymin;
    if (ndivs <= 0)
	ndivs = 10;

    if (locgrph->logy) {

	/* find smallest power of 10 greater than starting point */
	for (pwr = 76; p[pwr] > y0; pwr--);
	label_offset = 0.;
    }
    else {

	/* find greatest power of 10 less than axis length */
	xylen = yn - y0;
	for (pwr = 76; p[pwr] >= xylen; pwr--);
	label_offset = offset;
    }
    
    if (pwr <2)
      pwr = 2;
    /* above added by sid to try to eliminate a bug */

    ptr = 0.;
    cnt = ((locgrph->logy == 0) ? ((double) ndivs) : 0.);
    divcon = p[pwr] / (double) ndivs;
    eps =.5 * p[pwr - 2];
    ts = tside;
    ns = nside;

    if (y0 + label_offset != 0.) {
	while (1) {
	    if (locgrph->logy)
		cnt += 1.;

	    /* step ptr by p[pwr] until it passes y0 */
	    if (y0 + label_offset > 0.) 
		{
			ptr += p[pwr];
			if (ptr > y0 + label_offset)
		    	break;
	    }
	    else 
		{
			ptr -= p[pwr];
			if (ptr < y0 + label_offset)
		    	break;
	    }
	}
	if (!(locgrph->logy)) {

	    /* (if log axis, ptr and cnt are already set) */
	    if (y0 + label_offset < 0.)
		cnt = 0.;
	    while (1) {

		/*
		 * now move ptr in the opposite direction by steps of divcon
		 * until it passes y0 again. ptr will then equal the tick
		 * mark next to y0 just outside the axis
		 */
		if (y0 + label_offset > 0.) {
		    ptr -= divcon;
		    cnt -= 1.;
		    if (ptr - eps < y0 + label_offset)
			break;
		}
		else {
		    ptr += divcon;
		    cnt += 1.;
		    if (ptr + eps > y0 + label_offset)
			break;
		}
	    }
	    if (y0 + label_offset > 0.) {

		/*
		 * now move ptr in the original direction by divcon
		 */
		ptr += divcon;
		cnt += 1.;
	    }
	    /* ptr now equals the first tick mark after y0 */
	}
    }
    ptr -= label_offset;

    /* draw axis line and major tick mark at start point */
    pix_x0 = USER2PIXX(locgrph, x0);
    pix_y0 = USER2PIXY(locgrph, y0);
    pix_y1 = USER2PIXY(locgrph, yn);
    gdk_draw_line(pixmap, grct, pix_x0, pix_y0, pix_x0, pix_y1);
    g_ymajt(y0, pix_x0, pix_y0, 1, ABOVE);

    if (locgrph->logy) {
	while (ptr <= yn + eps) {
	    pix_y0 = USER2PIXY(locgrph, ptr);
	    if (cnt == 10.) {
		g_ymajt(ptr, pix_x0, pix_y0, 0, MIDM);
		cnt = 1.;
		pwr++;
	    }
	    else
		g_ymidt(pix_x0, pix_y0);
	    ptr += p[pwr];
	    cnt += 1.;
	}
    }
    else {
	while (ptr <= yn + eps) {
	    pix_y0 = USER2PIXY(locgrph, ptr);
	    if (g_eq(cnt, (double) ndivs,.1)) {
		/* at major tick mark */
		g_ymajt(ptr, pix_x0, pix_y0, 0, MIDM);
		cnt = 1.;
	    }
	    else {

		/*
		 * output middle tick mark if ndivs even and halfway between
		 * major tick marks
		 */
		if (g_eq(cnt, (double) ndivs *.5,.1))
		    g_ymidt(pix_x0, pix_y0);
		else
		    g_ymint(pix_x0, pix_y0);
		cnt += 1.;
	    }
	    ptr += divcon;
	}
    }
    g_ymajt(yn, pix_x0, pix_y1, 1, BELOW);
}

void g_ymajt(pt, xpt, ypt, print, pos)
// major tick y axis
    double          pt;
    int             xpt, ypt, print, pos;
{
    int             len, hz_offset, pix_x0=0, pix_y0=0;
    char            outpt[20];

    // plot tick 
	settings.general.font.font = AXISF;

    if (ts == 1)
		gdk_draw_line(pixmap, grct, xpt, ypt, xpt + majtick, ypt);
    else 
		if (ts == -1)
			gdk_draw_line(pixmap, grct, xpt, ypt, xpt - majtick, ypt);
    if (ns == 0 || print == 0)
		return;

    // create label
    switch(frmt)
    {
    	case MYFLOAT:
		    if (g_eq(0., pt, p[pwr - 5]))
				pt = 0.;
		    if ((int) pt/1000000 > 0)
				sprintf(outpt, "%.5g", pt + label_offset);
		    else 
			{
				memset(outpt, 0, 20);
				if (abs(pt) < 1.)
				{
					sprintf(outpt, "%.8f", pt);
				}
				else
				{
					sprintf(outpt, "%.3f", pt);
				}
				for(len=strlen(outpt)-1;len > 0; len--) 
				{
	    			if(outpt[len] == '0')
						outpt[len] = 0;
	    			else
						break;
				}
		    }
		break;
		
		case MYINT:
			memset(outpt, 0, 20);
			sprintf(outpt, "%d", (int) pt);
		break;
	}

    /*
     * determine length and position of tick label; if label will overlap the
     * previous label, don't print it
     */
    for (len = 0; outpt[len] != '\0'; len++);
    if (outpt[len - 1] == '.') 
	{
		if(abs(pt)>1)
			outpt[len - 1] = '\0';
		else
			strcpy(outpt, "0.0");
    }
	charheight = fontSIZE(outpt, HEIGHT);
	hz_offset = fontSIZE(outpt, WIDTH);
    switch(pos) 
	{
		case MIDM:
			pix_y0 = ypt + charheight / 2;
			break;

		case BELOW:
			pix_y0 = ypt; // + charheight -15;
			break;

		case ABOVE:
			pix_y0 = ypt - charheight;
			break;
    }

	/* print tick label */
	if ((ns == 1) && (ts != 1))
	    pix_x0 = xpt + majtick;
	else if ((ns == 1) && (ts == 1))
	    pix_x0 = xpt + 2 * majtick;
	else if ((ns == -1) && (ts == -1))
	    pix_x0 = xpt - hz_offset - 2 * majtick;
	else if ((ns == -1) && (ts != -1))
	    pix_x0 = xpt - hz_offset - majtick/2;
	fontDraw(pixmap, outpt, pix_x0, pix_y0, NORMALGC, SAMESCR, -1, -1);
}

void g_ymidt(xpt, ypt)
/* middle tick y axis */
    int             xpt, ypt;
{
    if (ts == 1)
	gdk_draw_line(pixmap, grct, xpt, ypt, xpt + midtick, ypt);
    else if (ts == -1)
	gdk_draw_line(pixmap, grct, xpt, ypt, xpt - midtick, ypt);
}

void g_ymint(xpt, ypt)
/* minor tick y axis */
    int             xpt, ypt;
{
    if (ts == 1)
	gdk_draw_line(pixmap, grct, xpt, ypt, xpt + midtick, ypt);
    else if (ts == -1)
	gdk_draw_line(pixmap, grct, xpt, ypt, xpt - midtick, ypt);
}

gboolean g_eq(x, y, e)
/* equal comparison of doubles */
    double          x, y, e;
{
    if ((x < y - e) || (x > y + e))
	return (0);
    else
	return (1);
}
