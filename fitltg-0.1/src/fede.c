#include "FC.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

/* Global variables (common blocks etc.) */

#define MXNN 10
static struct {
   int      jxt, jh, mm, ipdx, ipdy, ig, mpts;
   double       sig;
} para001;
static struct {
   double      scl[MXNN];
} para002;
static struct {
   double       rmn, rmn1, rmns1, g, eta;
} para003;
static struct {
   float*        z, *a, *xx, *yy;
} ededata;

/*
c                    ***************************
c                    *  Initialization of fede *
c                    ***************************
*/

void ini_f(void)
{
   if (para001.jxt != 0) {
      para003.rmn = para002.scl[3] + para002.scl[4];
      para003.g   = para002.scl[6];
      para002.scl[1] = para002.scl[1] * (para003.rmn + 1.);
      para002.scl[2] = para002.scl[2] * (para003.rmn + 1.);
   } else {
      para003.rmn = para002.scl[1];
      para003.g = para002.scl[2];
   }
   para003.rmn1 = para003.rmn + 1.;
   para003.rmns1 = 1. / para003.rmn1;
   para002.scl[0] = pow(para002.scl[0], para003.rmn1);
   para003.eta = 0.;
   if (para001.jh != 0) para003.eta = para002.scl[para001.mm - 1];
}

/*
c                    *************************************
c                    *  DE value given by the functional *
c                    *************************************
*/

float fede(float z, float a, float xxx)
{
   double       xx, hh, hlg, rho, re, vv, zalm, z2amu;

   xx = xxx - para002.scl[para001.ipdx - 1];
   if (para001.jh != 0) {
      rho = para003.eta * z * z * a;
      hh = xx;
      hlg = log(1. + hh / rho);
      xx = sqrt(hh * hh + 2.*hh * rho * (1. + hlg));
   }
   re = para003.g * xx;
   if (para001.jxt != 0) {
      z2amu = z * z * pow(a, para002.scl[3]);
      zalm = para002.scl[0] * pow(z, para002.scl[1]) * pow(a, para002.scl[2]);
      vv = para002.scl[5] * z2amu * pow(re, para002.scl[4]);
   } else {
      z2amu = z * z * pow(a, para002.scl[1]);
      zalm = para002.scl[0] * z2amu;
      vv = 0.;
   }
   return (pow((pow(re, para003.rmn1) + zalm + vv), para003.rmns1)
           - re + para002.scl[para001.ipdy - 1]);
}

/*
c     **************************************************
c     * Calculation of the functional and its gradient *
c     *             at each data point                 *
c     **************************************************
*/
void FC_GLOBAL(miniuser, MINIUSER)(int* ip, double* x, double* ecart, double* grd, int* iwf)
{
   static   double   rmn, rmn1, rmns1, g, eta;
   double   z, a, alg, zlg, z2a, z2amu, zal, zalm, rho, xx, yy;
   static   double   xt[MXNN];
   int      i;
   double xxc, re, rea, enu, penu, bra, rac, vv, vv0, v1, rle;
   double sg, hh, hl, hlg, hl1;

   *iwf = -1;
   if (*ip > para001.mpts) return;
   if (*ip == 1) {
      for (i = 1; i <= para001.mm; i++) {
         xt[i - 1] = x[i - 1] * para002.scl[i - 1];
      }
      if (para001.jxt != 0) {
         rmn = xt[3] + xt[4];
         g = xt[6];
      } else {
         rmn = xt[1];
         g = xt[2];
      }
      rmn1 = rmn + 1.;
      rmns1 = 1. / rmn1;
      eta = 0.;
      if (para001.jh != 0) eta = xt[para001.mm - 1];
   }
   *iwf = 0;

   z = ededata.z[*ip - 1];
   a = ededata.a[*ip - 1];
   xx = ededata.xx[*ip - 1];
   yy = ededata.yy[*ip - 1];

   zal = 0.;
   zlg = 0.;
   if (para001.jxt != 0) {
      zlg = log(z);
      alg = log(a);
      zal = pow(z, xt[1]) * pow(a, xt[2]) * xt[0];
      zalm = pow(zal, rmn1);
      z2amu = pow(z, 2.) * pow(a, xt[3]);
   } else {
      alg = log(xt[0] * a);
      z2amu = pow(z, 2.) * pow(a, rmn);
      zalm = pow(xt[0], rmn1) * z2amu;
   }
   z2a = z * z * a;
   rho = eta * z2a;

   xxc = xx - xt[para001.ipdx - 1];
   hh = xxc;
   hl1 = 0.;
   hlg = 0.;
   hl = 0.;
   if (para001.jh != 0) {
      hl = hh / rho;
      hlg = log(1. + hl);
      hl1 = 1. / (1. + hl);
      xxc = sqrt(hh * hh + 2.*hh * rho * (1. + hlg));
   }
   re = g * xxc;
   rle = log(re);
   rea = pow(re, rmn1);
   penu = enu = 0.;
   if (para001.jxt != 0) {
      enu = pow(re, xt[4]);
      penu = xt[5] * z2amu * enu;
   }
   bra = rea + zalm + penu;
   rac = pow(bra, rmns1);
   *ecart = rac - re + xt[para001.ipdy - 1] - yy;
   vv0 = rac / bra;
   vv = vv0 * zalm;
   grd[0] = vv / xt[0];
   v1 = 0.;
   if (para001.jxt != 0) {
      grd[1] = vv * zlg;
      grd[2] = vv * alg;
      v1 = xt[4] * rmns1 * penu;
   }
   vv = rac / bra / re * (rea + v1) - 1.;
   grd[para001.ig - 1] = vv * xxc;
   vv = vv * g / xxc * hh;
   if (para001.jh != 0) {
      grd[para001.mm - 1] = vv * (hl1 + hlg) * z * z * a;
      vv = vv * (1. + hl1 + (1. + hlg) / hl);
   }
   grd[para001.ipdx - 1] = -vv;
   vv0 = vv0 * rmns1;
   if (para001.jxt != 0) {
      grd[5] = vv0 * z2amu * enu;
      v1 = rea * rle + zalm * log(zal) - bra * log(bra) * rmns1;
      grd[3] = vv0 * (v1 + penu * alg);
      grd[4] = vv0 * (v1 + penu * rle);
   } else {
      v1 = rea * rle + zalm * log(a * xt[0]) - bra * log(bra) * rmns1;
      grd[1] = vv0 * v1;
   }
   grd[para001.ipdy - 1] = 1.;
   sg = para001.sig * sqrt(yy);
   *ecart = *ecart / sg;
   for (i = 1; i <= para001.mm; i++) {
      grd[i - 1] = grd[i - 1] * para002.scl[i - 1] / sg;
   }
   *iwf = 1;
}

/* declarations needed by fitede */

void FC_GLOBAL(mini8drv, MINI8DRV)(int*, double*, double*, double*, double*, int*, int*, double*, int*, int*);
void FC_GLOBAL(mini8, MINI8)(int*, int*, int*, double*,
                             int*, double*, double*, double*, double*, double*, double*, double*, int*, int*, int*, double*, int*, int*);

/*
c                    ******************
c                    *  Specific fit  *
c                    ******************
*/
void FC_GLOBAL(fitede, FITEDE)(int* npts, float* zd, float* ad, float* xd, float* yd,
                               int* ixt, int* ih, float* sc, float* bl, float* bu, float* par,
                               int* istate, int* irc)
{
   double       bl8[MXNN], bu8[MXNN], x[MXNN], g[MXNN], dx[MXNN];
   double       h[MXNN * (MXNN + 1) / 2], w[MXNN * (MXNN + 2) + 1];
   double       f;
   int          iw[MXNN * 2];

   int          i, ibound, iout, iprint, itest;
   int          liw, lw, mxcall;
   double       tolg = 1.e-5;

   iout = 6;
   para001.jxt = *ixt;
   para001.jh = *ih;
   /* On garde les adresses des tableaux de donnees */
   ededata.z = zd;
   ededata.a = ad;
   ededata.xx = xd;
   ededata.yy = yd;
   para001.mpts = *npts;
   liw = MXNN * 2;
   lw = MXNN * (MXNN + 1) + 1;
   /*   itest=0  -> on minimise        itest=1 -> on teste les derivees */
   itest = 0;

   para001.mm = 5;
   if (*ixt != 0) para001.mm = 9;
   para001.ipdy = para001.mm;
   para001.ipdx = para001.mm - 1;
   para001.ig = para001.mm - 2;
   if (*ih != 0) ++para001.mm;
   *irc = 11;
   for (i = 1; i <= para001.mm; i++) {
      if (bl[i - 1] > par[i - 1] || bu[i - 1] < par[i - 1]) return;
      para002.scl[i - 1] = sc[i - 1];
      x[i - 1] = par[i - 1];
      bl8[i - 1] = bl[i - 1];
      bu8[i - 1] = bu[i - 1];
   }
   if (sc[0] == 0.) {
      for (i = 1; i <= para001.mm; i++) {
         para002.scl[i - 1] = fabs(par[i - 1]);
      }
   }
   for (i = 1; i <= para001.mm; i++) {
      x[i - 1] = x[i - 1] / para002.scl[i - 1];
      bl8[i - 1] = bl8[i - 1] / para002.scl[i - 1];
      bu8[i - 1] = bu8[i - 1] / para002.scl[i - 1];
      if (bl8[i - 1] > x[i - 1]) bl8[i - 1] = x[i - 1];
      if (bu8[i - 1] < x[i - 1]) bu8[i - 1] = x[i - 1];
   }
   para001.sig = *npts;
   para001.sig = sqrt(para001.sig);
   /*                   Test des derivees               */
   if (itest > 0) {
      for (i = 1; i <= para001.mm; i++) {
         dx[i - 1] = 0.0000001;
      }
      FC_GLOBAL(mini8drv, MINI8DRV)(&para001.mm, x, dx, g, h, iw, &liw, w, &lw, &iout);
      for (i = 1; i <= para001.mm; i++) {
         istate[i - 1] = 0;
      }
      *irc = 0;
      f = 0.;
   }
   /*                   Minimisation          */
   else {
      mxcall = para001.mm * 50;
      ibound = 1;
      iprint = 1;
      for (i = 1; i <= para001.mm; i++) {
         dx[i - 1] = 0.2;
      }
      FC_GLOBAL(mini8, MINI8)(&para001.mm, &iprint, &mxcall, &tolg,
                              &ibound, bl8, bu8, dx, x, &f, g, h, istate, iw, &liw, w, &lw, irc);
   }
   for (i = 1; i <= para001.mm; i++) {
      x[i - 1] = x[i - 1] * para002.scl[i - 1];
      par[i - 1] = x[i - 1];
      g[i - 1] = g[i - 1] / para002.scl[i - 1];
      para002.scl[i - 1] = x[i - 1];
   }
   ini_f();
   printf(" +++++++  irc =%3d     f =%12.4g  +++++++++++++\n", *irc, f);
   printf("     X =");
   for (i = 1; i <= para001.mm; i++) {
      printf("%14.6E", x[i - 1]);
   }
   printf("\n");
   printf("     g =");
   for (i = 1; i <= para001.mm; i++) {
      printf("%14.4E", g[i - 1]);
   }
   printf("\n");
   printf("   ist =");
   for (i = 1; i <= para001.mm; i++) {
      printf("%14d", istate[i - 1]);
   }
   printf("\n");
   if (*ixt == 0) {
      printf(" lambda,mu,g =%10.5f%10.5f%12.6f\n", x[0], x[1], x[2]);
      printf(" pdx =%8.1f    pdy =%8.1f    \n", x[3], x[4]);
   } else {
      printf(" lambda,alpha,beta =%10.5f%10.5f%10.5f\n", x[0], x[1], x[2]);
      printf(" mu,nu =%10.5f%10.5f\n", x[3], x[4]);
      printf(" xi =%14.6f\n", x[5]);
      printf(" g  =%12.6f\n", x[6]);
      printf(" pdx =%8.1f    pdy =%8.1f    \n", x[7], x[8]);
   }
   if (*ih != 0) printf(" eta =%12.6f\n", x[para001.mm - 1]);
}
