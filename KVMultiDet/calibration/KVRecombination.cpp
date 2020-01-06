//Created by KVClassFactory on Wed Aug 28 11:48:51 2013
//Author: Guilain ADEMARD

#include "KVRecombination.h"

ClassImp(KVRecombination)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVRecombination</h2>
<h4>Silicon PHD described by electron-hole recombination model</h4>

<p>
The static TF1 "fParlog" calculates the PHD of a particle of incident energy
E, charge Z and Mass A calculated according to Parlog's formula (see Phys. Res. A 428 (1999) 379):
</p>

<p>
      PHD(Ed) = 1/2 * Ed *( 1-1/X * ln|1+X| + X * ln|1+1/X| )
 with X      = a*A*Z**2/Ed
      Ed     = energy lost by particle in detector (=E if particle stops)
</p>
<p>Only one parameter (a) can be set using method<br>
SetParameters(a) or SetParameter(0,a).
The charge and the mass numbers have to be set first the calculation using<br>
method SetZandA( Z, A).
</p>

<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

Double_t KVRecombination::PHDParlog(Double_t* x, Double_t* par)
{
   //Returns Parlog PHD for given E, Z and A:
   //
   //      PHD(E) = 1/2 * Ed *( 1-1/X * ln|1+X| + X * ln|1+1/X| )
   // with X      = a*A*Z**2/Ed
   //      Ed     = energy lost by particle in detector (=E if particle stops)
   //
   // x[0]   = E (MeV)
   // par[0] = Z
   // par[1] = A

   Int_t    Z = par[0];
   Int_t    A = par[1];
   Double_t a = GetParameter(0);
   Double_t E = x[0] > 0 ? x[0] : 0.000001;
   Double_t X = a * A * Z * Z / E;

   return (1. - 1. / X * TMath::Log(TMath::Abs(1. + X))
           + X * TMath::Log(TMath::Abs(1. + 1. / X))
          ) * E / 2;
}
//________________________________________________________________

void KVRecombination::init()
{
   //default initialisations
   SetType("Pulse Height Defect");
   SetZandA(1, 1);
   fParlog = fDeltaEphd = NULL;
}
//________________________________________________________________

KVRecombination::KVRecombination() : KVCalibrator()
{
   // Default constructor
   init();
}
//________________________________________________________________

KVRecombination::KVRecombination(KVDetector* d) : KVCalibrator()
{
   //Associate PHD calculation to detector
   init();
   SetDetector(d);
}
//________________________________________________________________

KVRecombination::~KVRecombination()
{
   // Destructor
}
//________________________________________________________________

Double_t KVRecombination::Compute(Double_t energy, const KVNameValueList&) const
{
   //Calculate the pulse height defect (in MeV) for a given energy loss in the detector,
   //a given Z and a given A (Z and A  of the particle should be set first using method SetZandA)
   //The Parlog formula is used:
   //
   //      PHD(E) = 1/2 * Ed *( 1-1/X * ln|1+X| + X * ln|1+1/X| )
   // with X      = a*A*Z**2/Ed
   //      Ed     = energy lost by particle in detector (=E if particle stops)

   return const_cast<KVRecombination*>(this)->GetParlogPHDFunction(GetZ(), GetA())->Eval(energy);
}

TF1* KVRecombination::GetELossFunction(Int_t Z, Int_t A, Bool_t Wrong)
{
   // Return pointer to TF1 giving energy loss in active layer of detector minus
   // the pulse height defect for a given nucleus (Z,A).
   //
   // If Wrong=kTRUE (default:kFALSE) this will be calculated incorrectly
   // (if the particle does not stop in the detector) by using the Parlog formula
   // with the incident energy of the particle instead of the calculated energy
   // loss of the particle.

   wrong = Wrong;

   if (!fDeltaEphd) {
      fDeltaEphd = new TF1(Form("KVRecombination:%s:ELossActive", GetDetector()->GetName()),
                           this, &KVRecombination::ELossActive, 0., 1.e+04, 2, "KVRecombination", "ELossActive");
      fDeltaEphd->SetNpx(gEnv->GetValue("KVPulseHeightDefect.EnergyLoss.Npx", 20));
   }
   fDeltaEphd->SetParameters((Double_t)Z, (Double_t)A);
   fDeltaEphd->SetRange(0., GetDetector()->GetSmallestEmaxValid(Z, A));
   fDeltaEphd->SetTitle(Form("PHD dependent energy loss [MeV] in detector %s for Z=%d A=%d", GetDetector()->GetName(), Z, A));
   GetParlogPHDFunction(Z, A);
   return fDeltaEphd;
}
//________________________________________________________________

Double_t KVRecombination::ELossActive(Double_t* x, Double_t* par)
{
   // Calculate energy lost in active layer of detector minus the Parlog PHD
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
   // calculate Parlog PHD corresponding to energy lost in active layer
   Double_t phd;
   if (wrong) phd = PHDParlog(&e, par); //incorrect calculation using incident energy
   else phd = PHDParlog(&dE, par);

   return dE - phd;
}
//________________________________________________________________

TF1* KVRecombination::GetParlogPHDFunction(Int_t Z, Int_t A)
{
   // Create TF1* fParlog if not already done

   if (!fParlog) {
      fParlog = new TF1(Form("ParlogPHD:%s", GetDetector()->GetName()),
                        this, &KVRecombination::PHDParlog, 0., 1.e+04, 2, "KVRecombination", "PHDParlog");
      fParlog->SetNpx(500);
   }
   fParlog->SetParameter(0, Z);
   fParlog->SetParameter(1, A);
   return fParlog;
}
//________________________________________________________________

Double_t KVRecombination::Invert(Double_t energy, const KVNameValueList&) const
{
   //Given the PHD (in MeV) of a particle of charge Z and mass A
   //(set using SetZandA() method), this method inverts the Parlog formula
   //in order to find the energy loss of the particle in the detector.

   const_cast<KVRecombination*>(this)->GetParlogPHDFunction(GetZ(), GetA());
   Double_t xmin, xmax;
   fParlog->GetRange(xmin, xmax);
   return fParlog->GetX(energy, xmin, xmax);
}
