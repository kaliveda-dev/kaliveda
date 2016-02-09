#ifndef KVIVReconIdent_h
#define KVIVReconIdent_h

#include "KVINDRAReconIdent.h"

class KVIVReconIdent: public KVINDRAReconIdent {

protected:

   enum {
      kVAMOSdata = BIT(20),
      kINDRAdata = BIT(21)
   };

   Bool_t fIsIVevent; // flag set when the event class inherits from KVIVReconEvent;


public:

   KVIVReconIdent()
   {
      fIsIVevent = kFALSE;
   }
   virtual ~ KVIVReconIdent() {}

   virtual void InitRun();
   virtual void InitAnalysis();
   virtual Bool_t Analysis();
   virtual void EndAnalysis();

   virtual void Init(TTree* tree)
   {
      //Before to call KVINDRAReconIdent::Init change the name of
      //the branch of reconstructed events which will be read  if
      //its class is a KVIVReconEvent and not a KVINDRAReconEvent
      if (!tree) return;
      TBranch* br = (TBranch*)tree->GetListOfBranches()->First();
      if (TClass::GetClass(br->GetClassName())->InheritsFrom("KVINDRAReconEvent"))
         SetINDRAReconEventBranchName(br->GetName());
      KVINDRAReconIdent::Init(tree);
   }

   ClassDef(KVIVReconIdent, 0) //Identification and reconstruction of VAMOS and INDRA events from recon data
};

#endif
