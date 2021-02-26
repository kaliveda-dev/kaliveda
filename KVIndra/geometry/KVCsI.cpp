#include "KVCsI.h"

#include <KVCalibratedSignal.h>

using namespace std;

ClassImp(KVCsI)
//_______________________________________________________________________________________

void KVCsI::init()
{
   //initialise non-persistent pointers
   fSegment = 2;
   fPinLaser = 0;
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
      SetActiveLayer((KVMaterial*)fAbsorbers->FindObject("CsI"));
   }
   SetType("CSI");
   SetLabel("CSI");//for use with KVReconNucTrajectory
   init();
}

void KVCsI::SetAlThickness(Float_t thickAl /* um */)
{
   KVMaterial* mat = 0;
   if (!(mat = GetAbsorber("Al"))) {
      mat = new KVMaterial("Al", thickAl * KVUnits::um);
      fAbsorbers->AddFirst(mat);
      SetActiveLayer((KVMaterial*)fAbsorbers->FindObject("CsI"));
   }
   else mat->SetThickness(thickAl * KVUnits::um);
}

//____________________________________________________________________________________________

KVCsI::~KVCsI()
{
}

void KVCsI::Print(Option_t* option) const
{
   //Print info on this detector - redefinition of KVDetector::Print for option="data" to include total light
   //if option="data" the energy loss, total light and coder channel data are displayed

   if (!strcmp(option, "data")) {
      cout << ((KVCsI*) this)->GetName() << " -- H=" << ((KVCsI*) this)->
           GetDetectorSignalValue("TotLight") << " -- E=" << ((KVCsI*) this)->
           GetEnergy() << "  ";

      if (BelongsToUnidentifiedParticle())
         cout << "(Belongs to an unidentified particle)";
      cout << endl;
   }
   else {
      KVDetector::Print(option);
   }
}

//void KVCsI::SetACQParams()
//{
//   // Set acquisition parameters for this CsI.
//   // Do not call before detector's name has been set.
//   // Initialises member pointers fACQ_R & fACQ_L for (fast) direct access.

//   AddACQParamType("R");
//   AddACQParamType("L");
//   AddACQParamType("T");
//}

//______________________________________________________________________________

Double_t KVCsI::GetCorrectedEnergy(KVNucleus* nuc, Double_t lum, Bool_t)
{
   // Calculate calibrated energy loss for a nucleus (Z,A) giving total light output "lum".
   // By default we use the current value of the detector's `TotLight` signal.

   if (!(GetDetectorSignalValue("TotLight") > 0)) return -1;

   Int_t Z = nuc->GetZ();
   Int_t A = nuc->GetA();

   Double_t eloss;
   if (lum > -1) eloss = GetDetectorSignalValue("Energy", Form("INPUT=%g,Z=%d,A=%d", lum, Z, A));
   else eloss = GetDetectorSignalValue("Energy", Form("Z=%d,A=%d", Z, A));
   if (((KVCalibratedSignal*)GetDetectorSignal("Energy"))->InversionFailure()) {
      Info("GetCorrectedEnergy", "Failed to invert Light-Energy calibration for %s with Z=%d A=%d LIGHT=%f",
           GetName(), Z, A, GetDetectorSignalValue("TotLight"));
      return -1;
   }

   if (GetDetectorSignal("TotLight")->GetStatus("LightIsGood")) {
      SetEnergy(eloss);
      return eloss;
   }
   return -1.;
}

void KVCsI::DeduceACQParameters(KVEvent*, KVNumberList&)
{
   AbstractMethod("DeduceACQParameters");

   // Deduce fast & slow acquisition parameters for particles hitting CsI in given
   // (simulated) event. The KVNumberList contains the indices of particles in the event.
//   GetACQParam("R")->SetData(-1);
//   GetACQParam("L")->SetData(-1);
//   GetACQParam("T")->SetData(-1);

//   UShort_t Mt = 110;
//   Double_t Xlen_sum = 0;
//   Double_t Yrap_sum = 0;

//   KVIDTelescope* idcsi = (KVIDTelescope*)GetIDTelescopes()->At(0);

//   KVIDZAGrid* idgcsi = (KVIDZAGrid*)idcsi->GetIDGrid();
//   if (!idgcsi) {
//      //Warning("DeduceACQParameters","%s, No grid available",GetName());
//      return;
//   }

//   index.Begin();
//   while (!index.End()) { // loop over all particles hitting CsI
//      KVNucleus* nunuc = e->GetParticle(index.Next());
//      Int_t zz = nunuc->GetZ();
//      Int_t aa = nunuc->GetA();
//      // calculate total light output corresponding to this particle and its individual energy loss
//      Double_t lumiere = GetLightFromEnergy(zz, aa, nunuc->GetParameters()->GetDoubleValue(GetName()));

////      Info("DAQ","Z=%d A=%d E=%f H=%f status=%d",zz,aa,nunuc->GetParameters()->GetDoubleValue(GetName()),lumiere,
////           GetStatusLumiere());

//      KVIDZALine* idline = (KVIDZALine*)idgcsi->GetIdentifier(zz, aa);
//      if (!idline && zz <= idgcsi->GetZmax()) {
//         // Z within limits of grid, but we don't have the isotope
//         // Look for line with closest mass
//         Int_t closest_a = 1000;
//         Int_t closest_index = -1;
//         Int_t nids = idgcsi->GetNumberOfIdentifiers();
//         for (Int_t iid = 0; iid < nids; iid++) {
//            KVIDZALine* ll = (KVIDZALine*)idgcsi->GetIdentifierAt(iid);
//            if (ll->GetZ() == zz) {
//               if (TMath::Abs(ll->GetA() - aa) < TMath::Abs(closest_a - aa)) {
//                  closest_index = iid;
//                  closest_a = ll->GetA();
//               }
//            }
//         }
//         idline = (KVIDZALine*)idgcsi->GetIdentifierAt(closest_index);
//      }
//      Double_t Xlen = 0;
//      Double_t Yrap = 0;

//      if (idline) {
//         Double_t  Yrap1, Yrap2, Xlen1, Xlen2;
//         idline->GetStartPoint(Xlen1, Yrap1);
//         idline->GetEndPoint(Xlen2, Yrap2);

//         Double_t  lumcalc1 = Calculate(kLumiere, Yrap1, Xlen1);
//         Double_t  lumcalc2 = Calculate(kLumiere, Yrap2, Xlen2);
////         Info("DAQ","minimum H=%f E=%f",lumcalc1,GetEnergyFromLight(zz,aa,lumcalc1));
////         Info("DAQ","maximum H=%f E=%f",lumcalc2,GetEnergyFromLight(zz,aa,lumcalc2));
//         if (lumiere < lumcalc1) {
//            Xlen2 = Xlen1;
//            Yrap2 = Yrap1;
//            lumcalc2 = lumcalc1;
//            Xlen1 = GetPedestal("L");
//            Yrap1 = idline->Eval(Xlen1, 0, "S");
//            lumcalc1 = Calculate(kLumiere, Yrap1, Xlen1);
//            //cout << "Extrapolating before start of ID line" << endl;
//         }
//         else if (lumiere > lumcalc2) {
//            Xlen1 = Xlen2;
//            Yrap1 = Yrap2;
//            lumcalc1 = lumcalc2;
//            Xlen2 = 4095;
//            Yrap2 = idline->Eval(Xlen2, 0, "S");
//            lumcalc2 = Calculate(kLumiere, Yrap2, Xlen2);
//            //cout << "Extrapolating after end of ID line" << endl;
//         }
//         //cout << "Xlen1=" << Xlen1 << " Yrap1="<<Yrap1<< "  Lum_min = " << lumcalc1 << endl;
//         //cout << "Xlen2=" << Xlen2 << " Yrap2=" << Yrap2<< "  Lum_max = " << lumcalc2 << endl;

//         Xlen = (Xlen1 + Xlen2) / 2.;
//         Yrap = idline->Eval(Xlen, 0, "S");
//         Double_t  lumcalc = Calculate(kLumiere, Yrap, Xlen);
//         //cout << "-1 : Rapide = " << Yrap << " Lente = " << Xlen << " lumcalc = " << lumcalc << endl;

//         Int_t niter = 0;
//         while (niter < 20 && TMath::Abs(lumcalc - lumiere) / lumiere > 0.01) {
//            if (lumcalc > lumiere) {
//               Xlen2 = Xlen;
//            }
//            else {
//               Xlen1 = Xlen;
//            }
//            Xlen = (Xlen1 + Xlen2) / 2.;
//            Yrap = idline->Eval(Xlen, 0, "S");
//            lumcalc = Calculate(kLumiere, Yrap, Xlen);
//            //cout << niter << " : Rapide = " << Yrap << " Lente = " << Xlen << " lumcalc = " << lumcalc << endl;
//            niter++;

//         }
//         //        TMarker *mrk = new TMarker(Xlen,Yrap,2);
//         //        mrk->SetMarkerSize(2);
//         //        mrk->SetMarkerColor(kRed);
//         //        if(idgcsi->IsDrawn()) idgcsi->IsDrawn()->cd();
//         //        else {new TCanvas; idgcsi->Draw();}
//         //        mrk->Draw();
//      }
//      else {
//         KVIDCutLine* imf_line = (KVIDCutLine*)idgcsi->GetCut("IMF_line");
//         if (!imf_line) {
//            //Warning("DeduceACQParameters","%s, No IMF_line defined",GetName());
//            return;
//         }
//         else {
//            Double_t  Yrap1, Yrap2, Xlen1, Xlen2;
//            imf_line->GetStartPoint(Xlen1, Yrap1);
//            imf_line->GetEndPoint(Xlen2, Yrap2);
//            Yrap1 += 10.; //au-dessus de la ligne fragment
//            Yrap2 += 10.; //au-dessus de la ligne fragment
//            Double_t  lumcalc1 = Calculate(kLumiere, Yrap1, Xlen1);
//            Double_t  lumcalc2 = Calculate(kLumiere, Yrap2, Xlen2);
//            if (lumiere < lumcalc1) {
//               Xlen2 = Xlen1;
//               Yrap2 = Yrap1;
//               lumcalc2 = lumcalc1;
//               Xlen1 = GetPedestal("L");
//               Yrap1 = imf_line->Eval(Xlen1, 0, "S") + 10.;
//               lumcalc1 = Calculate(kLumiere, Yrap1, Xlen1);
//               //cout << "Extrapolating before start of IMF line" << endl;
//            }
//            else if (lumiere > lumcalc2) {
//               Xlen1 = Xlen2;
//               Yrap1 = Yrap2;
//               lumcalc1 = lumcalc2;
//               Xlen2 = 4095;
//               Yrap2 = imf_line->Eval(Xlen2, 0, "S") + 10.;
//               lumcalc2 = Calculate(kLumiere, Yrap2, Xlen2);
//               //cout << "Extrapolating after end of IMF line" << endl;
//            }
//            //cout << "Xlen1=" << Xlen1 << " Yrap1="<<Yrap1<< "  Lum_min = " << lumcalc1 << endl;
//            //cout << "Xlen2=" << Xlen2 << " Yrap2=" << Yrap2<< "  Lum_max = " << lumcalc2 << endl;

//            Xlen = (Xlen1 + Xlen2) / 2.;
//            Yrap = imf_line->Eval(Xlen) + 10.;//au-dessus de la ligne fragment
//            Double_t  lumcalc = Calculate(kLumiere, Yrap, Xlen);
//            //cout << "-1 : Rapide = " << Yrap << " Lente = " << Xlen << " lumcalc = " << lumcalc << endl;

//            Int_t niter = 0;
//            while (niter < 20 && TMath::Abs(lumcalc - lumiere) / lumiere > 0.01) {
//               if (lumcalc > lumiere) {
//                  Xlen2 = Xlen;
//               }
//               else {
//                  Xlen1 = Xlen;
//               }
//               Xlen = (Xlen1 + Xlen2) / 2.;
//               Yrap = imf_line->Eval(Xlen) + 10.;//au-dessus de la ligne fragment
//               lumcalc = Calculate(kLumiere, Yrap, Xlen);
//               //cout << niter << " : Rapide = " << Yrap << " Lente = " << Xlen << " lumcalc = " << lumcalc << endl;
//               niter++;
//            }
//            if (niter == 20) {
//               Xlen = -1;
//               Yrap = -1;
//               Mt = -1;
//            }
//            //            TMarker *mrk = new TMarker(Xlen,Yrap,2);
//            //            mrk->SetMarkerSize(2);
//            //            mrk->SetMarkerColor(kBlue);
//            //            if(idgcsi->IsDrawn()) idgcsi->IsDrawn()->cd();
//            //            else {new TCanvas; idgcsi->Draw();}
//            //            mrk->Draw();
//         }

//      }
//      if (Xlen > 0) Xlen_sum += Xlen;
//      if (Yrap > 0) Yrap_sum += Yrap;
//   }
//   GetACQParam("R")->SetData((UShort_t)Yrap_sum);
//   GetACQParam("L")->SetData((UShort_t)Xlen_sum);
//   GetACQParam("T")->SetData(Mt);

}
