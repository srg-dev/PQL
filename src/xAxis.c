#include <string.h>
#include <math.h>
#include <stdlib.h>
#include "pql_defines.h"
#include "pql_externs.h"
#include "gtk_pql_externs.h"

#define ONEHOUR 3600
static int bdryIvals[] = {	1, 5, 10, 15, 30,				// seconds
					60, 120, 300,							// minutes - 1, 2, 5,
					600, 900, 1800,							// minutes - 10 - 60
					3600, 7200, 14400, 21600, 43200, 86400,	// hours - 1, 2, 4, 6, 12, 24
					86400*5, 86400*10, 86400*15, 86400*20	// days - 5, 10, 15, 20
					};

void xAxisABS(GdkPixmap *pmap, GdkGC *gxc, graph *grph, double start, int scr, gboolean labelDir)
{	// plot absolute time
	struct ptime	date;
	int			msec, x, x1, x2, y1=0, yPos=0, wind_height, wind_width;
	int			i, labelW, labelH, dir=0;
	double		nextPrint;
	float		incIval;
	gchar		*axis_label;
	bounds		*margin=NULL;

	switch(scr)
	{
		case TRACESCR:
			margin = &states.trace.limits.margin[states.currentScreen];
		break;
		case MAGNIFYSCR:
			margin = &states.magnify.limits.margin[states.currentScreen];
		break;
	}
			
	gdk_drawable_get_size(pmap, &wind_width, &wind_height);
	wind_height = grph->pix_ymax;		// for printing, height is less than pixmap size
	passcal_etoh(&date, start);
	msec = (((float) (start - (int) start))*1000.0);

	axis_label = g_strdup_printf("%i %03i %02i:%02i:%02i.%03i" , 
					date.yr, get_julday(date.mo, date.day, date.yr), 
					date.hr, date.mn, (int) date.sec, (int) msec);
	labelW = fontSIZE(axis_label, WIDTH);
	labelH = fontSIZE(axis_label, HEIGHT);
	x1 = (margin->left < labelW+1)
			? 1
			: margin->left - labelW+1;
			
	switch(labelDir)
	{
		case LABEL_BELOW:
			yPos = USER2PIXY(grph, (double) 9);
			y1 = yPos + ((wind_height-yPos)/2 - labelH/2);
			dir = -1;
		break;
		
		case LABEL_ABOVE:
			yPos = USER2PIXY(grph, (double) 9);
			y1 = yPos - (yPos)/2 - labelH/2 - 1;
			dir = 1;
		break;
	}
	fontDraw(pmap, axis_label, x1, y1, NORMALGC, SAMESCR, -1, -1);
	x1 += labelW;
	
	if (grph->user_xdif < 1)
	{	// window length is less than one second - xaxis is old style, with msecs
		xaxis(pmap, GXC[NORMALGC][MAGNIFYSCR], grph,
			  (double) 0.0, (double) 9.0,
			  (double) grph->user_xdif, 
			  1, -1, -1, (double) 0.0, FALSE, start);
	}
	else
	{
		for(i=0;
				(float) (grph->user_xdif/15) >
				(float) bdryIvals[i];
			i++);
		if (bdryIvals[i] > 1)
		{
			for(nextPrint=(int)start+1;
				(int)nextPrint%bdryIvals[i];
				nextPrint++);
			incIval = bdryIvals[i];
		}
		else	// interval is 1 second or less
		{
			if (!((float) (grph->user_xdif/15) <= .5))
			{	// interval is > .5 seconds
				nextPrint = start + (1 - (start - (int) start));
				incIval = 1;
			}
			else
			{
				nextPrint = start + 
							(((start - (int) start) <= .5)
							 ? (.5 - (start - (int) start))
							 : (1 - (start - (int) start)));
				incIval = .5;
			}
		}
		for(; 
			nextPrint < start + grph->user_xdif;
			nextPrint += incIval)
		{
			passcal_etoh(&date, nextPrint);
			if (incIval < 1)
			{	// with seconds and half seconds
				if (!date.hr && !date.mn && !((int)date.sec) && !((int) ((date.sec - (int) date.sec)*10)))
					sprintf(axis_label,"%04i:%03i", date.yr, get_julday(date.mo, date.day, date.yr));
				else
					sprintf(axis_label,"%02i:%02i:%02i.%1d", date.hr, date.mn, (int) date.sec, 
						(int) ((date.sec - (int) date.sec)*10));
			}
			else if (incIval < 60)	//ONEHOUR)
			{	// with seconds
				if (!date.hr && !date.mn && !((int)date.sec))
					sprintf(axis_label,"%04i:%03i", date.yr, get_julday(date.mo, date.day, date.yr));
				else
					sprintf(axis_label,"%02i:%02i:%02i", date.hr, date.mn, (int) date.sec);
			}
			else
			{
				if (!date.hr && !date.mn)
					sprintf(axis_label,"%04i:%03i", date.yr, get_julday(date.mo, date.day, date.yr));
				else
					sprintf(axis_label,"%02i:%02i", date.hr, date.mn);		// without seconds
			}
			x = USER2PIXX(grph, (nextPrint-start));
			gdk_draw_line(pmap, gxc, x, yPos, x, yPos+(4*dir));
			labelW = fontSIZE(axis_label, WIDTH);
			x2 = x - labelW/2;
			if (x2 < x1+2)
				continue;
			if (x2 + labelW > wind_width)
	    		x2 = wind_width - labelW - 1;
			fontDraw(pmap, axis_label, x2, y1, NORMALGC, SAMESCR, -1, -1);
		}
		
		gdk_draw_line(pmap, gxc, margin->left, yPos, margin->right, yPos);
		gdk_draw_line(pmap, gxc, margin->left, yPos, margin->left, yPos+(7*dir));
		gdk_draw_line(pmap, gxc, margin->right, yPos, margin->right, yPos+(7*dir));
	}
	free(axis_label);
}
