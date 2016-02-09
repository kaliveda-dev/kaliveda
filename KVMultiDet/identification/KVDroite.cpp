#include "KVDroite.h"

using namespace std;

ClassImp(KVDroite)

KVDroite::KVDroite()
{
   _a0      = 0.;
   _theta   = 0.;
   _is_initialized = false;
   _f       = 0;
}

KVDroite::KVDroite(double a0_, double theta_)
{
   _a0      = a0_;
   _theta   = theta_;
   _is_initialized = false;
   _f       = 0;
   SetName(Form("y=%lf+x*tan(%lf)", _a0, _theta));
   Init();
}

KVDroite::KVDroite(double x0_, double y0_, double theta_)
{
   _a0      = y0_ - x0_ * TMath::Tan(TMath::DegToRad() * theta_);
   _theta   = theta_;
   _is_initialized = false;
   _f       = 0;
   SetName(Form("y=%lf+x*tan(%lf)", _a0, _theta));
   Init();
}

double KVDroite::GetA0()
{
   return _a0;
}

double KVDroite::GetTheta()
{
   return _theta;
}

TF1* KVDroite::GetFunction()
{
   if (_is_initialized) return _f;
   cout << "ERROR: KVDroite::GetFunction: I'm not initialized !" << endl;
   return 0;
}

void KVDroite::Init(double min_, double max_)
{
   if ((_a0 == 0) && (_theta == 0.)) {
      cout << "ERROR: KVDroite::Init: you have to define theta and a0 befor initialisation !" << endl;
      return;
   } else if (_is_initialized) {
      cout << "WARNING: KVDroite::Init: already initialized !" << endl;
   }
   SetName(Form("y=%lf+x*tan(%lf)", _a0, _theta));
   _f = new TF1(Form("f_%s", GetName()), Form("%lf+x*%lf", _a0, TMath::Tan(TMath::DegToRad()*_theta)), min_, max_);
   _is_initialized = true;
}

void KVDroite::SetA0(double a0)
{
   _a0 = a0;
}

void KVDroite::SetTheta(double theta)
{
   _theta = theta;
}
