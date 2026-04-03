/* $Header: /home/schj/src/vamps_0.99g/src/maq.lib/RCS/marquard.c,v 1.7 1999/01/06 12:13:01 schj Alpha $ */
/*  $RCSfile: marquard.c,v $
 *  $Author: schj $
 *  $Date: 1999/01/06 12:13:01 $
 */
 
#include "marquard.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>


#ifndef DBL_EPSILON /* Should be in float.h */
#define DBL_EPSILON 1.0E-12
#endif

double *inp = NULL;
double *out = NULL;
double **jjinv = NULL;
double *par = NULL;
int *fixed = NULL;
double *rv = NULL;
char	funcstr[1024]="nothing";
int	maqverb=1;
double diffac = 1.01;
int	it; /* Number of iterations */
double  fpar;
int maq_n;
int maq_m;
int maqrun();


static void 
Jacobian(int m,int n,double *jpar,double *rv,double **jac)
{
  int i, j;
  double d, p;
  double *rvcopy;

  rvcopy=(double *)m_memory((void *)NULL,(m+1)*sizeof(double),PROGNAME);
  for (i=1;i<=m;i++) /* Make copy of array */
  	rvcopy[i]=rv[i];

  for (j = 1; j <= n; j++) {
    if (!fixed[j]){
    d = jpar[j] * diffac;
    p = jpar[j];
    jpar[j] = d;
    }else{
    d = jpar[j];
    p = jpar[j];
    jpar[j] = d;
    }
    if (Funct(m, n, jpar, rv) != 0)
      (void)fprintf(stderr,"%s: function evaluation error\n",PROGNAME);
    jpar[j] = p;
    for (i = 1; i <= m; i++)
    if (!fixed[j])
      jac[i][j - 1] = (rv[i] - rvcopy[i]) / (d - p);
    else
      jac[i][j - 1] = 0.0;
  }
  for (i=1;i<=m;i++) /* restore original */
  	rv[i]=rvcopy[i];

  free(rvcopy);
}



static void HshReaBid(double **a,int m,int n,double *d,double *b,double *em)
{
  int i, j, i1, k, norm;
  double machtol, w, s, f, g, h, tm, TEMP;

  norm = 0.0;
  for (i = 1; i <= m; i++) {
    w = 0.0;
    for (j = 0; j < n; j++)
      w = fabs(a[i][j]) + w;
    if (w > norm)
      norm = w;
  }
  machtol = em[0] * norm;
  em[1] = norm;
  for (i = 1; i <= n; i++) {
    i1 = i + 1;
    s = 0.0;
    for (k = i1; k <= m; k++) {
      TEMP = a[k][i - 1];
      s = TEMP * TEMP + s;
    }
    if (s < machtol)
      d[i] = a[i][i - 1];
    else {
      f = a[i][i - 1];
      s = f * f + s;
      if (f < 0)
	g = sqrt(s);
      else
	g = -sqrt(s);
      d[i] = g;
      h = f * g - s;
      a[i][i - 1] = f - g;
      for (j = i1 - 1; j < n; j++) {
	tm = 0.0;
	for (k = i; k <= m; k++)
	  tm = a[k][i - 1] * a[k][j] + tm;
	tm /= h;
	for (norm = i; norm <= m; norm++)
	  a[norm][j] += a[norm][i - 1] * tm;
      }
    }
    if (i < n) {
      s = 0.0;
      for (k = i1; k < n; k++) {
	TEMP = a[i][k];
	s = TEMP * TEMP + s;
      }
      if (s < machtol)
	b[i] = a[i][i1 - 1];
      else {
	f = a[i][i1 - 1];
	s = f * f + s;
	if (f < 0)
	  g = sqrt(s);
	else
	  g = -sqrt(s);
	b[i] = g;
	h = f * g - s;
	a[i][i1 - 1] = f - g;
	for (j = i1; j <= m; j++) {
	  tm = 0.0;
	  for (k = i1 - 1; k < n; k++)
	    tm = a[i][k] * a[j][k] + tm;
	  tm /= h;
	  for (norm = i1 - 1; norm < n; norm++)
	    a[j][norm] += a[i][norm] * tm;
	}
      }
    }
  }
}  /*HshReaBid*/


static void PstTfmMat(double **a,int n,double **v,double *b)
{
  int i, i1, j, k, l;
  double h, tm;

  i1 = n;
  v[n][n - 1] = 1.0;
  for (i = n - 1; i >= 1; i--) {
    h = b[i] * a[i][i1 - 1];
    if (h < 0) {
      for (j = i1; j <= n; j++)
	v[j][i - 1] = a[i][j - 1] / h;
      for (j = i1 - 1; j < n; j++) {
	tm = 0.0;
	for (k = i1; k <= n; k++)
	  tm = a[i][k - 1] * v[k][j] + tm;
	for (l = i1; l <= n; l++)
	  v[l][j] += v[l][i - 1] * tm;
      }
    }
    for (j = i1; j <= n; j++) {
      v[i][j - 1] = 0.0;
      v[j][i - 1] = 0.0;
    }
    v[i][i - 1] = 1.0;
    i1 = i;
  }
}  /*PstTfmMat*/


static void PreTfmMat(double **a,int m,int n,double *d)
{
  int i, i1, j, k, l;
  double g, h, tm;

  for (i = n; i >= 1; i--) {
    i1 = i + 1;
    g = d[i];
    h = g * a[i][i - 1];
    for (j = i1 - 1; j < n; j++)
      a[i][j] = 0.0;
    if (h < 0) {
      for (j = i1 - 1; j < n; j++) {
	tm = 0.0;
	for (k = i1; k <= m; k++)
	  tm = a[k][i - 1] * a[k][j] + tm;
	tm /= h;
	for (l = i; l <= m; l++)
	  a[l][j] += a[l][i - 1] * tm;
      }
      for (j = i; j <= m; j++)
	a[j][i - 1] /= g;
    } else {
      for (j = i; j <= m; j++)
	a[j][i - 1] = 0.0;
    }
    a[i][i - 1]++;
  }
}  /*PreTfmMat*/


static int QriSngValDecBid(double *d,double *b,int m,int n,double **a,double **v,double *em)
{
  int n0, n1, k, k1, i, i1, count, max, rnk, l;
  double tol, bmax, z, x, y, g, h, f, c, s, min, u, w;

  tol = em[2] * em[1];
  count = 0;
  bmax = 0.0;
  max = (int)floor(em[4] + 0.5);
  min = em[6];
  rnk = n;
  n0 = n;
_Lgoin:
  k = n;
  n1 = n - 1;
_Lnext:
  k--;
  if (k > 0) {
    if (fabs(b[k]) >= tol) {
      if (fabs(d[k]) >= tol)
	goto _Lnext;
      c = 0.0;
      s = 1.0;
      for (i = k; i <= n1; i++) {
	f = s * b[i];
	b[i] = c * b[i];
	i1 = i + 1;
	if (fabs(f) < tol)
	  goto _Lneglect;
	g = d[i1];
	h = sqrt(f * f + g * g);
	d[i1] = h;
	c = g / h;
	s = -(f / h);
	for (l = 1; l <= m; l++) {
	  u = a[l][k - 1];
	  w = a[l][i1 - 1];
	  a[l][k - 1] = u * c + w * s;
	  a[l][i1 - 1] = w * c - u * s;
	}
      }
_Lneglect: ;
    } else if (fabs(b[k]) > bmax)
      bmax = fabs(b[k]);
  }
  if (k == n1) {
    if (d[n] < 0) {
      d[n] = -d[n];
      for (i = 1; i <= n0; i++)
	v[i][n - 1] = -v[i][n - 1];
    }
    if (d[n] <= min)
      rnk--;
    n = n1;
  } else {
    count++;
    if (count > max)
      goto _Leind;
    k1 = k + 1;
    z = d[n];
    x = d[k1];
    y = d[n1];
    if (n1 == 1)
      g = 0.0;
    else
      g = b[n1 - 1];
    h = b[n1];
    f = ((y - z) * (y + z) + (g - h) * (g + h)) / (2 * h * y);
    g = sqrt(f * f + 1);
    if (f < 0)
      u = f - g;
    else
      u = f + g;
    f = ((x - z) * (x + z) + h * (y / u - h)) / x;
    c = 1.0;
    s = 1.0;
    for (i = k1; i < n; i++) {
      i1 = i;
      g = b[i1];
      y = d[i + 1];
      h = s * g;
      g = c * g;
      z = sqrt(f * f + h * h);
      c = f / z;
      s = h / z;
      if (i1 != k1)
	b[i1 - 1] = z;
      f = x * c + g * s;
      g = g * c - x * s;
      h = y * s;
      y *= c;
      for (l = 1; l <= n0; l++) {
	u = v[l][i1 - 1];
	w = v[l][i];
	v[l][i1 - 1] = u * c + w * s;
	v[l][i] = w * c - u * s;
      }
      z = sqrt(f * f + h * h);
      d[i1] = z;
      c = f / z;
      s = h / z;
      f = c * g + s * y;
      x = c * y - s * g;
      for (l = 1; l <= m; l++) {
	u = a[l][i1 - 1];
	w = a[l][i];
	a[l][i1 - 1] = u * c + w * s;
	a[l][i] = w * c - u * s;
      }
    }
    b[n1] = f;
    d[n] = x;
  }
  if (n > 0)
    goto _Lgoin;
_Leind:
  em[3] = bmax;
  em[5] = count;
  em[7] = rnk;
  return n;
}  /*QriSngValDecBid*/


static int QriSngValDec(double **a,int m,int n,double *val,double **v,double *em)
{
  double *b;
  int i;
 
  b=(double *)m_memory((void *)NULL,(1+n)*sizeof(double),PROGNAME);
  HshReaBid(a, m, n, val, b, em);
  PstTfmMat(a, n, v, b);
  PreTfmMat(a, m, n, val);
  i=(QriSngValDecBid(val, b, m, n, a, v, em));

  free(b);
  return  i;
}


/* Local variables for Marquardt: */
struct LOC_Marquardt {
  int m, n;
  double *par;
} ;

void PrintPar(LINK)
struct LOC_Marquardt *LINK;
{
  int i, FORLIM;

  FORLIM = LINK->n;
  if (it == 0){
	  fprintf(stderr,"\n---------------------------------------------------------\n");
	  fprintf(stderr,"iteration\tresidual");
	  for (i =1; i <=FORLIM;i++)
		  fprintf(stderr,"\tpar[%d]",i);
	  fprintf(stderr,"\n---------------------------------------------------------\n");
  }
  fprintf(stderr,"%d\t\t%f\t",it,sqrt(fpar));
  for (i = 1; i <= FORLIM; i++){
    if (!fixed[i])
    fprintf(stderr,"%f\t", LINK->par[i]);
    else
    fprintf(stderr,"*%f\t", LINK->par[i]);
  }
  fprintf(stderr,"\n");
}

double VecVecN(a, b, LINK)
double *a, *b;
struct LOC_Marquardt *LINK;
{
  int k;
  double s;
  int FORLIM;

  s = 0.0;
  FORLIM = LINK->n;
  for (k = 1; k <= FORLIM; k++)
    s = a[k] * b[k] + s;
  return s;
}

double VecVecM(a, b, LINK)
double *a, *b;
struct LOC_Marquardt *LINK;
{
  int k;
  double s;
  int FORLIM;

  s = 0.0;
  FORLIM = LINK->m;
  for (k = 1; k <= FORLIM; k++)
    s = a[k] * b[k] + s;
  return s;
}

/*-
 * int Marquardt(int m, int n, double *par, double *g, double **v,
 *      double *inp, double *out)
 * 	m	number of points
 *	n	number of parameters
 *	par	array with parameters (n+1)
 *	g	array containing residuals (m+1)
 *	v	matrix (m+1*n+1)
 *	inp	input parameters (array 8)
 *	out	output (array 8)
 */

int Marquardt(int m_,int n_,double *par_,double *g,double **v,double *inp,double *out)
{
  struct LOC_Marquardt V;
  int maxfe, fe, i, j, k, err;
  double s, vv, ww, w, mu, res,  fparpres, lambdamin, pw,
	 reltolres, abstolres;
  double lambda=0.0;
  double *em;
  double *val, *b, *bb, *parpres;
  double **jac;
  double TEMP;
  int p;
  int FORLIM, FORLIM1, FORLIM2;

  /* Allocate memory for arrays */
  em=(double *)m_memory((void *)NULL,8*sizeof(double),PROGNAME);
  val=(double *)m_memory((void *)NULL,(1+n_)*sizeof(double),PROGNAME);
  b=(double *)m_memory((void *)NULL,(n_+1)*sizeof(double),PROGNAME);
  bb=(double *)m_memory((void *)NULL,(1+n_)*sizeof(double),PROGNAME);
  parpres=(double *)m_memory((void *)NULL,(1 + n_)*sizeof(double),PROGNAME);
  jac=m_matr(m_+1,n_+1);
  V.par = par_;
  V.m = m_; V.n = n_;
  w = 0.5;
  mu = 0.01;
  if (inp[6] < 1e-7)
    ww = 1e-8;
  else
    ww = 0.1 * inp[6];
  vv = 10.0;
  em[0] = inp[0];
  em[2] = inp[0];
  em[6] = inp[0];
  em[4] = n_ * 10.0;
  reltolres = inp[3];
  TEMP = inp[4];
  abstolres = TEMP * TEMP;
  maxfe = (int)floor(inp[5] + 0.5);
  err = 0;
  fe = 1;
  it = 1;
  p = 0;
  fpar = 0.0;
  res = 0.0;
  pw = log(ww * inp[0]) / -2.3;
  if (Funct(m_, n_, par_, g) != 0) {
    err = 3;
    goto _Lescape;
  }
  fpar = VecVecM(g, g, &V);
  out[3] = sqrt(fpar);
  it = 0;
  if (maqverb)
	PrintPar(&V);
  do {
    it++;
    Jacobian(m_, n_, par_, g, jac);
    i = QriSngValDec(jac, m_, n_, val, v, em);
    if (it == 1)
      lambda = inp[6] * VecVecN(val, val, &V);
    else if (p == 0)
      lambda *= w;
    else
      p = 0;
    FORLIM = n_;
    for (i = 1; i <= FORLIM; i++) {
      s = 0.0;
      FORLIM1 = m_;
      for (k = 1; k <= FORLIM1; k++)
	s = jac[k][i - 1] * g[k] + s;
      b[i] = val[i] * s;
    }
_Ll:
    FORLIM = n_;
    for (i = 1; i <= FORLIM; i++) {
      TEMP = val[i];
      bb[i] = b[i] / (TEMP * TEMP + lambda);
    }
    FORLIM = n_;
    for (i = 1; i <= FORLIM; i++) {
      if (!fixed[i]){
      s = 0.0;
      FORLIM1 = n_;
      for (k = 1; k <= FORLIM1; k++)
	s = v[i][k - 1] * bb[k] + s;
      parpres[i] = par_[i] - s;
     }
    }
    fe++;
    if (fe >= maxfe)
      err = 1;
    else if (Funct(m_, n_, parpres, g) != 0)
      err = 2;
    if (err != 0)
      goto _Lexit;
    fparpres = VecVecM(g, g, &V);
    res = fpar - fparpres;
    if (res < mu * VecVecN(b, bb, &V)) {
      p++;
      lambda = vv * lambda;
      if (p == 1) {
	lambdamin = ww * VecVecN(val, val, &V);
	if (lambda < lambdamin)
	  lambda = lambdamin;
      }
      if ((double)p < pw){
	goto _Ll;
      }else {
	err = 4;
	goto _Lexit;
      }
    }
    for (i =0; i<=n_;i++)
      par_[i]=parpres[i];
    /*    memcpy(V.par, parpres, (n_+1)*sizeof(double));*/
    fpar = fparpres;
    if(maqverb)
	PrintPar(&V);
  } while (fpar > abstolres && res > reltolres * fpar + abstolres);
_Lexit:
  FORLIM = n_;
  for (i = 1; i <= FORLIM; i++) {
    FORLIM1 = n_;
    for (j = 1; j <= FORLIM1; j++)
      jac[j][i - 1] = v[j][i - 1] / (val[i] + inp[0]);
  }
  FORLIM = n_;
  for (i = 1; i <= FORLIM; i++) {
    for (j = 1; j <= i; j++) {
      s = 0.0;
      FORLIM2 = n_;
      for (k = 0; k < FORLIM2; k++)
	s = jac[i][k] * jac[j][k] + s;
      v[i][j - 1] = s;
      v[j][i - 1] = s;
    }
  }
  lambda = val[1];
  lambdamin = lambda;
  FORLIM = n_;
  for (i = 2; i <= FORLIM; i++) {
    if (val[i] > lambda)
      lambda = val[i];
    else if (val[i] < lambdamin)
      lambdamin = val[i];
  }
  TEMP = lambda / (lambdamin + inp[0]);
  out[7] = TEMP * TEMP;
  out[2] = sqrt(fpar);
  out[6] = sqrt(fabs(res + fpar)) - out[2];
_Lescape:
  out[4] = fe;
  out[5] = it - 1.0;
  out[1] = err;
  if (maqverb){
  	switch ((int)out[1]){
	case 0:
		(void)fprintf(stderr,"marquardt: Normal termination\n");
		break;
	case 1:
		(void)fprintf(stderr,"marquardt: function calls greater than %d\n",(int)inp[5]);
		break;
	case 2:
		(void)fprintf(stderr,"marquardt: function call returned false\n");
		break;
	case 3:
		(void)fprintf(stderr,"marquardt: func became false with initial parameters\n");
		break;
	case 4:
		(void)fprintf(stderr,"marquardt: asked precision could not be obtained\n");
		break;
	default:
		break;
	}
  }
	 
  free(em);
  free(val);
  free(b);
  free(bb);
  free(parpres);
  m_free_matr(jac,m_+1);

  return out[1];
}  /*Marquardt*/


/*-
 * int maqinit (int *m, int *n)
 *
 *  returns -1 on error
 */
int
maqinit(int *m,int *n)
{
	int i;
	static int add = 0;
	static int max_m = 32000, max_n = 15;

  if (*m < MINM){
	  fprintf(stderr,"error: m < %d\n",MINM);
	  return -1;
  }

  if (*m > max_m || *n > max_n){
	  fprintf(stderr,"error: n or m to large (%d > %d || %d > %d)\n",*n,max_n,*m,max_m);
	  return -1;
  }
  if (out != NULL){
	  fprintf(stderr,"error: marquardt already initialized: (m = %d, n = %d)\n",maq_m,maq_n);
	  fprintf(stderr,"use maqend first\n");
	  return -1;
  }

  maq_n = *n > max_n ? max_n : *n;
  maq_m = *m > max_m ? max_m : *m;

  if (!add){
	  max_m = maq_m;
	  max_n = maq_n;
	  par=(double *)m_memory(NULL,(maq_n + 2)*sizeof(double),PROGNAME);
	  fixed=(int *)m_memory(NULL,(maq_n+1)*sizeof(int),PROGNAME);
	  rv=(double *)m_memory(NULL,(maq_m + 1)*sizeof(double),PROGNAME);
	  inp=(double *)m_memory(NULL,8*sizeof(double),PROGNAME);
	  add++;
  }
  out=(double *)m_memory(NULL,8*sizeof(double),PROGNAME);
  jjinv=m_matr(maq_m+1,maq_n+1);


  inp[0] = DBL_EPSILON;
  inp[3] = 1e-4;
  inp[4] = 1e-3;
  inp[5] = 50.0;
  inp[6] = 0.01;
  for (i=0; i <= maq_n;i++)
	fixed[i]=0;

  return 0;
}

void
maqend(int *m)
{
  if (out == NULL){
		fprintf(stderr,"error: maq not initialized, use maqinit first.\n");
		return;
  }else{
	  free(par);
	  free(fixed);
	  free(rv);
	  free(inp);
	  par = NULL;
	  fixed = NULL;
	  rv = NULL;
	  inp = NULL;
	  free(out);
	  m_free_matr(jjinv,*m+1);
	  out = NULL;
	  jjinv = NULL;
  }
}

void
prhead()
{
	if (inp == NULL){
		fprintf(stderr,"error: use maqinit first.\n");
		return;
	}else{
		fprintf(stderr,"marquardt: input parameters:\n");
		fprintf(stderr,"inp[0] = %g (Machine precision)\n",inp[0]);
		fprintf(stderr,"inp[3] = %g (relative tolerance for the difference\n\tbetween the euclidean norm of the ultimate and penultimate\n\tresidual vector)\n",inp[3]);
		fprintf(stderr,"inp[4] = %g (absolut tolerance for the difference\n\tbetween the euclidean norm of the ultimate and penultimate\n\tresidual vector)\n",inp[4]);
		fprintf(stderr,"inp[5] = %g (The maximum number of call to funct allowed)\n",inp[5]);
		fprintf(stderr,"inp[6] = %g (A starting value used for the relation between\n\t the gradient and the gaus-newton direction)\n",inp[6]);
	}
}

void
prtail()
{
	if (inp == NULL){
		fprintf(stderr,"error: use maqinit first.\n");
		return;
	}else{
		fprintf(stderr,"marquardt: output parameters:\n");
		fprintf(stderr,"out[1] = %g (process exit code)\n",out[1]);
		fprintf(stderr,"out[2] = %g (The euclidean norm of the residual vector\n\tcalculated with values of the unknows delivered)\n",out[2]);
		fprintf(stderr,"out[3] = %g (The euclidean norm of the residual vector\n\tcalculated with  the initial values of the unknows variables)\n",out[3]);
		fprintf(stderr,"out[4] = %g (The number of calls to func performed)\n",out[4]);
		fprintf(stderr,"out[5] = %g (The total number of iterations performed)\n",out[5]);
		fprintf(stderr,"out[6] = %g (The improvement of the euclidean norm of the\n\tresidual vector in the last iteration step)\n",out[6]);
		fprintf(stderr,"out[7] = %g (The condition number of j' * j. I.E. The ratio\n\t of its largest to smallest eigenvalues)\n",out[7]);
	}
}


int maqrun()
{
	if (inp == NULL){
		fprintf(stderr,"error: use maqinit first.\n");
		return -1;
	}else
		return Marquardt(maq_m, maq_n, par,rv, jjinv, inp, out);
}

