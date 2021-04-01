//Created by KVClassFactory on Wed Jul 25 09:43:37 2012
//Author: Guilain ADEMARD

#include "Riostream.h"
#include "KVVAMOSDetector.h"
#include "KVNamedParameter.h"
using namespace std;

ClassImp(KVVAMOSDetector)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
   <h2>KVVAMOSDetector</h2>
   <h4>Detectors of VAMOS spectrometer</h4>
   <!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVString KVVAMOSDetector::fACQParamTypes("0:E, 1:Q, 2:T_HF, 3:T, 9:NO_TYPE");
KVString KVVAMOSDetector::fPositionTypes("0:X, 1:Y, 2:Z, 3:XY, 4:XZ, 5:YZ, 6:XYZ, 9:NO_TYPE");


KVVAMOSDetector::KVVAMOSDetector()
{
   // Default constructor
   init();
}
//________________________________________________________________

KVVAMOSDetector::KVVAMOSDetector(UInt_t number, const Char_t* type) : KVSpectroDetector(type)
{
   // create a new VAMOS detector of a given material.

   init();
   SetNumber(number);
}
//________________________________________________________________

KVVAMOSDetector::KVVAMOSDetector(const KVVAMOSDetector& obj)  : KVSpectroDetector()
{
   // Copy constructor
   // This ctor is used to make a copy of an existing object (for example
   // when a method returns an object), and it is always a good idea to
   // implement it.
   // If your class allocates memory in its constructor(s) then it is ESSENTIAL :-)

   init();
   obj.Copy(*this);
}
//________________________________________________________________

KVVAMOSDetector::~KVVAMOSDetector()
{
   // Destructor
   SafeDelete(fTlist);
   SafeDelete(fT0list);
}
//________________________________________________________________

void KVVAMOSDetector::Copy(TObject& obj) const
{
   // This method copies the current state of 'this' object into 'obj'
   // You should add here any member variables, for example:
   //    (supposing a member variable KVVAMOSDetector::fToto)
   //    CastedObj.fToto = fToto;
   // or
   //    CastedObj.SetToto( GetToto() );

   KVSpectroDetector::Copy(obj);
   //KVVAMOSDetector& CastedObj = (KVVAMOSDetector&)obj;
}
//________________________________________________________________

void KVVAMOSDetector::init()
{
   fTlist     = NULL;
   fT0list    = NULL;
   fNmeasX = fNmeasY = 1; // from its volum a detector can be used
   // to measured at least one position X and Y
   SetKVDetectorFiredACQParameterListFormatString();
}
//________________________________________________________________

Bool_t KVVAMOSDetector::Fired(Option_t* opt, Option_t* optP)
{
   //Returns kTRUE if detector was hit (fired) in an event
   //
   //The actual meaning of hit/fired depends on the context and the option string opt.
   //
   //opt="any" (default) and optP="":
   //Returns true if at least one working acquisition parameter
   //associated with the detector was fired in an event, for ANY of the types
   //in the list*.
   //
   //opt="all" and optP="" :
   //Returns true if at least one working acquisition parameter
   //associated with the detector was fired in an event, for ALL of the types
   //in the list*.
   //
   //opt="any" and optP="P" :
   //Returns true if at least one working acquisition parameter
   //associated with the detector was fired in an event and have a value
   //greater than their pedestal value, for ANY of the types in the list*.
   //
   //opt="all" and optP="P":
   //Returns true if at least one working acquisition parameter
   //associated with the detector was fired in an event and have a value
   //greater than their pedestal value, for ALL of the types in the list*.
   //
   // *the actual parameters taken into account can be fine tuned using environment variables such as
   //          KVVAMOSDetector.Fired.ACQParameterList.[type]: Q,E,T,T_HF,X,Y
   // See KVAMOSDetector::SetFiredBitmask() for more details.


   if (!IsDetecting()) return kFALSE; //detector not working, no answer at all

   Bool_t opt_all = !strcmp(opt, "all");
   Binary8_t event; // bitmask for event

   // Look at the three first bits for XYZ positions
   UChar_t xyz_mask = fFiredMask.Subvalue(2, 3);
// Info("Fired","Option %s, FiredBitmask %s, xyz_mask (%d)",opt,fFiredMask.String(), xyz_mask);
   if (xyz_mask) {
      Double_t xyz[3];
      UChar_t  xyz_res = GetRawPosition(xyz);
//    cout<<Form(" xyz_res (%d)",xyz_res)<<endl;
      if (opt_all && (xyz_mask != xyz_res)) return kFALSE;
      event.Set(xyz_res);
   }

   // Look at the other bits for  ACQ Parameters
   UChar_t Nbits = fFiredMask.GetNBits();
   Binary8_t keep_up(fFiredMask.Subvalue(Nbits - 1, Nbits - 3));
// Info("Fired","keep_up %s",keep_up.String());
   UChar_t id;
   TIter next(GetACQParamList());
   TIter next_t(GetTACQParamList());
   KVEBYEDAT_ACQParam* par;
   while (keep_up.Value() && ((par = (KVEBYEDAT_ACQParam*)next()) || (par = (KVEBYEDAT_ACQParam*)next_t()))) {
      if (par->IsWorking() && par->Fired(optP)) {
         id = GetACQParamTypeIdxFromID(par->GetUniqueID());
         event.SetBit(id + 3);
         keep_up.ResetBit(id);
      }
// Info("Fired","%s is %s fired, keep_up %s",par->GetName(), par->Fired( optP ) ? "" : "NOT" ,keep_up.String());
   }

   Binary8_t ok = fFiredMask & event;
// Info("Fired","ok %s", ok.String());

   // "all" considered parameters fired if ok == mask
   // "any" considered parameters fired if ok != 0
   if (opt_all)  return (ok == fFiredMask);
   return (ok != "0");
}
//________________________________________________________________

void KVVAMOSDetector::Initialize()
{
   // Initialize the data members. Called by KVVAMOS::Initialize().
}
//________________________________________________________________

void KVVAMOSDetector::SetCalibrators()
{
   // Setup the calibrators for this detector. Call once name
   // has been set.
   // The calibrators are KVFunctionCal.
   // By default the all the calibration functions are first-degree
   // polynomial function and the range [Xmin,Xmax]=[0,4096].
   // Here the calibrator are not ready (KVFunctionCal::GetStatus()).
   // You have to give the parameters and change the status
   // (see KVFunctionCal::SetParameter(...) and KVFunctionCal::SetStatus(...))


   TIter nextpar(GetACQParamList());

   KVEBYEDAT_ACQParam* par   = NULL;
   Double_t    maxch = 16384.;       // 14 bits

   TString  calibtype("ERROR");

   while ((par = (KVEBYEDAT_ACQParam*)nextpar())) {
      Bool_t isTparam = kFALSE;

      if (par->IsType("E")) {
         calibtype = "channel->MeV";
      }
      else if (par->IsType("Q")) {
         calibtype = "channel->Volt";
         maxch     = 4096.;           // 12 bits
      }
      else if (par->GetType()[0] == 'T') {
         isTparam = kTRUE;
         calibtype = "channel->ns";
      }
      else continue;

      calibtype.Append(" ");
      calibtype.Append(par->GetName());

      TF1* func        = new TF1(calibtype.Data(), "pol1", 0., maxch);
      KVFunctionCal* c = new KVFunctionCal(this, func);
      c->SetType(calibtype.Data());
      c->SetLabel(par->GetLabel());
      c->SetNumber(par->GetNumber());
      c->SetUniqueID(par->GetUniqueID());
      c->SetACQParam(par);
      c->SetStatus(kFALSE);
      if (!AddCalibrator(c)) delete c;
      else if (isTparam) {
         if (!fTlist) fTlist = new TList;
         fTlist->Add(par);
         if (!fT0list) fT0list = new TList;
         fT0list->Add(new KVNamedParameter(par->GetName(), 0.));
      }
   }

   // Define and set to zero the T0 values for time of flight measurment
   // from this detector. The time of flight acq parameters are associated
   // to gVamos
   if (gVamos) {
      TIter next_vacq(gVamos->GetVACQParams());
      while ((par = (KVEBYEDAT_ACQParam*)next_vacq())) {
         if ((par->GetType()[0] == 'T') && IsStartForT(par->GetName() + 1)) {
            if (!fTlist) fTlist = new TList;
            fTlist->Add(par);
            if (!fT0list) fT0list = new TList;
            fT0list->Add(new KVNamedParameter(par->GetName(), 0.));
         }
      }
   }

}
//________________________________________________________________

KVFunctionCal* KVVAMOSDetector::GetECalibrator() const
{
   // Returns the calibrator for the conversion channel->MeV of the energy
   // signal.
   // The method assumes that the calibrator is an instance of the class
   // KVFunctionCal.

   TString calname;
   calname.Form("channel->MeV %s", GetEBaseName());
   return (KVFunctionCal*)GetCalibrator(calname.Data());
}
//________________________________________________________________

Bool_t KVVAMOSDetector::GetPositionInVamos(Double_t& X, Double_t& Y)
{
   // Return a position X and Y of the particle track in the VAMOS
   // reference frame from the fired acquisition parameters and from the
   // the location of the TGeoVolumes which defined the detector geometry.
   // The position is randomized in the active volume.

   X = Y = -666;
   Warning("GetPositionInVamos", "To be implemented or overwritten in daughter classes");
   return kFALSE;
}
//________________________________________________________________

UChar_t KVVAMOSDetector::GetRawPosition(Double_t* XYZf)
{
   // Returns in the 'XYZf' array the X and Y coordinates of the position (strip)
   // deduced from the histogram representing the calibrated charge versus strip
   // number. The bit 0 (1) of the UChar_t returned value is set to 1 if
   // the X (Y) position is correctly deduced.

   UChar_t rval = 3;
   if ((XYZf[0] = GetRawPosition('X')) < 0) rval -= 1;
   if ((XYZf[1] = GetRawPosition('Y')) < 0) rval -= 2;
   return rval;
}
//________________________________________________________________

Double_t KVVAMOSDetector::GetRawPosition(Char_t dir, Int_t num)
{
   // Method overwritten and useful in child classes describing detectors
   // used to measured position in the direction 'dir' for the reconstruction of  nucleus
   // trajectory. For example, see this same method in the class KVSeD.
   // The argument 'num'=[1,2,...] is the number of the measured position if the
   // detector is able to measured several positions (e.g. KVDriftChamber)
   // i.e. GetNMeasuredX() or GetNMeasuredY() is greater than 1. If num=0
   // the mean value of the measured positions is returned.
   IGNORE_UNUSED(dir);
   IGNORE_UNUSED(num);
   return -500.;
}
//________________________________________________________________

UChar_t KVVAMOSDetector::GetRawPositionError(Double_t* EXYZf)
{
   // Returns in the 'EXYZf' array the errors of X and Y coordinates of the position
   // returned by GetRawPosition(...).
   // The bit 0 (1) of the UChar_t returned value is set to 1 if
   // the X (Y) position is correctly deduced.

   UChar_t rval = 3;
   if ((EXYZf[0] = GetRawPositionError('X')) < 0) rval -= 1;
   if ((EXYZf[1] = GetRawPositionError('Y')) < 0) rval -= 2;
   return rval;
}
//________________________________________________________________

Double_t KVVAMOSDetector::GetRawPositionError(Char_t dir, Int_t num)
{
   // Method overwritten and useful in child classes describing detectors
   // which returns the error of the measured position in the direction 'dir' for the reconstruction of  nucleus
   // trajectory. For example, see this same method in the class KVSeD.
   // The argument 'num'=[1,2,...] is the number of the measured position if the
   // detector is able to measured several positions (e.g. KVDriftChamber)
   // i.e. GetNMeasuredX() or GetNMeasuredY() is greater than 1. If num=0
   // the mean value of the measured position errors is returned.
   IGNORE_UNUSED(dir);
   IGNORE_UNUSED(num);
   return -500.;
}
//________________________________________________________________

KVFunctionCal* KVVAMOSDetector::GetTCalibrator(const Char_t* type) const
{
   // Returns the calibrator for the conversion channel->ns of a time
   // signal of type 'type' (for example SED_HF, SI_HF, SI_SED1, ...) if
   // the detector was used to defined this time.
   // The method assumes that the calibrator is an instance of the class
   // KVFunctionCal.
   // A time signal is always associated to an object KVVAMOS pointed by
   // gVamos, then gVamos has to be different to zero;

   if (!gVamos) return NULL;
   if (!IsStartForT(type)) return NULL;
   TString calname;
   calname.Form("channel->ns T%s", type);
   return (KVFunctionCal*)gVamos->GetVCalibrator(calname.Data());
}
//________________________________________________________________

Double_t KVVAMOSDetector::GetCalibE()
{
   // Calculate energy in MeV from coder values.
   // Returns 0 if calibration not ready or acquisition parameter not fired
   // (we require that the acquisition parameter has a value
   // greater than the current pedestal value)
   KVFunctionCal* cal = GetECalibrator();
   if (cal && cal->GetStatus() && cal->GetACQParam()->Fired("P"))
      return cal->Compute();
   return 0;
}
//________________________________________________________________

Double_t KVVAMOSDetector::GetCalibT(const Char_t* type)
{
   // Calculate calibrated time in ns of type 'type' (SED_HF, SI_HF,
   // SI_SED1, ...) for coder values.
   // Returns 0 if calibration not ready or time ACQ parameter not fired.
   // (we require that the acquisition parameter has a value
   // greater than the current pedestal value)


   KVFunctionCal* cal = GetTCalibrator(type);
   if (cal && cal->GetStatus() && cal->GetACQParam()->Fired("P"))
      return cal->Compute() + GetT0(type);
   return 0;
}
//________________________________________________________________

Float_t KVVAMOSDetector::GetT(const Char_t* type)
{
   // Returns the coder value of a time acquisition parameter (in channel)
   //  of type 'type' (SED_HF, SI_HF, SI_SED1, ...).
   // A time signal is always associated to an object KVVAMOS pointed by
   // gVamos, then gVamos has to be different to zero. If gVamos is null
   // or type is not correct, this method returns -1;

   if (!IsStartForT(type) || !gVamos) return -1;
   KVEBYEDAT_ACQParam* par = gVamos->GetACQParam(Form("T%s", type));
   return (par ? par->GetData() : -1);
}
//________________________________________________________________

Double_t KVVAMOSDetector::GetT0(const Char_t* type) const
{
   // Returns the value of the constant T0 (in ns) used for calibrating
   // time of flight of type 'type' (SED_HF, SI_HF, SI_SED1, ...).

   if (!fT0list) return 0.;
   KVNamedParameter* par = (KVNamedParameter*)fT0list->FindObject(Form("T%s", type));
   return (par ? par->GetDouble() : 0);
}
//________________________________________________________________

Bool_t KVVAMOSDetector::IsECalibrated() const
{
   // Returns true if the detector has been calibrated in energy.

   KVCalibrator* cal = GetECalibrator();
   return (cal && cal->GetStatus());
}
//________________________________________________________________

Bool_t KVVAMOSDetector::IsTCalibrated(const Char_t* type) const
{
   // Returns true if the detector has been calibrated in time for type 'type'
   // (SED_HF, SI_HF, SI_SED1, ...) i.e. if
   //  -  the ACQ parameter of type 'type' has associated calibrator
   //  -  this calibrator is ready

   KVCalibrator* cal = GetTCalibrator(type);
   return (cal && cal->GetStatus());
}
//________________________________________________________________

Bool_t KVVAMOSDetector::IsStartForT(const Char_t* type) const
{
   // Returns true if the detector is used as start for the time of flight
   // of type 'type' i.e. the type begins with the time base-name
   // (GetTBaseName()).

   TString tmp(type);
   if (tmp.BeginsWith(GetTBaseName())) return kTRUE;
   return kFALSE;
}
//________________________________________________________________

Bool_t KVVAMOSDetector::IsStopForT(const Char_t* type) const
{
   // Returns true if the detector is used as stop for the time of flight
   // of type 'type' i.e. the type ends with the time base-name
   // (GetTBaseName()).

   TString tmp(type);
   if (tmp.EndsWith(GetTBaseName())) return kTRUE;
   return kFALSE;
}
//________________________________________________________________
void KVVAMOSDetector::SetFiredBitmask(KVString& lpar_dummy)
{
   // Set bitmask used to determine which acquisition parameters are
   // taken into account by KVVAMOSDetector::Fired based on the environment variables
   //          [dataset].KVACQParam.[par name].Working:    NO
   //          [classname].Fired.ACQParameterList.[type]: PG,GG,T
   //   where [classname]=KVDetector by default, or the name of some class
   //   derived from KVDetector which calls the method KVDetector::SetKVDetectorFiredACQParameterListFormatString()
   //   in its constructor.
   // The first allows to define certain acquisition parameters as not functioning;
   // they will not be taken into account.
   // The second allows to "fine-tune" what is meant by "all" or "any" acquisition parameters
   // (i.e. when using Fired("all"), Fired("any"), Fired("Pall", etc.).
   // For each detector type, give a comma-separated list of the acquisition
   // parameter types to be taken into account in the KVDetector::Fired method.
   // Only those parameters which appear in the list will be considered:
   // then "all" means => for each type in the list at least one acquisition
   //                     parameter has to be fired
   // and  "any" means => at least for one type of the list an acquisition parameter
   //                     has to be fired
   // These lists are read during construction of VAMOS (KVVAMOS::Build),
   // the method KVVAMOS::SetACQParams uses them to define a mask for each detector
   // of the spectrometer.
   // X, Y and Z types that we can set in a list are not associated to acquisition
   // parameters. In the bitmask, the 3 first bits are kept for these coordinates.
   // If one of these 3 bits is 1 then the methode GetRawPosition(Double_t *)
   // is called and the returned values is compared to these 3 bits.
   // If no variable [dataset].[classname].Fired.ACQParameterList.[type] exists,
   // we set a bitmask authorizing all acquisition parameters of the detector, e.g.
   // if the detector has 3 types of acquisition parameters which are fired byt
   // no position type then the bitmask will be "111000"

   IGNORE_UNUSED(lpar_dummy); // lpar is determined below

   fFiredMask.Set("");

   KVString inst;
   inst.Form(GetFiredACQParameterListFormatString(), GetType());
   KVString lpar = gDataSet->GetDataSetEnv(inst);
   TObjArray* toks = lpar.Tokenize(",");
   if (!toks->GetEntries()) {
      fFiredMask.Set("11111111");
      delete toks;
      return;
   }

   // 3 first bits for XYZ positions
   UChar_t idx;
   const Char_t* pos[3] = {"X", "Y", "Z"};
   for (Int_t i = 0; i < 3; i++) {
      idx = GetPositionTypeIdx(pos[i]) ;
      if ((idx < 9) && (toks->FindObject(pos[i]))) fFiredMask.SetBit(i);
   }

   UChar_t Nbits = 3;

   // other bits for Acquisition parameters
   UChar_t idx_max = 0;
   Bool_t  found   = kFALSE;
   TIter next(toks);
   TObject* obj = NULL;
   while ((obj = next())) {
      idx = GetACQParamTypeIdx(obj->GetName());
      if (idx < 9) {
         found = kTRUE;
         fFiredMask.SetBit(idx + Nbits);
         if (idx > idx_max) idx_max = idx;
      }
   }
   delete toks;

   if (found) Nbits += idx_max + 1;

   fFiredMask.SetNBits(Nbits);
}
//________________________________________________________________

void KVVAMOSDetector::SetT0(const Char_t* type, Double_t t0)
{
   // Set the value of the constant T0 (in ns) used for calibrating
   // time of flight of type 'type' (SED_HF, SI_HF, SI_SED1, ...).

   KVNamedParameter* par = (KVNamedParameter*)fT0list->FindObject(Form("T%s", type));
   if (par) par->Set(t0);
   else Error("SetT0", "Impossible to set T0 for unknown time ACQ parameter %s", type);
}
//________________________________________________________________

const Char_t* KVVAMOSDetector::GetEBaseName() const
{
   // Base name of the energy used to be compatible
   // GANIL acquisition parameters
   //
   // The base name is "E<type><number>".

   return Form("E%s%d", GetType(), GetNumber());
}
//________________________________________________________________

const Char_t* KVVAMOSDetector::GetTBaseName() const
{
   // Base name of the time of flight parameters used to be compatible
   // GANIL acquisition parameters
   //
   // The root of the name is the detector type.

   return GetType();
}
//________________________________________________________________

Double_t KVVAMOSDetector::GetCalibT_HF()
{
   // Returns calibrated time of flight in ns,  from coder value of
   // the time measurment between the beam pulse and timing
   // from this detector.
   // Returns 0 if calibration not ready or ACQ parameter not fired.

   return GetCalibT(Form("%s_HF", GetTBaseName()));
}
//________________________________________________________________

Float_t KVVAMOSDetector::GetT_HF()
{
   // Access acquisition data value associated to the time of flight
   // recorded between the beam pulse and timing from this detector.
   // Returns value as a floating-point number which is the raw channel number
   // read from the coder plus a random number in the range [-0.5,+0.5].
   // If the detector has no ACQ parameter for time of flight,
   // or if the raw channel number = 0, the value returned is -1

   return GetT(Form("%s_HF", GetTBaseName()));
}
//________________________________________________________________

Double_t KVVAMOSDetector::GetT0_HF() const
{
   // Retruns value of the constant T0 (in ns) used for calibrating
   // time of flight recorded between the beam pulse and timing
   // from this detector.

   return GetT0(Form("%s_HF", GetTBaseName()));
}
//________________________________________________________________

Bool_t KVVAMOSDetector::IsTHFCalibrated() const
{

   return IsTCalibrated(Form("%s_HF", GetTBaseName()));
}
//________________________________________________________________

void KVVAMOSDetector::SetT0_HF(Double_t t0)
{
   // Set value of the constant T0 (in ns) used for calibrating
   // time of flight recorded between the beam pulse and timing
   // from this detector.

   SetT0(Form("%s_HF", GetTBaseName()), t0);
}
