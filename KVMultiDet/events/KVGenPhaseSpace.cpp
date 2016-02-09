//Created by KVClassFactory on Fri Apr 17 10:19:02 2015
//Author: John Frankland,,,

#include "KVGenPhaseSpace.h"
#include "TGenPhaseSpace.h"

ClassImp(KVGenPhaseSpace)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVGenPhaseSpace</h2>
<h4>Generate momenta for an event using microcanonical phase space sampling</h4>
<!-- */
// --> END_HTML
// We use ROOT class TGenPhaseSpace to sample the phase space for each event.
// This corresponds to sampling the microcanonical phase space for a gas
// composed of the nuclei of the break-up channel with a given total kinetic
// energy determined by energy balance.
//
// Example of use
// ==============
//
// // 3-alpha decay of 12C*
//
// KVNucleus CN(6,12);
// CN.SetExcitEnergy(20); // minimum = 7.27 MeV
//
// //  decay products
// KVEvent decay;
// KVNucleus* n = decay.AddParticle();
// n->SetZandA(2,4);
// n = decay.AddParticle();
// n->SetZandA(2,4);
// n = decay.AddParticle();
// n->SetZandA(2,4);
//
// KVGenPhaseSpace gps;
// if(!gps.SetBreakUpChannel(CN, &decay))
// {
//   printf("Break-up channel is not allowed\n");
//   return;
// }
//
// Double_t etot = gps.GetAvailableEnergy();
// TH1F* h1 = new TH1F("h1","Kinetic energy of alpha particle 3",200,0,etot*2./3.);
// h1->Sumw2();
//
// Int_t nevents = 10000;
// while(nevents--){
//   wgt = gps.Generate();
//   h1->Fill(n->GetKE(),wgt);
// }
//
// h1->Draw();
//
////////////////////////////////////////////////////////////////////////////////

void KVGenPhaseSpace::init()
{
   // default initialisations

   fMCSampler = 0;
   fMasses = 0;
   fEvent = 0;
   fOK = kFALSE;
}

void KVGenPhaseSpace::InitialiseMCSampler()
{
   // initialise the TGenPhaseSpace for the decay channel
   // We store the name of the generator in the list of parameters of the
   // KVEvent object pointed to by fEvent:
   //       fEvent->GetParameters()->SetValue("PHASESPACE_GENERATOR","TGenPhaseSpace");

   if (!fMCSampler) fMCSampler = new TGenPhaseSpace;
   ((TGenPhaseSpace*)fMCSampler)->SetDecay(fCompound, fEvent->GetMult(), fMasses);
   fEvent->GetParameters()->SetValue("PHASESPACE_GENERATOR", "TGenPhaseSpace");
}

KVGenPhaseSpace::KVGenPhaseSpace()
{
   // Default constructor
   init();
}

//________________________________________________________________

KVGenPhaseSpace::KVGenPhaseSpace(const Char_t* name, const Char_t* title) : KVBase(name, title)
{
   // Constructor with name and title
   init();
}

KVGenPhaseSpace::~KVGenPhaseSpace()
{
   // Destructor
   SafeDelete(fMCSampler);
   if (fMasses) delete [] fMasses;
}

//________________________________________________________________

void KVGenPhaseSpace::Copy(TObject& obj) const
{
   // This method copies the current state of 'this' object into 'obj'
   // You should add here any member variables, for example:
   //    (supposing a member variable KVGenPhaseSpace::fToto)
   //    CastedObj.fToto = fToto;
   // or
   //    CastedObj.SetToto( GetToto() );

   KVBase::Copy(obj);
   //KVGenPhaseSpace& CastedObj = (KVGenPhaseSpace&)obj;
}

Bool_t KVGenPhaseSpace::SetBreakUpChannel(const KVNucleus& CN, KVEvent* e)
{
   // Define the break-up channel, i.e. the initial compound nucleus with its
   // kinematics & excitation energy, and a list of nuclei to be produced
   // by the decay. Nuclei produced by decay may be excited.
   //
   // Returns kTRUE if decay is energetically allowed, kFALSE if not.

   fEvent = e;
   fCompound = CN;
   fMult = e->GetMult();

   fOK = CheckBreakUpChannel();
   if (!fOK) return fOK;

   if (fMasses) delete [] fMasses;
   fMasses = new Double_t[fMult];
   fEvent->GetMasses(fMasses);

   InitialiseMCSampler();

   return fOK;
}

Double_t KVGenPhaseSpace::Generate()
{
   // Generate 1 event for this break-up channel.
   // The weight associated to the event is returned.
   // We also store the value in the list of parameters of the
   // KVEvent object pointed to by fEvent:
   //       fEvent->GetParameters()->SetValue("PHASESPACE_WEIGHT", weight);
   //
   // NB: in order to get e.g. correct kinetic energy distributions for particles,
   //     this weight MUST be used as e.g. the filling weight for any histograms
   //     (see example in Class Description)

   if (!fOK) {
      Warning("Generate", "Generator is not initialised correctly.");
      return 0.;
   }
   Double_t weight = ((TGenPhaseSpace*)fMCSampler)->Generate();
   fEvent->GetParameters()->SetValue("PHASESPACE_WEIGHT", weight);
   for (int i = 0; i < fMult; i++) {
      fEvent->GetParticle(i + 1)->Set4Mom(*(((TGenPhaseSpace*)fMCSampler)->GetDecay(i)));
   }
   return weight;
}

Bool_t KVGenPhaseSpace::CheckBreakUpChannel()
{
   // We check that:
   //   - mass and charge are conserved
   //   - the excitation energy of the compound is greater than the channel Q-value
   //      (taking into account any excitation energy of nuclei in exit channel)

   Int_t ztot = (Int_t)fEvent->GetSum("GetZ");
   Int_t atot = (Int_t)fEvent->GetSum("GetA");
   if (ztot != fCompound.GetZ() || atot != fCompound.GetA()) {
      Warning("CheckBreakUpChannel", "Compound has (Z,A)=(%d,%d), break-up channel has (Z,A)=(%d,%d)",
              fCompound.GetZ(), fCompound.GetA(), ztot, atot);
      return kFALSE;
   }

   Double_t exmin = -fEvent->GetChannelQValue();
   fEtot = fCompound.GetExcitEnergy() - exmin;
   if (fEtot < 0.) {
      Warning("CheckBreakUpChannel", "Excitation energy of compound must be > %.2lf MeV", exmin);
      return kFALSE;
   }

   return kTRUE;
}
