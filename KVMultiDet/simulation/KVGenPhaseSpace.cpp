//Created by KVClassFactory on Fri Apr 17 10:19:02 2015
//Author: John Frankland,,,

#include "KVGenPhaseSpace.h"
#include "TGenPhaseSpace.h"

ClassImp(KVGenPhaseSpace)



void KVGenPhaseSpace::init()
{
   // default initialisations

   fMCSampler = 0;
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
   ((TGenPhaseSpace*)fMCSampler)->SetDecay(fCompound, fEvent->GetMult(), &fMasses[0]);
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

   fMasses.clear();
   fMasses.reserve(fMult);
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

/** \example KVGenPhaseSpace_example.C
# Calculate 12C*->3-alpha decay & fill TTree with generated events

 Example of use of KVGenPhaseSpace class, which uses TGenPhaseSpace
 to generate events according to microcanonical statistics.
 By default, 10**5 events are generated for the 3-alpha decay of
 a Carbon-12 nucleus with E*=50 MeV.

 To execute this function, do

     $ kaliveda
     kaliveda[0] .L KVGenPhaseSpace_example.C+
     kaliveda[1] example()

 */

/** \example KVGenPhaseSpace_example2.C
# Calculate 12C*->3-alpha decay & compare alpha KE with exact microcanonical distribution

 Example of use of KVGenPhaseSpace class, which uses TGenPhaseSpace
 to generate events according to microcanonical statistics.
 By default, 10**5 events are generated for the 3-alpha decay of
 a Carbon-12 nucleus with E*=50 MeV.
 A histogram is filled with the kinetic energy distribution of one of the
 alpha particles, which is then fitted using the exact microcanonical
 probability distribution for a classical gas of 3 equal-mass particles.

 To execute this function, do

 $ kaliveda
 kaliveda[0] .L KVGenPhaseSpace_example2.C+
 kaliveda[1] example()

 */
