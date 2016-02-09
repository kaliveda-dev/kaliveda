/***************************************************************************
                          kvcalibrator.h  -  description
                             -------------------
    begin                : mer sep 18 2002
    copyright            : (C) 2002 by Alexis Mignon
    email                : mignon@ganil.fr

$Id: KVCalibrator.h,v 1.17 2007/04/11 22:52:27 franklan Exp $
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef __KV_CALIBRATOR__
#define __KV_CALIBRATOR__

#include "KVBase.h"
#include "TRef.h"
#include "KVDetector.h"
#include "TGraph.h"

class KVCalibrator: public KVBase {
protected:
   Int_t fParamNumber;          //number of parameters in the calibration formula
   Double_t* fPar;              //[fParamNumber] array of parameters
   KVDetector* fDetector;       //Detector to which this calibration corresponds
   Bool_t fReady;               //=kTRUE if the detector is calibrated i.e. if the formula has been set up


public:
   KVCalibrator();
   KVCalibrator(UShort_t pnum);
   KVCalibrator(const Char_t* name, const Char_t* type, UShort_t pnum);
   KVCalibrator(const KVCalibrator&);
   void init();
   virtual ~ KVCalibrator();

   //Two calibrator objects are considered to be the same if they are
   //of the same class and of the same type
   Bool_t IsEqual(const TObject* obj) const
   {
      return (obj->IsA() == IsA() && !strcmp(obj->GetTitle(), GetTitle()));
   };

   inline Int_t GetNumberParams() const
   {
      return fParamNumber;
   };
   void SetNumberParams(Int_t npar);
   inline Double_t GetParameter(UShort_t i) const
   {
      return fPar[i];
   };
   inline Bool_t GetStatus() const
   {
      return fReady;
   };
   virtual KVDetector* GetDetector() const
   {
      return fDetector;
   };
   virtual void SetParameter(UShort_t i, Float_t par_val)
   {
      fPar[i] = par_val;
   };
   virtual void SetDetector(KVDetector* kvd);
   virtual void SetNameFromDetector();
   virtual void SetStatus(Bool_t ready)
   {
      fReady = ready;
   };
   virtual void Print(Option_t* opt = "") const;
   virtual void Reset();
   void SetParameters(Double_t val, ...);
   virtual Double_t Compute(Double_t x) const = 0;
   virtual Double_t operator()(Double_t x) = 0;
   virtual Double_t Invert(Double_t x) = 0;
   virtual TGraph* MakeGraph(Double_t xmin, Double_t xmax, Int_t npoints =
                                50) const;

#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   virtual void Copy(TObject&) const;
#else
   virtual void Copy(TObject&);
#endif

   ClassDef(KVCalibrator, 1)    //Base class for calibration of detectors
};

inline void KVCalibrator::Reset()
{
   for (Int_t i = 0; i < fParamNumber; i++) {
      fPar[i] = 0;
   }
   SetStatus(kFALSE);
}

#endif
