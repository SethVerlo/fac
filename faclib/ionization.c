#include "ionization.h"

static int n_usr = 0;
static double usr_egrid[MAX_USR_CIEGRID];
static double log_usr_egrid[MAX_USR_CIEGRID];
static int usr_egrid_type = 0;

static int n_egrid = 0;
static double egrid[MAX_CIEGRID];
static double log_egrid[MAX_CIEGRID];
static int n_tegrid = 0;
static double tegrid[MAX_IEGRID];
static double log_te[MAX_IEGRID];

static struct {
  int qr;
  int max_kl;
  int max_kl_1;
  int max_kl_f;
  double tolerence;
  int nkl0;
  int nkl;
  double kl[MAX_CINKL];
  double *qk;
} pw_scratch = {1, MAX_CIKL, 15, 0, EPS3, 0, 0};

static MULTI *qk_array;

int SetIEGrid(int n, double emin, double emax) {
  int i, ie;
  double del;

  if (n < 1) {
    tegrid[0] = -1.0;
    n_tegrid = 0;
    return 0;
  }

  if (emin < 0.0) {
    tegrid[0] = emin;
    return 0;
  }

  if (n > MAX_TEGRID) {
    printf("Max # of grid points reached \n");
    return -1;
  }

  if (n == 1) {
    n_tegrid = 1;
    tegrid[0] = emin;
    log_te[0] = log(emin);
    return 0;
  }

  if (n == 2) {
    n_tegrid = 2;
    tegrid[0] = emin;
    tegrid[1] = emax;
    log_te[0] = log(emin);
    log_te[1] = log(emax);
    return 0;
  }

  if (emax < emin) {
    printf("emin must > 0 and emax < emin\n");
    return -1;
  }
  
  n_tegrid = n;
  
  del = emax - emin;
  del /= n-1.0;
  tegrid[0] = emin;
  log_te[0] = log(emin);
  for (i = 1; i < n; i++) {
    tegrid[i] = tegrid[i-1] + del;
    log_te[i] = log(tegrid[i]);
  }
  
  return 0;
}


int SetCIPWOptions(int qr, int max, int max_1, int max_f, double tolerence) {
  pw_scratch.qr = qr;
  if (max > MAX_CIKL) {
    printf("The maximum partial wave reached in Ionization: %d\n", MAX_CIKL);
    abort();
  }
  pw_scratch.max_kl = max;
  pw_scratch.max_kl_1 = max_1;
  pw_scratch.max_kl_f = max_f;
  if (tolerence >= EPS16) pw_scratch.tolerence = tolerence;
  else tolerence = EPS3;
  pw_scratch.nkl0 = 1;
  pw_scratch.kl[0] = 0;
  pw_scratch.nkl = 0;
  return 0;
}

int AddCIPW(int n, int step) {
  int i;
  for (i = pw_scratch.nkl0; i < n+pw_scratch.nkl0; i++) {
    if (i >= MAX_CINKL) {
      printf("Maximum partial wave grid points reached in Ionization: %d\n", 
	     MAX_CINKL);
      abort();
    }
    pw_scratch.kl[i] = pw_scratch.kl[i-1] + step;
    if ((int) (pw_scratch.kl[i]) > pw_scratch.max_kl) break;
  }
  pw_scratch.nkl0 = i;
}

int SetCIPWGrid(int ns, int *n, int *step) {
  int i, m, k, j;
  if (ns > 0) {
    for (i = 0; i < ns; i++) {
      AddCIPW(n[i], step[i]);
    }
    k = step[ns-1]*2;
    j = 2;
  } else {
    ns = -ns;
    if (ns == 0) ns = 5;
    AddCIPW(ns, 1);
    k = 2;
    j = 2;
  } 

  m = pw_scratch.kl[pw_scratch.nkl0-1];
  while (m+k <= pw_scratch.max_kl) {
    AddCIPW(j, k);
    m = pw_scratch.kl[pw_scratch.nkl0-1];
    k *= 2;
  }
  pw_scratch.nkl = pw_scratch.nkl0;
}
  

int SetCIEGridDetail(int n, double *xg) {
  int i;
 
  n_egrid = n;
  for (i = 0; i < n; i++) {
    egrid[i] = xg[i];
    log_egrid[i] = log(egrid[i]);
  }

  return 0;
}

int SetCIEGrid(int n, double emin, double emax, int type) {
  double del;
  int i;

  if (n < 1) {
    n_egrid = 0;
    return -1;
  }
  if (emin < 0.0) {
    egrid[0] = emin;
    return 0;
  }
  if (n > MAX_EGRID) {
    printf("Max # of grid points reached \n");
    return -1;
  }

  if (emax < emin) {
    printf("emin must > 0 and emax < emin\n");
    return -1;
  }

  egrid[0] = emin;
  log_egrid[0] = log(egrid[0]);
  n_egrid = n;
  if (type == 0) {
    del = emax - emin;
    del /= n-1.0;
    for (i = 1; i < n; i++) {
      egrid[i] = egrid[i-1] + del;
      log_egrid[i] = log(egrid[i]);
    }
  } else {
    del = log(emax) - log(emin);
    del = del/(n-1.0);
    del = exp(del);
    for (i = 1; i < n; i++) {
      egrid[i] = egrid[i-1]*del;
      log_egrid[i] = log(egrid[i]);
    }
  }

  return 0;
}

int SetUsrCIEGridDetail(int n, double *xg, int type) {
  int i; 

  if (n < 1) {
    printf("Grid points must be at least 1\n");
    return -1;
  }
  if (n > MAX_USR_CIEGRID) {
    printf("Max # of grid points reached \n");
    return -1;
  }

  n_usr = n;
  usr_egrid_type = type;
  for (i = 0; i < n; i++) {
    usr_egrid[i] = xg[i];
    log_usr_egrid[i] = log(usr_egrid[i]);
  }
  return 0;
}

int SetUsrCIEGrid(int n, double emin, double emax, int type) {
  double del;
  int i;

  if (n < 1) {
    printf("Grid points must be at least 1\n");
    return -1;
  }
  if (n > MAX_USR_CIEGRID) {
    printf("Max # of grid points reached \n");
    return -1;
  }

  if (emin < EPS10 || emax < emin) {
    printf("emin must > 0 and emax < emin\n");
    return -1;
  }

  usr_egrid[0] = emin;
  log_usr_egrid[0] = log(usr_egrid[0]);
  n_usr = n;
  usr_egrid_type = type;

  del = log(emax) - log(emin);
  del = del/(n-1.0);
  del = exp(del);
  for (i = 1; i < n; i++) {
    usr_egrid[i] = usr_egrid[i-1]*del;
    log_usr_egrid[i] = log(usr_egrid[i]);
  }
  return 0;
}

int CIRadialQk(int ie1, int ie2, int kb, int kbp, int k) {
  int index[4];
  double pk[MAX_IEGRID][MAX_CIKL+1];
  double **p, e1, e2, e0, te;
  ORBITAL *orb;
  int kappab, jb, i, j, t, kl, klp;
  int js[4], ks[4];
  int jmin, jmax;
  int kf, kappaf, kf0, kappa0, kf1, kappa1;
  int kl0, kl0p, kl1, kl1p;
  int j0, j1, j1min, j1max;
  double r, rp, sd, se;
  double y2[MAX_CIKL+1], s;
  int kl_max1, kl_max2;

  index[0] = ie1;
  index[1] = ie2;
  index[2] = kb;
  index[3] = k/2;

  p = (double **) MultiSet(qk_array, index, NULL);
  if (*p) {
    pw_scratch.qk = *p;
    return 0;
  }

  for (i = 0; i < n_tegrid; i++) {
    for (j = 0; j < pw_scratch.nkl0; j++) {
      pk[i][j] = 0.0;
    }
  }
  (*p) = (double *) malloc(sizeof(double)*n_tegrid);
  pw_scratch.qk = *p;
  
  e1 = egrid[ie1];
  if (pw_scratch.max_kl_f > 0) {
    kl_max1 = pw_scratch.max_kl_f*(e1/tegrid[0]);
    kl_max1 = Max(kl_max1, pw_scratch.max_kl_1);
    kl_max1 = Min(pw_scratch.max_kl, kl_max1); 
  } else {
    kl_max1 = pw_scratch.max_kl;
  }
  if (k > 2) kl_max1 = Min(kl_max1, pw_scratch.max_kl_1);
  e2 = egrid[ie2];
  if (pw_scratch.max_kl_f > 0) {
    kl_max2 = pw_scratch.max_kl_f*(e1/tegrid[0]);
    kl_max2 = Max(kl_max2, pw_scratch.max_kl_1);
    kl_max2 = Min(pw_scratch.max_kl, kl_max2); 
  } else {
    kl_max2 = pw_scratch.max_kl;
  }
  if (k > 2) kl_max2 = Min(kl_max2, pw_scratch.max_kl_1);

  orb = GetOrbital(kb);
  kappab = orb->kappa;
  jb = GetJFromKappa(kappab);
  jmin = abs(k - jb);
  jmax = k + jb;
  for (j = jmin; j <= jmax; j += 2) {
    for (klp = j - 1; klp <= j + 1; klp += 2) {
      kappaf = GetKappaFromJL(j, klp);
      kl = klp/2;
      if (kl >= kl_max2) continue;
      if (kl < pw_scratch.qr) {
	js[2] = 0;
      } else {
	js[2] = j;
	if (IsOdd(kl)) {
	  if (kappaf < 0) kappaf = -kappaf - 1;
	} else {
	  if (kappaf > 0) kappaf = -kappaf - 1;
	}
      }
      kf = OrbitalIndex(0, kappaf, e2);	
      ks[2] = kf;  

      for (t = 0; t < pw_scratch.nkl0; t++) {
	kl0 = pw_scratch.kl[t];
	kl0p = 2*kl0;
	for (j0 = abs(kl0p - 1); j0 <= kl0p + 1; j0 += 2) {
	  kappa0 = GetKappaFromJL(j0, kl0p);
	  if (kl0 < pw_scratch.qr) {
	    js[1] = 0;
	  } else {
	    js[1] = j0;
	    if (IsOdd(kl0)) {
	      if (kappa0 < 0) kappa0 = -kappa0 - 1;
	    } else {
	      if (kappa0 > 0) kappa0 = -kappa0 - 1;
	    }
	  }
	  j1min = abs(j0 - k);
	  j1max = j0 + k;
	  for (j1 = j1min; j1 <= j1max; j1 += 2) {
	    for (kl1p = j1 - 1; kl1p <= j1 + 1; kl1p += 2) {
	      kappa1 = GetKappaFromJL(j1, kl1p);
	      kl1 = kl1p/2;
	      if (kl1 >= kl_max1) continue;
	      if (kl1 < pw_scratch.qr) {
		js[3] = 0;
	      } else {
		js[3] = j1;
		if (IsOdd(kl1)) {
		  if (kappa1 < 0) kappa1 = -kappa1 - 1;
		} else {
		  if (kappa1 > 0) kappa1 = -kappa1 - 1;
		}
	      }
	      kf1 = OrbitalIndex(0, kappa1, e1);
	      ks[3] = kf1;
	      for (i = 0; i < n_tegrid; i++) {
		te = tegrid[i];		
		e0 = e1 + e2 + te;
		kf0 = OrbitalIndex(0, kappa0, e0);
		ks[1] = kf0;

		js[0] = 0;
		ks[0] = kb;
		if (kl1 >= pw_scratch.qr &&
		    kl0 >= pw_scratch.qr) {
		  SlaterTotal(&sd, &se, js, ks, k, -1);
		} else {
		  SlaterTotal(&sd, &se, js, ks, k, 1);
		} 
		
		r = sd + se;

		if (kbp != kb) {
		  js[0] = 0;
		  ks[0] = kbp;
		  if (kl1 >= pw_scratch.qr &&
		      kl0 >= pw_scratch.qr) {
		    SlaterTotal(&sd, &se, js, ks, k, -1);
		  } else {
		    SlaterTotal(&sd, &se, js, ks, k, 1);
		  } 
		  rp = sd + se;
		} else {
		  rp = r;
		}
		r = r*rp;
		pk[i][t] += r;
	      }
	    }
	  }
	}
      }
    }
  }  

  for (i = 0; i < n_tegrid; i++) {
    spline(pw_scratch.kl, pk[i], pw_scratch.nkl, 1E30, 1E30, y2);
    r = pk[i][0];
    for (j = 1; j < pw_scratch.nkl; j++) {
      r += pk[i][j];
      kl0 = pw_scratch.kl[j-1];
      kl1 = pw_scratch.kl[j];
      for (kl = kl0+1; kl < kl1; kl++) {
	splint(pw_scratch.kl, pk[i], y2, pw_scratch.nkl, (double)kl, &s);
	r += s;
      }
    }
 
    r *= 8.0;
    pw_scratch.qk[i] = r;
  }

  return 0;
}

int IonizeStrength(double *s, double *te, int b, int f) {
  int i, ip, j, k, t;
  LEVEL *lev1, *lev2;
  double c, r, r0;
  int nz, j0, j0p, kb, kbp;
  double *rq[MAX_CIEGRID+1], *qy2[MAX_CIEGRID+1];
  double *x, x0, *rtmp; 
  double e, e2max, e2min, delta, e1, e2t, ehalf;
  double e2[N_INTEGRATE], integrand[N_INTEGRATE];
  double y2[MAX_IEGRID];
  ANGULAR_ZFB *ang;
  int ie1, ie2;

  lev1 = GetLevel(b);
  lev2 = GetLevel(f);
  *te = lev2->energy - lev1->energy;
  if (*te <= 0) return -1;
  
  nz = AngularZFreeBound(&ang, f, b);
  if (nz <= 0) return -1;
  
  for (i = 0; i <= n_egrid; i++) {
    rq[i] = malloc(n_egrid*sizeof(double));
    qy2[i] = malloc(n_egrid*sizeof(double));
  }
  rtmp = rq[n_egrid];

  for (j = 0; j < n_usr; j++) {
    s[j] = 0.0;
  }
  
  for (i = 0; i < nz; i++) {
    kb = ang[i].kb;
    j0 = GetOrbital(kb)->kappa;
    j0 = GetJFromKappa(j0);
    for (ip = 0; ip <= i; ip++) {
      kbp = ang[ip].kb;
      j0p = GetOrbital(kbp)->kappa;
      j0p = GetJFromKappa(j0p);
      if (j0p != j0) continue;
      c = ang[i].coeff*ang[ip].coeff/(j0+1.0);
      if (ip != i) {
	c *= 2.0;
      }
      
      for(ie1 = 0; ie1 < n_egrid; ie1++) {
	for (ie2 = 0; ie2 < n_egrid; ie2++) {
	  rq[ie1][ie2] = 0.0;
	  if (ie2 > ie1+1) break; 
	  for (k = 0; k <= MAX_CIK; k += 2) {
	    CIRadialQk(ie1, ie2, kb, kbp, k);
	    if (n_tegrid == 1) {
	      r = pw_scratch.qk[0];
	    } else {
	      x = log_te;
	      x0 = log(*te);
	      spline(x, pw_scratch.qk, n_tegrid, 1E30, 1E30, y2);
	      splint(x, pw_scratch.qk, y2, n_tegrid, x0, &r);
	    }
	    r = r/(k+1.0);
	    rq[ie1][ie2] += r;
	  }
	}
      }

      for (ie1 = 0; ie1 < n_egrid; ie1++) {
	spline(egrid, rq[ie1], n_egrid, 1E30, 1E30, qy2[ie1]);
      }
      for (j = 0; j < n_usr; j++) {
	e = usr_egrid[j];
	ehalf = e*0.5;
	e2min = 0.0;
	e2max = (*te)*0.5;
	e2max = Min(e2max, ehalf);
	delta = (e2max - e2min)/(N_INTEGRATE - 1);
	e2[0] = e2min;
	for (t = 1; t < N_INTEGRATE; t++) {
	  e2[t] = e2[t-1] + delta;
	}
	for (t = 0; t < N_INTEGRATE; t++) {
	  e1 = e - e2[t];
	  for (ie1 = 0; ie1 < n_egrid; ie1++) {
	    splint(egrid, rq[ie1], qy2[ie1], n_egrid, e2[t], &rtmp[ie1]);
	  }
	  spline(egrid, rtmp, n_egrid, 1E30, 1E30, qy2[ie1]);
	  splint(egrid, rtmp, qy2[ie1], n_egrid, e1, &r);
	  integrand[t] = r;
	}
	r0 = Simpson(integrand, 0, N_INTEGRATE-1);
	r0 *= delta;
      
	if (e2max < ehalf) {
	  e2min = e2max;
	  e2max = ehalf; 
	  e2[0] = log(e2min);
	  delta = (log(e2max)-e2[0])/(N_INTEGRATE-1);
	  for (t = 1; t < N_INTEGRATE; t++) {
	    e2[t] = e2[t-1] + delta;
	  }
	  for (t = 0; t < N_INTEGRATE; t++) {
	    e2t = exp(e2[t]);
	    e1 = e - e2t;
	    for (ie1 = 0; ie1 < n_egrid; ie1++) {
	      splint(egrid, rq[ie1], qy2[ie1], n_egrid, e2t, &rtmp[ie1]);
	    }
	    spline(egrid, rtmp, n_egrid, 1E30, 1E30, qy2[ie1]);
	    splint(egrid, rtmp, qy2[ie1], n_egrid, e1, &r);
	    integrand[t] = r*e2t;
	  }
	  r = Simpson(integrand, 0, N_INTEGRATE-1);
	  r *= delta;
	  r0 += r;
	}
	s[j] += 2*c*r0;
      }
    }
  }

  free(ang);
  for (i = 0; i <= n_egrid; i++) {
    free(rq[i]);
    free(qy2[i]);
  }
  
  return 0;
}

int SaveIonization(int nb, int *b, int nf, int *f, char *fn) {
  int i, j, k, n, m;
  int j1, j2, ie;
  FILE *file;
  LEVEL *lev1, *lev2;
  double emin, emax, e, s[MAX_USR_CIEGRID];
  
  if (n_usr == 0) {
    printf("No ionization energy specified \n");
    return -1;
  }

  file = fopen(fn, "w");
  if (!file) return -1;

  if (n_tegrid == 0) {
    n_tegrid = 3;
  }

  if (tegrid[0] < 0.0) {
    emin = 1E10;
    emax = 1E-10;
    for (i = 0; i < nb; i++) {
      lev1 = GetLevel(b[i]);
      for (j = 0; j < nf; j++) {
	lev2 = GetLevel(f[j]);
	e = lev2->energy - lev1->energy;
	if (e < emin && e > 0) emin = e;
	if (e > emax) emax = e;
      }
    }
    if ((emax - emin) < EPS3) {
      SetIEGrid(1, 0.5*(emin+emax), emax);
    } else {
      SetIEGrid(n_tegrid, emin, emax);
    }
  }

  if (n_egrid == 0) {
    n_egrid = 6;
  }

  if (egrid[0] < 0.0) {
    SetCIEGrid(n_egrid, EPS2*tegrid[0], usr_egrid[n_usr-1], 0);
  }

  if (pw_scratch.nkl0 == 0) {
    SetCIPWOptions(1, 30, 15, 0, 1E-3);
  } 
  if (pw_scratch.nkl == 0) {
    SetCIPWGrid(0, NULL, NULL);
  }

  fprintf(file, " IEGRID:   ");
  for (i = 0; i < n_tegrid; i++) {
    fprintf(file, "%10.4E ", tegrid[i]*HARTREE_EV);
  }
  fprintf(file, "\n");

  fprintf(file, " EGRID:    ");
  for (i = 0; i < n_egrid; i++) {
    fprintf(file, "%10.4E ", egrid[i]*HARTREE_EV);
  }
  fprintf(file, "\n\n");

  fprintf(file, "Bound 2J\tFree  2J\tDelta_E\n");
  for (i = 0; i < nb; i++) {
    j1 = LevelTotalJ(b[i]);
    for (j = 0; j < nf; j++) {
      j2 = LevelTotalJ(f[j]);
      k = IonizeStrength(s, &e, b[i], f[j]);
      if (k < 0) continue;
      fprintf(file, "%-5d %-2d\t%-5d %-2d\t%10.4E\n",
	      b[i], j1, f[j], j2, e*HARTREE_EV);
      for (ie = 0; ie < n_usr; ie++) {
	fprintf(file, "%-10.3E %-10.3E ", 
		usr_egrid[ie]*HARTREE_EV, s[ie]);
	s[ie] = AREA_AU20*(1E4*s[ie])/(2*(e+usr_egrid[ie])*(j1+1.0));
	fprintf(file, "%-10.3E ", s[ie]);
	fprintf(file, "\n");
      }
      fprintf(file, "\n");
    }
  }

  fclose(file);
  return 0;
}

void _FreeIonizationQk(void *p) {
  double *dp;
  dp = *((double **) p);
  free(dp);
}

int FreeIonizationQk() {
  ARRAY *b;
  
  b = qk_array->array;
  if (b == NULL) return 0;
  MultiFreeData(b, qk_array->ndim, _FreeIonizationQk);
  return 0;
}

int InitIonization() {
  int blocks[4] = {8, 8, 10, 6};
  int ndim = 4;
  
  qk_array = (MULTI *) malloc(sizeof(MULTI));
  MultiInit(qk_array, sizeof(double *), ndim, blocks);
  n_egrid = 0;
  n_tegrid = 0;
  egrid[0] = -1.0;
  tegrid[0] = -1.0;
  return 0;
}
