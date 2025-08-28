#ifndef __PQLX_PDF_HEAD
#define __PQLX_PDF_HEAD

// Plot Margin Defs
#define	PDFTOPM		(40)
#define PDFSIDEM	(50)
#define PDFBOTM		(35)
#define	PDFTRCMARG	(10)
#define PDFTRCSEP	(6)
#define PDFTRCDISP	(15)

#define	WWWTOPM		(60)
#define WWWSIDEM	(50)
#define WWWBOTM		(35)

#define THUMBTOPM	(28)
#define THUMBSIDEM	(0)
#define THUMBBOTM	(15)

enum {
	PERSTART,
	PEREND,
	TTLPERBOUNDS
};

typedef struct _BLINEP
{
	int bline_fk;
} BLINEP;

typedef struct _PDFXAXIS
{
	float	bounds[TTLPERBOUNDS];
} PDFXAXIS;

typedef struct _PLOTPDFARGS
{
	GdkPixmap	*pmap;
	gpointer	req;
	int			width, height, scr, type;
	int			fdUnits;
	PDFXAXIS	xAxis;
	int			dateFmt;
	gboolean 	hlnm;
	GSList		*blines;					// data = BLINEDEF
} PLOTPDFARGS;

typedef struct _SPPDFARGS
{
	int	row, col;
} SPPDFARGS;

#endif
