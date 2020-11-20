//Created by KVClassFactory on Thu Jan 30 10:29:29 2014
//Author: John Frankland,,,

#ifndef __KVDATAPATCH_H
#define __KVDATAPATCH_H

#include "KVBase.h"
#include "KVReconstructedEvent.h"

/**
\class KVDataPatch
\brief Correction to be applied to reconstructed calibrated data
\ingroup Analysis

 Abstract base class for patches to be applied to reconstructed, identified,
 and/or calibrated data.

 To make a patch, derive a class from this one, and override the following
 methods:

    virtual Bool_t IsRequired(TString dataset, TString datatype, Int_t runnumber,
       TString dataseries, Int_t datareleasenumber, TList* streamerinfolist)

           - returns kTRUE if patch is to be applied to the data
                "dataseries" and "datareleasenumber" are the KaliVeda (major) series
                and (minor) release number previously used to generate the data
                "streamerinfolist" is a pointer to the list of TStreamerInfo objects
                containing information on the versions of classes used for the different
                objects in the file (obtained with TFile::GetStreamerInfoCache())

    virtual Bool_t IsEventPatch()

           - returns kTRUE if patch is to be applied at the event level
               (in this case ApplyToEvent(KVEvent*) will be called for each event)

    virtual Bool_t IsParticlePatch()

           - returns kTRUE if patch is to be applied at the particle level
               (in this case ApplyToNucleus(KVNucleus*) will be called for each particle)

    virtual void ApplyToEvent(KVEvent*)

           - event-level patch. if not required, just define an empty method.

    virtual void ApplyToParticle(KVNucleus*)

           - particle-level patch. if not required, just define an empty method.

    virtual void PrintPatchInfo() const

           - print information about the patch. Will be called by Print().

*/
class KVDataPatch : public KVBase {

public:
   KVDataPatch();
   KVDataPatch(const Char_t* name, const Char_t* title = "");
   virtual ~KVDataPatch();

   virtual Bool_t IsRequired(TString dataset, TString datatype, Int_t runnumber,
                             TString dataseries, Int_t datareleasenumber, const TList* streamerinfolist) = 0;
   virtual Bool_t IsEventPatch() = 0;
   virtual Bool_t IsParticlePatch() = 0;
   virtual void ApplyToEvent(KVReconstructedEvent*) = 0;
   virtual void ApplyToParticle(KVReconstructedNucleus*) = 0;

   virtual void PrintPatchInfo() const = 0;

   void Print(Option_t* /*opt*/ = "") const;

   ClassDef(KVDataPatch, 1) //Correction to be applied to reconstructed calibrated data
};

#endif
