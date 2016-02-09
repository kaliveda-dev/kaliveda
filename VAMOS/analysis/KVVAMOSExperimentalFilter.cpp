//Created by KVClassFactory on Wed Jun 17 17:04:27 2015
//Author: Dijon Aurore

#include "KVVAMOSExperimentalFilter.h"

ClassImp(KVVAMOSExperimentalFilter)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVVAMOSExperimentalFilter</h2>
<h4>VAMOS filter created from experimental data</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVVAMOSExperimentalFilter::KVVAMOSExperimentalFilter()
{
   // Default constructor
}

//________________________________________________________________

KVVAMOSExperimentalFilter::KVVAMOSExperimentalFilter(Int_t nbins_delta, Double_t delta_low, Double_t delta_up, Int_t nbins_thetav, Double_t thetav_low, Double_t thetav_up, Double_t phiv_low, Double_t phiv_up) : TProfile2D("VAMOSfilter", "VAMOS experimental filter;#delta;#theta_{V} (deg)", nbins_delta, delta_low, delta_up, nbins_thetav, thetav_low, thetav_up, phiv_low, phiv_up, "s")
{
}

//________________________________________________________________

KVVAMOSExperimentalFilter::KVVAMOSExperimentalFilter(Int_t nbins_delta, Double_t delta_low, Double_t delta_up, Int_t nbins_thetav, Double_t thetav_low, Double_t thetav_up) : TProfile2D("VAMOSfilter", "VAMOS experimental filter;#delta;#theta_{V} (deg)", nbins_delta, delta_low, delta_up, nbins_thetav, thetav_low, thetav_up, "s")
{
}
//________________________________________________________________

KVVAMOSExperimentalFilter::~KVVAMOSExperimentalFilter()
{
   // Destructor
}
//________________________________________________________________

void KVVAMOSExperimentalFilter::Copy(TObject& obj) const
{
   // This method copies the current state of 'this' object into 'obj'
   // You should add here any member variables, for example:
   //    (supposing a member variable KVVAMOSExperimentalFilter::fToto)
   //    CastedObj.fToto = fToto;
   // or
   //    CastedObj.SetToto( GetToto() );

   TProfile2D::Copy(obj);
   KVVAMOSExperimentalFilter& CastedObj = (KVVAMOSExperimentalFilter&)obj;
   CastedObj.fCurBin       = fCurBin;
   CastedObj.fCurBinDelta  = fCurBinDelta;
   CastedObj.fCurBinThetaV = fCurBinThetaV;
   CastedObj.fRunList      = fRunList;
}
//________________________________________________________________

Bool_t KVVAMOSExperimentalFilter::IsTrajectoryAccepted(Double_t delta, Double_t thetav, Double_t phiv)
{

   fCurBin = fCurBinDelta = fCurBinThetaV = -1;

   Int_t bin = FindBin(delta, thetav);
   if (IsBinUnderflow(bin) ||  IsBinOverflow(bin)) return kFALSE;

   fCurBin = bin;
   Int_t bz;
   GetBinXYZ(bin, fCurBinDelta, fCurBinThetaV, bz);

   return ((PhiV() - DPhiV() / 2 <= phiv) && (phiv <= PhiV() + DPhiV() / 2));
}
