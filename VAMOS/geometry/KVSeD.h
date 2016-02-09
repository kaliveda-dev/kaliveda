//Created by KVClassFactory on Mon Sep  3 11:28:25 2012
//Author: Guilain ADEMARD

#ifndef __KVSED_H
#define __KVSED_H

#include "KVVAMOSDetector.h"
#include "TF1.h"
#include "KVSeDPositionCal.h"
#include "KVDataSet.h"
#include "KVMacros.h" // 'UNUSED' macro


#define IS_IN_SED_WINDOW( X, Y )( (-20 <= X && X <= 20  && -5 <= Y && Y <= 5) )
#define Border 5
#define ONEOVERSQRTTWO  0.707106781

class TH1F;

class KVSeD : public KVVAMOSDetector {

private:

   static KVString fACQParamTypes; //!types of available Acquision parameters
   static KVString fPositionTypes; //!types of available positions
   static Int_t    fNstripsOK;     //!minimum. numbers of strips for a good position measurement


protected:

   TH1F***   fQ;      //! array of TH1F for calibrated charge [raw, calibrated, clean][X, Y]
   TF1*       fPeakFunction;//!
   Double_t   fRawPos[2]; //!
   Double_t   fERawPos[2]; //!
   //    TSpectrum *fSpectrum;
   KVSeDPositionCal*  fPosCalib; //! position calibrator

   void init();

public:

   KVSeD();
   KVSeD(const KVSeD&) ;
   virtual ~KVSeD();
   void Copy(TObject&) const;

   virtual Float_t CalculateQThreshold(Char_t dir);
   virtual void  Clear(Option_t* option = "");
   virtual const Char_t* GetArrayName();
   virtual Double_t GetCalibT(const Char_t* type);
   virtual const Char_t* GetTBaseName() const;
   virtual UChar_t GetPosition(Double_t* XYZf, Char_t dir = 0, Int_t idx = 0);
   virtual void    GetDeltaXYZf(Double_t* XYZf, Char_t dir = 0, Int_t idx = 0);
   using KVVAMOSDetector::GetRawPosition;
   virtual Double_t GetRawPosition(Char_t dir = 'X', Int_t num = 0);
   virtual Double_t GetRawPosition2(Char_t dir = 'X', Double_t min_amp = 100, Double_t min_sigma = 0.5, Double_t max_sigma = 3., Int_t maxNpeaks = 10);
   using KVVAMOSDetector::GetRawPositionError;
   virtual Double_t GetRawPositionError(Char_t dir = 'X', Int_t num = 0);
   virtual TH1F* GetCleanQHisto(Char_t dir = 'X');
   virtual TH1F* GetQrawHisto(Char_t dir = 'X');
   virtual TH1F* GetQHisto(Char_t dir = 'X');
   virtual KVFunctionCal* GetXfCorrectorOfT0(const Char_t* type);
   virtual void Initialize();
   virtual Bool_t PositionIsOK();
   virtual void ResetCalculatedData();
   virtual void SetACQParams();
   virtual void SetCalibrators();
   virtual void ShowCleanQHisto(Char_t dir = 'X', Option_t* opt = "");
   virtual void ShowQrawHisto(Char_t dir = 'X', Option_t* opt = "");
   virtual void ShowQHisto(Char_t dir = 'X', Option_t* opt = "");
   virtual void SetRawPosition(Double_t* XYf);



   // ------ inline functions ----------------------//

   inline virtual KVString& GetACQParamTypes()
   {
      return fACQParamTypes;
   }

   inline virtual KVString& GetPositionTypes()
   {
      return fPositionTypes;
   }

   inline Bool_t IsPositionCalibrated() const
   {
      return (fPosCalib && fPosCalib->GetStatus());
   };


   ClassDef(KVSeD, 1) //Secondary Electron Detector, used at the focal plan of VAMOS
};

#endif
