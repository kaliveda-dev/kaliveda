//Created by KVClassFactory on Mon Sep  3 11:29:00 2012
//Author: Guilain ADEMARD

#ifndef __KVDRIFTCHAMBER_H
#define __KVDRIFTCHAMBER_H
#define Border 5
#define _DC_N_STRIPS_ 64

#include "KVVAMOSDetector.h"
#include "KVUnits.h"
#include "KVDataSet.h"

class TH1F;

class KVDriftChamber : public KVVAMOSDetector {
private:

   static KVString fACQParamTypes; //!types of available Acquision parameters
   static KVString fPositionTypes; //!types of available positions
   static Int_t    fNstripsOK;     //!minimum. numbers of strips for a good position measurement


protected:

   TH1F***    fQ; //! array of TH1F for calibrated charge [raw, calibrated, clean][Chamber 1, Chamber 2]
   Double_t   fRawPosX[3]; //! Measured X raw position for both Chambers
   Double_t   fERawPosX[3]; //! Error of measured X raw position for both Chambers
   Double_t   fRawPosY; //!  Measured Y raw position
   Double_t   fERawPosY; //! Error of measured Y raw position
   KVEBYEDAT_ACQParam* fTfilPar;   //! TFIL acquisition parameter
   Float_t     fDriftV;    //! Electron drift velocity in cm/us
   KVFunctionCal* fTfilCal;//! Calibrator of the  TFIL acquisition parameter
   Float_t     fStripWidth;//! X-Strip width
   Float_t     fOffsetZ[3];// Z offset in cm for Y, X1 and X2 measurements
   Float_t     fOffsetX[2];// X1 and X2 offset in fraction of the strip width
   Int_t       fPadMax[2];// Pad with maximum charge from the histogram representing the calibrated charge versus strip number
   Bool_t      fkSECHS;    //! Set kTRUE to use the Hyperbolic Secant Squared (SECHS) method to find X positions, else will use the Mean's and RMS's of the charges
   Bool_t      fkRawPosSetByHand; //! To know if the raw position was set/modified using the SetRawPosition() method

   void init();

public:
   KVDriftChamber();
   KVDriftChamber(const KVDriftChamber&) ;
   virtual ~KVDriftChamber();
   void Copy(TObject&) const;

   virtual Float_t CalculateQThreshold(Int_t c_num);
   virtual void  Clear(Option_t* option = "");

   virtual TH1F*    GetCleanQHisto(Int_t c_num);
   virtual TH1F*    GetQHisto(Int_t c_num);
   virtual TH1F*    GetQrawHisto(Int_t c_num);
   using KVVAMOSDetector::GetRawPosition;
   virtual Double_t GetRawPosition(Char_t dir = 'X', Int_t num = 0);
   using KVVAMOSDetector::GetRawPositionError;
   virtual Double_t GetRawPositionError(Char_t dir = 'X', Int_t num = 0);

   virtual void  Initialize();
   virtual const Char_t* GetArrayName();
   virtual const Char_t* GetEBaseName() const;
   virtual Bool_t PositionIsOK();
   virtual void  ResetCalculatedData();
   virtual void  SetACQParams();
   virtual void  SetPressure(Double_t);
   virtual void  SetTemperature(Double_t);

   virtual void ShowCleanQHisto(Int_t c_num = 1, Option_t* opt = "");
   virtual void ShowQrawHisto(Int_t c_num = 1, Option_t* opt = "");
   virtual void ShowQHisto(Int_t c_num = 1, Option_t* opt = "");
   virtual void SetRawPosition(Double_t* X1_X2_Y_Pad1_Pad2);

   virtual UChar_t GetPosition(Double_t* XYZf, Char_t dir = 0, Int_t num = 0);
   virtual void    GetDeltaXYZf(Double_t* XYZf, Char_t dir = 0, Int_t num = 0);


   // ------ inline functions ----------------------//

   virtual KVString& GetACQParamTypes();
   virtual KVString& GetPositionTypes();
   Float_t   GetDriftVelocity() const;
   void      SetDriftVelocity(Float_t v);
   KVFunctionCal* GetDriftTimeCalibrator() const;
   virtual Double_t  GetPressure() const;
   Float_t   GetStripWidth() const;
   Bool_t    IsPositionCalibrated() const;
   void      SetZOffsets(Float_t X1 = -2.5, Float_t X2 = 2.5, Float_t Y = 0);
   void      SetXOffsets(Float_t X1 = 0., Float_t X2 = 0.);
   Int_t     GetPadMax(Int_t row_num);
   void      SetSECHSReconstructionX(Bool_t kSECHS = kTRUE);
   Bool_t    IsSECHSReconstructionX();
   void      ResetRawPositions();
   Bool_t    IsRawPosSetByHand();

   ClassDef(KVDriftChamber, 1) //Drift Chamber, used at the focal plan of VAMOS
};
//________________________________________________________________

inline KVString& KVDriftChamber::GetACQParamTypes()
{
   return fACQParamTypes;
}
//________________________________________________________________

inline KVString& KVDriftChamber::GetPositionTypes()
{
   return fPositionTypes;
}
//________________________________________________________________

inline Float_t KVDriftChamber::GetDriftVelocity() const
{
   return fDriftV;
}
//________________________________________________________________

inline void KVDriftChamber::SetDriftVelocity(Float_t v)
{
   fDriftV = v;
}
//________________________________________________________________

inline KVFunctionCal* KVDriftChamber::GetDriftTimeCalibrator() const
{
   // Returns the calibrator used to calibrate the electron-drift
   // time (canal->ns).

   if (fTfilCal) return fTfilCal;
   TObject* obj =  GetListOfCalibrators()->FindObjectByLabel("T_FIL");
   if (obj->InheritsFrom(KVFunctionCal::Class()))
      return const_cast<KVDriftChamber*>(this)->fTfilCal = (KVFunctionCal*)obj;
   return NULL;
}
//________________________________________________________________

inline  Double_t KVDriftChamber::GetPressure() const /* mbar */
{
   // Give pressure of gas in mbar
   return KVDetector::GetPressure() / KVUnits::mbar;
}
//________________________________________________________________

inline Float_t KVDriftChamber::GetStripWidth() const
{
   return fStripWidth;
}
//________________________________________________________________

inline Bool_t KVDriftChamber::IsPositionCalibrated() const
{
   return GetDriftTimeCalibrator() && GetDriftTimeCalibrator()->GetStatus();
}
//________________________________________________________________

inline void KVDriftChamber::SetZOffsets(Float_t X1, Float_t X2, Float_t Y)
{
   // Sets the values of Z offsets for X1, X2 and Y position measurements.
   // By default these offsets are given by the position in the detector of :
   //   - the first  line of pads of the cathode ( -2.5 cm ) for X1
   //   - the second line of pads of the cathode ( +2.5 cm ) for X2
   //   - the center of the detector ( 0 cm ) for Y

   fOffsetZ[0] = Y;
   fOffsetZ[1] = X1;
   fOffsetZ[2] = X2;
}
//________________________________________________________________

inline void KVDriftChamber::SetXOffsets(Float_t X1, Float_t X2)
{
   // Sets the values of X1 and X2 offsets in fraction of the strip width.
   // By default the  The two cathode plans are offset by half a strip to
   // reduce the non linearity of the position measurement in between the
   // strips. The center of the detector is at the middle of the strip 32
   // of the range 2 and between strips 32 and 33 of the range 1 (offsets 0,0).
   // With this command you can change the strip position with respect to
   // the center of the detector for each plan.
   //
   // For example, if you want to move the plan 2 in order to have its strip
   // 35 at the center and keep the range 1 at its default position then use
   // SetXOffsets( 0., 3.);
   fOffsetX[0] = X1;
   fOffsetX[1] = X2;
}
//________________________________________________________________

inline Int_t KVDriftChamber::GetPadMax(Int_t row_num)
{
   // Returns the pad with the maximum charge from the histogram representing
   // the calibrated charge versus strip number for the chosen row of the drift
   // chamber.
   // row_num = {1,2}

   if (row_num < 3 && row_num > 0) return fPadMax[ row_num - 1 ];
   else return -1;
}

inline void KVDriftChamber::SetSECHSReconstructionX(Bool_t ksechs)
{
   fkSECHS = ksechs;
}
//________________________________________________________________

inline Bool_t KVDriftChamber::IsSECHSReconstructionX()
{
   return fkSECHS;
}
//________________________________________________________________

inline void KVDriftChamber::ResetRawPositions()
{
   fkRawPosSetByHand = kFALSE;
}

inline Bool_t KVDriftChamber::IsRawPosSetByHand()
{
   return fkRawPosSetByHand;
}
//________________________________________________________________

#endif
