//Created by KVClassFactory on Mon Sep 17 17:49:41 2012
//Author: Ademard Guilain

#include "KVIVDB.h"
#include "KVDBParameterSet.h"
#include "KVGANILDataReader.h"
#include "KVDataRepository.h"

ClassImp(KVIVDB)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
   <h2>KVIVDB</h2>
   <h4>Database for e494s experiment coupling INDRA with VAMOS</h4>
   <!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////


const Char_t KVIVDB::fVAMOSscalerNames[40][8] = {
   "SI01" , "SI02" , "SI03" , "SI04" , "SI05", "SI06"   , "SI07", "SI08", "SI09" , "SI10"   ,
   "SI11" , "SI12" , "SI13" , "SI14" , "SI15",  "SI16"   , "SI17", "SI18", "SIE05", "SIE06"  ,
   "SIE08", "U2M22", "U2M23", "U2M24", "SED1", "SED2"   , "SED3", "FTA" , "INDRA", "CLOCKDT",
   "CLOCK", "U2M32", "FIL1" , "FIL2" , "CSI" , "CURRENT", "MCP" , "ORSI", "U2M39", "U2M40"
};


KVIVDB::KVIVDB()
{
   // Default constructor
   init();
}
//________________________________________________________________

KVIVDB::KVIVDB(const Char_t* name) : KVINDRADB(name)
{
   init();
}
//________________________________________________________________

KVIVDB::~KVIVDB()
{
   // Destructor
}
//________________________________________________________________

void KVIVDB::init()
{
   // Default initialisations
   fDeltaPed = AddTable("DeltaPedestal", "Pedestal correction value of detectors");
   fVAMOSCalConf = AddTable("VAMOS calib. & conf.", "Calibration and configuration parameters for VAMOS and its detectors");

}
//________________________________________________________________

void KVIVDB::Build()
{
   // Read runlist and systems list, then read VAMOS scalers
   // Read calibration and configuration (Brho, angle, ...) parameters
   // of VAMOS for each run.

   KVINDRADB::Build();
   ReadPedestalCorrection();
   ReadVamosCalibrations();
}
//________________________________________________________________

void KVIVDB::ReadDeltaPedestal(ifstream& ifile)
{
   // Reading the pedestal correction (DeltaPed). Method called by
   // KVINDRADB_e503::ReadPedestalCorrection(). DeltaPed is equal to
   // DeltaNoise for high gain coder data. For anyother coder data,
   // DetaPed is given by DeltaGene if DeltaGene is less than 20 channels,
   // otherwise it is given by DeltaNoise. DeltaGene (DeltaNoise) is the
   // difference between pedestal positions from the generator (noise)
   // of the current run and of the reference run.


   KVString sline, signal, parname;
   KVDBParameterSet* parset = NULL;
   Int_t numQDC = -1;
   KVNumberList runs;
   Double_t Dped = -1., Dgene = -1., Dbruit = -1.;

   while (ifile.good()) {         //reading the file
      sline.ReadLine(ifile);
      // Skip comment line
      if (sline.BeginsWith("#")) continue;
      // End character
      if (sline.BeginsWith("!")) return;

      sline.ReplaceAll("\t", " ");
      // QDC number
      if (sline.BeginsWith("QDC=")) {
         sline.Remove(0, 4);
         numQDC = sline.Atof();
         continue;
      }
      // DeltaPed for each signal
      Ssiz_t idx = sline.Index(":");
      runs.SetList(TString(sline).Remove(idx));
      sline.Remove(0, idx + 1);
      sline.Begin(" ");
      while (!sline.End()) {
         signal = sline.Next(kTRUE);
         Dgene  = sline.Next(kTRUE).Atof();
         Dbruit = sline.Next(kTRUE).Atof();

         // Marie-France's selection
         Dped = ((Dgene > 20) || !strcmp(signal.Data(), "GG") ? Dbruit : Dgene);
         if (Dped > 20) Warning("ReadDeltaPedestal", "DeltaPed>20 for runs %s, signal %s", runs.GetList(), signal.Data());

         parname.Form("QDC%.2d", numQDC);
         parset = new KVDBParameterSet(parname.Data(), signal.Data(), 1);
         parset->SetParameter(Dped);
         fDeltaPed->AddRecord(parset);
         LinkRecordToRunRange(parset, runs);
      }
   }
}
//________________________________________________________________

void KVIVDB::ReadPedestalCorrection()
{
   // Reading the pedestal correction (DeltaPed). DeltaPed depends on
   // the run number but it is the same for all detectors connected to the
   // same QDC. The corrected pedestal is given by the reference
   // pedestal + DeltaPed.
   ifstream fin;

   if (!OpenCalibFile("PedestalCorrections", fin)) {
      Warning("ReadPedestalCorrection", "Pedestal correction file not found : %s",
              GetCalibFileName("PedestalCorrections"));
      return;
   }
   Info("ReadPedestalCorrection", "Reading in pedestal correction file : %s",
        GetCalibFileName("PedestalCorrections"));


   TString sline;
   while (fin.good()) {         //reading the file
      sline.ReadLine(fin);
      // Skip comment line
      if (sline.BeginsWith("#")) continue;

      sline.ReplaceAll("\t", " ");
      if (sline.BeginsWith("+DeltaPed")) ReadDeltaPedestal(fin);
   }
   fin.close();
}
//________________________________________________________________

void KVIVDB::ReadPedestalList()
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
   Char_t filename_chio[80], filename_csi[80], filename_etalons[80];
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

         line.ReadLine(fin);
         sscanf(line.Data(), "%s", filename_etalons);

         TList RRList;
         KVDBRecord* dummy = 0;
         dummy = new KVDBRecord(filename_chio, "ChIo/Si pedestals");
         dummy->AddKey("Runs", "Runs for which to use this pedestal file");
         fPedestals->AddRecord(dummy);
         RRList.Add(dummy);
         dummy = new KVDBRecord(filename_csi, "CsI pedestals");
         dummy->AddKey("Runs", "Runs for which to use this pedestal file");
         fPedestals->AddRecord(dummy);
         RRList.Add(dummy);
         dummy = new KVDBRecord(filename_etalons, "Si75/SiLi pedestals");
         dummy->AddKey("Runs", "Runs for which to use this pedestal file");
         fPedestals->AddRecord(dummy);
         RRList.Add(dummy);
         LinkListToRunRanges(&RRList, 1, runlist);
      }                         // balise trouvee
   }                            // lecture du fichier
   fin.close();
   cout << "Pedestals Read" << endl;
}
//________________________________________________________________

Bool_t KVIVDB::ReadVamosCalibFile(ifstream& ifile)
{
   // Reads the calibration file loaded in 'infile'.
   // Retruns kTRUE if the mimimum of information is
   // given inside the file (Run list, type of calibration,
   // function of calibration, acquisition parameter
   // associated to the calibration parameters... ).
   // The format is the following:
   //
   // RUNS: 1-30 34 50-200
   // TYPE: channel->Volt  (or channel->ns, Volt->MeV)
   // FUNCTION: pol1       (or formula format of TF1)
   //
   // these tree infromations are the minimum to give in the file.
   //
   // If the FUNCTION is 'no'( 'NO' or empty ) then it is assume that
   // the calibrator is defined by a class inheriting from KVCalibrator
   // different from KVFunctionCal.
   //
   // The following depends of the calibration type:
   // - If the type contains 'channel' then the calibration
   //   parameters have to be associated to an acquisition
   //   parameter. In this case the name of the acquisition
   //   parameter has to be given first followed by ':' and the parameters.
   // - else the  calibration parameters have to be
   //   associated to the corresponding detector.
   //
   // The parameters can be add on a new line if the previous line end
   // with the character '\'
   //
   // The comment line begins with '#'.
   //
   // You can change when you want the run list, or the
   // function. The example below illustrates these
   // possibilities:
   //
   // RUNS: 1-30 34 50-200
   // TYPE: channel->Volt
   // FUNCTION: pol1
   // SED1_X_001: par0 par1
   // SED1_X_002: par0 par1
   // #
   // FUNCTION: [0]+[1]*x+[2]/x
   // SED1_X_003: par0 par1 par2
   // ...
   // ...
   // RUNS: 201 203 207-266
   // SED1_X_001: par0 par1 par2
   // SED1_X_002: par0 par1 par3
   // ...
   // ...
   // RUNS: 1-266
   // FUNCTION: pol6
   // TYPE: channel->ns
   // SED1_HF: par0 par1 par2 par3 par4 par5 par6
   //
   // TYPE: position->cm
   // FUNCTION: no
   // SED1: par1 par2 par3 par4 par5 \.
   //       par6 par7 par8 par8 par10
   //
   //
   // It is possible to set a parameter for a detector (or for VAMOS) if
   // a Setter method exists for this parameter in the classe describing
   // the detector (or VAMOS). In this case the name of the parameter has
   // to be given in TYPE and FUNCTION has to be set to 'no' or null. For
   // example to set the reference Brho 0.2 T.m to VAMOS for run 1 to 100
   // write:
   //
   // RUNS: 1-100
   // TYPE: BrhoRef
   // FUNCTION: no
   // VAMOS: 0.2


   KVString sline, name;
   Int_t idx;
   Int_t OKbit = 0;
   KVDBParameterSet* parset = NULL;
   KVNumberList runs;
   TObjArray* tok = NULL;
   KVString infoNames[3] = {"RUNS", "TYPE", "FUNCTION"};
   KVString infos[3];

   while (ifile.good()) {         //reading the file
      sline.ReadLine(ifile);

      if (sline.IsNull()) continue;
      // Skip comment line
      if (sline.BeginsWith("#")) continue;

      sline.ReplaceAll("\t", " ");
      // Mandatory character ':'
      if ((idx = sline.Index(":")) < 0) continue;

      name = sline;
      name.Remove(idx);
      name.Remove(KVString::kBoth, ' ');
      sline.Remove(0, idx + 1);
      sline.Remove(KVString::kBoth, ' ');

      Int_t i;
      for (i = 0; i < 3; i++)
         if (name == infoNames[i]) break;

      // if line contains an info  ( RUNS, TYPE or FUNCTION )
      if (i < 3) {
         infos[i] = sline;
         OKbit = OKbit | (1 << i);
         if (i == 0) runs.SetList(sline);
      } else if (OKbit != 7) {
         // OKbit different from 7 means RUNS, TYPE or FUNCTION are missing
         for (Int_t j = 0; j < 3; j++) {
            if (!(OKbit & (1 << j)))
               Error("ReadVamosCalibFile", "Missing %s for the calibration of %s", infoNames[j].Data(), name.Data());

         }
         return kFALSE;
      } else {

         if (sline.EndsWith("\\")) {
            sline.Remove(KVString::kTrailing, '\\');
            sline.Remove(KVString::kTrailing, ' ');
            KVString extra;
            Bool_t next = kTRUE;
            while (next) {
               extra.ReadLine(ifile);
               if (extra.BeginsWith("#")) continue;
               if (extra.EndsWith("\\")) extra.Remove(KVString::kTrailing, '\\');
               else  next = kFALSE;
               extra.Remove(KVString::kBoth, ' ');
               sline.Append(" ");
               sline.Append(extra);
            }
         }

         tok = sline.Tokenize(" ");

         if (infos[2].CompareTo("no") && infos[2].CompareTo("NO") && !(infos[2].IsNull())) {
            // for calibrator with formula expression
            sline.Form("f%s", name.Data());
            TF1 f(sline.Data(), infos[2].Data());
            if (tok->GetEntries() != f.GetNpar()) {
               cout << "info 2: " << infos[2].Data() << endl;
               Error("ReadVamosCalibFile", "Different numbers of parameters:");
               cout << "- the function " << f.GetExpFormula().Data() << " ( " << f.GetNpar() << " parameters )" << endl;
               cout << "- the calibrator of " << name.Data() << " ( " << tok->GetEntries() << " parameters )" << endl;
               delete tok;
               return kFALSE;
            }
            TString pname(infos[1]);
            pname.Append(" f(x)=");
            pname.Append(f.GetExpFormula());
            parset = new KVDBParameterSet(name.Data(), pname.Data(), f.GetNpar() + 1);
            parset->SetParamName(0, "Npar");
            parset->SetParameter(0, f.GetNpar());
            for (Int_t j = 0; j < f.GetNpar(); j++) {
//             parset->SetParamName( j+1, f.GetParName(j));
//             parset->SetParameter( j+1, ((TObjString *)tok->At(j))->GetString().Atof() );
               parset->SetParamName(j + 1, ((TObjString*)tok->At(j))->GetString().Data());
            }
         } else {
            // for calibrator without formula expression
            parset = new KVDBParameterSet(name.Data(), infos[1].Data(), tok->GetEntries() + 1);
            parset->SetParamName(0, "Npar");
            parset->SetParameter(0, tok->GetEntries());
            for (Int_t j = 0; j < tok->GetEntries() ; j++) {
//             parset->SetParamName( j+1, Form("par%d",j) );
//             parset->SetParameter( j+1, ((TObjString *)tok->At(j))->GetString().Atof() );
               parset->SetParamName(j + 1, ((TObjString*)tok->At(j))->GetString().Data());
            }
         }
         delete tok;
         fVAMOSCalConf->AddRecord(parset);
         LinkRecordToRunRange(parset, runs);

//       cout<<parset->GetName()<<" "<<parset->GetTitle()<<" "<<runs.GetList()<<" "<<parset->GetParamName(0);
//       for( Int_t j=0; j< parset->GetParamNumber(); j++ )
//          cout<<" "<<parset->GetParameter(j);
//          cout<<endl;
      }
   }

   return kTRUE;
}
//________________________________________________________________

void KVIVDB::ReadVamosCalibrations()
{
   // All the calibration files of VAMOS have the same structure.
   // This method look for the global file where the list of calibration
   // files are written.
   // This global file is given in the  environment variable
   // 'VamosCalibFile' (see OpenCalibFile(...)).
   //

   ifstream inf;
   if (!OpenCalibFile("VamosCalibFile", inf)) {
      Error("ReadVamosCalibrations", "Could not open file %s", GetCalibFileName("VamosCalibFile"));
      return;
   }

   Info("ReadVamosCalibrations", "Reading calibration parameters of VAMOS...");

   TString sline;
   while (!inf.eof()) {
      sline.ReadLine(inf);
      if (sline.IsNull()) continue;

      // Skip comment line
      if (sline.BeginsWith("#")) continue;

      ifstream inf2;
      if (!gDataSet->OpenDataSetFile(sline.Data(), inf2)) {
         Error("ReadVamosCalibrations", "Could not open file %s", sline.Data());
         inf2.close();
         continue;
      }

      Info("ReadVamosCalibrations", "in file %s", sline.Data());
      if (!ReadVamosCalibFile(inf2)) Error("ReadVamosCalibrations", "Bad structure inside the file %s", sline.Data());

      inf2.close();
   }
   inf.close();
}//________________________________________________________________

void KVIVDB::ReadAndCorrectVamosScalers(const Char_t* runlist, Option_t* opt)
{
   // This method read the VAMOS scaler buffers in the raw data files for the given
   // runlist. The true value of each scaler is calculated. This calculation is
   // based on program "scaler.cpp" developped  by Abdou Chbihi (GANIL).
   // Then the new value of scalers are stored in this database.
   // Moreover the Vamos Dead-Time (DT in percent) is calculated:
   //  DT = 1 - FTA/ORSI
   //
   // If the option "save" is set, then the new database is saved in the file Runlist.csv
   // (see method WriteRunListFile ).
   //
   // HOW USE THIS METHODE?
   //
   // gDataRepositoryManager->GetRepository("ccali")->cd();
   // KVDataSet *ds = gDataRepository->GetDataSetManager()->GetDataSet("INDRA_e494s");
   // ds->cd();
   // ((KVIVDB *)gIndraDB)->ReadAndCorrectVamosScalers("527-532","save");

   KVDataSet* ds = gDataSet;

   gDataSet = NULL; // faster computing if gDataSet is null (no KVMultiDetArray built and loaded)

   // By default, scaler buffers are ignored during reading of raw data file.
   // Change the appropriate environment variable to be able to read the scaler buffers.
   TString sbm = gEnv->GetValue("KVGANILDataReader.ScalerBuffersManagement", "kReportScaler");
   gEnv->SetValue("KVGANILDataReader.ScalerBuffersManagement", "kReportScaler");

   // list of runs to be processed
   KVNumberList rl(runlist);
   rl.Begin();
   Int_t r_counter = 0; // counter of runs
   while (!rl.End()) { // loop over the runs

      Int_t run = rl.Next();
      r_counter ++;

      // Open raw data file for current run
      TString fname = ds->GetFullPathToRunfile("raw", run);
      if (fname.IsNull()) {
         Error("ReadAndCorrectVamosScalers", "Error in <ReadScalersWithKaliVeda>: no raw data file found for run %d (dataset: %s, repository: %s)", run, ds->GetLabel(), ds->GetRepository()->GetName());
         continue;
      }
      KVGANILDataReader runfile(fname.Data());


      UInt_t n_event  = 0;    // number of entries in the current data file
      Int_t n_buffers = 0;    // number of scaler buffer in the current data file
      Int_t n_buffers_ok = 0; // number of scaler buffer in the current data file with status == 1 (ok)

      UInt_t first_scaler_count[40];// first buffer of scalers
      UInt_t scaler_count[40];      // last buffer of scalers
      Int_t first_event_count = 0;
      Int_t event_count = 0;

      while (runfile.GetNextEvent()) {  // loop over the events
         if (runfile.HasScalerBuffer()) {
            n_buffers++;
            Int_t n_scalers = runfile.GetNumberOfScalers();
            if (n_scalers != 40) {
               // VAMOS has 40 scalers
               Error("ReadAndCorrectVamosScalers", "number of scalers != 40 for run %d at event %d", run, n_event);
               continue;
            }

            Int_t  status = runfile.GetScalerStatus(0);

            if (status == 1) { // acquisition on
               n_buffers_ok++;
               for (Int_t i = 0; i < n_scalers; i++) { // loop over VAMOS scalers
                  // store scalers of the first buffer
                  if (n_buffers_ok == 1) first_scaler_count[i] = runfile.GetScalerCount(i);
                  // store scalers of the last buffer
                  else scaler_count[i]  = runfile.GetScalerCount(i);
               } // loop over VAMOS scalers

               // store number of event of the first buffer
               if (n_buffers_ok == 1) first_event_count = n_event;
               // store number of event of the last buffer
               else event_count = n_event;

            } // acquistion on
         } // has scaler buffer
         else n_event++;
      } // loop over the events

      UInt_t total_event_count = n_event;
      //UInt_t total_event_count = runfile.GetEventCount();

      //--------------------------------------------------
      // Correction of scalers: last_buffer - first_buffer
      // and load scalers in the database
      //--------------------------------------------------

      KVINDRADBRun* db_run = GetRun(run);

      if (n_buffers_ok > 0) {

         if (n_buffers_ok > 1) {
            // the new scaler is the difference between the last and the first buffer
            // having status equal 1.
            for (Int_t i = 0; i < 40; i++) scaler_count[i] -= first_scaler_count[i];
            event_count -= first_event_count;
         }
         if ((n_buffers_ok == 1) || (scaler_count[27] <= 0 /*FTA==0*/)) {
            // if there is only 1 buffer then we assume that once it will be
            // normalized to the total_event_count then the new value will be
            // a good approximation
            for (Int_t i = 0; i < 40; i++) scaler_count[i] = first_scaler_count[i];
            event_count = scaler_count[27];
         }

         // correction factor to normalize the scalers to the total_event_count
         Double_t corrFactor = (event_count ? (1.*total_event_count) / event_count : 1.);
         //Double_t corrFactor = (scaler_count[27] ? (1.*total_event_count)/scaler_count[27] : 1. ); // total_event/FTA

         for (Int_t i = 0; i < 40; i++) {
            scaler_count[i] *= corrFactor;
            // set the scaler in the database
            db_run->SetScaler(GetVamosScalerName(i), scaler_count[i]);
         }

         // set VAMOS dead time in the database DT = 1-FTA/ORSI
         Double_t DT = 100.*(1. - (1.*scaler_count[27]) / scaler_count[37]);
         db_run->Set("DT", DT);

//       db_run->SetScaler("totalEvent",total_event_count);
//       db_run->SetScaler("EventCount",event_count);
//       db_run->SetScaler("NbuffersOK",n_buffers_ok);
      }

      db_run->Print();

      cout << endl;
      Info("ReadAndCorrectVamosScalers", "processing ------> %.0f %s  \n", (100.*r_counter) / rl.GetNValues(), "%");

   }

   // go back the the initial configuration
   ds->cd();
   gEnv->SetValue("KVGANILDataReader.ScalerBuffersManagement", sbm.Data());

   // write the run list file if the option is set
   TString option = opt;
   option.ToLower();
   if (option == "save") WriteRunListFile();
}
