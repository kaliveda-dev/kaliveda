//Created by KVClassFactory on Fri Nov  2 14:11:36 2012
//Author: Guilain ADEMARD
//// modified Feb. 20 2014 Marie-France Rivet

#include "KVIDSi75SiLi_e494s.h"
#include "KVIdentificationResult.h"
#include "KVDataSet.h"
#include "KVEBYEDAT_ACQParam.h"


ClassImp(KVIDSi75SiLi_e494s)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVIDSi75SiLi_e494s</h2>
<h4>E503/E494S experiment INDRA identification using Si75-SiLi matrices</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVIDSi75SiLi_e494s::KVIDSi75SiLi_e494s()
{
   // Default constructor
}

//________________________________________________________________

KVIDSi75SiLi_e494s::~KVIDSi75SiLi_e494s()
{
   // Destructor
}

//__________________________________________________________________________//

void KVIDSi75SiLi_e494s::Initialize()
{
   // Initialisation of telescope before identification.
   // This method MUST be called once before any identification is attempted.
   // IsReadyForID() will return kTRUE if KVTGID objects are associated
   // to this telescope for the current run.

   fSi75  = (KVINDRADetector*)GetDetector(1);
   fSiLi  = (KVINDRADetector*)GetDetector(2);

   Bool_t ok = fSi75 && fSiLi && GetListOfIDFunctions().GetEntries();
   if (ok) {
      KVTGID* idf = (KVTGID*)GetListOfIDFunctions().First();
      fThresholdX = GetThesholdFromVar(idf->GetVarX());
      fThresholdY = GetThesholdFromVar(idf->GetVarY());
//    Info("Initialize","Thresholds for %s: X= %f, Y= %f", GetName(), fThresholdX, fThresholdY);
   }

   SetBit(kReadyForID, ok);
}

//__________________________________________________________________________//

Double_t KVIDSi75SiLi_e494s::GetIDMapX(Option_t* opt)
{
   // This method gives the X-coordinate in a 2D identification map
   // associated with the Si75-SiLi identification telescope.
   // The X-coordinate is the SiLi current low gain coder data minus the
   // low gain pedestal correction (see KVACQParam::GetDeltaPedestal()).
   // If the high gain coder data is less than 3900 the low gain value
   // is calculated from the current high gain coder data minus the high
   // gain pedestal correction (see KVINDRADetector::GetPGfromGG()).


   return GetIDMapXY(fSiLi, opt);
}

//__________________________________________________________________________//

Double_t KVIDSi75SiLi_e494s::GetIDMapY(Option_t* opt)
{
   // This method gives the Y-coordinate in a 2D identification map
   // associated with the Si75-SiLi identification telescope.
   // The Y-coordinate is the Si75 current low gain coder data minus the
   // low gain pedestal correction (see KVACQParam::GetDeltaPedestal()).
   // If the high gain coder data is less than 3900 the low gain value
   // is calculated from the current high gain coder data minus the high
   // gain pedestal correction (see KVINDRADetector::GetPGfromGG()).

   return GetIDMapXY(fSi75, opt);
}

//__________________________________________________________________________//

Double_t KVIDSi75SiLi_e494s::GetIDMapXY(KVINDRADetector* det, Option_t* opt)
{
   IGNORE_UNUSED(opt);

   if (det->GetGG() <= 3900.5)
      return det->GetPGfromGG(det->GetGG() - det->GetACQParam("GG")->GetDeltaPedestal());

   return det->GetPG() - det->GetACQParam("PG")->GetDeltaPedestal();
}
//__________________________________________________________________________//

Bool_t KVIDSi75SiLi_e494s::Identify(KVIdentificationResult* IDR, Double_t x, Double_t y)
{
   //Identification of particles using SiCorrelated-CsI matrices for E503/E494s
   //First of all, Z identification is attempted with KVIDSi75SiLi_e494s::IdentZ.
   //If successful, if this telescope has mass identification capabilities
   //(HasMassID()=kTRUE), then if the identified Z is not too large for the
   //GG grid Z&A identification, we identify the mass of the particle.
   //In this case, a further check is made that the resulting nucleus is not
   //too bizarre, i.e. it must be in the KVNucleus mass table (KVNucleus::IsKnown = kTRUE).
   //If not, or if mass identification is not possible, the mass of the nucleus
   //is deduced from Z using the default mass formula of class KVNucleus.
   //
   // Note that optional arguments (x,y) for testing identification are not used.
   // We use the presence of the MT to check that SiLi is really stopping.
   // MFR march 2014

   Double_t X = (x < 0. ? GetIDMapX() : x);
   Double_t Y = (y < 0. ? GetIDMapY() : y);

   Double_t funLTG_Z = -1;
   Double_t funLTG_A = -1;
   Double_t mass     = -1;
   Double_t Z        = -1.;

   Int_t ia = -1;
   Int_t iz = -1;

   IDR->SetIDType(GetType());
   IDR->IDattempted = kTRUE;
   IDR->IDquality   = 15;

   // set general ID code
   IDR->IDcode = GetIDCode();

   KVEBYEDAT_ACQParam* pmt = (KVEBYEDAT_ACQParam*)fSiLi->GetACQParam("T");  // MFR
   Short_t MTSiLi = pmt->GetCoderData();                    // MFR

   const Bool_t inRange = (fThresholdX < X) && (X < 4090.) && (fThresholdY < Y) && (Y < 4090.) && (MTSiLi > 0); // MFR

   if (inRange) Z = IdentZ(GetName(), X, Y, funLTG_Z, ""); //Z = IdentZ(this, funLTG_Z, "", "");
   else {
      IDR->IDOK = kFALSE;                 // MFR
      return kFALSE;
   }

   //use KVTGIDManager::GetStatus value for IdentZ as identification subcode
   IDR->IDquality = GetStatus();

   if (GetStatus() == KVTGID::kStatus_NotBetween_IDMin_IDMax) return kFALSE;

   if (GetStatus() != KVTGIDManager::kStatus_OK) return kFALSE;   // no ID

   iz = TMath::Nint(Z);

   if (iz < 1) {
      IDR->IDquality = KVTGIDManager::kStatus_ZtooSmall;
      return kFALSE;
   }

   IDR->IDOK = kTRUE;

   //is mass identification a possibility ?
   if (iz < 9) {

      mass = IdentA(GetName(), X, Y, funLTG_A, "", iz); //IdentA(this, funLTG_A, "", "", iz);

      if (GetStatus() != KVTGIDManager::kStatus_OK) {    //mass ID not good ?

         //only Z identified
         IDR->Z = iz;
         IDR->PID = Z;
         IDR->Zident = kTRUE;
         //subcode says "Z ok but A failed because..."
         IDR->IDquality = GetStatus();
      }
      else {                     //both Z and A successful ?

         ia = TMath::Nint(mass);
         // fix 16B
         if (iz == 5 && ia == 16) ia = (mass > 16.0 ? 17 : 15);
         // fix 9B
         if (iz == 5 && ia == 9) ia = (mass > 9.0 ? 10 : 8);
         // fix 8Be
         if (iz == 4 && ia == 8) ia = (mass > 8.0 ? 9 : 7);
         // fix 13Be
         if (iz == 4 && ia == 13) ia = (mass > 13.0 ? 14 : 12);
         // fix 10Li
         if (iz == 3 && ia == 10) ia = (mass > 10.0 ? 11 : 9);
         // fix 5He
         if (iz == 2 && ia == 5) ia = (mass > 5.0 ? 6 : 4);
         // fix 7He
         if (iz == 2 && ia == 7) ia = (mass > 7.0 ? 8 : 6);
         // check that mass is not too bizarre
         KVNucleus tmp(iz, ia);
         if (!tmp.IsKnown()) {
            // if ia seems too light, we try with iz-1
            // if ia seems too heavy, we try with iz+1
            Info("Identify", "%s : initial ID Z=%d A=%d funLTG=%g", GetName(), iz, ia, funLTG_A);
            Int_t iz2 = (ia < 2 * iz ? iz - 1 : iz + 1);
            if (iz2 > 0) {
               Double_t old_funLTG_A = funLTG_A;
               Double_t new_mass = IdentA(GetName(), X, Y, funLTG_A, "", iz2); //IdentA(this, funLTG_A, "", "", iz2);
               // is this a better solution ?
               if (GetStatus() == KVTGIDManager::kStatus_OK) {
                  Int_t new_ia = TMath::Nint(new_mass);
                  tmp.SetZ(iz2);
                  tmp.SetA(new_ia);
                  Info("Identify", "%s : new ID Z=%d A=%d funLTG=%g", GetName(), iz2, new_ia, funLTG_A);
                  if (tmp.IsKnown() && (TMath::Abs(funLTG_A) < TMath::Abs(old_funLTG_A))) {
                     // new nucleus is known and point is closer to line
                     iz = iz2;
                     ia = new_ia;
                     mass = new_mass;
                     Info("Identify", "%s : accepted new ID", GetName());
                  }
                  else {
                     Info("Identify", "%s : rejected new ID", GetName());
                  }
               }
               else {
                  Info("Identify", "%s : failed to obtain new ID with Z=%d", GetName(), iz2);
               }
            }
         }
         IDR->Z = iz;
         IDR->A = ia;
         IDR->PID = mass;
         IDR->Zident = kTRUE;
         IDR->Aident = kTRUE;
      }
   }
   else {
      //only Z identified
      //ID subcode remains 'Zstatus'
      IDR->Z = iz;
      IDR->PID = Z;
      IDR->Zident = kTRUE;

   }

   return kTRUE;
}
//__________________________________________________________________________//

Bool_t KVIDSi75SiLi_e494s::SetIdentificationParameters(const KVMultiDetArray*)
{
   //Initialise the identification parameters (grids, etc.) of ALL identification telescopes of this
   //kind (label) in the multidetector array. Therefore this method need only be called once, and not
   //called for each telescope. The kind/label (returned by GetLabel) of the telescope corresponds
   //to the URI used to find the plugin class in $KVROOT/KVFiles/.kvrootrc.
   //Parameters are read from the file with name given by the environment variable
   //INDRA_camp5.IdentificationParameterFile.SI-CSI:       [filename]

   TString filename = gDataSet->GetDataSetEnv(Form("IdentificationParameterFile.%s", GetLabel()));
   if (filename == "") {
      Warning("SetIdentificationParameters",
              "No filename defined. Should be given by %s.IdentificationParameterFile.%s",
              gDataSet->GetName(), GetLabel());
      return kFALSE;
   }
   TString path;
   if (!SearchKVFile(filename.Data(), path, gDataSet->GetName())) {
      Error("SetIdentificationParameters",
            "File %s not found. Should be in $KVROOT/KVFiles/%s",
            filename.Data(), gDataSet->GetName());
      return kFALSE;
   }
   //read grids from file
   Info("SetIdentificationParameters", "Using file %s", path.Data());
   return ReadAsciiFile(path.Data());
}
//__________________________________________________________________________//

void KVIDSi75SiLi_e494s::RemoveIdentificationParameters()
{
   //Delete any KVTGID objects associated with this identification telescope
   RemoveAllTGID();
}
//__________________________________________________________________________//

Double_t KVIDSi75SiLi_e494s::GetThesholdFromVar(const Char_t* var)
{
   // returns the threshold deduced from VarX or VarY.
   // If the string 'var' contains the character '>', we consider
   // that the threshold is given in the string after this character.

   TString thresh = var;
   Int_t idx = thresh.Index(">");
   if (idx > -1) {
      thresh.Remove(0, idx + 1);
      return (thresh.IsFloat() ? thresh.Atof() : 0.);
   }
   return 0.;
}
