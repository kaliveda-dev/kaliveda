//Created by KVClassFactory on Fri Jun 19 10:59:24 2015
//Author: Dijon Aurore

#include "KVBasicVAMOSFilter.h"
#include "Riostream.h"
#include "TCutG.h"
using namespace std;

ClassImp(KVBasicVAMOSFilter)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVBasicVAMOSFilter</h2>
<h4>Basic filter for VAMOS using simple TCuts on delta vs ThetaV matrices</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVBasicVAMOSFilter::KVBasicVAMOSFilter()
{
   SetName("BasicVAMOSfilter");
   // Default constructor
   fCutList = NULL;
   fNcuts = 0;
}
//________________________________________________________________

KVBasicVAMOSFilter::~KVBasicVAMOSFilter()
{
   // Destructor
}
//________________________________________________________________

void KVBasicVAMOSFilter::Copy(TObject& obj) const
{
   // This method copies the current state of 'this' object into 'obj'
   // You should add here any member variables, for example:
   //    (supposing a member variable KVBasicVAMOSFilter::fToto)
   //    CastedObj.fToto = fToto;
   // or
   //    CastedObj.SetToto( GetToto() );

   KVBase::Copy(obj);
   //KVBasicVAMOSFilter& CastedObj = (KVBasicVAMOSFilter&)obj;
}
//________________________________________________________________

KVCutList* KVBasicVAMOSFilter::FindCutListForThetaVamos(Float_t th_vamos)
{

   return fCutList = (KVCutList*)fMainList.FindObject(GetCutListName(th_vamos));
}
//________________________________________________________________

const Char_t* KVBasicVAMOSFilter::GetCutListName(Float_t th_vamos)
{
   return KVCutList::GetCutListName(th_vamos);
}
//________________________________________________________________

KVCutList* KVBasicVAMOSFilter::NewCutListForThetaVamos(Float_t th_vamos)
{
   KVCutList* l = FindCutListForThetaVamos(th_vamos);
   if (l) return l;
   fMainList.Add(l = new KVCutList);
   l->SetThetaVamos(th_vamos);
   return l;
}
//________________________________________________________________

void KVBasicVAMOSFilter::AddCutForThetaVamos(Float_t th_vamos, TCutG* cut)
{
   KVCutList* l = NewCutListForThetaVamos(th_vamos);
   cut->SetName(Form("cut%d", fNcuts));
   l->Add(cut);
   fNcuts++;
}
//________________________________________________________________

Bool_t KVBasicVAMOSFilter::IsTrajectoryAccepted(Float_t th_vamos, Double_t delta, Double_t thetav, Double_t phiv)
{

   thetav -= th_vamos;

   if (!IsInPhiVrange(phiv)) {
//    Info("IsTrajectoryAccepted","out of PhiV range");
      return kFALSE;
   }
   if (!IsInThetaVrange(thetav)) {
//    Info("IsTrajectoryAccepted","out of ThetaV range");
      return kFALSE;
   }
   if (!IsInDeltaRange(delta)) {
//    Info("IsTrajectoryAccepted","out of delta range");
      return kFALSE;
   }

   if (!fCutList || (fCutList->GetThetaVamos() != th_vamos))
      FindCutListForThetaVamos(th_vamos);

   if (!fCutList) {
      Info("IsTrajectoryAccepted", "no cutlist of theta_vamos= %.1f", th_vamos);
      return kFALSE;
   }

   TIter next(fCutList);
   TCutG* cut = NULL;
   while ((cut = (TCutG*)next())) {
      if (cut->IsInside(thetav, delta)) return kTRUE;
   }

// Info("IsTrajectoryAccepted","out of all cuts");

   return kFALSE;
}
