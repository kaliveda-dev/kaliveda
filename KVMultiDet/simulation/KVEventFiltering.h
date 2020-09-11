//Created by KVClassFactory on Thu Jun 21 17:01:26 2012
//Author: John Frankland,,,

#ifndef __KVEventFiltering_H
#define __KVEventFiltering_H

#include "KVEventSelector.h"
#include "KVClassMonitor.h"
#include "KVReconstructedEvent.h"
#include <KVSimEvent.h>
#ifdef WITH_GEMINI
#include "KVGemini.h"
#endif

//#define DEBUG_FILTER 1

class KVDBSystem;
class KVEventFiltering : public KVEventSelector {
#ifdef DEBUG_FILTER
   KVClassMonitor* memory_check;
#endif
   Long64_t fEVN;//event number counter
   Bool_t fRotate;//true if random phi rotation should be applied [default: yes]
#ifdef WITH_GEMINI
   Bool_t fGemini;//true if Gemini++ decay should be performed before detection [default: no]
   Bool_t fGemAddRotEner;//true if rotational energy has to be added to excitation energy [default: no]
   Int_t fGemDecayPerEvent;//number of Gemini++ decays to be performed for each event [default:1]
   KVSimEvent fGemEvent;//event after decay with Gemini
   KVGemini GEM;
#endif

   const char* fIdCalMode; //! original exp setup hasIDandCalib to be reset in case of modifications

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
