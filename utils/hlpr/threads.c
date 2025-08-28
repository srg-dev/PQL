#ifdef _WIN32
#include <windows.h>
#elif MAC
#include <sys/param.h>
#include <sys/sysctl.h>
#else
#include <unistd.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <glib.h>
#include <gdk/gdk.h>
#include "db_iface_defs.h"

void makeThread(gboolean callType, GThreadFunc toCall(void *), void *args, GThreadFunc toReturn(void *))
{
	if (callType == ASYNC)
	{
		g_thread_create((GThreadFunc) toCall, args, FALSE, NULL);
		return;
	}

	if (toCall)
		(*toCall)( args );
	if (toReturn)
		(*toReturn)( args );

	return;
}

int get_ncpu_pqlx() 
{
#ifdef WIN32
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    return sysinfo.dwNumberOfProcessors;
#elif MAC
    int nm[2];
    size_t len = 4;
    uint32_t count;

    nm[0] = CTL_HW; nm[1] = HW_AVAILCPU;
    sysctl(nm, 2, &count, &len, NULL, 0);

    if(count < 1) {
        nm[1] = HW_NCPU;
        sysctl(nm, 2, &count, &len, NULL, 0);
        if(count < 1) { count = 1; }
    }
    return count;
#else
    return sysconf(_SC_NPROCESSORS_ONLN);
#endif
}

gboolean beepMe(gpointer nil)
{
	gdk_beep();
	return FALSE;
}
