/*
different digital signal filters
classes copied from FClasses of INFN-Firenze
*/


#include <math.h>
#include <stdlib.h>
#include <KVBase.h>
#include <KVSignal.h>

#ifndef __KVDigitalFilter_H
#define __KVDigitalFilter_H

class    KVDigitalFilter: public KVBase {
public:
   KVDigitalFilter(const double& tau = 10); //sconsigliato. usa Build*
   KVDigitalFilter(const double& tau, int N, const  double* Xcoeffs, const double* Ycoeffs);

   KVDigitalFilter(const    KVDigitalFilter&);
   KVDigitalFilter operator  = (const    KVDigitalFilter&);

   virtual ~   KVDigitalFilter();
   void PrintCoeffs() const;
   void PrintCoeffsDSP() const;
   void PrintCoeffs_AsC() const; //printed like a C array;

   virtual void Draw(Option_t* option = ""); // DRAW modulo!
//    virtual void DrawResponse(Option_t* option=""); // DRAW signal
//    virtual void DrawStepResponse(Option_t* option=""); // DRAW signal



   /******************** QUANTIZZA *************************************/
   void Quantize(int nbits, int use_pow2 = 0, double* xgain = NULL, int* x_out = NULL, int* y_out = NULL, int* x_scale = NULL, int* y_scale = NULL);

   /************************** CREATORI ********************************/
   static    KVDigitalFilter BuildRCLowPassDeconv(const double&  tau_usec, const double&  tau_clk);
   static    KVDigitalFilter BuildRCLowPass(const double&  tau_usec, const double&  tau_clk);
   static    KVDigitalFilter BuildRCHighPass(const double&  tau_usec, const double&  tau_clk);
   static    KVDigitalFilter BuildRCHighPassWithPZ(const double&  tau_usec, const double&  preamp_decay_usec, const double&  tau_clk);
   static    KVDigitalFilter BuildChebyshev(const double& freq_cutoff_mhz, int is_highpass,
         const double& percent_ripple, int npoles,
         const double& tau_clk);
   static    KVDigitalFilter BuildUnity(const double&  tau_clk);
   static    KVDigitalFilter BuildIntegrator(const double& tau_clk);

   static    KVDigitalFilter BuildInverse(KVDigitalFilter* filter);
   /************************** UTILS ***********************************/
   //-- conversion to/from DSP 1.15? notation
   static int Double2DSP(const double& val)
   {
      return (int)(val * 32768 * 256 + 0.5);
   }
   static double DSP2Double(const int val)
   {
      return val / (32768.*256.);
   }

   void Compress();// shorten removing unused coeffs;

   static    KVDigitalFilter CombineStages(const    KVDigitalFilter& f1,
                                           const    KVDigitalFilter& f2,
                                           int parallel = 0); //default is cascade

   static    KVDigitalFilter CombineStages(const    KVDigitalFilter* f1,
                                           const    KVDigitalFilter* f2,
                                           int parallel = 0); //default is cascade
   // se ne devi combinare + di 1 IN CASCATA!
   static    KVDigitalFilter CombineStagesMany(const    KVDigitalFilter* f1, const    KVDigitalFilter* f2,
         const    KVDigitalFilter* f3 = NULL, const    KVDigitalFilter* f4 = NULL,
         const    KVDigitalFilter* f5 = NULL, const    KVDigitalFilter* f6 = NULL,
         const    KVDigitalFilter* f7 = NULL, const    KVDigitalFilter* f8 = NULL,
         const    KVDigitalFilter* f9 = NULL, const    KVDigitalFilter* f10 = NULL);


   double ComputeGainDC(); // gain at 0 frequency
   double ComputeGainNy(); // gain at Nyquist frequency (0.5)
   inline void Multiply(double v)
   {
      for (int i = 0; i < Ncoeff; i++) a[i] *= v;
   }

   /*------------*/
   inline const double& GetXcoeff(int i) const
   {
      static const double zero = 0;
      if (i >= 0 && i < Ncoeff) return a[i];
      return zero;
   }
   inline const double& GetYcoeff(int i) const
   {
      static const double zero = 0;
      if (i >= 0 && i < Ncoeff) return b[i];
      return zero;
   }
   inline void SetXcoeff(int i, const double& val)
   {
      if (i >= 0 && i < Ncoeff)
         a[i] = val;
      else
         printf("ERROR in %s: index %d out of bounds (0..%d).\n",
                __PRETTY_FUNCTION__, i, Ncoeff - 1);
   }
   inline void SetYcoeff(int i, const double&  val)
   {
      if (i >= 1 && i < Ncoeff)
         b[i] = val;
      else
         printf("ERROR in %s: index %d out of bounds (0..%d).\n",
                __PRETTY_FUNCTION__, i, Ncoeff - 1);
   }
   inline double* GetXarray()
   {
      return a;
   }
   inline double* GetYarray()
   {
      return b;
   }


   inline const double& GetTauClk()
   {
      return tau_clk;
   }
   inline int GetNCoeff()
   {
      return Ncoeff;
   }
   /************************ FILTRAGGIO ********************************/
   void ApplyTo(double* data, const int N, int reverse = 0) const;
   void ApplyTo(float*  data, const int N, int reverse = 0) const;
   void ApplyTo(int*    data, const int N, int reverse = 0) const;
   void FIRApplyTo(double* datax, const int NSamples, int reverse) const;
   void FIRApplyTo(float* datax, const int NSamples, int reverse) const;
   inline void ApplyTo(KVSignal* s, int reverse = 0) const
   {
      if (fabs(s->GetChannelWidth() - tau_clk) > 1e-6) {
         printf("ERROR in %s: different tau_clk! %e != %e\n",
                __PRETTY_FUNCTION__, s->GetChannelWidth(),  tau_clk);
         return;
      }
      ApplyTo(s->GetArray()->GetArray(), s->GetNSamples(), reverse);
   }
   inline void FIRApplyTo(KVSignal* s, int reverse = 0) const
   {
      if (fabs(s->GetChannelWidth() - tau_clk) > 1e-6) {
         printf("ERROR in %s: different tau_clk! %e != %e\n",
                __PRETTY_FUNCTION__, s->GetChannelWidth(),  tau_clk);
         return;
      }
      FIRApplyTo(s->GetArray()->GetArray(), s->GetNSamples(), reverse);
   }
   void Alloc(const int Ncoeff);
   void Azzera();
   int ReadMatlabFIR(char* filecoeff);
   int WriteMatlabFIR(char* filecoeff);

protected:
   static void ComputeChebyshevCoeffs_serv(const double& fc, const double&   lh,
                                           const double& pr, const double&   np,
                                           int p,
                                           double* a0, double* a1, double* a2,
                                           double* b1, double* b2);

   static void ComputeChebyshevCoeffs(const double& freq_cutoff,
                                      int is_highpass, const double& percent_ripple, int npoles,
                                      double* a, double* b);



   /***************************** VARIABILI ********************/
   double* a; //coefficients.
   double* b;
   int Ncoeff;

   double tau_clk;

   ClassDef(KVDigitalFilter, 1)   // FIASCO: Class for digital filtering
};

#endif
