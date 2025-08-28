/* Public header file for NANO2AH_H	*/

#ifndef NANO2AH_H
#define NANO2AH_H

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <rpc/rpc.h>
#include "ahhead.h"
#include "nano.h"

typedef struct buffer_type {
	char str[1016];
	int pos;
	int size;
} MEMBUF;

#define	SIZECHAR		sizeof(char)
#define	SIZESHORT		sizeof(short)
#define	SIZELONG		sizeof(gint32)
#define	SIZEFLOAT		sizeof(float)
#define	SIZEDOUBLE		sizeof(double)
#define BLOCKSIZE		262144
#define NANOHEADSIZE		605

#ifdef ANSI_C
void read_stdin ();
void read_files ();
void verify_dir (char *);
char *strip_path (char *);
#else
void read_stdin();
void read_files();
void verify_dir();
char *strip_path();
#endif

#endif /* NANO2AH_H	*/
