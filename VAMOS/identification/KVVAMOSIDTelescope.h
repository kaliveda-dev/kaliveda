//Created by KVClassFactory on Mon May 27 10:38:29 2013
//Author: Guilain ADEMARD

#ifndef __KVVAMOSIDTELESCOPE_H
#define __KVVAMOSIDTELESCOPE_H

#include "KVINDRAIDTelescope.h"
#include "KVIDZAGrid.h"
#include "KVVAMOSDetector.h"

class KVVAMOSDetector;

class KVVAMOSIDTelescope : public KVINDRAIDTelescope {
protected:

   KVVAMOSDetector* fDEdet;//!
   KVVAMOSDetector* fEdet; //!
   KVIDZAGrid* fGrid; //!

   Bool_t fonlyZId; //!

public:

   KVVAMOSIDTelescope();
   virtual ~KVVAMOSIDTelescope();
   virtual const Char_t* GetArrayName();
   virtual Bool_t Identify(KVIdentificationResult*, Double_t x = -1., Double_t y = -1.);
   virtual void Initialize();

   //To decide if only Z identification will be proceed or both (Z,A) identification
   //By default (Z,A) identification will be used when possible
   void SetOnlyZId(Bool_t onlyZId)
   {
      fonlyZId = onlyZId;
   }
   Bool_t IsOnlyZId()
   {
      return fonlyZId;
   }


   // virtual const Char_t *GetIDSubCodeString(KVIDSubCode & concat) const;

   using KVINDRAIDTelescope::GetIDGrid;
   virtual KVIDGraph* GetIDGrid()
   {
      return fGrid;
   };
   ClassDef(KVVAMOSIDTelescope, 1) //A deltaE-E identification telescope in VAMOS
};

#endif
