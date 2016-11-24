//Created by KVClassFactory on Fri Jan 23 18:44:27 2015
//Author: ,,,

#include "KVFAZIADetector.h"
#include "KVFAZIACalibrator.h"
#include "KVIDTelescope.h"
#include "KVFAZIA.h"
#include "KVSignal.h"
#include "KVPSAResult.h"
#include "TClass.h"
#include "KVLightEnergyCsIFull.h"

ClassImp(KVFAZIADetector)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVFAZIADetector</h2>
<h4>Base class for FAZIA detector</h4>
<!-- */
// --> END_HTML
//
// DETECTOR NAMES
// ==============
// Detector names are assumed to be defined as
// label-xxx
// where xxx is computed as follows (see KVFAZIADetector::GetIndex) :
// 100*block number+10*quartet number+telescope number
// and label can be SI1, SI2 or CSI
// For example SI1-123 is the Silicon Si1 of the block 1, the quartet 2 and the telescope 3

////////////////////////////////////////////////////////////////////////////////

//________________________________________________________________
void KVFAZIADetector::init()
{
   //default initialisations
   fBlock = -1;
   fIdentifier = kOTHER;
   fQuartet = -1;
   fTelescope = -1;
   fIndex = -1;
   fIsRutherford = kFALSE;

   fSignals = 0;
   fChannelToEnergy = 0;
   fChannelToVolt = 0;
   fVoltToEnergy = 0;
   fLabel = -1;
   fChannel = 0;
   fVolt = 0;
}

//________________________________________________________________
KVFAZIADetector::KVFAZIADetector()
{
   // Default constructor
   init();
}

//________________________________________________________________
KVFAZIADetector::KVFAZIADetector(const Char_t* type, const Float_t thick) : KVDetector(type, thick)
{
   // Create detector of given material type and thickness (in centimetres)
   init();
}

//________________________________________________________________
KVFAZIADetector::~KVFAZIADetector()
{
   // Destructor
   delete fSignals;

}

//________________________________________________________________
KVList* KVFAZIADetector::PrepareIDTelescopeList()
{

   KVList* lsub = new KVList();
   lsub->SetOwner(kFALSE);

   KVList* ltel = GetAlignedIDTelescopes();
   TList* ldet = GetAlignedDetectors();
   KVNumberList nl;
   nl.SetMinMax(0, ltel->GetEntries() - 1);
   for (Int_t ii = 0; ii < ldet->GetEntries(); ii += 1) {
      KVDetector* det = (KVDetector*)ldet->At(ii);
      for (Int_t jj = 0; jj < ltel->GetEntries(); jj += 1) {
         KVIDTelescope* tel = (KVIDTelescope*)ltel->At(jj);
         if (tel->GetDetector(1) == det || (tel->GetSize() == 2 && tel->GetDetector(2) == det)) {
            if (nl.Contains(jj)) {
               lsub->Add(tel);
               nl.Remove(jj);
            }
         }
      }
   }
   return lsub;

}

//________________________________________________________________
void KVFAZIADetector::SortIDTelescopes()
{

   KVList* lsub = PrepareIDTelescopeList();
   KVList* ltel = GetAlignedIDTelescopes();
   Int_t nn = ltel->GetEntries();
   for (Int_t ii = 0; ii < nn; ii += 1)
      ltel->RemoveAt(0);
   for (Int_t ii = 0; ii < nn; ii += 1)
      ltel->Add(lsub->At(ii));
   delete lsub;

}

//________________________________________________________________
void KVFAZIADetector::SetCalibrators()
{
   //Set up calibrators for this detector. Call once name has been set.
   //test to check that there is not already defined calibrators
   //
   if (GetListOfCalibrators())
      return;

   TString sf = "";

   if (fIdentifier == kCSI) {
      fChannelToEnergy = new KVLightEnergyCsIFull(GetName(), "Channel-Energy", this, KVLightEnergyCsIFull::kExact);
   } else {
      fChannelToEnergy = new KVFAZIACalibrator(GetName(), "Channel-Energy");
      fChannelToEnergy->SetDetector(this);
      sf = gEnv->GetValue("FAZIADetector.Calib.Channel-Energy", "");
      if (sf == "") {
         Warning("SetCalibrators", "No formula defined for Calibration Channel-Energy");
      } else {
         ((KVFAZIACalibrator*)fChannelToEnergy)->SetFunction(sf.Data());
      }
   }

   fChannelToVolt = new KVFAZIACalibrator(GetName(), "Channel-Volt");
   fChannelToVolt->SetDetector(this);
   sf = gEnv->GetValue("FAZIADetector.Calib.Channel-Volt", "");
   if (sf == "") {
      Warning("SetCalibrators", "No formula defined for Calibration Channel-Volt");
   } else {
      ((KVFAZIACalibrator*)fChannelToVolt)->SetFunction(sf.Data());
   }

   fVoltToEnergy = new KVFAZIACalibrator(GetName(), "Volt-Energy");
   fVoltToEnergy->SetDetector(this);
   sf = gEnv->GetValue("FAZIADetector.Calib.Volt-Energy", "");
   if (sf == "") {
      Warning("SetCalibrators", "No formula defined for Calibration Volt-Energy");
   } else {
      ((KVFAZIACalibrator*)fVoltToEnergy)->SetFunction(sf.Data());
   }

   AddCalibrator(fChannelToEnergy);
   AddCalibrator(fChannelToVolt);
   AddCalibrator(fVoltToEnergy);

}

Bool_t KVFAZIADetector::IsCalibrated() const
{
   //Returns true if the detector has been calibrated
   //i.e. if
   //  -  it has Channel-Energy calibrator ready

   if (!GetCalibrator("Channel-Energy")) return kFALSE;
   else if (!GetCalibrator("Channel-Energy")->GetStatus()) return kFALSE;

   return kTRUE;
}

//________________________________________________________________
Double_t KVFAZIADetector::GetCalibratedEnergy()
{
   //Set up calibrators for this detector. Call once name has been set.

   if (fChannelToEnergy->GetStatus()) {
      return fChannelToEnergy->Compute(fChannel);
   }
   return 0;
}

//________________________________________________________________
Double_t KVFAZIADetector::GetEnergy()
{
   // Returns energy lost in active layer by particles.
   Double_t eloss = (GetActiveLayer() ? GetActiveLayer()->GetEnergyLoss() : KVMaterial::GetEnergyLoss());
   if (eloss <= 0) {
      Double_t ecal = GetCalibratedEnergy();
      if (ecal > 0) SetEnergy(ecal);
      return ecal;
   }
   return eloss;
}
//________________________________________________________________
Double_t KVFAZIADetector::GetCalibratedVolt()
{
   //Set up calibrators for this detector. Call once name has been set.

   if (fChannelToVolt->GetStatus()) {
      return fChannelToVolt->Compute(fChannel);
   }
   return 0;

}

//________________________________________________________________
void KVFAZIADetector::Copy(TObject& obj) const
{
   // This method copies the current state of 'this' object into 'obj'
   // You should add here any member variables, for example:
   //    (supposing a member variable KVFAZIADetector::fToto)
   //    CastedObj.fToto = fToto;
   // or
   //    CastedObj.SetToto( GetToto() );

   KVDetector::Copy(obj);
   //KVFAZIADetector& CastedObj = (KVFAZIADetector&)obj;
}

//________________________________________________________________
void  KVFAZIADetector::Clear(Option_t*)
{

   KVDetector::Clear("");
   if (fSignals) {
      fSignals->Execute("Set", "0");
   }

}

//________________________________________________________________
void KVFAZIADetector::SetName(const char* name)
{
   TNamed::SetName(name);
   SetProperties();
}

//________________________________________________________________
Bool_t KVFAZIADetector::SetProperties()
{
   // detector name are assumed to be defined as
   // label-xxx
   // where xxx is computed as follow :
   // 100*block number+10*quartet number+telescope number
   // and label can be SI1, SI2 or CSI
   // For example SI1-123 is the Silicon Si1 of the block 1, the quartet 2 and the telescope 3
   //
   //

   KVString tmp;
   KVString sname(GetName());

   sname.Begin("-");
   SetLabel(sname.Next());
   if (!strcmp(GetLabel(), "SI1")) fIdentifier = kSI1;
   else if (!strcmp(GetLabel(), "SI2")) fIdentifier = kSI2;
   else if (!strcmp(GetLabel(), "CSI")) fIdentifier = kCSI;

   gFazia->AddDetectorLabel(GetLabel());

   tmp = sname.Next();
   if (tmp == "RUTH") {
      fIsRutherford = kTRUE;
      fIndex = fTelescope = fQuartet = fBlock = 0;
   } else if (tmp.IsDigit()) {
      fIndex = tmp.Atoi();
      fBlock = fIndex / 100;
      fQuartet = (fIndex - fBlock * 100) / 10;
      fTelescope = fIndex - fBlock * 100 - fQuartet * 10;
   } else {
      Info("SetProperties", "Unkown format for the detector %s", GetName());
   }

   KVSignal* sig = 0;
   //"QH1", "I1", "QL1", "Q2", "I2", "Q3
   if (fSignals)
      delete fSignals;
   fSignals = new KVList(kTRUE);
   KVString lsignals = "";
   if (!strcmp(GetLabel(), "SI1")) {
      lsignals = "QH1,I1,QL1";
   } else if (!strcmp(GetLabel(), "SI2")) {
      lsignals = "Q2,I2";
   } else if (!strcmp(GetLabel(), "CSI")) {
      lsignals = "Q3";
   } else {
      Warning("SetProperties", "Unknown label \"%s\" for this detector : %s\n", GetLabel(), GetName());
      lsignals = "";
   }
   TClass* cl = 0;
   lsignals.Begin(",");
   while (!lsignals.End()) {

      KVString ssig = lsignals.Next();
      cl = TClass::GetClass(Form("KV%s", ssig.Data()));
      sig = (KVSignal*)cl->New();
      sig->SetName(ssig.Data());
      sig->SetType(ssig.Data());

      sig->LoadPSAParameters();
      sig->SetDetectorName(GetName());

      fSignals->Add(sig);
   }

   SetCalibrators();

   return kTRUE;
}
//________________________________________________________________
const Char_t* KVFAZIADetector::GetNewName(KVString oldname)
{
   // Translate an old-style FAZIA detector name (e.g. "SI1-T1-Q2-B001")
   // to the new format ("SI1-121")

   Int_t tt = 0, qq = 0, bb = 0;
   KVString tmp = "";
   KVString lab = "";
   oldname.Begin("-");

   if (!oldname.End()) {
      lab = oldname.Next();
   }
   if (!oldname.End()) {
      tmp = oldname.Next();
      tmp.ReplaceAll("T", "");
      tt = tmp.Atoi();
   }
   if (!oldname.End()) {
      tmp = oldname.Next();
      tmp.ReplaceAll("Q", "");
      qq = tmp.Atoi();
   }
   if (!oldname.End()) {
      tmp = oldname.Next();
      tmp.ReplaceAll("B", "");
      bb = tmp.Atoi();
   }
   static KVString newname;
   newname.Form("%s-%d", lab.Data(), bb * 100 + qq * 10 + tt);
   return newname.Data();

}

//________________________________________________________________
Bool_t KVFAZIADetector::Fired(Option_t*)
{
   // Returns kTRUE if detector was hit (fired) in an event
   //
   // The test is made on charge signals of the detectors
   // if one of them return kTRUE to KVSignal::IsFired() method KVDetector::Fired() return kTRUE
   // if not return kFALSE and the detector will not be considered in following analysis
   // except if one detector after it has been fired
   //
   // If the detector is in "simulation mode", i.e. if SetSimMode(kTRUE) has been called,
   // this method returns kTRUE if the calculated energy loss in the active layer is > 0.
   //

   if (!IsDetecting()) return kFALSE; //detector not working, no answer at all
   if (IsSimMode()) return (GetActiveLayer()->GetEnergyLoss() > 0.); // simulation mode: detector fired if energy lost in active layer
   KVSignal* sig;
   if (fSignals) {
      TIter next(fSignals);
      while ((sig = (KVSignal*)next())) {
         if (sig->GetN() > 0) {
            if (sig->IsCharge()) {

               //pre process to use the test method KVSignal::IsFired()
               sig->ComputeEndLine();
               sig->TreateSignal();

               if (sig->IsFired()) {
                  return kTRUE;
               } else {

               }
            }
         } else {
            //Warning("Fired","%s has empty signal %s",GetName(),sig->GetName());
         }
      }
   } else {
      Warning("Fired", "%s : No signal attached to this detector ...", GetName());
   }

   return kFALSE;
}

//_________________________________________________________________________________
void KVFAZIADetector::SetSignal(KVSignal* signal, const Char_t* type)
{
   if (!fSignals) {
      Error("SetSignal", "%s List of signals not defined", GetName());
      return;
   }
   KVSignal* sig = GetSignal(type);
   if (sig) {
      sig->SetData(signal->GetN(), signal->GetX(), signal->GetY());
   } else {
      Warning("SetSignal", "%s : No signal of type #%s# is available", GetName(), type);
   }
}

//_________________________________________________________________________________
Bool_t KVFAZIADetector::HasSignal() const
{
   // Returns kTRUE if detector has at least 1 associated signal
   return (fSignals && fSignals->GetEntries() > 0);
}

//_________________________________________________________________________________
KVSignal* KVFAZIADetector::GetSignal(const Char_t* name) const
{
   // Access detector signal by name, i.e. as in FAZIA raw data
   // e.g. "T1-Q3-B001-QL1"
   if (fSignals)
      return (KVSignal*)fSignals->FindObject(name);
   return nullptr;
}

//_________________________________________________________________________________
KVSignal* KVFAZIADetector::GetSignalByType(const Char_t* type) const
{
   // Access detector signal of given type: "I1", "I2", "Q2", "Q3", "QH1", "QL1"
   if (fSignals)
      return (KVSignal*)fSignals->FindObjectWithMethod(type, "GetType");
   return nullptr;
}

//_________________________________________________________________________________
KVSignal* KVFAZIADetector::GetSignal(Int_t idx) const
{
   // Access signal with given index in list of detector's signals
   // 0 <= idx < KVFAZIADetector::GetNumberOfSignals()
   if (fSignals && 0 <= idx && idx < fSignals->GetEntries())
      return (KVSignal*)fSignals->At(idx);
   return 0;
}

//_________________________________________________________________________________
Int_t KVFAZIADetector::GetNumberOfSignals() const
{
   if (fSignals)
      return fSignals->GetEntries();
   return 0;
}

//_________________________________________________________________________________
KVList* KVFAZIADetector::GetListOfSignals() const
{
   return fSignals;
}

//_________________________________________________________________________________
void KVFAZIADetector::ComputePSA()
{
   // Perform Pulse Shape Analysis on all signals
   KVSignal* sig = 0;
   TIter nexts(GetListOfSignals());
   while ((sig = (KVSignal*)nexts())) {
      sig->TreateSignal();
   }
}
