/*
$Id: KVPulseHeightDefect.cpp,v 1.5 2008/10/07 15:55:20 franklan Exp $
$Revision: 1.5 $
$Date: 2008/10/07 15:55:20 $
*/

//Created by KVClassFactory on Mon Feb 19 17:32:55 2007
//Author: franklan

#include "KVPulseHeightDefect.h"
#include "TMath.h"

ClassImp(KVPulseHeightDefect)

//////////////////////////////////////////

Double_t KVPulseHeightDefect::PHDMoulton(Double_t* x, Double_t* par)
{
   //Returns Moulton PHD for given E and Z:
   //
   // log_10(PHD) = b(Z) + a(Z)*log_10(E), for Z > Zmin
   //
   //               PHD  = 0,  for Z <= Zmin
   //
   //  with  a(Z) = a_1*(Z**2/1000) + a_2
   //          b(Z) = b_1*(100/Z) + b_2
   //            E = energy lost by particle in detector
   //
   // x[0] = E (MeV)
   // par[0] = Z

   Int_t      Z = par[0];
   // by definition, no PHD for Z=2 or less
   if (Z < 3) return 0;
   Double_t a_1 = GetParameter(0);
   Double_t a_2 = GetParameter(1);
   Double_t b_1 = GetParameter(2);
   Double_t b_2 = GetParameter(3);

   Double_t a = a_1 * Z * Z / 1000. + a_2;
   Double_t b = b_1 * 100. / Z + b_2;
   return (TMath::Power(10, b) * TMath::Power(x[0], a));
}

//___________________________________________________________________//

void KVPulseHeightDefect::init()
{
   //default initialisations
   SetType("Pulse Height Defect");
   SetZ(1);
   fMoulton = fDeltaEphd = 0;
}

KVPulseHeightDefect::KVPulseHeightDefect(): KVCalibrator()
{
   //Default constructor
   init();
}

KVPulseHeightDefect::KVPulseHeightDefect(KVDetector* d): KVCalibrator()
{
   //Associate PHD calculation to detector
   init();
   SetDetector(d);
}

KVPulseHeightDefect::~KVPulseHeightDefect()
{
   //Destructor
   SafeDelete(fMoulton);
   SafeDelete(fDeltaEphd);
}

//___________________________________________________________________________

Double_t KVPulseHeightDefect::Compute(Double_t energy, const KVNameValueList&) const
{
   //Calculate the pulse height defect (in MeV) for a given energy loss in the detector
   //and a given Z (the Z  of the particle should be set first using method SetZ)
   //The Moulton formula is used:
   //
   // log_10(PHD) = b(Z) + a(Z)*log_10(E)
   //
   //               PHD  = 0,  for Z <= Zmin
   //
   //  with  a(Z) = a_1*(Z**2/1000) + a_2
   //          b(Z) = b_1*(100/Z) + b_2
   //            E = energy loss of particle

   return const_cast<KVPulseHeightDefect*>(this)->GetMoultonPHDFunction(GetZ())->Eval(energy);
}

TF1* KVPulseHeightDefect::GetELossFunction(Int_t Z, Int_t A, Bool_t Wrong)
{
   // Return pointer to TF1 giving energy loss in active layer of detector minus
   // the pulse height defect for a given nucleus (Z,A).
   //
   // If Wrong=kTRUE (default:kFALSE) this will be calculated incorrectly
   // (if the particle does not stop in the detector) by using the Moulton formula
   // with the incident energy of the particle instead of the calculated energy
   // loss of the particle.

   wrong = Wrong;

   if (!fDeltaEphd) {
      fDeltaEphd = new TF1(Form("KVPulseHeightDefect:%s:ELossActive", GetDetector()->GetName()),
                           this, &KVPulseHeightDefect::ELossActive, 0., 1.e+04, 2, "KVPulseHeightDefect", "ELossActive");
      fDeltaEphd->SetNpx(gEnv->GetValue("KVPulseHeightDefect.EnergyLoss.Npx", 20));
   }
   fDeltaEphd->SetParameters((Double_t)Z, (Double_t)A);
   fDeltaEphd->SetRange(0., GetDetector()->GetSmallestEmaxValid(Z, A));
   fDeltaEphd->SetTitle(Form("PHD dependent energy loss [MeV] in detector %s for Z=%d A=%d", GetDetector()->GetName(), Z, A));
   GetMoultonPHDFunction(Z);
   return fDeltaEphd;
}

Double_t KVPulseHeightDefect::ELossActive(Double_t* x, Double_t* par)
{
   // Calculate energy lost in active layer of detector minus the Moulton PHD
   // x[0] = incident energy
   // par[0] = Z
   // par[1] = A

   Double_t e = x[0];
   TIter next(GetDetector()->GetListOfAbsorbers());
   KVMaterial* mat;
   while ((mat = (KVMaterial*)next()) != GetDetector()->GetActiveLayer()) {
      // calculate energy losses in absorbers before active layer
      e = mat->GetERes(par[0], par[1], e);     //residual energy after layer
      if (e <= 0.)
         return 0.;          // return 0 if particle stops in layers before active layer
   }
   // calculate energy loss in active layer
   Double_t dE = mat->GetDeltaE(par[0], par[1], e);
   // calculate Moulton PHD corresponding to energy lost in active layer
   Double_t phd;
   if (wrong) phd = PHDMoulton(&e, par); //incorrect calculation using incident energy
   else phd = PHDMoulton(&dE, par);

   return dE - phd;
}

TF1* KVPulseHeightDefect::GetMoultonPHDFunction(Int_t Z)
{
   // Create TF1* fMoulton if not already done

   if (!fMoulton) {
      fMoulton = new TF1(Form("MoultonPHD:%s", GetDetector()->GetName()),
                         this, &KVPulseHeightDefect::PHDMoulton, 0., 1.e+04, 1, "KVPulseHeightDefect", "PHDMoulton");
      fMoulton->SetNpx(500);
   }
   fMoulton->SetParameter(0, Z);
   return fMoulton;
}

//___________________________________________________________________________

Double_t KVPulseHeightDefect::Invert(Double_t energy, const KVNameValueList&) const
{
   //Given the PHD (in MeV) of a particle of charge Z
   //(set using SetZ() method), this method inverts the Moulton formula
   //in order to find the energy loss of the particle in the detector.

   const_cast<KVPulseHeightDefect*>(this)->GetMoultonPHDFunction(GetZ());
   Double_t xmin, xmax;
   fMoulton->GetRange(xmin, xmax);
   return fMoulton->GetX(energy, xmin, xmax);
}
