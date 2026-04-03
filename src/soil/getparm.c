/* $Header: /home/schj/src/vamps_0.99g/src/soil/RCS/getparm.c,v 1.8 1999/01/06 12:13:01 schj Alpha $ */
/* 
 *  $RCSfile: getparm.c,v $
 *  $Author: schj $
 *  $Date: 1999/01/06 12:13:01 $ 
 */

static char RCSid[] =
"$Id: getparm.c,v 1.8 1999/01/06 12:13:01 schj Alpha $";

#include <math.h>
#include <stdlib.h>
#include "vamps.h"
#include "deffile.h"
#include "swatsoil.h"
#include "marquard.h"
#include "soils.h"
extern double estdmc;
extern double tm_mult;

/*C:getparms
 *@ int getparams(char *infilename)
 * Gets the parameters from the inputfile needed for soil module
 *  Returns: 0 (always)
 *  Remarks: should be cleaned*/
int
getparams (infilename)
     char *infilename;
{
	int i;
	int pts;
	double *tmpar;
	int are_roots=0;
	XY *tmpxy;
	char s[1024];


	speed = getdefint ("soil","speed",speed,infilename,FALSE);
	switch (speed) {
		case 1: 
				/* This is the slowest setting */
				solvemet = GEN;
				maxitr = (int) (maxitr * 3);
				thetol *= 0.4;
				tm_mult *= 0.4;
				dtmin *= 0.5;
				dtmax *= 0.1;
				mktab = 0;
				break;
		case 2:  
				 solvemet = BAN;
				 maxitr = (int)(maxitr * 2);
				 thetol *= 0.8;
				 tm_mult *= 0.6;
				 dtmin *= 0.8;
				 dtmax *= 0.8;
				 mktab = 0;
				 break;
			 
		case 3:   /* default */
				 break;
			 
		case 4:  
				 maxitr = (int)(maxitr * 0.8);
				 tm_mult *= 1.2;
				 dtmin *= 2.0;
				 thetol *=1.1;
				 break;
			 
		case 5:  	
				 maxitr = (int)(maxitr * 0.6);
				 thetol *=1.1;
				 tm_mult *= 1.4;
				 mktab = 1;
				 dtmin *= 5.0;
				 break;
			 
		case 6: 
				 /* This is the fastest setting */
				 maxitr = (int)(maxitr * 0.3);
				 mktab = 1;
				 thetol *= 1.2;
				 dtmin *= 10.0;
				 break;
			 
	}
	dtmax = getdefdoub ("soil", "dtmax", dtmax, infilename, FALSE);
	mbck = getdefint ("soil", "mbck", mbck, infilename, FALSE);
	if (mbck == 1)
		ckcnv = mb_ckcnv;
	else
		ckcnv = th_ckcnv;
	mbalerr = getdefdoub ("soil", "mbalerr", mbalerr, infilename, FALSE);
	tm_mult = getdefdoub ("soil", "tm_mult", tm_mult, infilename, FALSE);
	dtmin = getdefdoub ("soil", "dtmin", dtmin, infilename, FALSE);
	thetol = getdefdoub ("soil", "thetol", thetol, infilename, FALSE);
	tm_mult = getdefint ("soil", "tm_mult", tm_mult, infilename, FALSE);
	maxitr = getdefint ("soil", "maxitr", maxitr, infilename, FALSE);
	noit = getdefint ("soil", "noit", noit, infilename, FALSE);
	solvemet = getdefint ("soil", "solvemet", solvemet, infilename, FALSE);

	/* Howmany steps to skip in output */
	outskip = getdefint ("soil", "outskip", outskip, infilename, FALSE);
	smddepth = getdefdoub ("soil", "smddepth", 0.0, infilename, FALSE);
	outskip = outskip <= 0 ? 1 : outskip;
	pondmx = getdefdoub ("soil", "pondmx", pondmx, infilename, FALSE);
	dodrain = getdefint ("drainage", "method", 0, infilename, FALSE);
	fieldcap = getdefdoub ("soil", "fieldcap", fieldcap, infilename, FALSE);
	/* All following parameters are needed */
	layers = getdefint ("soil", "layers", 0, infilename, TRUE);	/* number of soil layers */
	lbc = getdefint ("soil", "bottom", 0, infilename, TRUE);	/* lower boundary condition */
	initprof = getdefint ("soil", "initprof", 0, infilename, TRUE);	/* initial cond */
	/* dodrain:
	 * 0 = no drainage
	 * 1 = TOPOG (only flow at saturation)
	 * 2 = VAMPS always lateral flow
	 * */
	if (dodrain){
		slope = getdefdoub ("drainage", "slope", slope, infilename, TRUE);
		mqdra = (double *) ts_memory (NULL, layers * sizeof (double), progname);
		for (i = 0; i < layers; i++)
			mqdra[i] = 0.0;
	}

	allocall (layers);
	if (dodrain){
		tmpar = getdefar("drainage","exclude",NULL,infilename,&pts,FALSE);
		for (i=0; i< pts; i++)
			allowdrain[(int)tmpar[i]] = 0;
		free(tmpar);
	}

	switch (lbc){
		case 0:
			printcom ("Using bottom condition 0: Not tested !!.");
			Perror(progname,0,0,RCSid,"Bottom condition 0 (daily gw table)","NOT TESTED!!");
			get_data (getdefstr("ts","gwt","nothing",infilename,TRUE), "gwt",3);
			id.gwt = getsetbyname("gwt");
			if (data[id.gwt].points < steps){
				data[id.gwt].xy=resamp_a_to_b(data[id.gwt].xy,data[id.pre].xy,data[id.gwt].points,data[id.pre].points);
				data[id.gwt].points = data[id.pre].points;
			}
			break;
		case 1:
			printcom ("Using bottom condition 1: given qbo");
			get_data (getdefstr("ts","qbo","nothing",infilename,1), "qbo",3);
			id.qbo = getsetbyname("qbo");
			if (data[id.qbo].points < steps){
				data[id.qbo].xy=resamp_a_to_b(data[id.qbo].xy,data[id.pre].xy,data[id.qbo].points,data[id.pre].points);
				data[id.qbo].points = data[id.pre].points;
			}
			break;
		case 2:
			printcom ("Using bottom condition 2: Not tested!!.");
			Perror(progname,0,0,RCSid,"Bottom condition 2 (seepage/infilt)","NOT TESTED!!");
			break;
		case 3:
			printcom ("Using bottom condition 3: Not tested!!.");
			Perror(progname,0,0,RCSid,"Bottom condition 3 (Flux calculated as function of h)","NOT TESTED!!");
			break;
		case 4:
			printcom ("Using bottom condition 4: Fixed groundwater level.");
			get_data (getdefstr("ts","hea","nothing",infilename,TRUE), "hea",3);
			id.hea = getsetbyname("hea");
			if (data[id.hea].points < steps){
				data[id.hea].xy=resamp_a_to_b(data[id.hea].xy,data[id.pre].xy,data[id.hea].points,data[id.pre].points);
				data[id.hea].points = data[id.pre].points;
			}
			break;
		case 5:
			printcom ("Using bottom condition 5: No flow.");
			break;
		case 6:
			printcom ("Using bottom condition 6: Free drainage.");
			break;
		default:
			Perror (progname, 1,0, RCSid, "Unkown bottom boundary", "");
			break;
	}


	mktab = getdefint ("soil", "mktable", mktab, infilename, FALSE);
	if (mktab)
		estdmc = getdefint ("soil", "estdmc", estdmc, infilename, FALSE);
	tablesize = getdefint ("soil", "tablesize", tablesize, infilename, FALSE);
	dumptables = getdefint ("soil", "dumptables", dumptables, infilename, FALSE);

	if (iniinmem) /* some speedup */
		chk_only_mem = 1;
	for (i = 0; i < layers; i++)	/* Init each layer */
		initnode (i);
	/* Make link to soil pointer. This must be done now and not before
	   because of mem reallocation! */
	for (i = 0; i < layers; i++)
		node[i].sp = &sp[node[i].soiltype];

	chk_only_mem = 0;

	for (i = 0; i < layers; i++)
		if (node[i].sp->mktable)
			node[i].sp->method = 2;

	/* apply profle smoothing if nescessary */
	/*
	   smooth (getdefint ("soil", "smooth", 0, infilename, FALSE), 1);
	   smooth (getdefint ("soil", "smooth", 0, infilename, FALSE), 2);
	   smooth (getdefint ("soil", "smooth", 0, infilename, FALSE), 3);
	 */
	/* Print profiles in output file */
	fprintf (genoutfile, "ksat=");
	for (i = 0; i < layers; i++){/* Print ksat profile */
		fprintf (genoutfile, "%g ", node[i].sp->ksat);
	}
	fprintf (genoutfile, "\n");
	fprintf (genoutfile, "thetas=");
	for (i = 0; i < layers; i++){/* Print thetas profile */
		fprintf (genoutfile, "%g ", node[i].sp->thetas);
	}
	fprintf (genoutfile, "\n");
	fprintf (genoutfile, "residual_water=");
	for (i = 0; i < layers; i++){/* Print res profile */
		fprintf (genoutfile, "%g ", node[i].sp->residual_water);
	}
	fprintf (genoutfile, "\n");

	/*
	if ((id.inr = getsetbyname ("inr")) < 0){
		strcpy (s, getdefstr ("ts", "inr", "nothing", infilename, FALSE));
		if (strcmp (s, "nothing") == 0)
			Perror (progname, 1,0, RCSid, "Need interception data", "");
		get_data (s, "inr",steps);
		id.inr = getsetbyname ("inr");
	}

	if ((id.pev = getsetbyname ("pev")) < 0){
		strcpy (s, getdefstr ("ts", "pev", "nothing", infilename, FALSE));
		if (strcmp (s, "nothing") == 0)
			Perror (progname, 1,0, RCSid, "Need evaporation data", "");
		get_data (s, "pev",steps);
		id.pev = getsetbyname ("pev");
	}

	if ((id.ptr = getsetbyname ("ptr")) < 0){
		strcpy (s, getdefstr ("ts", "ptr", "nothing", infilename, FALSE));
		if (strcmp (s, "nothing") == 0)
			Perror (progname, 1,0, RCSid, "Need transpiration data", "");
		get_data (s, "ptr",steps);
		id.ptr = getsetbyname ("ptr");
	}
	*/

	/* evaporation reduction  def= no reduction */
	swredu = getdefint ("soil", "swredu", 0, infilename, FALSE);
	if (swredu){
		cofred = getdefdoub ("soil", "cofred", 0.0, infilename, TRUE);
	}

	/* blz 12 input */
	/* Root extraction stuff */
	/* First a time series is checked, otherwise the depth var is used */   
	strcpy (s, getdefstr ("ts", "rdp", "nothing", infilename, FALSE));
	if (strcmp (s, "nothing") != 0){
		get_data (s, "rdp",steps);
		id.rdp = getsetbyname ("rdp");
		data[id.rdp].xy=resamp_a_to_b(data[id.rdp].xy,data[id.pre].xy,data[id.rdp].points,data[id.pre].points);
		data[id.rdp].points = data[id.pre].points;
		/* Make negative */
		for (i=0;i<data[id.pre].points;i++)
			data[id.rdp].xy[i].y *= -1.0;
	}else{
		drootz = getdefdoub ("roots", "depth", 0.0, infilename, FALSE);
		tmpxy = (XY *) ts_memory (NULL,data[id.pre].points * sizeof(XY),
				progname);
		for (i=0;i<data[id.pre].points;i++){
			tmpxy[i].x=data[id.pre].xy[i].x;
			tmpxy[i].y=-1.0*drootz;
		}
		add_set(tmpxy,"rdp","rdp",data[id.pre].points,0,0);
		id.rdp = getsetbyname ("rdp");
	}


	for (i=0;i<data[id.pre].points;i++)
		if (data[id.rdp].xy[i].y != 0.0)
			are_roots++;


	if (are_roots){
		swsink = getdefint ("roots", "swsink", 0, infilename, TRUE);
		zronam = 0.0;			/* NO nonactive layers */
		swhypr = getdefint ("roots", "swhypr", 0, infilename, TRUE);
		swupfu = getdefint ("roots", "swupfu", 0, infilename, TRUE);
		if (swupfu == 1){
			cofsza = getdefdoub ("roots", "cofsza", 0.0, infilename, TRUE);
			cofszb = getdefdoub ("roots", "cofszb", 0.0, infilename, TRUE);
		}
		if (swupfu == 3){
			/* rootfrac does _not_ work with depth timeseries! */
			rootfrac = getdefar("roots","rootfrac",NULL,infilename,&pts,FALSE);
		}
		hlim1 = getdefdoub ("roots", "hlim1", 0.0, infilename, TRUE);
		hlim2u = getdefdoub ("roots", "hlim2u", 0.0, infilename, TRUE);
		hlim2l = getdefdoub ("roots", "hlim2l", 0.0, infilename, TRUE);
		hlim3h = getdefdoub ("roots", "hlim3h", 0.0, infilename, TRUE);
		hlim3l = getdefdoub ("roots", "hlim3l", 0.0, infilename, TRUE);
		if (swsink != 0)
			hlim3 = getdefdoub ("roots", "hlim3", 0.0, infilename, TRUE);
		hlim4 = getdefdoub ("roots", "hlim4", 0.0, infilename, TRUE);
	}

  return 0;
}

