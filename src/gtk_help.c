#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "pql_defines.h"
#include "pql_externs.h"
#include "gtk_pql_externs.h"
#include "help.h"
#include "helpText.h"

extern GtkTooltips *tips;
static char	toolTip[1000];

static GtkWidget *helpFrame()
{
  static GtkWidget *view;

	view = gtk_text_view_new();
	gtk_text_view_set_editable(GTK_TEXT_VIEW(view), FALSE);
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(view), FALSE);
	gtk_text_view_set_pixels_below_lines(GTK_TEXT_VIEW(view), 10);
//	gtk_text_view_set_indent(GTK_TEXT_VIEW(view), 10);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(view), GTK_WRAP_WORD);

	return view;
}
	
static GtkWidget *make_HelpButtons(int type)
{
  GtkWidget *widget, *button, *menu_box;
  GtkWidget *actionB;
  int	scr;

  menu_box = gtk_event_box_new();

  widget = gtk_vbox_new(FALSE, 0);
  gtk_container_set_border_width(GTK_CONTAINER(widget), 5);
  gtk_container_add(GTK_CONTAINER(menu_box), widget);

  switch (type)
  {
	case HELPCTRL:
		scr = TRACESCR;

		actionB = gtk_vbutton_box_new();
  		gtk_box_pack_start(GTK_BOX(widget), actionB, FALSE, FALSE, 0);
		gtk_button_box_set_layout(GTK_BUTTON_BOX(actionB), GTK_BUTTONBOX_START);
		gtk_box_set_spacing(GTK_BOX(actionB), 2);

		button = gtk_button_new_with_label("Cancel");
		gtk_container_add(GTK_CONTAINER(actionB), button);
		g_signal_connect(button, "clicked", G_CALLBACK(hideHelp), NULL);
		strcpy(toolTip, "Return to Controls Screen");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), button, toolTip, toolTip);

		break;
  }

	gtk_widget_modify_bg(menu_box, GTK_STATE_NORMAL, &settings.colors.sb[scr]);

	return menu_box;
}

static int	len;
static void formatT(GtkTextBuffer *buffer, int format, int ind, char *str)
{
    GtkTextIter	start, end;
    static GtkTextTag	*tagH1, *tagH2, *tagBN, *tagBB, *tagI1, 
						*tagI2, *tagI3, *tagI4, *tagI5, *tagPB2, *tagPA1;

	if (!len)
	{	// do we need to unref previous?
		tagH1 = gtk_text_buffer_create_tag(buffer, "header1", "font", HELPHFONT1, NULL);
		tagH2 = gtk_text_buffer_create_tag(buffer, "header2", "font", HELPHFONT2, NULL);
		tagBN = gtk_text_buffer_create_tag(buffer, "body1", "font", HELPBFONT, NULL);
		tagBB = gtk_text_buffer_create_tag(buffer, "body2", "font", HELPBBFONT, NULL);
		tagI1 = gtk_text_buffer_create_tag(buffer, "indent1", "left-margin", HELPIND1, NULL);
		tagI2 = gtk_text_buffer_create_tag(buffer, "indent2", "left-margin", HELPIND2, NULL);
		tagI3 = gtk_text_buffer_create_tag(buffer, "indent3", "left-margin", HELPIND3, NULL);
		tagI4 = gtk_text_buffer_create_tag(buffer, "indent4", "left-margin", HELPIND4, NULL);
		tagI5 = gtk_text_buffer_create_tag(buffer, "indent5", "left-margin", HELPIND5, NULL);
//		tagPB1 = gtk_text_buffer_create_tag(buffer, "pixelsB1", "pixels-below-lines", 20, NULL);
		tagPB2 = gtk_text_buffer_create_tag(buffer, "pixelsB2", "pixels-below-lines", 0, NULL);
		tagPA1 = gtk_text_buffer_create_tag(buffer, "pixelsA1", "pixels-above-lines", 20, NULL);
	}

	gtk_text_buffer_get_iter_at_offset(buffer, &start, len);
	len += strlen(str);
	gtk_text_buffer_get_iter_at_offset(buffer, &end, len);
		
	switch(format)
	{
		case H1:		// header, 1
			gtk_text_buffer_apply_tag(buffer, tagH1, &start, &end);
			break;

		case H2:		// header, 2
			gtk_text_buffer_apply_tag(buffer, tagH2, &start, &end);
			gtk_text_buffer_apply_tag(buffer, tagPA1, &start, &end);
			break;

		case BN0:		// body, nomral, 0 lines after
			gtk_text_buffer_apply_tag(buffer, tagBN, &start, &end);
			gtk_text_buffer_apply_tag(buffer, tagPB2, &start, &end);
			break;

		case BB0:		// body, bold, 0 lines after
			gtk_text_buffer_apply_tag(buffer, tagBB, &start, &end);
			gtk_text_buffer_apply_tag(buffer, tagPB2, &start, &end);
			break;

		case BN1:		// body, normal, 1 line after
			gtk_text_buffer_apply_tag(buffer, tagBN, &start, &end);
			break;

		case BB1:		// body, bold, 1 line after
			gtk_text_buffer_apply_tag(buffer, tagBB, &start, &end);
			break;

	}

	switch (ind)
	{
		case 1:
			gtk_text_buffer_apply_tag(buffer, tagI1, &start, &end);
			break;

		case 2:
			gtk_text_buffer_apply_tag(buffer, tagI2, &start, &end);
			break;

		case 3:
			gtk_text_buffer_apply_tag(buffer, tagI3, &start, &end);
			break;

		case 4:
			gtk_text_buffer_apply_tag(buffer, tagI4, &start, &end);
			break;

		case 5:
			gtk_text_buffer_apply_tag(buffer, tagI5, &start, &end);
			break;
	}
}

static GtkWidget *helpText(int helpPage)
{
	GtkTextBuffer *buffer;
	GtkWidget	*view=NULL;
	char		helpstring[10000];

	switch(helpPage)
	{
		case INTROHELP:
		view = helpFrame();
//		gtk_text_view_set_justification(GTK_TEXT_VIEW(view), GTK_JUSTIFY_FILL);	// not yet supported

		strcpy(helpstring, INTROH1);
		strcat(helpstring, INTROB1);

		strcat(helpstring, INTROH7);
		strcat(helpstring, INTROH7A);
		strcat(helpstring, INTROB22);
		strcat(helpstring, INTROB221);
		strcat(helpstring, INTROB23);
		strcat(helpstring, INTROB231);
		strcat(helpstring, INTROB24);
		strcat(helpstring, INTROB241);
		strcat(helpstring, INTROB25);
		strcat(helpstring, INTROB251);
		strcat(helpstring, INTROB26);
		strcat(helpstring, INTROB261);
		strcat(helpstring, INTROB27);
		strcat(helpstring, INTROB271);
		strcat(helpstring, INTROB28);
		strcat(helpstring, INTROB281);
		strcat(helpstring, INTROB29);
		strcat(helpstring, INTROB291);
		strcat(helpstring, INTROB30);
		strcat(helpstring, INTROB301);
		strcat(helpstring, INTROB3011);
		strcat(helpstring, INTROB3012);
		strcat(helpstring, INTROB3013);
		strcat(helpstring, INTROB3014);
		strcat(helpstring, INTROB31);
		strcat(helpstring, INTROB311);
		strcat(helpstring, INTROB32);
		strcat(helpstring, INTROB321);
		strcat(helpstring, INTROB33);
		strcat(helpstring, INTROB331);

		strcat(helpstring, INTROH6);
		strcat(helpstring, INTROB11);
		strcat(helpstring, INTROH6S1);
		strcat(helpstring, INTROB12);
		strcat(helpstring, INTROH6S2);
		strcat(helpstring, INTROB13);
		strcat(helpstring, INTROH6S3);
		strcat(helpstring, INTROB14);
		strcat(helpstring, INTROH6S4);
		strcat(helpstring, INTROB15);

		strcat(helpstring, INTROH4);
		strcat(helpstring, INTROH4A);
		strcat(helpstring, INTROH4S1);
		strcat(helpstring, INTROB16);
		strcat(helpstring, INTROH4S2);
		strcat(helpstring, INTROB17);
		strcat(helpstring, INTROH4S3);
		strcat(helpstring, INTROB18);
		strcat(helpstring, INTROH4S4);
		strcat(helpstring, INTROB19);
		strcat(helpstring, INTROH4S5);
		strcat(helpstring, INTROB20);
		strcat(helpstring, INTROH5);
		strcat(helpstring, INTROB21);

		strcat(helpstring, INTROH2);
		strcat(helpstring, INTROB2);
		strcat(helpstring, INTROB2A);
		strcat(helpstring, INTROB2B);
		strcat(helpstring, INTROB2C);
		strcat(helpstring, INTROB2D);
		strcat(helpstring, INTROBI1);
		strcat(helpstring, INTROB3);
		strcat(helpstring, INTROB3A);
		strcat(helpstring, INTROB4);
		strcat(helpstring, INTROH3);
		strcat(helpstring, INTROB5);
		strcat(helpstring, INTROB6);
		strcat(helpstring, INTROB7);
		strcat(helpstring, INTROB8);
		strcat(helpstring, INTROB9);
		strcat(helpstring, INTROB9A);
		strcat(helpstring, INTROB9B);
		strcat(helpstring, INTROB9C);
		strcat(helpstring, INTROB10);

		buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
		gtk_text_buffer_set_text(buffer, helpstring, -1);

		// format all the text
		len=0;
		formatT(buffer, H1, 0, INTROH1);
		formatT(buffer, BN0, 0, INTROB1);

		formatT(buffer, H2, 0, INTROH7);
		formatT(buffer, BN1, 0, INTROH7A);
		formatT(buffer, BB0, 1, INTROB22);
		formatT(buffer, BN0, 2, INTROB221);
		formatT(buffer, BB0, 1, INTROB23);
		formatT(buffer, BN0, 2, INTROB231);
		formatT(buffer, BB0, 1, INTROB24);
		formatT(buffer, BN0, 2, INTROB241);
		formatT(buffer, BB0, 1, INTROB25);
		formatT(buffer, BN0, 2, INTROB251);
		formatT(buffer, BB0, 1, INTROB26);
		formatT(buffer, BN0, 2, INTROB261);
		formatT(buffer, BB0, 1, INTROB27);
		formatT(buffer, BN0, 2, INTROB271);
		formatT(buffer, BB0, 1, INTROB28);
		formatT(buffer, BN0, 2, INTROB281);
		formatT(buffer, BB0, 1, INTROB29);
		formatT(buffer, BN0, 2, INTROB291);
		formatT(buffer, BB0, 1, INTROB30);
		formatT(buffer, BN0, 2, INTROB301);
		formatT(buffer, BN0, 3, INTROB3011);
		formatT(buffer, BN0, 3, INTROB3012);
		formatT(buffer, BN0, 3, INTROB3013);
		formatT(buffer, BN0, 3, INTROB3014);
		formatT(buffer, BB0, 1, INTROB31);
		formatT(buffer, BN0, 2, INTROB311);
		formatT(buffer, BB0, 1, INTROB32);
		formatT(buffer, BN0, 2, INTROB321);
		formatT(buffer, BB0, 1, INTROB33);
		formatT(buffer, BN0, 2, INTROB331);

		formatT(buffer, H2, 0, INTROH6);
		formatT(buffer, BN1, 0, INTROB11);
		formatT(buffer, BB0, 1, INTROH6S1);
		formatT(buffer, BN0, 2, INTROB12);
		formatT(buffer, BB0, 1, INTROH6S2);
		formatT(buffer, BN0, 2, INTROB13);
		formatT(buffer, BB0, 1, INTROH6S3);
		formatT(buffer, BN0, 2, INTROB14);
		formatT(buffer, BB0, 1, INTROH6S4);
		formatT(buffer, BN0, 2, INTROB15);

		formatT(buffer, H2, 0, INTROH4);
		formatT(buffer, BN1, 0, INTROH4A);
		formatT(buffer, BB0, 1, INTROH4S1);
		formatT(buffer, BN0, 2, INTROB16);
		formatT(buffer, BB0, 1, INTROH4S2);
		formatT(buffer, BN0, 2,INTROB17);
		formatT(buffer, BB0, 1,INTROH4S3);
		formatT(buffer, BN0, 2, INTROB18);
		formatT(buffer, BB0, 1, INTROH4S4);
		formatT(buffer, BN0, 2, INTROB19);
		formatT(buffer, BB0, 1, INTROH4S5);
		formatT(buffer, BN0, 2, INTROB20);

		formatT(buffer, H2, 0, INTROH5);
		formatT(buffer, BN0, 0, INTROB21);

		formatT(buffer, H2, 0, INTROH2);
		formatT(buffer, BN1, 0, INTROB2);
		formatT(buffer, BN0, 1, INTROB2A);
		formatT(buffer, BN0, 1, INTROB2B);
		formatT(buffer, BN1, 1, INTROB2C);
		formatT(buffer, BN1, 0, INTROB2D);
		formatT(buffer, BB1, 1, INTROBI1);
		formatT(buffer, BN1, 0, INTROB3);
		formatT(buffer, BN1, 0, INTROB3A);
		formatT(buffer, BN1, 0, INTROB4);
		formatT(buffer, BB1, 0, INTROH3);
		formatT(buffer, BN1, 0, INTROB5);
		formatT(buffer, BN1, 0, INTROB6);
		formatT(buffer, BN0, 1, INTROB7);
		formatT(buffer, BN1, 1, INTROB8);
		formatT(buffer, BN1, 0, INTROB9);
		formatT(buffer, BN0, 1, INTROB9A);
		formatT(buffer, BN1, 1, INTROB9B);
		formatT(buffer, BN1, 0, INTROB9C);
		formatT(buffer, BN0, 0, INTROB10);

		break;

		case TRACEHELP:
		view = helpFrame();

		strcpy(helpstring, TRACEH1);
		strcat(helpstring, TRACEB1);
		strcat(helpstring, TRACEBI1);
		strcat(helpstring, TRACEBI2);
		strcat(helpstring, TRACEBI3);
		strcat(helpstring, TRACEH2A);
		strcat(helpstring, TRACEH2);
		strcat(helpstring, TRACEH2S1);
		strcat(helpstring, TRACEB2);
		strcat(helpstring, TRACEH2S2);
		strcat(helpstring, TRACEB3);
		strcat(helpstring, TRACEH2S3);
		strcat(helpstring, TRACEB4);
		strcat(helpstring, TRACEH3);
		strcat(helpstring, TRACEH3S1);
		strcat(helpstring, TRACEH3S1A);
		strcat(helpstring, TRACEH3S11);
		strcat(helpstring, TRACEB5);
		strcat(helpstring, TRACEH3S12);
		strcat(helpstring, TRACEB6);
		strcat(helpstring, TRACEH3S3);
		strcat(helpstring, TRACEH3S3A);
		strcat(helpstring, TRACEH3S31);
		strcat(helpstring, TRACEB7);
		strcat(helpstring, TRACEH3S32);
		strcat(helpstring, TRACEB8);
		strcat(helpstring, TRACEH3S2);
		strcat(helpstring, TRACEB33);
		strcat(helpstring, TRACEB34);
		strcat(helpstring, TRACEB35);
		strcat(helpstring, TRACEB36);
		strcat(helpstring, TRACEB37);
		strcat(helpstring, TRACEB38);
		strcat(helpstring, TRACEB39);
		strcat(helpstring, TRACEH4);
		strcat(helpstring, TRACEH4S1);
		strcat(helpstring, TRACEB9);
		strcat(helpstring, TRACEH4S2);
		strcat(helpstring, TRACEB10);
		strcat(helpstring, TRACEH4S3);
		strcat(helpstring, TRACEB11);
		strcat(helpstring, TRACEH4S4);
		strcat(helpstring, TRACEB11A);

		strcat(helpstring, TRACEH5);
		strcat(helpstring, TRACEH5S1);
		strcat(helpstring, TRACEB12);
		strcat(helpstring, TRACEH5S2);
		strcat(helpstring, TRACEB13);
		strcat(helpstring, TRACEH5S3);
		strcat(helpstring, TRACEB14);
		strcat(helpstring, TRACEH5S4);
		strcat(helpstring, TRACEB15);

		strcat(helpstring, TRACEH7);
		strcat(helpstring, TRACEH7S1);
		strcat(helpstring, TRACEB26);
		strcat(helpstring, TRACEH7S3);
		strcat(helpstring, TRACEB28);
		strcat(helpstring, TRACEH7S4);
		strcat(helpstring, TRACEB29);
		strcat(helpstring, TRACEH7S6);
		strcat(helpstring, TRACEB31);
		strcat(helpstring, TRACEH7S7);
		strcat(helpstring, TRACEB32);

		strcat(helpstring, TRACEH6);
		strcat(helpstring, TRACEH6S1);
		strcat(helpstring, TRACEH6S11);
		strcat(helpstring, TRACEB16);
		strcat(helpstring, TRACEH6S12);
		strcat(helpstring, TRACEB17);
		strcat(helpstring, TRACEH6S2);
		strcat(helpstring, TRACEB17A);
		strcat(helpstring, TRACEH6S21);
		strcat(helpstring, TRACEB18);
		strcat(helpstring, TRACEH6S22);
		strcat(helpstring, TRACEB19);
		strcat(helpstring, TRACEH6S23);
		strcat(helpstring, TRACEB20);
		strcat(helpstring, TRACEH6S24);
		strcat(helpstring, TRACEB21);

		strcat(helpstring, TRACEH6S6);
		strcat(helpstring, TRACEH6S61);
		strcat(helpstring, TRACEB21A);

		strcat(helpstring, TRACEH6S3);
		strcat(helpstring, TRACEB22);
		strcat(helpstring, TRACEB221);
		strcat(helpstring, TRACEH6S4);
		strcat(helpstring, TRACEB23);
		strcat(helpstring, TRACEH6S5);
		strcat(helpstring, TRACEH6S51);
		strcat(helpstring, TRACEB24);
		strcat(helpstring, TRACEH6S52);
		strcat(helpstring, TRACEB25);

		buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
		gtk_text_buffer_set_text(buffer, helpstring, -1);
		len=0;
		formatT(buffer, H1, 0, TRACEH1);
		formatT(buffer, BN1, 0, TRACEB1);
		formatT(buffer, BN0, 1, TRACEBI1);
		formatT(buffer, BN0, 1, TRACEBI2);
		formatT(buffer, BN0, 1, TRACEBI3);

		formatT(buffer, H2, 0, TRACEH2A);
		formatT(buffer, BB0, 1, TRACEH2);
		formatT(buffer, BB0, 2, TRACEH2S1);
		formatT(buffer, BN0, 0, TRACEB2);
		formatT(buffer, BB0, 2, TRACEH2S2);
		formatT(buffer, BN0, 0, TRACEB3);
		formatT(buffer, BB1, 2, TRACEH2S3);
		formatT(buffer, BN1, 0, TRACEB4);

		formatT(buffer, BB0, 1, TRACEH3);

		formatT(buffer, BB0, 2, TRACEH3S1);		// Window Scale
		formatT(buffer, BN0, 3, TRACEH3S1A);		
		formatT(buffer, BB0, 3, TRACEH3S11);
		formatT(buffer, BN0, 4, TRACEB5);
		formatT(buffer, BB0, 3, TRACEH3S12);
		formatT(buffer, BN1, 4, TRACEB6);

		formatT(buffer, BB0, 2, TRACEH3S3);		// Time Axis
		formatT(buffer, BN0, 3, TRACEH3S3A);
		formatT(buffer, BB0, 3, TRACEH3S31);
		formatT(buffer, BN0, 4, TRACEB7);
		formatT(buffer, BB0, 3, TRACEH3S32);
		formatT(buffer, BN1, 4, TRACEB8);

		formatT(buffer, BB0, 2, TRACEH3S2);		// Display as
		formatT(buffer, BN0, 3, TRACEB33);
		formatT(buffer, BB0, 3, TRACEB34);
		formatT(buffer, BN0, 3, TRACEB35);
		formatT(buffer, BB0, 3, TRACEB36);
		formatT(buffer, BN0, 3, TRACEB37);
		formatT(buffer, BB1, 3, TRACEB38);
		formatT(buffer, BN1, 3, TRACEB39);

		formatT(buffer, BB0, 1, TRACEH4);		// TRACE Buttons
		formatT(buffer, BB0, 2, TRACEH4S1);
		formatT(buffer, BN0, 3, TRACEB9);
		formatT(buffer, BB0, 2, TRACEH4S2);
		formatT(buffer, BN0, 3, TRACEB10);
		formatT(buffer, BB0, 2, TRACEH4S3);
		formatT(buffer, BN0, 3, TRACEB11);
		formatT(buffer, BB0, 2, TRACEH4S4);
		formatT(buffer, BN1, 3, TRACEB11A);

		formatT(buffer, BB0, 1, TRACEH5);		// GENERAL Buttons
		formatT(buffer, BB0, 2, TRACEH5S1);
		formatT(buffer, BN0, 3, TRACEB12);
		formatT(buffer, BB0, 2, TRACEH5S2);
		formatT(buffer, BN0, 3, TRACEB13);
		formatT(buffer, BB0, 2, TRACEH5S3);
		formatT(buffer, BN0, 3, TRACEB14);
		formatT(buffer, BB0, 2, TRACEH5S4);
		formatT(buffer, BN0, 3, TRACEB15);

		formatT(buffer, H2, 0, TRACEH7);		// MOUSE
		formatT(buffer, BB0, 1, TRACEH7S1);		
		formatT(buffer, BN0, 2, TRACEB26);
		formatT(buffer, BB0, 1, TRACEH7S3);		
		formatT(buffer, BN0, 2, TRACEB28);
		formatT(buffer, BB0, 1, TRACEH7S4);		
		formatT(buffer, BN0, 2, TRACEB29);
		formatT(buffer, BB0, 1, TRACEH7S6);		
		formatT(buffer, BN0, 2, TRACEB31);
		formatT(buffer, BB0, 1, TRACEH7S7);		
		formatT(buffer, BN0, 2, TRACEB32);
			
		formatT(buffer, H2, 0, TRACEH6);		// Controls
		formatT(buffer, BB0, 1, TRACEH6S1);		
		formatT(buffer, BB0, 2, TRACEH6S11);
		formatT(buffer, BN0, 3, TRACEB16);
		formatT(buffer, BB0, 2, TRACEH6S12);
		formatT(buffer, BN1, 3, TRACEB17);
		formatT(buffer, BB0, 1, TRACEH6S2);		
		formatT(buffer, BN0, 2, TRACEB17A);
		formatT(buffer, BB0, 2, TRACEH6S21);
		formatT(buffer, BN0, 3, TRACEB18);
		formatT(buffer, BB0, 2, TRACEH6S22);
		formatT(buffer, BN0, 3, TRACEB19);
		formatT(buffer, BB0, 2, TRACEH6S23);
		formatT(buffer, BN0, 3, TRACEB20);
		formatT(buffer, BB0, 2, TRACEH6S24);
		formatT(buffer, BN1, 3, TRACEB21);

		formatT(buffer, BB0, 1, TRACEH6S6);		
		formatT(buffer, BB0, 2, TRACEH6S61);
		formatT(buffer, BN1, 3, TRACEB21A);

		formatT(buffer, BB0, 1, TRACEH6S3);		
		formatT(buffer, BN0, 2, TRACEB22);
		formatT(buffer, BN1, 2, TRACEB221);
		formatT(buffer, BB0, 1, TRACEH6S4);		
		formatT(buffer, BN1, 2, TRACEB23);
		formatT(buffer, BB0, 1, TRACEH6S5);		
		formatT(buffer, BB0, 2, TRACEH6S51);
		formatT(buffer, BN0, 3, TRACEB24);
		formatT(buffer, BB0, 2, TRACEH6S52);
		formatT(buffer, BN0, 3, TRACEB25);
		break;

		case MAGHELP:
		view = helpFrame();

		strcpy(helpstring, MAGH1);
		strcat(helpstring, MAGB1);
		strcat(helpstring, MAGB21);
		strcat(helpstring, MAGB22);
		strcat(helpstring, MAGH2);
		strcat(helpstring, MAGH3);
		strcat(helpstring, MAGB3);
		strcat(helpstring, MAGB3A);
		strcat(helpstring, MAGB30);
		strcat(helpstring, MAGB301);
		strcat(helpstring, MAGB30A);
		strcat(helpstring, MAGB30A1);
		strcat(helpstring, MAGB30B);
		strcat(helpstring, MAGB30B1);
		strcat(helpstring, MAGB31);
		strcat(helpstring, MAGB311);
		strcat(helpstring, MAGB32);
		strcat(helpstring, MAGB321);
		strcat(helpstring, MAGB4);
		strcat(helpstring, MAGB4A);
		strcat(helpstring, MAGB41);
		strcat(helpstring, MAGB411);
		strcat(helpstring, MAGB42);
		strcat(helpstring, MAGB421);
		strcat(helpstring, MAGB5);
		strcat(helpstring, MAGB5A);
		strcat(helpstring, MAGB6);
		strcat(helpstring, MAGB6A);
		strcat(helpstring, MAGB7);
		strcat(helpstring, MAGB7A);
		strcat(helpstring, MAGH4);
		strcat(helpstring, MAGB8);
		strcat(helpstring, MAGB8A);
		strcat(helpstring, MAGB81);
		strcat(helpstring, MAGB81A);
		strcat(helpstring, MAGB81B);
		strcat(helpstring, MAGB9);
		strcat(helpstring, MAGB9A);
		strcat(helpstring, MAGB91);
		strcat(helpstring, MAGB911);
		strcat(helpstring, MAGB92);
		strcat(helpstring, MAGB921);
		strcat(helpstring, MAGB93);
		strcat(helpstring, MAGB931);
		strcat(helpstring, MAGB10);
		strcat(helpstring, MAGB10A);
		strcat(helpstring, MAGB10B);
		strcat(helpstring, MAGB10B1);
		strcat(helpstring, MAGB10C);
		strcat(helpstring, MAGB10C1);
		strcat(helpstring, MAGB11);
		strcat(helpstring, MAGB11A);
		strcat(helpstring, MAGH5);
		strcat(helpstring, MAGB12);
		strcat(helpstring, MAGB121);
		strcat(helpstring, MAGB1211);
		strcat(helpstring, MAGB1212);
		strcat(helpstring, MAGB122);
		strcat(helpstring, MAGB1221);
		strcat(helpstring, MAGB123);
		strcat(helpstring, MAGB1231);
		strcat(helpstring, MAGB12311);
		strcat(helpstring, MAGB1232);
		strcat(helpstring, MAGB12321);
		strcat(helpstring, MAGB124);
		strcat(helpstring, MAGB1241);
		strcat(helpstring, MAGB125);
		strcat(helpstring, MAGB1251);
		strcat(helpstring, MAGB126);
		strcat(helpstring, MAGB1261);
		strcat(helpstring, MAGB127);
		strcat(helpstring, MAGB1271);
		strcat(helpstring, MAGB128);
		strcat(helpstring, MAGB1281);
		strcat(helpstring, MAGB1281A);
		strcat(helpstring, MAGB1282);
		strcat(helpstring, MAGB1281B);
		strcat(helpstring, MAGB12821);
		strcat(helpstring, MAGB1283);
		strcat(helpstring, MAGB1284);
		strcat(helpstring, MAGB1285);
		strcat(helpstring, MAGB1286);
		strcat(helpstring, MAGB13);
		strcat(helpstring, MAGB13A);
		strcat(helpstring, MAGH6);
		strcat(helpstring, MAGB14);
		strcat(helpstring, MAGB141);
		strcat(helpstring, MAGB1411);
		strcat(helpstring, MAGB142);
		strcat(helpstring, MAGB1421);
		strcat(helpstring, MAGB15);
		strcat(helpstring, MAGB151);
		strcat(helpstring, MAGB1511);
		strcat(helpstring, MAGB152);
		strcat(helpstring, MAGB1521);
		strcat(helpstring, MAGB15211);
		strcat(helpstring, MAGB1522);
		strcat(helpstring, MAGB15221);
		strcat(helpstring, MAGB153);
		strcat(helpstring, MAGB1531);
		strcat(helpstring, MAGB16);
		strcat(helpstring, MAGB161);
		strcat(helpstring, MAGB17);
		strcat(helpstring, MAGB17A);
		strcat(helpstring, MAGB18);
		strcat(helpstring, MAGB181);
		strcat(helpstring, MAGB19);
		strcat(helpstring, MAGB191);
		strcat(helpstring, MAGB1911);

		buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
		gtk_text_buffer_set_text(buffer, helpstring, -1);
		len=0;
		formatT(buffer, H1, 0, MAGH1);
		formatT(buffer, BN1, 0, MAGB1);
		formatT(buffer, BN0, 1, MAGB21);
		formatT(buffer, BN0, 1, MAGB22);
		formatT(buffer, H2, 0, MAGH2);
		formatT(buffer, BB0, 1, MAGH3);
		formatT(buffer, BB0, 2, MAGB3);
		formatT(buffer, BN0, 3, MAGB3A);
		formatT(buffer, BB0, 3, MAGB30);
		formatT(buffer, BN0, 4, MAGB301);
		formatT(buffer, BB0, 4, MAGB30A);
		formatT(buffer, BN0, 5, MAGB30A1);
		formatT(buffer, BB0, 4, MAGB30B);
		formatT(buffer, BN0, 5, MAGB30B1);
		formatT(buffer, BB0, 3, MAGB31);
		formatT(buffer, BN0, 4, MAGB311);
		formatT(buffer, BB0, 3, MAGB32);
		formatT(buffer, BN1, 4, MAGB321);
		formatT(buffer, BB0, 2, MAGB4);
		formatT(buffer, BN0, 3, MAGB4A);
		formatT(buffer, BB0, 3, MAGB41);
		formatT(buffer, BN0, 4, MAGB411);
		formatT(buffer, BB0, 3, MAGB42);
		formatT(buffer, BN1, 4, MAGB421);
		formatT(buffer, BB0, 2, MAGB5);
		formatT(buffer, BN1, 3, MAGB5A);
		formatT(buffer, BB0, 2, MAGB6);
		formatT(buffer, BN1, 3, MAGB6A);
		formatT(buffer, BB0, 2, MAGB7);
		formatT(buffer, BN1, 3, MAGB7A);
		formatT(buffer, BB0, 1, MAGH4);
		formatT(buffer, BB0, 2, MAGB8);
		formatT(buffer, BN0, 3, MAGB8A);
		formatT(buffer, BB0, 2, MAGB81);
		formatT(buffer, BN0, 3, MAGB81A);
		formatT(buffer, BN0, 3, MAGB81B);
		formatT(buffer, BB0, 2, MAGB9);
		formatT(buffer, BN0, 3, MAGB9A);
		formatT(buffer, BB0, 4, MAGB91);
		formatT(buffer, BN0, 0, MAGB911);
		formatT(buffer, BB0, 4, MAGB92);
		formatT(buffer, BN0, 0, MAGB921);
		formatT(buffer, BB1, 4, MAGB93);
		formatT(buffer, BN1, 0, MAGB931);
		formatT(buffer, BB0, 2, MAGB10);
		formatT(buffer, BN0, 3, MAGB10A);
		formatT(buffer, BB0, 4, MAGB10B);
		formatT(buffer, BN0, 0, MAGB10B1);
		formatT(buffer, BB0, 4, MAGB10C);
		formatT(buffer, BN0, 0, MAGB10C1);
		formatT(buffer, BB0, 2, MAGB11);
		formatT(buffer, BN0, 3, MAGB11A);
		formatT(buffer, H2, 0, MAGH5);
		formatT(buffer, BB0, 1, MAGB12);
		formatT(buffer, BB0, 2, MAGB121);
		formatT(buffer, BN0, 3, MAGB1211);
		formatT(buffer, BN0, 3, MAGB1212);
		formatT(buffer, BB0, 2, MAGB122);
		formatT(buffer, BN0, 3, MAGB1221);
		formatT(buffer, BB0, 2, MAGB123);
		formatT(buffer, BB0, 3, MAGB1231);
		formatT(buffer, BN0, 3, MAGB12311);
		formatT(buffer, BB0, 3, MAGB1232);
		formatT(buffer, BN0, 0, MAGB12321);
		formatT(buffer, BB0, 2, MAGB124);
		formatT(buffer, BN0, 3, MAGB1241);
		formatT(buffer, BB0, 2, MAGB125);
		formatT(buffer, BN0, 3, MAGB1251);
		formatT(buffer, BB0, 2, MAGB126);
		formatT(buffer, BN0, 3, MAGB1261);
		formatT(buffer, BB0, 2, MAGB127);
		formatT(buffer, BN1, 3, MAGB1271);
		formatT(buffer, BB0, 1, MAGB128);
		formatT(buffer, BN1, 2, MAGB1281);
		formatT(buffer, BB0, 3, MAGB1281A);
		formatT(buffer, BN0, 4, MAGB1282);
		formatT(buffer, BB0, 3, MAGB1281B);
		formatT(buffer, BN1, 4, MAGB12821);
		formatT(buffer, BN1, 2, MAGB1283);
		formatT(buffer, BN1, 2, MAGB1284);
		formatT(buffer, BN1, 2, MAGB1285);
		formatT(buffer, BN1, 2, MAGB1286);
		formatT(buffer, BB0, 1, MAGB13);
		formatT(buffer, BN0, 2, MAGB13A);
		formatT(buffer, H2, 0, MAGH6);
		formatT(buffer, BB0, 1, MAGB14);
		formatT(buffer, BB0, 2, MAGB141);
		formatT(buffer, BN0, 3, MAGB1411);
		formatT(buffer, BB0, 2, MAGB142);
		formatT(buffer, BN1, 3, MAGB1421);
		formatT(buffer, BB0, 1, MAGB15);
		formatT(buffer, BB0, 2, MAGB151);
		formatT(buffer, BN0, 3, MAGB1511);
		formatT(buffer, BB0, 2, MAGB152);
		formatT(buffer, BB0, 3, MAGB1521);
		formatT(buffer, BN0, 4, MAGB15211);
		formatT(buffer, BB0, 3, MAGB1522);
		formatT(buffer, BN0, 4, MAGB15221);
		formatT(buffer, BB0, 3, MAGB153);
		formatT(buffer, BN1, 4, MAGB1531);
		formatT(buffer, BB0, 1, MAGB16);
		formatT(buffer, BN1, 2, MAGB161);
		formatT(buffer, BB0, 1, MAGB17);
		formatT(buffer, BN1, 2, MAGB17A);
		formatT(buffer, BB0, 1, MAGB18);
		formatT(buffer, BN1, 2, MAGB181);
		formatT(buffer, BB0, 1, MAGB19);
		formatT(buffer, BB0, 2, MAGB191);
		formatT(buffer, BN0, 3, MAGB1911);
		break;

		case SPECHELP:
		view = helpFrame();

		strcpy(helpstring, SPECH1);
		strcat(helpstring, SPECB1);
		strcat(helpstring, SPECB1A);
		strcat(helpstring, SPECB1B);
		strcat(helpstring, SPECB1C);
		strcat(helpstring, SPECH2);
		strcat(helpstring, SPECH2A);
		strcat(helpstring, SPECB2);
		strcat(helpstring, SPECB2A);
		strcat(helpstring, SPECB21);
		strcat(helpstring, SPECB211);
		strcat(helpstring, SPECB22);
		strcat(helpstring, SPECB221);
		strcat(helpstring, SPECB3);
		strcat(helpstring, SPECB3A);
		strcat(helpstring, SPECB4);
		strcat(helpstring, SPECB4A);
		strcat(helpstring, SPECB5);
		strcat(helpstring, SPECB5A);
		strcat(helpstring, SPECH3);
		strcat(helpstring, SPECB6);
		strcat(helpstring, SPECB6A);
		strcat(helpstring, SPECB61);
		strcat(helpstring, SPECB62);
		strcat(helpstring, SPECB63);
		strcat(helpstring, SPECB7);
		strcat(helpstring, SPECB7A);
		strcat(helpstring, SPECB8);
		strcat(helpstring, SPECB8A);
		strcat(helpstring, SPECB9);
		strcat(helpstring, SPECB9A);
		strcat(helpstring, SPECB10);
		strcat(helpstring, SPECB10A);
		strcat(helpstring, SPECH4);
		strcat(helpstring, SPECB11);
		strcat(helpstring, SPECB11A);
		strcat(helpstring, SPECB111);
		strcat(helpstring, SPECB1111);
		strcat(helpstring, SPECB112);
		strcat(helpstring, SPECB1121);
		strcat(helpstring, SPECB113);
		strcat(helpstring, SPECB1131);
		strcat(helpstring, SPECB12);
		strcat(helpstring, SPECB12A);
		strcat(helpstring, SPECB121);
		strcat(helpstring, SPECB1211);
		strcat(helpstring, SPECB122);
		strcat(helpstring, SPECB1221);
		strcat(helpstring, SPECB123);
		strcat(helpstring, SPECB1231);
		strcat(helpstring, SPECB13);
		strcat(helpstring, SPECB13A);
		strcat(helpstring, SPECB14);
		strcat(helpstring, SPECB141);
		strcat(helpstring, SPECB141A);

		buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
		gtk_text_buffer_set_text(buffer, helpstring, -1);
		len=0;
		formatT(buffer, H1, 0, SPECH1);
		formatT(buffer, BN1, 0, SPECB1);
		formatT(buffer, BN0, 1, SPECB1A);
		formatT(buffer, BN0, 1, SPECB1B);
		formatT(buffer, BN0, 1, SPECB1C);
		formatT(buffer, H2, 0, SPECH2);
		formatT(buffer, BB0, 1, SPECH2A);
		formatT(buffer, BB0, 2, SPECB2);
		formatT(buffer, BN0, 3, SPECB2A);
		formatT(buffer, BB0, 3, SPECB21);
		formatT(buffer, BN0, 4, SPECB211);
		formatT(buffer, BB0, 3, SPECB22);
		formatT(buffer, BN1, 4, SPECB221);
		formatT(buffer, BB0, 2, SPECB3);
		formatT(buffer, BN1, 3, SPECB3A);
		formatT(buffer, BB0, 2, SPECB4);
		formatT(buffer, BN1, 3, SPECB4A);
		formatT(buffer, BB0, 2, SPECB5);
		formatT(buffer, BN0, 3, SPECB5A);
		formatT(buffer, H2, 0, SPECH3);
		formatT(buffer, BB0, 1, SPECB6);
		formatT(buffer, BN1, 2, SPECB6A);
		formatT(buffer, BN1, 3, SPECB61);
		formatT(buffer, BN1, 3, SPECB62);
		formatT(buffer, BN1, 3, SPECB63);
		formatT(buffer, BB0, 1, SPECB7);
		formatT(buffer, BN1, 2, SPECB7A);
		formatT(buffer, BB0, 1, SPECB8);
		formatT(buffer, BN0, 2, SPECB8A);
		formatT(buffer, BB0, 1, SPECB9);
		formatT(buffer, BN1, 2, SPECB9A);
		formatT(buffer, BB0, 1, SPECB10);
		formatT(buffer, BN0, 2, SPECB10A);
		formatT(buffer, H2, 0, SPECH4);
		formatT(buffer, BB0, 1, SPECB11);
		formatT(buffer, BN0, 2, SPECB11A);
		formatT(buffer, BB0, 2, SPECB111);
		formatT(buffer, BN0, 3, SPECB1111);
		formatT(buffer, BB0, 2, SPECB112);
		formatT(buffer, BN0, 3, SPECB1121);
		formatT(buffer, BB0, 2, SPECB113);
		formatT(buffer, BN1, 3, SPECB1131);
		formatT(buffer, BB0, 1, SPECB12);
		formatT(buffer, BN0, 2, SPECB12A);
		formatT(buffer, BB0, 2, SPECB121);
		formatT(buffer, BN0, 3, SPECB1211);
		formatT(buffer, BB0, 2, SPECB122);
		formatT(buffer, BN0, 3, SPECB1221);
		formatT(buffer, BB0, 2, SPECB123);
		formatT(buffer, BN1, 3, SPECB1231);
		formatT(buffer, BB0, 1, SPECB13);
		formatT(buffer, BN1, 2, SPECB13A);
		formatT(buffer, BB0, 1, SPECB14);
		formatT(buffer, BB0, 2, SPECB141);
		formatT(buffer, BN0, 3, SPECB141A);
		break;

		case SPLITHELP:
		view = helpFrame();

		strcpy(helpstring, SPLH1);
		strcat(helpstring, SPLB1);
		strcat(helpstring, SPLB1A);
		strcat(helpstring, SPLH2);
		strcat(helpstring, SPLH2A);
		strcat(helpstring, SPLB2);
		strcat(helpstring, SPLB2A);
		strcat(helpstring, SPLB3);
		strcat(helpstring, SPLB3A);
		strcat(helpstring, SPLB4);
		strcat(helpstring, SPLB4A);
		strcat(helpstring, SPLH3);
		strcat(helpstring, SPLB5);
		strcat(helpstring, SPLB5A);
		strcat(helpstring, SPLB6);
		strcat(helpstring, SPLB6A);

		buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
		gtk_text_buffer_set_text(buffer, helpstring, -1);
		len=0;
		formatT(buffer, H1, 0, SPLH1);
		formatT(buffer, BN1, 0, SPLB1);
		formatT(buffer, BN0, 0, SPLB1A);
		formatT(buffer, H2, 0, SPLH2);
		formatT(buffer, BB0, 1, SPLH2A);
		formatT(buffer, BB0, 2, SPLB2);
		formatT(buffer, BN1, 3, SPLB2A);
		formatT(buffer, BB0, 2, SPLB3);
		formatT(buffer, BN1, 3, SPLB3A);
		formatT(buffer, BB0, 2, SPLB4);
		formatT(buffer, BN1, 3, SPLB4A);
		formatT(buffer, BB0, 1, SPLH3);
		formatT(buffer, BB0, 2, SPLB5);
		formatT(buffer, BN1, 3, SPLB5A);
		formatT(buffer, BB0, 2, SPLB6);
		formatT(buffer, BN0, 3, SPLB6A);
		break;

		case HEADHELP:
		view = helpFrame();

		strcpy(helpstring, HEADH1);
		strcat(helpstring, HEADB1);
		strcat(helpstring, HEADB11);
		strcat(helpstring, HEADH2);
		strcat(helpstring, HEADH2A);
		strcat(helpstring, HEADB2);
		strcat(helpstring, HEADB2A);
		strcat(helpstring, HEADH3);
		strcat(helpstring, HEADB3);
		strcat(helpstring, HEADB3A);
		strcat(helpstring, HEADH4);
		strcat(helpstring, HEADB4);
		strcat(helpstring, HEADB4A);
		strcat(helpstring, HEADB5);
		strcat(helpstring, HEADB51);
		strcat(helpstring, HEADB51A);

		buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
		gtk_text_buffer_set_text(buffer, helpstring, -1);
		len=0;
		formatT(buffer, H1, 0, HEADH1);
		formatT(buffer, BN1, 0, HEADB1);
		formatT(buffer, BN0, 0, HEADB11);
		formatT(buffer, H2, 0, HEADH2);
		formatT(buffer, BB0, 1, HEADH2A);
		formatT(buffer, BB0, 2, HEADB2);
		formatT(buffer, BN0, 3, HEADB2A);
		formatT(buffer, H2, 0, HEADH3);
		formatT(buffer, BB0, 1, HEADB3);
		formatT(buffer, BN0, 2, HEADB3A);
		formatT(buffer, H2, 0, HEADH4);
		formatT(buffer, BB0, 1, HEADB4);
		formatT(buffer, BN1, 2, HEADB4A);
		formatT(buffer, BB0, 1, HEADB5);
		formatT(buffer, BB0, 2, HEADB51);
		formatT(buffer, BN0, 3, HEADB51A);
		break;

		case CTRLSHELP:
		view = helpFrame();

		strcpy(helpstring, CTRLH1);
		strcat(helpstring, CTRLB1);
		strcat(helpstring, CTRLH2);
		strcat(helpstring, CTRLH2A);
		strcat(helpstring, CTRLB2);
		strcat(helpstring, CTRLB21);
		strcat(helpstring, CTRLB3);
		strcat(helpstring, CTRLB31);
		strcat(helpstring, CTRLB4);
		strcat(helpstring, CTRLB41);
		strcat(helpstring, CTRLB5);
		strcat(helpstring, CTRLB51);
		strcat(helpstring, CTRLH3);
		strcat(helpstring, CTRLB6A);
		strcat(helpstring, CTRLB6B);
		strcat(helpstring, CTRLB61);
		strcat(helpstring, CTRLB611);
		strcat(helpstring, CTRLB6111);
		strcat(helpstring, CTRLB61111);
		strcat(helpstring, CTRLB6112);
		strcat(helpstring, CTRLB61121);
		strcat(helpstring, CTRLB621);
		strcat(helpstring, CTRLB6211);
		strcat(helpstring, CTRLB62111);
		strcat(helpstring, CTRLB6212);
		strcat(helpstring, CTRLB62121);
		strcat(helpstring, CTRLH4);
		strcat(helpstring, CTRLB7A);
		strcat(helpstring, CTRLB7B);
		strcat(helpstring, CTRLB7C);
		strcat(helpstring, CTRLB7D);
		strcat(helpstring, CTRLB7E);
		strcat(helpstring, CTRLB7F);
		strcat(helpstring, CTRLB71);
		strcat(helpstring, CTRLB711);
		strcat(helpstring, CTRLB7111);
		strcat(helpstring, CTRLB712);
		strcat(helpstring, CTRLB7121);
		strcat(helpstring, CTRLB713);
		strcat(helpstring, CTRLB7131);
		strcat(helpstring, CTRLB714);
		strcat(helpstring, CTRLB7141);
		strcat(helpstring, CTRLH5);
		strcat(helpstring, CTRLB8A);
		strcat(helpstring, CTRLB8AA);
		strcat(helpstring, CTRLB8B);
		strcat(helpstring, CTRLB8C);
		strcat(helpstring, CTRLB8);
		strcat(helpstring, CTRLB81);
		strcat(helpstring, CTRLB811);
		strcat(helpstring, CTRLB91);
		strcat(helpstring, CTRLB911);
		strcat(helpstring, CTRLH6);
		strcat(helpstring, CTRLB9A);

		buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
		gtk_text_buffer_set_text(buffer, helpstring, -1);
		len=0;

		formatT(buffer, H1, 0, CTRLH1);
		formatT(buffer, BN0, 0, CTRLB1);
		formatT(buffer, H2, 0, CTRLH2);
		formatT(buffer, BB0, 1, CTRLH2A);
		formatT(buffer, BB0, 2, CTRLB2);
		formatT(buffer, BN0, 3, CTRLB21);
		formatT(buffer, BB0, 2, CTRLB3);
		formatT(buffer, BN0, 3, CTRLB31);
		formatT(buffer, BB0, 2, CTRLB4);
		formatT(buffer, BN0, 3, CTRLB41);
		formatT(buffer, BB0, 2, CTRLB5);
		formatT(buffer, BN0, 3, CTRLB51);
		formatT(buffer, H2, 0, CTRLH3);
		formatT(buffer, BN1, 0, CTRLB6A);
		formatT(buffer, BN1, 0, CTRLB6B);
		formatT(buffer, BB0, 1, CTRLB61);
		formatT(buffer, BB0, 2, CTRLB611);
		formatT(buffer, BB0, 3, CTRLB6111);
		formatT(buffer, BN0, 4, CTRLB61111);
		formatT(buffer, BB0, 3, CTRLB6112);
		formatT(buffer, BN0, 4, CTRLB61121);
		formatT(buffer, BB0, 2, CTRLB621);
		formatT(buffer, BB0, 3, CTRLB6211);
		formatT(buffer, BN0, 4, CTRLB62111);
		formatT(buffer, BB0, 3, CTRLB6212);
		formatT(buffer, BN0, 4, CTRLB62121);
		formatT(buffer, H2, 0, CTRLH4);
		formatT(buffer, BN1, 0, CTRLB7A);
		formatT(buffer, BN1, 1, CTRLB7B);
		formatT(buffer, BN1, 1, CTRLB7C);
		formatT(buffer, BN1, 1, CTRLB7D);
		formatT(buffer, BN1, 1, CTRLB7E);
		formatT(buffer, BN1, 0, CTRLB7F);
		formatT(buffer, BB0, 1, CTRLB71);
		formatT(buffer, BB0, 2, CTRLB711);
		formatT(buffer, BN0, 3, CTRLB7111);
		formatT(buffer, BB0, 2, CTRLB712);
		formatT(buffer, BN0, 3, CTRLB7121);
		formatT(buffer, BB0, 2, CTRLB713);
		formatT(buffer, BN0, 3, CTRLB7131);
		formatT(buffer, BB0, 2, CTRLB714);
		formatT(buffer, BN0, 3, CTRLB7141);
		formatT(buffer, H2, 0, CTRLH5);
		formatT(buffer, BN1, 0, CTRLB8A);
		formatT(buffer, BB0, 1, CTRLB8AA);
		formatT(buffer, BB0, 2, CTRLB8B);
		formatT(buffer, BN1, 3, CTRLB8C);
		formatT(buffer, BB0, 1, CTRLB8);
		formatT(buffer, BB0, 2, CTRLB81);
		formatT(buffer, BN0, 3, CTRLB811);
		formatT(buffer, BB0, 2, CTRLB91);
		formatT(buffer, BN0, 3, CTRLB911);
		formatT(buffer, H2, 0, CTRLH6);
		formatT(buffer, BN0, 0, CTRLB9A);
		break;
	}
	return view;
}

void make_help()
{
  GtkWidget *ctrlBox, *vbox;
  GtkWidget *frame, *view, *scrollW;
	
		helpWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		gtk_window_resize(GTK_WINDOW (helpWindow), HELPW, HELPH); 
//		gtk_widget_set_size_request(helpWindow, HELPW, HELPH); 
		gtk_window_set_title(GTK_WINDOW (helpWindow), "PQL II - HELP Pages"); 
		gtk_container_set_border_width(GTK_CONTAINER(helpWindow), 2);

		ctrlBox = gtk_hbox_new(FALSE, 0);
		gtk_container_add(GTK_CONTAINER(helpWindow), ctrlBox);

		helpButtons = make_HelpButtons(HELPCTRL);
		gtk_box_pack_start(GTK_BOX(ctrlBox), helpButtons, FALSE, FALSE, 0);

		vbox = gtk_vbox_new(FALSE, 0);
		help_screens = gtk_notebook_new();
		gtk_box_pack_start(GTK_BOX(vbox), help_screens, TRUE, TRUE, 0);
		gtk_box_pack_start(GTK_BOX(ctrlBox), vbox, TRUE, TRUE, 0);

		// INTRODUCTION Help
		frame = gtk_frame_new(NULL);
		gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
		view = helpText(INTROHELP);

    		scrollW = gtk_scrolled_window_new(NULL, NULL);
  		gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrollW), GTK_SHADOW_ETCHED_IN);
  		gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollW),
				      GTK_POLICY_AUTOMATIC,
				      GTK_POLICY_AUTOMATIC);
		gtk_container_add(GTK_CONTAINER(scrollW), view);
		gtk_container_add(GTK_CONTAINER(frame), scrollW);
		gtk_notebook_append_page(GTK_NOTEBOOK(help_screens), frame, gtk_label_new(" Intro "));

		// TRACE Help
		frame = gtk_frame_new(NULL);
		gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
		view = helpText(TRACEHELP);

    		scrollW = gtk_scrolled_window_new(NULL, NULL);
  		gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrollW), GTK_SHADOW_ETCHED_IN);
  		gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollW),
				      GTK_POLICY_AUTOMATIC,
				      GTK_POLICY_AUTOMATIC);
		gtk_container_add(GTK_CONTAINER(scrollW), view);
		gtk_container_add(GTK_CONTAINER(frame), scrollW);
		gtk_notebook_append_page(GTK_NOTEBOOK(help_screens), frame, gtk_label_new(" Trace "));

		// MAGNIFY Help
		frame = gtk_frame_new(NULL);
		gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
		view = helpText(MAGHELP);

    		scrollW = gtk_scrolled_window_new(NULL, NULL);
  		gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrollW), GTK_SHADOW_ETCHED_IN);
  		gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollW),
				      GTK_POLICY_AUTOMATIC,
				      GTK_POLICY_AUTOMATIC);
		gtk_container_add(GTK_CONTAINER(scrollW), view);
		gtk_container_add(GTK_CONTAINER(frame), scrollW);
		gtk_notebook_append_page(GTK_NOTEBOOK(help_screens), frame, gtk_label_new(" Magnify "));

		// SPECTRA Help
		frame = gtk_frame_new(NULL);
		gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
		view = helpText(SPECHELP);

	    	scrollW = gtk_scrolled_window_new(NULL, NULL);
  		gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrollW), GTK_SHADOW_ETCHED_IN);
  		gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollW),
				      GTK_POLICY_AUTOMATIC,
				      GTK_POLICY_AUTOMATIC);
		gtk_container_add(GTK_CONTAINER(scrollW), view);
		gtk_container_add(GTK_CONTAINER(frame), scrollW);
		gtk_notebook_append_page(GTK_NOTEBOOK(help_screens), frame, gtk_label_new(" Spectra "));

		// SPLIT Help
		frame = gtk_frame_new(NULL);
		gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
		view = helpText(SPLITHELP);

    		scrollW = gtk_scrolled_window_new(NULL, NULL);
  		gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrollW), GTK_SHADOW_ETCHED_IN);
  		gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollW),
				      GTK_POLICY_AUTOMATIC,
				      GTK_POLICY_AUTOMATIC);
		gtk_container_add(GTK_CONTAINER(scrollW), view);
		gtk_container_add(GTK_CONTAINER(frame), scrollW);
		gtk_notebook_append_page(GTK_NOTEBOOK(help_screens), frame, gtk_label_new(" Split "));

		// HEADERS Help
		frame = gtk_frame_new(NULL);
		gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
		view = helpText(HEADHELP);

	    	scrollW = gtk_scrolled_window_new(NULL, NULL);
  		gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrollW), GTK_SHADOW_ETCHED_IN);
  		gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollW),
				      GTK_POLICY_AUTOMATIC,
				      GTK_POLICY_AUTOMATIC);
		gtk_container_add(GTK_CONTAINER(scrollW), view);
		gtk_container_add(GTK_CONTAINER(frame), scrollW);
		gtk_notebook_append_page(GTK_NOTEBOOK(help_screens), frame, gtk_label_new(" Headers "));

		// CONTROLS Help
		frame = gtk_frame_new(NULL);
		gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
		view = helpText(CTRLSHELP);

    		scrollW = gtk_scrolled_window_new(NULL, NULL);
  		gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrollW), GTK_SHADOW_ETCHED_IN);
  		gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollW),
				      GTK_POLICY_AUTOMATIC,
				      GTK_POLICY_AUTOMATIC);
		gtk_container_add(GTK_CONTAINER(scrollW), view);
		gtk_container_add(GTK_CONTAINER(frame), scrollW);
		gtk_notebook_append_page(GTK_NOTEBOOK(help_screens), frame, gtk_label_new(" Controls "));

//		gtk_window_set_transient_for((GtkWindow *) helpWindow, (GtkWindow *) topWindow);
		gtk_window_set_transient_for(GTK_WINDOW(helpWindow), GTK_WINDOW(topWindow));
		gtk_widget_show_all(helpWindow);

		return;
}
