#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <time.h>
#include "pql_defines.h"
#include "pql_externs.h"
#include "gtk_pql_externs.h"

void printPNG(int scr)
{	// convert the pixmap to a pixbuf and output
	int			fileType, width=0, height=0;
	static int	hite;
	char 		*out, *fileName, *str;
	GdkPixmap	*pixmap=NULL;
	GdkPixbuf	*pixbuf;
	GdkColor	*fg=&black, *bg=&white;
	static GdkGC	*gc;

	if (!hite)
	{
		settings.general.font.font = MSGF;
		str = g_strdup("TESTING 1, 2, 3");
		hite = 1.5*fontSIZE(str, HEIGHT);
		free(str);
	}

	if (!gc)
	{
		gc = gdk_gc_new(DAPixmap[TRACESCR]);
	  	gdk_gc_set_rgb_fg_color(gc, fg);
	  	gdk_gc_set_rgb_bg_color(gc, bg);
	    gdk_gc_set_line_attributes(gc, 2, 0, 0, 0);
	}
		
	if (!settings.general.print.fileN ||
		!settings.general.print.fileN[0])
	{	// no filename specified, request filename from user
		fileType = USER;
	}
	else
	{	// filename specified, use as BASE
		fileType = BASE;
	}

	switch (fileType)
	{
		case USER:
			getFilename(NULL, GINT_TO_POINTER(PRINTFTYPE));
			if (!settings.general.print.fileN ||
				!settings.general.print.fileN[0])
			{	// if the user cancels, do nothing
				return;
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
	

	switch(scr)
	{
		case TRACESCR:
		case MAGNIFYSCR:
		case SPECTRASCR:
			width = DA[scr]->allocation.width + 4;
			height = hite + DA[scr]->allocation.height + 6;
			pixmap = gdk_pixmap_new(DAPixmap[scr], width, height, -1);
			gdk_draw_rectangle(pixmap, GXC[ABNORMALGC][scr], TRUE, 0, 0, width, height);
			gdk_draw_drawable(pixmap, gc, DAPixmap[scr], 0, 0, 2, 2, -1, -1);
		break;
		case SPLITSCR:
			switch(states.split.panes)
			{
				case SPLIT2:
					scr = settings.split.DA[states.split.panes][BOT];
					width = DA[SPLITDA2B]->allocation.width + 4;
					height = hite + DA[SPLITDA2B]->allocation.height + DA[SPLITDA2T]->allocation.height + 8;
					pixmap = gdk_pixmap_new(DAPixmap[SPLITDA2B], width, height, -1);
					gdk_draw_rectangle(pixmap, GXC[ABNORMALGC][scr], TRUE, 0, 0, width, height);
					gdk_draw_drawable(pixmap, gc, DAPixmap[SPLITDA2T],
							0, 0, 2, 2, -1, -1);
					gdk_draw_drawable(pixmap, gc, DAPixmap[SPLITDA2B],
							0, 0, 2, 4+DA[SPLITDA2T]->allocation.height, -1, -1);
					gdk_draw_line(pixmap, gc, 1, 3+DA[SPLITDA2T]->allocation.height, 
							width, 3+DA[SPLITDA2T]->allocation.height);
				break;
				case SPLIT3:
					scr = settings.split.DA[states.split.panes][BOT];
					width = DA[SPLITDA2B]->allocation.width + 4;
					height = hite + DA[SPLITDA3B]->allocation.height + 
								DA[SPLITDA3T]->allocation.height + 
								DA[SPLITDA3M]->allocation.height + 10;
					pixmap = gdk_pixmap_new(DAPixmap[SPLITDA3B], width, height, -1);
					gdk_draw_rectangle(pixmap, GXC[ABNORMALGC][scr], TRUE, 0, 0, width, height);
					gdk_draw_drawable(pixmap, gc, DAPixmap[SPLITDA3T],
							0, 0, 2, 2, -1, -1);
					gdk_draw_drawable(pixmap, gc, DAPixmap[SPLITDA3M],
							0, 0, 2, 4+DA[SPLITDA3T]->allocation.height, -1, -1);
					gdk_draw_drawable(pixmap, gc, DAPixmap[SPLITDA3B],
							0, 0, 2, 6+DA[SPLITDA3T]->allocation.height+DA[SPLITDA3M]->allocation.height, -1, -1);
					gdk_draw_line(pixmap, gc, 1, 3+DA[SPLITDA3T]->allocation.height, 
							width, 3+DA[SPLITDA3T]->allocation.height);
					gdk_draw_line(pixmap, gc, 1, 
							5+DA[SPLITDA3T]->allocation.height+DA[SPLITDA3M]->allocation.height,
							width, 
							5+DA[SPLITDA3M]->allocation.height+DA[SPLITDA3T]->allocation.height);
				break;
			}
		break;
	}
	gdk_draw_line(pixmap, gc, 0, 1, width, 1);
	gdk_draw_line(pixmap, gc, 1, 1, 1, height);
	gdk_draw_line(pixmap, gc, width-1, height-1, width-1, 1);
	gdk_draw_line(pixmap, gc, 1, height-1, width, height-1);
	gdk_draw_line(pixmap, gc, 1, height-hite-2, width, height-hite-2);
	plot_dispInfo(scr, pixmap, height, width, PRINT_FMT_PNG);

	if ((pixbuf = gdk_pixbuf_get_from_drawable(NULL, pixmap, NULL, 0, 0, 0, 0, -1, -1)))
	{
		remove(fileName);
		if (!gdk_pixbuf_save(pixbuf, fileName, "png", NULL, NULL))
		{
			fprintf(stderr, "Could not save PDF plot in PNG format.  Most likely cause is missing libpng library.\n");
			fprintf(stderr, "You can obtain this library by downloading the latest release of PQL and re-compiling the PQLX system.\n");
			fprintf(stderr, "PQL can be downloaded from: http://www.passcal.nmt.edu/announcements/pqlII.htm\n");
		}
		g_object_unref(pixbuf);
	}
	else
	{
		fprintf(stderr, "Internal Error: Failed to Create PIXBUF for Output (this shouldn't happen!), Please Report");
	}
	
	switch(fileType)
	{
		case USER:
			free(settings.general.print.fileN);
			settings.general.print.fileN = NULL;
		break;
	}
	g_object_unref(pixmap);
	free(fileName);
}
