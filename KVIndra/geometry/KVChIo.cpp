#include "Riostream.h"
#include "KVChIo.h"
#include "TClass.h"

ClassImp(KVChIo)
//______________________________________________________________________________

void KVChIo::init()
{
   fSegment = 0;
}

KVChIo::KVChIo()
{
   //Default ctor
   //This ctor (which in turn calls the KVDetector default ctor) must exist in
   //order for Cloning of detectors to work (as used in KVTelescope::AddDetector).
   //Do not replace this ctor by giving a default value for the first argument of KVChIo(Float_t,Float_t).
   //
   init();
}

//______________________________________________________________________________
KVChIo::KVChIo(Float_t pressure, Float_t thick): KVINDRADetector("Myl", 2.5 * KVUnits::um)
{
   // Make an INDRA ChIo: 2.5micron mylar windows enclosing 'thick' cm of C3F8,
   // give gas pressure in mbar
   // By default 'thick'=5cm
   // The type of these detectors is "CI"

   //gas layer
   KVMaterial* mat = new KVMaterial("C3F8", thick, pressure * KVUnits::mbar);
   AddAbsorber(mat);
   SetActiveLayer(mat);         //gas is the active layer
   // mylar for second window
   mat = new KVMaterial("Myl", 2.5 * KVUnits::um);
   AddAbsorber(mat);

   SetType("CI");
   SetLabel("CI");
   init();
}

//____________________________________________________________________________________________

KVChIo::~KVChIo()
{
}

//____________________________________________________________________________________________
void KVChIo::SetMylarThicknesses(Float_t thickF, Float_t thickB)
{
   if (thickF > 0.)((KVMaterial*)fAbsorbers->At(0))->SetThickness(thickF * KVUnits::um);
   if (thickF > 0.)((KVMaterial*)fAbsorbers->At(2))->SetThickness(thickB * KVUnits::um);
}

//void KVChIo::SetACQParams()
//{
//   //Setup acquistion parameters for this ChIo.
//   //Do not call before ChIo name has been set.

//   AddACQParamType("GG");
//   AddACQParamType("PG");
//   AddACQParamType("T");

//}

//__________________________________________________________________________________________________________________________

Double_t KVChIo::GetELossMylar(UInt_t z, UInt_t a, Double_t egas, Bool_t stopped)
{
   //Based on energy loss in gas, calculates sum of energy losses in mylar windows
   //from energy loss tables.
   //If argument 'egas' not given, KVChIo::GetEnergy() is used.
   //if stopped=kTRUE, we give the correction for a particle which stops in the detector
   //(by default we assume the particle continues after the detector)
   //
   // WARNING: if stopped=kFALSE, and if the residual energy after the detector
   //   is known (i.e. measured in a detector placed after this one), you should
   //   first call
   //       SetEResAfterDetector(Eres);
   //   before calling this method. Otherwise, especially for heavy ions, the
   //   correction may be false for particles which are just above the punch-through energy.

   egas = ((egas < 0.) ? GetEnergy() : egas);
   if (egas <= 0.)
      return 0.0;               //check measured (calibrated) energy in gas is reasonable (>0)

   KVNucleus tmp(z, a);
   Double_t emylar = GetCorrectedEnergy(&tmp, egas, !stopped) - egas;
   return emylar;
}

//______________________________________________________________________________

//Short_t KVChIo::GetCalcACQParam(KVACQParam*, Double_t) const
//{
//   // Calculates & returns value of given acquisition parameter corresponding to
//   // given calculated energy loss in the detector
//   // Returns -1 if detector is not calibrated

//   AbstractMethod("GetCalcACQParam");
//   return -1;
//}

//______________________________________________________________________________

void KVChIo::DeduceACQParameters(KVEvent*, KVNumberList&)
{

   AbstractMethod("DeduceACQParameters");
//   Double_t volts = GetVoltsFromEnergy(GetEnergy());
//   Int_t cipg = (Int_t)GetCanalPGFromVolts(volts);
//   Int_t cigg = (Int_t)GetCanalGGFromVolts(volts);
//   //cout << "chio: pg = " << cipg << " gg = " << cigg << endl;
//   GetACQParam("PG")->SetData((UShort_t)TMath::Min(4095, cipg));
//   GetACQParam("GG")->SetData((UShort_t)TMath::Min(4095, cigg));
//   GetACQParam("T")->SetData(110);

}
