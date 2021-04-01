//Created by KVClassFactory on Wed Jul 25 10:12:53 2012
//Author: Guilain ADEMARD

#include "KVHarpeeSi.h"
#include "TGeoBBox.h"
#include "KVUnits.h"
#include "KVRecombination.h"
#include "TClass.h"

ClassImp(KVHarpeeSi)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVHarpeeSi</h2>
<h4>Silicon detectors of Harpee, used at the focal plan of VAMOS</h4>
<!-- */
// --> END_HTML
//
// In order to create a detector, use the KVHarpeeSi::KVHarpeeSi()
// constructor.
//
// Type of detector : "SI"
////////////////////////////////////////////////////////////////////////////////

KVList*     KVHarpeeSi::fHarpeeSiList  = NULL;
KVHarpeeSi* KVHarpeeSi::fSiForPosition = NULL;
KVString    KVHarpeeSi::fACQParamTypes("0:E, 1:T_HF, 2:T, 9:NO_TYPE");
KVString    KVHarpeeSi::fPositionTypes("9:NO_TYPE");

void KVHarpeeSi::init()
{
   // Initialise non-persistent pointers

   if (!fHarpeeSiList) {
      fHarpeeSiList = new KVList(kFALSE);
//    fHarpeeSiList->SetCleanup();
   }
   fHarpeeSiList->Add(this);

   fCanalE = NULL;
   fPHD    = NULL;

   // fSegment is set to 1 because this silicon detector is
   // an independant detector (see KVGroup::AnalyseParticles for
   // more information)
   fSegment = 1;
}
//________________________________________________________________

KVHarpeeSi::KVHarpeeSi()
{
   // Default constructor.
   init();
   SetType("SI");
   SetLabel("SI");
   SetName(GetArrayName());
}
//________________________________________________________________

KVHarpeeSi::KVHarpeeSi(UInt_t number, Float_t thick) : KVVAMOSDetector(number, "Si")
{
   // Make a silicon detector of Harpee.
   // Type of detector:  "SI"
   // Label of detector: "SI"

   init();
   SetType("SI");
   SetLabel("SI");
   SetName(GetArrayName());

   Float_t w  = 1.;                // width
   Float_t h  = 1.;                // height
   Float_t th = thick * KVUnits::um; // thickness


   Double_t dx = w / 2;
   Double_t dy = h / 2;
   Double_t dz = th / 2;

   fTotThick += th;

   // adding the absorber
   TGeoShape* shape = new TGeoBBox(dx, dy, dz);
   AddAbsorber("Si", shape, 0, kTRUE);

}
//________________________________________________________________

KVHarpeeSi::KVHarpeeSi(const KVHarpeeSi& obj)  : KVVAMOSDetector()
{
   // Copy constructor
   // This ctor is used to make a copy of an existing object (for example
   // when a method returns an object), and it is always a good idea to
   // implement it.
   // If your class allocates memory in its constructor(s) then it is ESSENTIAL :-)

   obj.Copy(*this);
}

KVHarpeeSi::~KVHarpeeSi()
{
   // Destructor
   fHarpeeSiList->Remove(this);
   if (fHarpeeSiList && !fHarpeeSiList->GetEntries()) SafeDelete(fHarpeeSiList);
}
//________________________________________________________________

void KVHarpeeSi::Copy(TObject& obj) const
{
   // This method copies the current state of 'this' object into 'obj'
   // You should add here any member variables, for example:
   //    (supposing a member variable KVHarpeeSi::fToto)
   //    CastedObj.fToto = fToto;
   // or
   //    CastedObj.SetToto( GetToto() );

   KVVAMOSDetector::Copy(obj);
   //KVHarpeeSi& CastedObj = (KVHarpeeSi&)obj;
}
//________________________________________________________________

const Char_t* KVHarpeeSi::GetArrayName()
{
   // Name of detector given in the form
   // SIE_01 SIE_02 ...
   // to be compatible with GANIL acquisition parameters.
   //
   // The root of the name is the detector type + number.

   fFName = Form("%s_%02d", GetType(), GetNumber());
   return fFName.Data();
}
//________________________________________________________________

Double_t KVHarpeeSi::GetCalibT(const Char_t* type)
{
   // Calculate calibrated time in ns of type 'type' (SI_HF, SI_SED1,
   // SI_INDRA, SI_MCP, ...) for coder values.
   // Returns 0 if calibration not ready or time ACQ parameter not fired.
   // (we require that the acquisition parameter has a value
   // greater than the current pedestal value).
   // The reference time T0 used for the calibration is the one of this
   // detector if it is fired otherwise T0 is the one of the
   // fired silicon detector of Harpee which stopped the time.

   KVHarpeeSi* firedSi = (Fired("Pany") ? this : GetFiredHarpeeSi());
   if (!firedSi) return 0;

   KVFunctionCal* cal = GetTCalibrator(type);
   if (cal && cal->GetStatus() && cal->GetACQParam()->Fired("P"))
      return cal->Compute() + firedSi->GetT0(type);
   return 0;
}
//________________________________________________________________

Double_t KVHarpeeSi::GetCanalFromEnergy(Double_t e)
{
   // Inverts calibration, i.e. calculates the channel value associated
   // to a given energy (in MeV). If the energy is not given, the value
   // returned by GetEnergy() is used.

   if (e < 0) e = GetEnergy();
   if (fCanalE && fCanalE->GetStatus()) return fCanalE->Invert(e);
   return -1.;
}
//________________________________________________________________

const Char_t* KVHarpeeSi::GetEBaseName() const
{
   // Base name of the energy used to be compatible
   // GANIL acquisition parameters
   //
   // The base name is "E<type><number>".

   return Form("%sE_%.2d", GetType(), GetNumber());
}
//______________________________________________________________________________

TF1* KVHarpeeSi::GetELossFunction(Int_t Z, Int_t A)
{
   // Overrides KVDetector::GetELossFunction
   // If the pulse height deficit (PHD) has been set for this detector,
   // we return an energy loss function which takes into account the PHD,
   // i.e. for an incident energy E we calculate
   //
   //      dEphd(E,Z,A) = dE(E,Z,A) - PHD(dE,Z)
   //
   // If no PHD is set, we return the usual KVDetector::GetELossFunction
   // which calculates dE(E,Z,A)

   if (fPHD && fPHD->GetStatus()) return fPHD->GetELossFunction(Z, A);

   return KVDetector::GetELossFunction(Z, A);
}
//________________________________________________________________

Double_t KVHarpeeSi::GetEnergy()
{
   //Redefinition of KVDetector::GetEnergy().
   //If energy lost in active volume is already set (e.g. by calculation of energy loss
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

Double_t KVHarpeeSi::GetEnergyFromCanal(Double_t c)
{
   // Calculates the calibrated energy in MeV from the channel value.
   // If the channel is not given, the coder value of the acq. parameter
   // associated to the calibrator channel->MeV is used.

   if (!fCanalE) Error("GetEnergyFromCanal", "fCanalE not found");
   if (fCanalE && fCanalE->GetStatus()) {
      if (c < 0) return fCanalE->Compute();
      else      return fCanalE->Compute(c);
   }
   return 0.;
}
//________________________________________________________________

KVHarpeeSi* KVHarpeeSi::GetFiredHarpeeSi(Option_t* opt)
{
   // This static method returns the first fired detector found
   // in the list of all the existing silicon detectors of HARPEE.
   // See KVDetector::Fired() for more information about the option 'opt'.

   TIter next(fHarpeeSiList);
   KVHarpeeSi* si = NULL;
   while ((si = (KVHarpeeSi*)next())) {
      if (si->Fired(opt)) return si;
   }
   return NULL;
}
//________________________________________________________________

KVList* KVHarpeeSi::GetHarpeeSiList()
{
   //Returns the global list of all KVHarpeeSi objects.
   return fHarpeeSiList;
}
//________________________________________________________________

Int_t KVHarpeeSi::GetMult(Option_t* opt)
{
   // Returns the multiplicity of fired silicon detectors of HARPEE.
   // See KVDetector::Fired() for more information about the option 'opt'.

   Int_t mult   = 0;

   TIter next(fHarpeeSiList);
   KVHarpeeSi* si = NULL;
   while ((si = (KVHarpeeSi*)next())) {
      if (si->Fired(opt)) mult++;
   }
   return mult;
}
//________________________________________________________________

Double_t KVHarpeeSi::GetPHD(Double_t dE, UInt_t Z, UInt_t A)
{
   //Calculate Pulse Height Defect in MeV for a given energy loss dE(MeV) and Z.
   //The formula of Parlog is used (see class KVRecombination).
   //
   //Returns 0 if PHD is not defined.

   if (!fPHD || !fPHD->GetStatus()) return 0.;
   fPHD->SetZandA(Z, A);
   return fPHD->Compute(dE);
}
//________________________________________________________________

void KVHarpeeSi::Initialize()
{
   // Initialize the data members. Called by KVVAMOS::Initialize().
   fSiForPosition = NULL;
   ResetBit(kPosIsOK);
}
//________________________________________________________________

Bool_t KVHarpeeSi::PositionIsOK()
{
   // Returns true if all the conditions to access to the particle position
   // are verified. In this case the position is given by the method
   // GetPosition(...).
   // The conditions are:
   //   -the multiplicity of fired ( "Pany" option of Fired() ) Harpee Si
   //    detectors must be equal to one;
   //   -this detectector must be the fired detector.

   if (!TestBit(kPosIsOK)) {
      Int_t mult   = 0;
      TIter next(fHarpeeSiList);
      KVHarpeeSi* si = NULL;
      while ((si = (KVHarpeeSi*)next())) {
         if (si->Fired("Pany")) {
            mult++;
            fSiForPosition = si;
         }
      }
      if (mult != 1) fSiForPosition = NULL;
      SetBit(kPosIsOK);
   }
   return fSiForPosition == this;
}
//________________________________________________________________

void KVHarpeeSi::SetACQParams()
{
// Setup the energy acquisition parameter for this silicon detector.
// This parameter has the name of the detector and has the type 'E'
// (for energy).
//
   KVEBYEDAT_ACQParam* par = new KVEBYEDAT_ACQParam;
   par->SetName(GetEBaseName());
   par->SetType("E");
   par->SetNumber(GetNumber());
   par->SetUniqueID(CalculateUniqueID(par));
   AddACQParam(par);
}
//________________________________________________________________

void KVHarpeeSi::SetCalibrators()
{
   // Pulse Height Defect calibrator as well as the calibrators of
   // KVVAMOSDetector.

   KVVAMOSDetector::SetCalibrators();

   // Set PHD calibrator only if the detector has an acq. parameter
   // with type 'E'
   TObject* par = GetACQParamList()->FindObjectByType("E");
   if (!par) return;

   TString type("channel->MeV ");
   type.Append(par->GetName());
   fCanalE = (KVFunctionCal*)GetCalibrator(type.Data());

   if (!fCanalE) Error("SetCalibrators", "channel->MeV calibrator not found");

   KVRecombination* c = new KVRecombination(this);
   type = c->GetType();
   type.Append(" ");
   type.Append(par->GetName());
   c->SetType(type.Data());
   if (!AddCalibrator(c)) delete c;

   fPHD = (KVRecombination*)GetCalibrator(type.Data());
}
//________________________________________________________________

void KVHarpeeSi::SetParlogPHDParameter(Double_t a)
{
   //Sets parameter of Parlog formula used to calculate PHD for particles
   //stopping in this detector. The parameters are as in the following:
   //
   //      PHD(E) = 1/2 * Ed *( 1-1/X * ln|1+X| + X * ln|1+1/X| )
   // with X      = a*A*Z**2/Ed
   //      Ed     = energy lost by particle in detector (=E if particle stops)
   //
   //See class KVRecombination

   if (fPHD) {
      fPHD->SetParameters(a);
      fPHD->SetStatus(kTRUE);
   }
}
//________________________________________________________________

void KVHarpeeSi::Streamer(TBuffer& R__b)
{
   // Stream an object of class KVHarpeeSi.
   // We set the pointers to the calibrator objects

   if (R__b.IsReading()) {
      KVHarpeeSi::Class()->ReadBuffer(R__b, this);
      TIter next(GetListOfCalibrators());
      TObject* cal = NULL;
      while ((cal = next())) {
         if (cal->InheritsFrom(KVRecombination::Class()))
            fPHD  = (KVRecombination*)cal;
         else if (cal->InheritsFrom(KVFunctionCal::Class()))
            fCanalE = (KVFunctionCal*)cal;
      }
   }
   else {
      KVHarpeeSi::Class()->WriteBuffer(R__b, this);
   }
}
