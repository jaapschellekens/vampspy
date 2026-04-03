/* $Header: /home/schj/src/vamps_0.99g/src/topsys/RCS/notree.c,v 1.4 1999/01/06 12:13:01 schj Alpha $ */

/* $RCSfile: notree.c,v $
 * $Author: schj $
 * $Date: 1999/01/06 12:13:01 $ */

static char RCSid[] =
 "$Id: notree.c,v 1.4 1999/01/06 12:13:01 schj Alpha $";

/*F:notree.c
 * This file contains the 'bare-soil' topsystem. In this case interception and
 * transpiration are always zero. All available energy is used for soil
 * evaporation. At the moment soilevaporation can be calculated using one of
 * the folowing methods:
 * 	0	E0SUNRAD
 * 	1	E0NETRAD
 * 	2	PENMON_NOSOIL
 * 	3	PENMON_SOIL
 * 	4	MAKKINK
 *
 * The following datasets are _always_ needed:
 * 	rlh
 * 	tem
 * 	for MAKKINK:
 * 		ira
 * 	for E0SUNRAD:
 * 		ira
 * 		win
 * 		sur
 * 	for E0NETRAD
 * 		ira
 * 		ref
 * 		win
 * 		nra
 */ 	

#include "topsys.h"
#include "vamps.h"
#include "met.h"
#include "deffile.h"


typedef struct  { 
	int	id;
	char	*description;
	int	*req_sets;	/* list of required sets */
} s_evaptype;

/* How to define soil evaporation we need a solution for the
 * soil-interaction as well */
#define E0SUNRAD 0
#define E0NETRAD 1
#define PENMON_NOSOIL   2
#define PENMON_SOIL 3
#define MAKKINK  4

static int how = MAKKINK;
void topout(int tstep);
static double soilevap(int tstep);
static double top_makkink(int tstep);
static double top_e0sunrad(int tstep);
static double top_e0netrad(int tstep);
	

/* No canopy at all */
void
tstep_top_notree(int tstep, double *precipitation, double *interception,
		double *transpiration, double *soilevaporation)
{
	*precipitation = data[id.pre].xy[tstep].y;
	*interception = 0.0;
	*transpiration = 0.0;
	*soilevaporation = soilevap(tstep);
	topout(tstep);
}


void
pre_top_notree()
{
	/* This one should set how to determine stuff and get info from
	 * input file. Also sets should be added and mem allocated */

#ifdef TRY_SUBDAY
	fprintf(stderr,"Warning: using experimental topsys only MAKKINK (4) supported\n");
#endif	
	/* Get evap - method */
	how = getdefint("top","soilevaporation",how,infilename,TRUE);

	/* we need at least: rlh, tem */
	if ((id.rlh = getsetbyname("rlh")) == -1)
		(void)get_data(getdefstr("ts","rlh","NON",infilename,TRUE),"rlh",0);
	id.rlh = getsetbyname("rlh");
	if ((id.tem = getsetbyname("tem")) == -1)
		(void)get_data(getdefstr("ts","tem","NON",infilename,TRUE),"tem",0);
	id.tem = getsetbyname("tem");

	switch (how){
		case MAKKINK: /* we need ira and nothing else */
			if ((id.ira = getsetbyname("ira")) == -1)
				(void)get_data(getdefstr("ts","ira","NON",
						infilename,TRUE),"ira",0);
			id.ira = getsetbyname("ira");
			break;
		case E0NETRAD:
			if ((id.ira = getsetbyname("ira")) == -1)
				(void)get_data(getdefstr("ts","ira","NON",
						infilename,TRUE),"ira",0);
			id.ira = getsetbyname("ira");
			if ((id.win = getsetbyname("win")) == -1)
				(void)get_data(getdefstr("ts","win","NON",
						infilename,TRUE),"win",0);
			id.win = getsetbyname("win");
			if ((id.nra = getsetbyname("nra")) == -1)
				(void)get_data(getdefstr("ts","nra","NON",
						infilename,TRUE),"nra",0);
			id.nra = getsetbyname("nra");
			if ((id.ref = getsetbyname("ref")) == -1)
				(void)get_data(getdefstr("ts","ref","NON",
						infilename,TRUE),"ref",0);
			id.ref = getsetbyname("ref");
			break;
		case E0SUNRAD:
			if ((id.ira = getsetbyname("ira")) == -1)
				(void)get_data(getdefstr("ts","ira","NON",
						infilename,TRUE),"ira",0);
			id.ira = getsetbyname("ira");
			if ((id.win = getsetbyname("win")) == -1)
				(void)get_data(getdefstr("ts","win","NON",
						infilename,TRUE),"win",0);
			id.win = getsetbyname("win");
			if ((id.sur = getsetbyname("sur")) == -1)
				(void)get_data(getdefstr("ts","sur","NON",
						infilename,TRUE),"sur",0);
			id.sur = getsetbyname("sur");
			break;
		case PENMON_NOSOIL:
			break;
		default:
			Perror(progname,1,0,RCSid,"Invalid soilevap method","");
			break;
	}
}

void
post_top_notree()
{
}

static double soilevap(int tstep)
{
	double retval = 0.0;

	switch (how){
	case E0SUNRAD:
		retval = 0.1 * top_e0sunrad(tstep);
		break;
	case E0NETRAD:
		retval = 0.1 * top_e0netrad(tstep);
		break;
	case MAKKINK:
		retval = 0.1 * top_makkink(tstep);
		break;
	default:
		break;
	}

	return retval;
}

static double
top_makkink(int tstep)
{
	double Slope, L, Gamma, Es,Ea;

	eaes(data[id.tem].xy[tstep].y,data[id.rlh].xy[tstep].y,
			&Ea, &Es);
	Slope = vslope(data[id.tem].xy[tstep].y, Es);
	L = lambda(data[id.tem].xy[tstep].y);
	Gamma = mgamma(data[id.tem].xy[tstep].y,L);
	return makkink(data[id.ira].xy[tstep].y,Slope, Gamma,L);
}

static double 
penman_mon_soilevap()
{
	return 0.0;
}

static double 
top_e0sunrad(int tstep)
{
	double Ea, Es,Slope,Gamma,L, Earo,rnopen;

	/* Get Ea and Es from tdry and relative humidity */
	eaes(data[id.tem].xy[tstep].y,data[id.rlh].xy[tstep].y,
			&Ea, &Es);
	Slope = vslope(data[id.tem].xy[tstep].y, Es);
	L = lambda(data[id.tem].xy[tstep].y);
	Gamma = mgamma(data[id.tem].xy[tstep].y,L);
	Earo = earo(Ea, Es, data[id.win].xy[tstep].y);

	rnopen = rnet_open_nN(data[id.ira].xy[tstep].y,data[id.sur].xy[tstep].y,
			data[id.tem].xy[tstep].y,Ea,L);

	return e0(rnopen,Slope,Gamma,Earo);
}

static double 
top_e0netrad(int tstep)
{
	double Ea, Es,Slope,Gamma,L, Earo,rnopen;;

	/* Get Ea and Es from tdry and relative humidity */
	eaes(data[id.tem].xy[tstep].y,data[id.rlh].xy[tstep].y,
			&Ea, &Es);
	Slope = vslope(data[id.tem].xy[tstep].y, Es);
	L = lambda(data[id.tem].xy[tstep].y);
	Gamma = mgamma(data[id.tem].xy[tstep].y,L);
	Earo = earo(Ea, Es, data[id.win].xy[tstep].y);
	rnopen = rnet_open(data[id.ira].xy[tstep].y,data[id.ref].xy[tstep].y,
			data[id.nra].xy[tstep].y,L);

	return e0(rnopen,Slope,Gamma,Earo);
}
