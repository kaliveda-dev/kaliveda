/*
$Id: KVClassFactory.cpp,v 1.19 2009/01/21 08:04:20 franklan Exp $
$Revision: 1.19 $
$Date: 2009/01/21 08:04:20 $
*/

//Created by KVClassFactory on Wed May 13 12:31:05 2009
//Author: Abdelouahao Chbihi

#include "KVFocalPlanVamos.h"
#include "TROOT.h"
#include "KVChIo.h"
#include "KVSilicon.h"
#include "KVCsI.h"
#include "KVTelescope.h"
#include "KVDetector.h"
#include "KVMaterial.h"
using namespace std;

ClassImp(KVFocalPlanVamos)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVFocalPlanVamos</h2>
<h4>Si and CsI of vamos at FP</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVFocalPlanVamos::KVFocalPlanVamos()
{
   // Default constructor

   fSi = new KVList(kFALSE);
   fSi->SetCleanup();
   fCsI = new KVList(kFALSE);
   fCsI->SetCleanup();
   fChIo = new KVList(kFALSE);
   fChIo->SetCleanup();
   fGap = new KVList(kFALSE);
   fGap->SetCleanup();

   fSiLayer = 0;
}

KVFocalPlanVamos::~KVFocalPlanVamos()
{
   // Destructor
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
   if (fGap && fGap->TestBit(kNotDeleted)) {
      fGap->Clear();
      delete fGap;
   }
   fGap = 0;

}

//_________________________________________________________________________________

void KVFocalPlanVamos::MakeListOfDetectorTypes()
{
//Make list of all detector types needed to build Vamos focal plan

   /*
   // ChIo ionisation chambers
   KVChIo *kvchio = (KVChIo* )KVDetector::MakeDetector(Form("%s.CHIO",gDataSet->GetName()), 10.45);
      kvchio->SetLabel("CI");
      fDetectorTypes->Add(kvchio);
   */

// Silicon detectors
   KVSilicon* kvsi = (KVSilicon*)KVDetector::MakeDetector(Form("%s.SI", gDataSet->GetName()), 530.0);
// KVSilicon *kvsi =new KVSilicon(530);   // The precise thickness should be set a later.
   KVMaterial* gas = 0;
   gas = new KVMaterial("C4H10", 13.65); //New unit system in KV_1.8.1
   gas->SetPressure(40. / KVUnits::mbar);
   //KVMaterial *gas = new KVMaterial("C4H10",136.5);
   kvsi->AddAbsorber(gas);
   //gas->SetPressure(40.0);
   kvsi->SetLabel("SIE");
   fDetectorTypes->Add(kvsi);
   kvsi->Print("all");

// CsI scintillators
   KVCsI* kvcsi = (KVCsI*)KVDetector::MakeDetector(Form("%s.CSI", gDataSet->GetName()), 1.);
   kvcsi->SetLabel("CSI");
   fDetectorTypes->Add(kvcsi);
   kvcsi->Print("all");


#ifdef KV_DEBUG
   Info("MakeListOfDetectorTypes", "Success");
#endif

}
//_________________________________________________________________________________

void KVFocalPlanVamos::PrototypeTelescopes()
{
#ifdef KV_DEBUG
   Info("PrototypeTelescopes", "Begin");
   GetDetectorTypes()->Print();
#endif
   /*
       for(Int_t ii=0;ii<7;ii+=1){
         KVTelescope *kvt = new KVTelescope;
         kvt->AddDetector((KVChIo *) fDetectorTypes->FindObjectByLabel("CI"));
      Char_t type[50];
      sprintf(type,"ChIo Ring%i",ii+1);
      kvt->SetType(type);
         kvt->SetAzimuthalWidth(6.0);     //6
         kvt->SetDepth(1, 0.0);
         fTelescopes->Add(kvt);
       }
   */
#ifdef KV_DEBUG
   Info("PrototypeTelescopes", "ChIo Ring1 to Ring7, step 1  Vamos - success");
#endif
   cout << "Adding CI Ring" << endl;

   for (Int_t ii = 0; ii < 18; ii += 2) {
      KVTelescope* kvt = new KVTelescope;
      kvt->AddDetector((KVSilicon*) fDetectorTypes->FindObjectByLabel("SIE"));
      Char_t type[50];
      sprintf(type, "Si Ring%i", ii + 1);
      kvt->SetType(type);
      kvt->SetAzimuthalWidth(2.9);     //6
      kvt->SetDepth(1, 0.0);
      fTelescopes->Add(kvt);
   }
#ifdef KV_DEBUG
   Info("PrototypeTelescopes", "Si Ring1 to Ring17, step 2  Vamos - success");
#endif
   cout << "Adding SI Ring" << endl;

   for (Int_t ii = 0; ii < 18; ii += 1) {
      KVTelescope* kvt = new KVTelescope;
      kvt->AddDetector((KVCsI*) fDetectorTypes->FindObjectByLabel("CSI"));
      Char_t type[50];
      sprintf(type, "CsI Ring%i", ii + 1);
      kvt->SetType(type);
      kvt->SetAzimuthalWidth(0.9);     //2
      kvt->SetDepth(1, 0.);
      fTelescopes->Add(kvt);

   }
   cout << "Adding CSI Ring" << endl;

#ifdef KV_DEBUG
   Info("PrototypeTelescopes", "CsI Ring1 to Ring18, Vamos - success");
#endif

}

//_________________________________________________________________________________
void KVFocalPlanVamos::BuildGeometry()
{

   // Construction of E503 Focal Plan of Vamos.
   // the focal plan is made of 18 Si detector (2 lines * 9; size 5*7cm2); each Si is followed by 6 CsI 2.5*2.5cm
   // In order to simplify the implementation of the FP in the KaliVeda we decided to give a structure like INDRA
   // ring and module. They are 18 rings of CsI, 6 module per ring; for Si they are 9 rings numbred 1,3,..,17
   // containing 2 Si per ring.
   // The polar angle ofthe entrance of Vamos relative to the beam was set at 2. with an opening angle of 5, it was
   // covering the angular range 2-7 deg. We tried to make a kind of an image of this angular opening to the FP.

//Build ChIo layer
//1st Layer - ChIo
   /*
      Float_t step = 9./7.;  // I assume : 9 deg is the opening polar angle of Vamos that I divide by 9 det in horizontal line of vamos
      KVLayer *kvl = new KVLayer;
      for(Int_t ii=0;ii<7;ii+=1){
      Char_t type[50];
      sprintf(type,"ChIo Ring%i",ii+1);
           Float_t theta_min=85.5+ii*step;
      Float_t theta_max=85.5+(ii+1)*step;
      Int_t ringnum = ii+1;
          kvl->                // verifier phi et distance
             AddRing(new
                  KVRing(ringnum, theta_min+.01, theta_max, 90, 1, 5056.,
                         (KVTelescope *) fTelescopes->
                         FindObjectByType(type), 1,1,6.) //60
          );

      }

      kvl->SetName("CI");
      AddLayer(kvl);
   cout<<"Build CI Ring"<<endl;
   */

   //Build Si layer
   //2nd Layer - Si
   Float_t step = 9. / 9.; // I assume : 9 deg is the opening polar angle of Vamos that I divid by 9 det in horizontal line of vamos
   KVLayer* kvl = new KVLayer;
   for (Int_t ii = 0; ii < 9; ii += 1) {
      Char_t type[50];
      sprintf(type, "Si Ring%i", 2 * ii + 1);
      Float_t theta_min = 85.5 + ii * step;
      Float_t theta_max = 85.5 + (ii + 1) * step;
      Int_t ringnum = 2 * ii + 1;
      kvl->
      AddRing(new
              KVRing(ringnum, theta_min + .01, theta_max, 88.5, 2, 5156.,
                     (KVTelescope*) fTelescopes->
                     FindObjectByType(type), 1, 1, 3.) //30
             );

   }

   kvl->SetName("SIE");
   AddLayer(kvl);
   fSiLayer = kvl;
   cout << "Build SI Ring" << endl;

   //Build CsI layer

   kvl = new KVLayer;
   step = 9. / 18.; // horizontaly they are 2 CsI for one Si (total of 18)
   for (Int_t ii = 0; ii < 16; ii += 1) {
      Char_t type[50];
      sprintf(type, "CsI Ring%i", ii + 1);
      Float_t theta_min = 85.5 + ii * step;
      Float_t theta_max = 85.5 + (ii + 1) * step;
      Int_t ringnum = ii + 1;
      if (ringnum < 13)
         kvl->
         AddRing(new
                 KVRing(ringnum, theta_min + .01, theta_max, 87.5, 6, 5160.,
                        (KVTelescope*) fTelescopes->
                        FindObjectByType(type), 1, 1, 1.)  //10.

                );
      else
         kvl->
         AddRing(new
                 KVRing(ringnum, theta_min + .01, theta_max, 89.5, 2, 5160.,
                        (KVTelescope*) fTelescopes->
                        FindObjectByType(type), 1, 1, 1.)  //10.

                );


   }
   kvl->SetName("CSI");
   AddLayer(kvl);
   cout << "Build CSI Ring" << endl;
#ifdef KV_DEBUG
   Info("Build", "Made CsI Layer");
#endif

}



void KVFocalPlanVamos::SetGroupsAndIDTelescopes()
{
   // set correct names for detectors instead of incorrect ring-
   // and module-based names a la INDRA

   SetNamesDetectors();
   KVMultiDetArray::SetGroupsAndIDTelescopes();
}

void KVFocalPlanVamos::SetNamesDetectors()
{

   // set correct names for detectors instead of incorrect ring-
   // and module-based names a la INDRA

   Float_t thick_si[18] = {522., 530., 531., 532., 533., 533., 534., 535., 531., 535., 524., 531., 529., 524., 533., 537., 519., 530.};
   UInt_t numcsi[81] = {1, 13, 25, 41, 57, 69, 2, 14, 26, 42, 58, 70, 3, 15, 27, 43, 59, 71, 4, 16, 28, 44, 60, 72, 5,
                        17, 29, 45, 61, 73, 6, 18, 30, 46, 62, 74, 7, 19, 31, 47, 63, 75, 8, 20, 32, 48, 64, 76, 9, 21, 33,
                        49, 65, 77, 10, 22, 34, 50, 66, 78, 11, 23, 35, 51, 67, 79, 12, 24, 36, 52, 68, 80, 37, 53,
                        38, 54, 39, 55, 40, 56
                       };

   TIter next_det(gMultiDetArray->GetListOfDetectors());
   KVDetector* det;
   UInt_t ksih = 0;
   UInt_t ksib = 19;
   UInt_t kcsi = 0;
   UInt_t kchio = 1;
   UInt_t kgaph = 0;
   UInt_t kgapb = 19;

   while ((det = (KVDetector*) next_det())) {

      /*
               if(det->IsType("CI")){
               Char_t name[50];
               sprintf(name,"CI_%02d",kchio);
               det->SetName(name);
               det->SetThickness(10.45);
               kchio++;
            }
      */
      if (det->IsType("SI")) {
         Char_t name[50];
         UInt_t modnum = det->GetTelescope()->GetNumber();
         if (modnum == 1) {
            ksih++;
            sprintf(name, "SIE_%02d", ksih);
            det->SetName(name);
            //det->GetAbsorber("Silicon")->SetThickness(thick_si[ksih-1]);

            // NEED TO MAKE THE FOLLOWING 'SAFE'!!
            det->GetAbsorber("Si")->SetThickness(thick_si[ksih - 1] * 1E-4); //um->cm
         } else if (modnum == 2) {
            ksib -= 1;
            sprintf(name, "SIE_%02d", ksib);
            det->SetName(name);
            //det->GetAbsorber("Silicon")->SetThickness(thick_si[ksib-1]);

            // NEED TO MAKE THE FOLLOWING 'SAFE'!!
            det->GetAbsorber("Si")->SetThickness(thick_si[ksib - 1] * 1E-4); //um->cm
         }
      }

      if (det->IsType("CSI")) {
         Char_t name[50];
         sprintf(name, "CSI%02d", numcsi[kcsi]);
         det->SetName(name);
         kcsi++;
      }
   }

#ifdef KV_DEBUG
   Info("SetNamesSi", "Success");
#endif

}

KVLayer* KVFocalPlanVamos::GetSiLayer(void)
{

   return fSiLayer;

}
