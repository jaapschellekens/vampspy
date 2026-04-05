/* $Header: /home/schj/src/vamps_0.99g/src/soil/RCS/swatsoil.c,v 1.37 1999/01/06 12:13:01 schj Alpha $ */
/*  $RCSfile: swatsoil.c,v $
 *  $Author: schj $
 *  $Date: 1999/01/06 12:13:01 $ */

static char RCSid[] =
"$Id: swatsoil.c,v 1.37 1999/01/06 12:13:01 schj Alpha $";

#include <math.h>
#include <stdlib.h>
#include "vamps.h"
#include "deffile.h"
#include "swatsoil.h"
#include "soils.h"
#include "nrutil.h"
#ifdef HAVE_LIBPYTHON
#include "py_init.h"
#endif

int headc_method = 0; /* head calculation method. SWATR (0) or TOPOG (1) */
double estdmc = 0;		/* should DMC be estimate numerically? */
double fieldcap = -100.0;	/* Field capacity in cm */
double avgtheta;		/* average water content */
double SMD; 			/* Soil moisture deficit (cm) */
double smddepth;		/* depth to calculate SMD to */
int outskip = 1;		/* Skip steps in output (soilout) */
int soilverb = 0;		/* Seperate verbose */
int layers;			/* number of soil nodes */
int spnr = 0;			/* Number of soil types in mem */
soilparmt *sp = NULL;		/* array of soil structs */
node_t *node = NULL;		/* array of soil nodes */
int initprof = 0;		/* swinco in swatr see intial() */
int speed = 3;			/* ranges from 1 to 5*/
int mktab = 1;			/* make look-up table or not */

/* See src91/headcalc.f for source */
/* These are alle the new gloabal vars needed */
/* These are all in swatsoil.h */
double volsat;			/* water volume at saturation [cm] */
double dsoilp;			/* total depth of soil profile [cm] */
double volact;			/* actual water volume [cm] */
double volini;			/* initial water content [cm] */
double cqbot = 0.0;
double reltol = 0.001;
double abstol = 0.5;
int ftoph;
int daynr;
double *gwl;			/* length 2 */
double *basegw;			/* length 2 */
double *gw;			/* length a year 366, must be same as input */
int *sol_error;			/* length a year 366, must be same as input */
int *itter;			/* steps long */
double *q;			/* length layers +1  flow for each layer */
double *inq;			/* length layers +1 in inflow for each layer */
double aqave, aqamp, aqomeg, aqtamx, rimlay, cofqha, cofqhb;

int fllast = FALSE;             /* end of day? */
int maxitr = 80;	       	/* Max number of iterations */
int numbit;
int lbc = 0;			/* Lower bottom condition (was swbotb) */
double prec = 0.0;
double intc = 0.0;
double qdrtot = 0.0;
double	ptra = 0.0;
double  peva = 0.0;
double dhead;
double mdhead;
int flendd;
#ifdef MPORE
double *mp_theta; 		/* amount of water in pore */
double *mp_res; 		/* resistance entering soil matrix */
#endif
double *k;			/* Conductivity (length is layers +1!!) */
double *depth;			/* Depth of each layer (disnod in swatr) */
double *h;			/* head in each layer */
double *kgeom;			/* geo gemm van k */
double *diffmoist;		/* differential moisture cap. */
double *theta;			/* Theta at t */
double *howsat;			/* How saturated ( on a 0 to 1 scale) */
double *thetm1;			/* Theta one step back */
double *dz;			/* Thickness of layers size of compartement */
double *dzf;			/* delta z forward (z[i] - z[i-1]) */
double *dzc;			/* delta z central ((z[i+1] - z[i-1])/2) */
double *z;			/* Depth */
double *hm1;			/* Head one timestep back */
double qtop;			/* Discharge at top */
double *qrot;			/* Root extraction rate,determined in rootex.f*/
double **qdra;			/* 4x layers matrix */
double qbot;			/* bottom q?? */
double *mqdra;

/* stuff for balance and rootex */
/* double osmota, osmotb; */

/* These are local to swatsoil.c */
void readvangenu (char *secname, int spnr, int layer);
void readclapp (char *secname, int spnr, int layer);
#ifdef HAVE_LIBPYTHON
void readuserfpy (char *secname, int spnr, int layer);
#endif
void read_soiltable (char *secname, int spnr, int layer);
extern void calcgwl ();
extern void watcon ();
extern void fluxes ();
extern void setzero ();
extern void output (int tstep);
extern void soilout (int tstep);
extern int headcalc (int i, double *t);
extern double timestep (double t, double e_t,double *dt, double *dtm1);
extern void bocobot (int pt);
extern double det_hatm(int i);
extern double reduceva (int swreduc);
extern void rootex (int pt, double drz);
void initial ();
extern int getparams (char *infilename);
int readstype (char *section, char *fname, int layer);




/*C:initial
 *@void initial()
 *
 * Sets initial values
 *
 * Return:  nothing*/
void
initial ()
{
	int i;
	double *tmparray,tt;
	int pts;

	/* Calculate depth of soil profile, make dz negative */
	dsoilp = 0.0;
	for (i = 0; i < layers; i++){
		dsoilp += dz[i];
		dz[i] = -1.0 * fabs (dz[i]);
	}

	z[0] = 0.5 * dz[0];
	depth[0] = z[0];
	dzf[0] = dz[0];
	for (i = 1; i < layers; i++){
		z[i] = z[i - 1] + (0.5 * (dz[i - 1] + dz[i]));
		depth[i] = z[i] - z[i - 1];
	}
	depth[layers] = 0.5 * dz[layers - 1];
	/* get dzf and dzc for TOPOG solution */
	dzc[0] = dz[0];
	dzf[0] = z[1] - z[0];
	for (i =1; i < layers; i++){
		if (i < layers -1)
			dzc[i] = (z[i+1] - z[i-1]) * 0.5;
		dzf[i] = z[i] - z[i-1];
	}

	/* Get inital conditions via one of three methods:
	   0 = Given theta profile
	   1 = given pressure head profile
	   2 = pressure head profile is calculated
	 */
	switch (initprof){
		case 0:	/* Water content profile */
			tmparray = getdefar("soil","theta_initial",NULL,infilename,&pts,TRUE);
			if (pts != layers){
				showit("swatsoil",ERR,"Point not layers (theta_initial)",1,verbose);
				Perror(progname,1,0,RCSid,"Theta initial array length not correct","");
			}
			for (i = 0; i < layers; i++){
				node[i].theta_initial = tmparray[i];
				theta[i] = node[i].theta_initial;
				h[i] = node[i].sp->t2h (node[i].soiltype, theta[i], depth[i], i);
			}
			free(tmparray);
			if (lbc == 4)	/* Change h for last layer */
				h[layers - 1] = _getval(&data[id.hea],t);
			theta[layers - 1] = node[layers-1].sp->h2t (node[layers - 1].soiltype, h[layers - 1], layers-1);

			break;
		case 1:	/* pressure head profile */
			tmparray = getdefar("soil","h_initial",NULL,infilename,&pts,TRUE);
			if (pts != layers){
				showit("swatsoil",ERR,"Point not layers (h_initial)",1,verbose);
				Perror(progname,1,0,RCSid,"H initial array length not correct","");
			}
			for (i = 0; i < layers; i++){
				node[i].h_initial = tmparray[i];
				h[i] = node[i].h_initial;
				theta[i] = node[i].sp->h2t (node[i].soiltype, h[i], i);
			}
			free(tmparray);
			break;
		case 2:	/* calculate pressure head profile, need gw level */
			if (lbc == 0)
				gwl[0] = gw[0];
			else
				gwl[0] = -fabs (getdefdoub ("soil", "gw_initial", 0.0, infilename, TRUE));

			for (i = 0; i < layers; i++){
				h[i] = gwl[0] - z[i];
				theta[i] = node[i].sp->h2t (node[i].soiltype, h[i], i);
			}
			break;
		case 3: /* uniform head */
			tt = getdefdoub ("soil", "h_initial", 0.0, infilename, TRUE);
			fprintf(stderr,"%f\n",tt);
			for (i = 0; i < layers; i++){
				node[i].h_initial = tt;
				h[i] = tt;
				theta[i] = node[i].sp->h2t (node[i].soiltype, h[i], i);
			}
			break;
		default:
			Perror (progname, 1,0, RCSid, "Initial profile method not known.", " initprof >3");
			break;
	}

	watcon ();	/* Calculate actual water content */
	volini = volact;

	for (i = 0; i < layers; i++)
		volsat -= dz[i] * node[i].sp->thetas;

  /* Save initial moisture content and pressure heads */
	for (i = 0; i < layers; i++){
		thetm1[i] = theta[i];
		hm1[i] = h[i];
	}

	/* hydraulic conductivities, diffrential moisture capacities
	   and geometrical mean hydraulic conductivities for each layer */
	diffmoist[0] = node[0].sp->h2dmc (node[0].soiltype, h[0], 0);
	k[0] = node[0].sp->t2k (node[0].soiltype, theta[0], 0);
	kgeom[0] = 0.5 * (node[0].sp->ksat + k[0]);
	for (i = 1; i < layers; i++){
		diffmoist[i] = node[i].sp->h2dmc (node[i].soiltype, h[i], i);
		k[i] = node[i].sp->t2k (node[i].soiltype, theta[i], i);
		kgeom[i] = MKKGEOM(i);
	}
	kgeom[layers] = k[layers - 1];

	/* This comes from evapotra.f */
	/* Calculate pressure head of the atmosphere */
	hatm = -2.75E5; /* CHECK THIS !!!*/
}

static int rlay = 0;

/*+Name: initnode
 *
 * Prototype:  void initnode(int layer)
 *
 * Description:  reads init values for layer layer
 *
 * Returns:  nothing+*/
void
initnode (int layer)
{
  char s[1024];
  int exitonerror = 1;
  void readtable ();

  sprintf (s, "layer_%d", layer);
  /* check if same as above */
  if (!issection (s, infilename) && layer != 0){
	  node[layer].as_above = TRUE;
	  exitonerror = 0;
  }else{
	  node[layer].as_above = FALSE;
	  exitonerror = 1;
	  rlay++;
  }

  node[layer].rlay = rlay;

  /* Geth depth info */
  node[layer].thickness = getdefdoub (s, "thickness", layer > 0 ? node[layer - 1].thickness : 0.0, infilename, exitonerror);
  dz[layer] = node[layer].thickness;
  strcpy(node[layer].soilsection,
	 getdefstr(s,"soilsection", layer > 0 ? 
		   node[layer - 1].soilsection : "Unknown section",
		   infilename,exitonerror));

  /* Link to existing soil type or read a new one */
  if (node[layer].as_above)
      node[layer].soiltype = node[layer -1].soiltype;
  else
      node[layer].soiltype = readstype(node[layer].soilsection, infilename, layer);
}


/*+Name: read_soiltable
 *
 *  Prototype:  void read_soiltable(char *secname, int spnr)
 *
 *  Description:  reads info from secname and calls readtablefile
 *
 *  Returns:  nothing+*/
void
read_soiltable (char *secname, int spnr, int layer)
{
  FILE *thef;
  char tabfname[1024];
  int exitonerror=1;
  int ftype = 1;		/* file type default = TOPOG */
  extern void readtablefile (FILE * stream, int layer, int type);

  if ((ftype = getdefint (secname, "tablefiletype", ftype, infilename, FALSE)) != 1)
    {				/* only the TOPOG format has this information in the file */
      exitonerror = TRUE;
    }
  sp[spnr].thetas = getdefdoub (secname, "thetas", 0.0, infilename, exitonerror);
  sp[spnr].ksat = getdefdoub (secname, "ksat", 0.0, infilename, exitonerror);
  sp[spnr].mktable = mktab;	/* set this to 1 as well */

  
 
  /* First create an empty table */
  strcpy (tabfname, getdefstr (secname, "tablefile", NULL, infilename, TRUE));
  if ((thef = fopen (tabfname, "r")) == NULL)
      Perror (progname, 1,1, RCSid, "Soil table file open failed:", tabfname);
  readtablefile (thef, spnr, ftype);
  (void)fclose (thef);		/* close the table file */
    
   /* Set the function pointers to what we want */
   sp[spnr].t2k=t2k_2;
   sp[spnr].h2dmc=h2dmc_2;
   sp[spnr].t2h=t2h_2;
   sp[spnr].h2t=h2t_2;
}

void
readclapp (char *secname, int spnr, int layer)
{
	int exitonerror = 1;
	extern void filltables (int, int, int);

	sp[spnr].thetas = getdefdoub (secname, "thetas", 0.0, infilename, exitonerror);
	sp[spnr].ksat = getdefdoub (secname, "ksat", 0.0, infilename, exitonerror);
	sp[spnr].b = getdefdoub (secname, "b", 0.0, infilename, exitonerror);
	sp[spnr].n = 2.0 + 3.0/sp[spnr].b;
	sp[spnr].psisat = getdefdoub (secname, "psisat", 0.0, infilename, exitonerror);

	/* Set function pointers to the right functions */
	sp[spnr].t2k=t2k_0;
	sp[spnr].h2dmc=h2dmc_0;
	sp[spnr].t2h=t2h_0;
	sp[spnr].h2t=h2t_0;
	sp[spnr].h2k=h2k_0;
	sp[spnr].h2dkdp=h2dkdp_0;
	sp[spnr].h2u=h2u_0;

   /* mktable for layer 0 is set in getparm */
	if ((sp[spnr].mktable = mktab)){
		filltables (spnr, layer, estdmc);
		/* ReSet function pointers to the right functions
		   (detbytable = 3) */
		sp[spnr].t2k=t2k_2;
		sp[spnr].h2dmc=h2dmc_2;
		sp[spnr].t2h=t2h_2;
		sp[spnr].h2t=h2t_2;
		sp[spnr].h2k=h2k_2;
		sp[spnr].h2dkdp=h2dkdp_2;
		sp[spnr].h2u=h2u_2;
	}
}


#ifdef HAVE_LIBPYTHON
void
readuserfpy (char *secname, int spnr, int layer)
{
	int exitonerror = 1;
	extern void filltables (int, int, int);

	sp[spnr].thetas = getdefdoub (secname, "thetas", 0.0, infilename, exitonerror);
	sp[spnr].ksat   = getdefdoub (secname, "ksat",   0.0, infilename, exitonerror);

	/* Call Python getspars(secname, spnr) so user script can read its params */
	py_getspars_f(secname, spnr);

	/* Point function pointers at Python-backed wrappers */
	sp[spnr].t2k     = py_t2k;
	sp[spnr].h2dmc   = py_h2dmc;
	sp[spnr].t2h     = py_t2h;
	sp[spnr].h2t     = py_h2t;
	sp[spnr].h2k     = py_h2k;
	sp[spnr].h2dkdp  = py_h2dkdp;
	sp[spnr].h2u     = py_h2u;

	/* If mktable, build lookup table now then switch to table functions */
	if ((sp[spnr].mktable = mktab)) {
		filltables (spnr, layer, estdmc);
		sp[spnr].t2k    = t2k_2;
		sp[spnr].h2dmc  = h2dmc_2;
		sp[spnr].t2h    = t2h_2;
		sp[spnr].h2t    = h2t_2;
		sp[spnr].h2k    = h2k_2;
		sp[spnr].h2dkdp = h2dkdp_2;
		sp[spnr].h2u    = h2u_2;
	}
}
#endif

static double ttl;



/*C:readvangenu
 *@void readvangenu(char *secname, int spnr, int layer)
 *
 *  Description:  read van gunuchten stuff for layer layer and soilinfo spnr
 *
 *  Returns:  nothing*/
void
readvangenu (char *secname, int spnr, int layer)
{
  int exitonerror=1;
  extern void filltables (int, int, int);

  sp[spnr].thetas = getdefdoub (secname, "thetas", 0.0, infilename, exitonerror);
  sp[spnr].ksat = getdefdoub (secname, "ksat", 0.0, infilename, exitonerror);
  sp[spnr].n = getdefdoub (secname, "n",  0.0, infilename, exitonerror);
  ttl = getdefdoub (secname, "l", 0.5, infilename, 0);
  sp[spnr].m = 1.0 - (1.0 / sp[spnr].n);
  sp[spnr].l = sp[spnr].m * (ttl + 2.0);
  sp[spnr].alpha = getdefdoub (secname, "alpha", 0.0, infilename, exitonerror);

   sp[spnr].t2k=t2k_1;
   sp[spnr].h2dmc=h2dmc_1;
   sp[spnr].t2h=t2h_1;
   sp[spnr].h2t=h2t_1;
   sp[spnr].h2k=h2k_1;
   sp[spnr].h2dkdp=h2dkdp_1;
   sp[spnr].h2u=h2u_1;
   /* mktable for layer 0 is set in getparm THIS SHOULD CHANGE*/
   if ((sp[spnr].mktable = mktab))
   {			/* First create an empty table */
	   filltables (spnr, layer, estdmc);
	   /* set pointter to table lookup functions */
	   sp[spnr].t2k=t2k_2;
	   sp[spnr].h2dmc=h2dmc_2;
	   sp[spnr].t2h=t2h_2;
	   sp[spnr].h2t=h2t_2;
	   sp[spnr].h2k=h2k_2;
	   sp[spnr].h2dkdp=h2dkdp_2;
	   sp[spnr].h2u=h2u_2;
   }
}



/*+Name: presoil
 *
 *  Prototype:  void presoil(void)
 *
 *  Description:  initializes soil stuff
 *
 *  Returns:  nothing+*/
static int firsttime=1;

void reset_presoil(void) { firsttime = 1; }

void
presoil (void)
{
	int i;
	extern void mkscratch ();

	if (firsttime){
		soilverb = getdefint ("soil", "verbose", verbose, infilename, FALSE);
		(void) showit ("swatsoil",MESG,"getting initial values",2,soilverb);
		getparams (infilename);
		mkscratch ();
	}
	/* Make link to soil pointer. This must be done now and not before
	   because of mem reallocation! */
	for (i = 0; i < layers; i++)
		node[i].sp = &sp[node[i].soiltype];

	initial ();			/* This was (about) initial.f */
	setzero ();
	fluxes ();

	/* Print this in output file  we ar in the initial section */
	printint ("layers", layers);
	printcom ("initial water content of the profile");
	printfl ("volini", volini);
	printfl ("volsat", volsat);
	printar ("z", z, layers);
	printar ("dz", dz, layers);
	printar ("theta", theta, layers);
	printar ("k", k, layers);
	printar ("h", h, layers);
	printfl ("outskip",outskip);
	fprintf (genoutfile, "as_above=");
	for (i = 0; i < layers; i++)
		fprintf (genoutfile, "%d ", node[i].as_above);
	fprintf (genoutfile, "\n");

	/* set initial time  to first record in precip file (experimental) */
	dtm1 = dt;
	if (steps > data[id.pre].points){
		fprintf(stderr,"P = %d, steps = %d id.pre = %d\n",data[id.pre].points,steps,id.pre);
		Perror (progname, 1,0, RCSid, "more steps then P values !!", "");
	}
	if (firsttime)
		initprogress ();

	firsttime = 0;
}

/*C:tstep_soil
 *@ void tstep_soil (int i,double e_t, double t_prec, double t_intc,double t_ptra,
 *@			double t_peva)
 *
 *  Description: Calculated theta etc for end of time in record  i
 *  of the precipitation file. This is the core of the soil module
 *
 *  Returns: nothing*/
void
tstep_soil (int i, double e_t,double t_prec, double t_intc, double t_ptra, double t_peva)
{
	int instep_i = 0;

	setzero ();	/* Set daily stuff to zero */
	daynr = i + 1; /* Calculate day number*/
	flendd = FALSE;	/* end of timestep set in timestep.c */
	/* Soil evaporation */
	prec = t_prec;
	intc = t_intc;
	ptra = t_ptra;
	peva = t_peva;
	hatm = det_hatm(i);
	reva = reduceva (swredu); /* reduce soil evaporation*/
	do{ /* Loop _within_ the given timestep */
		instep_i++; /* number of steps within a timestep */
		if (lbc == 2 || lbc == 3 /*|| swdrai == 1*/) 
			calcgwl();
		tm1 = t;
		(void)timestep (t,e_t,&dt,&dtm1);	/* Calculate new timestep */
		bocobot (i);		/* Bottom boundary conditions */
		if (dodrain)		/* Lateral drainage */
			drainage (dodrain);
		qtop = bocotop (&kgeom[0],&ftoph);/* Top boundary conditions */
#ifdef MPORE
		mpore(i); /* flow and water exchange with matrix */
#endif
#ifdef NHC
		nheadcalc(n,&t,&dt);
#else
		rootex (i,data[id.rdp].xy[i].y);/* root extraction */
		sol_error[i] += headcalc (i, &t);
#endif
		watcon ();
		fluxes ();
		integral (i);
		dethowsat ();
		if (soilverb > 3)
			showprogress (i + 1 - startpos);
	}while (!flendd);		/* end of timestep reached, set in timestep */
  	/* cummulative amounts  */
	cumprec += prec * thiststep;
  	cumintc += intc * thiststep;
  	cumeva += reva * thiststep;
  	cumtra += ptra * thiststep;
	/* these are used within timestep, they have to be reset to
	   prevent rounding errors from becoming to large!*/
	_cumprec = cumprec;
	_cumintc = cumintc;
	_cumeva = cumeva;
	_cumtra = cumtra;

	itter[i]/=instep_i;
	instep_i = 0;
	calcgwl ();
	avgtheta= detavgtheta(layers); 
	SMD = smd(smddepth == 0.0 ? data[id.rdp].xy[i].y : smddepth,fieldcap);
	/* Updat the data-sets */
	data[id.qbo].xy[i].y = qbot;
	data[id.qbo].xy[i].x = data[id.pre].xy[i].x;
	data[id.vol].xy[i].y = volact;
	data[id.vol].xy[i].x = data[id.pre].xy[i].x;
	data[id.avt].xy[i].y = avgtheta;
	data[id.avt].xy[i].x = data[id.pre].xy[i].x;

	if (!(i % outskip))
		soilout (i);/* Produce daily output to stdout or named file(genoutfile) */

	if (soilverb > 0)
		showprogress (i + 1 - startpos);
}

/*C:postsoil
 *
 *  Prototype:  void postsoil()
 * 
 *  Description:  Cleans up after presoil and tstep_soil. Frees all mem
 *
 *  Returns:  nothing+*/
void
postsoil ()
{
	int i, j;
	extern void freescratch ();

	setzero();
	for (i = 0; i< spnr; i++){
		if (sp[i].mktable == 1){
			for (j = 0; j < 4; j++) {
				if (sp[i].tab[j].x)
					free (sp[i].tab[j].x);
				if (sp[i].tab[j].y)
					free (sp[i].tab[j].y);
			}
			if (sp[i].tab)
				free (sp[i].tab);
		}
	}
	spnr = 0;
	firsttime = 1;
	free(sp);
	free (node);

	free_dvector(theta, 0, layers -1);
	free_dvector(howsat, 0, layers -1);
	free_dvector(diffmoist, 0, layers -1);
	free_dvector(thetm1, 0, layers -1);
	free_dvector(dz, 0, layers -1);
	free_dvector(dzc, 0, layers -1);
	free_dvector(dzf, 0, layers -1);
	free_dvector(z, 0, layers -1);
	free_dvector(h, 0, layers -1);
	free_dvector(hm1, 0, layers -1);
	free_dvector(qrot, 0, layers -1);
	free_dvector(depth, 0, layers);
	free_dvector(k, 0, layers -1);
	free_ivector(allowdrain, 0, layers -1);
	free_dvector(kgeom, 0, layers);
	free_dvector(q, 0, layers);
	free_dvector(inq, 0, layers);

	free_dmatrix (qdra, 0,layers,0,3);
	free_dvector (gwl,0,steps + 1);
	free_dvector (basegw,0,1);
	free_ivector (sol_error,0,steps+1);
	free_ivector (itter,0,steps+1);

	sp = NULL;
	node = NULL;
	freescratch ();
#ifdef NR_MEM_DEBUG
	prmeminfo();
#endif
}

/*C:readstype 
 *@ int readstype (char *section, char *fname, int layer)
 *
 *  Allocates new sp if needed and reads soil parameters
 *  from section and calls one of the method specific read functions
 *
 *  Returns: index of sp linked to node */
int 
readstype (char *secname, char *fname, int layer)

{
	int exitonerror = 1;

	spnr++; /* allocate new soil type and inc array index */
	sp = (soilparmt *) ts_memory ((void *)sp, spnr * sizeof (soilparmt), progname);

	exitonerror = FALSE;

	strcpy (sp[spnr -1].description, 
			getdefstr (secname, "description", "NO DESCRIPTION", fname, FALSE));
	sp[spnr -1].residual_water = 
		getdefdoub (secname, "theta_residual", 0.0, fname, TRUE);

	exitonerror = layer > 0 ? FALSE : TRUE;

	/* Get k_hor/k_ver if drainage is used */
	if (dodrain)
		sp[spnr - 1].kh_kv = getdefdoub (secname, "kh/kv", 1.0, fname, FALSE);

	sp[spnr -1].method = getdefint (secname, "method", 1, infilename, TRUE);
	switch (sp[spnr -1].method){
		case 0:	/* Use clapp & Hornberger stuff */
			readclapp (secname, spnr -1, layer);
			break;
		case 1:
			readvangenu (secname, spnr -1, layer);
			break;
		case 2:	/* This set in readvangenu as well !!! */
			Perror (progname, 1,0, RCSid, "Method obsolute, use mktable in soil section", "");
			break;
		case 3:	/* Dtermine parameters from marquardt, use stand-alone version ! */
			Perror (progname, 1,0, RCSid, "This function no longer updated (k vs h method = 3)", "Use stand-alone version, or _soil program");
			break;
		case 4:	/* Read input from TOPOG _soil tables (experimental) */
			read_soiltable (secname, spnr -1, layer);
			break;
#ifdef HAVE_LIBPYTHON
		case 5:	/* Use Python user-defined functions */
			readuserfpy (secname, spnr -1, layer);
			break;
#endif
		default:
			Perror (progname, 1,0, RCSid, "Method not known:", "k vs h method");
			break;
	}

  return spnr -1;
}
