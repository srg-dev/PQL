#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <time.h>
#include "pql_defines.h"
#include "pql_externs.h"
#include "gtk_pql_externs.h"
#include <gdk/gdk.h>
#include <unistd.h>

static FILE *PSfp;
static int	paperW, paperH;

static int print_ps_image (GdkPixbuf *pixbuf, int imageNum, float imageP)
{
	char linebuf[80];
	int x, y;
	int pos;
	int startline, ix;
	unsigned char b;
	const char tohex[16] = "0123456789abcdef";
	int bytes_per_line;
	int	width, height, rowstride, bytes_per_pixel=3;
	guchar *data;
	static float	totalIP;
	int	transW=0, transH=0, scaleW=0, scaleH=0;

	data = gdk_pixbuf_get_pixels(pixbuf);
	width = gdk_pixbuf_get_width(pixbuf);
	height = gdk_pixbuf_get_height(pixbuf);
	rowstride = gdk_pixbuf_get_rowstride(pixbuf);

	bytes_per_line = width * bytes_per_pixel;

	fprintf(PSfp, "DisplayImage\n");
	switch (imageNum)
	{
		case 1:
			totalIP = imageP;
			transW = PMARGIN+5;		// lower left corner placement of the image on paper
			transH = PMARGIN*6;
			scaleW = paperW-100;	// size of the image on paper
			scaleH = (paperH-94) * imageP;
			break;

		case 2:
		case 3:
			transW = PMARGIN+5;		// lower left corner placement of the image on paper
			transH = PMARGIN*6 + (paperH-94)*totalIP + imageNum; 
			scaleW = paperW-100;	// size of the image on paper
			scaleH = (paperH-94) * imageP;
			totalIP += imageP;
			break;
	}
	fprintf(PSfp, "%d %d\n", transW, transH);		// translate arguments
	fprintf(PSfp, "%d %d\n", scaleW, scaleH);		// scale arguments
	fprintf(PSfp, "%d %d\n", width, height);		// size of image to plot (sizeof(pixmap))
	fprintf(PSfp, "0\n");
	fprintf(PSfp, "0\n");

	pos = 0;
	startline = 0;
	for (y = 0; y < height; y++)
	{
	    ix = startline;
	    for (x = 0; x < bytes_per_line; x++)
		{
	  		b = data[ix++];
	  		linebuf[pos++] = tohex[b >> 4];
	  		linebuf[pos++] = tohex[b & 15];
	  		if (pos == 72)
	    	{
	      		linebuf[pos++] = '\n';
	      		if (fwrite (linebuf, sizeof(char), pos, PSfp) < pos)
					return -1;
	      		pos = 0;
	    	}
		}
    	startline += rowstride;
	}
	if (pos)
  	{
    	linebuf[pos++] = '\n';
      	if (fwrite (linebuf, sizeof(char), pos, PSfp) < pos)
			return -1;
  	}
	return 0;
}

static int printPSHeadFoot(char fileName[255], int HF, int pWidth, int pHeight)
{
	struct tm *timenow;
	time_t		tloc;
	
	time(&tloc);
	timenow = gmtime(&tloc);

	switch(HF)
	{
		case PRINTHEADER:
			paperW = pWidth;
			paperH = pHeight;

			if ((PSfp=fopen(fileName, "w")) == NULL)
			{
				states.msg = g_strdup_printf("Unable to open Print File %s for PostScript Output.", fileName);
				return FAILURE;
			}

			fprintf(PSfp, "%%!PS-Adobe-3.0\n");
			fprintf(PSfp, "%%%%Creator: (PQL II)\n");
			fprintf(PSfp, "%%%%Title: (%s)\n", fileName);
			fprintf(PSfp, "%%%%CreationDate: (%d:%03d:%02d:%02d:%02d)\n", 
							timenow->tm_year+1900, timenow->tm_yday+1,
							timenow->tm_hour, timenow->tm_min, timenow->tm_sec);
			fprintf(PSfp, "%%%%BoundingBox: 0 0 %d %d\n", pWidth, pHeight);
			fprintf(PSfp, "%%%%HiResBoundingBox: 0 0 %d %d\n", pWidth, pHeight);
			fprintf(PSfp, "%%%%DocumentData: Clean7Bit\n");
			fprintf(PSfp, "%%%%LanguageLevel: 1\n");
//			fprintf(PSfp, "%%%%Orientation: Landscape\n");		// landscape now hard-coded, see below
			fprintf(PSfp, "%%%%PageOrder: Ascend\n");
			fprintf(PSfp, "%%%%Pages: 1\n");
			fprintf(PSfp, "%%%%EndComments\n");
			fprintf(PSfp, "\n");
			fprintf(PSfp, "%%%%BeginDefaults\n");
			fprintf(PSfp, "%%%%EndDefaults\n");
			fprintf(PSfp, "\n");
			fprintf(PSfp, "%%%%BeginProlog\n");
			fprintf(PSfp, "%%\n");
			fprintf(PSfp, "%% Display a color image.  The image is displayed in color on\n");
			fprintf(PSfp, "%% Postscript viewers or printers that support color, otherwise\n");
			fprintf(PSfp, "%% it is displayed as grayscale.\n");
			fprintf(PSfp, "%%\n");
			fprintf(PSfp, "/DirectClassPacket\n");
			fprintf(PSfp, "{\n");
			fprintf(PSfp, "  %%\n");
			fprintf(PSfp, "  %% Get a DirectClass packet.\n");
			fprintf(PSfp, "  %%\n");
			fprintf(PSfp, "  %% Parameters:\n");
			fprintf(PSfp, "  %%   red.\n");
			fprintf(PSfp, "  %%   green.\n");
			fprintf(PSfp, "  %%   blue.\n");
			fprintf(PSfp, "  %%   length: number of pixels minus one of this color (optional).\n");
			fprintf(PSfp, "  %%\n");
			fprintf(PSfp, "  currentfile color_packet readhexstring pop pop\n");
			fprintf(PSfp, "  compression 0 eq\n");
			fprintf(PSfp, "  {\n");
			fprintf(PSfp, "    /number_pixels 3 def\n");
			fprintf(PSfp, "  }\n");
			fprintf(PSfp, "  {\n");
			fprintf(PSfp, "    currentfile byte readhexstring pop 0 get\n");
			fprintf(PSfp, "    /number_pixels exch 1 add 3 mul def\n");
			fprintf(PSfp, "  } ifelse\n");
			fprintf(PSfp, "  0 3 number_pixels 1 sub\n");
			fprintf(PSfp, "  {\n");
			fprintf(PSfp, "    pixels exch color_packet putinterval\n");
			fprintf(PSfp, "  } for\n");
			fprintf(PSfp, "  pixels 0 number_pixels getinterval\n");
			fprintf(PSfp, "} bind def\n");
			fprintf(PSfp, "\n");
			fprintf(PSfp, "/DirectClassImage\n");
			fprintf(PSfp, "{\n");
			fprintf(PSfp, "  %%\n");
			fprintf(PSfp, "  %% Display a DirectClass image.\n");
			fprintf(PSfp, "  %%\n");
			fprintf(PSfp, "  systemdict /colorimage known\n");
			fprintf(PSfp, "  {\n");
			fprintf(PSfp, "    columns rows 8\n");
			fprintf(PSfp, "    [\n");
			fprintf(PSfp, "      columns 0 0\n");
			fprintf(PSfp, "      rows neg 0 rows\n");
			fprintf(PSfp, "    ]\n");
			fprintf(PSfp, "    { DirectClassPacket } false 3 colorimage\n");
			fprintf(PSfp, "  }\n");
			fprintf(PSfp, "  {\n");
			fprintf(PSfp, "    %%\n");
			fprintf(PSfp, "    %% No colorimage operator;  convert to grayscale.\n");
			fprintf(PSfp, "    %%\n");
			fprintf(PSfp, "    columns rows 8\n");
			fprintf(PSfp, "    [\n");
			fprintf(PSfp, "      columns 0 0\n");
			fprintf(PSfp, "      rows neg 0 rows\n");
			fprintf(PSfp, "    ]\n");
			fprintf(PSfp, "    { GrayDirectClassPacket } image\n");
			fprintf(PSfp, "  } ifelse\n");
			fprintf(PSfp, "} bind def\n");
			fprintf(PSfp, "\n");
			fprintf(PSfp, "/GrayDirectClassPacket\n");
			fprintf(PSfp, "{\n");
			fprintf(PSfp, "  %%\n");
			fprintf(PSfp, "  %% Get a DirectClass packet;  convert to grayscale.\n");
			fprintf(PSfp, "  %%\n");
			fprintf(PSfp, "  %% Parameters:\n");
			fprintf(PSfp, "  %%   red\n");
			fprintf(PSfp, "  %%   green\n");
			fprintf(PSfp, "  %%   blue\n");
			fprintf(PSfp, "  %%   length: number of pixels minus one of this color (optional).\n");
			fprintf(PSfp, "  %%\n");
			fprintf(PSfp, "  currentfile color_packet readhexstring pop pop\n");
			fprintf(PSfp, "  color_packet 0 get 0.299 mul\n");
			fprintf(PSfp, "  color_packet 1 get 0.587 mul add\n");
			fprintf(PSfp, "  color_packet 2 get 0.114 mul add\n");
			fprintf(PSfp, "  cvi\n");
			fprintf(PSfp, "  /gray_packet exch def\n");
			fprintf(PSfp, "  compression 0 eq\n");
			fprintf(PSfp, "  {\n");
			fprintf(PSfp, "    /number_pixels 1 def\n");
			fprintf(PSfp, "  }\n");
			fprintf(PSfp, "  {\n");
			fprintf(PSfp, "    currentfile byte readhexstring pop 0 get\n");
			fprintf(PSfp, "    /number_pixels exch 1 add def\n");
			fprintf(PSfp, "  } ifelse\n");
			fprintf(PSfp, "  0 1 number_pixels 1 sub\n");
			fprintf(PSfp, "  {\n");
			fprintf(PSfp, "    pixels exch gray_packet put\n");
			fprintf(PSfp, "  } for\n");
			fprintf(PSfp, "  pixels 0 number_pixels getinterval\n");
			fprintf(PSfp, "} bind def\n");
			fprintf(PSfp, "\n");
			fprintf(PSfp, "/GrayPseudoClassPacket\n");
			fprintf(PSfp, "{\n");
			fprintf(PSfp, "  %%\n");
			fprintf(PSfp, "  %% Get a PseudoClass packet;  convert to grayscale.\n");
			fprintf(PSfp, "  %%\n");
			fprintf(PSfp, "  %% Parameters:\n");
			fprintf(PSfp, "  %%   index: index into the colormap.\n");
			fprintf(PSfp, "  %%   length: number of pixels minus one of this color (optional).\n");
			fprintf(PSfp, "  %%\n");
			fprintf(PSfp, "  currentfile byte readhexstring pop 0 get\n");
			fprintf(PSfp, "  /offset exch 3 mul def\n");
			fprintf(PSfp, "  /color_packet colormap offset 3 getinterval def\n");
			fprintf(PSfp, "  color_packet 0 get 0.299 mul\n");
			fprintf(PSfp, "  color_packet 1 get 0.587 mul add\n");
			fprintf(PSfp, "  color_packet 2 get 0.114 mul add\n");
			fprintf(PSfp, "  cvi\n");
			fprintf(PSfp, "  /gray_packet exch def\n");
			fprintf(PSfp, "  compression 0 eq\n");
			fprintf(PSfp, "  {\n");
			fprintf(PSfp, "    /number_pixels 1 def\n");
			fprintf(PSfp, "  }\n");
			fprintf(PSfp, "  {\n");
			fprintf(PSfp, "    currentfile byte readhexstring pop 0 get\n");
			fprintf(PSfp, "    /number_pixels exch 1 add def\n");
			fprintf(PSfp, "  } ifelse\n");
			fprintf(PSfp, "  0 1 number_pixels 1 sub\n");
			fprintf(PSfp, "  {\n");
			fprintf(PSfp, "    pixels exch gray_packet put\n");
			fprintf(PSfp, "  } for\n");
			fprintf(PSfp, "  pixels 0 number_pixels getinterval\n");
			fprintf(PSfp, "} bind def\n");
			fprintf(PSfp, "\n");
			fprintf(PSfp, "/PseudoClassPacket\n");
			fprintf(PSfp, "{\n");
			fprintf(PSfp, "  %%\n");
			fprintf(PSfp, "  %% Get a PseudoClass packet.\n");
			fprintf(PSfp, "  %%\n");
			fprintf(PSfp, "  %% Parameters:\n");
			fprintf(PSfp, "  %%   index: index into the colormap.\n");
			fprintf(PSfp, "  %%   length: number of pixels minus one of this color (optional).\n");
			fprintf(PSfp, "  %%\n");
			fprintf(PSfp, "  currentfile byte readhexstring pop 0 get\n");
			fprintf(PSfp, "  /offset exch 3 mul def\n");
			fprintf(PSfp, "  /color_packet colormap offset 3 getinterval def\n");
			fprintf(PSfp, "  compression 0 eq\n");
			fprintf(PSfp, "  {\n");
			fprintf(PSfp, "    /number_pixels 3 def\n");
			fprintf(PSfp, "  }\n");
			fprintf(PSfp, "  {\n");
			fprintf(PSfp, "    currentfile byte readhexstring pop 0 get\n");
			fprintf(PSfp, "    /number_pixels exch 1 add 3 mul def\n");
			fprintf(PSfp, "  } ifelse\n");
			fprintf(PSfp, "  0 3 number_pixels 1 sub\n");
			fprintf(PSfp, "  {\n");
			fprintf(PSfp, "    pixels exch color_packet putinterval\n");
			fprintf(PSfp, "  } for\n");
			fprintf(PSfp, "  pixels 0 number_pixels getinterval\n");
			fprintf(PSfp, "} bind def\n");
			fprintf(PSfp, "\n");
			fprintf(PSfp, "/PseudoClassImage\n");
			fprintf(PSfp, "{\n");
			fprintf(PSfp, "  %%\n");
			fprintf(PSfp, "  %% Display a PseudoClass image.\n");
			fprintf(PSfp, "  %%\n");
			fprintf(PSfp, "  %% Parameters:\n");
			fprintf(PSfp, "  %%   class: 0-PseudoClass or 1-Grayscale.\n");
			fprintf(PSfp, "  %%\n");
			fprintf(PSfp, "  currentfile buffer readline pop\n");
			fprintf(PSfp, "  token pop /class exch def pop\n");
			fprintf(PSfp, "  class 0 gt\n");
			fprintf(PSfp, "  {\n");
			fprintf(PSfp, "    currentfile buffer readline pop\n");
			fprintf(PSfp, "    token pop /depth exch def pop\n");
			fprintf(PSfp, "    /grays columns 8 add depth sub depth mul 8 idiv string def\n");
			fprintf(PSfp, "    columns rows depth\n");
			fprintf(PSfp, "    [\n");
			fprintf(PSfp, "      columns 0 0\n");
			fprintf(PSfp, "      rows neg 0 rows\n");
			fprintf(PSfp, "    ]\n");
			fprintf(PSfp, "    { currentfile grays readhexstring pop } image\n");
			fprintf(PSfp, "  }\n");
			fprintf(PSfp, "  {\n");
			fprintf(PSfp, "    %%\n");
			fprintf(PSfp, "    %% Parameters:\n");
			fprintf(PSfp, "    %%   colors: number of colors in the colormap.\n");
			fprintf(PSfp, "    %%   colormap: red, green, blue color packets.\n");
			fprintf(PSfp, "    %%\n");
			fprintf(PSfp, "    currentfile buffer readline pop\n");
			fprintf(PSfp, "    token pop /colors exch def pop\n");
			fprintf(PSfp, "    /colors colors 3 mul def\n");
			fprintf(PSfp, "    /colormap colors string def\n");
			fprintf(PSfp, "    currentfile colormap readhexstring pop pop\n");
			fprintf(PSfp, "    systemdict /colorimage known\n");
			fprintf(PSfp, "    {\n");
			fprintf(PSfp, "      columns rows 8\n");
			fprintf(PSfp, "      [\n");
			fprintf(PSfp, "        columns 0 0\n");
			fprintf(PSfp, "        rows neg 0 rows\n");
			fprintf(PSfp, "      ]\n");
			fprintf(PSfp, "      { PseudoClassPacket } false 3 colorimage\n");
			fprintf(PSfp, "    }\n");
			fprintf(PSfp, "    {\n");
			fprintf(PSfp, "      %%\n");
			fprintf(PSfp, "      %% No colorimage operator;  convert to grayscale.\n");
			fprintf(PSfp, "      %%\n");
			fprintf(PSfp, "      columns rows 8\n");
			fprintf(PSfp, "      [\n");
			fprintf(PSfp, "        columns 0 0\n");
			fprintf(PSfp, "        rows neg 0 rows\n");
			fprintf(PSfp, "      ]\n");
			fprintf(PSfp, "      { GrayPseudoClassPacket } image\n");
			fprintf(PSfp, "    } ifelse\n");
			fprintf(PSfp, "  } ifelse\n");
			fprintf(PSfp, "} bind def\n");
			fprintf(PSfp, "\n");
			fprintf(PSfp, "/DisplayImage\n");
			fprintf(PSfp, "{\n");
			fprintf(PSfp, "  %%\n");
			fprintf(PSfp, "  %% Display a DirectClass or PseudoClass image.\n");
			fprintf(PSfp, "  %%\n");
			fprintf(PSfp, "  %% Parameters:\n");
			fprintf(PSfp, "  %%   x & y translation.\n");
			fprintf(PSfp, "  %%   x & y scale.\n");
			fprintf(PSfp, "  %%   image columns & rows.\n");
			fprintf(PSfp, "  %%   class: 0-DirectClass or 1-PseudoClass.\n");
			fprintf(PSfp, "  %%   compression: 0-none or 1-RunlengthEncoded.\n");
			fprintf(PSfp, "  %%   hex color packets.\n");
			fprintf(PSfp, "  %%\n");
			fprintf(PSfp, "  gsave\n");
			fprintf(PSfp, "  /buffer 512 string def\n");
			fprintf(PSfp, "  /byte 1 string def\n");
			fprintf(PSfp, "  /color_packet 3 string def\n");
			fprintf(PSfp, "  /pixels 768 string def\n");
			fprintf(PSfp, "\n");
			fprintf(PSfp, "  currentfile buffer readline pop\n");
			fprintf(PSfp, "  token pop /x exch def\n");
			fprintf(PSfp, "  token pop /y exch def pop\n");
			fprintf(PSfp, "  x y translate\n");
			fprintf(PSfp, "  currentfile buffer readline pop\n");
			fprintf(PSfp, "  token pop /x exch def\n");
			fprintf(PSfp, "  token pop /y exch def pop\n");
//			fprintf(PSfp, "  %%currentfile buffer readline pop\n");
//			fprintf(PSfp, "  %%token pop /pointsize exch def pop\n");
//			fprintf(PSfp, "  %%/Times-Roman findfont pointsize scalefont setfont\n");
			fprintf(PSfp, "  x y scale\n");
			fprintf(PSfp, "  currentfile buffer readline pop\n");
			fprintf(PSfp, "  token pop /columns exch def\n");
			fprintf(PSfp, "  token pop /rows exch def pop\n");
			fprintf(PSfp, "  currentfile buffer readline pop\n");
			fprintf(PSfp, "  token pop /class exch def pop\n");
			fprintf(PSfp, "  currentfile buffer readline pop\n");
			fprintf(PSfp, "  token pop /compression exch def pop\n");
			fprintf(PSfp, "  class 0 gt { PseudoClassImage } { DirectClassImage } ifelse\n");
			fprintf(PSfp, "  grestore\n");
			fprintf(PSfp, "} bind def\n");
			fprintf(PSfp, "%%%%EndProlog\n");
			fprintf(PSfp, "%%%%Page:  1 1\n");
			fprintf(PSfp, "%%%%PageBoundingBox: 0 0 %d %d\n", pWidth, pHeight);
			fprintf(PSfp, "%d 0 translate 90 rotate\n", pHeight+26);	// landscape mode, tran and rotate
			break;

		case PRINTFOOTER:

			fprintf(PSfp, "%%%%PageTrailer\n");
			fprintf(PSfp, "  showpage\n");
			fprintf(PSfp, "%%%%Trailer\n");
			fprintf(PSfp, "%%%%EOF\n");

			fclose(PSfp);
			break;
	}

	return PQLXSUCCESS;
}

#define STARTP 20
#define SPACEP 40

void plot_dispInfo(int dArea, GdkPixmap *pPixmap, int imageh, int imagew, int format)
{
	char	*str=NULL;
	int		h, w;
	struct tm *timenow;
	time_t		tloc;
	
	time(&tloc);
	timenow = localtime(&tloc);

	switch(format)
	{
		case PRINT_FMT_PS:
			settings.general.font.font = MSGF;
		break;
		case PRINT_FMT_PNG:
			settings.general.font.font = LABELF;
		break;
	}
	
	switch(dArea)
	{
		case TRACESCR:
			switch(settings.trace.amp)
			{
				case WSCALETRC:
					str = g_strdup_printf("TRACE");
				break;
				case WSCALEWIND:
					str = g_strdup_printf("WINDOW");
				break;
			}
			h = fontSIZE(str, HEIGHT);
			w = 0;
			fontDraw(pPixmap, str, STARTP*2+w, imageh-(5+h), NORMALGC, SAMESCR, -1, -1);
			free(str);
			str = g_strdup("Window Scale:");
			fontDraw(pPixmap, str, STARTP+w, imageh-(8+2*h), NORMALGC, SAMESCR, -1, -1);
			w += fontSIZE(str, WIDTH) + SPACEP;
			free(str);

#if 0			
			switch(settings.general.units)
			{
				case MYCOUNTS:
					str = g_strdup_printf("Counts");
				break;
				case MYVOLTS:
					str = g_strdup_printf("Volts");
				break;
			}
			fontDraw(pPixmap, str, STARTP*2+w, imageh-(5+h), NORMALGC, SAMESCR, -1, -1);
			free(str);
#endif
			str = g_strdup("Displaying:");
			fontDraw(pPixmap, str, STARTP+w, imageh-(8+2*h), NORMALGC, SAMESCR, -1, -1);
			free(str);
		break;
		
		case SPECTRASCR:
			switch(settings.spectra.display.amp)
			{
				case WSCALETRC:
					str = g_strdup_printf("TRACE");
				break;
				case WSCALEWIND:
					str = g_strdup_printf("WINDOW");
				break;
			}
			h = fontSIZE(str, HEIGHT);
			w = 0;
			fontDraw(pPixmap, str, STARTP*2+w, imageh-(5+h), NORMALGC, SAMESCR, -1, -1);
			free(str);
			str = g_strdup("Window Scale:");
			fontDraw(pPixmap, str, STARTP+w, imageh-(8+2*h), NORMALGC, SAMESCR, -1, -1);
			w += fontSIZE(str, WIDTH) + SPACEP;
			free(str);
			
#if 0
			switch(settings.general.units)
			{
				case MYCOUNTS:
					str = g_strdup_printf("Counts");
				break;
				case MYVOLTS:
					str = g_strdup_printf("Volts");
				break;
			}
			fontDraw(pPixmap, str, STARTP*2+w, imageh-(5+h), NORMALGC, SAMESCR, -1, -1);
			free(str);
#endif
			str = g_strdup("Displaying:");
			fontDraw(pPixmap, str, STARTP+w, imageh-(8+2*h), NORMALGC, SAMESCR, -1, -1);
			w += fontSIZE(str, WIDTH) + SPACEP;
			free(str);
		break;
		
		default:	// MAGNIFY and SPLIT
			switch(settings.magnify.Tamp)
			{
				case FIXEDS:
					str = g_strdup_printf("FIXED");
				break;
				case DISPS:
					str = g_strdup_printf("DISPLAY");
				break;
				case WINDS:
					str = g_strdup_printf("TRACE");
				break;
			}
			h = fontSIZE(str, HEIGHT);
			w = 0;
			fontDraw(pPixmap, str, STARTP*2+w, imageh-(5+h), NORMALGC, SAMESCR, -1, -1);
			free(str);
			str = g_strdup("Trace Scale:");
			fontDraw(pPixmap, str, STARTP+w, imageh-(8+2*h), NORMALGC, SAMESCR, -1, -1);
			w += fontSIZE(str, WIDTH) + SPACEP;
			free(str);
			
			switch(settings.magnify.Wamp)
			{
				case WSCALETRC:
					str = g_strdup_printf("TRACE");
				break;
				case WSCALEWIND:
					str = g_strdup_printf("WINDOW");
				break;
			}
			fontDraw(pPixmap, str, STARTP*2+w, imageh-(5+h), NORMALGC, SAMESCR, -1, -1);
			free(str);
			str = g_strdup("Window Scale:");
			fontDraw(pPixmap, str, STARTP+w, imageh-(8+2*h), NORMALGC, SAMESCR, -1, -1);
			w += fontSIZE(str, WIDTH) + SPACEP;
			free(str);
			
#if 0
			switch(settings.general.units)
			{
				case MYCOUNTS:
					str = g_strdup_printf("Counts");
				break;
				case MYVOLTS:
					str = g_strdup_printf("Volts");
				break;
			}
			fontDraw(pPixmap, str, STARTP*2+w, imageh-(5+h), NORMALGC, SAMESCR, -1, -1);
			free(str);
#endif
			str = g_strdup("Displaying:");
			fontDraw(pPixmap, str, STARTP+w, imageh-(8+2*h), NORMALGC, SAMESCR, -1, -1);
			w += fontSIZE(str, WIDTH) + SPACEP;
			free(str);
		break;
	}
	
	switch(dArea)
	{
		case TRACESCR:		// no filters for TRACESCR
		break;
		
		default:			// possible filters for all others
			if (!(states.tForm.on))
				break;
				
			if (settings.general.filter.filts[states.tForm.filter]->poles[HIGHF])
			{
				str = g_strdup_printf("High Pass Filter (%s):", 
						settings.general.filter.filts[states.tForm.filter]->name);
				fontDraw(pPixmap, str, STARTP+w, imageh-(8+2*h), NORMALGC, SAMESCR, -1, -1);
				free(str);
				str = g_strdup_printf("Poles: %d  Cutoff: %f", 
							settings.general.filter.filts[states.tForm.filter]->poles[HIGHF],
							settings.general.filter.filts[states.tForm.filter]->cutoff[HIGHF]);
				fontDraw(pPixmap, str, STARTP*2+w, imageh-(5+h), NORMALGC, SAMESCR, -1, -1);
				w += fontSIZE(str, WIDTH) + 2*SPACEP;
				free(str);
			}
			if (settings.general.filter.filts[states.tForm.filter]->poles[LOWF])
			{
				str = g_strdup_printf("Poles: %d  Cutoff: %f", 
							settings.general.filter.filts[states.tForm.filter]->poles[LOWF],
							settings.general.filter.filts[states.tForm.filter]->cutoff[LOWF]);
				fontDraw(pPixmap, str, STARTP*2+w, imageh-(5+h), NORMALGC, SAMESCR, -1, -1);
				free(str);
				str = g_strdup_printf("Low Pass Filter (%s):", 
						settings.general.filter.filts[states.tForm.filter]->name);
				fontDraw(pPixmap, str, STARTP+w, imageh-(8+2*h), NORMALGC, SAMESCR, -1, -1);
				free(str);
			}
		break;
	}

	str = g_strdup_printf("%d:%03d %02d:%02d:%02d", 
							timenow->tm_year+1900, timenow->tm_yday+1,
							timenow->tm_hour, timenow->tm_min, timenow->tm_sec);
	w = imagew-20-fontSIZE(str, WIDTH);
	fontDraw(pPixmap, str, w, imageh-(5+h), NORMALGC, SAMESCR, -1, -1);
	free(str);	
	str = g_strdup("PQL II - Print");
	fontDraw(pPixmap, str, w, imageh-(8+2*h), NORMALGC, SAMESCR, -1, -1);
	free(str);	
}

static void plotDA(GdkPixbuf **pPixbuf, int dArea, int imageW, int imageh)
{
	GdkPixmap	*pPixmap;
	GtkWidget	*da;
	int		screen, imageH;
	char	*str;
	gboolean	plotDispInfo=TRUE, split=TRUE;

	// set up which drawing area and screen type that requires printing
	da = DA[dArea];
	switch(dArea)
	{
		case SPLITDA2B:
			screen = settings.split.DA[states.split.panes][BOT];
		break;
			
		case SPLITDA2T:
			screen = settings.split.DA[states.split.panes][TOP];
			plotDispInfo = FALSE;
		break;
			
		case SPLITDA3B:
			screen = settings.split.DA[states.split.panes][BOT];
		break;
			
		case SPLITDA3M:
			screen = settings.split.DA[states.split.panes][MID];
			plotDispInfo = FALSE;
		break;
			
		case SPLITDA3T:
			screen = settings.split.DA[states.split.panes][TOP];
			plotDispInfo = FALSE;
		break;
		
		default:
			screen = dArea;
			split = FALSE;
		break;
	}

	pPixmap = gdk_pixmap_new(da->window, imageW, imageh, -1);						// make our pixmap
	gdk_draw_rectangle (pPixmap, PRINTGC[ABNORMALGC], TRUE, 0, 0, imageW, imageh);	// draw the background

	// draw the data to the pixmap
	settings.general.font.font = MSGF;
	str = g_strdup("TESTING 1, 2, 3");
	if (plotDispInfo)
		imageH = imageh-13-2*fontSIZE(str, HEIGHT);
	else 
		imageH = imageh;
	free(str);
	switch(screen)
	{
		case TRACESCR:
			plot_files(da, pPixmap, imageW, imageH, split);
			plot_extras(da, pPixmap, PRINTDISPLAY, TRACESCR, FALSE, imageW, imageH);
			break;

		case MAGNIFYSCR:
			magnify_proc(da, pPixmap, imageW, imageH, TRUE);
			plot_extras(da, pPixmap, PRINTDISPLAY, MAGNIFYSCR, FALSE, imageW, imageH);
			break;

		case SPECTRASCR:
			transform_proc(da, pPixmap, imageW, imageH, FALSE);
			plot_extras(da, pPixmap, PRINTDISPLAY, SPECTRASCR, FALSE, imageW, imageH);
			break;
	}
	if (plotDispInfo)
		plot_dispInfo(dArea, pPixmap, imageh, imageW, PRINT_FMT_PS);

	// draw a box around our picture
	gdk_draw_line(pPixmap, GXC[NORMALGC][screen], 0, 0, imageW-1, 0);						// top frame line
	gdk_draw_line(pPixmap, GXC[NORMALGC][screen], 0, 0, 0, imageh-1);						// left 
	gdk_draw_line(pPixmap, GXC[NORMALGC][screen], imageW-1, 0, imageW-1, imageh-1);			// right
	gdk_draw_line(pPixmap, GXC[NORMALGC][screen], 0, imageH-1, imageW-1, imageH-1);			// bottom1
	if (plotDispInfo)
		gdk_draw_line(pPixmap, GXC[NORMALGC][screen], 0, imageh-1, imageW-1, imageh-1);		// bottom2

	// convert the pixmap to a pixbuf
	if (!(*pPixbuf = gdk_pixbuf_get_from_drawable(NULL, pPixmap, NULL, 0, 0, 0, 0, -1, -1)))
	{
		states.msg = g_strdup("Internal Error: Failed to Create PIXBUF for Printing, Please Report");
		fclose(PSfp);
	}

	g_object_unref(pPixmap);

	return;
}

void printPS(int scr)
{
	GdkPixbuf	*pixbuf;
	int			paperW=0, paperH=0, currentScreen, imageW, imageH, imageTH;
	float		imageP;
	int			PSfd;
	int			fileType;
	char 		*out, *fileName;

	currentScreen = states.currentScreen;		// save our current display state
	states.currentScreen = PRINT;

	switch(settings.general.print.paperSize)
	{
		case LETTER:
			paperW = LTRH;		// landscape mode
			paperH = LTRW;
		break;

		case A4:
			paperW = A4H;		// landscape mode
			paperH = A4W;
		break;
	}

	if ((!settings.general.print.fileN || 
		 !settings.general.print.fileN[0]) &&
		(!settings.general.print.command  ||
		 !settings.general.print.command[0]))
	{	// no filename or print cmd specified, request filename from user
		fileType = USER;
	}
	else if ((!settings.general.print.fileN || 
		 !settings.general.print.fileN[0]) &&
		(settings.general.print.command  &&
		 settings.general.print.command[0]))
		{	// no filename specified and printing, use a temporary file
			fileType = TEMP;
		}
		else 
		{	// filename specified, use as BASE
			fileType = BASE;
		}

	switch (fileType)
	{
		case USER:
			getFilename(NULL, GINT_TO_POINTER(PRINTFTYPE));
			if (!settings.general.print.fileN[0])
			{	// if the user cancels, do nothing
				states.currentScreen = currentScreen;
				gdk_window_set_cursor(topLevel, NULL);
				return;
			}
			fileName = g_strdup(settings.general.print.fileN);
		break;
		
		case TEMP:
			settings.general.print.fileN = g_strdup("/tmp/pql.XXXXXX");
#ifdef WIN32
			if ((PSfd = mktemp(settings.general.print.fileN))!= 1)
#else
			if ((PSfd = mkstemp(settings.general.print.fileN))!= 1)
#endif
			{	// create and open the temporary file
				close(PSfd);	// but i don't want a file descriptor, so close it and proceed
			}
			fileName = g_strdup(settings.general.print.fileN);
		break;
		
		case BASE:
		{
			int i;
			FILE *fp;
			char	sedFile[255], *tmpFile, *zeroFile;
			tmpFile = g_strdup("/tmp/pqlLastFileName");
	
			for (i=0;i<strlen(settings.general.print.fileN); i++)
			{
				if (settings.general.print.fileN[i] == '/')
				{
					sedFile[i] = '.';
				}
				else
				{
					sedFile[i] = settings.general.print.fileN[i];
				}
			}
			sedFile[i] = 0;
			zeroFile = g_strdup_printf("%s0.ps", settings.general.print.fileN);
			out = g_strdup_printf("touch %s", zeroFile);
			(void) system(out);	free(out);
			out = g_strdup_printf("ls %s* | awk 'END {print $1}' | sed s/%s// | sed s/.ps// >%s", 
							settings.general.print.fileN, sedFile, tmpFile);
			(void) system(out);	free(out);
			fp = fopen(tmpFile, "r");
			(void) fscanf(fp, "%d", &i);
			fclose(fp);
			remove(tmpFile);	free(tmpFile);
			remove(zeroFile);	free(zeroFile);
			fileName = g_strdup_printf("%s%d.ps", settings.general.print.fileN, i+1);
		}
		break;
	}

	// print the PS header
	if (printPSHeadFoot(fileName, PRINTHEADER, paperW, paperH) == FAILURE)
	{	
		dispMsg(DISPLAY, states.msg);
		free(states.msg);
		states.msg = NULL;
		gdk_beep();
		states.currentScreen = currentScreen;		// restore to current display state
		gdk_window_set_cursor(topLevel, NULL);
		free(fileName);
		return;
	}

	switch(scr)
	{
		case TRACESCR:
		case MAGNIFYSCR:
		case SPECTRASCR:
			imageW = paperW * printRes[settings.general.print.paperRes];
			imageH = paperH * printRes[settings.general.print.paperRes];
			plotDA(&pixbuf, scr, imageW, imageH);

			if (pixbuf == NULL)
				break;

			// postscript out the pixbuf
			print_ps_image(pixbuf, 1, 1);
//sample printing image directly to a .png file:
//gdk_pixbuf_save(pixbuf, "/tmp/testout.png", "png", NULL, NULL);
//			free(pixbuf);
		break;

		case SPLITSCR:
			imageW = paperW * printRes[settings.general.print.paperRes];	// width
			switch(states.split.panes)
			{
				case SPLIT2:
					imageTH =	DA[SPLITDA2B]->allocation.height + 		// total height of all splits
								DA[SPLITDA2T]->allocation.height;

					// draw the bottom drawing area
					imageP = ((float) DA[SPLITDA2B]->allocation.height/(float) imageTH);	// proportion
					imageH = paperH * (imageP) * printRes[settings.general.print.paperRes];
					plotDA(&pixbuf, SPLITDA2B, imageW, imageH);
					if (pixbuf == NULL)
						break;

					// postscript out the pixbuf
					print_ps_image(pixbuf, 1, imageP);
					g_object_unref(pixbuf);
					pixbuf = NULL;

					// draw the top drawing area
					imageP = ((float) DA[SPLITDA2T]->allocation.height/(float) imageTH);	
					imageH = paperH * (imageP) * printRes[settings.general.print.paperRes];
					plotDA(&pixbuf, SPLITDA2T, imageW, imageH);
					if (pixbuf == NULL)
						break;

					// postscript out the pixbuf
					print_ps_image(pixbuf, 2, imageP);
					g_object_unref(pixbuf);
					pixbuf = NULL;
				break;

				case SPLIT3:
					imageTH =	DA[SPLITDA3B]->allocation.height + 
								DA[SPLITDA3T]->allocation.height + 
								DA[SPLITDA3M]->allocation.height;

					// draw the bottom drawing area
					imageP = ((float) DA[SPLITDA3B]->allocation.height/(float) imageTH);
					imageH = paperH * (imageP) * printRes[settings.general.print.paperRes];
					plotDA(&pixbuf, SPLITDA3B, imageW, imageH);
					if (pixbuf == NULL)
						break;

					// postscript out the pixbuf
					print_ps_image(pixbuf, 1, imageP);
					g_object_unref(pixbuf);
					pixbuf = NULL;

					// draw the middle drawing area
					imageP = ((float) DA[SPLITDA3M]->allocation.height/(float) imageTH);
					imageH = paperH * (imageP) * printRes[settings.general.print.paperRes];
					plotDA(&pixbuf, SPLITDA3M, imageW, imageH);
					if (pixbuf == NULL)
						break;

					// postscript out the pixbuf
					print_ps_image(pixbuf, 2, imageP);
					g_object_unref(pixbuf);

					// draw the top drawing area
					imageP = ((float) DA[SPLITDA3T]->allocation.height/(float) imageTH);	
					imageH = paperH * (imageP) * printRes[settings.general.print.paperRes];
					plotDA(&pixbuf, SPLITDA3T, imageW, imageH);
					if (pixbuf == NULL)
						break;

					// postscript out the pixbuf
					print_ps_image(pixbuf, 3, imageP);
				break;
			}
		break;
	}
	if (pixbuf)
	{
		g_object_unref(pixbuf);
		pixbuf = NULL;
	}

	// PS footer
	printPSHeadFoot(fileName, PRINTFOOTER, paperW, paperH);

	if (settings.general.print.command)
	{	// if print command defined, use it
		char	sys[255];
		sprintf(sys, "%s %s", settings.general.print.command, fileName);
		(void) system(sys);
		if (settings.general.print.fileN &&
			settings.general.print.fileN[0] &&
			fileType != TEMP)
		{
			out = g_strdup_printf("Screen Printed to file %s and to Printer", fileName);
		}
		else
		{
			out = g_strdup("Screen Printed to Printer");
		}
	}
	else
	{	// if not, then just to the file
		out = g_strdup_printf("Screen Printed to File %s", fileName);
	}
	dispMsg(DISPLAY, out);
	free(out);

	if (fileType == TEMP)
	{	// remove the tmp file
		remove(settings.general.print.fileN);
	}
	switch(fileType)
	{
		case TEMP:
		case USER:
			free(settings.general.print.fileN);
			settings.general.print.fileN = NULL;
		break;
	}

	states.currentScreen = currentScreen;		// restore to current display state
	free(fileName);
}
