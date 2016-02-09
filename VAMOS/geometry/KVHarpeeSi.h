//Created by KVClassFactory on Wed Jul 25 10:12:53 2012
//Author: Guilain ADEMARD

#ifndef __KVHARPEESI_H
#define __KVHARPEESI_H

#include "KVVAMOSDetector.h"

class KVRecombination;

class KVHarpeeSi : public KVVAMOSDetector {
private:

   enum {
      kPosIsOK = BIT(22) //flag set when PositionIsOK method is called
   };
   static KVString fACQParamTypes; //!types of available Acquision parameters
   static KVString fPositionTypes; //!types of available positions


protected:

   static KVHarpeeSi* fSiForPosition;//! Si used to obtain particle position
   static KVList*     fHarpeeSiList;//! Global list of all KVHarpeeSi objects
   KVFunctionCal*   fCanalE;        //! Canal->Energy calibrator
   KVRecombination* fPHD;           //! pulse height defect
   void init();

public:

   KVHarpeeSi();
   KVHarpeeSi(UInt_t number, Float_t thick /* um */);
   KVHarpeeSi(const KVHarpeeSi&) ;
   virtual ~KVHarpeeSi();
   void Copy(TObject&) const;

   virtual const Char_t* GetArrayName();
   virtual const Char_t* GetEBaseName() const;

   virtual Double_t    GetCalibT(const Char_t* type);
   virtual Double_t    GetCanalFromEnergy(Double_t e = -1);
   virtual TF1*        GetELossFunction(Int_t Z, Int_t A);
   virtual Double_t    GetEnergy();
   virtual Double_t    GetEnergyFromCanal(Double_t c = -1);
   static  KVHarpeeSi* GetFiredHarpeeSi(Option_t* opt = "Pany");
   static  KVList*     GetHarpeeSiList();
   virtual Int_t       GetMult(Option_t* opt = "Pany");
   Double_t    GetPHD(Double_t dE, UInt_t Z, UInt_t A);
   virtual void        Initialize();
   virtual Bool_t      PositionIsOK();
   virtual void        SetACQParams();
   virtual void        SetCalibrators();
   virtual void        SetParlogPHDParameter(Double_t a);
   // -------- inline methods ---------------//

   virtual KVString& GetACQParamTypes();
   virtual KVString& GetPositionTypes();

   ClassDef(KVHarpeeSi, 1) //Silicon detectors of Harpee, used at the focal plan of VAMOS
};
//_________________________________________________________________________

inline KVString& KVHarpeeSi::GetACQParamTypes()
{
   return fACQParamTypes;
}
inline KVString& KVHarpeeSi::GetPositionTypes()
{
   return fPositionTypes;
}

#endif
