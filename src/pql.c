#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <getopt.h>

#include "pql_defines.h"
#include "pql_externs.h"
#include "gtk_pql_externs.h"

extern char	*progname;

void make_pql();
void init_app();

void printHelp()
{
      fprintf(stderr, "PQL II - Version\t%s\n", PROG_VERSION);
      fprintf(stderr, "Usage:\npql [ -h ] [ TRACES | -s sort_file ]\n"
      					"\t[ -i instrument_sort_file ] [ -t shot_start_time_file ]\n"
						"\t[ --mi multi-plex mini-SEED channels to include, ',' separated, no spaces ]\n"
						"\t[ --me multi-plex mini-SEED channels to exclude, ',' separated, no spaces ]\n"
						"\t[ --ml multi-plex mini-SEED trace length, ',' separated, no spaces\n"\
						"\t\tusing D,H,M,S specifiers, in any combination\n"
						"\t\te.g.: 1D,2H,3M,4S = 1Day, 2Hours, 3Minutes, and 4Seconds ]\n" 
      					"\t[ -l (=low resolution mode) ]\n");
}

static gint sortShotTimes(TIMEENTRY *one, TIMEENTRY *two)
{
	return(strcmp(one->str, two->str));
}

int main(argc, argv)
   int             argc;
   char          **argv;
{
	int i, opt, read_traces = TRUE;
	gboolean low_res = FALSE;
	PQLDATA *pqlIn;
	char *chnStr;

	int this_option_optind = optind ? optind : 1;
	int option_index = 0;
	static struct option long_options[] = {
		{"mi", 1, 0, 0},
		{"me", 1, 0, 0},
		{"ml", 1, 0, 0},
		{0, 0, 0, 0}
	};

	if (argv[0])
		progname = argv[0];
	else
		progname = "";

	read_defaults_file(&settings, DEFAULTSET);
	read_defaults_file(&settings, FILEDEFSET);

	while(1) 
	{
		this_option_optind = optind ? optind : 1;
		option_index = 0;
		opt = getopt_long(argc, argv, "i:lhs:t:", long_options, &option_index);
		if (opt == -1)
			break;
		
      switch(opt)
      {
		  case 0:
			  if (!mseed_opts)
				  mseed_opts = calloc(1, sizeof(MPLX_MSEED));
			  switch(option_index)
			  {
				  case 0:
					for(chnStr = strtok(optarg, CHNARGSEP);
						chnStr;
						chnStr = strtok(NULL, CHNARGSEP))
					{
						g_strstrip(chnStr);
						TOUPPER(chnStr);
						mseed_opts->incChans = g_slist_prepend(mseed_opts->incChans, g_strdup(chnStr));
					}
				  break;
				  case 1:
					for(chnStr = strtok(optarg, CHNARGSEP);
						chnStr;
						chnStr = strtok(NULL, CHNARGSEP))
					{
						g_strstrip(chnStr);
						TOUPPER(chnStr);
						mseed_opts->excChans = g_slist_prepend(mseed_opts->excChans, g_strdup(chnStr));
					}
				  break;
				  case 2:
					  mseed_opts->length = parseMMLen(optarg);
				  break;
			  }
			  break;

		  case 'h':
            printHelp();
            exit(0);
         break;
         case 'l':
            low_res = TRUE;
         break;
         case 's':
		{   
			char fileStr[255];
			FILE	*fp;
			char	*sortFile;
			
            read_traces = FALSE;
            sortFile = g_strdup(optarg);
            if ((fp = fopen(sortFile, "r")) == NULL)
            {
               fprintf(stderr, "Unable to open sort file %s, exiting.\n", sortFile);
               exit(-1);
            }
            i=0;
            while (fgets(fileStr, 255, fp))
            {
               fileStr[strlen(fileStr)-1] = 0;
               pqlIn = calloc(1, sizeof(PQLDATA));
               pqlIn->fileName = g_strdup(fileStr);
               states.input.Files = g_slist_prepend(states.input.Files, pqlIn);
               i++;
            }
			states.input.Files = g_slist_reverse(states.input.Files);
            fclose(fp);
            free(sortFile);
			free(criteria[0].desc);
			criteria[0].desc = g_strdup("Sort File");
            states.input.numFiles = g_slist_length(states.input.Files);
         }
         break;
         
        case 't':
        {
        	char *timeFile = g_strdup(optarg), time[25];
			FILE *fp = fopen(timeFile, "r");
			TIMEENTRY	*timeEntry;
			struct ptime    date;
			
			if (!fp)
			{
				fprintf(stderr, "Unable to open file %s, ignoring...\n", timeFile);
				free(timeFile);
				break;
			}
            while (fgets(time, 25, fp))
            {
				timeEntry = calloc(1, sizeof(TIMEENTRY));
				sscanf(time, "%d:%d:%d:%d:%d.%d",
								&timeEntry->year,
								&timeEntry->jday,
								&timeEntry->hour,
								&timeEntry->minute,
								&timeEntry->second,
								&timeEntry->msec);
				time[strlen(time)-1] = 0;
				timeEntry->str = g_strdup(time);
				date.yr = timeEntry->year;
				date.day = timeEntry->jday;
				date.hr = timeEntry->hour;
				date.mn = timeEntry->minute;
				date.sec = timeEntry->second;
				timeEntry->epochs = passcal_htoe(&date, FALSE) + timeEntry->msec/1000.;
				settings.general.input.shotTimes = 
						g_slist_prepend(settings.general.input.shotTimes,
											timeEntry);
			}
			settings.general.input.shotTimes = g_slist_sort(settings.general.input.shotTimes,
															(GCompareFunc) sortShotTimes);
			fclose(fp);
			free(timeFile);
		}
		break;

		case 'i':
		{
			char *instFile = g_strdup(optarg), idNO[10];
			FILE *fp = fopen(instFile, "r");
			GHashTable *hash;
			int		i, *pos;
			
			if (!fp)
			{
				fprintf(stderr, "Unable to open file %s, ignoring...\n", instFile);
				free(instFile);
				break;
			}
			
			hash = settings.general.sort.instOrder = g_hash_table_new(g_str_hash, g_str_equal);
			i=1;
            while (fgets(idNO, 10, fp))
            {
				idNO[strlen(idNO)-1] = 0;
				pos = malloc(sizeof(int));
				*pos = i++;
				g_hash_table_insert(hash, g_strdup(idNO), pos);
			}
			fclose(fp);
			free(instFile);
			criteria[SENSOR].fieldType = MYINSTSORT;
		}			
		break;

        default:
            printHelp();
            exit(-1);
        break;
      }
   }

   states.general.plot.state = NOTRACES;
   if(read_traces)
   {	// read the TRACE files into Files array
      for(i=optind;i<argc;i++)
      {
         pqlIn = calloc(1, sizeof(PQLDATA));
         pqlIn->fileName = g_strdup(argv[i]);
         states.input.Files = g_slist_prepend(states.input.Files, pqlIn);
      }
      states.input.numFiles = g_slist_length(states.input.Files);
   }

   g_thread_init(NULL);
   gtk_init(&argc, &argv);

   if (states.input.numFiles)
   {
	   g_idle_add((GSourceFunc) _makePixmap, GINT_TO_POINTER(TRACESCR));
   }

   make_pql(low_res);
   gtk_main();
   return TRUE;
}
