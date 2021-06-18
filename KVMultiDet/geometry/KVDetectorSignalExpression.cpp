//Created by KVClassFactory on Thu Jun 20 15:36:03 2019
//Author: John Frankland,,,

#include "KVDetector.h"
#include "KVDetectorSignalExpression.h"

ClassImp(KVDetectorSignalExpression)

KVDetectorSignalExpression::KVDetectorSignalExpression(const Char_t* type, const KVString& _expr, KVDetector* det)
   : KVDetectorSignal(type, det), fFormula(nullptr)
{
   // \param[in] type the typename for this expression, will be used as an alias for the expression
   // \param[in] _expr a mathematical expression using names of signals already defined for detector \a det. See TFormula
   //             class for valid operators/functions.
   // \param[in] det the detector to which this expression is to be associated
   //
   // If no valid signals are contained in the expression (i.e. signals not already defined for the
   // detector \a det), IsValid() returns kFALSE and the expression should not be used

   int nsigs = 0;
   KVString expr = _expr;
   SetLabel(_expr);
   TIter it_sig(&det->GetListOfDetectorSignals());
   KVDetectorSignal* ds;
   fRaw = kTRUE;
   while ((ds = (KVDetectorSignal*)it_sig())) {
      if (expr.Contains(ds->GetName())) {
         fSignals.push_back(ds);
         if (!ds->IsRaw()) fRaw = kFALSE;
         expr.ReplaceAll(ds->GetName(), Form("[%d]", nsigs));
         if (expr.CompareTo(_expr) != 0) ++nsigs; // a replacement was made => a signal was found
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
   // \param[in] params comma-separated list of "param=value" pairs if extra parameters are required to evaluate any signals in the expression
   // \returns value of the expression using all current values of signals

   int nsigs = fSignals.size();
   for (int i = 0; i < nsigs; ++i) {
      fFormula->SetParameter(i, fSignals[i]->GetValue(params));
   }
   return fFormula->Eval(0);
}
