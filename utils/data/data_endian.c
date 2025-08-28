#include <string.h>
#include <glib.h>
#include "myendian.h"

int testByteOrder()
{	// determines the endianness of the running machine
	short int	word = 0x0001;
	char *byte = (char *) &word;
	return(byte[0] ? MY_LITTLE_ENDIAN : MY_BIG_ENDIAN);
}

/*===========================================================================*/
/* SEED reader     |               swap_2byte              |    subprocedure */
/*===========================================================================*/
/*
	Name:		swap_2byte
	Purpose:	reorder a 2-byte word from 10 to 01 (MSB-first to MSB-last)
				or from 01 to 10
	Usage:		unsigned short int swap_2byte ();
				unsigned short int word2;
				unsigned short int result;
				result = swap_2byte (word2);
	Input:		a 2-byte word in order 10
	Output:		a 2-byte word in order 01
	Externals:	none
	Warnings:	none
	Errors:		none
	Called by:	anything
	Calls to:	none
	Algorithm:	Using a union between an unsigned short int and 2 chars,
				shuffle the bytes around to achieve the reverse word order.
	Notes:		none
	Problems:	none known
	References:	Halbert et al, 1988; see main routine
	Language:	C, hopefully ANSI standard
	Author:		Dennis O'Neill
	Revisions:	11/09/88  Dennis O'Neill  original version
				11/21/88  Dennis O'Neill  Production release 1.0
*/

unsigned short swap_2byte(unsigned short nValue)
{
	return (((nValue>>8))|(nValue <<8));
}

guint32 swap_4byte(guint32 nValue)
{
	return (((nValue&0x000000FF)<<24)+((nValue&0x0000FF00)<<8)+((nValue&0x00FF0000)>>8)+((nValue&0xFF000000)>>24));
/*
	unsigned long nRet;

	__asm
	{
		mov eax, nValue
		xchg ah, al
		ror eax, 16
		xchg ah, al
		mov nRet, eax
	}

	return nRet;
*/
}

float	swap_4byteF(char *nValue)
{
	float	result;
	char	k1, k2, k3, k4;

	k1 = *nValue;
	k2 = *(nValue+1);
	k3 = *(nValue+2);
	k4 = *(nValue+3);
	
	*nValue = k4;
	*(nValue+1) = k3;
	*(nValue+2) = k2;
	*(nValue+3) = k1;

	memcpy(&result, nValue, sizeof(float));
	return(result);
}

