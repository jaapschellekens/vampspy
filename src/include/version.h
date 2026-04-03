/* $Header: /home/schj/src/vamps_0.99g/src/include/version.h,v 1.2 1999/01/06 12:13:01 schj Alpha $ */
/*- $RCSfile: version.h,v $
 *  $Author: schj $
 *  $Date: 1999/01/06 12:13:01 $
 */
#ifndef VERSION_H_
#define VERSION_H_

#ifdef HAVE_CONFIG_H
#include "vconfig.h"
#endif

#ifndef OS
#define OS "unknown"
#endif

#define IDENTIFY "vamps"
#define DESCRIPTION "a model of Vegetation-AtMosPhere-Soil water flow"

#ifdef __STDC__
#define BUILDDATE __DATE__
#else
#define BUILDDATE "now"
#endif

#ifndef VERS_CHAR
#define VERS_CHAR 'i'
#endif
#ifndef VERSIONNR
#define VERSIONNR 99
#endif
#ifndef PROVERSION
#define PROVERSION "0.99i -- Nov 2001"
#endif
#ifndef WHO
#define WHO "schj"
#endif
#ifndef WHERE
#define WHERE "flow.geo.vu.nl"
#endif
#ifndef STATUS
#define STATUS "beta version"
#endif
#ifndef AUTHOR
#define AUTHOR "J. Schellekens"
#endif
#ifndef DATE
#define DATE "Nov 2001"
#endif

#ifndef EMAIL
#define EMAIL "schj@xs4all.nl"
#endif

#ifndef ADDRESS
#define ADDRESS "\n \
\tJ. Schellekens\
\n\tTh. de Bockstr 29 hs\
\n\t1058 TW Amsterdam\
\n\tThe Netherlands\
\n\tE-mail: schj@xs4all.nl"
#endif

#define CPR " \
This program may be used free of charge providing\n \
it is used for NO-COMMERCIAL purposes.\n \
For commercial use written permission must be obtained from\n \
ITF and FUA (see the 0readne file) and the author."

#define GNUL " \
       Copyright (C) 1996-2001 Jaap Schellekens.\n \
\n \
 This program is free software; you can redistribute it and/or modify\n \
 it under the terms of the GNU General Public License as published by\n \
 the Free Software Foundation; either version 2, or (at your option)\n \
 any later version.\n \
 \n \
 This program is distributed in the hope that it will be useful,\n \
 but WITHOUT ANY WARRANTY; without even the implied warranty of\n \
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n \
 GNU General Public License for more details.\n \
 \n \
 You should have received a copy of the GNU General Public License\n \
 along with this program; see the file COPYING.  If not, write to\n \
 the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.\n \
 \n\
 \t(C) Jaap Schellekens\
 \n\tAmsterdam\
 \n\tThe Netherlands\
 \n\tE-mail: schj@xs4all.nl"

#endif /* VERSION_H_ */ 
