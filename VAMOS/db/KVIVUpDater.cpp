//Created by KVClassFactory on Thu Oct 11 18:23:43 2012
//Author: Dijon Aurore

#include "KVIVUpDater.h"
#include "KVINDRA.h"
#include "KVMultiDetArray.h"
#include "KVIDGridManager.h"
#include "KVConfig.h"
#ifdef WITH_FITLTG
#include "KVRTGIDManager.h"
#endif
#include "KVDBParameterSet.h"
#include "KVINDRADetector.h"
#include "KVINDRADB.h"
#include "KVSilicon.h"
#include "KVFunctionCal.h"
#include "KVVAMOS.h"

#include <KVIDTelescope.h>

using namespace std;

ClassImp(KVIVUpDater)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
   <h2>KVIVUpDater</h2>
   <h4>Class for setting INDRA-VAMOS parameter for each run (e494s/e503 experiment)</h4>
   <!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVIVUpDater::KVIVUpDater()
{
   // Default constructor
}
//________________________________________________________________

KVIVUpDater::~KVIVUpDater()
{
   // Destructor
}
//________________________________________________________________

void KVIVUpDater::SetIDGrids(UInt_t run)
{
   // Set identification grid or identification function (KVTGID)
   // for all ID telescopes for this run.
   // The global ID grid manager gIDGridManager is used to set the
   // grids. The static function KVRTGIDManager::SetIDFuncInTelescopes
   //  is used to set the ID functions. First, any previously set grids/functions are removed.
   // Then all grids/functions for current run are set in the associated ID telescopes.

   Info("KVIVUpDater::SetIDGrids", "Setting Identification Grids/Functions");
   TIter next_idt(gMultiDetArray->GetListOfIDTelescopes());

   KVIDTelescope*  idt    = NULL;
#ifdef WITH_FITLTG
   KVRTGIDManager* rtgidm = NULL;
#endif

   while ((idt = (KVIDTelescope*) next_idt())) {
      idt->RemoveGrids();
#ifdef WITH_FITLTG
      if (idt->InheritsFrom("KVRTGIDManager")) {
         rtgidm = (KVRTGIDManager*)idt->IsA()->DynamicCast(KVRTGIDManager::Class(), idt);
         rtgidm->RemoveAllTGID();
      }
#endif

   }
   gMultiDetArray->SetGridsInTelescopes(run);
#ifdef WITH_FITLTG
   KVRTGIDManager::SetIDFuncInTelescopes(run);
#endif
}
//________________________________________________________________

void KVIVUpDater::SetParameters(UInt_t run)
{
   //Set the parameters of INDRA and VAMOS for this run.
   //
   //For INDRA, this will:
   //      set the multiplicity trigger of gIndra using the database value for the run
   //      set special detector gains for run (if any)
   //      set the target corresponding to the run
   //      set the ChIo pressures for the run
   //      set calibration parameters for the run
   //      set the correction of pedestals
   //      set identification parameters for the run
   //
   //For VAMOS, this will reset and set:
   //      the rotation angle of VAMOS around the target for the run
   //      the high frequency of the beam for the run
   //      the position and the type of the stripping foil for the run
   //      reference magnetic rigidity for the run
   //      calibration parameters for the run
   //      identification parameters for the run

   KVINDRAUpDater::SetParameters(run);
   SetChVoltRefGains();
   KVDBRun* kvrun = gIndraDB->GetRun(run);
   if (!kvrun) return;
   SetPedestalCorrections(kvrun);

   if (!gVamos) {
      Error("KVIVUpDater::SetParameters", "VAMOS is not found ( gVamos = NULL )");
      return;
   }

   gVamos->ResetParameters();
   SetVamosCalibAndConfParams(kvrun);
}
//________________________________________________________________

void KVIVUpDater::SetPedestalCorrections(KVDBRun* run)
{
   // For any detectors, having a pedestal correction  defined
   // for their associated QDC and  one of their signals, will have
   // their correction corresponding to the signal, set to the value
   // for the run.

   Info("KVIVUpDater::SetPedestalCorrections", "Setting pedestal corrections");

   KVRList* dp_list = run->GetLinks("DeltaPedestal");
   if (!dp_list) return;

   KVDBParameterSet* dp = NULL;
   TIter next_dp(dp_list);
   TString QDCnum;

   // Loop over INDRA detectors
   KVINDRADetector* det = NULL;
   TIter next_det(gMultiDetArray->GetDetectors());
   while ((det = (KVINDRADetector*)next_det())) {

      // Initializing each ACQ parameter pedestal correction for
      // all detectors
      KVEBYEDAT_ACQParam* acqpar = NULL;
      TIter next_acqp(det->GetACQParamList());
      while ((acqpar = (KVEBYEDAT_ACQParam*)next_acqp()))
         acqpar->SetDeltaPedestal(0.);

      // Set the pedestal correction if a value exists for
      // the QDC associated to this detector
      next_dp.Reset();
      while ((dp = (KVDBParameterSet*)next_dp())) {
         QDCnum = dp->GetName();
         QDCnum.Remove(0, 3);
         if (det->GetNumeroCodeur() == QDCnum.Atoi()) {
            acqpar = det->GetACQParam(dp->GetTitle());
            acqpar->SetDeltaPedestal(dp->GetParameter());
         }
      }
   }
}
//________________________________________________________________

void KVIVUpDater::SetVamosCalibAndConfParams(KVDBRun* run)
{
   // For a given run:
   // - set calibration parameters to the calibrators of the detectors
   //   placed at the focal plane of VAMOS (HarpeeSi, HarpeeIC, SeD, ...);
   // - set parameter of a detector by calling setter method of the
   //   detector (pedestal, Moulton PHD parameters, ...).
   // - set configuration parameters (Brho, rotation angle, ...) of VAMOS.

   Info("KVIVUpDater::SetVamosCalibAndConfParams", "Setting VAMOS calibration parameters");

   if (!gVamos) {
      Error("KVIVUpDater::SetVamosCalibAndConfParams", "VAMOS is not found ( gVamos = NULL )");
      return;
   }

   KVRList* list = run->GetLinks("VAMOS calib. & conf.");
   if (!list) return;

   TObject* obj = NULL;
   KVDBParameterSet* par = NULL;
   TIter next(list);
   TString formula, partype;

   while ((par = (KVDBParameterSet*)next())) {
      formula = par->GetTitle();
      partype = formula;
      Int_t fidx = formula.Index("f(x)=");
      if (fidx > 0) {
         formula.Remove(0, fidx + 5);
         partype.Remove(fidx - 1);
      }
      partype.Append(" ");
      partype.Append(par->GetName());
      KVCalibrator* cal = gVamos->GetCalibrator(partype.Data());
      // Case 1: parameter associated to a calibrator
      if (cal) {
         if (cal->InheritsFrom("KVFunctionCal"))
            ((KVFunctionCal*)cal)->SetExpFormula(formula.Data());

         if (cal->GetNumberParams() != par->GetParameter(0)) {
            Error("KVIVUpDater::SetVamosCalibAndConfParams",
                  "In the database, different number of parameters for the calibrator %s (%s)", cal->ClassName(), cal->GetType());
            continue;
         }

         for (Int_t i = 0; i < par->GetParameter(0); i++) {
            cal->SetParameter(i, TString(par->GetParamName(i + 1)).Atof());
            cal->SetStatus(kTRUE);
         }

         continue;
      }


      // Case 2: parameter associated to a setter method of a detector or of VAMOS
      if (!(obj = gVamos->GetDetector(par->GetName()))) {
         if (!strcmp(gVamos->GetName(), par->GetName())) obj = gVamos;
         else {
            Error("KVIVUpDater::SetVamosCalibAndConfParams",
                  "The parameter %s is not associated to an existing calibrator, detector or VAMOS",
                  partype.Data());
            continue;
         }
      }

      TString meth, arg;
      meth.Form("Set%s", par->GetTitle());
      for (Int_t i = 0; i < par->GetParameter(0); i++) {
         //          arg+=par->GetParameter( i+1 );
         arg += par->GetParamName(i + 1);
         arg += ",";
      }
      arg.Remove(TString::kTrailing, ',');
      if (!obj->IsA()->GetMethod(meth.Data(), arg.Data())) {

         Error("KVIVUpDater::SetVamosCalibAndConfParams",
               "Impossible to call %s::%s(%s) for detector %s",
               obj->ClassName(), meth.Data(), arg.Data(), obj->GetName());
         continue;
      }
      obj->Execute(meth.Data(), arg.Data());
   }
}

//________________________________________________________________

void KVIVUpDater::SetPedestals(KVDBRun* kvrun)
{
   //Set pedestals for this run
   // modifie MFR nov 2012 : add specific reading for etalons

   Info("KVIVUpDater::SetPedestals", "Setting Pedestals");

   SetChIoSiPedestals(kvrun);
   SetSi75SiLiPedestals(kvrun);
   SetCsIPedestals(kvrun);
}
//________________________________________________________________

void KVIVUpDater::SetChIoSiPedestals(KVDBRun* kvrun)
{
   //read Chio-Si pedestals
   // modified MFR november 2012 : suppress etalons from this reading list

   if (!kvrun->GetKey("Pedestals"))
      return;
   if (!kvrun->GetKey("Pedestals")->GetLinks())
      return;
   if (!kvrun->GetKey("Pedestals")->GetLinks()->At(0))
      return;

   ifstream file_pied_chiosi;
   if (!KVBase::
         SearchAndOpenKVFile(kvrun->GetKey("Pedestals")->GetLinks()->At(0)->
                             GetName(), file_pied_chiosi, fDataSet.Data())) {
      Error("KVIVUpDater::SetChIoSiPedestals", "Problem opening file %s",
            kvrun->GetKey("Pedestals")->GetLinks()->At(0)->GetName());
      return;
   }
   Info("KVIVUpDater::SetChIoSiPedestals", "Setting ChIo/Si pedestals from file: %s",
        kvrun->GetKey("Pedestals")->GetLinks()->At(0)->GetName());

   TString line;

   int cou, mod, type, n_phys, n_gene;
   float ave_phys, sig_phys, ave_gene, sig_gene;

   while (file_pied_chiosi.good())  {
      line.ReadLine(file_pied_chiosi);

      if ((line.Sizeof() > 1) && !(line.BeginsWith("#")))  {
         if (sscanf(line.Data(), "%d %d %d %d %f %f %d %f %f", &cou, &mod, &type,
                    &n_phys, &ave_phys, &sig_phys, &n_gene, &ave_gene, &sig_gene) != 9) {
            Warning("KVIVUpDater::SetChIoSiPedestals"
                    , "Bad Format in line :\n%s\nUnable to read",
                    line.Data());
         }
         else  {

            KVDetector* det = gIndra->GetDetectorByType(cou, mod, type);
            if (det) {
               switch (type)  {

                  case ChIo_GG:

                     det->SetPedestal("GG", ave_gene);
                     break;

                  case ChIo_PG:

                     det->SetPedestal("PG", ave_gene);
                     break;

                  case Si_GG:

                     det->SetPedestal("GG", ave_gene);
                     break;

                  case Si_PG:

                     det->SetPedestal("PG", ave_gene);
                     break;

                  default:

                     break;
               } // end switch
            }   //end if det
         }     // end if (line.Data) else
      }       // end line.SizeOf
   }         // end while


   file_pied_chiosi.close();
}
//________________________________________________________________

void KVIVUpDater::SetSi75SiLiPedestals(KVDBRun* kvrun)
{
   //read Etalons pedestals
   // new method MFR november 2012 : reading etalons pedestals

   if (!kvrun->GetKey("Pedestals"))
      return;
   if (!kvrun->GetKey("Pedestals")->GetLinks())
      return;
   if (!kvrun->GetKey("Pedestals")->GetLinks()->At(2))
      return;

   ifstream file_pied_etalons;
   if (!KVBase::
         SearchAndOpenKVFile(kvrun->GetKey("Pedestals")->GetLinks()->At(2)->
                             GetName(), file_pied_etalons, fDataSet.Data())) {
      Error("KVIVUpDater::SetSi75SiLiPedestals", "Problem opening file %s",
            kvrun->GetKey("Pedestals")->GetLinks()->At(2)->GetName());
      return;
   }

   Info("KVIVUpDater::SetSi75SiLiPedestals", "Setting Si75/SiLi pedestals from file: %s",
        kvrun->GetKey("Pedestals")->GetLinks()->At(2)->GetName());

   TString line;

   int cou, mod, type, n_phys, n_gene;
   float ave_phys, sig_phys, ave_gene, sig_gene;

   while (file_pied_etalons.good())  {
      line.ReadLine(file_pied_etalons);
      //      cout<<"ligne lue :"<<line.Data()<<endl;

      if ((line.Sizeof() > 1) && !(line.BeginsWith("#")))  {
         if (sscanf(line.Data(), "%d %d %d %d %f %f %d %f %f", &cou, &mod, &type,
                    &n_phys, &ave_phys, &sig_phys, &n_gene, &ave_gene, &sig_gene) != 9) {
            Warning("KVIVUpDater::SetSi75SiLiPedestals"
                    , "Bad Format in line :\n%s\nUnable to read",
                    line.Data());
         }
         else  {
            KVDetector* det = gIndra->GetDetectorByType(cou, mod, type);

            if (det) {
               switch (type) {

                  case SiLi_GG:
                     det->SetPedestal("GG", ave_gene);
                     break;

                  case SiLi_PG:
                     det->SetPedestal("PG", ave_gene);
                     break;

                  case Si75_GG:
                     det->SetPedestal("GG", ave_gene);
                     break;

                  case Si75_PG:
                     det->SetPedestal("PG", ave_gene);
                     break;

                  default:

                     break;
               }  //end switch
            } // end if det
         } // end if line.Data else
      } // end line.Begins
   } // end while

   file_pied_etalons.close();
}
//________________________________________________________________

void KVIVUpDater::SetChVoltRefGains()
{
   // For any detector, the gain is already taken into account
   // in the Channel->Volt calibration, then the reference gain of the
   // calibrator (KVChannelVolt) must be equal to the gain. (see
   // KVChannelVolt::Compute()).

   Info("KVIVUpDater::SetChVoltRefGains", "Setting channel->Volt calibrator reference gain = detector gain");

   KVDetector* det = NULL;
   TIter next_det(gMultiDetArray->GetDetectors());
   KVSeqCollection* sublist = NULL;
   while ((det = (KVDetector*)next_det())) {
      if (!det->GetListOfCalibrators()) continue;
      sublist = det->GetListOfCalibrators()->GetSubListWithClass("KVChannelVolt");
      sublist->R__FOR_EACH(KVChannelVolt, SetGainRef)(det->GetGain());
      SafeDelete(sublist);
   }
}
