//Created by KVClassFactory on Fri Feb 17 14:52:45 2017
//Author: John Frankland,,,

#include "KVBatchSystemParametersGUI.h"

ClassImp(KVBatchSystemParametersGUI)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVBatchSystemParametersGUI</h2>
<h4>KVNameValueListGUI</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVBatchSystemParametersGUI::KVBatchSystemParametersGUI(const TGWindow* main, KVNameValueList* params, KVDataAnalyser* dan, Bool_t* cancel)
   : KVNameValueListGUI(main, params, cancel, kFALSE), fAnalyser(dan)
{
   // Default constructor
   fJN = (TGTextEntry*)GetDataWidget(GetList()->GetNameIndex("JobName"));
   fJNF = (TGTextEntry*)GetDataWidget(GetList()->GetNameIndex("AutoJobNameFormat"));
   fJNF->SetToolTipText(dan->GetRecognisedAutoBatchNameKeywords());
   fAJN = (TGCheckButton*)GetDataWidget(GetList()->GetNameIndex("AutoJobName"));
   fAJN->Connect("Toggled(Bool_t)", "KVBatchSystemParametersGUI", this, "SetAutoBatchName(Bool_t)");
   fJNF->Connect("TextChanged(const char*)", "KVBatchSystemParametersGUI", this, "UpdateAutoBatchName(const char*)");
   SetAutoBatchName(fAJN->IsDown());

   if (GetList()->HasParameter("MultiJobsMode")) {
      fMJ = (TGCheckButton*)GetDataWidget(GetList()->GetNameIndex("MultiJobsMode"));
      fRPJ = (TGNumberEntry*)GetDataWidget(GetList()->GetNameIndex("RunsPerJob"));
      fRPJ->SetLimits(TGNumberFormat::kNELLimitMinMax, 1, dan->GetNumberOfFilesToAnalyse());
      if (dan->GetNumberOfFilesToAnalyse() < 2) {
         fRPJ->SetState(kFALSE);
         fMJ->SetEnabled(kFALSE);
      }
      else {
         fMJ->SetEnabled(kTRUE);
         fMJ->SetDown();
         fRPJ->SetNumber(1);
         fMJ->Connect("Toggled(Bool_t)", "TGNumberEntry", fRPJ, "SetState(Bool_t)");
         fRPJ->SetState(fMJ->IsDown());
      }
   }
   gClient->WaitFor(GetMain());
}

//____________________________________________________________________________//

KVBatchSystemParametersGUI::~KVBatchSystemParametersGUI()
{
   // Destructor
}

void KVBatchSystemParametersGUI::UpdateAutoBatchName(const char* text)
{
   fJN->SetText(fAnalyser->ExpandAutoBatchName(text));
}

void KVBatchSystemParametersGUI::SetAutoBatchName(Bool_t on)
{
   if (on) {
      fJN->SetEnabled(kFALSE);
      fJNF->SetEnabled();
      UpdateAutoBatchName(fJNF->GetText());
   }
   else {
      fJN->SetEnabled();
      fJNF->SetEnabled(kFALSE);
   }
}

//____________________________________________________________________________//

