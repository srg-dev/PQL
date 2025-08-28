#ifndef __STEIM_H
#define __STEIM_H

#include <sys/types.h>
#include <string.h>

#ifndef u_char
#define u_char	unsigned char
#define u_int	unsigned int
#endif

typedef union steim_frame 
{
	char str[64];
	int word[16];
} Frame;

void revmem(u_char *, int);
void rmemcpy(u_char *, u_char *, int);
int endian (void);

#endif /* STEIM_H	*/
