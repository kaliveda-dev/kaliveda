/*
$Id: KVINDRAUpDater_e475s.cpp,v 1.7 2009/04/15 09:52:45 ebonnet Exp $
$Revision: 1.7 $
$Date: 2009/04/15 09:52:45 $
*/

//Created by KVClassFactory on Tue Sep 18 12:14:51 2007
//Author: Eric Bonnet

#include "KVINDRAUpDater_e475s.h"
#include "KVDBRun.h"
#include "KVDetector.h"
#include "KVCalibrator.h"
#include "KVDBParameterSet.h"
#include "KVINDRA.h"
#include "KVSilicon_e475s.h"
#include "KVChIo_e475s.h"
#include "KVCsI_e475s.h"
#include "KVEBYEDAT_ACQParam.h"
#include "KVINDRADB.h"

#include "TList.h"
#include "TString.h"

using namespace std;

ClassImp(KVINDRAUpDater_e475s)


KVINDRAUpDater_e475s::KVINDRAUpDater_e475s()
{
   //Default constructor
}

KVINDRAUpDater_e475s::~KVINDRAUpDater_e475s()
{
   //Destructor
}

void KVINDRAUpDater_e475s::SetCalibrationParameters(UInt_t run)
{


   //Set calibration parameters for this run.
   //This will:
   //      remove all the calibrators of all the detectors ready to receive the calibrators for the run (handled by child classes),
   //      set calibration parameters for the run
   //      set pedestals for the run

   cout << "Setting calibration parameters of INDRA array for run " << run << ":" <<
        endl;
   KVDBRun* kvrun = gIndraDB->GetRun(run);
   if (!kvrun) {
      Error("SetParameters(UInt_t)", "Run %u not found in database!", run);
      return;
   }
   //Reset all calibrators of all detectors first
   TIter next(fArray->GetDetectors());
   KVDetector* kvd;
   while ((kvd = (KVDetector*) next())) {
      if (kvd->InheritsFrom("KVSiLi") || kvd->InheritsFrom("KVSi75")) {
         if (kvd->GetListOfCalibrators())
            kvd->RemoveCalibrators();
         kvd->SetCalibrators();
      }
      else {
         if (kvd->GetListOfCalibrators()) {
            kvd->RemoveCalibrators();
            TIter lacq(kvd->GetACQParamList());
            KVEBYEDAT_ACQParam* acq = 0;
            while ((acq = (KVEBYEDAT_ACQParam*)lacq())) {
               acq->SetPedestal(0);
            }
         }
      }
   }
   SetCalibParameters(kvrun);
   SetPedestals(kvrun);

}



void KVINDRAUpDater_e475s::SetCalibParameters(KVDBRun* kvrun)
{

   KVRList* param_list = kvrun->GetLinks("Calibrations");

   if (!param_list)
      return;
   if (!param_list->GetSize())
      return;

   //KVDetector *kvd;
   KVDBParameterSet* kvps;
   TIter next_ps(param_list);
   TString str;

   // Setting all calibration parameters available
   while ((kvps = (KVDBParameterSet*) next_ps())) {     // boucle sur les parametres
      str = kvps->GetName();
      if (gIndra->GetDetector(str.Data())) {

         if (gIndra->GetDetector(str.Data())->InheritsFrom("KVSilicon_e475s")) {
            ((KVSilicon_e475s*)gIndra->GetDetector(str.Data()))->SetCalibrator(kvps);
         }
         else if (gIndra->GetDetector(str.Data())->InheritsFrom("KVChIo_e475s")) {
            ((KVChIo_e475s*)gIndra->GetDetector(str.Data()))->SetCalibrator(kvps);
         }
         else if (gIndra->GetDetector(str.Data())->InheritsFrom("KVCsI_e475s")) {
            ((KVCsI_e475s*)gIndra->GetDetector(str.Data()))->SetCalibrator(kvps);
         }
         else {
            Warning("SetCalibParameters(KVDBRun*)",
                    "Calibrator %s %s not found ! ",
                    kvps->GetName(), kvps->GetTitle());
         }
      }                        //detector found
   }                             //boucle sur les parameters

}



void KVINDRAUpDater_e475s::SetPedestals(KVDBRun* kvrun)
{

   KVRList* param_list = kvrun->GetLinks("Pedestals");

   if (!param_list)
      return;
   if (!param_list->GetSize())
      return;

   KVDetector* det;
   KVDBParameterSet* kvps;
   TIter next_ps(param_list);
   KVString str;
   Double_t pied_value;

   // Setting all pedestal values available
   while ((kvps = (KVDBParameterSet*) next_ps())) {     // boucle sur les parametres
      str = kvps->GetName();
      if ((det = gIndra->GetDetector(str.Data()))) {
         pied_value = kvps->GetParameter(1);
         det->SetPedestal(kvps->GetTitle(), pied_value);
      }                      //detector found
   }                          //boucle sur les parameters

}
