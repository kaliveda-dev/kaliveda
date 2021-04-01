//Created by KVClassFactory on Wed Jul 25 10:12:53 2012
//Author: Guilain ADEMARD

#include "KVHarpeeCsI.h"
#include "TGeoBBox.h"
#include "KVUnits.h"
#include "TClass.h"

ClassImp(KVHarpeeCsI)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVHarpeeSi</h2>
<h4>CsI detectors of Harpee, used at the focal plan of VAMOS for the e503 experiment</h4>
<!-- */
// --> END_HTML
//
// In order to create a detector, use the KVHarpeeCsI::KVHarpeeCsI()
// constructor.
//
// Type of detector : "CSI"
////////////////////////////////////////////////////////////////////////////////

KVList*      KVHarpeeCsI::fHarpeeCsIList  = NULL;
KVHarpeeCsI* KVHarpeeCsI::fCsIForPosition = NULL;
KVString    KVHarpeeCsI::fACQParamTypes("0:E, 9:NO_TYPE");
KVString    KVHarpeeCsI::fPositionTypes("9:NO_TYPE");

void KVHarpeeCsI::init()
{
   // Initialise non-persistent pointers

   if (!fHarpeeCsIList) {
      fHarpeeCsIList = new KVList(kFALSE);
   }
   fHarpeeCsIList->Add(this);

   fCal = NULL;

   // fSegment is set to 1 because this CsI detector is
   // an independant detector (see KVGroup::AnalyseParticles for
   // more information)
   fSegment = 1;
}
//________________________________________________________________

KVHarpeeCsI::KVHarpeeCsI()
{
   // Default constructor.
   init();
   SetType("CSI");
   SetLabel("CSI");
   SetName(GetArrayName());
}
//________________________________________________________________


KVHarpeeCsI::KVHarpeeCsI(const KVHarpeeCsI& obj)  : KVVAMOSDetector()
{
   // Copy constructor
   // This ctor is used to make a copy of an existing object (for example
   // when a method returns an object), and it is always a good idea to
   // implement it.
   // If your class allocates memory in its constructor(s) then it is ESSENTIAL :-)

   obj.Copy(*this);
}

KVHarpeeCsI::~KVHarpeeCsI()
{
   // Destructor
   fHarpeeCsIList->Remove(this);
   if (fHarpeeCsIList && !fHarpeeCsIList->GetEntries()) SafeDelete(fHarpeeCsIList);
}
//________________________________________________________________

void KVHarpeeCsI::Copy(TObject& obj) const
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

const Char_t* KVHarpeeCsI::GetArrayName()
{
   // Name of detector given in the form
   // CSI01 CSI02 ...
   // to be compatible with GANIL acquisition parameters.
   //
   // The root of the name is the detector type + number.

   fFName = Form("%s%02d", GetType(), GetNumber());
   return fFName.Data();
}
//________________________________________________________________

KVHarpeeCsI* KVHarpeeCsI::GetFiredHarpeeCsI(Option_t* opt)
{
   // This static method returns the first fired detector found
   // in the list of all the existing silicon detectors of HARPEE.
   // See KVDetector::Fired() for more information about the option 'opt'.

   TIter next(fHarpeeCsIList);
   KVHarpeeCsI* csi = NULL;
   while ((csi = (KVHarpeeCsI*)next())) {
      if (csi->Fired(opt)) return csi;
   }
   return NULL;
}
//________________________________________________________________

KVList* KVHarpeeCsI::GetHarpeeCsIList()
{
   //Returns the global list of all KVHarpeeSi objects.
   return fHarpeeCsIList;
}
//________________________________________________________________

Int_t KVHarpeeCsI::GetMult(Option_t* opt)
{
   // Returns the multiplicity of fired silicon detectors of HARPEE.
   // See KVDetector::Fired() for more information about the option 'opt'.

   Int_t mult   = 0;

   TIter next(fHarpeeCsIList);
   KVHarpeeCsI* csi = NULL;
   while ((csi = (KVHarpeeCsI*)next())) {
      if (csi->Fired(opt)) mult++;
   }
   return mult;
}
//________________________________________________________________

void KVHarpeeCsI::SetACQParams()
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

const Char_t* KVHarpeeCsI::GetEBaseName() const
{
   // Base name of the energy used to be compatible
   // GANIL acquisition parameters
   //
   // The base name is "<type><number>".

   return Form("%s%.2d", GetType(), GetNumber());
}

//______________________________________________________________________________
void KVHarpeeCsI::SetCalibrators()
{
   // Build and set to the list of calibrators the Total light calibrator.
   // This calibrator has the type "Light->MeV <name>"
   if (fCal) return;
   fCal = new KVLightEnergyCsIVamos(this);
   fCal->SetType(Form("Light->MeV %s", GetName()));
   if (AddCalibrator(fCal)) return;
   KVLightEnergyCsIVamos* tmp = fCal;
   fCal = (KVLightEnergyCsIVamos*)fCalibrators->FindObject(tmp);
   delete tmp;
}
//________________________________________________________________

void KVHarpeeCsI::Initialize()
{
   // Initialize the data members. Called by KVVAMOS::Initialize().
   fCsIForPosition = NULL;
   ResetBit(kPosIsOK);
}
//________________________________________________________________

Bool_t KVHarpeeCsI::PositionIsOK()
{
   // Returns true if all the conditions to access to the particle position
   // are verified. In this case the position is given by the method
   // GetPosition(...).
   // The conditions are:
   //   -the multiplicity of fired ( "Pany" option of Fired() ) Harpee CsI
   //    detectors must be equal to one;
   //   -this detectector must be the fired detector.

   if (!TestBit(kPosIsOK)) {
      Int_t mult   = 0;
      TIter next(fHarpeeCsIList);
      KVHarpeeCsI* csi = NULL;
      while ((csi = (KVHarpeeCsI*)next())) {
         if (csi->Fired("Pany")) {
            mult++;
            fCsIForPosition = csi;
         }
      }
      if (mult != 1) fCsIForPosition = NULL;
      SetBit(kPosIsOK);
   }
   return fCsIForPosition == this;
}
//________________________________________________________________

void KVHarpeeCsI::Streamer(TBuffer& R__b)
{
   // Stream an object of class KVHarpeeCsI.
   // We set the pointers to the calibrator objects

   if (R__b.IsReading()) {
      KVHarpeeCsI::Class()->ReadBuffer(R__b, this);
      TIter next(GetListOfCalibrators());
      TObject* cal = NULL;
      while (!fCal && (cal = next())) {
         if (cal->InheritsFrom(KVLightEnergyCsIVamos::Class()))
            fCal  = (KVLightEnergyCsIVamos*)cal;
      }
   }
   else {
      KVHarpeeCsI::Class()->WriteBuffer(R__b, this);
   }
}

Bool_t KVHarpeeCsI::IsECalibrated() const
{
   if (!fCal) return kFALSE;
   return fCal->GetStatus();
}

Double_t KVHarpeeCsI::GetCorrectedEnergy(KVNucleus* nuc, Double_t light,
      Bool_t transmission)
{
   IGNORE_UNUSED(transmission);

   // Ensure the nucleus is not modified
   const KVNucleus* const n(nuc);
   assert(n);

   Int_t Z(n->GetZ());
   Int_t A(n->GetA());

   assert((Z > 0) && (Z < 150));
   assert((A > 0) && (A < 300));

   Double_t eloss(0.);
   assert(fCal);

   fCal->SetZ(Z);
   fCal->SetA(A);

   if (IsSimMode()) {
      // Calculate energy loss from the predicted light output of the detected
      // energy.
      eloss = fCal->Compute(fCal->Invert(GetEnergy()));
   }
   else if (light < 0.) {
      // Light has not been set so use the ACQ data
      eloss = fCal->Compute(GetACQData(GetEBaseName()) -
                            GetPedestal(GetEBaseName()));
   }
   else {
      // Light has been set manually
      eloss = fCal->Compute(light);
   }

   if (eloss < 0.) eloss = 0.;
   SetEnergy(eloss);

   return eloss;
}
