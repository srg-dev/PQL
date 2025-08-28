#ifndef __PQLX_SRVR_UTILS
#define __PQLX_SRVR_UTILS

enum {
	SCAN_FCREATE,
	SCAN_FDELETE
};

int makePNG(PNGARGS *pngArgs);
void freePNGData(int which, void *pngData);
void logMsg(int type, gpointer d);
void	TIMESTRGMT(char *timeStr);

#endif
