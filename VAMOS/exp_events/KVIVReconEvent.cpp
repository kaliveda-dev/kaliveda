//Created by KVClassFactory on Mon Mar 18 10:44:28 2013
//Author: Guilain ADEMARD

#include "KVIVReconEvent.h"
#include "KVVAMOSReconEvent.h"
#include "KVVAMOS.h"
#include "KVTarget.h"

using namespace std;

ClassImp(KVIVReconEvent)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVIVReconEvent</h2>
<h4>Event reconstructed from energy losses in INDRA array and VAMOS spectrometer</h4>
<!-- */
// --> END_HTML
//
//
//
//    1) METHODS FOR INDRA
//
//      GetParticle(Int_t i)       -  returns ith reconstructed particle of event (i=1,...,GetINDRAMult())
//      GetParticleWithName(const Char_t*)    -  returns the first particle with given name
//      GetParticle(const Char_t*)            -  returns the first particle belonging to a given group
//      UseMeanAngles()      - particle's theta & phi are taken from mean theta & phi of detectors
//      UseRandomAngles()      - particle's theta & phi are randomized within theta & phi limits of detectors
//      HasMeanAngles()/HasRandomAngles()   -  indicate in which of the two previous cases we find ourselves
//
//      GetNextParticle(Option_t* opt)  -
//              Use this method to iterate over the list of particles in the INDRA event
//              To make sure you begin at the start of the list, call ResetGetNextParticle()
//              before beginning the iteration.
//              After the last particle GetNextParticle() returns a null pointer and
//              resets itself ready for a new iteration over the particle list:
//
//              Example: (supposing "KVIVReconEvent* event" points to a valid event)
//              KVINDRAReconNuc* indra_part; event->ResetGetNextParticle();
//              while( (indra_part = event->GetNextParticle()) ){
//                      ...
//              }
//
//              If opt="ok" only particles with KVReconstructedNucleus::IsOK() = kTRUE
//              are included in the iteration.
//
//
//    2) METHODS FOR VAMOS
//
//      GetNucleus(Int_t i)       -  returns ith reconstructed nucleuis of VAMOS event (i=1,...,GetVAMOSMult())
//      GetNucleus(const Char_t*)             -  returns the first nucleus belonging to a given group
//
//      GetNextNucleus(Option_t* opt)  -
//              Use this method to iterate over the list of nuclei in the VAMOS event
//              To make sure you begin at the start of the list, call ResetGetNextNucleus()
//              before beginning the iteration.
//              After the last nucleus GetNextNucleus() returns a null pointer and
//              resets itself ready for a new iteration over the nucleus list:
//
//              Example: (supposing "KVIVReconEvent* event" points to a valid event)
//              KVVAMOSReconNuc* vamos_nuc; event->ResetGetNextNucleus();
//              while( (vamos_nuc = event->GetNextNucleus()) ){
//                      ...
//              }
//
//              If opt="ok" only nuclei with KVReconstructedNucleus::IsOK() = kTRUE
//              are included in the iteration.
//
//   Normally the VAMOS event is composed by only one nucleus.
//
//
//   The "OK" status of particles/nuclei is defined by setting acceptable identification and calibration (reconstruction)
//   codes using AcceptIDCodes_INDRA, AcceptECodes_INDRA/ AcceptIDCodes_VAMOS, AcceptECodes_VAMOS ...
//   The comparison of each particle/nucleus codes with the "acceptable" codes then determines whether
//   KVParticle::IsOK() is set or not.
////////////////////////////////////////////////////////////////////////////////
void KVIVReconEvent::init()
{
   //Default initialisations
   fVAMOSev = NULL;
}
//________________________________________________________________

KVIVReconEvent::KVIVReconEvent(Int_t mult_indra, const char* cl_name_indra, Int_t mult_vamos, const char* cl_name_vamos)
   : KVINDRAReconEvent(mult_indra, cl_name_indra)
{
   // Default constructor
   init();
   fVAMOSev = new KVVAMOSReconEvent(mult_vamos, cl_name_vamos);
}
//________________________________________________________________

KVIVReconEvent::~KVIVReconEvent()
{
   // Destructor
   SafeDelete(fVAMOSev);
}
//________________________________________________________________

void KVIVReconEvent::Clear(Option_t* opt)
{
   //Reset the event to zero ready for new event.

   KVINDRAReconEvent::Clear(opt);
   fVAMOSev->Clear(opt);
}
//________________________________________________________________

Int_t KVIVReconEvent::GetTotalMult(Option_t* opt)
{
   //Returns total multiplicity (number of particles in INDRA + nucleus in VAMOS) of event.
   //If opt = "" (default), returns number of particles in TClonesArray* fParticles (INDRA) + fVAMOSev->fParticles (VAMOS).
   //If opt = "ok" only particles and nuclei  with IsOK()==kTRUE are included.
   //
   //IN THE LATTER CASE, YOU MUST NOT USE THIS METHOD INSIDE A LOOP
   //OVER THE INDRA'S EVENT USING GETNEXTPARTICLE() OR OVER THE VAMOS'S
   //EVENT USING GETNEXTNUCLEUS!!!
   //
   //BE CAREFUL: the method GetMult(Option_t *) does not return the total
   //multiplicity but only the multiplicity in INDRA.

   return GetINDRAMult(opt) + GetVAMOSMult(opt);
}
//________________________________________________________________

void KVIVReconEvent::Print(Option_t* option) const
{
   //Print out list of particles in the INDRA's event and list of nuclei reconstructed
   //in VAMOS.
   //If option="ok" only particles and nuclei with IsOK=kTRUE are included.

   cout << "------------- In INDRA --------------------" << endl;
   KVINDRAReconEvent::Print(option);

   cout << "------------- In VAMOS --------------------" << endl;
   fVAMOSev->Print(option);
}
