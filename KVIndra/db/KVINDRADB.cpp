/***************************************************************************
                          kvINDRADB.cpp  -  description
                             -------------------
    begin                : 9/12 2003
    copyright            : (C) 2003 by J.D. Frankland
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
#include "KVINDRADB.h"
#include "KVINDRADBRun.h"
#include "KVDBSystem.h"
#include "KVDBAlphaPeak.h"
#include "KVDBElasticPeak.h"
#include "KVDBChIoPressures.h"
#include "KVDBParameterSet.h"
#include "KVRunListLine.h"
#include "KVChIo.h"
#include "KVString.h"
#include "KVTarget.h"
#include "TObjArray.h"
#include "TEnv.h"
#include "KVDataSet.h"
#include "KVDataSetManager.h"
#include "KVCsI.h"
#include "TH1.h"
#include "KVNumberList.h"

KVINDRADB* gIndraDB;

////////////////////////////////////////////////////////////////////////////////
//Database containing information on runs, systems, calibrations etc. for an INDRA
//experiment or set of experiments (dataset).
//
//Each dataset is described by a KVDataSet object which is initialised by the KVDataSetManager.
//
//For each dataset, a directory exists under $KVROOT/KVFiles/name_of_dataset,
//where it is assumed the database for this dataset can be found.
//
//If DataBase.root does not exist, KVDataSet will try to rebuild it using the appropriate
//KVINDRADB class (see Plugins in $KVROOT/KVFiles/.kvrootrc).
//
//By default, KVINDRADB::Build() will read any or all of the standard format files Runlist.csv,
//Systems.dat and ChIoPressures.dat which may be found in the dataset's directory.

using namespace std;

ClassImp(KVINDRADB)

void KVINDRADB::init()
{

   fChIoPressures = AddTable("ChIo Pressures", "Pressures of ChIo");
   fTapes = AddTable("Tapes", "List of data storage tapes");
   fCsILumCorr = AddTable("CsIGainCorr", "CsI gain corrections for total light output");
   fPedestals = AddTable("Pedestals", "List of pedestal files");
   fChanVolt =
      AddTable("Channel-Volt",
               "Calibration parameters for Channel-Volts conversion");
   fVoltMeVChIoSi =
      AddTable("Volt-Energy ChIo-Si",
               "Calibration parameters for ChIo-Si Volts-Energy conversion");
   fLitEnerCsIZ1 =
      AddTable("Light-Energy CsI Z=1",
               "Calibration parameters for CsI detectors");
   fLitEnerCsI =
      AddTable("Light-Energy CsI Z>1",
               "Calibration parameters for CsI detectors");

   fGains = 0;
   fAbsentDet = 0;
   fOoODet = 0;
   fOoOACQPar = 0;

   fPulserData = 0;
}

KVINDRADB::KVINDRADB(const Char_t* name): KVExpDB(name,
         "INDRA experiment parameter database")
{
   init();
}


KVINDRADB::KVINDRADB(): KVExpDB("KVINDRADB",
                                   "INDRA experiment parameter database")
{
   init();
}

void KVINDRADB::cd()
{

   KVDataBase::cd();
   gIndraDB = this;
}

//___________________________________________________________________________
KVINDRADB::~KVINDRADB()
{
   //reset global pointer gIndraDB if it was pointing to this database

   if (gIndraDB == this)
      gIndraDB = 0;
   SafeDelete(fPulserData);
}

//____________________________________________________________________________

KVList* KVINDRADB::GetCalibrationPeaks(Int_t run, KVDetector* detector,
                                       Int_t peak_type, Int_t signal_type,
                                       Double_t peak_energy)
{
   //Use this method to access the 'peaks' (see class KVDBPeak and derivatives)
   //used for the calibration of detectors in a given run.
   //
   //Mandatory argument :
   //      run             :       number of the run for which calibration peaks are valid
   //Optional arguments :
   //      detector        :       detector for which peaks are valid
   //      peak_type       :       type of peak
   //                              peak_type = 1  Thoron alpha peak E=6.062 MeV
   //                              peak_type = 2  Thoron alpha peak E=8.785 MeV
   //                              peak_type = 3  Elastic scattering peak
   //                              peak_type = 4  Thoron alpha peak E=6.062 MeV (no gas in ChIo)
   //                              peak_type = 5  Thoron alpha peak E=8.785 MeV (no gas in ChIo)
   //      signal_type     :    one of the INDRA signal types, see KVINDRA::GetDetectorByType
   //      peak_energy :   nominal energy corresponding to peak (incident energy of projectile
   //                              in case of elastic scattering peak).
   //
   //The peaks are read as and when necessary in the peak database file.
   //
   //USER'S RESPONSIBILITY TO DELETE LIST AFTERWARDS.

   if (!gIndra) {
      Error("GetCalibrationPeaks",
            "No KVINDRA object found. You need to build INDRA before using this method.");
      return 0;
   }

   if (!OpenCalibrationPeakFile()) {
      //Calibration peak file not defined or not found
      Error("GetCalibrationPeaks",
            "Calibration peak file not defined or not found");
      return 0;
   }
   //types of peaks
   enum {
      kAlpha6MeV = 1,
      kAlpha8MeV = 2,
      kElastic = 3,
      kAlpha6MeVSG = 4,
      kAlpha8MeVSG = 5
   };

   TString sline;
   Int_t frun = 0, lrun = 0;
   KVList* peak_list = new KVList();

   //set to true after reading a run range corresponding to 'run'
   Bool_t ok_for_this_run = kFALSE;

   Int_t cour, modu, sign, typ2, first, last, entries;
   Float_t mean, error, sigma, constante;

   while (GetPeakFileStream().good()) {

      sline.ReadLine(GetPeakFileStream());

      if (sline.BeginsWith("Run Range :")) {    // Run Range found

         if (sscanf(sline.Data(), "Run Range : %d %d", &frun, &lrun) != 2) {
            Warning("ReadPeakList()",
                    "Bad format in line :\n%s\nUnable to read run range values",
                    sline.Data());
         } else {
            if (TMath::Range(frun, lrun, run) == run) { //frun <= run <= lrun
               ok_for_this_run = kTRUE;
            } else
               ok_for_this_run = kFALSE;
         }
      }                         //Run Range found
      else if (!sline.BeginsWith("#") && ok_for_this_run) {

         //only analyse (non-commented) lines which follow a valid run range

         if (sscanf(sline.Data(), "%d %d %d %d %f %f %f %d %f %d %d",
                    &cour, &modu, &sign, &typ2, &mean, &error, &sigma,
                    &entries, &constante, &first, &last) != 11) {
            Warning("GetCalibrationPeaks()",
                    "Bad format in line :\n%s\nUnable to read peak parameters",
                    sline.Data());
         } else {               // parameters correctly read

            //find corresponding detector

            KVDetector* pic_det =
               //the chio's in the file are written with the
               //ring,module of the Si/CsI in coinc
               (sign > ChIo_T) ? gIndra->GetDetectorByType(cour, modu, sign)
               : gIndra->GetDetectorByType(cour, modu, CsI_R)->GetChIo();

            //is it the right detector ?
            if (detector && detector != pic_det)
               continue;        //read next line

            //is it the right type of peak ?
            if (peak_type > 0 && peak_type != typ2)
               continue;

            //is it the right signal type ?
            if (signal_type && signal_type != sign)
               continue;

            KVDBPeak* peak = 0;
            KVDBSystem* sys = 0;

            switch (typ2) {     //what type of peak

               case kAlpha8MeVSG:

                  peak = new KVDBAlphaPeak(pic_det->GetName());
                  peak->SetGas(kFALSE);
                  peak->SetEnergy(PEAK_THORON_2);
                  break;

               case kAlpha6MeVSG:

                  peak = new KVDBAlphaPeak(pic_det->GetName());
                  peak->SetGas(kFALSE);
                  peak->SetEnergy(PEAK_THORON_1);
                  break;

               case kElastic:

                  peak = new KVDBElasticPeak(pic_det->GetName());
                  sys = (KVDBSystem*) GetRun(first)->GetSystem();
                  if (sys) {
                     peak->SetEnergy(sys->GetEbeam() * sys->GetAbeam());
                     ((KVDBElasticPeak*) peak)->SetZproj(sys->GetZbeam());
                     ((KVDBElasticPeak*) peak)->SetAproj(sys->GetAbeam());
                  }
                  break;

               case kAlpha8MeV:

                  peak = new KVDBAlphaPeak(pic_det->GetName());
                  peak->SetGas(kTRUE);
                  peak->SetEnergy(PEAK_THORON_2);
                  break;

               case kAlpha6MeV:

                  peak = new KVDBAlphaPeak(pic_det->GetName());
                  peak->SetGas(kTRUE);
                  peak->SetEnergy(PEAK_THORON_1);
                  break;

            }                   //what type of peak

            //test energy of peak if necessary
            if (peak && peak_energy > 0) {
               if (typ2 == kElastic) {
                  if (!
                        ((TMath::
                          Abs(peak->GetEnergy() /
                              (((KVDBElasticPeak*) peak)->GetAproj()) -
                              peak_energy))
                         <= 0.1 * (peak->GetEnergy() /
                                   ((KVDBElasticPeak*) peak)->GetAproj())))
                     continue;  //read next line
               } else {
                  if (!
                        (TMath::Abs(peak->GetEnergy() - peak_energy) <=
                         0.1 * peak->GetEnergy()))
                     continue;
               }
            }

            if (peak) {
               peak->SetSigType(KVINDRA::SignalTypes[sign]);
               peak->SetRing(cour);
               peak->SetModule(modu);
               peak->SetParameters(mean, error, sigma, (Double_t) entries,
                                   constante, (Double_t) first,
                                   (Double_t) last);
               peak->SetParamName(0, "Mean");
               peak->SetParamName(1, "Error");
               peak->SetParamName(2, "Sigma");
               peak->SetParamName(3, "Peak integral (entries)");
               peak->SetParamName(4, "Gaussian amplitude");
               peak->SetParamName(5, "First run");
               peak->SetParamName(6, "Last run");
               peak_list->Add(peak);

               //Set gain associated with peak.
               //This is the gain of the detector during the first run used to trace the peak.
               KVDBRun* kvrun = (KVDBRun*) GetRun(first);
               KVRList* param_list = kvrun->GetLinks("Gains");
               if (!param_list) {
                  //no gains defined - everybody has gain=1
                  peak->SetGain(1.00);
               } else {
                  KVDBParameterSet* kvdbps =
                     (KVDBParameterSet*) param_list->
                     FindObjectByName(pic_det->GetName());
                  if (!kvdbps) {
                     //no gain defined for this detector for this run - gain=1
                     peak->SetGain(1.00);
                  } else {
                     peak->SetGain(kvdbps->GetParameter(0));
                  }
               }
            }                   //if (peak)

         }                      //parameters correctly read

      }                         //Run range found

   }                            //while( .good() )

   CloseCalibrationPeakFile();
   return peak_list;
}

//_____________________________________________________________________________

Bool_t KVINDRADB::OpenCalibrationPeakFile()
{
   //Returns kTRUE if calibration peak file is open, connected to
   //ifstream __ifpeaks (access through GetPeakFileStream()), and ready
   //for reading.
   //
   //The file is opened if not already open.
   //The stream is repositioned at the beginning of the file if already open.

   if (GetPeakFileStream().is_open()) {
      Error("OpenCalibrationPeakFile", "File already open");
      return kTRUE;
   }
   return OpenCalibFile("CalibPeaks", __ifpeaks);
}

//____________________________________________________________________________

void KVINDRADB::CloseCalibrationPeakFile()
{
   //Close file containing database of calibration peaks
   __ifpeaks.close();
   __ifpeaks.clear();
}

//____________________________________________________________________________
void KVINDRADB::ReadGainList()
{

   // Read the file listing any detectors whose gain value changes during exeriment

   ifstream fin;
   if (!OpenCalibFile("Gains", fin)) {
      Error("ReadGainList()", "Could not open file %s",
            GetCalibFileName("Gains"));
      return;
   }
   Info("ReadGainList()", "Reading gains ...");

   //Add table for gains
   fGains = AddTable("Gains", "Gains of detectors during runs");

   TString sline;

   UInt_t frun = 0, lrun = 0;
   UInt_t run_ranges[MAX_NUM_RUN_RANGES][2];
   UInt_t rr_number = 0;
   Bool_t prev_rr = kFALSE;     // was the last line a run range indication ?

   Float_t gain;

   Char_t det_name[80];
   KVDBParameterSet* parset = 0;
   TList* par_list = new TList();

   while (fin.good()) {         //reading the file
      sline.ReadLine(fin);
      if (fin.eof()) {          //fin du fichier
         LinkListToRunRanges(par_list, rr_number, run_ranges);
         par_list->Clear();
         delete par_list;
         fin.close();
         return;
      }
      if (sline.BeginsWith("Run Range :")) {    // Run Range found
         if (!prev_rr) {        // new run ranges set
            if (par_list->GetSize() > 0)
               LinkListToRunRanges(par_list, rr_number, run_ranges);
            par_list->Clear();
            rr_number = 0;
         }
         if (sscanf(sline.Data(), "Run Range : %u %u", &frun, &lrun) != 2) {
            Warning("ReadGainList()",
                    "Bad format in line :\n%s\nUnable to read run range values",
                    sline.Data());
            cout << "sscanf=" << sscanf(sline.Data(), "Run Range : %u %u",
                                        &frun, &lrun) << endl;
         } else {
            prev_rr = kTRUE;
            run_ranges[rr_number][0] = frun;
            run_ranges[rr_number][1] = lrun;
            rr_number++;
            if (rr_number == MAX_NUM_RUN_RANGES) {
               Error("ReadGainList", "Too many run ranges (>%d)",
                     rr_number);
               rr_number--;
            }
         }
      }                         //Run Range found
      else if (sline.Sizeof() > 1 && !sline.BeginsWith("#")) {  //non void nor comment line
         if (sscanf(sline.Data(), "%7s %f", det_name, &gain) != 2) {
            Warning("ReadGainList()",
                    "Bad format in line :\n%s\nUnable to read",
                    sline.Data());
         } else {               //parameters correctly read
            parset = new KVDBParameterSet(det_name, "Gains", 1);
            parset->SetParameters((Double_t) gain);
            prev_rr = kFALSE;
            fGains->AddRecord(parset);
            par_list->Add(parset);
         }                      //parameters correctly read
      }                         //non void nor comment line
   }                            //reading the file
   delete par_list;
   fin.close();
}

//____________________________________________________________________________

void KVINDRADB::ReadChIoPressures()
{
   //Read ChIo pressures for different run ranges and enter into database.
   //Format of file is:
   //
   //# some comments
   //#which start with '#'
   //Run Range : 6001 6018
   //ChIos 2_3    50.0
   //ChIos 4_5    50.0
   //ChIos 6_7    50.0
   //ChIos 8_12   30.0
   //ChIos 13_17  30.0
   //
   //Pressures (of C3F8) are given in mbar).

   ifstream fin;
   if (!OpenCalibFile("Pressures", fin)) {
      Error("ReadChIoPressures()", "Could not open file %s",
            GetCalibFileName("Pressures"));
      return;
   }
   Info("ReadChIoPressures()", "Reading ChIo pressures parameters...");

   TString sline;
   UInt_t frun = 0, lrun = 0;
   UInt_t run_ranges[MAX_NUM_RUN_RANGES][2];
   UInt_t rr_number = 0;
   Bool_t prev_rr = kFALSE;     //was the previous line a run range indication ?
   Bool_t read_pressure = kFALSE; // have we read any pressures recently ?

   KVDBChIoPressures* parset = 0;
   TList* par_list = new TList();

   //any ChIo not in list is assumed absent (pressure = 0)
   Float_t pressure[5] = { 0, 0, 0, 0, 0 };

   while (fin.good()) {         // parcours du fichier

      sline.ReadLine(fin);
      if (sline.BeginsWith("Run Range :")) {    // run range found
         if (!prev_rr) {        // New set of run ranges to read

            //have we just finished reading some pressures ?
            if (read_pressure) {
               parset = new KVDBChIoPressures(pressure);
               GetTable("ChIo Pressures")->AddRecord(parset);
               par_list->Add(parset);
               LinkListToRunRanges(par_list, rr_number, run_ranges);
               par_list->Clear();
               for (int zz = 0; zz < 5; zz++) pressure[zz] = 0.;
               read_pressure = kFALSE;
            }
            rr_number = 0;

         }
         if (sscanf(sline.Data(), "Run Range : %u %u", &frun, &lrun) != 2) {
            Warning("ReadChIoPressures()",
                    "Bad format in line :\n%s\nUnable to read run range values",
                    sline.Data());
         } else {
            prev_rr = kTRUE;
            run_ranges[rr_number][0] = frun;
            run_ranges[rr_number][1] = lrun;
            rr_number++;
            if (rr_number == MAX_NUM_RUN_RANGES) {
               Error("ReadChIoPressures", "Too many run ranges (>%d)",
                     rr_number);
               rr_number--;
            }
         }
      }                         // Run Range found
      if (fin.eof()) {          //fin du fichier
         //have we just finished reading some pressures ?
         if (read_pressure) {
            parset = new KVDBChIoPressures(pressure);
            GetTable("ChIo Pressures")->AddRecord(parset);
            par_list->Add(parset);
            LinkListToRunRanges(par_list, rr_number, run_ranges);
            par_list->Clear();
            for (int zz = 0; zz < 5; zz++) pressure[zz] = 0.;
            read_pressure = kFALSE;
         }
      }
      if (sline.BeginsWith("ChIos")) {  //line with chio pressure data

         prev_rr = kFALSE;

         //take off 'ChIos' and any leading whitespace
         sline.Remove(0, 5);
         sline.Strip(TString::kLeading);
         //split up ChIo ring numbers and pressure
         TObjArray* toks = sline.Tokenize(' ');
         TString chio = ((TObjString*)(*toks)[0])->String();
         KVString press = ((TObjString*)(*toks)[1])->String();
         delete toks;

         read_pressure = kTRUE;

         if (chio == "2_3") pressure[0] = press.Atof();
         else if (chio == "4_5") pressure[1] = press.Atof();
         else if (chio == "6_7") pressure[2] = press.Atof();
         else if (chio == "8_12") pressure[3] = press.Atof();
         else if (chio == "13_17") pressure[4] = press.Atof();
         else read_pressure = kFALSE;

      }                         //line with ChIo pressure data
   }                            //parcours du fichier
   delete par_list;
   fin.close();
}

//__________________________________________________________________________________________________________________

Double_t KVINDRADB::GetEventCrossSection(Int_t run, Double_t Q_apres_cible,
      Double_t Coul_par_top) const
{
   //Returns calculated cross-section [mb] per event for the run in question.
   //See KVINDRADBRun::GetEventCrossSection()
   if (!GetRun(run))
      return 0;
   return GetRun(run)->GetEventCrossSection(Q_apres_cible, Coul_par_top);
}

//__________________________________________________________________________________________________________________

Double_t KVINDRADB::GetTotalCrossSection(Int_t run, Double_t Q_apres_cible,
      Double_t Coul_par_top) const
{
   //Returns calculated total measured cross-section [mb] for the run in question.
   //See KVINDRADBRun::GetTotalCrossSection()
   if (!GetRun(run))
      return 0;
   return GetRun(run)->GetTotalCrossSection(Q_apres_cible, Coul_par_top);
}

//__________________________________________________________________________________________________________________

Double_t KVINDRADB::GetEventCrossSection(Int_t run1, Int_t run2,
      Double_t Q_apres_cible,
      Double_t Coul_par_top) const
{
   // Returns calculated average cross-section [mb] per event for the runs in question.
   // It is assumed that all runs correspond to the same reaction,
   // with the same beam & target characteristics and multiplicity trigger.
   // The target thickness etc. are taken from the first run.

   KVTarget* targ = GetRun(run1)->GetTarget();
   if (!targ) {
      Error("GetEventCrossSection", "No target for run %d", run1);
      return 0;
   }
   Double_t sum_xsec = 0;
   for (int run = run1; run <= run2; run++) {

      if (!GetRun(run))
         continue;              //skip non-existent runs
      sum_xsec +=
         GetRun(run)->GetNIncidentIons(Q_apres_cible,
                                       Coul_par_top) * (1. - GetRun(run)->GetTempsMort());
   }
   //average X-section [mb] per event = 1e27 / (no. atoms in target * SUM(no. of projectile nuclei * (1 - TM)) )
   return (1.e27 / (targ->GetAtomsPerCM2() * sum_xsec));
}

//__________________________________________________________________________________________________________________

Double_t KVINDRADB::GetTotalCrossSection(Int_t run1, Int_t run2,
      Double_t Q_apres_cible,
      Double_t Coul_par_top) const
{
   //Returns calculated total measured cross-section [mb] for the runs in question.
   //This is SUM (GetEventCrossSection(run1,run2) * SUM( events )
   //where SUM(events) is the total number of events measured in all the runs
   Int_t sum = 0;
   for (int run = run1; run <= run2; run++) {

      if (!GetRun(run))
         continue;              //skip non-existent runs
      sum += GetRun(run)->GetEvents();

   }
   return sum * GetEventCrossSection(run1, run2, Q_apres_cible,
                                     Coul_par_top);
}

//__________________________________________________________________________________________________________________


Double_t KVINDRADB::GetEventCrossSection(const Char_t* system_name,
      Int_t mult_trig,
      Double_t Q_apres_cible,
      Double_t Coul_par_top) const
{
   //Returns calculated average cross-section [mb] per event for all runs of the named system* with trigger multiplicity 'mul_trig'.
   //                                               *to see the list of all system names, use gIndraDB->GetSystems()->ls()
   //See KVINDRADBRun::GetNIncidentIons() for meaning of other arguments

   KVDBSystem* system = GetSystem(system_name);
   if (!system) {
      Error("GetEventCrossSection",
            "System %s unknown. Check list of systems (gIndraDB->GetSystems()->ls()).",
            system_name);
      return 0.;
   }
   KVTarget* targ = system->GetTarget();
   if (!targ) {
      Error("GetEventCrossSection", "No target defined for system %s",
            system_name);
      return 0;
   }
   //loop over all runs of system, only using those with correct trigger multiplicity
   Double_t sum_xsec = 0;
   TIter next_run(system->GetRuns());
   KVINDRADBRun* run;
   while ((run = (KVINDRADBRun*) next_run())) {

      if (run->GetTrigger() != mult_trig)
         continue;              //skip runs with bad trigger
      sum_xsec +=
         run->GetNIncidentIons(Q_apres_cible,
                               Coul_par_top) * (1. - run->GetTempsMort());

   }
   //average X-section [mb] per event = 1e27 / (no. atoms in target * SUM(no. of projectile nuclei * (1 - temps mort)))
   return (1.e27 / (targ->GetAtomsPerCM2() * sum_xsec));
}

//__________________________________________________________________________________________________________________

Double_t KVINDRADB::GetTotalCrossSection(const Char_t* system_name,
      Int_t mult_trig,
      Double_t Q_apres_cible,
      Double_t Coul_par_top) const
{
   //Returns calculated total cross-section [mb] for all events in all runs of the named system* with trigger multiplicity 'mul_trig'.
   //                                               *to see the list of all system names, use gIndraDB->GetSystems()->ls()
   //See KVINDRADBRun::GetNIncidentIons() for meaning of other arguments

   KVDBSystem* system = GetSystem(system_name);
   if (!system) {
      Error("GetTotalCrossSection",
            "System %s unknown. Check list of systems (gIndraDB->GetSystems()->ls()).",
            system_name);
      return 0.;
   }
   Int_t sum = 0;
   //loop over all runs of system, only using those with correct trigger multiplicity
   TIter next_run(system->GetRuns());
   KVINDRADBRun* run;
   while ((run = (KVINDRADBRun*) next_run())) {

      if (run->GetTrigger() != mult_trig)
         continue;              //skip runs with bad trigger
      sum += run->GetEvents();
   }

   return sum * GetEventCrossSection(system_name, mult_trig, Q_apres_cible,
                                     Coul_par_top);
}

//__________________________________________________________________________________________________________________

Double_t KVINDRADB::GetTotalCrossSection(TH1* events_histo, Double_t Q_apres_cible, Double_t Coul_par_top)
{
   // Calculate the cross-section [mb] for a given selection of events in several runs,
   // given by the TH1, which is a distribution of run numbers (i.e. a histogram filled with
   // the number of selected events for each run, the run number is on the x-axis of the histogram).

   Info("GetTotalCrossSection", "Calculating cross-section for q=%f", Q_apres_cible);
   Double_t xsec = 0, ninc = 0;
   KVTarget* targ = 0;
   for (int i = 1; i <= events_histo->GetNbinsX(); i++) {
      Double_t events = events_histo->GetBinContent(i);
      if (events == 0) continue;
      int run_num = events_histo->GetBinCenter(i);
      KVINDRADBRun* run = GetRun(run_num);
      if (!targ) targ = run->GetTarget();
      ninc +=
         run->GetNIncidentIons(Q_apres_cible,
                               Coul_par_top);
      xsec += events / (1. - run->GetTempsMort());
      cout << "Run#" << run_num << "   Events : " << events
           << "   Dead time : " << run->GetTempsMort() << endl;
   }
   return (1.e27 / (ninc * targ->GetAtomsPerCM2())) * xsec;
}

//__________________________________________________________________________________________________________________

void KVINDRADB::Build()
{

   //Use KVINDRARunListReader utility subclass to read complete runlist

   //get full path to runlist file, using environment variables for the current dataset
   TString runlist_fullpath;
   KVBase::SearchKVFile(GetDBEnv("Runlist"), runlist_fullpath, fDataSet.Data());

   //set comment character for current dataset runlist
   SetRLCommentChar(GetDBEnv("Runlist.Comment")[0]);

   //set field separator character for current dataset runlist
   if (!strcmp(GetDBEnv("Runlist.Separator"), "<TAB>"))
      SetRLSeparatorChar('\t');
   else
      SetRLSeparatorChar(GetDBEnv("Runlist.Separator")[0]);

   //by default we set two keys for both recognising the 'header' lines and deciding
   //if we have a good run line: the "Run" and "Events" fields must be present
   GetLineReader()->SetFieldKeys(2, GetDBEnv("Runlist.Run"),
                                 GetDBEnv("Runlist.Events"));
   GetLineReader()->SetRunKeys(2, GetDBEnv("Runlist.Run"),
                               GetDBEnv("Runlist.Events"));

   kFirstRun = 999999;
   kLastRun = 0;
   ReadRunList(runlist_fullpath.Data());
   //new style runlist
   if (IsNewRunList()) {
      ReadNewRunList();
   };

   ReadSystemList();
   ReadChIoPressures();
   ReadGainList();
   ReadChannelVolt();
   ReadVoltEnergyChIoSi();
   ReadCalibCsI();
   ReadPedestalList();
   ReadAbsentDetectors();
   ReadOoOACQParams();
   ReadOoODetectors();

   // read all available mean pulser data and store in tree
   if (!fPulserData) fPulserData = new KVINDRAPulserDataTree;
   fPulserData->SetRunList(GetRuns());
   fPulserData->Build();

   ReadCsITotalLightGainCorrections();
}


//____________________________________________________________________________
void KVINDRADB::ReadNewRunList()
{
   //Read new-style runlist (written using KVDBRun v.10 or later)

   ifstream fin;
   if (!OpenCalibFile("Runlist", fin)) {
      Error("ReadNewRunList()", "Could not open file %s",
            GetCalibFileName("Runlist"));
      return;
   }

   Info("ReadNewRunList()", "Reading run parameters ...");

   KVString line;
   KVINDRADBRun* run;

   while (fin.good() && !fin.eof()) {
      line.ReadLine(fin);

      if (line.Length() > 1 && !line.BeginsWith("#") && !line.BeginsWith("Version")) {
         run = new KVINDRADBRun;
         run->ReadRunListLine(line);
         if (run->GetNumber() < 1) {
            delete run;
         } else {
            AddRun(run);
            kLastRun = TMath::Max(kLastRun, run->GetNumber());
            kFirstRun = TMath::Min(kFirstRun, run->GetNumber());
         }
      }
   }
   fin.close();
}
//____________________________________________________________________________

void KVINDRADB::GoodRunLine()
{
   //For each "good run line" in the run list file, we:
   //      add a KVINDRADBRun to the database if it doesn't already exist
   //      add a KVDBTape to the database if the "tape" field is active and if it doesn't already exist
   //      set properties of run and tape objects
   //kFirstRun & kLastRun are set

   KVRunListLine* csv_line = GetLineReader();

   //run number
   Int_t run_n = csv_line->GetIntField(GetDBEnv("Runlist.Run"));

   if (!run_n) {
      cout << "run_n = 0 ?????????  line number =" << GetRLLineNumber() <<
           endl;
      GetLineReader()->Print();
      return;
   }
   kLastRun = TMath::Max(kLastRun, run_n);
   kFirstRun = TMath::Min(kFirstRun, run_n);

   /*********************************************
   IF LINE HAS A TAPE NUMBER WE
    LOOK FOR THE TAPE IN THE DATA
    BASE. IF IT DOESN'T EXIST WE
    CREATE IT.
   *********************************************/
   KVDBTape* tape = 0;
   //tape number (if tape field is filled)
   if (csv_line->HasFieldValue(GetDBEnv("Runlist.Tape"))) {
      Int_t tape_n = csv_line->GetIntField(GetDBEnv("Runlist.Tape"));
      //already exists ?
      tape = GetTape(tape_n);
      if (!tape) {
         tape = new KVDBTape(tape_n);
         AddTape(tape);
      }
   }

   /*********************************************
   WE CREATE A NEW RUN AND ADD
    IT TO THE DATABASE. WE SET ALL
    AVAILABLE INFORMATIONS ON
    RUN FROM THE FILE. ERROR IF
    DBASE RUN ALREADY EXISTS =>
    SAME RUN APPEARS TWICE
   *********************************************/
   KVINDRADBRun* run = GetRun(run_n);
   if (!run) {

      run = new KVINDRADBRun(run_n);
      AddRun(run);

      //add run to tape ?
      if (tape)
         tape->AddRun(run);

      TString key = GetDBEnv("Runlist.Buffers");
      if (csv_line->HasFieldValue(key.Data()))
         run->SetScaler("Buffers", csv_line->GetIntField(key.Data()));
      key = GetDBEnv("Runlist.Events");
      if (csv_line->HasFieldValue(key.Data()))
         run->SetEvents(csv_line->GetIntField(key.Data()));
      key = GetDBEnv("Runlist.Far1");
      if (csv_line->HasFieldValue(key.Data()))
         run->SetScaler("Faraday 1", csv_line->GetIntField(key.Data()));
      key = GetDBEnv("Runlist.Far2");
      if (csv_line->HasFieldValue(key.Data()))
         run->SetScaler("Faraday 2", csv_line->GetIntField(key.Data()));
      key = GetDBEnv("Runlist.Time");
      if (csv_line->HasFieldValue(key.Data()))
         run->SetTime(csv_line->GetFloatField(key.Data()));
      key = GetDBEnv("Runlist.Size");
      if (csv_line->HasFieldValue(key.Data()))
         run->SetSize(csv_line->GetFloatField(key.Data()));
      key = GetDBEnv("Runlist.GDir");
      if (csv_line->HasFieldValue(key.Data()))
         run->SetScaler("Gene DIRECT", csv_line->GetIntField(key.Data()));
      key = GetDBEnv("Runlist.GLas");
      if (csv_line->HasFieldValue(key.Data()))
         run->SetScaler("Gene LASER", csv_line->GetIntField(key.Data()));
      key = GetDBEnv("Runlist.GElec");
      if (csv_line->HasFieldValue(key.Data()))
         run->SetScaler("Gene ELECT", csv_line->GetIntField(key.Data()));
      key = GetDBEnv("Runlist.GTest");
      if (csv_line->HasFieldValue(key.Data()))
         run->SetScaler("Gene TEST", csv_line->GetIntField(key.Data()));
      key = GetDBEnv("Runlist.GMarq");
      if (csv_line->HasFieldValue(key.Data()))
         run->SetScaler("Gene MARQ", csv_line->GetIntField(key.Data()));
      key = GetDBEnv("Runlist.GTM");
      if (csv_line->HasFieldValue(key.Data()))
         run->SetScaler("Gene TM", csv_line->GetIntField(key.Data()));
      key = GetDBEnv("Runlist.DEC");
      if (csv_line->HasFieldValue(key.Data()))
         run->SetScaler("DEC", csv_line->GetIntField(key.Data()));
      key = GetDBEnv("Runlist.FC");
      if (csv_line->HasFieldValue(key.Data()))
         run->SetScaler("FC", csv_line->GetIntField(key.Data()));
      key = GetDBEnv("Runlist.OK");
      if (csv_line->HasFieldValue(key.Data()))
         run->SetScaler("OK", csv_line->GetIntField(key.Data()));
      key = GetDBEnv("Runlist.FT");
      if (csv_line->HasFieldValue(key.Data()))
         run->SetScaler("FT", csv_line->GetIntField(key.Data()));
      key = GetDBEnv("Runlist.AVL");
      if (csv_line->HasFieldValue(key.Data()))
         run->SetScaler("AVL", csv_line->GetIntField(key.Data()));
      key = GetDBEnv("Runlist.OCD");
      if (csv_line->HasFieldValue(key.Data()))
         run->SetScaler("OCD", csv_line->GetIntField(key.Data()));
      key = GetDBEnv("Runlist.OA");
      if (csv_line->HasFieldValue(key.Data()))
         run->SetScaler("OA", csv_line->GetIntField(key.Data()));
      key = GetDBEnv("Runlist.RAZ");
      if (csv_line->HasFieldValue(key.Data()))
         run->SetScaler("RAZ", csv_line->GetIntField(key.Data()));
      key = GetDBEnv("Runlist.PlastAll");
      if (csv_line->HasFieldValue(key.Data()))
         run->SetScaler("Plast All", csv_line->GetIntField(key.Data()));
      key = GetDBEnv("Runlist.PlastG");
      if (csv_line->HasFieldValue(key.Data()))
         run->SetScaler("Plast G", csv_line->GetIntField(key.Data()));
      key = GetDBEnv("Runlist.PlastC");
      if (csv_line->HasFieldValue(key.Data()))
         run->SetScaler("Plast C", csv_line->GetIntField(key.Data()));
      key = GetDBEnv("Runlist.PlastD");
      if (csv_line->HasFieldValue(key.Data()))
         run->SetScaler("Plast D", csv_line->GetIntField(key.Data()));
      key = GetDBEnv("Runlist.TMpc");
      if (csv_line->HasFieldValue(key.Data()))
         run->SetTMpercent(csv_line->GetFloatField(key.Data()));
      key = GetDBEnv("Runlist.Trigger");
      if (csv_line->HasField(key.Data()))
         run->SetTrigger(GetRunListTrigger(key.Data(), "M>=%d"));
      key = GetDBEnv("Runlist.Start");
      if (csv_line->HasField(key.Data()))
         run->SetStartDate(csv_line->GetField(key.Data()));
      key = GetDBEnv("Runlist.End");
      if (csv_line->HasField(key.Data()))
         run->SetEndDate(csv_line->GetField(key.Data()));
      key = GetDBEnv("Runlist.Comments");
      if (csv_line->HasField(key.Data()))
         run->SetComments(csv_line->GetField(key.Data()));
      key = GetDBEnv("Runlist.Log");
      if (csv_line->HasField(key.Data()))
         run->SetLogbook(csv_line->GetField(key.Data()));

   } else {
      Error("GoodRunLine", "Run %d already exists", run_n);
   }
}
//__________________________________________________________________________________________________________________

const Char_t* KVINDRADB::GetDBEnv(const Char_t* type) const
{
   //Will look for gEnv->GetValue name "name_of_dataset.INDRADB.type"
   //then "INDRADB.type" if no dataset-specific value is found.

   if (!gDataSetManager)
      return "";
   KVDataSet* ds = gDataSetManager->GetDataSet(fDataSet.Data());
   if (!ds)
      return "";
   return ds->GetDataSetEnv(Form("INDRADB.%s", type));
}

//____________________________________________________________________________

void KVINDRADB::WriteObjects(TFile* file)
{
   // Write associated objects (i.e. KVINDRAPulserDataTree's TTree) in file
   if (fPulserData) fPulserData->WriteTree(file);
}

//____________________________________________________________________________

void KVINDRADB::ReadObjects(TFile* file)
{
   // Read associated objects (i.e. KVINDRAPulserDataTree's TTree) from file
   if (!fPulserData) fPulserData = new KVINDRAPulserDataTree;
   fPulserData->ReadTree(file);
}

//____________________________________________________________________________

void KVINDRADB::ReadCsITotalLightGainCorrections()
{
   // Read in gain corrections for CsI total light output.
   // Looks for directory
   //    $KVROOT/KVFiles/[dataset name]/[lumcorrdir]
   // where [lumcorrdir] is defined in .kvrootrc by one of the two variables
   //    INDRADB.CsILumCorr:   [lumcorrdir]
   //    [dataset name].INDRADB.CsILumCorr:   [lumcorrdir]
   // the latter value takes precedence for a given dataset over the former, generic, value.
   //
   // If the directory is not found we look for a compressed archive file
   //    $KVROOT/KVFiles/[dataset name]/[lumcorrdir].tgz
   //
   // The files in the directory containing the corrections for each run have
   // names with the format given by
   //    INDRADB.CsILumCorr.FileName:   [format]
   //    [dataset name].INDRADB.CsILumCorr.FileName:   [format]
   // the latter value takes precedence for a given dataset over the former, generic, value.
   // The [format] should include a placeholder for the run number, e.g.
   //    INDRADB.CsILumCorr.FileName:   run%04d.cor
   //    INDRADB.CsILumCorr.FileName:   Run%d.corrLum
   // etc. etc.
   //
   // The contents of each file should be in the following format:
   //    CSI_0221_R    1.00669
   //    CSI_0321_R    1.01828
   //    CSI_0322_R    1.00977
   // i.e.
   //   name_of_detector   correction
   //Any other lines are ignored.

   Info("ReadCsITotalLightGainCorrections",
        "Reading corrections...");

   // get name of directory for this dataset from .kvrootrc
   TString search;
   search = GetDBEnv("CsILumCorr");
   if (search == "") {
      Error("ReadCsITotalLightGainCorrections", "INDRADB.CsILumCorr is not defined. Check .kvrootrc files.");
   }

   KVTarArchive gain_cor(search.Data(), GetDataSetDir());
   if (!gain_cor.IsOK()) {
      Info("ReadCsITotalLightGainCorrections", "No corrections found");
      return;
   }

   TString filefmt;
   filefmt = GetDBEnv("CsILumCorr.FileName");
   if (filefmt == "") {
      Error("ReadCsITotalLightGainCorrections", "INDRADB.CsILumCorr.FileName is not defined. Check .kvrootrc files.");
   }

   // boucle sur tous les runs
   TIter next_run(GetRuns());
   KVINDRADBRun* run = 0;
   while ((run = (KVINDRADBRun*)next_run())) {

      Int_t run_num = run->GetNumber();
      TString filepath;
      filepath.Form(filefmt.Data(), run_num);
      filepath.Prepend("/");
      filepath.Prepend(search.Data());
      ifstream filereader;
      if (KVBase::SearchAndOpenKVFile(filepath, filereader, fDataSet.Data())) {

         KVString line;
         line.ReadLine(filereader);
         while (filereader.good()) {

            line.Begin(" ");
            if (line.End()) {
               line.ReadLine(filereader);
               continue;
            }
            KVString det_name = line.Next(kTRUE);
            if (!det_name.BeginsWith("CSI_")) {
               line.ReadLine(filereader);
               continue;
            }
            if (line.End()) {
               line.ReadLine(filereader);
               continue;
            }
            Double_t correction = line.Next(kTRUE).Atof();

            KVDBParameterSet* cps = new KVDBParameterSet(det_name.Data(),
                  "CsI Total Light Gain Correction", 1);

            cps->SetParameters(correction);
            fCsILumCorr->AddRecord(cps);
            cps->AddLink("Runs", run);
            line.ReadLine(filereader);
         }
         filereader.close();
      } else {
         Warning("ReadCsITotalLightGainCorrections", "Run %d: no correction", run_num);
      }

   }

}

//________________________________________________________________________________
void KVINDRADB::ReadChannelVolt()
{
   //Read Channel-Volt calibrations for ChIo and Si detectors (including Etalons).
   //The parameter filenames are taken from the environment variables
   //        [dataset name].INDRADB.ElectronicCalibration:     [chio & si detectors]
   //        [dataset name].INDRADB.ElectronicCalibration.Etalons:   [etalons]

   ifstream fin;
   if (!OpenCalibFile("ElectronicCalibration", fin)) {
      Error("ReadChannelVolt()", "Could not open file %s",
            GetCalibFileName("ElectronicCalibration"));
      return;
   }
   Info("ReadChannelVolt()",
        "Reading electronic calibration for ChIo and Si...");

   TString sline;

   UInt_t frun = 0, lrun = 0;
   UInt_t run_ranges[MAX_NUM_RUN_RANGES][2];
   UInt_t rr_number = 0;
   Bool_t prev_rr = kFALSE;     // was the last line a run range indication ?

   UInt_t cour, modu, sign;
   Float_t a0, a1, a2, dum1, dum2;

   Char_t det_name[80];
   Char_t cal_type[80];
   KVDBParameterSet* parset;
   TList* par_list = new TList();


   while (fin.good()) {         //reading the file
      sline.ReadLine(fin);
      if (fin.eof()) {          //fin du fichier
         LinkListToRunRanges(par_list, rr_number, run_ranges);
         par_list->Clear();
         break;
      }
      if (sline.BeginsWith("Run Range :")) {    // Run Range found
         if (!prev_rr) {        // new run ranges set
            if (par_list->GetSize() > 0)
               LinkListToRunRanges(par_list, rr_number, run_ranges);
            par_list->Clear();
            rr_number = 0;
         }
         if (sscanf(sline.Data(), "Run Range : %u %u", &frun, &lrun) != 2) {
            Warning("ReadChannelVolt()",
                    "Bad format in line :\n%s\nUnable to read run range values",
                    sline.Data());
         } else {
            prev_rr = kTRUE;
            run_ranges[rr_number][0] = frun;
            run_ranges[rr_number][1] = lrun;
            rr_number++;
            if (rr_number == MAX_NUM_RUN_RANGES) {
               Error("ReadChannelVolt", "Too many run ranges (>%d)",
                     rr_number);
               rr_number--;
            }
         }
      }                         //Run Range foundTObjString
      else if (sline.Sizeof() > 1 && !sline.BeginsWith("#")) {  //non void nor comment line
         if (sscanf(sline.Data(), "%u %u %u %f %f %f %f %f",
                    &cour, &modu, &sign, &a0, &a1, &a2, &dum1,
                    &dum2) != 8) {
            Warning("ReadChannelVolt()",
                    "Bad format in line :\n%s\nUnable to read",
                    sline.Data());
         } else {               //parameters correctly read
            // naming detector
            switch (sign) {
               case ChIo_GG:
                  sprintf(det_name, "CI_%02u%02u_GG", cour, modu);
                  strcpy(cal_type, "Channel-Volt GG");
                  break;
               case ChIo_PG:
                  sprintf(det_name, "CI_%02u%02u_PG", cour, modu);
                  strcpy(cal_type, "Channel-Volt PG");
                  break;
               case Si_GG:
                  sprintf(det_name, "SI_%02u%02u_GG", cour, modu);
                  strcpy(cal_type, "Channel-Volt GG");
                  break;
               case Si_PG:
                  sprintf(det_name, "SI_%02u%02u_PG", cour, modu);
                  strcpy(cal_type, "Channel-Volt PG");
                  break;
            }
            parset = new KVDBParameterSet(det_name, cal_type, 5);
            parset->SetParameters(a0, a1, a2, dum1, dum2);
            prev_rr = kFALSE;
            fChanVolt->AddRecord(parset);
            par_list->Add(parset);
         }                      //parameters correctly read
      }                         //non void nor comment line
   }                            //reading the file
   delete par_list;
   fin.close();

   /********** ETALONS ***************/

   ifstream fin2;
   if (!OpenCalibFile("ElectronicCalibration.Etalons", fin2)) {
      Error("ReadChannelVolt()", "Could not open file %s",
            GetCalibFileName("ElectronicCalibration.Etalons"));
      return;
   }
   Info("ReadChannelVolt()",
        "Reading electronic calibration for Si75 and SiLi...");
   frun = lrun = 0;
   rr_number = 0;
   prev_rr = kFALSE;     // was the last line a run range indication ?
   par_list = new TList;
   TObjArray* toks = 0;
   while (fin2.good()) {         //reading the file
      sline.ReadLine(fin2);
      if (fin2.eof()) {          //fin du fichier
         LinkListToRunRanges(par_list, rr_number, run_ranges);
         par_list->Clear();
         delete par_list;
         fin2.close();
         return;
      }
      if (sline.BeginsWith("Run Range :")) {    // Run Range found
         if (!prev_rr) {        // new run ranges set
            if (par_list->GetSize() > 0)
               LinkListToRunRanges(par_list, rr_number, run_ranges);
            par_list->Clear();
            rr_number = 0;
         }
         if (sscanf(sline.Data(), "Run Range : %u %u", &frun, &lrun) != 2) {
            Warning("ReadChannelVolt()",
                    "Bad format in line :\n%s\nUnable to read run range values",
                    sline.Data());
         } else {
            prev_rr = kTRUE;
            run_ranges[rr_number][0] = frun;
            run_ranges[rr_number][1] = lrun;
            rr_number++;
            if (rr_number == MAX_NUM_RUN_RANGES) {
               Error("ReadChannelVolt", "Too many run ranges (>%d)",
                     rr_number);
               rr_number--;
            }
         }
      }                         //Run Range found
      else if (sline.Sizeof() > 1 && !sline.BeginsWith("#")) {  //non void nor comment line
         if (sscanf(sline.Data(), "%s %f %f %f",
                    det_name, &a0, &a1, &a2) != 4) {
            Warning("ReadChannelVolt()",
                    "Bad format in line :\n%s\nUnable to read",
                    sline.Data());
         } else {               //parameters correctly read
            KVString gain;
            gain.Form("%s", det_name);
            toks = gain.Tokenize("_");
            KVString scal_type;
            scal_type.Form("Channel-Volt %s", ((TObjString*)toks->At(2))->GetString().Data());
            delete toks;
            parset = new KVDBParameterSet(det_name, scal_type.Data(), 3);
            parset->SetParameters(a0, a1, a2);
            prev_rr = kFALSE;
            fChanVolt->AddRecord(parset);
            par_list->Add(parset);
         }                      //parameters correctly read
      }                         //non void nor comment line
   }                            //reading the file
   delete par_list;
}

//__________________________________________________________________________________

void KVINDRADB::ReadVoltEnergyChIoSi()
{
   //Read Volt-Energy(MeV) calibrations for ChIo and Si detectors.
   //The parameter filename is taken from the environment variable
   //        [dataset name].INDRADB.ChIoSiVoltMeVCalib:

   ifstream fin;
   if (!OpenCalibFile("ChIoSiVoltMeVCalib", fin)) {
      Error("ReadVoltEnergyChIoSi()", "Could not open file %s",
            GetCalibFileName("ChIoSiVoltMevCalib"));
      return;
   }
   Info("ReadVoltEnergyChIoSi()",
        "Reading ChIo/Si calibration parameters...");

   TString sline;

   UInt_t frun = 0, lrun = 0;
   UInt_t run_ranges[MAX_NUM_RUN_RANGES][2];
   UInt_t rr_number = 0;
   Bool_t prev_rr = kFALSE;     // was the last line a run range indication ?

   Char_t det_name[80];
   KVDBParameterSet* parset;
   TList* par_list = new TList();

   Float_t a0, a1, chi;         // calibration parameters

   while (fin.good()) {         //reading the file
      sline.ReadLine(fin);
      if (fin.eof()) {          //fin du fichier
         LinkListToRunRanges(par_list, rr_number, run_ranges);
         par_list->Clear();
         delete par_list;
         fin.close();
         return;
      }
      if (sline.BeginsWith("Run Range :")) {    // Run Range found
         if (!prev_rr) {        // new run ranges set
            if (par_list->GetSize() > 0)
               LinkListToRunRanges(par_list, rr_number, run_ranges);
            par_list->Clear();
            rr_number = 0;
         }
         if (sscanf(sline.Data(), "Run Range : %u %u", &frun, &lrun) != 2) {
            Warning("ReadVoltEnergyAlpha()",
                    "Bad format in line :\n%s\nUnable to read run range values",
                    sline.Data());
         } else {
            prev_rr = kTRUE;
            run_ranges[rr_number][0] = frun;
            run_ranges[rr_number][1] = lrun;
            rr_number++;
            if (rr_number == MAX_NUM_RUN_RANGES) {
               Error("ReadVoltEnergyAlpha", "Too many run ranges (>%d)",
                     rr_number);
               rr_number--;
            }
         }
      }                         //Run Range found
      if (sline.BeginsWith("SI") || sline.BeginsWith("CI")) {   //data line
         if (sscanf(sline.Data(), "%7s %f %f %f", det_name, &a0, &a1, &chi)
               != 4) {
            Warning("ReadVoltEnergyAlpha()",
                    "Bad format in line :\n%s\nUnable to read parameters",
                    sline.Data());
         } else {               //parameters correctly read
            parset = new KVDBParameterSet(det_name, "Volt-Energy", 3);
            parset->SetParameters(a0, a1, chi);
            prev_rr = kFALSE;
            fVoltMeVChIoSi->AddRecord(parset);
            par_list->Add(parset);
         }                      //parameters correctly read
      }                         //data line
   }                            //reading the file
   delete par_list;
   fin.close();
}

//__________________________________________________________________________

void KVINDRADB::ReadPedestalList()
{
   //Read the names of pedestal files to use for each run range, found
   //in file with name defined by the environment variable:
   //   [dataset name].INDRADB.Pedestals:    ...

   ifstream fin;
   if (!OpenCalibFile("Pedestals", fin)) {
      Error("ReadPedestalList()", "Could not open file %s",
            GetCalibFileName("Pedestals"));
      return;
   }
   Info("ReadPedestalList()", "Reading pedestal file list...");

   KVString line;
   Char_t filename_chio[80], filename_csi[80];
   UInt_t runlist[1][2];

   while (fin.good()) {         //lecture du fichier

      // lecture des informations
      line.ReadLine(fin);

      //recherche une balise '+'
      if (line.BeginsWith('+')) {       //balise trouvee

         line.Remove(0, 1);

         if (sscanf
               (line.Data(), "Run Range : %u %u", &runlist[0][0],
                &runlist[0][1]) != 2) {
            Warning("ReadPedestalList()", "Format problem in line \n%s",
                    line.Data());
         }

         line.ReadLine(fin);

         sscanf(line.Data(), "%s", filename_chio);

         line.ReadLine(fin);

         sscanf(line.Data(), "%s", filename_csi);

         TList RRList;
         KVDBRecord* dummy = 0;
         dummy =
            new KVDBRecord(filename_chio, "ChIo/Si/Etalons pedestals");
         dummy->AddKey("Runs", "Runs for which to use this pedestal file");
         fPedestals->AddRecord(dummy);
         RRList.Add(dummy);
         dummy = new KVDBRecord(filename_csi, "CsI pedestals");
         dummy->AddKey("Runs", "Runs for which to use this pedestal file");
         fPedestals->AddRecord(dummy);
         RRList.Add(dummy);
         LinkListToRunRanges(&RRList, 1, runlist);
      }                         // balise trouvee
   }                            // lecture du fichier
   fin.close();
   cout << "Pedestals Read" << endl;
}


void KVINDRADB::ReadCalibCsI()
{
   //Read CsI Light-Energy calibrations for Z=1 and Z>1
   //The parameter filenames are taken from the environment variables
   //        [dataset name].INDRADB.CalibCsI.Z=1
   //        [dataset name].INDRADB.CalibCsI.Z>1
   //These calibrations are valid for all runs
   ReadLightEnergyCsI("Z=1", fLitEnerCsIZ1);
   ReadLightEnergyCsI("Z>1", fLitEnerCsI);
}
//_____________________________________________________________________________

void KVINDRADB::ReadLightEnergyCsI(const Char_t* zrange, KVDBTable* table)
{
   //Read CsI Light-Energy calibrations for Z=1 (zrange="Z=1") or Z>1 (zrange="Z>1")
   //and add them to the KVDBTable whose pointer is given as 2nd argument.
   //These calibrations are valid for all runs

   ifstream fin;
   TString filename;
   filename.Form("CalibCsI.%s", zrange);
   if (!OpenCalibFile(filename.Data(), fin)) {
      Error("ReadLightEnergyCsI()", "Could not open file %s",
            GetCalibFileName(filename.Data()));
      return;
   }
   Info("ReadLightEnergyCsI()",
        "Reading Light-Energy CsI calibration parameters (%s)...", zrange);

   //need description of INDRA geometry
   if (!gIndra) {
      KVMultiDetArray::MakeMultiDetector(fDataSet.Data());
   }
   //gIndra exists, but has it been built ?
   if (!gIndra->IsBuilt())
      gIndra->Build();

   TString sline;

   KVDBParameterSet* parset;
   TList* par_list = new TList();

   Float_t a2, a1, a3, a4;    // calibration parameters
   Int_t ring, mod;

   while (fin.good()) {         //reading the file
      sline.ReadLine(fin);
      if (fin.good()) {
         if (!sline.BeginsWith("#")) {  //skip comments
            if (sscanf
                  (sline.Data(), "%d %d %f %f %f %f", &ring, &mod, &a1,
                   &a2, &a3, &a4) != 6) {
               Warning("ReadLightEnergyCsI()",
                       "Bad format in line :\n%s\nUnable to read parameters",
                       sline.Data());
               return;
            } else {            //parameters correctly read
               KVCsI* csi =
                  (KVCsI*) gIndra->GetDetectorByType(ring, mod, CsI_R);
               if (!csi) {
                  Warning("ReadLightEnergyCsI()", "Read calibration for non-existent detector CSI_%02d%02d",
                          ring, mod);
               } else {
                  parset =
                     new KVDBParameterSet(csi->GetName(), Form("Light-Energy CsI %s", zrange),
                                          4);
                  parset->SetParameters(a1, a2, a3, a4);
                  table->AddRecord(parset);
                  par_list->Add(parset);
               }
            }                   //parameters correctly read
         }                      //data line
      }                         //if(fin.good
   }                            //reading the file
   fin.close();

   //these calibrators are valid for all runs
   UInt_t nranges, r_ranges[MAX_NUM_RUN_RANGES][2];
   nranges = 1;
   r_ranges[0][0] = kFirstRun;
   r_ranges[0][1] = kLastRun;
   LinkListToRunRanges(par_list, nranges, r_ranges);
   par_list->Clear();
   delete par_list;
}

//_____________________________________________________________________________
//__________________________________________________________________________________________________________________

Double_t KVINDRADB::GetEventCrossSection(KVNumberList runs,
      Double_t Q_apres_cible,
      Double_t Coul_par_top) const
{
   // Returns calculated average cross-section [mb] per event for the runs in question.
   // It is assumed that all runs correspond to the same reaction,
   // with the same beam & target characteristics and multiplicity trigger.
   // The target thickness etc. are taken from the first run.

   runs.Begin();
   Int_t run1 = runs.Next();
   KVTarget* targ = GetRun(run1)->GetTarget();
   if (!targ) {
      Error("GetEventCrossSection", "No target for run %d", run1);
      return 0;
   }
   Double_t sum_xsec = 0;
   runs.Begin();
   while (!runs.End()) {

      int run = runs.Next();
      if (!GetRun(run))
         continue;              //skip non-existent runs
      sum_xsec +=
         GetRun(run)->GetNIncidentIons(Q_apres_cible,
                                       Coul_par_top) * (1. - GetRun(run)->GetTempsMort());
   }

   //average X-section [mb] per event = 1e27 / (no. atoms in target * SUM(no. of projectile nuclei * (1 - TM)) )
   return (1.e27 / (targ->GetAtomsPerCM2() * sum_xsec));
}

//__________________________________________________________________________________________________________________

Double_t KVINDRADB::GetTotalCrossSection(KVNumberList runs,
      Double_t Q_apres_cible,
      Double_t Coul_par_top) const
{
   //Returns calculated total measured cross-section [mb] for the runs in question.
   //This is SUM (GetEventCrossSection(run1,run2) * SUM( events )
   //where SUM(events) is the total number of events measured in all the runs
   Int_t sum = 0;
   runs.Begin();
   while (!runs.End()) {
      int run = runs.Next();
      if (!GetRun(run))
         continue;              //skip non-existent runs
      sum += GetRun(run)->GetEvents();

   }
   return sum * GetEventCrossSection(runs, Q_apres_cible,
                                     Coul_par_top);
}


//____________________________________________________________________________
void KVINDRADB::ReadAbsentDetectors()
{
   //Lit le fichier ou sont listés les détecteurs retirés au cours
   //de la manip
   TString fp;
   if (!KVBase::SearchKVFile(GetCalibFileName("AbsentDet"), fp, fDataSet.Data())) {
      Error("ReadAbsentDetectors", "Fichier %s, inconnu au bataillon", GetCalibFileName("AbsentDet"));
      return;
   }
   Info("ReadAbsentDetectors()", "Lecture des detecteurs absents...");
   fAbsentDet = AddTable("Absent Detectors", "Name of physically absent detectors");

   KVDBRecord* dbrec = 0;
   TEnv env;
   TEnvRec* rec = 0;
   env.ReadFile(fp.Data(), kEnvAll);
   TIter it(env.GetTable());

   while ((rec = (TEnvRec*)it.Next())) {
      KVString srec(rec->GetName());
      KVNumberList nl(rec->GetValue());
      cout << rec->GetValue() << endl;
      if (srec.Contains(",")) {
         srec.Begin(",");
         while (!srec.End()) {
            dbrec = new KVDBRecord(srec.Next(), "Absent Detector");
            dbrec->AddKey("Runs", "List of Runs");
            fAbsentDet->AddRecord(dbrec);
            LinkRecordToRunRange(dbrec, nl);
         }
      } else {
         dbrec = new KVDBRecord(rec->GetName(), "Absent Detector");
         dbrec->AddKey("Runs", "List of Runs");
         fAbsentDet->AddRecord(dbrec);
         LinkRecordToRunRange(dbrec, nl);
      }
   }

}

//____________________________________________________________________________
void KVINDRADB::ReadOoODetectors()
{

   //Lit le fichier ou sont listés les détecteurs ne marchant plus au cours
   //de la manip
   TString fp;
   if (!KVBase::SearchKVFile(GetCalibFileName("OoODet"), fp, fDataSet.Data())) {
      Error("ReadOoODetectors", "Fichier %s, inconnu au bataillon", GetCalibFileName("OoODet"));
      return;
   }
   Info("ReadOoODetectors()", "Lecture des detecteurs hors service ...");
   fOoODet = AddTable("OoO Detectors", "Name of out of order detectors");

   KVDBRecord* dbrec = 0;
   TEnv env;
   TEnvRec* rec = 0;
   env.ReadFile(fp.Data(), kEnvAll);
   TIter it(env.GetTable());

   while ((rec = (TEnvRec*)it.Next())) {
      KVString srec(rec->GetName());
      KVNumberList nl(rec->GetValue());
      cout << rec->GetValue() << endl;
      if (srec.Contains(",")) {
         srec.Begin(",");
         while (!srec.End()) {
            dbrec = new KVDBRecord(srec.Next(), "OoO Detector");
            dbrec->AddKey("Runs", "List of Runs");
            fOoODet->AddRecord(dbrec);
            LinkRecordToRunRange(dbrec, nl);
         }
      } else {
         dbrec = new KVDBRecord(rec->GetName(), "OoO Detector");
         dbrec->AddKey("Runs", "List of Runs");
         fOoODet->AddRecord(dbrec);
         LinkRecordToRunRange(dbrec, nl);
      }
   }

}

//____________________________________________________________________________
void KVINDRADB::ReadOoOACQParams()
{

   //Lit le fichier ou sont listés les parametres d acquisition ne marchant plus au cours
   //de la manip
   TString fp;
   if (!KVBase::SearchKVFile(GetCalibFileName("OoOACQPar"), fp, fDataSet.Data())) {
      Error("ReadNotWorkingACQParams", "Fichier %s, inconnu au bataillon", GetCalibFileName("OoOACQPar"));
      return;
   }
   Info("ReadNotWorkingACQParams()", "Lecture des parametres d acq hors service ...");
   fOoOACQPar = AddTable("OoO ACQPars", "Name of not working acq parameters");

   KVDBRecord* dbrec = 0;
   TEnv env;
   TEnvRec* rec = 0;
   env.ReadFile(fp.Data(), kEnvAll);
   TIter it(env.GetTable());

   while ((rec = (TEnvRec*)it.Next())) {
      KVString srec(rec->GetName());
      KVNumberList nl(rec->GetValue());
      cout << rec->GetValue() << endl;
      if (srec.Contains(",")) {
         srec.Begin(",");
         while (!srec.End()) {
            dbrec = new KVDBRecord(srec.Next(), "OoO ACQPar");
            dbrec->AddKey("Runs", "List of Runs");
            fOoOACQPar->AddRecord(dbrec);
            LinkRecordToRunRange(dbrec, nl);
         }
      } else {
         dbrec = new KVDBRecord(rec->GetName(), "OoO ACQPar");
         dbrec->AddKey("Runs", "List of Runs");
         fOoOACQPar->AddRecord(dbrec);
         LinkRecordToRunRange(dbrec, nl);
      }
   }

}

