/* $Header: /home/schj/src/vamps_0.99g/src/plot/ts_time.c,v 1.2 1999/01/06 12:13:01 schj Alpha $ */
/*- $RCSfile: ts_time.c,v $
 *  $Author: schj $
 *  $Date: 1999/01/06 12:13:01 $
 *  $Log: ts_time.c,v $
 *  Revision 1.2  1999/01/06 12:13:01  schj
 *  Version: 0.99g
 *
 *  Revision 1.1  1999/01/06 12:07:49  schj
 *  Version: 0.99g
 *
 * Revision 1.2  1996/04/27  19:56:23  schj
 * added <ctype.h>
 *
 *  Revision 1.1  1995/09/21 09:33:14  schj
 *  Initial revision
 *
 */
static  char RCSid[] =
"$Id: ts_time.c,v 1.2 1999/01/06 12:13:01 schj Alpha $";
/*-
 *	void ts_jday(TM *tm, double *jul)
 *		Converts *tm into Julian date, the number of days since
 *		01 Jan 1990 00:00:00.  The Julian is returned in jul.
 *
 *	void ts_jdate(TM *tm, double jul)
 *		Converts Julian date jul, the number of days since
 *		01 Jan 1990 00:00:00, into broken down time stored at TM.
 *
 *	Note:	Both functions use/set only the tm_year, tm_mon, tm_mday
 *		tm_hour, tm_min, and tm_sec fields of TM.
 *		Currently only works correct for years >= 1900.
 *
 *	Reference:
 *		Modified from JDAY and JDATE by Robert G. Tantzen,
 *		algorithm 199 in the Collected Algorithms from CACM
 */
#include "ts.h"
#include <ctype.h>

/* 0000/03/01 - 1900/01/01 = 1721119 - 2415021 */
#define DATUM	(-693902)

void
ts_jday(tm, jul)
TM	*tm;
double	*jul;
{
	int	c, yr, mo, dy, j, s;

	dy = tm->tm_mday;
	if(tm->tm_mon > 1) {
		yr = tm->tm_year + 1900;
		mo = tm->tm_mon - 2;
	}
	else {
		yr = tm->tm_year - 1 + 1900;
		mo = tm->tm_mon + 10;
	}
	c = yr / 100;
	yr = yr - 100 * c;
	j = 146097 * c / 4 + 1461 * yr / 4 + (153 * mo + 2) / 5 + dy + DATUM;
	s = tm->tm_hour * 3600 + tm->tm_min * 60 + tm->tm_sec;
	*jul = (double)j + (double)s / 86400.0;

	return;
}

void
ts_jdate(tm, jul)
TM	*tm;
double	jul;
{
	int	j, yr, mo, dy, s;

	j = (int)jul;
	s = (int)((jul - (double)j) * 86400.0 + 0.5);
	tm->tm_sec = s % 60;
	tm->tm_min = s / 60 % 60;
	tm->tm_hour = s / 3600;

	j -= DATUM;
	yr = (4 * j - 1) / 146097;
	j = 4 * j - 1 - 146097 * yr;
	dy = j / 4;
	j = (4 * dy + 3) / 1461;
	dy = (4 * dy + 3 - 1461 * j + 4) / 4;
	mo = (5 * dy - 3) / 153;
	tm->tm_mday = (5 * dy - 153 * mo + 2) / 5;
	if(mo < 10) {
		tm->tm_mon = mo + 2;
		tm->tm_year = yr * 100 + j - 1900;
	}
	else {
		tm->tm_mon = mo - 10;
		tm->tm_year = yr * 100 + j + 1 - 1900;
	}

	return;
}

/*-
 *	int ts_time(char *str, char **eptr, Time *tm)
 *		Scans time in str ([[yyyy]mm]dd[.HH[MM[SS]]]) and
 *		modifies the fields at *tm accordingly. Leading whitespace
 *		is skipped. If yy or mm is 00 or missing, no modification
 *		is done. If any of HHMMSS are missing, they are set to 00.
 *		If dd is 00, no modification of any kind is made and
 *		success status is returned.
 *	Ret:	0 on succes, -1 on format error. If eptr != NULL and the
 *		scan succeeded, the contents will point to the first
 *		character in str that terminated the scan, else str is
 *		returned in *eptr.
 */

int
ts_time(str, eptr, tm)
char	*str, **eptr;
TM	*tm;
{
	int	n, yy, mm, dd, HH, MM, SS;

	yy = mm = dd = HH = MM = SS = 0;
	if(eptr)
		*eptr = str;

	while(isspace(*str))
		str++;

	for(n = 0; str[n]; n++)
		if(!isdigit(str[n]))
			break;
	if(!n || n > 8)
		return(-1);

	for( ; n > 4; n--)
		yy = yy * 10 + *str++ - '0';
	for( ; n > 2; n--)
		mm = mm * 10 + *str++ - '0';
	for( ; n; n--)
		dd = dd * 10 + *str++ - '0';

	if(*str == '.') {
		if(isdigit(*++str))
			HH = *str++ - '0';
		if(isdigit(*str))
			HH = HH * 10 + *str++ - '0';
		if(isdigit(*str))
			MM = *str++ - '0';
		if(isdigit(*str))
			MM = MM * 10 + *str++ - '0';
		if(isdigit(*str))
			SS = *str++ - '0';
		if(isdigit(*str))
			SS = SS * 10 + *str++ - '0';
	}
	if(eptr)
		*eptr = str;
	if(!dd)
		return(0);
	tm->tm_mday = dd;
	if(mm)
		tm->tm_mon = mm - 1;
	if(yy)
		tm->tm_year = yy - 1900;
	tm->tm_sec = SS;
	tm->tm_min = MM;
	tm->tm_hour = HH;

	return(0);
}
