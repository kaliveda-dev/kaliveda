//Created by KVClassFactory on Fri Jan 23 18:44:27 2015
//Author: ,,,

#include "KVFAZIADetector.h"
#include "KVIDTelescope.h"
#include "KVFAZIA.h"
#include "KVSignal.h"
#include "TClass.h"
#include "KVDataSet.h"

ClassImp(KVFAZIADetector)


//________________________________________________________________
void KVFAZIADetector::init()
{
   //default initialisations
   fSignals.SetOwner();

   fBlock = -1;
   fIdentifier = kOTHER;
   fQuartet = -1;
   fTelescope = -1;
   fIndex = -1;
   fIsRutherford = kFALSE;
   fLabel = -1;
}

//________________________________________________________________
Double_t KVFAZIADetector::GetSetupParameter(const Char_t* parname)
{

   Double_t lval = -1;
   if (gDataSet)  lval = gDataSet->GetDataSetEnv(parname, 0.0);
   else           lval = gEnv->GetValue(parname, 0.0);
   return lval;

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
void  KVFAZIADetector::Clear(Option_t* opt)
{
   //If opt="N" we do not reset any raw data, signals, etc

   KVDetector::Clear(opt);
   if (strncmp(opt, "N", 1)) fSignals.Execute("Set", "0");
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

   // read thresholds to be applied on FPGA energies to decide if detector is fired
   fQH1Threshold = GetSetupParameter("QH1.MinimumAmplitude");
   fQ2Threshold = GetSetupParameter("Q2.MinimumAmplitude");
   fQ3Threshold = GetSetupParameter("Q3.MinimumAmplitude");

   // if f[DET]FiredFromSignals = true, use the signal to decide if detector is fired
   // instead of FPGA energies (old fashion)
   fIsFiredFromSignals = GetSetupParameter(Form("%s.IsFiredFromSignal", GetLabel())) > 0.5;

   tmp = sname.Next();
   if (tmp == "RUTH") {
      fIsRutherford = kTRUE;
      fIndex = fTelescope = fQuartet = fBlock = 0;
   }
   else if (tmp.IsDigit()) {
      fIndex = tmp.Atoi();
      fBlock = fIndex / 100;
      fQuartet = (fIndex - fBlock * 100) / 10;
      fTelescope = fIndex - fBlock * 100 - fQuartet * 10;
   }
   else {
      Info("SetProperties", "Unkown format for the detector %s", GetName());
   }

   KVSignal* sig = 0;
   //"QH1", "I1", "QL1", "Q2", "I2", "Q3
   fSignals.Clear();
   KVString lsignals = "";
   KVString dsigs = "DetTag,GTTag";
   if (!strcmp(GetLabel(), "SI1")) {
      lsignals = "QH1,I1,QL1";
   }
   else if (!strcmp(GetLabel(), "SI2")) {
      lsignals = "Q2,I2";
   }
   else if (!strcmp(GetLabel(), "CSI")) {
      lsignals = "Q3";
      dsigs += ",Q3.FastAmplitude,Q3.FastFPGAEnergy";
   }
   else {
      Warning("SetProperties", "Unknown label \"%s\" for this detector : %s\n", GetLabel(), GetName());
      lsignals = "";
      dsigs = "";
   }
   KVString sigtypes = "Amplitude,RawAmplitude,FPGAEnergy,RiseTime,BaseLine,SigmaBaseLine";
   lsignals.Begin(",");
   while (!lsignals.End()) {
      sigtypes.Begin(",");
      KVString sig = lsignals.Next();
      while (!sigtypes.End()) {
         KVString stype = sigtypes.Next();
         if (sig.BeginsWith("I") && (stype == "FPGAEnergy" || stype == "RiseTime")) continue;
         if (sig == "QL1" && stype == "FPGAEnergy") continue;
         dsigs += Form(",%s.%s", sig.Data(), stype.Data());
      }
   }
   dsigs.Begin(",");
   while (!dsigs.End()) AddDetectorSignal(new KVDetectorSignal(dsigs.Next(), this));
   if (lsignals == "Q3") AddDetectorSignalExpression("Q3.SlowAmplitude", "Q3.Amplitude - 0.8*Q3.FastAmplitude");

   lsignals.Begin(",");
   while (!lsignals.End()) {

      KVString ssig = lsignals.Next();
      sig = KVSignal::MakeSignal(ssig);

      sig->SetName(Form("%s-%s", ssig.Data(), tmp.Data()));
      sig->SetType(ssig.Data());

      sig->LoadPSAParameters();
      sig->SetDetectorName(GetName());

      fSignals.Add(sig);
   }

   //SetCalibrators();

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
Bool_t KVFAZIADetector::Fired(Option_t*) const
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

   if (!fIsFiredFromSignals) {
      switch (GetIdentifier()) {
         case kSI1:
            if (GetDetectorSignalValue("QH1.FPGAEnergy") > fQH1Threshold) return kTRUE;
            else return kFALSE;
            break;
         case kSI2:
            if (GetDetectorSignalValue("Q2.FPGAEnergy") > fQ2Threshold) return kTRUE;
            else return kFALSE;
            break;
         case kCSI:
            if (GetDetectorSignalValue("Q3.FPGAEnergy") > fQ3Threshold) return kTRUE;
            else return kFALSE;
            break;
         default:
            return kFALSE;
            break;
      }
   }

//   hereafter : old way of doing...
   KVSignal* sig;
   if (fSignals.GetSize()) {
      TIter next(&fSignals);
      while ((sig = (KVSignal*)next())) {
         if (sig->IsOK()) {
            if (sig->IsCharge()) {
               //pre process to use the test method KVSignal::IsFired()
               if (!sig->PSAHasBeenComputed()) {
                  sig->ComputeEndLine();
                  sig->TreateSignal();
               }
               if (sig->IsFired()) {
                  return kTRUE;
               }
               else {
               }
            }
         }
         else {
            //Warning("Fired", "%s has empty signal %s", GetName(), sig->GetName());
         }
      }
   }
   else {
      Warning("Fired", "%s : No signal attached to this detector ...", GetName());
   }
   return kFALSE;
}

//_________________________________________________________________________________
void KVFAZIADetector::SetSignal(TGraph* signal, const Char_t* signal_name)
{
   // Copy waveform data from TGraph into the signal with the given name (QH1-345 etc.)
   // Then calculate Ymin/ymax, ADC data, shit, blabla

   KVSignal* sig = GetSignal(signal_name);
   if (sig) {
      sig->SetData(signal->GetN(), signal->GetX(), signal->GetY());
   }
   else {
      Warning("SetSignal", "%s : No signal of name #%s# is available", GetName(), signal_name);
   }
}

//_________________________________________________________________________________
Bool_t KVFAZIADetector::HasSignal() const
{
   // Returns kTRUE if detector has at least 1 associated signal
   return (fSignals.GetEntries() > 0);
}

//_________________________________________________________________________________
KVSignal* KVFAZIADetector::GetSignal(const Char_t* name) const
{
   // Access detector signal by name, i.e. as in FAZIA raw data
   // e.g. "QL1-231"
   return (KVSignal*)fSignals.FindObject(name);
}

//_________________________________________________________________________________
KVSignal* KVFAZIADetector::GetSignalByType(const Char_t* type) const
{
   // Access detector signal of given type: "I1", "I2", "Q2", "Q3", "QH1", "QL1"
   return (KVSignal*)fSignals.FindObjectByType(type);
}

//_________________________________________________________________________________
KVSignal* KVFAZIADetector::GetSignal(Int_t idx) const
{
   // Access signal with given index in list of detector's signals
   // 0 <= idx < KVFAZIADetector::GetNumberOfSignals()
   if (0 <= idx && idx < fSignals.GetEntries())
      return (KVSignal*)fSignals.At(idx);
   return nullptr;
}

//_________________________________________________________________________________
Int_t KVFAZIADetector::GetNumberOfSignals() const
{
   return fSignals.GetEntries();
}

//_________________________________________________________________________________
const KVSeqCollection* KVFAZIADetector::GetListOfSignals() const
{
   return &fSignals;
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

KVDetectorSignal* KVFAZIADetector::SetFPGAEnergy(int sigid, Int_t idx, Double_t energy)
{
   // Set FPGA energy value in appropriate KVDetectorSignal of detector and set its state
   // to 'fired'. Returns address of the signal which was set.

   switch (sigid) {
      case KVSignal::kQH1:
         if (idx == 0) return SetQH1FPGAEnergy(energy);
         break;
      case KVSignal::kI1:
         break;
      case KVSignal::kQL1:
         break;
      case KVSignal::kQ2:
         if (idx == 0) return SetQ2FPGAEnergy(energy);
         break;
      case KVSignal::kI2:
         break;
      case KVSignal::kQ3:
         if (idx == 0) return SetQ3FPGAEnergy(energy);
         if (idx == 1) return SetQ3FastFPGAEnergy(energy);
         break;
   }
   return nullptr;
}
