/*
  $Header: /home/CVS/luigi/FClasses/KVDigitalFilter.cxx,v 1.6 2013-05-23 19:40:20 garfield Exp $
  $Log: KVDigitalFilter.cxx,v $
  Revision 1.6  2013-05-23 19:40:20  garfield
  KVDigitalFilter: added methods to apply a FIR filter (no b coefficients)
             added methods to read/write FIR coefficients for MatLab

  Revision 1.5  2013-05-23 11:29:09  garfield
  Added method to draw the step response of the filter.

  Revision 1.4  2010-01-26 09:48:12  bini
  Ottimizzazioni in KVSignal
  Patch per interpolazione cubica in KVDigitalFilterWizard

  Revision 1.3  2009/10/12 12:57:31  bardelli
  Aggiunto il metodo static KVDigitalFilter BuildInverse(KVDigitalFilter *filter)

  Revision 1.2  2006/05/05 13:10:26  bardelli
  Aggiunto Header e Log in testa a tutti i files

*/


//=======================
// Author: G.Pasquali & L.Bardelli
// @(#) 17 11 2003
//=======================
//////////////////////////////////////////////////////////////////////////
//                                                                      //
// KVDigitalFilter                                                            //
//                                                                      //
// Class for digital filtering                                          //
//                                                                      //
// No limits on number of a[] and b[] coeffs.                           //
//                                                                      //
// b[0]  = 0. (this is different from the sources of KVSignal!)          //
//                                                                      //
// INTERNAL COMPUTATIONS DONE USING "DspGuide notation".                //
//                                                                      //
// Example:                                                             //
//                                                                      //
//   // Low pass RC with tau=2 us, coeff. for a 10ns sampling period    //
//KVDigitalFilter rc=KVDigitalFilter::BuildRCLowPass( 2, 10 );                   //
//                                                                      //
//   // Apply it to a signal: intermediate results are "double"         //
//   // type (i.e. 64 bit).                                             //
//   KVSignal s;                                                         //
//   rc.ApplyTo( &s );                                                  //
//                                                                      //
//   // Apply it to generic data: intermediate results are "double"     //
//   // type (i.e. 64 bit).                                             //
//     void ApplyTo(double *data, const int N, int reverse=0) const;    //
//     void ApplyTo(float  *data, const int N, int reverse=0) const;    //
//     void ApplyTo(int    *data, const int N, int reverse=0) const;    //
//                                                                      //
//   // Combine two  filters                                            //
//KVDigitalFilter rc=KVDigitalFilter::BuildRCLowPass( 2, 10 );                   //
//KVDigitalFilter cr=KVDigitalFilter::BuildRCHighPass( 2, 10 );                  //
//KVDigitalFilter total=KVDigitalFilter::CombineStages( &rc, &cr );              //
//                                                                      //
//   // Combine Many series (parallel=0)                                //
//KVDigitalFilter total4=KVDigitalFilter::CombineStagesMany(&cr,&rc,&rc,&rc,&rc);//
//                                                                      //
//   // Write Coeffs                                                    //
//   total.PrintCoeffs()                                                //
//        Coefficients valid with tau_clk=10.000 ns                     //
//        a[ 0]=         4.975083e-03    -                              //
//        a[ 1]=        -4.975083e-03    b[ 1]= 1.990025e+00            //
//        a[ 2]=         0.000000e+00    b[ 2]= -9.900498e-01           //
//        TOTAL: 4 coefficients (a+b).                                  //
//                                                                      //
//   - Various "Build*" methods...                                      //
//   - ComputeGainDC(); // gain at 0 frequency                          //
//   - ComputeGainNy(); // gain at Nyquist frequency (0.5)              //
//                                                                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
#include "KVDigitalFilter.h"
#define DUEPI 6.28318530717958623
#define    PI (DUEPI/2.)

#define LINE() // printf("Now  %s line %d...\n",__PRETTY_FUNCTION__, __LINE__);

ClassImp(KVDigitalFilter);

//============================================
KVDigitalFilter KVDigitalFilter::BuildRCLowPass(const double& tau_usec, const double& tau_clk)
{

   double f = 1. / (DUEPI * tau_usec * 1000.) * tau_clk;
   double x = exp(-DUEPI * f);
   KVDigitalFilter filter(tau_clk);
   filter.Alloc(2);
   filter.a[0] = 1 - x;
   filter.b[1] = x;

   return filter;
}
//============================================
KVDigitalFilter KVDigitalFilter::BuildRCLowPassDeconv(const double& tau_usec, const double& tau_clk)
{

   double f = 1. / (DUEPI * tau_usec * 1000.) * tau_clk;
   double x = exp(-DUEPI * f);
   KVDigitalFilter filter(tau_clk);
   filter.Alloc(2);
   filter.a[0] = 1 / (1 - x);
   filter.a[1] = -x / (1 - x);

   return filter;
}
//============================================
KVDigitalFilter KVDigitalFilter::BuildRCHighPassWithPZ(const double&  tau_usec, const double&  preamp_decay_usec, const double&  tau_clk)
{
   KVDigitalFilter highpass = KVDigitalFilter::BuildRCHighPass(tau_usec, tau_clk);
   double ycoef[] = {0};
   double xcoef[] = {tau_usec / (preamp_decay_usec + tau_usec)};
   KVDigitalFilter polez(tau_clk, 1, xcoef, ycoef);
   return KVDigitalFilter::CombineStages(&highpass, &polez, 1);
}
//============================================
KVDigitalFilter KVDigitalFilter::BuildRCHighPass(const double& tau_usec, const double& tau_clk)
{

   double f = 1. / (DUEPI * tau_usec * 1000.) * tau_clk;
   double x = exp(-DUEPI * f);
   KVDigitalFilter filter(tau_clk);
   filter.Alloc(2);
   filter.a[0] = (1 + x) / 2.;
   filter.a[1] = -(1 + x) / 2.;
   filter.b[1] = x;

   return filter;
}
//============================================
KVDigitalFilter KVDigitalFilter::BuildChebyshev(const double& freq_cutoff_mhz, int is_highpass,
      const double&  percent_ripple, int npoles,
      const double& tau_clk)
{
   KVDigitalFilter filter(tau_clk);
   filter.Alloc(30);
   ComputeChebyshevCoeffs(freq_cutoff_mhz * (tau_clk / 1e3),
                          is_highpass, percent_ripple, npoles, filter.a, filter.b);
   filter.Compress();
   return filter;
}
//============================================

void KVDigitalFilter::ComputeChebyshevCoeffs_serv(const double& fc, const double&   lh,
      const double& pr, const double&   np,
      int p,
      double* a0, double* a1, double* a2,
      double* b1, double* b2)
{
   double rp, ip, es, vx, kx, t, w, m, d, k = 0;
   double x0, x1, x2, y1, y2;

   //     printf("%e %e\n",cos(PI/(np*2.)+(p-1)*PI/np),PI/(np*2.)+(p-1)*PI/np);

   /* calculate the pole location on the unit circle */
   rp = -cos(PI / (np * 2.) + (p - 1) * PI / np);
   ip = sin(PI / (np * 2.) + (p - 1) * PI / np);

   ///    printf("rp=%e ip=%e\n",rp,ip);

   /* warp from a circle to an ellipse */

   if (pr == 0) goto pr_zero;

   //     printf("pr=%e\n",pr);

   es = sqrt((100. / (100. - pr)) * (100. / (100. - pr)) - 1);
   vx = (1 / np) * log((1. / es) + sqrt(1. / es / es + 1.));
   kx = (1 / np) * log((1. / es) + sqrt(1. / es / es - 1.));
   kx = (exp(kx) + exp(-kx)) / 2.;
   rp = rp * ((exp(vx) - exp(-vx)) / 2.) / kx;
   ip = ip * ((exp(vx) + exp(-vx)) / 2.) / kx;

   /* s-domain to z-domain conversion */
   ////     printf("rp=%e ip=%e es=%e vx=%e kx=%e\n",rp,ip,es,vx,kx);

pr_zero:
   t = 2 * tan(1 / 2.);
   w = 2.*PI * fc;
   m = rp * rp + ip * ip;
   d = 4 - 4 * rp * t + m * t * t;

   x0 = t * t / d;
   x1 = 2.*t * t / d;
   x2 = t * t / d;

   y1 = (8 - 2.*m * t * t) / d;
   y2 = (-4. - 4.*rp * t - m * t * t) / d;

   //     printf("t=%e w=%e m=%e d=%e\n",t,w,m,d);
   //     printf("x0=%e x1=%e x2=%e y1=%e y2=%e\n",x0,x1,x2,y1,y2);
   /* LP to LP or LP to HP transform */
   if (lh == 1) k = -cos(w / 2. + 1 / 2.) / cos(w / 2. - 1 / 2.);
   if (lh == 0) k = sin(-w / 2. + 1 / 2.) / sin(w / 2. + 1 / 2.);
   d = 1 + y1 * k - y2 * k * k ;
   *a0 = (x0 - x1 * k + x2 * k * k) / d;
   *a1 = (-2.*x0 * k + x1 + x1 * k * k - 2.*x2 * k) / d;
   *a2 = (x0 * k * k - x1 * k + x2) / d;
   *b1 = (2.*k + y1 + y1 * k * k - 2.*y2 * k) / d;
   *b2 = (-k * k - y1 * k + y2) / d;
   if (lh == 1) *a1 = -(*a1);
   if (lh == 1) *b1 = -(*b1);


   return;
}

//============================================

void KVDigitalFilter::ComputeChebyshevCoeffs(const double& freq_cutoff,
      int is_highpass, const double& percent_ripple, int npoles,
      double* a, double* b)
{
   double fc = freq_cutoff;
   double lh = is_highpass;
   double pr = percent_ripple;
   double np = npoles;

   double a0, a1, a2, b1, b2, gain;
   double ta[22], tb[22];

#define CHECK(A,MIN,MAX) if(A<MIN || A>MAX) printf("ERROR in %s: %s=%e! (ok is %e..%e)\n",__PRETTY_FUNCTION__,#A,(double)A,(double)MIN,(double)MAX);
   CHECK(freq_cutoff, 0, 0.5);
   CHECK(lh, 0, 1);
   CHECK(pr, 0, 29);
   CHECK(np, 2, 20);
#undef CHECK

   for (int i = 0; i < 22; i++) {
      a[i] = 0.;
      b[i] = 0.;
   }
   a[2] = 1.;
   b[2] = 1.;

   for (int p = 1; p <= np / 2; p++) {
      ComputeChebyshevCoeffs_serv(fc, lh, pr, np, p, &a0, &a1, &a2, &b1, &b2);
      for (int i = 0; i < 22; i++) {
         ta[i] = a[i];
         tb[i] = b[i];
      }
      for (int i = 2; i < 22; i++) {
         a[i] = a0 * ta[i] + a1 * ta[i - 1] + a2 * ta[i - 2];
         b[i] =    tb[i] - b1 * tb[i - 1] - b2 * tb[i - 2];
      }
   }
   b[2] = 0.;
   for (int i = 0; i < 20; i++) {
      a[i] = a[i + 2];
      b[i] = -b[i + 2];
   }
   double sa = 0;
   double sb = 0;

   /***** ORIGINALE DI Gab.*********/
#define AAA
#ifdef AAA
   for (int i = 0; i < 20; i++) {
      if (lh == 0) sa = sa + a[i];
      if (lh == 0) sb = sb + b[i];
      if (lh == 1) sa = sa + a[i] * pow(-1., i);
      if (lh == 1) sb = sb + b[i] * pow(-1., i);
   }
#else
   /**********************/
   if (lh == 0) {
      for (int i = 0; i < 20; i++) {
         sa = sa + a[i];
         sb = sb + b[i];
      }
   }//end of lh==0
   else {
      int sign = 1;
      for (int i = 0; i < 20; i++) {
         sa = sa + a[i] * sign;
         sb = sb + b[i] * sign;
         sign *= -1;
      }
   }
#endif
/////  printf("lh=%f sa=%f sb=%f\n",lh,sa,sb);
   gain = sa / (1. - sb);
   for (int i = 0; i < 20; i++) a[i] = a[i] / gain;
////  for(int i=0; i<20; i++) printf("%d: %e\n",i,a[i]);
   return;
}
//============================================
KVDigitalFilter::KVDigitalFilter(const double& tau)
{

   a = b = NULL;
   tau_clk = tau;
   Ncoeff = 0;

}
//============================================
KVDigitalFilter::KVDigitalFilter(const double& tau, int N, const double* Xcoeffs, const double* Ycoeffs)
{

   a = b = NULL;
   tau_clk = tau;
   Ncoeff = 0;
   Alloc(N);
   memcpy(a, Xcoeffs, sizeof(double)*N);
   memcpy(b, Ycoeffs, sizeof(double)*N);

}
//=============================================
KVDigitalFilter::~KVDigitalFilter()
{

   if (a != NULL) {
      delete [] a;
      delete [] b;
   }

}
//=============================================
void KVDigitalFilter::Alloc(const int N)
{

   //  printf("a=%p, N=%d, Ncoeff=%d\n", a, N, Ncoeff);
   Ncoeff = N;
   if (a != NULL) {
      delete [] a;
      delete [] b;
      a = NULL;
      b = NULL;
   };
   if (N > 0) {
      a = new double [N];
      b = new double [N];
      Azzera();
   }

}
//=============================================
void KVDigitalFilter::Azzera()
{

   memset(a, 0, sizeof(double)*Ncoeff);
   memset(b, 0, sizeof(double)*Ncoeff);

}
//=============================================
KVDigitalFilter::KVDigitalFilter(const KVDigitalFilter& orig)
   : KVBase()
{

   Ncoeff = 0;
   tau_clk = orig.tau_clk;
   a = b = NULL;
   if (orig.Ncoeff > 0) {
      Alloc(orig.Ncoeff);
      memcpy(a, orig.a, sizeof(double)*Ncoeff);
      memcpy(b, orig.b, sizeof(double)*Ncoeff);
   }

}
//=============================================
KVDigitalFilter KVDigitalFilter::operator =(const KVDigitalFilter& orig)
{

   tau_clk = orig.tau_clk;
   Alloc(orig.Ncoeff);
   if (orig.Ncoeff > 0) {
      memcpy(a, orig.a, sizeof(double)*Ncoeff);
      memcpy(b, orig.b, sizeof(double)*Ncoeff);
   }

   return *this;
}
//=============================================
void KVDigitalFilter::ApplyTo(double* datax, const int NSamples, int reverse) const
{
   // Copiato +- da KVSignal.cxx
   // Diversa la convenzione per a0 b0!

   long double* datay = new long double[NSamples];
   int i = 0, k = 0;
   switch (reverse) {
      case 0:// direct
         for (i = 0; i < Ncoeff; i++) { // primo loop su Npunti
            datay[i] = a[0] * datax[i];
            for (k = 0; k < i; k++)
               datay[i] += a[k + 1] * datax[i - k - 1] + b[k + 1] * datay[i - k - 1];
         }
         for (i = Ncoeff; i < NSamples; i++) { //secondo loop. cambia l'indice interno.
            datay[i] = a[0] * datax[i];
            for (k = 0; k < Ncoeff - 1; k++)
               datay[i] += a[k + 1] * datax[i - k - 1] + b[k + 1] * datay[i - k - 1];
         }
         break; // end of direct
      case 1: //reverse: cut & paste from direct and NSamples-1-
         for (i = 0; i < Ncoeff; i++) { // primo loop su Npunti
            datay[NSamples - 1 - i] = a[0] * datax[NSamples - 1 - i];
            for (k = 0; k < i; k++)
               datay[NSamples - 1 - i] += a[k + 1] * datax[NSamples - 1 - (i - k - 1)]
                                          + b[k + 1] * datay[NSamples - 1 - (i - k - 1)];
         }
         for (i = Ncoeff; i < NSamples; i++) { //secondo loop. cambia l'indice interno.
            datay[NSamples - 1 - i] = a[0] * datax[NSamples - 1 - i];
            for (k = 0; k < Ncoeff - 1; k++)
               datay[NSamples - 1 - i] += a[k + 1] * datax[NSamples - 1 - (i - k - 1)]
                                          + b[k + 1] * datay[NSamples - 1 - (i - k - 1)];
         }
         break;
      case -1: // bidirectional
         ApplyTo(datax, NSamples, 0);
         ApplyTo(datax, NSamples, 1);
         return;
      default:
         printf("ERROR in %s: reverse=%d not supported\n", __PRETTY_FUNCTION__, reverse);
   }// end of reverse switch.
   /*----------------------------------------------*/
   //   void *memcpy(void *dest, const void *src, size_t n);
   memcpy(datax, datay, NSamples * sizeof(double));
   delete [] datay;

}
//=============================================
void KVDigitalFilter::ApplyTo(float* datax, const int NSamples, int reverse) const
{
   // Copiato +- da KVSignal.cxx
   // Diversa la convenzione per a0 b0!

   long double* datay = new long double[NSamples];
   int i = 0, k = 0;
   switch (reverse) {
      case 0:// direct
         for (i = 0; i < Ncoeff; i++) { // primo loop su Npunti
            datay[i] = a[0] * datax[i];
            for (k = 0; k < i; k++)
               datay[i] += a[k + 1] * datax[i - k - 1] + b[k + 1] * datay[i - k - 1];
         }
         for (i = Ncoeff; i < NSamples; i++) { //secondo loop. cambia l'indice interno.
            datay[i] = a[0] * datax[i];
            for (k = 0; k < Ncoeff - 1; k++)
               datay[i] += a[k + 1] * datax[i - k - 1] + b[k + 1] * datay[i - k - 1];
         }
         break; // end of direct
      case 1: //reverse: cut & paste from direct and NSamples-1-
         for (i = 0; i < Ncoeff; i++) { // primo loop su Npunti
            datay[NSamples - 1 - i] = a[0] * datax[NSamples - 1 - i];
            for (k = 0; k < i; k++)
               datay[NSamples - 1 - i] += a[k + 1] * datax[NSamples - 1 - (i - k - 1)]
                                          + b[k + 1] * datay[NSamples - 1 - (i - k - 1)];
         }
         for (i = Ncoeff; i < NSamples; i++) { //secondo loop. cambia l'indice interno.
            datay[NSamples - 1 - i] = a[0] * datax[NSamples - 1 - i];
            for (k = 0; k < Ncoeff - 1; k++)
               datay[NSamples - 1 - i] += a[k + 1] * datax[NSamples - 1 - (i - k - 1)]
                                          + b[k + 1] * datay[NSamples - 1 - (i - k - 1)];
         }
         break;
      case -1: // bidirectional
         ApplyTo(datax, NSamples, 0);
         ApplyTo(datax, NSamples, 1);
         return;
      default:
         printf("ERROR in %s: reverse=%d not supported\n", __PRETTY_FUNCTION__, reverse);
   }// end of reverse switch.
   /*----------------------------------------------*/
   //   void *memcpy(void *dest, const void *src, size_t n);
   for (int i = 0; i < NSamples; i++)
      datax[i] = (float)datay[i];

   delete [] datay;

}
//=============================================
void KVDigitalFilter::ApplyTo(int* datax, const int NSamples, int reverse) const
{
   // Copiato +- da KVSignal.cxx
   // Diversa la convenzione per a0 b0!

   long double* datay = new long double[NSamples];
   int i = 0, k = 0;
   switch (reverse) {
      case 0:// direct
         for (i = 0; i < Ncoeff; i++) { // primo loop su Npunti
            datay[i] = a[0] * datax[i];
            for (k = 0; k < i; k++)
               datay[i] += a[k + 1] * datax[i - k - 1] + b[k + 1] * datay[i - k - 1];
         }
         for (i = Ncoeff; i < NSamples; i++) { //secondo loop. cambia l'indice interno.
            datay[i] = a[0] * datax[i];
            for (k = 0; k < Ncoeff - 1; k++)
               datay[i] += a[k + 1] * datax[i - k - 1] + b[k + 1] * datay[i - k - 1];
         }
         break; // end of direct
      case 1: //reverse: cut & paste from direct and NSamples-1-
         for (i = 0; i < Ncoeff; i++) { // primo loop su Npunti
            datay[NSamples - 1 - i] = a[0] * datax[NSamples - 1 - i];
            for (k = 0; k < i; k++)
               datay[NSamples - 1 - i] += a[k + 1] * datax[NSamples - 1 - (i - k - 1)]
                                          + b[k + 1] * datay[NSamples - 1 - (i - k - 1)];
         }
         for (i = Ncoeff; i < NSamples; i++) { //secondo loop. cambia l'indice interno.
            datay[NSamples - 1 - i] = a[0] * datax[NSamples - 1 - i];
            for (k = 0; k < Ncoeff - 1; k++)
               datay[NSamples - 1 - i] += a[k + 1] * datax[NSamples - 1 - (i - k - 1)]
                                          + b[k + 1] * datay[NSamples - 1 - (i - k - 1)];
         }
         break;
      case -1: // bidirectional
         ApplyTo(datax, NSamples, 0);
         ApplyTo(datax, NSamples, 1);
         return;
      default:
         printf("ERROR in %s: reverse=%d not supported\n", __PRETTY_FUNCTION__, reverse);
   }// end of reverse switch.
   /*----------------------------------------------*/
   //   void *memcpy(void *dest, const void *src, size_t n);
   for (int i = 0; i < NSamples; i++)
      datax[i] = (int)datay[i];
   delete [] datay;

}
/**********************************************/
void KVDigitalFilter::Compress()
{
   // shorten filter. No deallocation of memory.

   int M = Ncoeff;
   //  printf("COmpress: %d\n", M);
   const double zero_level = 1e-20;
   for (; M >= 1; M--) {
      if (fabs(a[M - 1]) > zero_level || fabs(b[M - 1]) > zero_level)
         break;
   }
/////  printf("Compress: from %d to %d\n",Ncoeff,M);

   for (int i = 0; i < M; i++) {
      if (fabs(a[i]) < zero_level)
         a[i] = 0;
      if (fabs(b[i]) < zero_level)
         b[i] = 0;
   }

   b[0] = 0;

   Ncoeff = M; // le lascio allocate, ma non le usero' nei conti...

}


/***************************************************/
/***************************************************/
KVDigitalFilter KVDigitalFilter::CombineStagesMany(const KVDigitalFilter* f1, const KVDigitalFilter* f2,
      const KVDigitalFilter* f3, const KVDigitalFilter* f4,
      const KVDigitalFilter* f5, const KVDigitalFilter* f6,
      const KVDigitalFilter* f7, const KVDigitalFilter* f8,
      const KVDigitalFilter* f9, const KVDigitalFilter* f10)
{
   // se ne devi combinare + di 1 IN CASCATA!
   // per fare parallelo usare l'altro...
   const KVDigitalFilter* filters[10] = {f1, f2, f3, f4, f5, f6, f7, f8, f9, f10};
   KVDigitalFilter total = BuildUnity(f1->tau_clk);
   for (int i = 0; i < 10; i++)
      if (filters[i] != NULL) {
         total = CombineStages(&total, filters[i]);
      }
   total.Compress();
   return total;
}
/***************************************************/
/***************************************************/
KVDigitalFilter KVDigitalFilter::CombineStages(const KVDigitalFilter& f1,
      const KVDigitalFilter& f2,
      int parallel)
{
   return CombineStages(&f1, &f2, parallel);
}
/***************************************************/
KVDigitalFilter KVDigitalFilter::CombineStages(const KVDigitalFilter* f1,
      const KVDigitalFilter* f2,
      int parallel)
{
   if (fabs(f1->tau_clk - f2->tau_clk) > 1e-4) {
      printf("ERROR in %s: cannot combine with different taus! %e != %e\n",
             __PRETTY_FUNCTION__, f1->tau_clk, f2->tau_clk);
      return KVDigitalFilter(f1->tau_clk);
   }
   int Nmax = f1->Ncoeff;
   int __N = f2->Ncoeff;
   if (__N > Nmax)
      Nmax = __N;

   Nmax *= 2;

   /******** now N is the max *********/
   double a1[Nmax], a2[Nmax], a3[2 * Nmax];
   double b1[Nmax], b2[Nmax], b3[2 * Nmax];
   for (int i = 0; i < Nmax; i++) {
      a1[i] = f1->GetXcoeff(i);
      b1[i] = f1->GetYcoeff(i);
      a2[i] = f2->GetXcoeff(i);
      b2[i] = f2->GetYcoeff(i);
//////      printf("%d: %e %e %e %e\n",i, a1[i], b1[i], a2[i], b2[i]);
   }

   for (int i = 0; i < Nmax; i++) {
      b2[i] = - b2[i];
      b1[i] = - b1[i];
   }
   b1[0] = 1;
   b2[0] = 1;
   for (int i = 0; i < 2 * Nmax; i++) {
      a3[i] = 0;
      b3[i] = 0;
      for (int j = 0; j < Nmax; j++) {
         if (j > i || (i - j) >= Nmax) continue;
         if (!parallel)
            a3[i] = a3[i] + a1[j] * a2[i - j];
         else
            a3[i] = a3[i] + a1[j] * b2[i - j] + a2[j] * b1[i - j];
         b3[i] = b3[i] + b1[j] * b2[i - j];
      }
   }

   for (int i = 0; i < Nmax; i++) b3[i] = -b3[i];
   b3[0] = 0;
   /*
       for(int i=0; i< Nmax*2; i++){
       printf("Combine stages, output: a3[%2d]=%15.8e b3[%2d]=%15.8e\n",
       i,a3[i],i,b3[i]);
       }
   */
   /********** COPY INTO NEW FILTER ****************/
   KVDigitalFilter filter(f1->tau_clk, 2 * Nmax, a3, b3);
   filter.Compress();
   return filter;

}
/***********************************************/
double KVDigitalFilter::ComputeGainDC()
{
   // Gain of filter at f=0
   // eq. 33.7 del solito libro
   double num = 0, den = 0;
   for (int i = 0; i < Ncoeff; i++) {
      num += a[i];
      den += b[i];
   }
   return num / (1 - den);
}
/***********************************************/
double KVDigitalFilter::ComputeGainNy()
{
   // Gain of filter at f=Nyquist (0.5)
   // eq. 33.8 del solito libro
   double num = 0, den = 0;
   int sign = 1;
   for (int i = 0; i < Ncoeff; i++) {
      num += a[i] * sign;
      den += b[i] * sign;
      sign *= -1;
   }
   return num / (1 - den);
}
KVDigitalFilter KVDigitalFilter ::BuildUnity(const double&  tau_clk)
{
   double a[2] = {1, 0};
   double b[2] = {0, 0};
   KVDigitalFilter filter(tau_clk, 2, a, b);
   filter.Compress();
   return filter;
}
KVDigitalFilter KVDigitalFilter ::BuildIntegrator(const double&  tau_clk)
{
   double a[2] = {1, 0};
   double b[2] = {0, 1};
   KVDigitalFilter filter(tau_clk, 2, a, b);
   return filter;
}
/***********************************************/
void KVDigitalFilter::PrintCoeffs() const
{
   printf("------------------------------------------------------\n");
   printf("Coefficients valid with tau_clk=%.3f ns.\n", tau_clk);
   int tot = 0;
   for (int i = 0; i < Ncoeff; i++) {
      if (i == 0)
         printf("*** Xcoeff[%2d]= %20.20e -\n", i, a[i]);
      else
         printf("*** Xcoeff[%2d]= %20.20e Ycoeff[%2d]= %20.20e\n", i, a[i], i,  b[i]);
      if (fabs(a[i]) > 1e-20) tot++;
      if (fabs(b[i]) > 1e-20) tot++;
   }
   printf("TOTAL: %d coefficients (a+b).\n", tot);
   printf("(DspGuide : Xcoeff <-> a\n"
          "            Ycoeff <-> b\n"
          " Oppenheim: Xcoeff <-> b\n"
          "            Ycoeff <-> a)\n"
          "------------------------------------------------------\n"
         );
}
void KVDigitalFilter::PrintCoeffsDSP() const
{
   printf("------------------------------------------------------\n");
   printf("DSP Coefficients valid with tau_clk=%.3f ns.\n", tau_clk);
   int tot = 0;
   for (int i = 0; i < Ncoeff; i++) {
      if (i == 0)
         printf("*** Xcoeff[%2d]= 0x%6.6x    -\n", i, Double2DSP(a[i]));
      else
         printf("*** Xcoeff[%2d]= 0x%6.6x    Ycoeff[%2d]= 0x%6.6x\n",
                i, Double2DSP(a[i]), i,  Double2DSP(b[i]));
      if (fabs(a[i]) > 1e-20) tot++;
      if (fabs(b[i]) > 1e-20) tot++;
   }
   printf("TOTAL: %d coefficients (a+b).\n", tot);
   printf("(DspGuide : Xcoeff <-> a\n"
          "            Ycoeff <-> b\n"
          " Oppenheim: Xcoeff <-> b\n"
          "            Ycoeff <-> a)\n"
          "------------------------------------------------------\n"
         );
}
/***********************************************/
void KVDigitalFilter::PrintCoeffs_AsC() const
{
   printf("/****** filter valid for %.1f tau_clk *********/\n", tau_clk);
   printf(" int Ncoeff=%d;\n", Ncoeff);
   printf(" double Xcoeffs[%d]={\n", Ncoeff);
   for (int i = 0; i < Ncoeff; i++)
      printf("\t%20.20e%s\n", a[i], i == Ncoeff - 1 ? "};" : ",");
   printf(" double Ycoeffs[%d]={\n", Ncoeff);
   for (int i = 0; i < Ncoeff; i++)
      printf("\t%20.20e%s\n", b[i], i == Ncoeff - 1 ? "};" : ",");
   printf(" KVDigitalFilter filter(%d, Xcoeffs, Ycoeffs, %f);\n", Ncoeff, tau_clk);
   printf("/**********************************************/\n");
}
/***********************************************/
void KVDigitalFilter::Quantize(int nbits, int use_pow2,
                               double* xgain, int* x_out, int* y_out, int* x_scale, int* y_scale)
{

   if (nbits <= 0) return;
   Compress();
   double* x = GetXarray();
   double* y = GetYarray();

   double Nlevel = pow(2.f, nbits - 1) - 1; // non posso fare con << xe' non funziona se nbits==32 !!
   // normalizzazione delle X a 1:
   double factor = fabs(x[0]);
   //  int imax=0;
   for (int i = 1; i < GetNCoeff(); i++)
      if (fabs(x[i]) > factor) {
         factor = fabs(x[i]);
         // imax=i;
      }
   if (factor <= 1e-16) {
      printf("ERROR in %s: factor=%e?\n", __PRETTY_FUNCTION__, factor);
      factor = 1;
   }
   if (xgain != NULL) {
      *xgain = factor;
   }
   if (x_out  != NULL) memset(x_out, 0, GetNCoeff()*sizeof(int));
   if (y_out  != NULL) memset(y_out, 0, GetNCoeff()*sizeof(int));
   if (x_scale != NULL) memset(x_scale, 0, GetNCoeff()*sizeof(int));
   if (y_scale != NULL) memset(y_scale, 0, GetNCoeff()*sizeof(int));


   ///  if(stampa) printf("x gain factor: %e\n",factor);
   for (int i = 0; i < GetNCoeff(); i++) {
      x[i] /= factor;


      // quantizziamo il coefficiente N-esimo...
      int N = 0;
      if (use_pow2 == 1) {
         N = (int)ceil(log(fabs(x[i])) / log(2.));
         N = (1 << N);
      } else
         N = (int)ceil(fabs(x[i])); // numero intero.
      if (N == 0) {
         x[i] *= factor;
         continue;
      }
      x[i] /= N;
      //-- adesso i bits: adesso y[i] e' un numero tra -1..1
      int k = (int)rint(x[i] * Nlevel);
      if (k >= Nlevel) k = (int)(Nlevel - 1.);
      if (k < -Nlevel) k = (int)(-Nlevel);
      //      if(stampa && (k&0x00FFFFFF)!=0) printf("x[%d] 0x %6.6Xoo  with factor %d %s\n",i,k&0x00FFFFFF,N,imax==i?" << MAX" : "");
      if (x_out != NULL) {
///     printf("%d: k=%d %p\n",i,k,k);
         x_out[i] = k;
      }
      if (x_scale != NULL) {
         x_scale[i] = N;
      }

      x[i] = (double)k / (double)Nlevel;
      //--- ripristino N
      x[i] *= N;
      //--- ripristino factor
      x[i] *= factor;

   }

   for (int i = 1; i < GetNCoeff(); i++) {

      //      printf("Now coeff %d\n",i);
      // quantizziamo il coefficiente N-esimo...

      int N = 0;
      if (use_pow2 == 1) {
         N = (int)ceil(log(fabs(y[i])) / log(2.));
         N = (1 << N);
      } else
         N = (int)ceil(fabs(y[i])); // numero intero.
      if (N == 0) {
         y[i] *= factor;
         continue;
      }
      y[i] /= N;
      //-- adesso i bits: adesso y[i] e' un numero tra -1..1
      int k = (int)rint(y[i] * Nlevel);
      if (k >= Nlevel) k = (int)(Nlevel - 1);
      if (k < -Nlevel) k = (int)(-Nlevel);
      ////      if(stampa && k!=0) printf("y[%d] 0x %6.6Xoo  with factor %d\n",i,k&0x00FFFFFF,N);
      if (y_out != NULL)
         y_out[i] = k;
      if (y_scale != NULL)
         y_scale[i] = N;
      y[i] = (double)k / (double)Nlevel;
      //--- ripristino N
      y[i] *= N;

   }

}
/*******************************************************/
void KVDigitalFilter::Draw(Option_t* option)
{
   int Nbins = 10000;

   TH1F* h1 = new TH1F("hKVDigitalFilter", "Filter response", Nbins, 0, 1. / GetTauClk() / 2.*1000.);
   h1->SetStats(kFALSE);
   h1->GetXaxis()->SetTitle("Frequency (MHz)");
   printf("NCoeffs: %d\n", GetNCoeff());
   for (int k = 0; k < Nbins; k++) {
      double angolo = 3.14159265358979312 * k / (Nbins - 1);

      double numRE = 0;
      double numIM = 0;
      double denRE = 0;
      double denIM = 0;

      for (int i = 0; i < GetNCoeff(); i++) {
         double a_re = cos(angolo * i);
         double a_im = sin(angolo * i);

         numRE += GetXcoeff(i) * a_re;
         numIM += GetXcoeff(i) * a_im;
         if (i == 0) {
            denRE += 1;
         } else {
            denRE -=  GetYcoeff(i) * a_re;
            denIM -=  GetYcoeff(i) * a_im;
         }
      }
      /*-------------------------*/
      double val = sqrt((numRE * numRE + numIM * numIM) / ((denRE * denRE + denIM * denIM)));
      h1->SetBinContent(k + 1, val);
      /*-------------------------*/
   }
   h1->Draw(option);
}
/**************************************/
//void KVDigitalFilter::DrawResponse(Option_t* option)
//{
//  KVSignal s(tau_clk);
//  s.SetMaxT(10000); // 10 us moooolto arbitrariamente...
//  s.SetData((int)(100/tau_clk), 1);
//  ApplyTo(&s);
//  s.Draw(option);
//}
///**************************************/
//void KVDigitalFilter::DrawStepResponse(Option_t* option)
//{
//  KVSignal s;
//  s.SetChannelWidth(tau_clk);
//  s.SetMaxT(10000); // 10 us moooolto arbitrariamente...
//  s.SetData((int)(100/tau_clk), 1);
//  s.Integra();
//  ApplyTo(&s);
//  s.Draw(option);
//}
/**************************************/
KVDigitalFilter KVDigitalFilter::BuildInverse(KVDigitalFilter* filter)
{
   KVDigitalFilter inv_filter(filter->GetTauClk());
   inv_filter.Alloc(filter->GetNCoeff());
   inv_filter.SetXcoeff(0, 1. / filter->GetXcoeff(0));
   for (int i = 1; i < filter->GetNCoeff(); i ++) {
      inv_filter.SetXcoeff(i, - filter->GetYcoeff(i)*inv_filter.GetXcoeff(0));
      inv_filter.SetYcoeff(i, - filter->GetXcoeff(i)*inv_filter.GetXcoeff(0));
   }
   return inv_filter;
}


/********************************************/

// function to load filter coefficients
int KVDigitalFilter::ReadMatlabFIR(char* filecoeff)
{

   //  FILE *fin = fopen("notch_coeffs.txt","r");
   FILE* fin = fopen(filecoeff, "r");
   int i = 0;
   int n;
   if (fscanf(fin, "%d", &n)) {
      ;
   }
   Alloc(n);
   //  while(fscanf(fin, "%lg",&b[i])!=EOF){
   for (i = 0; i < n; i++) {
      if (fscanf(fin, "%lg", &a[i])) {
         ;
      }
      b[i] = 0.0;
      printf("%s: i=%d  a[%d]=%20.20e\n", __PRETTY_FUNCTION__, i, i, a[i]);
      //i++;
   }
   printf("%s: Read %d coefficients\n", __PRETTY_FUNCTION__, i);
   Ncoeff = i ;
   fclose(fin);
   return Ncoeff;
}

/********************************************/

// function to load filter coefficients
int KVDigitalFilter::WriteMatlabFIR(char* filecoeff)
{

   FILE* fout = fopen(filecoeff, "w");
   int i = 0;
   fprintf(fout, "%d\n", Ncoeff);
   for (i = 0; i < Ncoeff; i++) {
      fprintf(fout, "%20.20e\n", a[i]);
   }
   printf("%s: Written %d coefficients\n", __PRETTY_FUNCTION__, i);
   fclose(fout);
   return Ncoeff;
}


// =========================== ApplyTo per filtri FIR:
//  eliminata ricorsivita' per risparmiare tempo:
//=============================================
void KVDigitalFilter::FIRApplyTo(float* datax, const int NSamples, int reverse) const
{
   // Copiato +- da KVSignal.cxx
   // Diversa la convenzione per a0 b0!

   long double* datay = new long double[NSamples];
   int i = 0, k = 0;
   switch (reverse) {
      case 0:// direct
         for (i = 0; i < Ncoeff; i++) { // primo loop su Npunti
            datay[i] = a[0] * datax[i];
            for (k = 0; k < i; k++)
               datay[i] += a[k + 1] * datax[i - k - 1];
         }
         for (i = Ncoeff; i < NSamples; i++) { //secondo loop. cambia l'indice interno.
            datay[i] = a[0] * datax[i];
            for (k = 0; k < Ncoeff - 1; k++)
               datay[i] += a[k + 1] * datax[i - k - 1];
         }
         break; // end of direct
      case 1: //reverse: cut & paste from direct and NSamples-1-
         for (i = 0; i < Ncoeff; i++) { // primo loop su Npunti
            datay[NSamples - 1 - i] = a[0] * datax[NSamples - 1 - i];
            for (k = 0; k < i; k++)
               datay[NSamples - 1 - i] += a[k + 1] * datax[NSamples - 1 - (i - k - 1)];
         }
         for (i = Ncoeff; i < NSamples; i++) { //secondo loop. cambia l'indice interno.
            datay[NSamples - 1 - i] = a[0] * datax[NSamples - 1 - i];
            for (k = 0; k < Ncoeff - 1; k++)
               datay[NSamples - 1 - i] += a[k + 1] * datax[NSamples - 1 - (i - k - 1)];
         }
         break;
      case -1: // bidirectional
         FIRApplyTo(datax, NSamples, 0);
         FIRApplyTo(datax, NSamples, 1);
         return;
      default:
         printf("ERROR in %s: reverse=%d not supported\n", __PRETTY_FUNCTION__, reverse);
   }// end of reverse switch.
   /*----------------------------------------------*/
   //   void *memcpy(void *dest, const void *src, size_t n);
   for (int i = 0; i < NSamples; i++)
      datax[i] = (float)datay[i];

   delete [] datay;

}
//=============================================
void KVDigitalFilter::FIRApplyTo(double* datax, const int NSamples, int reverse) const
{
   // Copiato +- da KVSignal.cxx
   // Diversa la convenzione per a0 b0!

   long double* datay = new long double[NSamples];
   int i = 0, k = 0;
   switch (reverse) {
      case 0:// direct
         for (i = 0; i < Ncoeff; i++) { // primo loop su Npunti
            datay[i] = a[0] * datax[i];
            for (k = 0; k < i; k++)
               datay[i] += a[k + 1] * datax[i - k - 1];
         }
         for (i = Ncoeff; i < NSamples; i++) { //secondo loop. cambia l'indice interno.
            datay[i] = a[0] * datax[i];
            for (k = 0; k < Ncoeff - 1; k++)
               datay[i] += a[k + 1] * datax[i - k - 1];
         }
         break; // end of direct
      case 1: //reverse: cut & paste from direct and NSamples-1-
         for (i = 0; i < Ncoeff; i++) { // primo loop su Npunti
            datay[NSamples - 1 - i] = a[0] * datax[NSamples - 1 - i];
            for (k = 0; k < i; k++)
               datay[NSamples - 1 - i] += a[k + 1] * datax[NSamples - 1 - (i - k - 1)];
         }
         for (i = Ncoeff; i < NSamples; i++) { //secondo loop. cambia l'indice interno.
            datay[NSamples - 1 - i] = a[0] * datax[NSamples - 1 - i];
            for (k = 0; k < Ncoeff - 1; k++)
               datay[NSamples - 1 - i] += a[k + 1] * datax[NSamples - 1 - (i - k - 1)];
         }
         break;
      case -1: // bidirectional
         FIRApplyTo(datax, NSamples, 0);
         FIRApplyTo(datax, NSamples, 1);
         return;
      default:
         printf("ERROR in %s: reverse=%d not supported\n", __PRETTY_FUNCTION__, reverse);
   }// end of reverse switch.
   /*----------------------------------------------*/
   //   void *memcpy(void *dest, const void *src, size_t n);
   memcpy(datax, datay, NSamples * sizeof(double));
   delete [] datay;

}
