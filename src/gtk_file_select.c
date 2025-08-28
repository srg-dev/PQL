#include <errno.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <gdk/gdkkeysyms.h>

#include "pql_defines.h"
#include "pql_externs.h"
#include "gtk_pql_externs.h"

extern GtkTooltips *tips;

enum {
	MPLEX_INCLUDE,
	MPLEX_EXCLUDE,
	MPLEX_LENGTH,
	TTLMPLEXOPTS
};
static GtkWidget *fileSel, *replace, *filterEntry, *mplexOpt[TTLMPLEXOPTS];

static void freeMMOPTS(MPLX_MSEED *opts)
{
	GSList *iter;
	for(iter = opts->incChans;
		iter;
		iter = g_slist_next(iter))
	{
		free(iter->data);
	}
	g_slist_free(opts->incChans);
	for(iter = opts->excChans;
		iter;
		iter = g_slist_next(iter))
	{
		free(iter->data);
	}
	g_slist_free(opts->excChans);
	free(opts);
}

int parseMMLen(char *lenStr)
{
	char *timeStr;
	int	length = 0;
	
	for(timeStr = strtok(lenStr, CHNARGSEP);
		timeStr;
		timeStr = strtok(NULL, CHNARGSEP))
	{
		switch(timeStr[strlen(timeStr)-1])
		{
			case 'd':
			case 'D':
				timeStr[strlen(timeStr)-1] = 0;		// cut off the specifier
				g_strstrip(timeStr);
				length += (24*60*60 * atoi(timeStr));
			break;
			case 'h':
			case 'H':
				timeStr[strlen(timeStr)-1] = 0;		// cut off the specifier
				g_strstrip(timeStr);
				length += (60 * 60 * atoi(timeStr));
			break;
			case 'm':
			case 'M':
				timeStr[strlen(timeStr)-1] = 0;		// cut off the specifier
				g_strstrip(timeStr);
				length += (60 * atoi(timeStr));
			break;
			case 's':
			case 'S':
				timeStr[strlen(timeStr)-1] = 0;		// cut off the specifier
				g_strstrip(timeStr);
				length += atoi(timeStr);
			break;
			default:
				// unknown specifier, ignored
			break;
		}
	}
	return length;
}

gint getFilename(GtkButton *button, gpointer t)
{
	int		type = GPOINTER_TO_INT(t);
	static GtkWidget	*fileSel[MAXFTYPES], *entry, *ctrlWdgt;
	char	*msg=NULL, *filename=NULL, **ctrlStr=NULL;
	GSList	 *selected;

	switch(type)
	{
		case PICKFTYPE:
			msg = g_strdup("Please Select the File for PICK Output");
			ctrlWdgt = MpickFN;
			ctrlStr = &settings.magnify.pickFile;
		break;
		case PRINTFTYPE:
			msg = g_strdup("Please Select the File for PRINT Output");
			ctrlWdgt = TprtFnm;
			ctrlStr = &settings.general.print.fileN;
		break;
		case MARKFTYPE:
			msg = g_strdup("Please Select the MARK File BaseName");
			ctrlWdgt = TmarkFN;
			ctrlStr = &settings.general.mark.fileN;
		break;
	}
	if (*ctrlStr)
		free(*ctrlStr);

	if (!fileSel[type])
	{
		GtkWidget	*hbox, *label;
		fileSel[type] = gtk_file_chooser_dialog_new(msg,
											GTK_WINDOW (topWindow),
											GTK_FILE_CHOOSER_ACTION_OPEN,
											GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
											GTK_STOCK_OK, GTK_RESPONSE_CLOSE,
											NULL);
		hbox = gtk_hbox_new(FALSE, 0);
		label = gtk_label_new("FileName:  ");
		entry = gtk_entry_new();
		gtk_entry_set_max_length(GTK_ENTRY(entry), 40);
		gtk_box_pack_end(GTK_BOX(hbox), entry, FALSE, FALSE, 0);
		gtk_box_pack_end(GTK_BOX(hbox), label, FALSE, FALSE, 0);
		gtk_widget_show_all(hbox);
		gtk_file_chooser_set_extra_widget(GTK_FILE_CHOOSER (fileSel[type]), hbox);
	}

	if (settings.general.dirs.last[type] &&
		g_file_test(settings.general.dirs.last[type], G_FILE_TEST_IS_DIR))
		gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(fileSel[type]), settings.general.dirs.last[type]);

	if (gtk_dialog_run(GTK_DIALOG(fileSel[type])) == GTK_RESPONSE_CLOSE)
	{
		selected = gtk_file_chooser_get_filenames (GTK_FILE_CHOOSER (fileSel[type]));
		filename =  g_strdup(gtk_entry_get_text(GTK_ENTRY(entry)));
		if (filename[0])
		{	// filename specified by user, use it
			if (filename[0] != '/' &&
				filename[0] != '\\')
			{	// file provided by user is located in directory defined by dialog
				int pos;
				char *str;
#ifdef WIN32
				pos = (int) ((long) strrchr(selected->data, '\\') - (long) selected->data + 1);
#else
				pos = (int) ((long) strrchr(selected->data, '/') - (long) selected->data + 1);
#endif
				str = g_strndup(selected->data, pos);
				*ctrlStr = g_strdup_printf("%s%s", str, filename);
				free(str);
			}
			else
			{	// file provided by user is absolute pathname, ignore dialog directory
				*ctrlStr = g_strdup(filename);
			}
		}
		else
		{	// file chosen via dialog, use it
			*ctrlStr = g_strdup(selected->data);
		}
		if (button)		// set the text entry only when called from the controls panel
			gtk_entry_set_text(GTK_ENTRY(ctrlWdgt), *ctrlStr);

		g_free(selected->data);
		g_slist_free(selected);
	}

	if (settings.general.dirs.last[type])
		free(settings.general.dirs.last[type]);
	settings.general.dirs.last[type] = g_strdup(*ctrlStr);
	gtk_widget_hide(fileSel[type]);
	free(msg);
	if (filename)
		free(filename);
	return TRUE;
}

enum {
	FTDIR,
	FTFILE
};

static int fileType(char *name)
{	// is the input a file or a directory?
	// can't use extended field d_type from dirent.h, doesn't exist on SOLARIS
	DIR		*dp;
	int 	ret = FTFILE;
	
	if (!((dp=opendir(name))==NULL))
	{	// succeeded, it's a directory
		ret = FTDIR;
		closedir(dp);
	}

	return ret;
}

static void registerFile(char *file)
{
	PQLDATA	*pqlIn;
	
	pqlIn = calloc(1, sizeof(PQLDATA));
	pqlIn->fileName = g_strdup(file);
	states.input.Files = g_slist_prepend(states.input.Files, pqlIn);
}

static void getFiles(char *dirFile)
{	// recursive function: 
	// descend into every directory and 
	//  1) if it's a directory call us again, or
	//	2) if it's a file, register with states.input.Files for processing
	DIR		*dp;
	struct dirent *ep;
	char *newDirFile;

	if ((dp=opendir(dirFile))==NULL)
	{	// failed, it must be a file
		registerFile(dirFile);
	}
	else
	{	// succeeded, it must be a directory
		while((ep=readdir(dp)))
		{
			if (!strcmp(ep->d_name,".") || 
				!strcmp(ep->d_name, ".."))
				continue;	// don't read '.' and '..' directories

			if (!strcmp(ep->d_name, "0"))
				continue;	// don't process '0' directory (for RT130 data)

#ifdef WIN32
			newDirFile = g_strdup_printf("%s\\%s", dirFile, ep->d_name);	
#else
			newDirFile = g_strdup_printf("%s/%s", dirFile, ep->d_name);	
#endif
			switch(fileType(newDirFile))
			{
				case FTDIR:
					getFiles(newDirFile);
				break;
				case FTFILE:
					registerFile(newDirFile);
				break;
			}
			free(newDirFile);
		}
		closedir(dp);
	}
}

#if 0
static gboolean isRTData(char *head)
{	// recursive function: return TRUE when directory name '0' is found, else FALSE
	DIR		*dp;
	struct dirent *ep;
	char	*newDir;
	gboolean rtDATA=FALSE;
	
	if ((dp=opendir(head))==NULL)
	{	// failed, it must be a file, no RT data here
		return FALSE;
	}
	
	while((ep=readdir(dp)))
	{
		if (!strcmp(ep->d_name,".") || !strcmp(ep->d_name, ".."))
			continue;	// don't read . and ..
		
		if (!strcmp(ep->d_name, "0"))
		{
			rtDATA = TRUE;					// success!  we're in RT mode
			break;
		}
			
#ifdef WIN32
		newDir = g_strdup_printf("%s\\%s", head, ep->d_name);
#else
		newDir = g_strdup_printf("%s/%s", head, ep->d_name);
#endif
		rtDATA = isRTData(newDir);
		free(newDir);
		if (rtDATA)
			break;
	}
	closedir(dp);
	return rtDATA;
}
#endif

static void readFiles()
{
	GSList	*selected, *sel;

	sel = selected = gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(fileSel));

	if (!selected)
		return;
		
	// save the directory for next dialogue invocation
	if (settings.general.startUP.lastDir)
		free(settings.general.startUP.lastDir);
	settings.general.startUP.lastDir = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(fileSel));

	if (settings.general.startUP.openReplace)
	{
		kill_all_traces(ALL);
	}
	
	while(selected)
	{
		getFiles(selected->data);
		g_free(selected->data);
		selected=selected->next;
	}
	g_slist_free(sel);
	
	states.input.numFiles = g_slist_length(states.input.Files);
}

static void filterApply(GtkWidget *entry, gpointer nil)
{	// apply the user-specifid filter to file listing, callback from ENTER key on filterEntry widget below
	GtkFileFilter *fileFilter;
	char *filterS;

	fileFilter = gtk_file_filter_new();
	filterS = g_strdup(gtk_entry_get_text(GTK_ENTRY(entry)));
	if (!filterS[0])
		filterS = g_strdup("*");
	gtk_file_filter_add_pattern(fileFilter, filterS);
	gtk_file_chooser_set_filter(GTK_FILE_CHOOSER (fileSel), fileFilter);
	free(filterS);
	
	return;
}

static void setFileDefs(GtkButton *button, gpointer nil)
{
	free(settings.general.input.file.directory);
	free(settings.general.input.file.filter);
	free(settings.general.input.file.mseedInc);
	free(settings.general.input.file.mseedExc);
	free(settings.general.input.file.mseedLen);
	
	settings.general.input.file.filter = g_strdup(gtk_entry_get_text(GTK_ENTRY(filterEntry)));
	settings.general.input.file.mseedInc = g_strdup(gtk_entry_get_text(GTK_ENTRY(mplexOpt[MPLEX_INCLUDE])));
	settings.general.input.file.mseedExc = g_strdup(gtk_entry_get_text(GTK_ENTRY(mplexOpt[MPLEX_EXCLUDE])));
	settings.general.input.file.mseedLen = g_strdup(gtk_entry_get_text(GTK_ENTRY(mplexOpt[MPLEX_LENGTH])));
	settings.general.input.file.replace = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (replace));

	settings.general.input.file.directory = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER (fileSel));
	if (!settings.general.input.file.directory)
		settings.general.input.file.directory = g_strdup("");

	make_defaults_file(&settings, FILEDEFSET);
}

gint openTraces(GtkButton *button, gpointer nothing)
{
	if (!fileSel)
	{
		GtkWidget 		*hbox, *vbox, *label, *frame, *hbox1, *entry, *but;
		GtkSizeGroup	*size_group;
		char	toolTip[1000];
		fileSel = gtk_file_chooser_dialog_new ("Please Select Directories and/or TRACE Files for Viewing",
											GTK_WINDOW (topWindow),
											GTK_FILE_CHOOSER_ACTION_OPEN,
											GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
											GTK_STOCK_OPEN, GTK_RESPONSE_CLOSE,
											NULL);
		gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER (fileSel), TRUE);

		tips = gtk_tooltips_new();
		size_group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);

		frame = gtk_frame_new("PQL TRACE Options");
		vbox = gtk_vbox_new(FALSE, 2);
		
		hbox1 = gtk_hbox_new(FALSE, 0);

		hbox = gtk_hbox_new(FALSE, 2);
		label = gtk_label_new("File Filter: ");
		gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);
		gtk_size_group_add_widget(size_group, label);
		gtk_misc_set_alignment(GTK_MISC (label), 1, .5);
		filterEntry = entry = gtk_entry_new();
		strcpy(toolTip, "Specify Wildcard Filter\n<ENTER> to Apply Filter");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), entry, toolTip, toolTip);
		gtk_box_pack_end(GTK_BOX(hbox), entry, FALSE, FALSE, 2);
		gtk_entry_set_max_length(GTK_ENTRY(entry), 40);
		gtk_entry_set_width_chars(GTK_ENTRY(entry), 15);
		g_signal_connect(filterEntry, "activate", G_CALLBACK(filterApply), NULL);
		gtk_box_pack_start(GTK_BOX(hbox1), hbox, FALSE, FALSE, 0);
		
		hbox = gtk_hbox_new(FALSE, 2);
		label = gtk_label_new("Replace Current Traces");
		replace = gtk_check_button_new ();
		strcpy(toolTip, "Unchecked:\tADD selected files to current input list\n"\
						"  Checked:\tREPLACE current input file list");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), replace, toolTip, toolTip);
		gtk_box_pack_end(GTK_BOX(hbox), label, FALSE, FALSE, 2);
		gtk_box_pack_end(GTK_BOX(hbox), replace, FALSE, FALSE, 2);
		gtk_box_pack_end(GTK_BOX(hbox1), hbox, FALSE, FALSE, 2);

		gtk_box_pack_start(GTK_BOX(vbox), hbox1, FALSE, FALSE, 2);
		
		hbox = gtk_hbox_new(FALSE, 2);
		label = gtk_label_new("Multiplex mini-SEED INCLUDE Channels: ");
		gtk_size_group_add_widget(size_group, label);
		gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);
		gtk_misc_set_alignment(GTK_MISC (label), 1, .5);
		mplexOpt[MPLEX_INCLUDE] = entry = gtk_entry_new();
		strcpy(toolTip, "Specify Channels to INCLUDE for Viewing\n"\
						"Comma Separated List, no spaces, e.g.:\n"\
						"\tBHE,BHN,BHZ");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), entry, toolTip, toolTip);
		gtk_box_pack_start(GTK_BOX(hbox), entry, FALSE, FALSE, 2);
		gtk_entry_set_max_length(GTK_ENTRY(entry), 70);
		gtk_entry_set_width_chars(GTK_ENTRY(entry), 25);
		gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 2);

		hbox = gtk_hbox_new(FALSE, 2);
		label = gtk_label_new("Multiplex mini-SEED EXCLUDE Channels: ");
		gtk_size_group_add_widget(size_group, label);
		gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);
		gtk_misc_set_alignment(GTK_MISC (label), 1, .5);
		mplexOpt[MPLEX_EXCLUDE] = entry = gtk_entry_new();
		strcpy(toolTip, "Specify Channels to EXCLUDE for Viewing\n"\
						"Comma Separated List, e.g.:\n"\
						"\tLHE,LHN,LHZ");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), entry, toolTip, toolTip);
		gtk_box_pack_start(GTK_BOX(hbox), entry, FALSE, FALSE, 2);
		gtk_entry_set_max_length(GTK_ENTRY(entry), 70);
		gtk_entry_set_width_chars(GTK_ENTRY(entry), 25);
		gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 2);

		hbox = gtk_hbox_new(FALSE, 2);
		label = gtk_label_new("Multiplex mini-SEED Trace Length: ");
		gtk_size_group_add_widget(size_group, label);
		gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);
		gtk_misc_set_alignment(GTK_MISC (label), 1, .5);
		mplexOpt[MPLEX_LENGTH] = entry = gtk_entry_new();
		strcpy(toolTip, "Specify Length of Traces for Viewing\n"\
						"Using D,H,M,S Specifiers, in any combination, e.g.:\n"\
						"\t1D,12H,30M,30S\n"\
						"\t= 1 Day, 12 Hours, 30 Minutes and 30 Seconds");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), entry, toolTip, toolTip);
		gtk_box_pack_start(GTK_BOX(hbox), entry, FALSE, FALSE, 2);
		gtk_entry_set_max_length(GTK_ENTRY(entry), 70);
		gtk_entry_set_width_chars(GTK_ENTRY(entry), 25);
		but = gtk_button_new_with_label("Set Defaults");
		g_signal_connect(but, "clicked", G_CALLBACK(setFileDefs), NULL);
		gtk_box_pack_end(GTK_BOX(hbox), but, FALSE, FALSE, 2);
		strcpy(toolTip, "Save File Reading Options");
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), but, toolTip, toolTip);
		
		gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 2);

		gtk_container_add(GTK_CONTAINER(frame), vbox);
		gtk_widget_show_all(frame);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (replace), settings.general.startUP.openReplace);
		gtk_file_chooser_set_extra_widget(GTK_FILE_CHOOSER (fileSel), frame);

		gtk_entry_set_text(GTK_ENTRY(filterEntry), settings.general.input.file.filter);
		gtk_entry_set_text(GTK_ENTRY(mplexOpt[MPLEX_INCLUDE]), settings.general.input.file.mseedInc);
		gtk_entry_set_text(GTK_ENTRY(mplexOpt[MPLEX_EXCLUDE]), settings.general.input.file.mseedExc);
		gtk_entry_set_text(GTK_ENTRY(mplexOpt[MPLEX_LENGTH]), settings.general.input.file.mseedLen);
	}
	else
	{	// already made...
		gtk_widget_show(fileSel);
	}
	filterApply(filterEntry, NULL);		// apply any defined filter

	if (settings.general.startUP.lastDir &&
		settings.general.startUP.lastDir[0])
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(fileSel), settings.general.startUP.lastDir);


	if (gtk_dialog_run(GTK_DIALOG(fileSel)) == GTK_RESPONSE_CLOSE)
	{	// on file chooser OPEN button
		char *incChn, *excChn, *lenChn, *chnStr;
		incChn = g_strdup(gtk_entry_get_text(GTK_ENTRY(mplexOpt[MPLEX_INCLUDE])));
		excChn = g_strdup(gtk_entry_get_text(GTK_ENTRY(mplexOpt[MPLEX_EXCLUDE])));
		lenChn = g_strdup(gtk_entry_get_text(GTK_ENTRY(mplexOpt[MPLEX_LENGTH])));
		settings.general.startUP.openReplace = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(replace));
		if (mseed_opts)
		{
			freeMMOPTS(mseed_opts);
		}
		mseed_opts = calloc(1, sizeof(MPLX_MSEED));
		for(chnStr = strtok(incChn, CHNARGSEP);
			chnStr;
			chnStr = strtok(NULL, CHNARGSEP))
		{
			g_strstrip(chnStr);
			TOUPPER(chnStr);
			mseed_opts->incChans = g_slist_prepend(mseed_opts->incChans, g_strdup(chnStr));
		}
		for(chnStr = strtok(excChn, CHNARGSEP);
			chnStr;
			chnStr = strtok(NULL, CHNARGSEP))
		{
			g_strstrip(chnStr);
			TOUPPER(chnStr);
			mseed_opts->excChans = g_slist_prepend(mseed_opts->excChans, g_strdup(chnStr));
		}
		mseed_opts->length = parseMMLen(lenChn);
		free(incChn);	free(excChn);	free(lenChn);

		readFiles();
		gtk_widget_hide(fileSel);

		states.general.plot.state = READHDRS;
		if (criteria[0].desc[0] != 'I')
		{
			free(criteria[0].desc);
			criteria[0].desc = g_strdup("Input Order");
		}

		readHdr_1();
	}
	else
	{	// on file chooser CANCEL button
		states.general.plot.state = DATADONE;
		g_idle_add((GSourceFunc) _makePixmap, GINT_TO_POINTER(TRACEDA));
		gtk_widget_hide(fileSel);
	}

	return TRUE;
}

gboolean _openTraces(gpointer nil)
{
	openTraces(NULL, NULL);
	return FALSE;
}
