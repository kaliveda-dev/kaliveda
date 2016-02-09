/***************************************************************************
$Id: KVCsI.cpp,v 1.38 2009/04/09 09:25:43 ebonnet Exp $
                          kvcsi.cpp  -  description
                             -------------------
    begin                : Thu May 16 2002
    copyright            : (C) 2002 by J.D. Frankland
    email                : frankland@ganil.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "Riostream.h"
#include "KVCsI.h"
#include "KVGroup.h"
#include "KVChIo.h"
#include "KVTelescope.h"
#include "KVACQParam.h"
#include "KVMaterial.h"
#include "KVLightEnergyCsI.h"
#include "KVIDZAGrid.h"
#include "KVIDZALine.h"
#include "KVIDCutLine.h"
#include <TCanvas.h>
#include "TMarker.h"
#include "KVIDTelescope.h"

using namespace std;

ClassImp(KVCsI);
//_______________________________________________________________________________________
//
//Child class of KVDetector, specifically describing the
//CsI(Tl) scintillation detectors of the INDRA multidetector array.
//
//Type of detector: "CSI"
//
//Calibration:
//  Two KVLightEnergyCsI calibrators are used, one for Z=1, the other for Z>1

//_______________________________________________________________________________________

void KVCsI::init()
{
   //initialise non-persistent pointers
   fSegment = 2;
   fLumiereTotale = 0.0;
   fLumTotStatus = NOT_CALCULATED;
   fCal = fCalZ1 = 0;
   fPinLaser = 0;
   fGainCorrection = 1;
   fACQ_R = 0;
   fACQ_L = 0;
}

//______________________________________________________

KVCsI::KVCsI()
{
   //Default ctor
   //This ctor (which in turn calls the KVDetector default ctor) must exist in
   //order for Cloning of detectors to work (as used in KVTelescope::AddDetector).
   //Do not replace this ctor by giving a default value for the argument of KVCsI(Float_t).
   //
   init();
}

//______________________________________________________________________________
KVCsI::KVCsI(Float_t thick, Float_t thickAl): KVINDRADetector("CsI", thick)
{
   //Make a CsI detector "thick" cm long
   //with a Al dead layer "thickAl" um long
   //Set type of detector to "CSI"
   //By default 'thick'=0

   if (thickAl > 0.0) {
      KVMaterial* mat = new KVMaterial("Al", thickAl * KVUnits::um);
      fAbsorbers->AddFirst(mat);
      SetActiveLayer(1);
   }
   SetType("CSI");
   init();
}

void KVCsI::SetAlThickness(Float_t thickAl /* um */)
{
   KVMaterial* mat = 0;
   if (!(mat = GetAbsorber("Al"))) {
      mat = new KVMaterial("Al", thickAl * KVUnits::um);
      fAbsorbers->AddFirst(mat);
      SetActiveLayer(1);
   } else mat->SetThickness(thickAl * KVUnits::um);
}

//____________________________________________________________________________________________

KVCsI::~KVCsI()
{
}

//______________________________________________________________________________________________

Double_t KVCsI::GetLumiereTotale(Double_t rapide, Double_t lente)
{
   // Returns calculated total light output for a CsI detector from its pedestal-
   // corrected measured fast and slow components.
   // If no arguments are given, the detector's own ACQData parameters ("R" and "L")
   // are used, with pedestal subtraction.
   //
   // NOTE: Simulations
   // If detector is in 'SimMode', then we look for a KVSimNucleus in the list of particles
   // which hit the detector in the event, and use the Z & A of this nucleus and the energy
   // deposited in the CsI to calculate the light; then we use the Z & A of 'nuc' (not necessarily
   // the same) to calculate the calibrated energy from the light.

   if (IsSimMode()) {
      //GetHits()->ls();
      if (GetACQParam("T")->Fired()) {
         return Calculate(kLumiere, rapide, lente);
      }
      KVNucleus* nunuc = 0;
      if (GetNHits() > 0) {
         Int_t zz = 0, aa = 0;
         TIter nxthit(GetHits());
         while ((nunuc = (KVNucleus*)nxthit())) {
            zz = nunuc->GetZ();
            aa = nunuc->GetA();
         }
         return GetLightFromEnergy(zz, aa, GetEnergy());
      } else {
         return -1;
      }
      /*
      KVNucleus *nunuc = 0;
      TIter nxthit(GetHits()); KVNucleus *nunuc, *simnuc=0;
      while( (nunuc = (KVNucleus*)nxthit()) ) {if(nunuc->InheritsFrom("KVSimNucleus")) simnuc=nunuc;}
      if(!simnuc) return -1.;
      */
      return GetLightFromEnergy(nunuc->GetZ(), nunuc->GetA(), GetEnergy());
   }
   return Calculate(kLumiere, rapide, lente);
}

//______________________________________________________________________________________________

Double_t KVCsI::GetCorrectedLumiereTotale(Double_t rapide, Double_t lente)
{
   // Returns total light output corrected for gain variations, if known (see KVINDRADB::ReadCsITotalLightGainCorrections
   // for how corrections are read into database, format of correction files, etc.).
   // Correction is calculated using
   //      rap_corr = (rapide-piedR) * fGainCorrection + piedR
   //      len_corr = (lente-piedL) * fGainCorrection + piedL
   //      lum_corr = GetLumiereTotale( rap_corr, len_corr )
   // For more details on arguments, etc., see GetLumiereTotale.

   Double_t rap_corr;
   Double_t len_corr;
   rap_corr = (rapide < 0 ? GetR() : rapide);
   len_corr = (lente < 0 ? GetL() : lente);
   Double_t piedR = fACQ_R->GetPedestal();
   Double_t piedL = fACQ_L->GetPedestal();
   rap_corr = (rap_corr - piedR) * fGainCorrection + piedR;
   len_corr = (len_corr - piedL) * fGainCorrection + piedL;
   return GetLumiereTotale(rap_corr, len_corr);
}

//______________________________________________________________________________________________

Double_t KVCsI::GetTauZero(Double_t rapide, Double_t lente)
{
   //Returns "tau_0" for a CsI detector. This is the time constant for the exponential
   //fall-off of the light output, i.e. i(t) ~ exp( - t / tau_0 ). See Parlog et al.
   //for details.
   //If no arguments are given, the detector's own ACQData parameters ("R" and "L")
   //are used, with pedestal subtraction.

   return Calculate(kTau, rapide, lente);
}

//______________________________________________________________________________________________

Double_t KVCsI::Calculate(UShort_t mode, Double_t rapide, Double_t lente)
{
   //Private method
   //This function is called by GetLumiereTotale and GetTauZero, the return value depends
   //on the value of 'mode' (kLumiere or kTau).
   //The 'TauZero' in the code is the variable 'x'.
   //In either case, if problems are encountered in the calculation, the value returned
   //is zero and fLumTotStatus should give information on the problem (GetStatusLumiere).

   UInt_t ring = GetRingNumber();
   UInt_t module = GetModuleNumber();

   //pedestal-corrected fast and slow components from raw data
   Double_t rap_corr;
   Double_t len_corr;
   rap_corr = (rapide < 0 ? GetR() : rapide);
   len_corr = (lente < 0 ? GetL() : lente);

   if (rap_corr <= 0. && len_corr <= 0.) {
      //no R/L coder values set
      fLumiereTotale = 0.0;
      return fLumiereTotale;
   }
   rap_corr -= fACQ_R->GetPedestal();
   len_corr -= fACQ_L->GetPedestal();

   if (rap_corr < 0 || len_corr < 0) {
      fLumTotStatus = NEGATIVE_PEDESTAL_CORRECTED_VALUE;
      return (fLumiereTotale = 0.0);
   }
   Double_t p0 = 400;
   Double_t p1 = 900;
   Double_t eps = 1.e-4;
   /* Cette variable n'est pas utilisee, et ne figure pas dans e.g. le fortran de la 4eme campagne
    Float_t pre=0.4318;
   */
   Double_t c1 = 1.5;
   Double_t c2;
   Double_t x;
   Double_t x2;
   Double_t x3;
   Double_t bx;
   /****************************************************************************
   * Definition of PM rise-time constant tau
   * Comment based on a note from Bernard Borderie:
   *   rings 11-16 use PM bases with a different component, thus tau is 60 nsec
   *   ring 16, module 5 is an exception, it has a tau of 20 nsec !!
   *   rung 5, module 11 is also an exception, it has a tau of 60 nsec !!
   */
   Double_t tau = 20.;
   if (ring >= 11 && ring <= 16)
      tau = 60.;
   if (ring == 16 && module == 5)
      tau = 20.;
   if (ring == 5 && module == 11)
      tau = 60.;
   /****************************************************************************/
   Double_t tau0 = 390.;
   Double_t tau1 = 1590.;
   Double_t tau2 = 3090.;

   if (ring > 3) {
      c2 = 2.2;
   } else {
      c2 = 3.3;
   }
   Double_t dx;
   Double_t rp = (len_corr / rap_corr) * (c1 / c2);
   if (rp >= 1 && rap_corr <= 10) {
      //calculation without correction for PM gain
      x = 600;
      x2 = TMath::Exp(-tau1 / x);
      x3 = TMath::Exp(-tau2 / x);
      bx = x2 - x3;
      fLumTotStatus = NO_GAIN_CORRECTION;
   } else {
      //iterative calculation of total light output
      fLumTotStatus = CALCULATED_WITH_GAIN_CORRECTION;
      x = p0 + p1 * rp;
      x = x - 100;
      Double_t at = 1. - TMath::Exp(-tau0 / tau);
      at = tau * at;
      Int_t niter = 0;
      do {
         niter++;
         Double_t xx = x * x;
         Double_t x1 = TMath::Exp(-tau0 / x);
         x2 = TMath::Exp(-tau1 / x);
         x3 = TMath::Exp(-tau2 / x);
         Double_t ax = 1. - x1;
         ax = ax - at / x;
         bx = x2 - x3;
         Double_t fx = bx / ax - rp;
         Double_t apx = -tau0 * x1;
         apx = (apx + at) / xx;
         Double_t bpx = tau1 * x2 - tau2 * x3;
         bpx = bpx / xx;
         Double_t fpx = (bpx * ax - apx * bx) / (ax * ax);
         dx = -fx / fpx;
         x = x + dx;
      } while (TMath::Abs(dx / x) >= eps && niter <= 50);

      if (niter > 50 || x <= tau) {
         fLumTotStatus = CALCULATION_NOT_CONVERGED;
         //Warning("GetLumiereTotale","Iterative calculation did not converge for %s",
         //       GetName());
         return (fLumiereTotale = 0.0);
      }
   }

   //GetLumiereTotale calling function
   if (mode == kLumiere)
      return (fLumiereTotale = len_corr * (x - tau) / (x * bx));

   //GetTauZero calling function
   return x;
}

//_______________________________________________________________________________________________

UInt_t KVCsI::GetStatusLumiere()
{
   //Status concerning the calculation of the total light output for
   //this CsI detector. The possible values have been defined in the KVCsI.h
   //header file (you can use the named constants for comparisons):
   // NOT_CALCULATED                    = 0
   // CALCULATED_WITH_GAIN_CORRECTION   = 1
   // NO_GAIN_CORRECTION                = 4
   // NEGATIVE_PEDESTAL_CORRECTED_VALUE = 15
   // CALCULATION_NOT_CONVERGED         = 12

   return fLumTotStatus;
}

//_______________________________________________________________________________________________

Bool_t KVCsI::LightIsGood()
{
   //Returns kTRUE if total light output has been successfully calculated
   //for the CsI, either with (KVCsI::GetStatusLumiere()=CALCULATED_WITH_GAIN_CORRECTION)
   //or without (KVCsI::GetStatusLumiere()=NO_GAIN_CORRECTION) iterative gain correction.

   if (GetStatusLumiere() == CALCULATED_WITH_GAIN_CORRECTION
         || GetStatusLumiere() == NO_GAIN_CORRECTION)
      return kTRUE;

   return kFALSE;
}

//_______________________________________________________________________________________________

void KVCsI::Clear(Option_t* opt)
{
//Redefinition of KVDetector::Clear - set fLumiereTotale and status to zero
   KVDetector::Clear(opt);
   fLumiereTotale = 0.0;
   fLumTotStatus = NOT_CALCULATED;
}


void KVCsI::Print(Option_t* option) const
{
   //Print info on this detector - redefinition of KVDetector::Print for option="data" to include total light
   //if option="data" the energy loss, total light and coder channel data are displayed

   if (!strcmp(option, "data")) {
      cout << ((KVCsI*) this)->GetName() << " -- H=" << ((KVCsI*) this)->
           GetLumiereTotale() << " -- E=" << ((KVCsI*) this)->
           GetEnergy() << "  ";
      TIter next(fACQParams);
      KVACQParam* acq;
      while ((acq = (KVACQParam*) next())) {
         cout << acq->GetName() << "=" << (Short_t) acq->
              GetCoderData() << "/" << TMath::Nint(acq->GetPedestal()) << "  ";
      }
      if (BelongsToUnidentifiedParticle())
         cout << "(Belongs to an unidentified particle)";
      cout << endl;
   } else {
      KVDetector::Print(option);
   }
}

void KVCsI::SetACQParams()
{
   //Set acquisition parameters for this CsI.
   //Do not call before detector's name has been set.
   //Initialises member pointers fACQ_R & fACQ_L for (fast) direct access.

   AddACQParamType("R");
   AddACQParamType("L");
   AddACQParamType("T");
   fACQ_R = GetACQParam("R");
   fACQ_L = GetACQParam("L");
}

void KVCsI::SetCalibrators()
{
   //Set up calibrators for this detector. Call once name has been set.
   //Two KVLightEnergyCsI calibrators are used, one for Z=1, the other for Z>1
   fCalZ1 = new KVLightEnergyCsI(this);
   fCalZ1->SetType("Light-Energy CsI Z=1");
   AddCalibrator(fCalZ1);
   fCal = new KVLightEnergyCsI(this);
   fCal->SetType("Light-Energy CsI Z>1");
   AddCalibrator(fCal);
}

//______________________________________________________________________________

void KVCsI::Streamer(TBuffer& R__b)
{
   // Stream an object of class KVCsI.
   // We set the pointers to the calibrator objects

   if (R__b.IsReading()) {
      UInt_t R__s, R__c;
      Version_t R__v = R__b.ReadVersion(&R__s, &R__c);
      KVCsI::Class()->ReadBuffer(R__b, this, R__v, R__s, R__c);
      fCalZ1 = (KVLightEnergyCsI*) GetCalibrator("Light-Energy CsI Z=1");
      fCal = (KVLightEnergyCsI*) GetCalibrator("Light-Energy CsI Z>1");
      // backwards compatibility for CsI with only one calibrator
      if (!fCal)  fCal = (KVLightEnergyCsI*) GetCalibrator("Light-Energy CsI");
      if (R__v < 4) {
         // backwards compatibility: persistent member pointers to acquisition
         // parameters fACQ_R & fACQ_L introduced in class version 4 (10 june 2009)
         // As the detector name is not always available at this point
         // (KVDetector::GetName generates dynamically the name from the
         // module & ring numbers, no persistent name stored in TNamed::fName)
         // we have to loop over the list of acquisition parameters (if it exists)
         // and look for parameters which end with "R" or "L"
         if (fACQParams) {
            TIter next(fACQParams);
            KVACQParam* par = 0;
            while ((par = (KVACQParam*)next())) {
               TString name(par->GetName());
               if (name.EndsWith("R")) fACQ_R = par;
               else if (name.EndsWith("L")) fACQ_L = par;
            }
         }
      }
   } else {
      KVCsI::Class()->WriteBuffer(R__b, this);
   }
}

//______________________________________________________________________________

Double_t KVCsI::GetCorrectedEnergy(KVNucleus* nuc, Double_t lum, Bool_t)
{
   // Calculate calibrated energy loss for a nucleus (Z,A) giving total light output "lum".
   // If "lum" is not given, the total light of the detector
   // calculated from the current values of the "R" and "L" acquisition
   // parameters will be used (taking into account an eventual correction for gain variations,
   // see GetCorrectedLumiereTotale()).
   //
   //Two KVLightEnergyCsI calibrators are used, one for Z=1, the other for Z>1
   // Returns -1 in case of problems (no calibration available or light calculation not valid).
   //
   // NOTE: Simulations
   // If detector is in 'SimMode', then we look for a KVSimNucleus in the list of particles
   // which hit the detector in the event, and use the Z & A of this nucleus and the energy
   // deposited in the CsI to calculate the light; then we use the Z & A of 'nuc' (not necessarily
   // the same) to calculate the calibrated energy from the light.

   Int_t Z = nuc->GetZ();
   Int_t A = nuc->GetA();

   KVLightEnergyCsI* calib = 0;

   if (Z == 1 && fCalZ1) calib = fCalZ1;
   else calib = fCal;

   if (calib && calib->GetStatus()) {
      if (IsSimMode()) {
         lum = GetLumiereTotale();
         if (lum < 0.) return -1.;
         //force "OK" status for light
         fLumTotStatus = NO_GAIN_CORRECTION;
      } else if (lum < 0.) {
         //light not given - calculate from R and L components
         lum = GetCorrectedLumiereTotale(); // include gain correction
      } else {
         //light given as argument - force "OK" status for light
         fLumTotStatus = NO_GAIN_CORRECTION;
      }

      //check light calculation status
      if (LightIsGood()) {
         calib->SetZ(Z);
         calib->SetA(A);
         Double_t eloss = calib->Compute(lum);
         SetEnergy(eloss);
         return eloss;
      }
   }
   return -1.;
}

//__________________________________________________________________________________________//

Double_t KVCsI::GetLightFromEnergy(Int_t Z, Int_t A, Double_t E)
{
   //Calculate calibrated light output given by a nucleus (Z,A) deposing E (MeV)
   //in the detector. If E is not given, the current value of GetEnergy() is used.
   //Returns -1 in case of problems (no calibration available)

   KVLightEnergyCsI* calib = 0;

   if (Z == 1 && fCalZ1) calib = fCalZ1;
   else calib = fCal;

   if (calib && calib->GetStatus()) {
      E = (E < 0. ? GetEnergy() : E);
      calib->SetZ(Z);
      calib->SetA(A);
      Double_t lum = calib->Invert(E);

      return lum;
   }
   return -1.;
}

//__________________________________________________________________________________________//

Double_t KVCsI::GetEnergyFromLight(Int_t Z, Int_t A, Double_t lum)
{
   //Calculate energy from a given Z and A and light output (lum)
   //Returns -1 in case of problems (no calibration available)
   //This method assumes that the particle is stopped in CsI

   KVLightEnergyCsI* calib = 0;

   if (Z == 1 && fCalZ1) calib = fCalZ1;
   else calib = fCal;

   if (calib && calib->GetStatus()) {
      calib->SetZ(Z);
      calib->SetA(A);
      Double_t E = calib->Compute(lum);
      return E;
   }
   return -1.;
}

//__________________________________________________________________________________________//

void KVCsI::DeduceACQParameters(Int_t zz, Int_t aa)
{

   GetACQParam("R")->SetData(-1);
   GetACQParam("L")->SetData(-1);
   GetACQParam("T")->SetData(-1);
   if (zz == -1 || aa == -1) {
      return;
   }

   UShort_t Mt = 110;
   Double_t Xlen = 0;
   Double_t Yrap = 0;
   Double_t lumiere = GetLightFromEnergy(zz, aa);
   //Info("DeduceACQParameters", "z=%d a=%d e=%f h=%f",zz,aa,GetEnergy(),lumiere);

   KVIDTelescope* idcsi = (KVIDTelescope*)GetIDTelescopes()->At(0);

   KVIDZAGrid* idgcsi = (KVIDZAGrid*)idcsi->GetIDGrid();
   if (!idgcsi) {
      //Warning("DeduceACQParameters","%s, No grid available",GetName());
      return;
   }
   KVIDZALine* idline = (KVIDZALine*)idgcsi->GetIdentifier(zz, aa);
   if (!idline && zz <= idgcsi->GetZmax()) {
      // Z within limits of grid, but we don't have the isotope
      // Look for line with closest mass
      Int_t closest_a = 1000;
      Int_t closest_index = -1;
      Int_t nids = idgcsi->GetNumberOfIdentifiers();
      for (Int_t iid = 0; iid < nids; iid++) {
         KVIDZALine* ll = (KVIDZALine*)idgcsi->GetIdentifierAt(iid);
         if (ll->GetZ() == zz) {
            if (TMath::Abs(ll->GetA() - aa) < TMath::Abs(closest_a - aa)) {
               closest_index = iid;
               closest_a = ll->GetA();
            }
         }
      }
      idline = (KVIDZALine*)idgcsi->GetIdentifierAt(closest_index);
   }

   if (idline) {
      Double_t  Yrap1, Yrap2, Xlen1, Xlen2;
      idline->GetStartPoint(Xlen1, Yrap1);
      idline->GetEndPoint(Xlen2, Yrap2);
      Double_t  lumcalc1 = Calculate(kLumiere, Yrap1, Xlen1);
      Double_t  lumcalc2 = Calculate(kLumiere, Yrap2, Xlen2);
      if (lumiere < lumcalc1) {
         Xlen2 = Xlen1;
         Yrap2 = Yrap1;
         lumcalc2 = lumcalc1;
         Xlen1 = GetPedestal("L");
         Yrap1 = idline->Eval(Xlen1, 0, "S");
         lumcalc1 = Calculate(kLumiere, Yrap1, Xlen1);
         //cout << "Extrapolating before start of ID line" << endl;
      } else if (lumiere > lumcalc2) {
         Xlen1 = Xlen2;
         Yrap1 = Yrap2;
         lumcalc1 = lumcalc2;
         Xlen2 = 4095;
         Yrap2 = idline->Eval(Xlen2, 0, "S");
         lumcalc2 = Calculate(kLumiere, Yrap2, Xlen2);
         //cout << "Extrapolating after end of ID line" << endl;
      }
      //cout << "Xlen1=" << Xlen1 << " Yrap1="<<Yrap1<< "  Lum_min = " << lumcalc1 << endl;
      //cout << "Xlen2=" << Xlen2 << " Yrap2=" << Yrap2<< "  Lum_max = " << lumcalc2 << endl;

      Xlen = (Xlen1 + Xlen2) / 2.;
      Yrap = idline->Eval(Xlen, 0, "S");
      Double_t  lumcalc = Calculate(kLumiere, Yrap, Xlen);
      //cout << "-1 : Rapide = " << Yrap << " Lente = " << Xlen << " lumcalc = " << lumcalc << endl;

      Int_t niter = 0;
      while (niter < 20 && TMath::Abs(lumcalc - lumiere) / lumiere > 0.01) {
         if (lumcalc > lumiere) {
            Xlen2 = Xlen;
         } else {
            Xlen1 = Xlen;
         }
         Xlen = (Xlen1 + Xlen2) / 2.;
         Yrap = idline->Eval(Xlen, 0, "S");
         lumcalc = Calculate(kLumiere, Yrap, Xlen);
         //cout << niter << " : Rapide = " << Yrap << " Lente = " << Xlen << " lumcalc = " << lumcalc << endl;
         niter++;

      }
//        TMarker *mrk = new TMarker(Xlen,Yrap,2);
//        mrk->SetMarkerSize(2);
//        mrk->SetMarkerColor(kRed);
//        if(idgcsi->IsDrawn()) idgcsi->IsDrawn()->cd();
//        else {new TCanvas; idgcsi->Draw();}
//        mrk->Draw();
   } else {
      KVIDCutLine* imf_line = (KVIDCutLine*)idgcsi->GetCut("IMF_line");
      if (!imf_line) {
         //Warning("DeduceACQParameters","%s, No IMF_line defined",GetName());
         return;
      } else {
         Double_t  Yrap1, Yrap2, Xlen1, Xlen2;
         imf_line->GetStartPoint(Xlen1, Yrap1);
         imf_line->GetEndPoint(Xlen2, Yrap2);
         Yrap1 += 10.; //au-dessus de la ligne fragment
         Yrap2 += 10.; //au-dessus de la ligne fragment
         Double_t  lumcalc1 = Calculate(kLumiere, Yrap1, Xlen1);
         Double_t  lumcalc2 = Calculate(kLumiere, Yrap2, Xlen2);
         if (lumiere < lumcalc1) {
            Xlen2 = Xlen1;
            Yrap2 = Yrap1;
            lumcalc2 = lumcalc1;
            Xlen1 = GetPedestal("L");
            Yrap1 = imf_line->Eval(Xlen1, 0, "S") + 10.;
            lumcalc1 = Calculate(kLumiere, Yrap1, Xlen1);
            //cout << "Extrapolating before start of IMF line" << endl;
         } else if (lumiere > lumcalc2) {
            Xlen1 = Xlen2;
            Yrap1 = Yrap2;
            lumcalc1 = lumcalc2;
            Xlen2 = 4095;
            Yrap2 = imf_line->Eval(Xlen2, 0, "S") + 10.;
            lumcalc2 = Calculate(kLumiere, Yrap2, Xlen2);
            //cout << "Extrapolating after end of IMF line" << endl;
         }
         //cout << "Xlen1=" << Xlen1 << " Yrap1="<<Yrap1<< "  Lum_min = " << lumcalc1 << endl;
         //cout << "Xlen2=" << Xlen2 << " Yrap2=" << Yrap2<< "  Lum_max = " << lumcalc2 << endl;

         Xlen = (Xlen1 + Xlen2) / 2.;
         Yrap = imf_line->Eval(Xlen) + 10.;//au-dessus de la ligne fragment
         Double_t  lumcalc = Calculate(kLumiere, Yrap, Xlen);
         //cout << "-1 : Rapide = " << Yrap << " Lente = " << Xlen << " lumcalc = " << lumcalc << endl;

         Int_t niter = 0;
         while (niter < 20 && TMath::Abs(lumcalc - lumiere) / lumiere > 0.01) {
            if (lumcalc > lumiere) {
               Xlen2 = Xlen;
            } else {
               Xlen1 = Xlen;
            }
            Xlen = (Xlen1 + Xlen2) / 2.;
            Yrap = imf_line->Eval(Xlen) + 10.;//au-dessus de la ligne fragment
            lumcalc = Calculate(kLumiere, Yrap, Xlen);
            //cout << niter << " : Rapide = " << Yrap << " Lente = " << Xlen << " lumcalc = " << lumcalc << endl;
            niter++;
         }
         if (niter == 20) {
            Xlen = -1;
            Yrap = -1;
            Mt = -1;
         }
//            TMarker *mrk = new TMarker(Xlen,Yrap,2);
//            mrk->SetMarkerSize(2);
//            mrk->SetMarkerColor(kBlue);
//            if(idgcsi->IsDrawn()) idgcsi->IsDrawn()->cd();
//            else {new TCanvas; idgcsi->Draw();}
//            mrk->Draw();
      }

   }
   GetACQParam("R")->SetData((UShort_t)Yrap);
   GetACQParam("L")->SetData((UShort_t)Xlen);
   GetACQParam("T")->SetData(Mt);

}
