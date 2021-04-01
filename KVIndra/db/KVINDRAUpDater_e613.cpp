//Created by KVClassFactory on Wed Nov 16 14:10:43 2011
//Author: bonnet

#include "KVINDRAUpDater_e613.h"
#include "KVDBParameterSet.h"
#include "KVINDRA.h"
#include "KVDetector.h"
#include "KVCalibrator.h"
//#include "KVChannelVolt.h"

ClassImp(KVINDRAUpDater_e613)

KVINDRAUpDater_e613::KVINDRAUpDater_e613()
{
   // Default constructor
}

KVINDRAUpDater_e613::~KVINDRAUpDater_e613()
{
   // Destructor
}

//_______________________________________________________________//

void KVINDRAUpDater_e613::SetGains(KVDBRun* kvrun)
{
   //Set gains used during this run
   //Read gains in database just print detectors for which gains have changed
   //

   KVRList* gain_list = kvrun->GetLinks("Gains");
   if (!gain_list) {
      return;
      Warning("SetGains", "No gains defined for this run in database");
   }
   Int_t ndets = gain_list->GetSize();
   Info("SetGains", "Loop on %d detectors : ...", ndets);

   Int_t nchange = 0;
   KVDetector* kvd = 0;
   Double_t oldgain;
   TString list;
   for (Int_t i = 0; i < ndets; i++) {
      KVDBParameterSet* dbps = (KVDBParameterSet*) gain_list->At(i);
      kvd = gIndra->GetDetector(dbps->GetName());
      if (!kvd) {
         //Error("SetGains",
         //      "Detector %s is unknown or does not exist at the current time...???",
         //   dbps->GetName());
         continue;
      }
      else {
         oldgain = kvd->GetGain();
         if (oldgain != dbps->GetParameter(0)) {
            kvd->SetGain(dbps->GetParameter(0));
            //cout << "            " << kvd->GetName() << " set gain from " << oldgain << " to G=" << kvd->GetGain() << endl;
            list += kvd->GetName();
            list += ",";
            nchange += 1;
         }
      }
   }
   if (nchange == 0)
      Info("SetGains", "Gains of the %d detectors are the same than the run before ", ndets);
   else
      Info("SetGains", "Gains have been changed for %d detectors (total = %d)", nchange, ndets);

}

//_______________________________________________________________//

void KVINDRAUpDater_e613::SetPedestals(KVDBRun* kvrun)
{
   //Set pedestals for this run

   KVRList* ped_list = kvrun->GetLinks("Pedestals");
   if (!ped_list) {
      return;
      Warning("SetPedestals", "No pedestals defined for this run in database");
   }
   Int_t ndets = ped_list->GetSize();
   Info("SetPedestals", "Loop on %d acquisition parameter : ...", ndets);

   Int_t nchange = 0;
   KVEBYEDAT_ACQParam* acq = 0;
   KVDBParameterSet* dbps = 0;
   Float_t oldped;
   TString list;
   for (Int_t i = 0; i < ndets; i++) {
      dbps = (KVDBParameterSet*) ped_list->At(i);
      acq = nullptr; // gIndra->GetACQParam(dbps->GetName());
      if (!acq) {
         //Error("SetPedestals","ACQ Parameter not defined %s",dbps->GetName());
      }
      else {
//         oldped = acq->GetPedestal();
//         if (oldped != Float_t(dbps->GetParameter(0))) {
//            acq->SetPedestal(Float_t(dbps->GetParameter(0)));

//            list += acq->GetName();
//            list += ",";
//            nchange += 1;
//         }
      }
   }
   if (nchange == 0)
      Info("SetGains", "Pedestals of the %d acquisition parameters are the same than the run before ", ndets);
   else
      Info("SetGains", "Pedestals have been changed for %d acquisition parameters (total = %d)", nchange, ndets);

}
//______________________________________________________________________________

void KVINDRAUpDater_e613::SetChVoltParameters(KVDBRun* kvrun)
{


   KVRList* param_list = kvrun->GetLinks("Channel-Volt");
   if (!param_list)
      return;
   if (!param_list->GetSize())
      return;

   KVDetector* kvd;
   KVDBParameterSet* kvps;
   KVCalibrator* kvc;
   TIter next_ps(param_list);


   TString str;

   // Setting Channel-Volts calibration parameters
   while ((kvps = (KVDBParameterSet*) next_ps())) {     // boucle sur les parametres
      str = kvps->GetName();
      str.Remove(str.Sizeof() - 4, 3);  //Removing 3 last letters (ex : "_PG")
      kvd = gIndra->GetDetector(str.Data());
      if (!kvd) {
         //    Warning("SetChVoltParameters(UInt_t)", "Dectector %s not found !",
         //            str.Data());
      }
      else {                    // detector found
         kvc = kvd->GetCalibrator(kvps->GetName(), kvps->GetTitle());
         if (!kvc)
            Warning("SetChVoltParameters(UInt_t)",
                    "Calibrator %s %s not found !", kvps->GetName(),
                    kvps->GetTitle());
         else {                 //calibrator found
            //Prise en compte du gain du detecteur quand la rampe gene a ete faite
            //pour ponderation des coef dans KVChannelVolt
            Double_t gain_ref = kvps->GetParameter(kvc->GetNumberParams());
            //((KVChannelVolt*)kvc)->SetGainRef(gain_ref);
            for (Int_t i = 0; i < kvc->GetNumberParams(); i++) {
               kvc->SetParameter(i, kvps->GetParameter(i));
            }
            kvc->SetStatus((gain_ref != 0)); // calibrator ready
         }                      //calibrator found
      }                         //detector found
   }                            //boucle sur les parameters
}
//_______________________________________________________________//

void KVINDRAUpDater_e613::SetParameters(UInt_t run)
{
   // Do some e613 run-dependent fixing and bodging
   //
   // - for runs < 559, the cables of CsI detectors 2.9 and 3.10 were interchanged
   //     for these runs, we change the names of the corresponding acquisition
   //     parameters so that correct data are associated to each detector

   // inversion cables CsI 2.9 & 3.10
//   if (run < 559) {
//      KVDetector* d = gIndra->GetDetector("CSI_0209");
//      d->GetACQParam("R")->SetName("CSI_0310_R");
//      d->GetACQParam("L")->SetName("CSI_0310_L");
//      d->GetACQParam("T")->SetName("CSI_0310_T");
//      d = gIndra->GetDetector("CSI_0310");
//      d->GetACQParam("R")->SetName("CSI_0209_R");
//      d->GetACQParam("L")->SetName("CSI_0209_L");
//      d->GetACQParam("T")->SetName("CSI_0209_T");
//      ((KVHashList*)gIndra->GetACQParams())->Rehash();
//   }
//   else {
//      KVDetector* d = gIndra->GetDetector("CSI_0209");
//      d->GetACQParam("R")->SetName("CSI_0209_R");
//      d->GetACQParam("L")->SetName("CSI_0209_L");
//      d->GetACQParam("T")->SetName("CSI_0209_T");
//      d = gIndra->GetDetector("CSI_0310");
//      d->GetACQParam("R")->SetName("CSI_0310_R");
//      d->GetACQParam("L")->SetName("CSI_0310_L");
//      d->GetACQParam("T")->SetName("CSI_0310_T");
//      ((KVHashList*)gIndra->GetACQParams())->Rehash();
//   }
   KVINDRAUpDater::SetParameters(run);
}

