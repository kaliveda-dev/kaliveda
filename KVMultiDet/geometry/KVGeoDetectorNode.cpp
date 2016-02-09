//Created by KVClassFactory on Fri Apr 26 12:45:15 2013
//Author: John Frankland,,,

#include "KVGeoDetectorNode.h"
#include "KVDetector.h"
#include "KVUniqueNameList.h"

ClassImp(KVGeoDetectorNode)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVGeoDetectorNode</h2>
<h4>Stores lists of detectors in front and behind this node</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

void KVGeoDetectorNode::init()
{
   fInFront = 0;
   fBehind = 0;
}

KVGeoDetectorNode::KVGeoDetectorNode()
{
   // Default constructor
   init();
}

//________________________________________________________________

KVGeoDetectorNode::KVGeoDetectorNode(const Char_t* name) : KVBase(name, "Detector node")
{
   // Write your code here
   init();
}

KVGeoDetectorNode::~KVGeoDetectorNode()
{
   // Destructor
   SafeDelete(fInFront);
   SafeDelete(fBehind);
}

void KVGeoDetectorNode::ls(Option_t*) const
{
   std::cout << "Detector Node " << GetName() << std::endl;
   if (fInFront) {
      std::cout << "In front:" << std::endl;
      fInFront->ls();
   }
   if (fBehind) {
      std::cout << "Behind:" << std::endl;
      fBehind->ls();
   }
}

void KVGeoDetectorNode::AddInFront(KVDetector* d)
{
   if (!fInFront) fInFront = new KVUniqueNameList;
   fInFront->Add(d);
}

void KVGeoDetectorNode::AddBehind(KVDetector* d)
{
   if (!fBehind) fBehind = new KVUniqueNameList;
   fBehind->Add(d);
}
Bool_t KVGeoDetectorNode::IsInFrontOf(KVDetector* d)
{
   // return true if this node is directly in front of the detector
   return (fBehind && fBehind->FindObject(d) != 0);
}
Bool_t KVGeoDetectorNode::IsBehind(KVDetector* d)
{
   // return true if this node is directly behind the detector
   return (fInFront && fInFront->FindObject(d) != 0);
}

Int_t KVGeoDetectorNode::GetNDetsInFront() const
{
   // Returns number of detectors directly in front of this one
   return (fInFront ? fInFront->GetEntries() : 0);
}

Int_t KVGeoDetectorNode::GetNDetsBehind() const
{
   // Returns number of detectors directly behind this one
   return (fBehind ? fBehind->GetEntries() : 0);
}

void KVGeoDetectorNode::RehashLists()
{
   // Call this method if detector names change after lists are filled
   // (they are hash lists, if names of objects change, strange behaviour
   // will occur: you could put the same object in a list twice)

   if (fInFront) dynamic_cast<KVUniqueNameList*>(fInFront)->Rehash();
   if (fBehind) dynamic_cast<KVUniqueNameList*>(fBehind)->Rehash();
}
