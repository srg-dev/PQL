#include <stdio.h>
#include "pql_defines.h"
#include "pql_externs.h"

#define LOG10(x) ((double) (log(x)*.434294))
#define POW10(x) ((double) (exp(x*2.302585)))

/*	This collection of routines allow a user to reference particular pixels
  of a pixmap indirectly through the user's own coordinate system. The user
  creates this mapping explicitly by allocating a "graph" structure (defined in
  "defines.h" and setting its fields as desired, either through calls to
  viewport() and window() or by direct assignment. Mappings may be either
  linear or logarithmic.
	Note that y-values of pixmaps increase going down. The pix_ymin and
  pix_ymax fields should be set to lesser and greater positive integer values
  respectively. The conversion functions will handle the inversion. (It is
  assumed that the user's xy-coordinates increase to the right and up.)
*/

void window(graph *gr, double x1, double x2, double y1, double y2, int lx, int ly)
{
    double          log();

/*  if (lx && ((x1 <= 0.) || (x2 <= 0.))) {
	msg("window: logarithmic mapping containing non-positive points", 1, INVERSE);
	exit(-1);
	}
	if (ly && ((y1 <= 0.) || (y2 <= 0.))) {
	msg("window: logarithmic mapping containing non-positive points", 1, INVERSE);
	exit(-1);
	}
	if (((lx != 0) && (lx != 1)) || ((ly != 0) && (ly != 1))) {
	msg("window: lx and ly must be 0 or 1", 1, INVERSE);
	exit(-1);
	}   
*/
    
    if (lx) {
	gr->user_xmin = LOG10(x1);
	gr->user_xmax = LOG10(x2);
    }
    else {
	gr->user_xmin = x1;
	gr->user_xmax = x2;
    }
    if (ly) {
	gr->user_ymin = LOG10(y1);
	gr->user_ymax = LOG10(y2);
    }
    else {
	gr->user_ymin = (float) y1;
	gr->user_ymax = (float) y2;
    }
    gr->user_xdif = gr->user_xmax - gr->user_xmin;
    gr->user_ydif = (double) gr->user_ymax - (double) gr->user_ymin;
    if (gr->user_ydif == 0) {
	gr->user_ymax += 1;
	gr->user_ymin -= 1;  
        gr->user_ydif = (double) gr->user_ymax - (double) gr->user_ymin;
    }
    gr->logx = lx;
    gr->logy = ly;
}

void viewport(graph *gr, int x1, int x2, int y1, int y2)
/* change area of pixmap to which user-coordinates will be mapped
   y1 must be less than y2, i.e, y1 is 'above' y2 in the actual pixmap;
   note that this does not create a viewport in the strict sense of the
   word, i.e., vectors will not be clipped at the boundaries of the
   viewport, but only at the boundaries of the enclosing pixmap      */
{
    gr->pix_xmin = x1;
    gr->pix_xmax = x2;
    gr->pix_xdif = x2 - x1;
    gr->pix_ymin = y1;
    gr->pix_ymax = y2;
    gr->pix_ydif = y2 - y1;

}

#if 0

int user_to_pix_x(graph *gr, double x)
/* convert user x-coordinates to pixels */
{
    double          a, b, log();
    double          rint();

    if (gr->logx)
	a = LOG10(x);
    else
	a = x;
	a = ((gr)->logx?LOG10(x):x)
    b = ((a - gr->user_xmin) / gr->user_xdif) * ((double) gr->pix_xdif) + ((double) gr->pix_xmin);
    /*
      return ((int) b);
      */
    return ((rint)(b));
}

int user_to_pix_y(graph *gr, double y)
/* convert user y-coordinates to pixels */
{
    double          a, b, log();

    if (gr->logy)
	a = LOG10(y);
    else
	a = y;
    b = ((double) gr->pix_ydif) - ((double) (((gr)->logy ? LOG10(y) : y) - gr->user_ymin) / gr->user_ydif) * ((double) gr->pix_ydif) + ((double) gr->pix_ymin);
    return ((int) b);
}

double pix_to_user_x(graph *gr, int pix_x)
/* convert pixels to user x-coordinates */
{
    double          a, factor, exp();

    factor = ((double) (pix_x - gr->pix_xmin)) / ((double) gr->pix_xdif);
    a = factor * (double)(gr->user_xdif) + gr->user_xmin;
    if (gr->logx)
	a = POW10(a);
    return (a);
    
    return ((a))
    
}

double pix_to_user_y(graph *gr, int pix_y)
/* convert pixels to user y-coordinates */
{
    double          a, factor, exp();

    factor = (1. - (((double) pix_y - gr->pix_ymin) / ((double) gr->pix_ydif)));
    a = factor * gr->user_ydif + gr->user_ymin;
    if (gr->logy)
	a = POW10(a);
    return (a);
}
#endif
