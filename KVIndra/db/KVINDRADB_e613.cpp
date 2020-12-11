//Created by KVClassFactory on Mon Oct 24 14:38:16 2011
//Author: bonnet

#include "KVINDRADB_e613.h"
#include "KVINDRA.h"
#include "KVFileReader.h"
#include "TObjArray.h"
#include "KVDBParameterSet.h"
#include "KVDBChIoPressures.h"

using namespace std;

ClassImp(KVINDRADB_e613)

KVINDRADB_e613::KVINDRADB_e613()
{
   // Default constructor
}

//___________________________________________________________________________
KVINDRADB_e613::KVINDRADB_e613(const Char_t* name): KVINDRADB(name)
{
   Info("KVINDRADB_e613", "Hi coquine, tu es sur la manip e613 ...");
}

//___________________________________________________________________________
KVINDRADB_e613::~KVINDRADB_e613()
{
   // Destructor
}

//___________________________________________________________________________
void KVINDRADB_e613::Build()
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

   ReadRunList(runlist_fullpath.Data());
   //new style runlist
   if (IsNewRunList()) {
      ReadNewRunList();
   };

   ReadSystemList();
   ReadChIoPressures();
   ReadGainList();
   ReadPedestalList();
   ReadChannelVolt();
   ReadVoltEnergyChIoSi();
   ReadCalibCsI();
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
void KVINDRADB_e613::ReadChIoPressures()
{
   //Read ChIo pressures for different run ranges and enter into database.
   //Format of file is:
   //
   //# some comments
   //#which start with '#'
   //RunRange  6001 6018
   //2_3 50.0
   //4_5 50.0
   //6_7 50.0
   //8_12   30.0
   //13_17  30.0
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

   Bool_t prev_rr = kFALSE;     //was the previous line a run range indication ?
   Bool_t read_pressure = kFALSE; // have we read any pressures recently ?
   KVNumberList nl;
   KVDBChIoPressures* parset = 0;
   TList* par_list = new TList();
   TObjArray* toks = 0;
   //any ChIo not in list is assumed absent (pressure = 0)

   Float_t pressure[5] = { 0, 0, 0, 0, 0 };

   while (fin.good()) {         // parcours du fichier

      sline.ReadLine(fin);
      if (sline.BeginsWith("#")) {

      }
      else if (sline.BeginsWith("RunRange")) {    // run range found
         if (!prev_rr) {        // New set of run ranges to read

            //have we just finished reading some pressures ?
            if (read_pressure) {
               parset = new KVDBChIoPressures(pressure);
               GetTable("ChIo Pressures")->AddRecord(parset);
               par_list->Add(parset);
               LinkListToRunRange(par_list, nl);
               par_list->Clear();
               for (int zz = 0; zz < 5; zz++) pressure[zz] = 0.;
               read_pressure = kFALSE;
            }
         }
         toks = sline.Tokenize("\t");

         if (toks->GetEntries() != 2) {
            Error("ReadChIoPressures", "Pb de format, il faut RunRange\tRun1-Run2 ... ");
            return;
         }

         prev_rr = kTRUE;
         nl.SetList(((TObjString*)toks->At(1))->GetString().Data());
         delete toks;
      }                         // Run Range found
      else if (fin.eof()) {          //fin du fichier
         //have we just finished reading some pressures ?
         if (read_pressure) {
            parset = new KVDBChIoPressures(pressure);
            GetTable("ChIo Pressures")->AddRecord(parset);
            par_list->Add(parset);
            LinkListToRunRange(par_list, nl);
            par_list->Clear();
            for (int zz = 0; zz < 5; zz++) pressure[zz] = 0.;
            read_pressure = kFALSE;
         }
      }
      else {
         prev_rr = kFALSE;

         toks = sline.Tokenize("\t");
         if (toks->GetEntries() != 2) {
            Error("ReadChIoPressures", "Pb de format, il faut numero de la chio (ex 2_3)\tpression");
            return;
         }

         TString chio = ((TObjString*)toks->At(0))->String();
         TString press = ((TObjString*)toks->At(1))->String();

         printf("%s %lf\n", chio.Data(), press.Atof());
         delete toks;

         read_pressure = kTRUE;

         if (chio == "2_3")         pressure[0] = press.Atof();
         else if (chio == "4_5")    pressure[1] = press.Atof();
         else if (chio == "6_7")    pressure[2] = press.Atof();
         else if (chio == "8_12")   pressure[3] = press.Atof();
         else if (chio == "13_17") pressure[4] = press.Atof();
         else {
            printf("#%s# ne correspond a rien\n", chio.Data());
            read_pressure = kFALSE;
         }
      }                         //line with ChIo pressure data
   }                            //parcours du fichier
   delete par_list;
   fin.close();
}

//____________________________________________________________________________
void KVINDRADB_e613::ReadGainList()
{
   // Read the file listing any detectors whose gain value changes during experiment
   // need description of INDRA geometry
   // information are in  [dataset name].INDRADB.Gains:    ...
   //

   //need description of INDRA geometry
   if (!gIndra) {
      KVMultiDetArray::MakeMultiDetector(fDataSet.Data());
   }
   //gIndra exists, but has it been built ?
   if (!gIndra->IsBuilt())
      gIndra->Build();

   KVFileReader flist;
   TString fp;
   if (!KVBase::SearchKVFile(GetCalibFileName("Gains"), fp, fDataSet.Data())) {
      Error("ReadGainList", "Fichier %s, inconnu au bataillon", GetCalibFileName("Gains"));
      return;
   }

   if (!flist.OpenFileToRead(fp.Data())) {
      //Error("ReadGainList","Error opening file named %s",fp.Data());
      return;
   }
   Info("ReadGainList()", "Reading gains ...");
   //Add table for gains
   fGains = AddTable("Gains", "Gains of detectors during runs");

   while (flist.IsOK()) {

      flist.ReadLine(".");
      if (! flist.GetCurrentLine().IsNull()) {
         unique_ptr<TObjArray> toks(flist.GetReadPar(0).Tokenize("_"));
         Int_t nt = toks->GetEntries();
         Int_t ring = -1;
         unique_ptr<KVSeqCollection> sl;
         TString det_type = "";
         if (nt <= 1) {
            Warning("ReadGainList", "format non gere");
         }
         else {
            //format : Gain_[det_type]_R[RingNumber].dat
            //exemple  : Gain_SI_R07.dat
            det_type = ((TObjString*)toks->At(1))->GetString();
            //on recupere les detecteurs par type
            sl.reset(gIndra->GetDetectors()->GetSubListWithType(det_type.Data()));
            if (nt == 2) {
               ring = 0;
            }
            else if (nt == 3) {
               sscanf(((TObjString*)toks->At(2))->GetString().Data(), "R%d", &ring);
            }
            else {
               Warning("ReadGainList", "format non gere");
            }
            if (ring != 0)
               sl.reset(sl->GetSubListWithMethod(Form("%d", ring), "GetRingNumber"));
         }

         if (sl.get()) {
            KVDBParameterSet* par(nullptr);
            TIter it(sl.get());
            TObject* obj(nullptr);
            KVNumberList nl;
            KVFileReader ffile;
            if (KVBase::SearchKVFile(flist.GetCurrentLine().Data(), fp, fDataSet.Data())) {
               ffile.OpenFileToRead(fp.Data());
               //Info("ReadGainList","Lecture de %s",fp.Data());
               while (ffile.IsOK()) {
                  ffile.ReadLine(":");
                  if (! ffile.GetCurrentLine().IsNull()) {

                     toks.reset(ffile.GetReadPar(0).Tokenize("."));
                     //liste des runs ...
                     nl.SetList(((TObjString*)toks->At(1))->GetString());
                     // ... associee a la valeur de gain
                     Double_t gain = ffile.GetDoubleReadPar(1);

                     printf("%s Ring %d -> Runs=%s Gain=%1.3lf\n", det_type.Data(), ring, nl.AsString(), gain);

                     it.Reset();
                     while ((obj = it.Next())) {
                        par = new KVDBParameterSet(obj->GetName(), "Gains", 1);
                        par->SetParameter(gain);
                        fGains->AddRecord(par);
                        LinkRecordToRunRange(par, nl);
                     }
                  }
               }
            }
            ffile.CloseFile();
         }
      }
   }

   flist.CloseFile();

}
//____________________________________________________________________________
void KVINDRADB_e613::ReadPedestalList()
{
   //Read the names of pedestal files to use for each run range, found
   //in file with name defined by the environment variable:
   //   [dataset name].INDRADB.Pedestals:    ...
   //Actuellement lecture d un seul run de piedestal
   //et donc valeur unique pour l ensemble des runs

   KVFileReader flist;
   TString fp;
   if (!KVBase::SearchKVFile(gDataSet->GetDataSetEnv("INDRADB.Pedestals", ""), fp, gDataSet->GetName())) {
      Error("ReadPedestalList", "Fichier %s, inconnu au bataillon", gDataSet->GetDataSetEnv("INDRADB.Pedestals", ""));
      return;
   }
   fPedestals->SetTitle("Values of pedestals");
   if (!flist.OpenFileToRead(fp.Data())) {
      return;
   }
   TEnv* env = 0;
   TEnvRec* rec = 0;
   KVDBParameterSet* par = 0;

   KVNumberList default_run_list = GetRunList();
   Info("ReadPedestalList", "liste des runs par defaut %s", default_run_list.AsString());

   while (flist.IsOK()) {
      flist.ReadLine(NULL);
      KVString file = flist.GetCurrentLine();
      KVNumberList nl;
      if (file != "" && !file.BeginsWith('#')) {
         if (KVBase::SearchKVFile(file.Data(), fp, gDataSet->GetName())) {
            Info("ReadPedestalList", "Lecture de %s", fp.Data());
            env = new TEnv();
            env->ReadFile(fp.Data(), kEnvAll);
            TIter it(env->GetTable());
            while ((rec = (TEnvRec*)it.Next())) {
               if (!strcmp(rec->GetName(), "RunRange")) {
                  nl.SetList(rec->GetValue());
               }
               else {
                  par = new KVDBParameterSet(rec->GetName(), "Piedestal", 1);
                  par->SetParameter(env->GetValue(rec->GetName(), 0.0));
                  fPedestals->AddRecord(par);
                  LinkRecordToRunRange(par, default_run_list);
               }
            }
            delete env;

         }
      }
   }
   Info("ReadPedestalList", "End of reading");
}

//____________________________________________________________________________
void KVINDRADB_e613::ReadChannelVolt()
{
   //Read the names of pedestal files to use for each run range, found
   //in file with name defined by the environment variable:
   //   [dataset name].INDRADB.Pedestals:    ...

   //need description of INDRA geometry
   if (!gIndra) {
      KVMultiDetArray::MakeMultiDetector(fDataSet.Data());
   }
   //gIndra exists, but has it been built ?
   if (!gIndra->IsBuilt())
      gIndra->Build();

   KVNumberList default_run_list = GetRunList();
   Info("ReadChannelVolt", "liste des runs par defaut %s", default_run_list.AsString());

   KVFileReader flist;
   TString fp;
   if (!KVBase::SearchKVFile(gDataSet->GetDataSetEnv("INDRADB.ElectronicCalibration", ""), fp, gDataSet->GetName())) {
      Error("ReadChannelVolt", "Fichier %s, inconnu au bataillon", gDataSet->GetDataSetEnv("INDRADB.ElectronicCalibration", ""));
      return;
   }

   if (!flist.OpenFileToRead(fp.Data())) {
      return;
   }

   TEnv* env = 0;
   TEnvRec* rec = 0;
   KVDBParameterSet* par = 0;
   KVDBParameterSet* par_pied = 0;
   TObjArray* toks = 0;
   Double_t a0, a1, a2; //parametre du polynome d ordre 2
   Double_t gain = 0; //valeur du gain de reference
   Double_t dum2 = -2;
   Double_t pied = 0; //valeur du piedestal

   KVINDRADBRun* dbrun = 0;
   KVINDRADBRun* dbpied = 0;
   KVNameValueList ring_run;
   while (flist.IsOK()) {
      flist.ReadLine(NULL);
      KVString file = flist.GetCurrentLine();
      KVNumberList nl;
      if (file != "" && !file.BeginsWith('#')) {
         if (KVBase::SearchKVFile(file.Data(), fp, gDataSet->GetName())) {
            Info("ReadChannelVolt", "Lecture de %s", fp.Data());
            TString cal_type;
            TString sgain = "GG";
            if (fp.Contains("PGtoVolt")) sgain = "PG";
            cal_type = "Channel-Volt " + sgain;

            env = new TEnv();
            env->ReadFile(fp.Data(), kEnvAll);
            TIter it(env->GetTable());
            while ((rec = (TEnvRec*)it.Next())) {
               KVNumberList nring;
               TString srec(rec->GetName());
               //On recupere le run reference pour lequel a ete fait la calibration
               if (srec.BeginsWith("Ring.")) {
                  srec.ReplaceAll("Ring.", "");
                  nring.SetList(srec);
                  nring.Begin();
                  while (!nring.End()) {
                     Int_t rr = nring.Next();
                     ring_run.SetValue(Form("%d", rr), TString(rec->GetValue()).Atoi());
                     Info("ReadChannelVolt", "Couronne %d, run associee %d", rr, TString(rec->GetValue()).Atoi());
                  }
               }
               else if (srec.BeginsWith("Pedestal")) {
                  srec.ReplaceAll("Pedestal.", "");
                  dbpied = GetRun(TString(rec->GetValue()).Atoi());
               }
               else {

                  TString spar(rec->GetValue());
                  toks = spar.Tokenize(",");
                  if (toks->GetEntries() >= 3) {
                     a0 = ((TObjString*)toks->At(1))->GetString().Atof();
                     a1 = ((TObjString*)toks->At(2))->GetString().Atof();
                     a2 = ((TObjString*)toks->At(3))->GetString().Atof();
                     par_pied = ((KVDBParameterSet*)dbpied->GetLink("Pedestals", Form("%s_%s", rec->GetName(), sgain.Data())));
                     if (par_pied)
                        pied = par_pied->GetParameter();
                     //Fit Canal-Volt realise avec soustraction piedestal
                     //chgmt de variable pour passer de (Canal-piedestal) a Canal brut
                     a0 = a0 - pied * a1 + pied * pied * a2;
                     a1 = a1 - 2 * pied * a2;
                     //a2 inchange
                     //On recupere le run de ref, pour avoir le gain associe a chaque detecteur
                     KVINDRADetector* det = (KVINDRADetector*)gIndra->GetDetector(rec->GetName());
                     if (det) {

                        Int_t runref = ring_run.GetIntValue(Form("%d", det->GetRingNumber()));
                        if (!dbrun) {
                           dbrun = GetRun(runref);
                        }
                        else if (dbrun->GetNumber() != runref) {
                           dbrun = GetRun(runref);
                        }
                        if (!dbrun) {
                           Warning("ReadChannelVolt", "Pas de run reference numero %d", runref);
                        }
                        //le gain est mis comme troisieme parametre
                        KVDBParameterSet* pargain = ((KVDBParameterSet*) dbrun->GetLink("Gains", rec->GetName()));
                        if (pargain) {
                           gain = pargain->GetParameter(0);
                        }
                        else {
                           Info("ReadChannelVolt", "pas de gain defini pour le run %d et le detecteur %s", runref, rec->GetName());
                        }

                        //Si tout est dispo on enregistre la calibration pour ce detecteur
                        //
                        par = new KVDBParameterSet(Form("%s_%s", rec->GetName(), sgain.Data()), cal_type, 5);
                        par->SetParameters(a0, a1, a2, gain, dum2);

                        fChanVolt->AddRecord(par);
                        LinkRecordToRunRange(par, default_run_list);
                     }
                  }
                  else {
                     a0 = a1 = a2 = gain = 0;
                     Warning("ReadChannelVolt", "Pb de format %s", rec->GetValue());
                  }
                  delete toks;
               }
            }
            delete env;

         }
      }
   }
   Info("ReadChannelVolt", "End of reading");

}

//____________________________________________________________________________
void KVINDRADB_e613::ReadVoltEnergyChIoSi()
{
   //Read Volt-Energy(MeV) calibrations for ChIo and Si detectors.
   //The parameter filename is taken from the environment variable
   //        [dataset name].INDRADB.ChIoSiVoltMeVCalib:

   KVFileReader flist;
   TString fp;
   if (!KVBase::SearchKVFile(gDataSet->GetDataSetEnv("INDRADB.ChIoSiVoltMeVCalib", ""), fp, gDataSet->GetName())) {
      Error("ReadVoltEnergyChIoSi", "Fichier %s, inconnu au bataillon", gDataSet->GetDataSetEnv("INDRADB.ChIoSiVoltMeVCalib", ""));
      return;
   }

   if (!flist.OpenFileToRead(fp.Data())) {
      return;
   }
   TEnv* env = 0;
   TEnvRec* rec = 0;
   KVDBParameterSet* par = 0;
   TObjArray* toks = 0;

   KVNumberList default_run_list = GetRunList();
   Info("ReadVoltEnergyChIoSi", "liste des runs par defaut %s", default_run_list.AsString());

   while (flist.IsOK()) {
      flist.ReadLine(NULL);
      KVString file = flist.GetCurrentLine();
      KVNumberList nl;
      if (file != "" && !file.BeginsWith('#')) {
         if (KVBase::SearchKVFile(file.Data(), fp, gDataSet->GetName())) {
            Info("ReadPedestalList", "Lecture de %s", fp.Data());
            env = new TEnv();
            env->ReadFile(fp.Data(), kEnvAll);
            TIter it(env->GetTable());
            while ((rec = (TEnvRec*)it.Next())) {

               Double_t a0 = 0, a1 = 1, chi = 1;
               TString spar(rec->GetValue());
               toks = spar.Tokenize(",");
               if (toks->GetEntries() >= 2) {
                  a0 = ((TObjString*)toks->At(1))->GetString().Atof();
                  a1 = ((TObjString*)toks->At(2))->GetString().Atof();
               }
               delete toks;

               par = new KVDBParameterSet(rec->GetName(), "Volt-Energy", 3);
               par->SetParameters(a0, a1, chi);
               fVoltMeVChIoSi->AddRecord(par);
               LinkRecordToRunRange(par, default_run_list);

            }
            delete env;
         }
      }
   }
   Info("ReadVoltEnergyChIoSi", "End of reading");

}
