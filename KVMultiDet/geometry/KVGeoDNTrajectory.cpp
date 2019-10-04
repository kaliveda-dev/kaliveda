//Created by KVClassFactory on Tue Feb 25 16:31:36 2014
//Author: John Frankland,,,

#include "KVGeoDNTrajectory.h"
#include "KVDetector.h"

#include <TPluginManager.h>

ClassImp(KVGeoDNTrajectory)

Int_t KVGeoDNTrajectory::fGDNTrajNumber = 0;

KVGeoDNTrajectory::KVGeoDNTrajectory() : fNodes(3, 0), fIDTelescopes(kFALSE), fPathInTitle(kTRUE), fAddToNodes(kTRUE)
{
   // Default constructor
   init();
}
//______________________
KVGeoDNTrajectory::KVGeoDNTrajectory(KVGeoDetectorNode* node)
   : fNodes(3, 0), fIDTelescopes(kFALSE), fPathInTitle(kTRUE), fAddToNodes(kTRUE)
{
   // Create a new trajectory starting from node
   AddLast(node);
   init();
}
//______________________
KVGeoDNTrajectory::KVGeoDNTrajectory(const KVGeoDNTrajectory& obj)
   : KVBase(), fNodes(3, 0), fIDTelescopes(kFALSE), fPathInTitle(kTRUE), fAddToNodes(kTRUE)
{
   //copy ctor
   obj.Copy(*this);
   init();
}

KVGeoDNTrajectory::~KVGeoDNTrajectory()
{
   // Destructor
}

KVGeoDNTrajectory* KVGeoDNTrajectory::Factory(const char* plugin, const KVGeoDNTrajectory* t, const KVGeoDetectorNode* n)
{
   // Instantiate & return object of class corresponding to plugin

   TPluginHandler* ph = LoadPlugin("KVGeoDNTrajectory", plugin);
   if (ph) {

      return (KVGeoDNTrajectory*)ph->ExecPlugin(2, t, n);
   }
   return (KVGeoDNTrajectory*)nullptr;
}

void KVGeoDNTrajectory::init()
{
   fIter_idx = fIter_idx_sav = -1;
   ++fGDNTrajNumber;
   if (fPathInTitle) SetName(Form("GDNTraj_%d", fGDNTrajNumber));
   else SetTitle(Form("GDNTraj_%d", fGDNTrajNumber));
   SetNumber(fGDNTrajNumber);
}

void KVGeoDNTrajectory::rebuild_title()
{
   // called every time a new node is added to the trajectory
   // to update the title with the new node name
   // Dynamically constructed title: DET1/DET2/DET3/...
   // if fPathInTitle = kFALSE, we change the name not the title

   TString t;
   TIter next(&fNodes);
   KVGeoDetectorNode* n;
   while ((n = (KVGeoDetectorNode*)next())) {
      t += n->GetName();
      t += "/";
   }
   if (fPathInTitle) SetTitle(t);
   else SetName(t);
}

void KVGeoDNTrajectory::increment_identified_particle_counters(int identified, int unidentified) const
{
   // increment counters in all detectors on trajectory
   IterateFrom();
   KVGeoDetectorNode* node;
   while ((node = GetNextNode())) {
      node->GetDetector()->IncrementIdentifiedParticles(identified);
      node->GetDetector()->IncrementUnidentifiedParticles(unidentified);
   }
}

void KVGeoDNTrajectory::Copy(TObject& obj) const
{
   KVBase::Copy(obj);
   KVGeoDNTrajectory& CastedObj = (KVGeoDNTrajectory&)obj;
   TIter next(&fNodes);
   KVGeoDetectorNode* node;
   CastedObj.fAddToNodes = fAddToNodes;
   while ((node = (KVGeoDetectorNode*)next())) CastedObj.AddLast(node);
   fIDTelescopes.Copy(CastedObj.fIDTelescopes);
   CastedObj.fPathInTitle = fPathInTitle;
}

KVGeoDNTrajectory& KVGeoDNTrajectory::operator=(const KVGeoDNTrajectory& t)
{
   if (&t != this) t.Copy(*this);
   return (*this);
}

void KVGeoDNTrajectory::Clear(Option_t*)
{
   // Clear list of nodes in trajectory

   fNodes.Clear();
   fIter_idx = fIter_idx_sav = -1;
   if (fPathInTitle) SetTitle("");
   else SetName("");
}

void KVGeoDNTrajectory::ReverseOrder()
{
   // Reverse the order of the nodes in the trajectory

   int idx = fNodes.GetEntries();
   TObjArray tmp(idx);
   int N = idx;
   TIter it(&fNodes);
   KVGeoDetectorNode* node;
   while ((node = (KVGeoDetectorNode*)it())) tmp.AddAt(node, --idx);
   fNodes.Clear();
   for (idx = 0; idx < N; ++idx) fNodes.AddAt(tmp[idx], idx);
   rebuild_title();
}

void KVGeoDNTrajectory::AddToNodes()
{
   // Add reference to this trajectory to all nodes on it

   IterateFrom();
   KVGeoDetectorNode* node;
   while ((node = GetNextNode())) node->AddTrajectory(this);
}


