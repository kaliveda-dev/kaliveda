//Created by KVClassFactory on Wed Jul 25 09:43:37 2012
//Author: Guilain ADEMARD

#ifndef __KVVAMOSDETECTOR_H
#define __KVVAMOSDETECTOR_H

#include "KVSpectroDetector.h"
#include "KVVAMOS.h"
#include "KVFunctionCal.h"
#include "KVDataSet.h"
#include "KVMacros.h" // 'UNUSED' macro

class KVVAMOSDetector : public KVSpectroDetector {

private:

   static KVString fACQParamTypes; //!types of available Acquision parameters
   static KVString fPositionTypes; //!types of available positions


protected:

   TList*         fT0list; //! list of T0 saved in a KVNamedParameter
   TList*         fTlist;  //! list of Time ACQ parameters
   Int_t         fNmeasX;  // number of measured X positions
   Int_t         fNmeasY;  // number of measured Y positions

   void init();
   virtual Bool_t Fired(Option_t* opt, Option_t* optP);


public:

   KVVAMOSDetector();
   KVVAMOSDetector(UInt_t number, const Char_t* type);
   KVVAMOSDetector(const KVVAMOSDetector&) ;
   virtual ~KVVAMOSDetector();
   void Copy(TObject&) const;
   inline virtual Bool_t Fired(Option_t* opt = "any");
   inline virtual Bool_t FiredP(Option_t* opt = "any");
   virtual void   Initialize();
   virtual void   SetCalibrators();
   virtual KVFunctionCal* GetECalibrator() const;
   virtual Bool_t GetPositionInVamos(Double_t& X, Double_t& Y);
   virtual UChar_t  GetRawPosition(Double_t* XYZf);
   virtual Double_t GetRawPosition(Char_t dir = 'X', Int_t num = 0);
   virtual UChar_t GetRawPositionError(Double_t* EXYZf);
   virtual Double_t GetRawPositionError(Char_t dir = 'X', Int_t num = 0);

   KVFunctionCal* GetTCalibrator(const Char_t* type) const;


   virtual Double_t GetCalibE();
   virtual Double_t GetCalibT(const Char_t* type);
   virtual Float_t  GetT(const Char_t* type);
   Double_t GetT0(const Char_t* type) const;
   virtual Bool_t IsECalibrated() const;
   Bool_t   IsTCalibrated(const Char_t* type) const;
   Bool_t   IsStartForT(const Char_t* type) const;
   Bool_t   IsStopForT(const Char_t* type) const;
   void     SetFiredBitmask(KVString& lpar_dummy);
   void     SetT0(const Char_t* type, Double_t t0 = 0.);

   virtual const Char_t* GetEBaseName() const;
   virtual const Char_t* GetTBaseName() const;
   Double_t GetCalibT_HF();
   Float_t  GetT_HF();
   Double_t GetT0_HF() const;
   Bool_t   IsTHFCalibrated() const;
   void     SetT0_HF(Double_t t0 = 0.);

   // -------- inline methods ---------------//


   inline TList* GetListOfT0() const
   {
      return fT0list;
   }

   inline TList* GetTACQParamList() const
   {
      return fTlist;
   }


   inline virtual KVString& GetACQParamTypes()
   {
      return fACQParamTypes;
   }

   inline virtual KVString& GetPositionTypes()
   {
      return fPositionTypes;
   }

   inline UChar_t GetACQParamTypeIdx(const Char_t* type)
   {
      return KVVAMOS::GetACQParamTypeIdx(type, this);
   }

   inline UChar_t GetPositionTypeIdx(const Char_t* type)
   {
      return KVVAMOS::GetPositionTypeIdx(type, this);
   }

   inline UInt_t CalculateUniqueID(KVBase* param)
   {
      return   KVVAMOS::CalculateUniqueID(param, this);
   }

   inline UChar_t GetACQParamTypeIdxFromID(UInt_t id)
   {
      return KVVAMOS::GetACQParamTypeIdxFromID(id);
   }

   inline UChar_t GetPositionTypeIdxFromID(UInt_t id)
   {
      return KVVAMOS::GetPositionTypeIdxFromID(id);
   }

   inline Bool_t IsUsedToMeasure(const Char_t* type)
   {
      return KVVAMOS::IsUsedToMeasure(type, this);
   }

   inline Int_t GetNMeasuredX() const
   {
      return fNmeasX;
   }
   inline Int_t GetNMeasuredY() const
   {
      return fNmeasY;
   }

   inline Float_t  GetRawE()
   {
      return GetACQData(GetEBaseName());
   }

   ClassDef(KVVAMOSDetector, 1) //Detectors of VAMOS spectrometer
};



//________________________________________________________________
Bool_t KVVAMOSDetector::Fired(Option_t* opt)
{
   //Returns kTRUE if detector was hit (fired) in an event
   //
   //The actual meaning of hit/fired depends on the context and the option string opt.
   //
   //opt="any" (default):
   //Returns true if at least one working acquisition parameter
   //associated with the detector was fired in an event, for ANY of the types
   //in the list*.
   //
   //opt="all" :
   //Returns true if at least one working acquisition parameter
   //associated with the detector was fired in an event, for ALL of the types
   //in the list*.
   //
   //opt="Pany" :
   //Returns true if at least one working acquisition parameter
   //associated with the detector was fired in an event and have a value
   //greater than their pedestal value, for ANY of the types in the list*.
   //
   //opt="Pall" :
   //Returns true if at least one working acquisition parameter
   //associated with the detector was fired in an event and have a value
   //greater than their pedestal value, for ALL of the types in the list*.
   //
   // *the actual parameters taken into account can be fine tuned using environment variables such as
   //          KVVAMOSDetector.Fired.ACQParameterList.[type]: Q,E,T,T_HF,X,Y
   // See KVAMOSDetector::SetFiredBitmask() for more details.

   if (opt[0] == 'P') return FiredP(opt + 1);
   return Fired(opt, "");
}
//_________________________________________________________________________________

Bool_t KVVAMOSDetector::FiredP(Option_t* opt)
{
   //opt="any" :
   //Returns true if at least one working acquisition parameter
   //associated with the detector was fired in an event and have a value
   //greater than their pedestal value, for ANY of the types in the list*.
   //
   //opt="all" :
   //Returns true if at least one working acquisition parameter
   //associated with the detector was fired in an event and have a value
   //greater than their pedestal value, for ALL of the types in the list*.
   //
   // *the actual parameters taken into account can be fine tuned using environment variables such as
   //          KVVAMOSDetector.Fired.ACQParameterList.[type]: Q,E,T,T_HF,X,Y
   // See KVAMOSDetector::SetFiredBitmask() for more details.

   return Fired(opt, "P");
}

#endif
