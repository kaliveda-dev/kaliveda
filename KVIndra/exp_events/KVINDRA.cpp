/***************************************************************************
$Id: KVINDRA.cpp,v 1.68 2009/01/21 10:05:51 franklan Exp $
                          kvindra.cpp  -  description
                             -------------------
    begin                : Mon May 20 2002
    copyright            : (C) 2002 by J.D. Frankland
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

#include "Riostream.h"
#include "KVINDRA.h"
#include "KVMaterial.h"
#include "KVDetector.h"
#include "KVTelescope.h"
#include "KVIDTelescope.h"
#include "KVIDSiCsI.h"
#include "KVIDCsI.h"
#include "KVIDChIoSi.h"
#include "KVIDChIoCsI.h"
#include "KVIDChIoSi75.h"
#include "KVIDSi75SiLi.h"
#include "KVIDSiLiCsI.h"
#include "KVIDPhoswich.h"
#include "KVLayer.h"
#include "KVRing.h"
#include "KVGroup.h"
#include "KVSilicon.h"
#include "KVChIo.h"
#include "KVCsI.h"
#include "KVPhoswich.h"
#include "KVNucleus.h"
#include "KVDetectorEvent.h"
#include "KVINDRAReconEvent.h"
#include "TEnv.h"
#include "KVFileReader.h"
#include "INDRAGeometryBuilder.h"
#include <KVASGroup.h>
#include "TROOT.h"
#include "KVGeoNavigator.h"
#include <KVGeoImport.h>
#include <KVRangeTableGeoNavigator.h>
#include <KVRawDataReader.h>
#ifdef WITH_MFM
#include "KVMFMDataFileReader.h"
#include "MFMEbyedatFrame.h"
#endif

using namespace std;

ClassImp(KVINDRA)

//Use this static array to translate EBaseIndra_type
//signal type to a string giving the signal type
Char_t KVINDRA::SignalTypes[][3] = {
   "",                          //dummy for index=0
   "GG", "PG", "T",
   "GG", "PG", "T",
   "R", "L", "T",
   "GG", "PG", "T",
   "GG", "PG", "T"
};

#define KV_DEBUG 1

KVINDRA* gIndra;
//_________________________________________________________________________________
KVINDRA::KVINDRA()
{
   //Default constructor
   //Set up lists of ChIo, Si, CsI, Phoswich
   fChIo = new KVHashList();
   fChIo->SetCleanup(kTRUE);
   fSi = new KVHashList();
   fSi->SetCleanup(kTRUE);
   fCsI = new KVHashList();
   fCsI->SetCleanup(kTRUE);
   fPhoswich = new KVHashList();
   fPhoswich->SetCleanup(kTRUE);
   fTrigger = 0;
   gIndra = this;
   fPHDSet = kFALSE;
   fSelecteur = 0;
   // unlike a normal multidetector, INDRA does not own its detectors
   // they are owned by the TELESCOPE structures
   SetOwnsDetectors(kFALSE);
}

//_________________________________________________________________________________
KVINDRA::~KVINDRA()
{
   //Delets lists of ChIo, Si, etc.
   //Resets global gIndra pointer.

   if (fChIo && fChIo->TestBit(kNotDeleted)) {
      fChIo->Clear();
      delete fChIo;
   }
   fChIo = 0;
   if (fSi && fSi->TestBit(kNotDeleted)) {
      fSi->Clear();
      delete fSi;
   }
   fSi = 0;
   if (fCsI && fCsI->TestBit(kNotDeleted)) {
      fCsI->Clear();
      delete fCsI;
   }
   fCsI = 0;
   if (fPhoswich && fPhoswich->TestBit(kNotDeleted)) {
      fPhoswich->Clear();
      delete fPhoswich;
   }
   fPhoswich = 0;
   if (fSelecteur) delete fSelecteur;
   gIndra = 0;
}

//_________________________________________________________________________________

void KVINDRA::BuildGeometry()
{
   // Construction of INDRA detector array.
   //
   // Uses infos in file
   //        $KVROOT/KVFiles/data/indra_struct.[dataset].env
   // or
   //        $KVROOT/KVFiles/data/indra_struct.env
   //
   // if no dataset-specific file found
   //
   // Alternatively, by defining the variable
   //
   //   [dataset].INDRA.StructureFile: [path to file]


   TString path = Form("indra-struct.%s.env", fDataSet.Data());
   TString path2 = GetDataSetEnv(fDataSet, "INDRA.StructureFile", "");
   if (path2 == "") {
      if (!SearchKVFile(path.Data(), path2, "data")) {
         path = "indra-struct.env";
         SearchKVFile(path.Data(), path2, "data");
      }
   }
   fStrucInfos.ReadFile(path2, kEnvAll);

   KVString lruns = fStrucInfos.GetValue("AddOnForRuns", "");
   //test if additional geometrical specification exists
   if (lruns != "") {
      lruns.Begin(",");
      while (!lruns.End()) {
         KVString sruns = lruns.Next();
         KVNumberList nlr(sruns.Data());
         //the current run needs specific geometry
         if (nlr.Contains(fCurrentRun)) {
            path = fStrucInfos.GetValue(sruns.Data(), "");
            Info("BuildGeometry", "Additional geometry for run=%d in file #%s#", fCurrentRun, path.Data());
            SearchKVFile(path.Data(), path2, "data");
            if (path2 == "") {
               Warning("BuildGeometry", "fichier %s inconnu", path.Data());
            }
            else {
               fStrucInfos.ReadFile(path2, kEnvChange);
            }
         }
      }
   }

   SetName(fStrucInfos.GetValue("INDRA.Name", ""));
   SetTitle(fStrucInfos.GetValue("INDRA.Title", ""));

   KVString layers = fStrucInfos.GetValue("INDRA.Layers", "");
   layers.Begin(" ");
   while (!layers.End()) BuildLayer(layers.Next());
}

void KVINDRA::BuildLayer(const Char_t* name)
{
   // Build layer 'name' with infos in file "$KVROOT/KVFiles/data/indra-struct.[dataset].env"

   KVLayer* layer = new KVLayer;
   Int_t number = fStrucInfos.GetValue(Form("INDRA.Layer.%s.Number", name), 0);
   layer->SetName(name);
   layer->SetNumber(number);
   Add(layer);
   Info("BuildLayer", "Building layer %d:%s", number, name);
   KVNumberList rings = fStrucInfos.GetValue(Form("INDRA.Layer.%s.Rings", name), "");
   rings.Begin();
   while (!rings.End()) {
      Int_t ring_num = rings.Next();
      TString prefix = Form("INDRA.Layer.%s.Ring.%d", name, ring_num);
      KVRing* ring = BuildRing(ring_num, prefix);
      layer->Add(ring);
   }
}

KVRing* KVINDRA::BuildRing(Int_t number, const Char_t* prefix)
{
   // Build ring with infos in file "$KVROOT/KVFiles/data/indra-struct.[dataset].env"

   KVRing* ring = new KVRing;
   Info("BuildRing", "Building ring %d (%s)", number, prefix);

   Double_t thmin = fStrucInfos.GetValue(Form("%s.ThMin", prefix), 0.0);
   Double_t thmax = fStrucInfos.GetValue(Form("%s.ThMax", prefix), 0.0);
   Double_t phi_0 = fStrucInfos.GetValue(Form("%s.Phi0", prefix), 0.0);
   Int_t ntel = fStrucInfos.GetValue(Form("%s.Ntel", prefix), 0);
   Int_t step = fStrucInfos.GetValue(Form("%s.Step", prefix), 1);
   Int_t num_first = fStrucInfos.GetValue(Form("%s.Nfirst", prefix), 1);
   Double_t dphi = fStrucInfos.GetValue(Form("%s.Dphi", prefix), -1.0);
   KVNumberList absent = fStrucInfos.GetValue(Form("%s.Absent", prefix), "");

   ring->SetPolarMinMax(thmin, thmax);
   KVINDRATelescope* kvt = BuildTelescope(prefix, num_first);
   Double_t phi_min = phi_0 - 0.5 * (kvt->GetAzimuthalWidth());
   phi_min = (phi_min < 0. ? phi_min + 360. : phi_min);
   dphi = (dphi < 0. ? 360. / ntel : dphi);
   Double_t phi_max =
      phi_0 + ((ntel - 1.0) * dphi) + 0.5 * (kvt->GetAzimuthalWidth());
   phi_max = (phi_max > 360. ? phi_max - 360. : phi_max);
   ring->SetAzimuthalMinMax(phi_min, phi_max);
   ring->SetNumber(number);

   Double_t phi = phi_0;
   UInt_t counter = num_first;
   for (int i = 0; i < ntel; i++) {
      kvt->SetPolarMinMax(thmin, thmax);
      kvt->SetPhi(phi);
      phi = (phi + dphi > 360. ? (phi + dphi - 360.) : (phi + dphi));
      kvt->SetNumber(counter);
      if (!absent.Contains(counter)) ring->Add(kvt);
      else delete kvt;
      counter += step;
      if (i + 1 < ntel) kvt = BuildTelescope(prefix, counter);
   }
   ring->SetName(Form("RING_%d", number));
   return ring;
}

KVINDRATelescope* KVINDRA::BuildTelescope(const Char_t* prefix, Int_t module)
{
   // Build telescope from infos in file "$KVROOT/KVFiles/data/indra-struct.[dataset].env"

   //Info("BuildTelescope", "Building telescope %s",name);
   KVINDRATelescope* kvt = new KVINDRATelescope;
   KVString telescopes = fStrucInfos.GetValue(Form("%s.Telescope", prefix), "");
   telescopes.Begin(" ");
   KVString name;
   while (!telescopes.End()) {
      name = telescopes.Next();
      KVNumberList mods = fStrucInfos.GetValue(Form("%s.Telescope.%s.Modules", prefix, name.Data()), "1-100");
      if (mods.Contains(module)) break;
   }

   KVString detectors = fStrucInfos.GetValue(Form("INDRA.Telescope.%s.Detectors", name.Data()), "");
   Double_t aziwidth = fStrucInfos.GetValue(Form("INDRA.Telescope.%s.AziWidth", name.Data()), 1.0);
   Double_t dist = fStrucInfos.GetValue(Form("%s.Dist", prefix), 0.0);
   kvt->SetDistance(dist);
   detectors.Begin(" ");
   Int_t idet = 1;
   while (!detectors.End()) {
      KVString detector = detectors.Next();
      detector.Begin("()");
      KVString dettype = detector.Next();
      Double_t detthick = detector.Next().Atof();
      KVDetector* det = KVDetector::MakeDetector(Form("%s.%s", fDataSet.Data(), dettype.Data()), detthick);
      kvt->Add(det);
      Double_t depth = fStrucInfos.GetValue(Form("INDRA.Telescope.%s.Depth.%s", name.Data(), dettype.Data()), 0.);
      kvt->SetDepth(idet, depth);
      idet++;
   }
   kvt->SetAzimuthalWidth(aziwidth);
   return kvt;
}

void KVINDRA::FillTrajectoryIDTelescopeLists()
{
   // Kludge to make INDRA ROOT geometry work like any other
   // Normally ID telescopes are deduced from successive detectors on the different trajectories
   // Each trajectory then possesses its list of ID telescopes
   // These lists are then used when reconstruction trajectories are calculated
   //
   // When INDRA ROOT geometry is used, trajectory lists need to be filled
   // by hand before reconstruction trajectories are calculated

   TIter it_traj(GetTrajectories());
   KVGeoDNTrajectory* tr;
   while ((tr = (KVGeoDNTrajectory*)it_traj())) {
      // get first detector on trajectory
      KVDetector* det = tr->GetNodeAt(0)->GetDetector();
      // list of 'aligned' id telescopes
      TIter it_idt(det->GetAlignedIDTelescopes());
      KVIDTelescope* idt;
      while ((idt = (KVIDTelescope*)it_idt())) {
         if (tr->ContainsAll(idt->GetDetectors())) { // all detectors on trajectory
            if (idt->GetDetectors()->GetEntries() > 1) {
               // make sure dE and E detector are consecutive on trajectory
               // i.e. dE has to be immediately in front of E on the trajectory
               if (tr->GetNodeInFront(idt->GetDetector(2)->GetNode()) == idt->GetDetector(1)->GetNode())
                  tr->AccessIDTelescopeList()->Add(idt);
            }
            else
               tr->AccessIDTelescopeList()->Add(idt);//single-detector telescope
         }
      }
   }
}

//_________________________________________________________________________________________

void KVINDRA::Build(Int_t run)
{
   // Overrides KVASMultiDetArray::Build
   // Correspondance between CsI detectors and pin lasers is set up if known.
   //Correspondance between Si and ChIo detectors and nunmber of the QDC is made

   if (run != -1) {
      fCurrentRun = run;
   }
   BuildGeometry();

   MakeListOfDetectors();

   SetGroupsAndIDTelescopes();

   SetNamesOfIDTelescopes();

   //SetACQParams();

   SetDetectorThicknesses();

   SetIdentifications();

   //set flag to say Build() was called
   SetBit(kIsBuilt);

   SetPinLasersForCsI();

   //LinkToCodeurs();
}

//void KVINDRA::SetArrayACQParams()
//{
//   // Overrides KVASMultiDetArray::SetArrayACQParams() in order to
//   // add the following acquisition parameters which are not associated to a detector:
//   //
//   // STAT_EVE
//   // R_DEC
//   // CONFIG
//   // PILA_01_PG
//   // PILA_01_GG
//   // PILA_02_PG
//   // PILA_02_GG
//   // PILA_03_PG
//   // PILA_03_GG
//   // PILA_04_PG
//   // PILA_04_GG
//   // PILA_05_PG
//   // PILA_05_GG
//   // PILA_06_PG
//   // PILA_06_GG
//   // PILA_07_PG
//   // PILA_07_GG
//   // PILA_08_PG
//   // PILA_08_GG
//   // SI_PIN1_PG
//   // SI_PIN1_GG
//   // SI_PIN2_PG
//   // SI_PIN2_GG
//   //
//   // We also create and initialize the KVINDRATriggerInfo object (fSelecteur) used to read
//   // the status of the DAQ trigger event by event (access through GetTriggerInfo()).

//   KVACQParam* ste = new KVACQParam("STAT_EVE");
//   AddArrayACQParam(ste);
//   KVACQParam* dec = new KVACQParam("R_DEC");
//   AddArrayACQParam(dec);
//   KVACQParam* conf = new KVACQParam("CONFIG");
//   AddArrayACQParam(conf);
//   fSelecteur = new KVINDRATriggerInfo;
//   fSelecteur->SetSTAT_EVE_PAR(ste);
//   fSelecteur->SetR_DEC_PAR(dec);
//   fSelecteur->SetVXCONFIG_PAR(conf);

//   AddArrayACQParam(new KVACQParam("PILA_01_PG"));
//   AddArrayACQParam(new KVACQParam("PILA_01_GG"));
//   AddArrayACQParam(new KVACQParam("PILA_02_PG"));
//   AddArrayACQParam(new KVACQParam("PILA_02_GG"));
//   AddArrayACQParam(new KVACQParam("PILA_03_PG"));
//   AddArrayACQParam(new KVACQParam("PILA_03_GG"));
//   AddArrayACQParam(new KVACQParam("PILA_04_PG"));
//   AddArrayACQParam(new KVACQParam("PILA_04_GG"));
//   AddArrayACQParam(new KVACQParam("PILA_05_PG"));
//   AddArrayACQParam(new KVACQParam("PILA_05_GG"));
//   AddArrayACQParam(new KVACQParam("PILA_06_PG"));
//   AddArrayACQParam(new KVACQParam("PILA_06_GG"));
//   AddArrayACQParam(new KVACQParam("PILA_07_PG"));
//   AddArrayACQParam(new KVACQParam("PILA_07_GG"));
//   AddArrayACQParam(new KVACQParam("PILA_08_PG"));
//   AddArrayACQParam(new KVACQParam("PILA_08_GG"));
//   AddArrayACQParam(new KVACQParam("SI_PIN1_PG"));
//   AddArrayACQParam(new KVACQParam("SI_PIN1_GG"));
//   AddArrayACQParam(new KVACQParam("SI_PIN2_PG"));
//   AddArrayACQParam(new KVACQParam("SI_PIN2_GG"));
//}

//_________________________________________________________________________________________

void KVINDRA::MakeListOfDetectors()
{
   //Overrides KVASMultiDetArray method to add FillListsOfDetectorsByType()

   KVASMultiDetArray::MakeListOfDetectors();
   FillListsOfDetectorsByType();
}

//_________________________________________________________________________________________
void KVINDRA::FillListsOfDetectorsByType()
{
   //Fill lists of ChIo, Si, CsI and phoswich

   fChIo->Clear();
   fSi->Clear();
   fCsI->Clear();
   fPhoswich->Clear();
   TIter next_det(GetDetectors());
   KVDetector* kvd;
   while ((kvd = (KVDetector*) next_det())) {
      kvd->SetNameOfArray("INDRA");
      if (kvd->InheritsFrom("KVChIo")) {
         fChIo->Add(kvd);
      }
      if (kvd->InheritsFrom("KVSilicon")) {
         fSi->Add(kvd);
      }
      if (kvd->InheritsFrom("KVCsI")) {
         fCsI->Add(kvd);
      }
      if (kvd->InheritsFrom("KVPhoswich")) {
         fPhoswich->Add(kvd);
      }
   }
}

//_________________________________________________________________________________________

void KVINDRA::SetGroupsAndIDTelescopes()
{
   //Find groups of telescopes in angular alignment placed on different layers.
   //List is in fGroups.
   //Also creates all ID telescopes in array and stores them in fIDTelescopes.
   //Any previous groups/idtelescopes are deleted beforehand.
   CalculateGroupsFromGeometry();

   //now read list of groups and create list of ID telescopes
   CreateIDTelescopesInGroups();
}

KVChIo* KVINDRA::GetChIoOf(const Char_t* detname)
{

   //Returns a pointer to the Ionisation Chamber placed directly in front of the
   //detector "detname". If no ChIo is present, a null pointer is returned.

   KVINDRADetector* kvd;
   kvd = dynamic_cast<KVINDRADetector*>(GetDetector(detname));
   return (kvd ? (KVChIo*)kvd->GetChIo() : NULL);
}

//________________________________________________________________________________________

KVLayer* KVINDRA::GetChIoLayer(void)
{
   //Return pointer to layer in INDRA structure corresponding to ionisation
   //chambers.

   return (KVLayer*)GetStructure("LAYER", "CHIO");
}

//_________________________________________________________________________________

void KVINDRA::SetTrigger(UChar_t trig)
{
   // Define multiplicity trigger used for acquisition and filter.
   // Events with multipicity >= trig are OK.

   fTrigger = trig;
}

KVINDRADetector* KVINDRA::GetDetectorByType(UInt_t cou, UInt_t mod, UInt_t type) const
{
   //Find a detector based on the old BaseIndra type definitions:
   //
//enum EBaseIndra_type {
//      ChIo_GG=1,
//      ChIo_PG,//=2
//      ChIo_T,//=3
//      Si_GG,//=4
//      Si_PG,//=5
//      Si_T,//=6
//      CsI_R,//=7
//      CsI_L,//=8
//      CsI_T,//=9
//      Si75_GG,//=10
//      Si75_PG,//=11
//      Si75_T,//=12
//      SiLi_GG,//=13
//      SiLi_PG,//=14
//      SiLi_T//=15
//};
//enum EBaseIndra_typePhos {
//      Phos_R=1,
//      Phos_L,//=2
//      Phos_T,//=3
//};

   KVINDRADetector* det = 0;

   char nom_det[20];
   if (cou == 1) {
      if (type >= Phos_R && type <= Phos_T) {
         sprintf(nom_det, "PHOS_%02d", mod);
         det =
            (KVINDRADetector*) GetListOfPhoswich()->FindObjectByName(nom_det);
      }
   }
   if (det)
      return det;
   else if (type >= ChIo_GG && type <= ChIo_T) {
      sprintf(nom_det, "CI_%02d%02d", cou, mod);
      det = (KVINDRADetector*) GetListOfChIo()->FindObject(nom_det);
      return det;
   }
   else if (type >= Si_GG && type <= Si_T) {
      sprintf(nom_det, "SI_%02d%02d", cou, mod);
      det = (KVINDRADetector*) GetListOfSi()->FindObject(nom_det);
      return det;
   }
   else if (type >= SiLi_GG && type <= SiLi_T) {
      sprintf(nom_det, "SILI_%02d", cou);
      det = (KVINDRADetector*) GetListOfSi()->FindObject(nom_det);
      return det;
   }
   else if (type >= Si75_GG && type <= Si75_T) {
      sprintf(nom_det, "SI75_%02d", cou);
      det = (KVINDRADetector*) GetListOfSi()->FindObject(nom_det);
      return det;
   }
   else if (type >= CsI_R && type <= CsI_T) {
      sprintf(nom_det, "CSI_%02d%02d", cou, mod);
      det = (KVINDRADetector*) GetListOfCsI()->FindObject(nom_det);
      return det;
   }
   return 0;
}

//_______________________________________________________________________________________

Int_t KVINDRA::GetIDTelescopes(KVDetector* de, KVDetector* e, TCollection* idtels)
{
   //Override KVASMultiDetArray method for special case of "etalon" modules:
   //we need to add ChIo-CsI identification telescope by hand

   Int_t n = KVASMultiDetArray::GetIDTelescopes(de, e, idtels);

   if (de->InheritsFrom("KVSiLi") && e->InheritsFrom("KVCsI")) {
      KVChIo* chio = (KVChIo*)((KVINDRADetector*)e)->GetChIo();
      if (chio) {
         n += KVASMultiDetArray::GetIDTelescopes(chio, e, idtels);
      }
   }
   return n;
}

void KVINDRA::SetNamesOfIDTelescopes() const
{
   // Change default names of ID telescopes to INDRA standard
   //
   // This method also sets the types of the ID telescopes

   TIter it(GetListOfIDTelescopes());
   KVIDTelescope* idt;
   const TString etalon_numbers[] = {"1002", "1102", "1202", "1304", "1403", "1503", "1602", "1702"};
   while ((idt = (KVIDTelescope*)it())) {
      KVString N = idt->GetDetector(1)->GetName();
      N.Begin("_");
      KVString de_type = N.Next();
      KVString de_number = N.Next();
      if (idt->GetSize() == 1) {
         // PHOS_R_L_MM or CSI_R_L_RRMM
         idt->SetName(Form("%s_R_L_%s", de_type.Data(), de_number.Data()));
         idt->SetType(Form("%s_R_L", de_type.Data()));
      }
      else {
         N = idt->GetDetector(2)->GetName();
         N.Begin("_");
         KVString e_type = N.Next();
         KVString e_number = N.Next();
         if (e_type == "SILI" || e_type == "SI75") {
            e_number = etalon_numbers[dynamic_cast<KVINDRADetector*>(idt->GetDetector(2))->GetRingNumber() - 10];
         }
         idt->SetName(Form("%s_%s_%s", de_type.Data(), e_type.Data(), e_number.Data()));
         idt->SetType(Form("%s_%s", de_type.Data(), e_type.Data()));
      }
   }
   // changed names of all id telescope objects in hashlist: must rehash
   dynamic_cast<KVHashList*>(GetListOfIDTelescopes())->Rehash();
}

void KVINDRA::PerformClosedROOTGeometryOperations()
{
   // Finalise the ROOT geometry description by performing operations which can
   // only be done once the geometry is closed
   CreateROOTGeometry();
}

#ifdef WITH_MFM
Bool_t KVINDRA::handle_raw_data_event_mfmframe_ebyedat(const MFMEbyedatFrame& f)
{
   // Override base method to retrieve CENTRUM timestamp from data if present.
   // It will be added to fReconParameters as a 64-bit value "INDRA.TS" (if != 0)
   // Event number is retrieved and stored as "INDRA.EN" (if != 0)
   // Any parameter which appears as [name] and [name]_UP is an unsigned 32-bit value
   // split into two 16-bit words. We replace the two parameters with a 64-bit
   // value (to hold correctly all unsigned 32-bit values) with [name].

   if (!KVMultiDetArray::handle_raw_data_event_mfmframe_ebyedat(f)) return kFALSE;
   ULong64_t ts = f.GetCENTRUMTimestamp();
   if (ts != 0) fReconParameters.SetValue64bit("INDRA.TS", ts);
   ULong64_t en = f.GetEventNumber();
   if (en != 0) fReconParameters.SetValue64bit("INDRA.EN", en);
   int npars = fReconParameters.GetNpar();
   std::vector<TString> names;
   for (int i = 0; i < npars; ++i) {
      TString name = fReconParameters.GetParameter(i)->GetName();
      if (name.EndsWith("_UP")) names.push_back(name);
   }
   if (names.size()) {
      for (std::vector<TString>::iterator it = names.begin(); it != names.end(); ++it) {
         TString name = (*it);
         name.Remove(name.Index("_UP"), 3);
         TString name_up = (*it);
         ULong64_t par_up = fReconParameters.GetIntValue(name_up);
         ULong64_t par = fReconParameters.GetIntValue(name);
         UInt_t par32 = (par_up << 16) + par;
         fReconParameters.RemoveParameter(name_up);
         fReconParameters.RemoveParameter(name);
         fReconParameters.SetValue64bit(name, par32);
      }
   }
   return kTRUE;
}
#endif
//_______________________________________________________________________________________

void KVINDRA::SetPinLasersForCsI()
{
   // Sets the KVCsI::fPinLaser member of each CsI detector with the number of the
   // pin laser associated for the stability control of these detectors.
   //
   // We look for a file with the following format:
   //
   // CSI_0101     1
   // CSI_0102     1
   // CSI_0103     1
   // CSI_0104     1
   // etc.
   //
   // i.e. 'name of CsI detector'   'number of pin laser (1-8)'
   // Comment lines must begin with '#'
   //
   // The default name of this file is defined in .kvrootrc by
   //
   // INDRADB.CsIPinCorr:          CsI_PILA.dat
   //
   // Dataset-specific version can be specified:
   //
   // INDRA_e999.INDRADB.CsIPinCorr:    CorrCsIPin_2054.dat
   //
   // This file should be in the directory corresponding to the current dataset,
   // i.e. in $KVROOT/KVFiles/name_of_dataset

   ifstream pila_file;
   if (gDataSet->OpenDataSetFile(gDataSet->GetDataSetEnv("INDRADB.CsIPinCorr", ""), pila_file)) {

      Info("SetPinLasersForCsI", "Setting correspondance CsI-PinLaser using file %s.",
           gDataSet->GetDataSetEnv("INDRADB.CsIPinCorr", ""));
      // read file, set correspondance
      KVString line;
      line.ReadLine(pila_file);
      while (pila_file.good()) {
         if (!line.BeginsWith("#")) {
            line.Begin(" ");
            KVString detname = line.Next(kTRUE);
            KVCsI* det = (KVCsI*)GetDetector(detname.Data());
            Int_t pila = line.Next(kTRUE).Atoi();
            if (det) {
               det->SetPinLaser(pila);
            }
         }
         line.ReadLine(pila_file);
      }
      pila_file.close();
   }
   else {
      Info("SetPinLasersForCsI", "File %s not found. Correspondance Csi-PinLaser is unknown.",
           gDataSet->GetDataSetEnv("CsIPinCorr", ""));
   }
}

//_______________________________________________________________________________________

//void KVINDRA::LinkToCodeurs()
//{

//   // Link detectors with electronic modules
//   // for the moment only QDC for Si and ChIo are implemented
//   // This information is accessible via KVINDRADetector::GetNumeroCodeur()
//   // To be active one has to put in the dataset directory
//   // a file name Codeurs.dat containing the name of the file for the concerned type
//   // of electronic module
//   // for example see INDRA_e613 dataset
//   // [dataset name].INDRADB.Codeurs:    ...


//   KVFileReader flist;
//   TString fp;
//   if (!KVBase::SearchKVFile(gDataSet->GetDataSetEnv("INDRADB.Codeurs", ""), fp, gDataSet->GetName())) {
//      Warning("LinkToCodeurs", "Fichier %s, inconnu au bataillon", gDataSet->GetDataSetEnv("INDRADB.Codeurs", ""));
//      return;
//   }

//   if (!flist.OpenFileToRead(fp.Data())) {
//      //Error("ReadGainList","Error opening file named %s",fp.Data());
//      return;
//   }
//   Info("LinkToCodeurs()", "Reading correspondance Codeur-Detecteur ...");

//   TEnv* env = 0;
//   KVINDRADetector* idet = 0;
//   while (flist.IsOK()) {
//      flist.ReadLine(NULL);
//      KVString file = flist.GetCurrentLine();
//      if (file != "") {
//         if (KVBase::SearchKVFile(file.Data(), fp, gDataSet->GetName())) {
//            env = new TEnv();
//            env->ReadFile(fp.Data(), kEnvAll);
//            TEnvRec* rec = 0;
//            TObjArray* toks = 0;
//            TIter it(env->GetTable());
//            while ((rec = (TEnvRec*)it.Next())) {
//               if (!strcmp(rec->GetName(), "type")) {
//                  Info("LinkToCodeurs", "Module type %s", rec->GetValue());
//               }
//               else {
//                  toks = TString(rec->GetValue()).Tokenize(",");
//                  for (Int_t ii = 0; ii < toks->GetEntries(); ii += 1) {
//                     idet = (KVINDRADetector*)gIndra->GetDetector(((TObjString*)toks->At(ii))->GetString().Data());
//                     if (idet)
//                        idet->SetNumeroCodeur(TString(rec->GetName()).Atoi());
//                  }
//                  delete toks;
//               }
//            }
//            delete env;
//         }
//      }
//   }

//   flist.CloseFile();


//}


//_____________________________________________________________________________

void KVINDRA::GetDetectorEvent(KVDetectorEvent* detev, const TSeqCollection* fired_params)
{
   // Overrides KVASMultiDetArray::GetDetectorEvent.
   // If the list of fired acquisition parameters is given (meaning we are reading raw data)
   // then we check that what we have read is in fact an INDRA event
   // (see KVINDRATriggerInfo::IsINDRAEvent()) : if not, we do not try to find the hit groups.

   if (((fired_params && fired_params->GetEntries()) || fFiredACQParams.GetEntries())
         && !GetTriggerInfo()->IsINDRAEvent()) return;
   KVASMultiDetArray::GetDetectorEvent(detev, fired_params);
}

//_______________________________________________________________________________

//void KVINDRA::SetGGtoPGConversionFactors()
//{
//   // Sets the parameters for linear conversion of silicon & ChIo coder values
//   // between GG and PG, using the following formula:
//   //
//   //   PG  = alpha + beta*(GG - GG_0) + PG_0
//   //
//   // where GG_0 and PG_0 are respectively GG and PG pedestals
//   //
//   // We look for the file whose name is given by the .kvrootrc variable
//   //    [dataset].INDRADB.GGtoPGFactors:
//   // or by default
//   //    INDRADB.GGtoPGFactors:
//   // and expect to find in it a line for each detector of the form:
//   //    Det_Name   alpha   beta
//   // Comments in the file can be written on lines beginning with the character '#'

//   ifstream datfile;
//   if (!gDataSet->OpenDataSetFile(gDataSet->GetDataSetEnv("INDRADB.GGtoPGFactors", ""), datfile)) {

//      Info("SetGGtoPGConversionFactors", "Cannot open file with parameters for conversion (%s).",
//           gDataSet->GetDataSetEnv("INDRADB.GGtoPGFactors", ""));
//      return;
//   }
//   else {
//      Info("SetGGtoPGConversionFactors", "Reading parameters from file %s",
//           gDataSet->GetDataSetEnv("INDRADB.GGtoPGFactors", ""));

//      Char_t detname[30];
//      Double_t a, b;
//      TString aline;
//      aline.ReadLine(datfile);
//      while (datfile.good()) {

//         if (aline[0] != '#') {   //skip comments

//            sscanf(aline.Data(), "%s %lf %lf", detname, &a, &b);
//            KVINDRADetector* det = (KVINDRADetector*)GetDetector(detname);
//            if (!det) {
//               //no detector found with cou, mod and type
//               Error("SetGGtoPGConversionFactors", "Unknown detector : %s", detname);
//            }
//            else {
//               det->SetGGtoPGConversionFactors(a, b);
//               //Info("SetGGtoPGConversionFactors", "%s : PG = %f + %f * GG", detname, a, b);
//            }
//         }
//         aline.ReadLine(datfile);
//      }                            //while( datfile.good()
//      datfile.close();
//   }
//}

//_________________________________________________________________________________

void KVINDRA::CreateROOTGeometry()
{
   // Overrides KVASMultiDetArray::CreateGeoManager in order to use INDRAGeometryBuilder
   // which builds the TGeo representation of INDRA using the Y. Huguet CAO data.
   //
   // The optional arguments (dx,dy,dz) are the half-lengths in centimetres of the "world"/"top" volume
   // into which all the detectors of the array are placed. This should be big enough so that all detectors
   // fit in. The default values of 500 give a "world" which is a cube 1000cmx1000cmx1000cm (with sides
   // going from -500cm to +500cm on each axis).
   //
   // If closegeo=kFALSE we leave the geometry open for other structures to be added.

   if (!IsBuilt()) {
      Error("CreateROOTGeometry", "gIndra has to be build first");
      return;
   }
   if (!GetNavigator()) {
      //Error("CreateROOTGeometry","No existing navigator"); return;
      SetNavigator(new KVRangeTableGeoNavigator(gGeoManager, KVMaterial::GetRangeTable()));
      GetNavigator()->SetNameCorrespondanceList("INDRA.names");
   }

   // set up shape & matrix pointers in detectors
   Info("CreateROOTGeometry", "Scanning geometry shapes and matrices...");
   KVGeoImport gimp(gGeoManager, KVMaterial::GetRangeTable(), this, kFALSE);
   gimp.SetNameCorrespondanceList("INDRA.names");
   KVEvent* evt = new KVEvent();
   KVNucleus* nuc = evt->AddParticle();
   nuc->SetZAandE(1, 1, 1);
   KVINDRADetector* det;
   TIter next(GetDetectors());
   Int_t nrootgeo = 0;
   while ((det = (KVINDRADetector*)next())) {
      nuc->SetTheta(det->GetTheta());
      nuc->SetPhi(det->GetPhi());
      gimp.SetLastDetector(0);
      gimp.PropagateEvent(evt);
      if (!(det->ROOTGeo())) {
         Info("CreateROOTGeometry", "Volume checking for %s", det->GetName());
         Double_t theta0 = det->GetTheta();
         Double_t phi0 = det->GetPhi();
         for (Double_t TH = theta0 - 0.5; TH <= theta0 + 0.5; TH += 0.1) {
            for (Double_t PH = phi0 - 10; PH <= phi0 + 10; PH += 1) {
               nuc->SetTheta(TH);
               nuc->SetPhi(PH);
               gimp.SetLastDetector(0);
               gimp.PropagateEvent(evt);
               if (det->ROOTGeo()) break;
            }
            if (det->ROOTGeo()) break;
         }
      }
      if (!(det->ROOTGeo())) {
         Info("CreateROOTGeometry", "Volume checking failed for : %s", det->GetName());
      }
      // check etalon trajectories (if etalons are present)
      if (det->ROOTGeo() && det->GetRingNumber() > 9) {
         if (GetDetector(Form("SI75_%d", det->GetRingNumber())) || GetDetector(Form("SILI_%d", det->GetRingNumber()))) {
            if ((det->IsCalled("CSI_1002") || det->IsCalled("CSI_1102")
                  || det->IsCalled("CSI_1202") || det->IsCalled("CSI_1304")
                  || det->IsCalled("CSI_1403") || det->IsCalled("CSI_1503")
                  || det->IsCalled("CSI_1602") || det->IsCalled("CSI_1702"))
                  && det->GetNode()->GetNDetsInFront() < 2) {
               Info("CreateROOTGeometry", "Trajectory checking for %s", det->GetName());
               Double_t theta0 = det->GetTheta();
               Double_t phi0 = det->GetPhi();
               for (Double_t TH = theta0 - 0.5; TH <= theta0 + 0.5; TH += 0.1) {
                  for (Double_t PH = phi0 - 10; PH <= phi0 + 10; PH += 1) {
                     nuc->SetTheta(TH);
                     nuc->SetPhi(PH);
                     gimp.SetLastDetector(0);
                     gimp.PropagateEvent(evt);
                     if (det->GetNode()->GetNDetsInFront() == 2) break;
                  }
                  if (det->GetNode()->GetNDetsInFront() == 2) break;
               }
            }
         }
      }
      nrootgeo += (det->ROOTGeo());
   }
   delete evt;

   Info("CreateROOTGeometry", "ROOT geometry initialised for %d/%d detectors", nrootgeo, GetDetectors()->GetEntries());

   // Set up trajectories
   TIter it(GetDetectors());
   KVDetector* d;
   while ((d = (KVDetector*)it())) d->GetNode()->RehashLists();// make sure detector nodes are correct
   AssociateTrajectoriesAndNodes();
   DeduceGroupsFromTrajectories();
   FillTrajectoryIDTelescopeLists();
   CalculateReconstructionTrajectories();
   GetNavigator()->AbsorbDetectorPaths(&gimp);
}

void KVINDRA::SetROOTGeometry(Bool_t on)
{
   // Override base class method
   // If ROOT geometry is requested but has not been built, we create it

   if (on) {
      CreateGeoManager();
      INDRAGeometryBuilder igb;
      igb.Build(kFALSE, fCloseGeometryNow);
      if (fCloseGeometryNow) PerformClosedROOTGeometryOperations();
   }
   else {
      KVMultiDetArray::SetROOTGeometry(on);
   }
}

void KVINDRA::SetMinimumOKMultiplicity(KVEvent* e) const
{
   // Set minimum OK multiplicity for (reconstructed) event
   // This is the multiplicity trigger used for the current run (if known)

   if (GetTrigger() > 0) e->SetMinimumOKMultiplicity(GetTrigger());
}

KVGroupReconstructor* KVINDRA::GetReconstructorForGroup(const KVGroup* g) const
{
   // Special INDRA group reconstructors:
   //   KVINDRAForwardGroupReconstructor       rings 1-9
   //   KVINDRABackwardGroupReconstructor      rings 10-17
   //   KVINDRAEtalonGroupReconstructor for groups with etalon telescopes

   KVGroupReconstructor* gr(nullptr);
   if (GetGroup(g->GetName())) { // make sure group belongs to us
      // etalons ?
      if (g->GetDetectorByType("SILI") || g->GetDetectorByType("SI75"))
         gr = KVGroupReconstructor::Factory("INDRA.etalon");
      else {
         KVINDRADetector* id = (KVINDRADetector*)g->GetDetectors()->First();
         if (id->GetRingNumber() < 10) gr = KVGroupReconstructor::Factory("INDRA.forward");
         else gr = KVGroupReconstructor::Factory("INDRA.backward");
      }
   }
   return gr;
}

void KVINDRA::SetReconParametersInEvent(KVReconstructedEvent* e) const
{
   // If "INDRA.EN" parameter has been set, we use it to set the event number

   KVASMultiDetArray::SetReconParametersInEvent(e);
   if (GetReconParameters().HasValue64bit("INDRA.EN")) e->SetNumber(GetReconParameters().GetValue64bit("INDRA.EN"));
}


