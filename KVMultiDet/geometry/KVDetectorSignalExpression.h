//Created by KVClassFactory on Thu Jun 20 15:36:03 2019
//Author: John Frankland,,,

#ifndef __KVDETECTORSIGNALEXPRESSION_H
#define __KVDETECTORSIGNALEXPRESSION_H

#include "KVDetectorSignal.h"
#include "TFormula.h"

/**
 \class KVDetectorSignalExpression
 \ingroup Calibration
 \brief Signal output from a mathematical combination of other signals

 Detector signals (see KVDetectorSignal) can be combined in mathematical expressions in order to define
 new signals - these are KVDetectorSignalExpression objects. See TFormula for valid mathematical operators,
 functions, etc.

 ### Example of use
 ~~~~{.cpp}
 KVDetector det;
 auto A = det.AddDetectorSignal("A");
 auto B = det.AddDetectorSignal("B");

 det.AddDetectorSignalExpression("C", "2*A+TMath::Sqrt(B)");

 A->SetValue(6);
 B->SetValue(16);

 det.GetDetectorSignalValue("C")
 (double) 16.000000

det.GetListOfDetectorSignals().ls()
OBJ: KVUniqueNameList   KVSeqCollection_156  Optimised list in which objects with the same name can only be placed once : 0
 KVDetectorSignal        A        Signal A of detector Det_1       [6.000000]
 KVDetectorSignal        B        Signal B of detector Det_1       [16.000000]
 KVDetectorSignalExpression       C        Signal calculated as 2*A+TMath::Sqrt(B) for detector Det_1        [16.000000]
 ~~~~
 */

class KVDetectorSignalExpression : public KVDetectorSignal {

   friend class KVDetector;

#if ROOT_VERSION_CODE < ROOT_VERSION(6,0,0)
   TFormula* fFormula;//!
#else
   std::unique_ptr<TFormula> fFormula;
#endif
   std::vector<KVDetectorSignal*> fSignals;
   Bool_t fValid;
   Bool_t fRaw;
   KVDetectorSignalExpression(const Char_t* type, const KVString& _expr, KVDetector* det);

public:
   virtual ~KVDetectorSignalExpression()
   {
#if ROOT_VERSION_CODE < ROOT_VERSION(6,0,0)
      SafeDelete(fFormula);
#endif
   }

   Double_t GetValue(const KVNameValueList& params = "") const;
   Bool_t IsValid() const
   {
      // \returns kTRUE if the expression is valid (all signals are known and all mathematical operations are understood by TFormula)
      return fValid;
   }
   Bool_t IsRaw() const
   {
      // \returns kFALSE as this is not a raw data parameter, in the sense that it should not be used to decide whether
      // the detector fired in the last read raw data
      return kFALSE;
   }
   Bool_t IsExpression() const
   {
      // \returns kTRUE, obviously
      return kTRUE;
   }
   void SetValue(Double_t)
   {
      // Has no effect for an expression: its value cannot be set, only evaluated.
      Warning("SetValue", "[%s] : Calling SetValue for a signal expression has no effect", GetName());
   }
   KVString GetExpression() const
   {
      // \returns the mathematical expression used to define this signal
      return GetLabel();
   }

   ClassDef(KVDetectorSignalExpression, 1) //Mathematical expression involving one or more detector signals
};

#endif
