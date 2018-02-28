/***************************************************************************
                          KVIDSiCsI5.cpp  -  description
                             -------------------
    begin                : Fri Feb 20 2004
    copyright            : (C) 2004 by J.D. Frankland
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

#include "KVIDSiCsI5.h"
#include "KVINDRACodeMask.h"
#include "Riostream.h"
#include "KVTGIDZA.h"
#include "TObjString.h"
#include "KVIdentificationResult.h"
#include "KVDataSet.h"
#include "KVMultiDetArray.h"

using namespace std;

ClassImp(KVIDSiCsI5)
/////////////////////////////////////////////////////////////////////////////////////
//KVIDSiCsI5
//
//Identification in Si(GG/PG)-CsI(lumiere) matrices of INDRA 5th campaign.
//(Mathieu Pichon & Josquin Marie)
//
//For each telescope, at least 2 grids were used :
//      GG      identification using Si canal GG
//      PG1     identification using Si canal PG
//For certain telescopes, a second PG grid (PG2) is available, used for
//large Z identification.
//Each of the three grids has an associated maximum and minimum Z (GetZmax, GetZmin)
//corresponding to the lines drawn by hand used for the fit.
//Z identification is considered possible between max(Zmin-0.5, 0.5) and min(Zmax+0.5, 100.5)
//(Zmax is taken equal to 100 when using the highest Z grid of the telescope (i.e. either
//PG1 or PG2).
//In addition certain telescopes have both Z and A identification for the SiGG grid.
//In this case KVIDSiCsI5::HasMassID=kTRUE.
//
//All three grids were fitted using the polynomials tassangot_Z and tassangot_A
//defined in KVTGIDFunctions.
//
//ID SUB-CODES:
//-------------
// 0  KVIDSiCsI5::k_OK_GG            "ok in GG",
// 1  KVIDSiCsI5::k_noTGID_GG        "no Z identification available - last grid tried GG",
// 2  KVIDSiCsI5::k_OutOfIDRange_GG  "point outside of range of possible Z identification in GG",
// 3  KVIDSiCsI5::k_OK_PG1           "ok in PG1",
// 4  KVIDSiCsI5::k_noTGID_PG1       "no Z identification available - last grid tried PG1",
// 5  KVIDSiCsI5::k_OutOfIDRange_PG1 "point outside of range of possible Z identification in PG1",
// 6  KVIDSiCsI5::k_OK_PG2           "ok in PG2",
// 7  KVIDSiCsI5::k_noTGID_PG2       "no Z identification available - last grid tried PG2",
// 8  KVIDSiCsI5::k_OutOfIDRange_PG2 "point outside of range of possible Z identification in PG2",
// 9  KVIDSiCsI5::kZOK_AnoTGID       "Z ok, A identification tried but not available",
//10  KVIDSiCsI5::kZOK_AOutOfIDRange "Z ok, A identification tried but point to identify outside of identification range of function",
//11  KVIDSiCsI5::kZOK_A_ZtooSmall   "Z ok, A identification tried but IdentA called with Z<1",
//12  KVIDSiCsI5::kZOK_A_ZtooLarge   "Z ok, A identification tried but IdentA called with Z larger than max Z defined for KVTGIDZA isotopic identification object"
/////////////////////////////////////////////////////////////////////////////////////
Double_t KVIDSiCsI5::IdentifyZ(Double_t& funLTG)
{
   //Identify Z using the following method:
   //      - first try GG matrix
   //      if this doesn't work, or if the Z found is greater than the maximum
   //      Z for GG....
   //      - try PG1 matrix
   //      if this doesn't work, or if the Z found is greater than the maximum
   //      Z for PG1....
   //      - try PG2 matrix (if it exists)

   Double_t Z = -1.;

   if (((KVSilicon*) GetDetector(1))->GetGG() < 4090) {
      //try GG identification if SIGG is not saturated
      //the condition is applied directly to the raw channel number instead of GetIDMapY()
      //so that it is correct even if GetIDMapY rescales for the change in gain
      Z = IdentZ(GetName(), GetIDMapX("GG"), GetIDMapY("GG"), funLTG, "GG");//IdentZ(this, funLTG, "GG", "GG");
      fWhichGrid = k_GG;
   }

   if (Z < 0.) {

      Z = IdentZ(GetName(), GetIDMapX("PG"), GetIDMapY("PG"), funLTG, "PG1"); //IdentZ(this, funLTG, "PG1", "PG");
      fWhichGrid = k_PG1;

   } else {
      //successful GG identification
      return Z;
   }

   if (!HasPG2()) {
      //no 2nd PG fit - this is the end
      return Z;
   }

   if (Z < 0.) {

      Z = IdentZ(GetName(), GetIDMapX("PG"), GetIDMapY("PG"), funLTG, "PG2"); //IdentZ(this, funLTG, "PG2", "PG");
      fWhichGrid = k_PG2;

   }

   return Z;
}

//______________________________________________________________________________
Bool_t KVIDSiCsI5::Identify(KVIdentificationResult* IDR, Double_t, Double_t)
{
   //Identification of particles using Si(300)-CsI matrices for 5th campaign.
   //First of all, Z identification is attempted with KVIDSiCsI5::IdentZ.
   //If successful, if this telescope has mass identification capabilities
   //(HasMassID()=kTRUE), then if the identified Z is not too large for the
   //GG grid Z&A identification, we identify the mass of the particle.
   //In this case, a further check is made that the resulting nucleus is not
   //too bizarre, i.e. it must be in the KVNucleus mass table (KVNucleus::IsKnown = kTRUE).
   //If not, or if mass identification is not possible, the mass of the nucleus
   //is deduced from Z using the default mass formula of class KVNucleus.
   //
   // Note that optional arguments (x,y) for testing identification are not used.

   Double_t funLTG_Z = -1;
   Double_t funLTG_A = -1;
   Double_t mass = -1;
   Int_t ia = -1;
   Int_t iz = -1;

   IDR->SetIDType(GetType());
   IDR->IDattempted = kTRUE;
   Double_t Z = IdentifyZ(funLTG_Z);

   //use KVTGIDManager::GetStatus value for IdentZ as identification subcode
   Int_t Zstatus = (GetStatus() + 3 * fWhichGrid);
   IDR->IDquality = Zstatus;

   if (GetStatus() != KVTGIDManager::kStatus_OK)
      return kFALSE;            // no ID

   iz = TMath::Nint(Z);

   IDR->IDOK = kTRUE;

   //is mass identification a possibility ?
   if (HasMassID() && Zstatus == k_OK_GG) {     //only in GG...

      mass = IdentA(GetName(), GetIDMapX("GG"), GetIDMapY("GG"), funLTG_A, "GG", iz); //IdentA(this, funLTG_A, "GG", "GG", iz);

      if (GetStatus() != KVTGIDManager::kStatus_OK) {   //mass ID not good ?
         //only Z identified
         IDR->Z = iz;
         IDR->PID = Z;
         IDR->Zident = kTRUE;
         //subcode says "Z ok but A failed because..."
         IDR->IDquality = k_OutOfIDRange_PG2 + GetStatus();
      } else {                //both Z and A successful ?
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
               Double_t new_mass = IdentA(GetName(), GetIDMapX("GG"), GetIDMapY("GG"), funLTG_A, "GG", iz2); //IdentA(this, funLTG_A, "GG", "GG", iz2);
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
                  } else {
                     Info("Identify", "%s : rejected new ID", GetName());
                  }
               } else {
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
   } else {
      //only Z identified
      //ID subcode remains 'Zstatus'
      IDR->Z = iz;
      IDR->PID = Z;
      IDR->Zident = kTRUE;
   }

   // set general ID code
   IDR->IDcode = kIDCode3;
   return kTRUE;
}

//__________________________________________________________________________//

void KVIDSiCsI5::Print(Option_t*) const
{
   cout << "KVIDSiCsI5 : " << GetName() << endl;
   if (GetListOfIDFunctions().GetSize() > 0) {
      if (HasMassID()) {
         cout << "  A and Z identification" << endl;
      } else {
         cout << "  Z identification only" << endl;
      }
      cout << "Identification objects :" << endl;
      GetListOfIDFunctions().Print();
   }
}

//__________________________________________________________________________//

Double_t KVIDSiCsI5::GetIDMapX(Option_t*)
{
   //Calculates current X coordinate for identification.
   //It is the CsI detector's total light output calculated from current values of 'R' and 'L'
   //raw data without pedestal correction (because identification maps were drawn without
   //correcting).
   //'opt' has no effect.

   Double_t rapide = (Double_t)fCsI->GetR() + fCsIRPedestal;
   Double_t lente = (Double_t)fCsI->GetL() + fCsILPedestal;
   Double_t h = (Double_t)fCsI->GetLumiereTotale(rapide, lente);
   return h;
}
//__________________________________________________________________________//

Double_t KVIDSiCsI5::GetPedestalX(Option_t*)
{
   //Returns pedestal of X coordinate for identification.
   //It is the CsI detector's total light output calculated values of 'R' and 'L'
   //equal to their pedestal values (because identification maps were drawn without
   //correcting for pedestals).
   //'opt' has no effect.

   Double_t rapide = fCsIRPedestal;
   Double_t lente = fCsILPedestal;
   Double_t h = (Double_t)fCsI->GetLumiereTotale(rapide, lente);
   return h;
}

//____________________________________________________________________________________

void KVIDSiCsI5::Initialize()
{
   // Initialisation of telescope before identification.
   // This method MUST be called once before any identification is attempted.
   // IsReadyForID() will return kTRUE if KVTGID objects are associated
   // to this telescope for the current run.

   fSi = (KVSilicon*)GetDetector(1);
   fSiPGPedestal = fSi->GetPedestal("PG");
   fSiGGPedestal = fSi->GetPedestal("GG");
   fSiGain = fSi->GetGain();
   fCsI = (KVCsI*)GetDetector(2);
   fCsIRPedestal = fCsI->GetPedestal("R");
   fCsILPedestal = fCsI->GetPedestal("L");
   if (GetListOfIDFunctions().GetEntries()) {
      SetBit(kReadyForID);
   } else
      ResetBit(kReadyForID);
}

//__________________________________________________________________________//

Double_t KVIDSiCsI5::GetIDMapY(Option_t* opt)
{
   //Calculates current Y coordinate for identification.
   //It is the silicon detector's current grand gain (by default, i.e. opt="", or if opt="GG")
   //or petit gain (opt="PG") coder data, without pedestal correction.
   //We include a "correction" for the gain of the Silicon amplifier:
   //this was set to 1.0 during the runs for which the identification grids were drawn;
   //when it increases to 1.41 we simply scale the data down by the same factor.

   Double_t si, si_ped;
   if (!strcmp(opt, "PG")) {
      si = (Double_t)fSi->GetPG();
      si_ped = fSiPGPedestal;
   } else {
      si = (Double_t)fSi->GetGG();
      si_ped = fSiGGPedestal;
   }
   //gain "correction"
   if (fSiGain > 1.1) si = (si - si_ped) / fSiGain + si_ped;
   return si;
}

//__________________________________________________________________________//

Double_t KVIDSiCsI5::GetPedestalY(Option_t* opt)
{
   //Calculates current Y coordinate pedestal for identification.
   //It is the silicon detector's current grand gain (by default, i.e. opt="", or if opt="GG")
   //or petit gain (opt = "PG") pedestal.

   Double_t si_ped = 0.;
   if (!strcmp(opt, "PG")) {
      si_ped = fSiPGPedestal;
   } else {
      si_ped = fSiGGPedestal;
   }
   return si_ped;
}



//____________________________________________________________________________________

Bool_t KVIDSiCsI5::SetIdentificationParameters(const KVMultiDetArray* MDA)
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
   Info("SetIdentificationParameters", "Using file %s", path.Data());
   ifstream datfile;
   datfile.open(path.Data());
   KVString aline;
   int zmin, zmax;
   char type[5], name[20];
   KVIDSiCsI5* idt = 0;

   aline.ReadLine(datfile);
   while (datfile.good()) {

      if (!aline.BeginsWith('#')) {     //skip comments in header

         if (aline.BeginsWith("+")) {

            aline.Remove(0, 2);
            sscanf(aline.Data(), "%s", name);
            idt = (KVIDSiCsI5*) MDA->GetIDTelescope(name);

            aline.ReadLine(datfile);
            Bool_t _Aident = kFALSE;
            if (aline.BeginsWith("MASS_ID")) {
               if (idt) idt->SetHasMassID();
               _Aident = kTRUE;
               aline.Remove(0, 8);
            }
            sscanf(aline.Data(), "%s", type);

            if (!strcmp(type, "PG2")) {
               if (idt) idt->SetHasPG2();
            }

            aline.ReadLine(datfile);
            sscanf(aline.Data(), "ZMIN=%d  ZMAX=%d", &zmin, &zmax);

            //create new Tassan-Got ID object for telescope
            KVTGID* _tgid = 0, *_tgid2 = 0;
            if (idt) {
               if (_Aident) {
                  //When mass ID is possible, have to create two objects: one
                  //for Z ident and the other for A ident
                  //A-identification functional. Set name of parameter "Z"
                  _tgid =
                     new KVTGIDZA(idt->
                                  GetTGIDName(idt->GetName(), "A", type),
                                  "tassangot_A", 0.1, 100., 13, 11, 12);
                  _tgid->SetParName(10, "Z");
                  _tgid2 =
                     new KVTGIDZ(idt->GetTGIDName(idt->GetName(), "Z", type),
                                 "pichon_Z", 0.1, 100., 12, 10, 11);
               } else {
                  //Z-identification functional.
                  _tgid =
                     new KVTGIDZ(idt->GetTGIDName(idt->GetName(), "Z", type),
                                 "pichon_Z", 0.1, 100., 12, 10, 11);
               }

               _tgid->SetIDmin((Double_t) zmin);
               _tgid->SetIDmax((Double_t) zmax);
               if (_Aident) {
                  _tgid2->SetIDmin((Double_t) zmin);
                  _tgid2->SetIDmax((Double_t) zmax);
               }
            }
            //read line with parameters on it from file
            aline.ReadLine(datfile);
            if (idt) {
               unique_ptr<TObjArray> par_arr(aline.Tokenize(" "));   //split up into 1 string per parameter
               for (int i = 0; i < par_arr->GetEntries(); i++) {
                  //read Double_t value in string
                  KVString kvs(((TObjString*)(*par_arr)[i])->String());
                  Double_t param = kvs.Atof();
                  //set parameter of ID functional
                  _tgid->SetParameter(i, param);
                  if (_Aident)
                     _tgid2->SetParameter(i, param);
               }
               //add identification object to telescope's ID manager
               idt->AddTGID(_tgid);
               if (_Aident)
                  idt->AddTGID(_tgid2);
            }
         }
      }
      aline.ReadLine(datfile);
   }                            //while( datfile.good()
   datfile.close();
   return kTRUE;
}

//____________________________________________________________________________________

void KVIDSiCsI5::RemoveIdentificationParameters()
{
   //Delete any KVTGID objects associated with this telescope
   RemoveAllTGID();
   SetHasMassID(kFALSE);
   SetHasPG2(kFALSE);
}

Double_t KVIDSiCsI5::GetMeanDEFromID(Int_t& status, Int_t Z, Int_t A, Double_t Eres)
{
   // Returns the Y-axis value in the 2D identification map containing isotope (Z,A)
   // corresponding to either the given X-axis/Eres value or the current X-axis value given by GetIDMapX.
   // If no mass information is available, just give Z.
   //
   // This method overrides KVIDTelescope::GetMeanDEFromID, as here we have
   // to handle the LTG fits used for identification. This means scanning the TGID objects associated with
   // this telescope until we find one with the right Z range, and then calculating
   // the Y-coordinate for the current X-coordinate value.
   //
   // Status is same as for KVIDTelescope::GetMeanDEFromID.

   status = kMeanDE_OK;
   if (!HasMassID()) {
      // mass identification not possible for telescope - ignore A
      A = -1;
   }
   Double_t x = (Eres < 0 ? GetIDMapX() : Eres);
   // loop over TGID objects
   TIter next(&GetListOfIDFunctions());
   KVTGID* tgid;
   while ((tgid = (KVTGID*)next())) {
      if (Z >= tgid->GetIDmin() && Z <= tgid->GetIDmax()) {
         if (A > 0 && tgid->GetZorA()) {
            // this is a Z-only identification object & we require isotopic resolution
            continue;
         }
         if (A > 0 && !tgid->GetZorA()) {
            // get position on isotopic line (Z,A)
            Double_t y = 0;
            tgid->SetParameter("Z", Z);
            y = tgid->GetDistanceToLine(x, y, A);
            return y;
         }
         if (A == -1 && !tgid->GetZorA()) {
            // this is an A-identifying object, but we only care about Z
            continue;
         }
         if (A < 0 && tgid->GetZorA()) {
            // get position on Z line
            Double_t y = 0;
            y = tgid->GetDistanceToLine(x, y, Z);
            return y;
         }
      }
   }
   status = kMeanDE_NoIdentifier;
   return -1.0;
}
