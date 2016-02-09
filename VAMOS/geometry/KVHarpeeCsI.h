//Created by KVClassFactory on Wed Jul 25 10:12:53 2012
//Author: Guilain ADEMARD

#ifndef __KVHARPEECSI_H
#define __KVHARPEECSI_H

#include "KVVAMOSDetector.h"
#include "KVLightEnergyCsIVamos.h"

class KVHarpeeCsI : public KVVAMOSDetector {

private:

   enum {
      kPosIsOK = BIT(22) //flag set when PositionIsOK method is called
   };
   static KVString fACQParamTypes; //!types of available Acquision parameters
   static KVString fPositionTypes; //!types of available positions

   KVLightEnergyCsIVamos*  fCal;


protected:

   static KVHarpeeCsI* fCsIForPosition;//! Si used to obtain particle position
   static KVList*     fHarpeeCsIList;//! Global list of all KVHarpeeCsI objects
   void init();

public:

   KVHarpeeCsI();
   KVHarpeeCsI(const KVHarpeeCsI&) ;
   virtual ~KVHarpeeCsI();
   void Copy(TObject&) const;

   virtual const Char_t* GetArrayName();
   virtual const Char_t* GetEBaseName() const;

   static  KVHarpeeCsI* GetFiredHarpeeCsI(Option_t* opt = "Pany");
   static  KVList*      GetHarpeeCsIList();
   virtual Int_t       GetMult(Option_t* opt = "Pany");
   virtual void        Initialize();
   virtual Bool_t      PositionIsOK();
   virtual void        SetACQParams();
   virtual void        SetCalibrators();

   // -------- inline methods ---------------//

   virtual KVString& GetACQParamTypes();
   virtual KVString& GetPositionTypes();

   ClassDef(KVHarpeeCsI, 1) //CsI detectors of Harpee, used at the focal plan of VAMOS
};
//_________________________________________________________________________

inline KVString& KVHarpeeCsI::GetACQParamTypes()
{
   return fACQParamTypes;
}
inline KVString& KVHarpeeCsI::GetPositionTypes()
{
   return fPositionTypes;
}

#endif
