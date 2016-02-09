
#ifndef __KVDROITE_H
#define __KVDROITE_H

#include <TF1.h>
#include <TMath.h>
#include <TNamed.h>

#include <Riostream.h>


class KVDroite : public TNamed {
private:
   double _a0;
   double _theta;
   bool _is_initialized;
   TF1* _f;

public:
   KVDroite();
   KVDroite(double a0_, double theta_);
   KVDroite(double x0_, double _y0, double theta_);

   double GetA0();
   double GetTheta();
   TF1* GetFunction();

   void Init(double min_ = 0., double max_ = 4096.);

   void SetA0(double a0);
   void SetTheta(double theta);

   ClassDef(KVDroite, 1) // a faire !
};


#endif






