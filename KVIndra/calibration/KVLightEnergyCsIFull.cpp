//Created by KVClassFactory on Fri Feb  8 09:58:44 2013
//Author: dgruyer

#include "KVLightEnergyCsIFull.h"
using namespace std;

ClassImp(KVLightEnergyCsIFull)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVLightEnergyCsIFull</h2>
<h4>Light-energy calibration for CsI detectors uding exact expression</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////


//________________________________________________________________
Double_t KVLightEnergyCsIFull::GetDeltaFraction(Double_t beta, Double_t beta_delta)
{
   double frac = 0.;
   double rap = 1.;

   if (beta > beta_delta) rap = TMath::Power(beta / beta_delta, 2.);
   else return 0.;

   double deno = log(1.022e6 / (16.*TMath::Power(fZmed, 0.9)) * TMath::Power(beta_delta, 2)) + log(rap);
   frac = 0.5 * log(rap) / deno;

   return frac;
}

//________________________________________________________________
Double_t KVLightEnergyCsIFull::dLightIntegralApprox(double* x , double* par)
{
   double e = x[0];

   double zfit = par[4];
   double afit = par[5];

   double ag = par[0];
   double ar = par[1];
   double an = par[2];
   double e_delta = par[3];

   double m = float(afit) * u;
   if (zfit == 2. && afit == 4.) m = 3727.4;
   if ((zfit == 1.) && (afit == 1.)) m = 938.;

   double beta = 0.;
   double fraction = 0.;
   double dlum = 0.;

   double beta_delta = sqrt(2.*e_delta / u);

   beta = sqrt(1. - m * m / ((e + m) * (e + m)));

   fraction = GetDeltaFraction(beta, beta_delta);
   if (fraction > 1.) fraction = 1.;
   if (fraction < 0.) fraction = 0.;


   double se = sp_e(zfit, afit, e);
   double sn = sp_n(zfit, afit, e);
   double deno = 1. + ar * se + an * sn;
   double deno2 = 1. + (1. - fraction) * ar * se + an * sn;
   double raps = 1. + sn / se;
   if (e <= e_delta * afit) {
      if (se > 0. && raps > 0. && deno > 0.) dlum = (1. + an * sn) / (deno * raps);
   } else {
      if (se > 0. && raps > 0. && deno > 0.) {
         dlum = (1. + an * sn) * (1. - fraction) / (deno2 * raps) + fraction / raps;
      }
   }

   dlum *= ag;
   return dlum;
}

//________________________________________________________________
Double_t KVLightEnergyCsIFull::dLightIntegral(double* x , double* par)
{
   double e = x[0];

   double zfit = par[4];
   double afit = par[5];

   double ag = par[0];
   double ar = par[1];
   double an = par[2];
   double e_delta = par[3];

   double m = afit * u;
   if ((zfit == 2.) && (afit == 4.)) m = 3727.4;
   if ((zfit == 1.) && (afit == 1.)) m = 938.;

   double beta = 0.;
   double fraction = 0.;
   double dlum = 0.;

   double beta_delta = sqrt(2.*e_delta / u);

   beta = sqrt(1. - m * m / ((e + m) * (e + m)));
   fraction = GetDeltaFraction(beta, beta_delta);
   if (fraction > 1.) fraction = 1.;
   if (fraction < 0.) fraction = 0.;

   double se = sp_e(zfit, afit, e);
   double sn = sp_n(zfit, afit, e);
   double raps = 1. + sn / se;
   double arg  = 1. - ar * se / (1. + an * sn + ar * se);
   double fact = 1.;

   if (e <= e_delta * afit) {
      if ((arg > 0.) && (ar * se > 0.) && (raps > 0.)) dlum = -fact * TMath::Log(arg) / (raps * ar * se);
   } else {
      Double_t y = 1 - fraction * ar * se / (1. + an * sn + ar * se);
      arg = 1. - y * ar * se / (1. + an * sn + ar * se);
      if ((arg > 0.) && (ar * se > 0.) && (raps > 0.)) dlum = -fact * (1 - fraction) * TMath::Log(arg) / (raps * ar * se * y) + fraction / raps;
   }

   dlum *= ag;
   return dlum;
}

//________________________________________________________________
Double_t KVLightEnergyCsIFull::sp_e(double z, double a, double e)
{
   if (e < 1.e-4) return 0.;
   Double_t se = fMaterialTable->GetStoppingFunction(z, a)->Eval(e); //-sp_n(z,a,e); // in units of MeV/(g/cm**2)
   se *= 1.e-3;                                                                    // in units of MeV/(mg/cm**2)
   return se - sp_n(z, a, e);
}

//________________________________________________________________
Double_t KVLightEnergyCsIFull::sp_n(double z, double a, double e)
{
   //    return 0.;
   if (e < 1.e-4) return 0.;
   double ered = 32.53 * fAmed * e * 1000. / (z * fZmed * (a + fAmed) * sqrt(TMath::Power(z, 2. / 3.) + TMath::Power(fZmed, 2. / 3.)));
   double sn = 1.593 * sqrt(ered);
   if (ered >= 0.01 && ered <= 10.) sn = 1.7 * sqrt(ered) * (log(ered + exp(1.)) / (1. + 6.8 * ered + 3.4 * TMath::Power(ered, 1.5)));
   if (ered > 10.) sn = log(0.47 * ered) / (2.*ered);
   //
   // This is in eV/(10**15 atoms/cm**2)
   //
   sn = sn * 8.462 * z * fZmed * a / ((a + fAmed) * sqrt(TMath::Power(z, 2. / 3.) + TMath::Power(fZmed, 2. / 3.)));
   //
   // convert in MeV/(mg/cm**2)
   //
   sn = sn * 6.022 / (10.*fAmed);
   //
   // Convert in MeV/micron
   //
   //if ( fZmed==zcsi ) sn*=0.4510;
   //else if ( fZmed==14. ) sn=sn*0.2330;
   //
   //    Info("sp_n","method called : e=%f Zmed=%f Amed=%f sp_n=%f",e,fZmed,fAmed,sn);
   return sn;
}

//________________________________________________________________
Double_t KVLightEnergyCsIFull::GetLight(double* x, double* par)
{
   // x[0] = energie (MeV)
   // par[0] = pied
   // par[1] = a1
   // par[2] = a2
   // par[3] = a3
   // par[4] = a4
   // par[5] = Z
   // par[6] = A

   double emin    = 1.e-4; //arbitrary set to avoid divergence of de/dx
   double emax    = x[0];
   double epsilon = 1.;    //arbitrary set

#if ROOT_VERSION_CODE > ROOT_VERSION(5,99,01)
   // for compilation with latest ROOT svn trunk version called 5.99/01
   fDlight->SetParameters(par + 1);
   return par[0] + fDlight->Integral(emin, emax, epsilon);
#else
   return par[0] + fDlight->Integral(emin, emax, par + 1, epsilon);
#endif

}

//________________________________________________________________
Double_t KVLightEnergyCsIFull::GetLightApprox(double* x, double* par)
{
   //Calcul de la lumiere totale a partir de Z, A d'une particule et son energie -> copie de KVLightEnergyCsI->CalculeLumiere(...)
   //
   // x[0] = energie (MeV)
   // par[0] = pied
   // par[1] = a1
   // par[2] = a2
   // par[3] = a3
   // par[4] = a4
   // par[5] = Z
   // par[6] = A

   Double_t energie = x[0];

   Double_t Z = par[5];
   Double_t A = par[6];
   Double_t c1 = par[1];
   Double_t c2 = Z * Z * A * par[2];
   Double_t c3 = A * par[3];
   Double_t c4 = par[4];
   Double_t T = 8 * A;
   Double_t c4_new = c4 / (1. + TMath::Exp((c3 - energie) / T));
   Double_t c0 = c4 / (1. + TMath::Exp(c3 / T));

   Double_t lumcalc = c1 * energie;
   if (c2 > 0.0) {
      Double_t xm = -c1 * c0 * c2 * TMath::Log(c2 / (c2 + c3));
      lumcalc = lumcalc - c1 * c2 * TMath::Log(1. + energie / c2) + c1 * c2 * c4_new * TMath::Log((energie + c2) / (c3 + c2)) + xm;
   }

   return par[0] + lumcalc;
}

//________________________________________________________________
Double_t KVLightEnergyCsIFull::GetLightApproxSilicon(double* x, double* par)
{
   //Calcul de la lumiere totale a partir de Z, A d'une particule et son energie -> copie de ami_4 Lopez, Parlog LPCCaen
   //
   // x[0] = energie (MeV)
   // par[0] = pied
   // par[1] = a1
   // par[2] = a2
   // par[3] = a3
   // par[4] = a4
   // par[5] = Z
   // par[6] = A

   Double_t E = x[0];

   Double_t pied = par[0];
   Double_t a0   = par[1];
   Double_t a1   = par[2];
   Double_t a2   = par[3];
   Double_t a3   = par[4];

   Double_t z = par[5];
   Double_t a = par[6];

   Double_t raz2 = a * z * z * a1;
   Double_t si = a0 * (E - raz2 * log(1. + E / raz2) + raz2 * log((E + raz2) / (a3 * a + raz2)) - (1. - a2) * (1. - a2) * raz2 * log((E + (1. - a2) * raz2) / (a3 * a + (1. - a2) * raz2)));

   return pied + si;
}

//________________________________________________________________
KVLightEnergyCsIFull::KVLightEnergyCsIFull(): KVCalibrator(5)
{
   // Default constructor
   fDetector = 0;
   //    init();
}

//________________________________________________________________
KVLightEnergyCsIFull::KVLightEnergyCsIFull(KVDetector* kvd, Int_t lightFormula): KVCalibrator(5) // : KVCalibrator(kvd)
{
   SetDetector(kvd);
   SetLightFormula(lightFormula);
   init();
}

//________________________________________________________________
void KVLightEnergyCsIFull::init()
{
   //default initialisations
   SetType("Light-Energy CsI");
   if (fDetector) fMaterialTable = fDetector->GetRangeTable()->GetMaterial(fDetector->GetMaterialName());
   else Error("init", "No detector provided !");
   SetA(1);
   SetZ(1);
   fZmed = fDetector->GetZ();
   fAmed = fDetector->GetMass();
   u = 931.5;
   fDlight = 0;

   switch (fLightFormula) {
      case kExact :
         fDlight = new TF1("fDlight_CsI", this, &KVLightEnergyCsIFull::dLightIntegral, 0., 10000., 6, "KVLightEnergyCsIFull", "dLightIntegral");
         fLight  = new TF1("fLight_CsI" , this, &KVLightEnergyCsIFull::GetLight, 0., 10000., 7, "KVLightEnergyCsIFull", "GetLight");
         break;

      case kApproxIntegral :
         fDlight = new TF1("fDlight_CsI", this, &KVLightEnergyCsIFull::dLightIntegralApprox, 0., 10000., 6, "KVLightEnergyCsIFull", "dLightIntegralApprox");
         fLight  = new TF1("fLight_CsI" , this, &KVLightEnergyCsIFull::GetLight, 0., 10000., 7, "KVLightEnergyCsIFull", "GetLightIntegral");
         break;

      case kApprox :
         fLight  = new TF1("fLight_CsI" , this, &KVLightEnergyCsIFull::GetLightApprox, 0., 10000., 7, "KVLightEnergyCsIFull", "GetLightApprox");
         break;

      case kApproxSilicon :
         fLight  = new TF1("fLight_CsI" , this, &KVLightEnergyCsIFull::GetLightApproxSilicon, 0., 10000., 7, "KVLightEnergyCsIFull", "GetLightApproxSilicon");
         break;

      default :
         fLight  = new TF1("fLight_CsI" , this, &KVLightEnergyCsIFull::GetLightApprox, 0., 10000., 7, "KVLightEnergyCsIFull", "GetLightApprox");
         break;
   }
}

void KVLightEnergyCsIFull::Print(Option_t* opt) const
{
   cout << "Formula : " << fLightFormula << endl;
   KVCalibrator::Print(opt);
   cout << "Formula : " << fLightFormula << endl;

}

KVLightEnergyCsIFull::~KVLightEnergyCsIFull()
{
   // Destructor
   SafeDelete(fDlight);
   SafeDelete(fLight);
}

//________________________________________________________________

void KVLightEnergyCsIFull::Copy(TObject& obj) const
{
   // This method copies the current state of 'this' object into 'obj'
   // You should add here any member variables, for example:
   //    (supposing a member variable KVLightEnergyCsIFull::fToto)
   //    CastedObj.fToto = fToto;
   // or
   //    CastedObj.SetToto( GetToto() );

   KVCalibrator::Copy(obj);
   //KVLightEnergyCsIFull& CastedObj = (KVLightEnergyCsIFull&)obj;
}

//___________________________________________________________________________
Double_t KVLightEnergyCsIFull::Compute(Double_t light) const
{
   // Calculate the calibrated energy (in MeV) for a given total light output.
   // The Z and A of the particle should be given first using SetZ, SetA.
   // By default, Z=A=1 (proton).
   //
   // This is done by inversion of the light-energy function using TF1::GetX.

   //set parameters of light-energy function
   Double_t par[7];
   for (int i = 0; i < 5; i++)
      par[i] = GetParameter(i);
   par[5] = (Double_t) fZ;
   par[6] = (Double_t) fA;
   fLight->SetParameters(par);

   //invert light vs. energy function to find energy
   Double_t xmin, xmax;
   fLight->GetRange(xmin, xmax);
   Double_t energy = fLight->GetX(light, xmin, xmax);

   return energy;
}


//___________________________________________________________________________
Double_t KVLightEnergyCsIFull::operator()(Double_t light)
{
   //Same as Compute()

   return Compute(light);
}

//___________________________________________________________________________
Double_t KVLightEnergyCsIFull::Invert(Double_t energy)
{
   //Given the calibrated (or simulated) energy in MeV,
   //calculate the corresponding total light output according to the
   //calibration parameters (useful for filtering simulations).

   //set parameters of light-energy function
   Double_t par[7];
   for (int i = 0; i < 5; i++)
      par[i] = GetParameter(i);
   par[5] = (Double_t) fZ;
   par[6] = (Double_t) fA;
   fLight->SetParameters(par);

   return fLight->Eval(energy);
}

//Double_t KVLightEnergyCsIFull::dLightIntegral( double *x , double *par )
//{
//    double e = x[0];

//    double zfit = par[4];
//    double afit = par[5];

//    double ag=par[0];
//    double ar=par[1];
//    double an=par[2];
//    double e_delta=par[3];

//    double u = 931.5;
//    double m = afit*u;
//    if((zfit==2.)&&(afit==4.)) m = 3727.4;
//    if((zfit==1.)&&(afit==1.)) m = 938.;

//    double beta = 0.;
//    double fraction = 0.;
//    double dlum = 0.;

//    double beta_delta = sqrt(2.*e_delta/u);

//    //cout << "Beta for Delta-electrons = " << beta_delta << endl;
//    //    beta = sqrt(1.-TMath::Power(m,2)/TMath::Power(e+m,2));
//    beta = sqrt(1.-m*m/((e+m)*(e+m)));
//    fraction = GetDeltaFraction(beta, beta_delta);
//    if(fraction>1.) fraction = 1.;
//    if(fraction<0.) fraction = 0.;

//    double se = sp_e(zfit,afit,e);
//    double sn = sp_n(zfit,afit,e);
//    double raps = 1.+sn/se;
//    double arg  = 1.-ar*se/(1.+an*sn+ar*se);
//    double fact = 1.+an*sn;
//    //    if ( zmed==zcsi) fact=1.;
//    fact=1.;
//    if(e<=e_delta*afit)
//    {
//        if((arg>0.)&&(ar*se>0.)&&(raps>0.)) dlum = -fact*TMath::Log(arg)/(raps*ar*se);
//    }
//    else
//    {
//        arg = 1.-(1.-fraction)*ar*se/(1.+an*sn+(1.-fraction)*ar*se);
//        if((arg>0.)&&(ar*se>0.)&&(raps>0.)) dlum = -fact*TMath::Log(arg)/(raps*ar*se)+fraction/raps;
//    }

////    Info("dLightIntegral","method called : e=%f  dlum=%f",e,dlum);

//    dlum *= ag;
//    return dlum;
//}
