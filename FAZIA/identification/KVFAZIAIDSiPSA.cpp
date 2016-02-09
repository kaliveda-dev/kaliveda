//Created by KVClassFactory on Mon Feb 17 13:52:51 2014
//Author: John Frankland,,,

#include "KVFAZIAIDSiPSA.h"

ClassImp(KVFAZIAIDSiPSA)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVFAZIAIDSiPSA</h2>
<h4>PSA identification in first silicon of FAZIA telescopes</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVFAZIAIDSiPSA::KVFAZIAIDSiPSA()
{
   // Default constructor
   fRearThreshold = new TGraph(8);
   fRearThreshold->SetPoint(0, 2.919586798, 1.505566588);
   fRearThreshold->SetPoint(1, 4.968677141, 2.393311626);
   fRearThreshold->SetPoint(2, 5.963949593, 2.837184146);
   fRearThreshold->SetPoint(3, 9.945039402, 4.733730365);
   fRearThreshold->SetPoint(4, 14.9799471, 5.984643828);
   fRearThreshold->SetPoint(5, 20.0148548, 7.840838);
   fRearThreshold->SetPoint(6, 25.10830794, 9.858440361);
   fRearThreshold->SetPoint(7, 30.0846702, 14.21646146);
   SetType("SiPSA");
}

KVFAZIAIDSiPSA::~KVFAZIAIDSiPSA()
{
   // Destructor
   delete fRearThreshold;
}

Bool_t KVFAZIAIDSiPSA::CheckTheoreticalIdentificationThreshold(KVNucleus* ION, Double_t)
{
   // Return kTRUE if energy of ION is > minimum incident energy required for identification
   // in charge (Z) given by figure 9 of "Comparison of charged particle identification
   // using pulse shape discrimination and DeltaE-E methods between front and rear injection in
   // silicon detector" by R. Bougault et al (FAZIA collaboration).
   // Thresholds used are those for rear-injection (red curve, lowest values).

   Double_t seuil = fRearThreshold->Eval(ION->GetZ());
   return (ION->GetEnergyPerNucleon() > seuil);
}
