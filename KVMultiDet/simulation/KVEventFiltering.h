//Created by KVClassFactory on Thu Jun 21 17:01:26 2012
//Author: John Frankland,,,

#ifndef __KVEventFiltering_H
#define __KVEventFiltering_H

#include "KVEventSelector.h"
#include "KVClassMonitor.h"
#include "KVReconstructedEvent.h"
#include <KVSimEvent.h>

class KVDBSystem;
class KVEventFiltering : public KVEventSelector {
   //KVClassMonitor memory_check;
   Long64_t fEVN;//event number counter
   Bool_t fRotate;//true if random phi rotation should be applied [default: yes]
   Bool_t fGemini;//true if Gemini++ decay should be performed before detection [default: no]
   KVSimEvent fGemEvent;//event after decay with Gemini

   void RandomRotation(KVEvent* to_rotate, const TString& frame_name = "") const;
public:
   KVEventFiltering();
   KVEventFiltering(const KVEventFiltering&) ;
   virtual ~KVEventFiltering();
   void Copy(TObject&) const;

   Bool_t Analysis();
   void EndAnalysis();
   void EndRun();
   void InitAnalysis();
   void InitRun();
   void OpenOutputFile(KVDBSystem*, Int_t);

   TFile* fFile;
   TTree* fTree;
   KVReconstructedEvent* fReconEvent;
   TVector3 fCMVelocity;
   TVector3 fProjVelocity;
   Bool_t fTransformKinematics;//=kTRUE if simulation not in lab frame
   TString fNewFrame;   //allow the definition of a specific frame

   ClassDef(KVEventFiltering, 1) //Filter simulated events with multidetector response
};

#endif
