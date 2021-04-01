//Created by KVClassFactory on Wed Jul 25 10:14:20 2012
//Author: Guilain ADEMARD

#include "KVHarpeeIC.h"
#include "TGeoBBox.h"
#include "TGeoMatrix.h"

ClassImp(KVHarpeeIC)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVHarpeeIC</h2>
<h4>Ionisiation chamber of Harpee, used at the focal plan of VAMOS</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVString KVHarpeeIC::fACQParamTypes("0:E, 9:NO_TYPE");
KVString KVHarpeeIC::fPositionTypes("9:NO_TYPE");

void KVHarpeeIC::init()
{
   // Initialise non-persistent pointers

}
//________________________________________________________________

KVHarpeeIC::KVHarpeeIC()
{
   // Default constructor
   // This constructor must exist in order for cloning of detectors
   // to work (as used in KVTelescope::AddDetector).
   // Do not replace this constructor by giving a default value for
   // the first argument of KVHarpeeIC(Float_t, Float_t).
   init();
   SetType("CHI");
   SetLabel("CHI");
   SetName(GetArrayName());
}
//________________________________________________________________

KVHarpeeIC::KVHarpeeIC(UInt_t number, Float_t pressure, Float_t temp, Float_t thick) : KVVAMOSDetector(number, "C4H10")
{
   // Make the ionisation chamber of Harpee composed by:
   // - one 1.5 mylar window;
   // - 3 volumes of C4H10 with thicknesses (2*dz): 2.068 cm, thick, 1.278 cm.
   //   By default thick=10.457 cm;
   //   Only the 2nd gaz volume is "active".
   //
   // The width (2*dx) of this detector is ??? and the height (2*dy) is ???.
   // The ionisation chamber is closed with the silicon wall in Harpee.
   //
   // Input: pressure - pressure of the gaz in mbar
   //        temp     - temperature of the gaz in degrees celsius.
   //                   Default temperature = 19°C
   //        thick    - the thickness in cm of the 2nd volume of gaz.
   Warning("KVHarpeeIC", "Check if the width, the height and the thickness are correct in this constructor");

   init();
   SetType("CHI");
   SetLabel("CHI");
   SetName(GetArrayName());

   // number of absorber
   Int_t Nabs = 4;

   // material of each absorber
   const Char_t* mat[] = {"Myl", "C4H10", "C4H10", "C4H10"};

   // thickness of each absorber
   Float_t th[] = {
      static_cast<Float_t>(1.5 * KVUnits::um),
      static_cast<Float_t>(2.068 * KVUnits::cm),
      static_cast<Float_t>(thick * KVUnits::cm),
      static_cast<Float_t>(1.278 * KVUnits::cm)
   };

   // active absorber flag
   Bool_t  active[] = { kFALSE, kFALSE, kTRUE, kFALSE};

   // width and height of the detector. TO BE VERIFIED
   Float_t w = 40 * KVUnits::cm;
   Float_t h = 12 * KVUnits::cm;

   // total thickness of the detector
   Float_t wtot = 0;
   for (Int_t i = 0; i < Nabs; i++) wtot += th[i];

   // pressure and temperature of the detector
   SetPressure(pressure * KVUnits::mbar);
   SetTemperature(temp);

   // Adding the absorbers
   TGeoShape* shape = NULL;
   for (Int_t i = 0; i < Nabs; i++) {

      Double_t dx = w / 2;
      Double_t dy = h / 2;
      Double_t dz = th[i] / 2;

      fTotThick += th[i];

      // box shape of the absorber
      shape  = new TGeoBBox(dx, dy, dz);

      // build and position absorber in mother volume.
      // Reference is the center of absorber
      Double_t ztrans = dz - wtot / 2;
      for (Int_t j = 0; j < Nabs - 1; j++) ztrans += (j < i) * th[j];
      TGeoTranslation* tr = new TGeoTranslation(0., 0., ztrans);
      AddAbsorber(mat[i], shape, tr, active[i]);
   }
}
//________________________________________________________________

KVHarpeeIC::KVHarpeeIC(const KVHarpeeIC& obj)  : KVVAMOSDetector()
{
   // Copy constructor
   // This ctor is used to make a copy of an existing object (for example
   // when a method returns an object), and it is always a good idea to
   // implement it.
   // If your class allocates memory in its constructor(s) then it is ESSENTIAL :-)

   obj.Copy(*this);
}
//________________________________________________________________

KVHarpeeIC::~KVHarpeeIC()
{
   // Destructor
}
//________________________________________________________________

void KVHarpeeIC::Copy(TObject& obj) const
{
   // This method copies the current state of 'this' object into 'obj'
   // You should add here any member variables, for example:
   //    (supposing a member variable KVHarpeeIC::fToto)
   //    CastedObj.fToto = fToto;
   // or
   //    CastedObj.SetToto( GetToto() );

   KVVAMOSDetector::Copy(obj);
   //KVHarpeeIC& CastedObj = (KVHarpeeIC&)obj;
}
//________________________________________________________________

const Char_t* KVHarpeeIC::GetArrayName()
{
   // Name of the detector given in the form
   // ECHI
   //
   // The root of the name is 'E'+ the detector type.

   fFName = Form("%s", GetType());
   return fFName.Data();
}
//________________________________________________________________

const Char_t* KVHarpeeIC::GetEBaseName() const
{
   // Base name of the energy used to be compatible
   // GANIL acquisition parameters
   //
   // The base name is "E<type>".

   return Form("E%s", GetType());
}
//________________________________________________________________

Double_t KVHarpeeIC::GetEnergy()
{
   //Redefinition of KVDetector::GetEnergy().
   //If energy lost in active (gas) volume is already set (e.g. by calculation of energy loss
   //of charged particles), return its value.
   //If not, we calculate it and set it using the values read from coders (if fired)
   //and the calibrations, if present
   //
   //Returns 0 if (i) no calibration present (ii) calibration present but no data in acquisition parameters

   //fELoss already set, return its value
   Double_t ELoss = KVDetector::GetEnergy();
   if (IsSimMode()) return ELoss; // in simulation mode, return calculated energy loss in active layer
   if (ELoss > 0) return ELoss;
   ELoss = GetCalibE();
   if (ELoss < 0) ELoss = 0;
   SetEnergy(ELoss);
   return ELoss;
}
//________________________________________________________________

UInt_t KVHarpeeIC::GetFiredSegNumber(Option_t* opt)
{
   // return the number of the fired segment of this Harpee ionisation chamber
   // ( number between 1 to 7 ). Returns 0 if no segment or several segments
   // are fired. A segment is considered as fired if at least one of its 3
   // acquisition parameters (A, B and C) is fired (KVACQParam::Fired( opt )).
   // Set the option opt = "P" to accept only the acquisition parameters with
   // their value above the pedestal.

   TIter next(GetACQParamList());
   KVEBYEDAT_ACQParam* par = NULL;
   UInt_t num = 0;

   while ((par = (KVEBYEDAT_ACQParam*)next())) {
      if (par->Fired(opt)) {
         if (num && (num != par->GetNumber())) return 0;
         num = par->GetNumber();
      }
   }
   return num;
}
//________________________________________________________________

Double_t KVHarpeeIC::GetCalibE()
{
   // Calculate energy in MeV from coder values. Only the 7 segments B
   // are used to calculate calibrated energy.
   // Returns 0 if calibration not ready for fired acquisition parameters
   // or if no acquisition parameter is fired.
   // (we require that at least one acquisition parameter has a value
   // greater than the current pedestal value).


   Bool_t ok    = kTRUE;
   Double_t E   = 0;

   TIter next(GetListOfCalibrators());
   KVFunctionCal* cal = NULL;
   while (ok && (cal = (KVFunctionCal*)next())) {

      //Energy only coded in ACQParam with label 'A'
      if (cal->GetLabel()[0] != 'A') continue;

//    Int_t num = cal->GetUniqueID();
//    if( ((num%100)/10) != 1 ) continue;

      if (cal->GetACQParam()->Fired("P")) {
         if (cal->GetStatus()) {
            E += cal->Compute();
         }
         else ok = kFALSE;
      }
   }
   return (ok ? E : 0.);
}
//________________________________________________________________

Double_t KVHarpeeIC::GetCalibE(Int_t seg_num)
{
   // Calculate energy in MeV from the coder value of the acquisition parameter
   // with number 'seg_num' (i.e. segment number) and its corresponding calibrator.
   // Returns 0 if calibration not ready for the acquisition parameter.

   KVFunctionCal* cal = (KVFunctionCal*)GetListOfCalibrators()->FindObjectWithMethod(Form("%d", seg_num), "GetNumber");
   if (cal && cal->GetStatus() && cal->GetACQParam()->Fired("P")) return cal->Compute();
   return 0.;
}
//________________________________________________________________

Bool_t KVHarpeeIC::IsECalibrated() const
{
   // Returns true if the detector has been calibrated in energy.
   // The ionization chamber is considered as calibrated if the
   // The 7 segments B are calibrated.

   Int_t Ncal = 0;
   Bool_t ok  = kTRUE;

   TIter next(GetListOfCalibrators());
   KVCalibrator* cal = NULL;
   while ((cal = (KVCalibrator*)next())) {

      //Energy only coded in ACQParam with label 'A'
      if (cal->GetLabel()[0] != 'A') continue;

//    Int_t num = cal->GetUniqueID();
//    if( ((num%100)/10) != 1 ) continue;
      if (!cal->GetStatus()) {
         ok = kFALSE;
         break;
      }
      Ncal++;
   }
   return ok && (Ncal > 6);
}
//________________________________________________________________

Bool_t KVHarpeeIC::PositionIsOK()
{
   // Check if all the conditions to access to the particle position
   // are verified. For the moment the position can not be given by harpee
   // ionization chamber.
   return kFALSE;
}
//________________________________________________________________

void KVHarpeeIC::SetACQParams()
{
   // Setup acquisition parameters of this ionisation chamber.
   // ACQ parameters with type 'E':
   // E[detector type]_[A,B,C]_[detector number]
   //
   // WARNING: actually only the acquisition parameters with indice 'A'
   // are defined ('B' and 'C' were not coded in e494s and e503 experiment ).

   TString name;
   Char_t idx[] = {'A', 'B', 'C'};

   for (Int_t num = 1; num <= ARPEEIC_NSEG; num++) {
//    for(Int_t i = 0; i<3; i++){
      for (Int_t i = 0; i < 1; i++) {
         KVEBYEDAT_ACQParam* par = new KVEBYEDAT_ACQParam;
         name.Form("E%s_%c_%d", GetType(), idx[i], num);
         par->SetName(name);
         par->SetType("E");
         par->SetLabel(Form("%c", idx[i]));
         par->SetNumber(num);
         par->SetUniqueID(CalculateUniqueID(par));
         AddACQParam(par);
      }
   }
}
//________________________________________________________________

void KVHarpeeIC::SetPressure(Double_t p /* mbar */)
{
   // Set the same  pressure for each gas layer (in mbar)

   if (!IsGas()) return;

   KVMaterial* abs = NULL;
   TIter next(GetListOfAbsorbers());
   while ((abs = (KVMaterial*)next())) {
      if (abs->IsGas()) {
         abs->SetPressure(p * KVUnits::mbar);
         if (abs->GetAbsGeoVolume())
            abs->GetAbsGeoVolume()->GetMaterial()->SetPressure(p * KVUnits::mbar);
      }
   }

   KVDetector::SetPressure(p * KVUnits::mbar);
}
//________________________________________________________________

void KVHarpeeIC::SetTemperature(Double_t t)
{
   // Set the same temperature for each gaz layer.
   // The units are: degrees celsius

   if (!IsGas()) return;

   KVMaterial* abs = NULL;
   TIter next(GetListOfAbsorbers());
   while ((abs = (KVMaterial*)next())) {
      if (abs->IsGas()) {
         abs->SetTemperature(t);
         if (abs->GetAbsGeoVolume())
            abs->GetAbsGeoVolume()->GetMaterial()->SetTemperature(t);
      }
   }
}
