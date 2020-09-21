//Created by KVClassFactory on Thu Jun 20 15:36:03 2019
//Author: John Frankland,,,

#include "KVDetector.h"
#include "KVDetectorSignalExpression.h"

ClassImp(KVDetectorSignalExpression)

KVDetectorSignalExpression::KVDetectorSignalExpression(const Char_t* type, const KVString& _expr, KVDetector* det)
   : KVDetectorSignal(type, det), fFormula(nullptr)
{
   // 'type' will be an alias for this expression
   //
   // 'expr' is any mathematically valid expression involving signals which are defined for 'det'
   //
   // If no valid signals are contained in the expression, IsValid() returns kFALSE
   // and the expression should not be used

   int nsigs = 0;
   KVString expr = _expr;
   TIter it_sig(&det->GetListOfDetectorSignals());
   KVDetectorSignal* ds;
   fRaw = kTRUE;
   while ((ds = (KVDetectorSignal*)it_sig())) {
      if (expr.Contains(ds->GetName())) {
         fSignals.push_back(ds);
         if (!ds->IsRaw()) fRaw = kFALSE;
         expr.ReplaceAll(ds->GetName(), Form("[%d]", nsigs));
         ++nsigs;
      }
   }
   if (nsigs) {
#if ROOT_VERSION_CODE < ROOT_VERSION(6,0,0)
      fFormula = new TFormula(type, expr);
#else
      fFormula.reset(new TFormula(type, expr));
#endif
      fValid = kTRUE;
   }
   else
      fValid = kFALSE;

   SetTitle(Form("Signal calculated as %s for detector %s", _expr.Data(), det->GetName()));
}

Double_t KVDetectorSignalExpression::GetValue(const KVNameValueList& params) const
{
   // Evaluate the expression using all current values of signals
   //
   // Any extra parameters required by any calibrations in the expression can be passed
   // as a comma-separated list of "param=value" pairs

   int nsigs = fSignals.size();
   for (int i = 0; i < nsigs; ++i) {
      fFormula->SetParameter(i, fSignals[i]->GetValue(params));
   }
   return fFormula->Eval(0);
}
