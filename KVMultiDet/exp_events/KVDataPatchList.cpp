//Created by KVClassFactory on Thu Jan 30 11:06:10 2014
//Author: John Frankland,,,

#include "KVDataPatchList.h"
#include "KVDataPatch.h"
#include <TPluginManager.h>

ClassImp(KVDataPatchList)

KVUniqueNameList* KVDataPatchList::fPatchList = 0;

void KVDataPatchList::FillPatchList()
{
   // Fill static list of patches from KVDataPatch plugin list

   KVString plugins = GetListOfPlugins("KVDataPatch");
   if (plugins == "") {
      Info("FillPatchList", "No data patch plugins defined");
      return;
   }
   plugins.Begin(" ");
   while (!plugins.End()) {
      KVString plug = plugins.Next(kTRUE);
      TPluginHandler* h = LoadPlugin("KVDataPatch", GetPluginURI("KVDataPatch", plug));
      if (h) {
         KVDataPatch* p = (KVDataPatch*)h->ExecPlugin(0);
         fPatchList->Add(p);
      }
      else
         Warning("FillPatchList", "Cannot load plugin data patch %s", plug.Data());
   }
}

void KVDataPatchList::init()
{
   fEventPatches = kFALSE;
   fParticlePatches = kFALSE;
   if (!fPatchList) {
      fPatchList = new KVUniqueNameList;
      FillPatchList();
   }
}

KVDataPatchList::KVDataPatchList()
{
   // Default constructor
   init();
}

//________________________________________________________________

KVDataPatchList::KVDataPatchList(const Char_t* name, const Char_t* title) : KVBase(name, title)
{
   // Write your code here
   init();
}

KVDataPatchList::~KVDataPatchList()
{
   // Destructor
}

void KVDataPatchList::InitializePatchList(TString dataset, TString datatype, Int_t runnumber,
      TString dataseries, Int_t datareleasenumber, const TList* streamerinfolist)
{
   // Fill lists of active patches for given data

   Info("InitializePatchList",
        "Initializing patches for |%s|%s|%04d|%s|%02d|",
        dataset.Data(), datatype.Data(), runnumber, dataseries.Data(), datareleasenumber);
   fActiveEventPatches.Clear();
   fActiveParticlePatches.Clear();
   fEventPatches = kFALSE;
   fParticlePatches = kFALSE;
   TIter nxtP(fPatchList);
   KVDataPatch* patch;
   while ((patch = (KVDataPatch*)nxtP())) {
      if (patch->IsRequired(dataset, datatype, runnumber, dataseries, datareleasenumber, streamerinfolist)) {
         if (patch->IsEventPatch()) {
            fActiveEventPatches.Add(patch);
            fEventPatches = kTRUE;
         }
         if (patch->IsParticlePatch()) {
            fActiveParticlePatches.Add(patch);
            fParticlePatches = kTRUE;
         }
      }
   }
}

void KVDataPatchList::Apply(KVReconstructedEvent* event)
{
   // Apply all currently active patches to event

   if (fEventPatches) {
      // Apply event-level patches
      TIter nxt(&fActiveEventPatches);
      KVDataPatch* patch;
      while ((patch = (KVDataPatch*)nxt())) patch->ApplyToEvent(event);
   }
   if (fParticlePatches) {
      // Apply particle-level patches
      TIter nxt(&fActiveParticlePatches);
      KVDataPatch* patch;
      KVReconstructedNucleus* particle;
      while ((particle = event->GetNextParticle())) {
         while ((patch = (KVDataPatch*)nxt())) patch->ApplyToParticle(particle);
         nxt.Reset();
      }
   }
}

void KVDataPatchList::Print(Option_t*) const
{
   // Print list of currently active patches
   if (!fEventPatches && !fParticlePatches) {
      Info("Print", "No currently active data patches");
      return;
   }
   if (fEventPatches) {
      Info("Print", "Currently active event-level patches:");
      fActiveEventPatches.Print();
   }
   if (fParticlePatches) {
      Info("Print", "Currently active particle-level patches:");
      fActiveParticlePatches.Print();
   }
}
