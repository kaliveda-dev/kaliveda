#ifndef KVIDENTIFICATIONRESULT_H
#define KVIDENTIFICATIONRESULT_H

#include "KVBase.h"
#include "TString.h"

class KVIdentificationResult : public KVBase {
public:
   Bool_t IDattempted;  // =kTRUE if identification was attempted
   Bool_t IDOK;  // general quality of identification, =kTRUE if acceptable identification made
   Int_t IDcode; // a general identification code for this type of identification
   Bool_t Zident;  // =kTRUE if Z of particle established
   Bool_t Aident; // = kTRUE if A of particle established
   Int_t IDquality; // specific quality code returned by identification procedure
   Int_t Z; // Z of particle found (if Zident==kTRUE)
   Int_t A; // A of particle found (if Aident==kTRUE)
   Double_t PID; // = "real" Z if Zident==kTRUE and Aident==kFALSE, "real" A if Zident==Aident==kTRUE
   Int_t deltaEpedestal; // special code for handling particles which give no signal in deltaE

   enum {
      deltaEpedestal_UNKNOWN, // status unknown, case not treated
      deltaEpedestal_YES,     // the particle to identify has a delta-E consistent with pedestal
      deltaEpedestal_NO       // the particle to identify has a delta-E > pedestal
   };

   KVIdentificationResult() :
      IDattempted(0), IDOK(0), IDcode(-1),
      Zident(0), Aident(0), IDquality(-1), Z(-1), A(-1), PID(-1.0), deltaEpedestal(deltaEpedestal_UNKNOWN)
   {};
   virtual ~KVIdentificationResult() {};

   KVIdentificationResult(const KVIdentificationResult& id) : KVBase()
   {
      // copy constructor
      id.Copy(*this);
   };
   void Clear(Option_t* opt = "");
   void Copy(TObject&) const;
   void Print(Option_t* opt = "") const;
   KVIdentificationResult& operator=(const KVIdentificationResult& i)
   {
      i.Copy(*this);
      return *this;
   };

   void SetIDType(const Char_t* t)
   {
      // Set type of identification (= type of KVIDTelescope)
      SetType(t);
   };
   const Char_t* GetIDType() const
   {
      // Gives type of identification (= type of KVIDTelescope)
      return GetType();
   };
   void SetComment(const Char_t* c)
   {
      // Set an explanatory comment for the identification procedure quality code
      SetLabel(c);
   };
   const Char_t* GetComment() const
   {
      // Give an explanatory comment for the identification procedure quality code
      return GetLabel();
   };
   ClassDef(KVIdentificationResult, 2) //Full result of one attempted particle identification
};

#endif
