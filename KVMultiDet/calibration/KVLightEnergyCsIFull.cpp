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
Double_t KVLightEnergyCsIFull::dLightIntegralApprox(double* x, double* par)
{
   double e = x[0];

   double zfit = par[0];
   double afit = par[1];

   double ag = par[2];
   double ar = par[3];
   double an = par[4];
   double e_delta = par[5];

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
Double_t KVLightEnergyCsIFull::dLightIntegral(double* x, double* par)
{
   double e = x[0];
   double zfit = par[0];
   double afit = par[1];
   double ag = par[2];
   double ar = par[3];
   if (ar < 1.e-8) ar = 0.;
   double an = par[4];
   double e_delta = par[5];
   double m = afit * u;
   if (zfit == 1. && afit == 1.) m = 938.2;
   if (zfit == 1. && afit == 2.) m = 1876.;
   if (zfit == 1. && afit == 3.) m = 2809.4;
   if (zfit == 2. && afit == 3.) m = 2809.4;
   if (zfit == 2. && afit == 4.) m = 3727.4;
   if (zfit == 2. && afit == 6.) m = 5606.6;
   if (zfit == 2. && afit == 8.) m = 7483.7;
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
   double raps = 1.;
   if (se > 1.e-6) raps = 1. + sn / se;
   double arg = 1. - ar * se / (1. + an * sn + ar * se);
   double fact = 1.;
   double deno = 1. + an * sn + (1. - fraction) * ar * se;
// If energy is below the delta production threshold : 1 term
   if (e <= e_delta * afit) {
      if (arg > 0. && raps * ar * se > 0.) dlum = -fact * log(arg) / (raps * ar * se);
      else dlum = 0.;
   }
// If energy is above the delta production threshold : 2 terms
   else {
      if (deno > 0.) arg = 1. - (1. - fraction) * ar * se / deno;
      if (arg > 0. && ar * se > 0. && raps > 0.) dlum = -fact * log(arg) / (raps * ar * se) + fraction / raps;
      else dlum = 0.;
   }
// Multiply by the gain factor
   dlum = dlum * ag;
   return dlum;
   /*    double raps = 1.+sn/se;
       double arg  = 1.-ar*se/(1.+an*sn+ar*se);
       double fact = 1.;

       if(e<=e_delta*afit)
       {
           if((arg>0.)&&(ar*se>0.)&&(raps>0.)) dlum = -fact*TMath::Log(arg)/(raps*ar*se);
       }
       else
       {
           Double_t y = 1 - fraction*ar*se/(1. + an*sn + ar*se);
           arg = 1. - y*ar*se/(1. + an*sn+ar*se);
           if((arg>0.)&&(ar*se>0.)&&(raps>0.)) dlum = -fact*(1-fraction)*TMath::Log(arg)/(raps*ar*se*y) + fraction/raps;
       }

       dlum *= ag;
       return dlum;
   */
}

//________________________________________________________________
Double_t KVLightEnergyCsIFull::sp_e(double z, double a, double e)
{
// if energy E<0.1 keV, return 0
   if (e < 1.e-4) return 0.;
// coefficients for Helium particles (master curves) on Iodium (Z=53), see ref. Ziegler
   double a1_i = 3.118;
   double a2_i = 0.6519;
   double a3_i = 164.9;
   double a4_i = 1.208;
   double a5_i = 1.51;
   double a6_i = 4.614;
   double a7_i = 0.1043;
   double a8_i = -0.1315;
   double a9_i = -0.009175;
// coefficients for Helium particles (master curves) on Cesium (Z=55), see ref. Ziegler
   double a1_cs = 14.4;
   double a2_cs = 0.3923;
   double a3_cs = 152.5;
   double a4_cs = 8.354;
   double a5_cs = 2.597;
   double a6_cs = 4.671;
   double a7_cs = 0.1136;
   double a8_cs = -0.1298;
   double a9_cs = -0.009078;
// Reference particles for master curves, here alpha particles
   int zref = 2;
   int aref = 4;
   double ei = float(aref) * e / a * 1000.; // energy in keV !
   double slow_cs = 0.;
   double shigh_cs = 0.;
   double se_cs = 0.;
   double slow_i = 0.;
   double shigh_i = 0.;
   double se_i = 0.;
   double se = 0.;
   slow_cs = a1_cs * pow(ei, a2_cs);
   shigh_cs = 1000.*a3_cs * log(1. + 1000.*a4_cs / ei + a5_cs * ei / 1000.) / ei;
   slow_i = a1_i * pow(ei, a2_i);
   shigh_i = 1000.*a3_i * log(1. + 1000.*a4_i / ei + a5_i * ei / 1000.) / ei;
   if (ei <= 10000.) {
      double fact = 0.997;
      se_cs = fact * slow_cs * shigh_cs / (slow_cs + shigh_cs);
      se_i = fact * slow_i * shigh_i / (slow_i + shigh_i);
   } else {
      se_cs = exp(a6_cs + a7_cs * log(1000. / ei) + a8_cs * pow(log(1000. / ei), 2.) + a9_cs * pow(log(1000. / ei), 3.));
      se_i = exp(a6_i + a7_i * log(1000. / ei) + a8_i * pow(log(1000. / ei), 2.) + a9_i * pow(log(1000. / ei), 3.));
   }
// Electronic stopping power for the composite Cs+I = CsI
   double seref = (127.*se_i + 133.*se_cs) / (127. + 133.);
// Get the effective charge
   se = pow(gamma(z, a, e) * float(z) / (gamma(zref, aref, ei) * float(zref)), 2.) * seref;
// Special cases for Z=1 (gamma=1) and Z=2 (reference!)
   if (z == int(zref)) se = seref;
   if (z == 1.) se = seref / (pow(float(zref), 2.) * gamma(zref, aref, ei));
// Convert the stopping power from eV/(10**15 atoms/cm**2) to MeV/(mg/cm**2)
   se *= 6.022 / (10.*fAmed);
// Convert in MeV/micron,  not here !
   return se;
   /*     if ( e<1.e-4 ) return 0.;
       Double_t se = fMaterialTable->GetStoppingFunction(z,a)->Eval(e);//-sp_n(z,a,e); // in units of MeV/(g/cm**2)
       se*=1.e-3;                                                                      // in units of MeV/(mg/cm**2)
       return se-sp_n(z,a,e);
    */
}

//________________________________________________________________
Double_t KVLightEnergyCsIFull::gamma(double z, double a, double e)
{
// Hubert, Bimbot and Gauvin
   double g0 = 0.;
   g0 = (gamma_hbg(z, a, e) + gamma_ziegler(z, a, e)) / 2.;
// if E<2.5 meV/n, take effective charge gamma from Ziegler, otherwise take Hubert and Bimbot's formula
// A linear adjustment is made to avoid discontinuity between the two prescriptions at E=2.5 MeV/n
// For very low energy (E<400 keV/n), take the gamma value from Ziegler's formula at E=400 keV/n
   if (e < 0.4 * a) g0 = gamma_ziegler(z, a, 0.4 * a);
   return g0;
}

//________________________________________________________________
Double_t KVLightEnergyCsIFull::gamma_ziegler(double z, double a, double e)
{
   double ei = e;
   double m = a * u;
   if (z == 2. && a == 4.) m = 3727.4;
   double v0 = 7.2983824e-3;                          // Bohr velocity for electrons
   double v = sqrt(2.*ei / m);
   double v1 = 0.886 * (v / v0) * pow(z, -2. / 3.);
   double v2 = v1 + 0.038 * sin(3.141592654 * v1 / 2.);
   double xgamma = 1. - (1.034 - 0.17777 * exp(-0.08114 * z)) * exp(-v2);
   return xgamma;
}

//________________________________________________________________
Double_t KVLightEnergyCsIFull::gamma_hbg(double z, double a, double e)
{
   double z2 = fZmed; // Here the atomic number Z for the stopping material
   double b = 1.658;
   double c = 0.0517;
   double d = 1.164 + 0.2319 * exp(-0.0004302 * z2);
   double x1 = d + b * exp(-c * z);
   double x2 = 8.144 + 0.09876 * log(z2);
   double x3 = 0.314 + 0.01072 * log(z2);
   double x4 = 0.5218 + 0.02521 * log(z2);
// Special values for Beryllium
   if (z == 4) {
      b = 2.;
      c = 0.04369;
      d = 2.045;
      x1 = d + b * exp(-c * z);
      x2 = 7.;
      x3 = 0.2643;
      x4 = 0.4171;
   }
// Special values for Carbon
   if (z == 6) {
      b = 1.91;
      c = 0.03958;
      d = 2.584;
      x1 = d + b * exp(-c * z);
      x2 = 6.933;
      x3 = 0.2433;
      x4 = 0.3969;
   }
   double xgamma = 1. - x1 * exp(-x2 * pow(e / a, x3) / pow(z, x4));
   if (xgamma > 1.) xgamma = 1.;
   if (xgamma < 0.) xgamma = 0.;
   return xgamma;
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
   // par[0] = Z
   // par[1] = A
   // par[2] = a1
   // par[3] = a2
   // par[4] = a3
   // par[5] = a4
   // par[6] = pied

   double emin    = 1.e-4; //arbitrary set to avoid divergence of de/dx
   double emax    = x[0];
   double epsilon = 1.;    //arbitrary set

#if ROOT_VERSION_CODE > ROOT_VERSION(5,99,01)
   // for compilation with latest ROOT svn trunk version called 5.99/01
   fDlight->SetParameters(par);
   return par[6] + fDlight->Integral(emin, emax, epsilon);
#else
   return par[6] + fDlight->Integral(emin, emax, par, epsilon);
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
KVLightEnergyCsIFull::KVLightEnergyCsIFull(const Char_t* name, const Char_t* type, KVDetector* kvd, Int_t lightFormula): KVCalibrator(name, type, 5) // : KVCalibrator(kvd)
{
   SetDetector(kvd);
   SetLightFormula(lightFormula);
   init();
}

//________________________________________________________________
void KVLightEnergyCsIFull::init()
{
   //default initialisations
//   SetType("Light-Energy CsI");
//   if(fDetector&&(fLightFormula==kExact)) fMaterialTable = fDetector->GetRangeTable()->GetMaterial(fDetector->GetMaterialName());
//   else Error("init", "No detector provided !");
   SetA(1);
   SetZ(1);
   fZmed = 54;//fDetector->GetZ();
   fAmed = 130;//fDetector->GetMass();

   u = 931.5;
   fDlight = 0;

   switch (fLightFormula) {
      case kExact :
         fDlight = new TF1("fDlight_CsI", this, &KVLightEnergyCsIFull::dLightIntegral, 0., 10000., 6, "KVLightEnergyCsIFull", "dLightIntegral");
         fLight  = new TF1("fLight_CsI", this, &KVLightEnergyCsIFull::GetLight, 0., 10000., 7, "KVLightEnergyCsIFull", "GetLight");
         break;

      case kApproxIntegral :
         fDlight = new TF1("fDlight_CsI", this, &KVLightEnergyCsIFull::dLightIntegralApprox, 0., 10000., 6, "KVLightEnergyCsIFull", "dLightIntegralApprox");
         fLight  = new TF1("fLight_CsI", this, &KVLightEnergyCsIFull::GetLight, 0., 10000., 7, "KVLightEnergyCsIFull", "GetLightIntegral");
         break;

      case kApprox :
         fLight  = new TF1("fLight_CsI", this, &KVLightEnergyCsIFull::GetLightApprox, 0., 10000., 7, "KVLightEnergyCsIFull", "GetLightApprox");
         break;

      case kApproxSilicon :
         fLight  = new TF1("fLight_CsI", this, &KVLightEnergyCsIFull::GetLightApproxSilicon, 0., 10000., 7, "KVLightEnergyCsIFull", "GetLightApproxSilicon");
         break;

      default :
         fLight  = new TF1("fLight_CsI", this, &KVLightEnergyCsIFull::GetLightApprox, 0., 10000., 7, "KVLightEnergyCsIFull", "GetLightApprox");
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
      par[i + 2] = GetParameter(i);
   par[0] = (Double_t) fZ;
   par[1] = (Double_t) fA;
   fLight->SetParameters(par);
//for(int i=0; i<7; i++)printf("P2  [%d]=%f  ", i, fLight->GetParameter(i)); printf("Z=%d  A=%d \n",fZ, fA);
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
