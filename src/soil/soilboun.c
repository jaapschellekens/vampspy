/* $Header: /home/schj/src/vamps_0.99g/src/soil/RCS/soilboun.c,v 1.26 1999/01/06 12:13:01 schj Alpha $ */
/*  $RCSfile: soilboun.c,v $
 *  $Author: schj $
 *  $Date: 1999/01/06 12:13:01 $
 */
/* soilboun.c
 * 	contains functions for calculation of the boundary
 *	conditions
 */

static char RCSid[] =
"$Id: soilboun.c,v 1.26 1999/01/06 12:13:01 schj Alpha $";

#include "soils.h"
#include "vamps.h"
#include <math.h>
#include "swatsoil.h"

#define MINPOND 1.0E-6
int	fltsat;
double  hatm, hatmd, ksurf;
double  pond = 0.0;
double  pondmx = 0.0;
double qsurf=0.0;
int dodrain = FALSE;
double slope  = 0.0;
double inflow;
int *allowdrain;/* 0 == no drianage in layer allowed */
extern double reva;

static double qmax;

/*C:bocotop
 *@ double bocotop (double *kgeo0,int *ftoph)
 *
 * Determines the top boundary conditions for each day (timestep)
 *
 *  Returns:  qtop 
 */
double
bocotop (double *kgeo0,int *ftoph)
{
	double _qtop;

	/* Determine potential flux at the top of the profile */
	if (pond > MINPOND)
		_qtop = (reva + intc - (pond / dt) - prec); 
	else
		_qtop = (reva + intc - prec);


	if (_qtop >= 0.0){/* determine max evaporation flux */
		ksurf = node[0].sp->
			t2k(node[0].soiltype, node[0].sp->
					h2t (node[0].soiltype, hatm));
		*kgeo0 = 0.5 * (ksurf + k[0]);
		qmax = kgeom[0] * (hatm - h[0]) / depth[0] - kgeom[0];
		_qtop = _qtop < fabs (qmax) ? _qtop : fabs (qmax);
		*ftoph = FALSE;
	}else{
		/* Determine boundary conditions during infiltration */
		*kgeo0 = 0.5 * (node[0].sp->ksat + k[0]);
		qsurf = kgeom[0] * (pond - h[0]) / depth[0] - kgeom[0];
		inflow = (qbot - _qtop - qdrtot) * dt;

		if (inflow > fabs(volsat - volact)){
			*ftoph = TRUE;
		}else{
			if (_qtop > qsurf){
				*ftoph = FALSE;
			}else{
				if (_qtop > -node[0].sp->ksat)
					*ftoph = FALSE;
				else
					*ftoph = TRUE;
			}
		}
	}

	return _qtop;
}

/*C:bocobot
 *@ void bocobot(int pt)
 *
 * Determines the values of the boundary conditions at the bottom of
 * the soil profile
 *
 *  Returns:  nothing */
void
bocobot (int daynr)
{
	double  tfrac;
	int     i;
	int     lastl;
	double  deepgw;
	double  qsat;

	tfrac = t - (int) t;

	if (tfrac == 0.0)
		tfrac = 1.0;


	lastl = layers - 1;
	switch (lbc){/* Process for different boundary conditions */
		case 0:
			/* Interpolation between daily values of given groundwaterlevel */
			/*if ((daynr == dayend && year == yearen) || daynr == ndyear)
				gwl[0] = data[id.gwt].xy[daynr].y;
			else 
				gwl[0] = data[id.gwt].xy[daynr].y + tfrac * (data[id.gwt].xy[daynr + 1].y - data[id.gwt].xy[daynr].y); */
			qbot = _getval(&data[id.gwt], t);
			h[lastl] = gwl[0] - z[lastl];
			theta[lastl] = node[lastl].sp->h2t (node[lastl].soiltype, h[lastl]);
			diffmoist[lastl] = node[lastl].sp->h2dmc (node[lastl].soiltype, h[lastl]);
			k[lastl] = node[lastl].sp->t2k (node[lastl].soiltype, theta[lastl]);
			kgeom[lastl] = MKKGEOM(lastl); /*sqrt ((k[lastl] * k[lastl - 1]));*/
			showit ("swatsoil",ERR,"NOT YET IMPLEMENTED LBC = 0 ",0,soilverb);
			exit (1);
			break;
		case 1: /* Given flux */
			/*
			if ((daynr == dayend && year == yearen) || daynr == ndyear)
				qbot = data[id.qbo].xy[daynr].y;
			else{
				if (daynr >= steps)
					qbot = data[id.qbo].xy[daynr].y;
				else
					qbot = data[id.qbo].xy[daynr].y + tfrac * (data[id.qbo].xy[daynr + 1].y - data[id.qbo].xy[daynr].y);
			}
			*/
			qbot = _getval(&data[id.qbo], t);
			break;
		case 2:
			/* Seepage or infiltration from/to deep groundwater */
			deepgw = aqave + aqamp * cos (aqomeg * (t - aqtamx));
			qbot = (deepgw - gwl[0]) / rimlay;
			break;
		case 3:
			/* Flux calculated as function of h */
			qbot = cofqha * exp (cofqhb * fabs (gwl[0]));
			break;
		case 4:	/* interpolation between daily values of given pressurehead */
			/*
			if ((daynr == dayend && year == yearen) || daynr == ndyear){
				h[layers - 1] = hgiven[daynr];
			}else{
				h[layers - 1] = hgiven[daynr] + tfrac * (hgiven[daynr + 1] - hgiven[daynr]);
			}
			*/
			h[layers - 1]  = _getval(&data[id.hea], t);
			theta[lastl] = node[lastl].sp->h2t (node[lastl].soiltype, h[lastl]);
			diffmoist[lastl] = node[lastl].sp->h2dmc (node[lastl].soiltype, h[lastl]);
			k[lastl] = node[lastl].sp->t2k (node[lastl].soiltype, theta[lastl]);
			kgeom[lastl] = MKKGEOM(lastl);/*0.5 * (k[lastl] + k[lastl - 1]);*/
			break;
		case 5:			/* Zero flux at bottom */
			qbot = 0.0;
			break;
		case 6:			/* Free drainage */
			qbot = -1.0 * kgeom[layers];
			break;
		default:
			break;
	}
	/* Bottom compartiment saturated ?
	   if qbot up find maximum flux through saturated part */
	qsat = 1000.0;
	if (h[lastl] >= 0.0 && qbot > 0.0){
		fltsat = TRUE;
		for (i = lastl - 1; i >= 0; i--){
			if (fltsat){
				if (h[i] >= 0.0)
					qsat = qsat < kgeom[i + 1] ? qsat : kgeom[i + 1];
				else
					fltsat = FALSE;
			}
		}
	}

	/* qbot up and larger than qsat, print error and continue */
	if (qbot > 0.0 && qbot > qsat && lbc != 0){
		Perror (progname, 0,0, RCSid, "Bottom bound flux large", "");
	}
}

/*+Name: drainage
 *
 *  Prototype:  void drainage (int method)
 *
 *  Description:  method: 1) topog, only flow at saturation 2) vamps, also unsaturated flow
 *
 *  Returns:  nothing+*/
void
drainage(int method)
{
	int	j;
	double  kk;

	for (j=0;j<layers; j++){
		if (allowdrain[j]){ /*drainage allowed only in these layers */
			kk = k[j]  * node[j].sp->kh_kv;
			if (method == 1 && theta[j] < node[j].sp->thetas - 0.01) /* Not saturated, no flow (TOPOG model)*/
				kk = 0.0; /* set kd to zero */
			qdra[0][j] = kk * slope; /* Calculated outflow cm/day */
			mqdra[j] += qdra[0][j]*dt;
		}
	}
}


/* Program DeZeeuwHellinga; {13-may-91} {$N+} {$S+} {$R+} {$M 16384,0,65520} {

DZH.PAS is de opvolger van ERNST.PAS en ZEEHEL.PAS. Output will be printed
to printer and to printfile.
}
uses DOS, Crt, Printer, Tbox, Windows, Editor, Graphix;

const
  numint   = 12;                      { number of timesteps per year }
  shift    = -0.17;                    { delay within usaturated zone }
  Hbase    = 0.0;                     { drainage basis above mean sea level }
{ start    = 1955 + 0.5/numint + shift; }
  PPName   = 'AVG-POT.NE';
  measName = 'P33INT.DAT';
  outName  = 'DZH.OUT';
var
  P,              { oorspronkelijke neerslag }
  EVAP,           { potentiele evapotranspiratie: maandelijkse cropfactor*Eo }
  h0,             { initiele stijghoogte }
  hOld,           { stijghoogte op t-1 }
  hNew,           { stijghoogte op t }
  dt,             { tijdstap }
  STO,            { storage coefficient }
  e,              { exp(-jt) }
  j,              { reservoir factor }
  W    : real;    { drainage resistance }
  L,              { lengte vh aquifer }
  KD,             { transmissiviteit }
  ERE  : integer; { evenwijdig, radiaal of elliptisch }
  t    : word;    { tijd }
  pp,mm,oo   : text;
  MinX,MaxX,MinY,MaxY,start : single;

Procedure Calculation;
var N,Wt : real;
    xx : single;
    i  : byte;

begin
  assign(pp,PPname); reset(pp);
  assign(oo,OutName); rewrite(oo);
  PopOpen(On,' - Calculating - ');
    t:= 0;
    dt:= 365.25/numint;
    Wt:= W/dt;
    e:= exp(-j*dt);
    readln(pp,xx,P);
    Start:= xx + shift;
    repeat
      N:= P/1000; { invoer --> in meters }
      hNew:= hOld*e + N*Wt*(1-e);
      writeln(oo,(Start+t/numint):10:3,(hNew+Hbase):10:3);
      hOld:= hNew;
      readln(pp,xx,P); inc(t);
    until EOF(pp);
  CloseFrame;
  Close(pp);
  Close(oo);
  XY(5,18,'Eindwaarde h na '); write((t/12):4:2,' jaar neerslag        : ',hOld:10:3);
end;

    XY(7, 5,'            Input name : '); write(grfname[inp]);
    XY(7, 6,'           Output name : '); write(grfname[oup]);
    XY(7, 7,'        skip textlines : '); write(txtrow);
    XY(7, 8,'      Start at X-value : '); write(start:1:0);
    XY(7, 9,'         Column number : '); write(column);
    XY(7,10,'Number of moving terms : '); write(dt);
    textcolor(14);
    CheckFileName(7, 5,'            input name : ',grfname[inp]);
    grfname[oup] := grfname[inp];
    capital(grfname[oup]); changeExt(grfname[oup],'MA ');
    CheckFileName(7, 6,'           output name : ',grfname[oup]);
    CheckWord    (7, 7,'        skip textlines : ',txtrow);
    CheckSing    (7, 8,'      start at X-value : ',start,1,0);
    CheckWord    (7, 9,'         column number : ',column);
    CheckWord    (7,10,'number of moving terms : ',dt);


Procedure Display;
var X,Y : byte;
begin
  XY(6,2,'           -jt         -jt        ');
  XY(6,3,'h  = h   .e    + (1 - e    ).R .W ');
  XY(6,4,' t    t-1                     t   ');
  XY(5,6,'컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴');
  repeat
    XY(6, 7,'Initiele stijghoogte [m]                  : '); X:= whereX; Y:= whereY; write(h0:8:3,' ? ');
    GetReal(WhereX,WhereY,h0); gotoXY(X,Y); clreol; write(h0:8:3);
    XY(6, 8,'Aquifer lengte [m]                        : '); X:= whereX; Y:= whereY; write(L:8,' ? ');
    GetInt(WhereX,WhereY,L); gotoXY(X,Y); clreol; write(L:8);
    XY(6, 9,'Porositeit [-]                            : '); X:= whereX; Y:= whereY; write(STO:8:3,' ? ');
    GetReal(WhereX,WhereY,STO); gotoXY(X,Y); clreol; write(STO:8:3);
    XY(6,10,'Transmissiviteit [m2/d]                   : '); X:= whereX; Y:= whereY; write(KD:8,' ? ');
    GetInt(WhereX,WhereY,KD); gotoXY(X,Y); clreol; write(KD:8);
    XY(6,11,'[1] Evenwijdig [2] Radiaal [3] Elliptisch : '); X:= whereX; Y:= whereY; write('[',ERE,'] ? ');
    GetInt(WhereX,WhereY,ERE); gotoXY(X,Y); clreol; write('[',ERE:1,']');
    Case ERE of
      1 : W:= L/(2*KD)*L;  { evenwijdig }
      2 : W:= L/(4*KD)*L;  { radiaal }
      3 : W:= L/(3*KD)*L;  { elliptisch }
    end;
    j:= 1/(W*STO);
    hOld:= h0;
    XY(5,16,'Drainage weerstand wordt (elliptisch) [d]  : '); write(W:10:3);
    XY(5,17,'Reservoir factor wordt (1/j) [d]           : '); write((1/j):10:3);
  until ChPop(Off,'Ready (Y/N)? Y') <> 'N';
end;

Procedure Print;
const PrtName = 'DZH.PRT';
var i   : byte;
    prt : text;
begin
  assign(prt,prtName);
  append(prt);
  for i:= 1 to 80 do write(prt,'_');
  writeln(prt);
  writeln(prt,' Precipitation Surplus Name  : ',PPName);
  writeln(prt,' Measurements FileName       : ',measName);
  writeln(prt,' Output FileName             : ',outName);
  writeln(prt);
  writeln(prt,' Aantal tijdstappen per jaar                : ',numint:6);
  writeln(prt,' Verschuiving van de berekeningen           : ',shift:9:2);
  writeln(prt,' Begin van de berekeningen                  : ',start:9:2);
  writeln(prt,' Drainage basis a.m.s.l.                    : ',Hbase:9:2);
  writeln(prt,' Initiele stijghoogte [m]                   : ',h0:9:2);
  writeln(prt,' [1] Evenwijdig [2] Radiaal [3] Elliptisch  :    [',ERE:1,']');
  writeln(prt);
  writeln(prt,' Aquifer lengte [m]                         : ',L:6);
  writeln(prt,' Transmissiviteit [m2/d]                    : ',KD:6);
  writeln(prt,' Porositeit [-]                             : ',STO:9:2);
  writeln(prt,' Drainage weerstand [d]                     : ',W:9:2);
  writeln(prt,' Reservoir factor (1/j) [d]                 : ',(1/j):9:2);
  for i:= 1 to 80 do write(prt,'_');
  writeln(prt);
  close(prt);

  for i:= 1 to 80 do write(lst,'_');
  writeln(lst);
  writeln(lst,' Precipitation Surplus Name  : ',PPName);
  writeln(lst,' Measurements FileName       : ',measName);
  writeln(lst,' Output FileName             : ',outName);
  writeln(lst);
  writeln(lst,' Aantal tijdstappen per jaar                : ',numint:6);
  writeln(lst,' Verschuiving van de berekeningen           : ',shift:9:2);
  writeln(lst,' Begin van de berekeningen                  : ',start:9:2);
  writeln(lst,' Drainage basis a.m.s.l.                    : ',Hbase:9:2);
  writeln(lst,' Initiele stijghoogte [m]                   : ',h0:9:2);
  writeln(lst,' [1] Evenwijdig [2] Radiaal [3] Elliptisch  :    [',ERE:1,']');
  writeln(lst);
  writeln(lst,' Aquifer lengte [m]                         : ',L:6);
  writeln(lst,' Transmissiviteit [m2/d]                    : ',KD:6);
  writeln(lst,' Porositeit [-]                             : ',STO:9:2);
  writeln(lst,' Drainage weerstand [d]                     : ',W:9:2);
  writeln(lst,' Reservoir factor (1/j) [d]                 : ',(1/j):9:2);
  for i:= 1 to 80 do write(lst,'_');
  writeln(lst);
end;

Procedure Init;
begin
  SetScreen(' - De Zeeuw-Hellinga - ',' relatie tussen neerslag en voeding','---',Off);
  h0  := 21.25 - Hbase;
  L   := 7500;
  STO := 0.56;
  KD  := 875;
  ERE := 3;
end;

begin
  Init;
  Repeat
    Cursor(Off);
    Display;
    Calculation;
    if ChPop(off,'Display graph (Y/N)? Y') <> 'N' then begin
      CalcXYtremes(measName,MinX,MaxX,MinY,MaxY);
      MakeGraph(measName,outName,MinX,MaxX,MinY,MaxY,'Simulation of:');
    end;
    if ChPop(off,'Print (Y/N)? N') = 'Y' then Print;
  Until ChPop(Off,'Finished (Y/N)? N') = 'Y';
  Done;
end.

*/
