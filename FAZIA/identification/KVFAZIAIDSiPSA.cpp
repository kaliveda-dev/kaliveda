//Created by KVClassFactory on Mon Feb 17 13:52:51 2014
//Author: John Frankland,,,

#include "KVFAZIAIDSiPSA.h"
#include "KVDataSet.h"

ClassImp(KVFAZIAIDSiPSA)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVFAZIAIDSiPSA</h2>
<h4>PSA identification in first silicon of FAZIA telescopes</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

TF1* KVFAZIAIDSiPSA::fZThreshold = 0;
TF1* KVFAZIAIDSiPSA::fAThreshold = 0;

KVFAZIAIDSiPSA::KVFAZIAIDSiPSA()
{
   // Default constructor

   if (!fZThreshold) {
      // Threshold for Z identification by PSA
      // Data from NIM A ?, ? (2017)
//      Double_t Z[] = {3,6,9,12,15,18,21,24,27,30};
//      Double_t E[] = {10,30,65,105,155,220,310,420,520,590};
//      Double_t dE = 5;// +/- 5 MeV for all points
//      fZThreshold = new TGraphErrors(10,Z,E);
//      for(int i=0;i<10;++i) fZThreshold->SetPointError(i,0,dE);
      fZThreshold = new TF1("FAZIAIDSiPSA-ZSEUIL", "pol2", 0, 100);
//      fZThreshold->SetParameters(12.261900, -1.767320, 0.765392);  // Previous implementation
      fZThreshold->SetParameters(1.71777, -0.760697, 0.752485) ;  // New implementation after thresholds determination with FAZIACOR experiment
   }

   if (!fAThreshold) {
      // Threshold for A identification by PSA
      // Data from NIM A ?, ? (2017)
//      int i=0; fZmaxAID=3;
//      Double_t E[] = {15,22.5,37.5,67.5,97.5,112.5,140,172.5,202.5,247.5,277.5,322.5,375,410,487.5,530,612.5};
//      Double_t Z[17]; for(int i=0;i<17;++i) Z[i] = i+3;
//      Double_t dE[17]; for(int i=0;i<11;++i) dE[i] = 15; for(int i=11;i<17;++i) dE[i] = 25;
//      fAThreshold = new TGraphErrors(17,Z,E,0,dE);
      fAThreshold = new TF1("FAZIAIDSiPSA-ASEUIL", "pol2", 0, 100);
      fAThreshold->SetParameters(-5.14823, 2.03461, 1.55798);
   }
   SetType("SiPSA");
   set_id_code(kSi1);
}

KVFAZIAIDSiPSA::~KVFAZIAIDSiPSA()
{
   // Destructor
}

Bool_t KVFAZIAIDSiPSA::Identify(KVIdentificationResult* idr, Double_t x, Double_t y)
{
   //Particle identification and code setting using identification grid (class KVIDZAFromZGrid).

   Bool_t ok = KVFAZIAIDTelescope::Identify(idr, x, y);

   return ok;
}

void KVFAZIAIDSiPSA::Initialize()
{
   // Initialisation of telescope before identification.
   // This method MUST be called once before any identification is attempted.
   // Initialisation of grid is performed here.
   // IsReadyForID() will return kTRUE if a grid is associated to this telescope for the current run.
//printf("Initializing SiPSA\n");
   IGrid = (KVIDZAGrid*) GetIDGrid(); // for the moment we defined only Igrids -> to be modified latter
   fSi = (KVFAZIADetector*)GetDetector(1);
   if (IGrid) {
      SetHasMassID(IGrid->HasMassIDCapability());
      IGrid->Initialize();
      SetBit(kReadyForID);
   }
   else {
      ResetBit(kReadyForID);
   }
   if (!gDataSet->HasCalibIdentInfos()) { // for filtering simulations
      SetBit(kReadyForID, fSi->IsLabelled("SI1"));// only activate PSA for SI1
      // if not, no particles are identified in SI1-SI2
   }
}

Bool_t KVFAZIAIDSiPSA::CheckTheoreticalIdentificationThreshold(KVNucleus* ION, Double_t)
{
   // Return kTRUE if energy of ION is > minimum incident energy required for identification
   // in charge (Z) given by figure 9 of "Comparison of charged particle identification
   // using pulse shape discrimination and DeltaE-E methods between front and rear injection in
   // silicon detector" NIM A 701, 145 (2013) (FAZIA collaboration).
   // Thresholds used are those for rear-injection (red curve, lowest values).

   Double_t seuil = fZThreshold->Eval(ION->GetZ());
   return (ION->GetEnergy() >= seuil);
}

void KVFAZIAIDSiPSA::SetIdentificationStatus(KVReconstructedNucleus* n)
{
   // For filtering simulations
   // If n->GetEnergy() is above threshold for mass identification, we set
   // n->IsAMeasured(kTRUE) (and n->IsZMeasured(kTRUE)).
   // Otherwise, we just set n->IsZMeasured(kTRUE) and use the A given by
   // the mass formula for the particle
   //
   // Z-dependence of A identification:
   //    all ok above threshold if Z<=16, decreasing probability for 17<=Z<=21
   //    no A identification for Z>21
   //
   // If A is not measured, we make sure the KE of the particle corresponds to the simulated one

   n->SetZMeasured();
   fMassIDProb->SetParameters(18.5, .4);
   Bool_t okmass = (n->GetZ() < 17) || (n->GetZ() < 22 && gRandom->Uniform() < fMassIDProb->Eval(n->GetZ()));
   okmass = okmass && (n->GetEnergy() >= fAThreshold->Eval(n->GetZ()));
   if (okmass) {
      n->SetAMeasured();
   }
   else {
      double e = n->GetE();
      n->SetZ(n->GetZ());
      n->SetE(e);
   }
}
