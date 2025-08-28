#ifndef __PQL_DEFINE_INCLUDE
#define __PQL_DEFINE_INCLUDE

// include the GNU General Public License
#include "PQLX.GPL"
#include "pql_version.h"
 
//	PQL II: Seismic Trace Viewer
//  PQLX: Seismic Data Analysis System
//	Copyright (C) 2005-2008:	Richard I. Boaz
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program (file PQL.GPL); if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#define MY_BIG_ENDIAN		1
#define MY_LITTLE_ENDIAN	0

#include "pqlx_defs.h"

typedef struct gr {
    int		logx, logy;
    double	user_xmin, user_xmax;
    float	user_ymin, user_ymax;
    float	user_xdif, user_ydif;
    int		pix_xmin, pix_xmax;
    int		pix_ymin, pix_ymax;
    int		pix_ydif, pix_xdif;
} graph;

typedef struct _PQLDATA
{	// input structure to marryTrcs(), cutTrc(), PQL, and others
	char	*fileName;
	void	*trc;				// traceInfo *
	void	*trcData;			// TRCDATA	- db_iface.h
	void	*anaData;			// ANADATA	- STN System
	void	*evtData;			// EVTDATA	- STN System
	struct {
		void	*chnMETA;		// chnMETA info must be retained when Cutting
		gint	hashKey;		// = ANADATA->hashKey, must be externalized to ANADATA itself
	} misc;
} PQLDATA;

#define passcal_isleap(a)	((a % 4 == 0 && (a % 100 != 0 || a % 400 == 0)))

// GTK implementation defines
//#define	TRACE	TRUE
//#define WIND	FALSE
#define TRACESCREEN TRUE
#define TRACEWHOLE FALSE
#define DISPLAY	0
#define UNDISPLAY 1
#define PRINTDISPLAY -1

#include "pql_data_structs.h"

#endif 
