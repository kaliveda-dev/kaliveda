//Created by KVClassFactory on Thu Jan 30 10:29:29 2014
//Author: John Frankland,,,

#include "KVDataPatch.h"

ClassImp(KVDataPatch)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVDataPatch</h2>
<h4>Correction to be applied to reconstructed calibrated data</h4>
<!-- */
// --> END_HTML
// Abstract base class for patches to be applied to reconstructed, identified,
// and/or calibrated data.
//
// To make a patch, derive a class from this one, and override the following
// methods:
//
//    virtual Bool_t IsRequired(TString dataset, TString datatype, Int_t runnumber,
//       TString dataseries, Int_t datareleasenumber, TList* streamerinfolist)
//
//           - returns kTRUE if patch is to be applied to the data
//                "dataseries" and "datareleasenumber" are the KaliVeda (major) series
//                and (minor) release number previously used to generate the data
//                "streamerinfolist" is a pointer to the list of TStreamerInfo objects
//                containing information on the versions of classes used for the different
//                objects in the file (obtained with TFile::GetStreamerInfoCache())
//
//    virtual Bool_t IsEventPatch()
//
//           - returns kTRUE if patch is to be applied at the event level
//               (in this case ApplyToEvent(KVEvent*) will be called for each event)
//
//    virtual Bool_t IsParticlePatch()
//
//           - returns kTRUE if patch is to be applied at the particle level
//               (in this case ApplyToNucleus(KVNucleus*) will be called for each particle)
//
//    virtual void ApplyToEvent(KVEvent*)
//
//           - event-level patch. if not required, just define an empty method.
//
//    virtual void ApplyToParticle(KVNucleus*)
//
//           - particle-level patch. if not required, just define an empty method.
//
//    virtual void PrintPatchInfo() const
//
//           - print information about the patch. Will be called by Print().
//
////////////////////////////////////////////////////////////////////////////////

KVDataPatch::KVDataPatch()
{
   // Default constructor
}

KVDataPatch::KVDataPatch(const Char_t* name, const Char_t* title) : KVBase(name, title)
{
}

KVDataPatch::~KVDataPatch()
{
}

void KVDataPatch::Print(Option_t*) const
{
   // Print infos on patch

   std::cout << "PATCH NAME : [" << GetName() << "]" << std::endl;
   std::cout << "PATCH TITLE: " << GetTitle() << std::endl << std::endl;
   PrintPatchInfo();
   std::cout << std::endl;
}
