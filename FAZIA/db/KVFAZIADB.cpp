//Created by KVClassFactory on Tue Jan 20 11:52:49 2015
//Author: ,,,

#include "KVFAZIADB.h"
#include "KVNumberList.h"
#include "KVDataSetManager.h"
#include "KVRunListLine.h"
#include "KVFileReader.h"
#include "KVDBParameterList.h"
#include "KVDBParameterSet.h"

KVFAZIADB* gFaziaDB;

ClassImp(KVFAZIADB)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVFAZIADB</h2>
<h4>database for FAZIA detector</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

void KVFAZIADB::init()
{
   //default initialisations
   kFirstRun = 0;
   kLastRun = 0;

   fRuns = AddTable("Runs", "List of available runs");
   fRuns->SetDefaultFormat("Run %d"); // default format for run names
   fSystems = AddTable("Systems", "List of available systems");
   fExceptions = AddTable("Exceptions", "List signals with different PSA parameters");
   fCalibrations = AddTable("Calibrations", "Available calibration for FAZIA detectors");
}

KVFAZIADB::KVFAZIADB(const Char_t* name): KVDataBase(name,
         "FAZIA experiment parameter database")
{
   init();
}


KVFAZIADB::KVFAZIADB(): KVDataBase("KVFAZIADB",
                                      "FAZIA experiment parameter database")
{
   init();
}

//___________________________________________________________________________
KVFAZIADB::~KVFAZIADB()
{
   //reset global pointer gFaziaDB if it was pointing to this database

   if (gFaziaDB == this)
      gFaziaDB = 0;
}

//___________________________________________________________________________
void KVFAZIADB::cd()
{
   KVDataBase::cd();
   gFaziaDB = this;

}
//__________________________________________________________________________________________________________________

Bool_t KVFAZIADB::OpenCalibFile(const Char_t* type, std::ifstream& fs) const
{
   //Find and open calibration parameter file of given type. Return kTRUE if all OK.
   //types are defined in $KVROOT/KVFiles/.kvrootrc by lines such as
   //
   //# Default name for file describing systems for each dataset.
   //INDRADB.Systems:     Systems.dat
   //
   //A file with the given name will be looked for in $KVROOT/KVFiles/name_of_dataset
   //where name_of_dataset is given by KVDataBase::fDataSet.Data()
   //i.e. the name of the associated dataset.
   //
   //Filenames specific to a given dataset may also be defined:
   //
   //INDRA_camp5.INDRADB.Pedestals:      Pedestals5.dat
   //
   //where 'INDRA_camp5' is the name of the dataset in question.
   //GetDBEnv() always returns the correct filename for the currently active dataset.

   return KVBase::SearchAndOpenKVFile(GetDBEnv(type), fs, fDataSet.Data());
}
//__________________________________________________________________________________________________________________

const Char_t* KVFAZIADB::GetDBEnv(const Char_t* type) const
{
   //Will look for gEnv->GetValue name "name_of_dataset.INDRADB.type"
   //then "INDRADB.type" if no dataset-specific value is found.

   if (!gDataSetManager)
      return "";
   KVDataSet* ds = gDataSetManager->GetDataSet(fDataSet.Data());
   if (!ds)
      return "";
   return ds->GetDataSetEnv(Form("FAZIADB.%s", type));
}

//_____________________________________________________________________
void KVFAZIADB::LinkRecordToRunRange(KVDBRecord* rec, UInt_t first_run,
                                     UInt_t last_run)
{
   //If the KVDBRecord 'rec' (i.e. set of calibration parameters, reaction system, etc.) is
   //associated to, or valid for, a range of runs, we use this method in order to link the record
   //and the runs. The list of associated runs will be kept with the record, and each of the runs
   //will have a link to the record.

   for (UInt_t ii = first_run; ii <= last_run; ii++) {
      LinkRecordToRun(rec, ii);
   }
}
//_____________________________________________________________________
void KVFAZIADB::LinkRecordToRunRange(KVDBRecord* rec, KVNumberList nl)
{
   //If the KVDBRecord 'rec' (i.e. set of calibration parameters, reaction system, etc.) is
   //associated to, or valid for, a range of runs, we use this method in order to link the record
   //and the runs. The list of associated runs will be kept with the record, and each of the runs
   //will have a link to the record.
   nl.Begin();
   while (!nl.End()) {
      Int_t rr = nl.Next();
      //Info("LinkRecordToRunRange","run number %d",rr);
      LinkRecordToRun(rec, rr);
   }
}

//_____________________________________________________________________
void KVFAZIADB::LinkRecordToRun(KVDBRecord* rec, Int_t rnumber)
{

   KVFAZIADBRun* run = GetRun(rnumber);
   if (run)
      rec->AddLink("Runs", run);

}

//_____________________________________________________________________
void KVFAZIADB::LinkRecordToRunRanges(KVDBRecord* rec, UInt_t rr_number,
                                      UInt_t run_ranges[][2])
{
   //Call LinkRecordToRunRange for a set of run ranges stored in the two-dimensional array
   //in the following way:
   //      run_ranges[0][0] = first run of first run range
   //      run_ranges[0][1] = last run of first run range
   //      run_ranges[1][0] = first run of second run range etc. etc.
   //rr_number is the number of run ranges in the array

   for (UInt_t i = 0; i < rr_number; i++) {
      LinkRecordToRunRange(rec, run_ranges[i][0], run_ranges[i][1]);
   }
}

//______________________________________________________________________________
void KVFAZIADB::LinkListToRunRanges(TList* list, UInt_t rr_number,
                                    UInt_t run_ranges[][2])
{
   //Link the records contained in the list to the set of runs (see LinkRecordToRunRanges).

   if (!list) {
      Error("LinkListToRunRanges",
            "NULL pointer passed for parameter TList");
      return;
   }
   if (list->GetSize() == 0) {
      Error("LinkListToRunRanges(TList*,UInt_t,UInt_t*)",
            "The list is empty");
      return;
   }
   TIter next(list);
   KVDBRecord* rec;
   for (UInt_t ru_ra = 0; ru_ra < rr_number; ru_ra++) {
      UInt_t first_run = run_ranges[ru_ra][0];
      UInt_t last_run = run_ranges[ru_ra][1];
      for (UInt_t i = first_run; i <= last_run; i++) {
         KVFAZIADBRun* run = GetRun(i);
         while ((rec = (KVDBRecord*) next())) {
            if (run)
               rec->AddLink("Runs", run);
         }
         next.Reset();
      }
   }
}
//______________________________________________________________________________
void KVFAZIADB::LinkListToRunRange(TList* list, KVNumberList nl)
{
   //Link the records contained in the list to the set of runs (see LinkRecordToRunRanges).

   if (!list) {
      Error("LinkListToRunRange",
            "NULL pointer passed for parameter TList");
      return;
   }
   if (list->GetSize() == 0) {
      Error("LinkListToRunRange(TList*,KVNumberList)",
            "The list is empty");
      return;
   }
   TIter next(list);
   KVDBRecord* rec;
   while ((rec = (KVDBRecord*) next())) {
      LinkRecordToRunRange(rec, nl);
   }
}

//____________________________________________________________________________
void KVFAZIADB::ReadSystemList()
{
   //Reads list of systems with associated run ranges, creates KVDBSystem
   //records for these systems, and links them to the appropriate KVFAZIADBRun
   //records using LinkListToRunRanges.
   //
   //There are 2 formats for the description of systems:
   //
   //+129Xe + natSn 25 MeV/A           '+' indicates beginning of system description
   //129 54 119 50 0.1 25.0            A,Z of projectile and target, target thickness (mg/cm2), beam energy (MeV/A)
   //Run Range : 614 636               runs associated with system
   //Run Range : 638 647               runs associated with system
   //
   //This is sufficient in the simple case where the experiment has a single
   //layer target oriented perpendicular to the beam. However, for more
   //complicated targets we can specify as follows :
   //
   //+155Gd + 238U 36 MeV/A
   //155 64 238 92 0.1 36.0
   //Target : 3 0.0                    target with 3 layers, angle 0 degrees
   //C 0.02                            1st layer : carbon, 20 g/cm2
   //238U 0.1                          2nd layer : uranium-238, 100 g/cm2
   //C 0.023                           3rd layer : carbon, 23 g/cm2
   //Run Range : 770 804
   //
   //Lines beginning '#' are comments.


   std::ifstream fin;
   if (OpenCalibFile("Systems", fin)) {
      Info("ReadSystemList()", "Reading Systems parameters ...");

      TString line;

      char next_char = fin.peek();
      while (next_char != '+' && fin.good()) {
         line.ReadLine(fin, kFALSE);
         next_char = fin.peek();
      }

      while (fin.good() && !fin.eof() && next_char == '+') {
         KVDBSystem* sys = new KVDBSystem("NEW SYSTEM");
         AddSystem(sys);
         sys->Load(fin);
         next_char = fin.peek();
      }
      fin.close();
   } else {
      Error("ReadSystemList()", "Could not open file %s",
            GetCalibFileName("Systems"));
   }
   // if any runs are not associated with any system
   // we create an 'unknown' system and associate it to all runs
   KVDBSystem* sys = 0;
   TIter nextRun(GetRuns());
   KVFAZIADBRun* run;
   while ((run = (KVFAZIADBRun*)nextRun())) {
      if (!run->GetSystem()) {
         if (!sys) {
            sys = new KVDBSystem("[unknown]");
            AddSystem(sys);
         }
         sys->AddRun(run);
      }
   }

   // rehash the record table now that all names are set
   fSystems->Rehash();
}

void KVFAZIADB::WriteSystemsFile() const
{
   //Write the 'Systems.dat' file for this database.
   //The actual name of the file is given by the value of the environment variable
   //[dataset_name].INDRADB.Systems (if it exists), otherwise the value of
   //INDRADB.Systems is used. The file is written in the
   //$KVROOT/[dataset_directory] directory.

   std::ofstream sysfile;
   KVBase::SearchAndOpenKVFile(GetDBEnv("Systems"), sysfile, fDataSet.Data());
   TIter next(GetSystems());
   KVDBSystem* sys;
   TDatime now;
   sysfile << "# " << GetDBEnv("Systems") << " file written by "
           << ClassName() << "::WriteSystemsFile on " << now.AsString() << std::endl;
   std::cout << GetDBEnv("Systems") << " file written by "
             << ClassName() << "::WriteSystemsFile on " << now.AsString() << std::endl;
   while ((sys = (KVDBSystem*)next())) {
      if (strcmp(sys->GetName(), "[unknown]")) { //do not write dummy 'unknown' system
         sys->Save(sysfile);
         sysfile << std::endl;
      }
   }
   sysfile.close();
}

//__________________________________________________________________________________________________________________

void KVFAZIADB::Save(const Char_t* what)
{
   //Save (in the appropriate text file) the informations on:
   // what = "Systems" : write Systems.dat file
   // what = "Runlist" : write Runlist.csv
   if (!strcmp(what, "Systems")) WriteSystemsFile();
   else if (!strcmp(what, "Runlist")) WriteRunListFile();
}

void KVFAZIADB::WriteRunListFile() const
{
   //Write a file containing a line describing each run in the database.
   //The delimiter symbol used in each line is '|' by default.
   //The first line of the file will be a header description, given by calling
   //KVFAZIADBRun::WriteRunListHeader() for the first run in the database.
   //Then we call KVFAZIADBRun::WriteRunListLine() for each run.
   //These are virtual methods redefined by child classes of KVFAZIADBRun.

   std::ofstream rlistf;
   KVBase::SearchAndOpenKVFile(GetDBEnv("Runlist"), rlistf, fDataSet.Data());
   TDatime now;
   rlistf << "# " << GetDBEnv("Runlist") << " file written by "
          << ClassName() << "::WriteRunListFile on " << now.AsString() << std::endl;
   std::cout << GetDBEnv("Runlist") << " file written by "
             << ClassName() << "::WriteRunListFile on " << now.AsString() << std::endl;
   TIter next_run(GetRuns());
   //write header in file
   ((KVFAZIADBRun*) GetRuns()->At(0))->WriteRunListHeader(rlistf, GetDBEnv("Runlist.Separator")[0]);
   KVFAZIADBRun* run;
   while ((run = (KVFAZIADBRun*) next_run())) {

      run->WriteRunListLine(rlistf, GetDBEnv("Runlist.Separator")[0]);

   }
   rlistf.close();
}

//__________________________________________________________________________________________________________________

void KVFAZIADB::Build()
{

   //Use KVINDRARunListReader utility subclass to read complete runlist

   //get full path to runlist file, using environment variables for the current dataset

   TString runlist_fullpath;
   KVBase::SearchKVFile(GetDBEnv("Runlist"), runlist_fullpath, fDataSet.Data());
   kFirstRun = 999999;
   kLastRun = 0;

   ReadNewRunList();
   ReadSystemList();
   ReadExceptions();
   ReadComments();
   ReadCalibrationFiles();
}


//____________________________________________________________________________
void KVFAZIADB::ReadNewRunList()
{
   //Read new-style runlist (written using KVFAZIADBRun v.10 or later)

   std::ifstream fin;
   if (!OpenCalibFile("Runlist", fin)) {
      Error("ReadNewRunList()", "Could not open file %s",
            GetCalibFileName("Runlist"));
      return;
   }

   Info("ReadNewRunList()", "Reading run parameters ...");

   KVString line;
   KVFAZIADBRun* run;
   TObjArray* toks = 0;

   while (fin.good() && !fin.eof()) {
      line.ReadLine(fin);
      if (line.Length() > 1 && !line.BeginsWith("#") && !line.BeginsWith("Version")) {
         run = new KVFAZIADBRun;
         run->SetTrigger(0);
         run->SetNumberOfTriggerBlocks(0);
         run->SetDeadTime(0);
         run->SetTriggerRate(0);

         toks = line.Tokenize("|");
         for (Int_t ii = 0; ii < toks->GetEntries(); ii += 1) {
            KVString couple = ((TObjString*)toks->At(ii))->GetString();
            couple.Begin("=");
            KVString name = couple.Next();
            name = name.Strip(TString::kBoth);
            KVString value = "";
            if (!couple.End()) {
               value = couple.Next();
               value = value.Strip(TString::kBoth);
            }
            if (name == "run") {
               run->SetNumber(value.Atoi());
            } else if (name == "read events") {
               run->SetEvents(value.Atoi());
            } else if (name == "good events") {
               run->SetGoodEvents(value.Atoi());
            } else if (name == "starting date") {
               run->SetStartDate(value);
            } else if (name == "stopping date") {
               run->SetEndDate(value);
            } else if (name == "aqcuisition status") {
               run->SetACQStatus(value);
            } else if (name == "wrong number of blocks") { //events rejected due to the wrong number of blocks
               run->SetError_WrongNumberOfBlocks(value.Atoi());
            } else if (name == "block errors") { //events rejected due to internal error in one block
               run->SetError_InternalBlockError(value.Atoi());
            } else if (name == "nfiles") { //number of acquisition files
               run->SetNumberOfAcqFiles(value.Atoi());
            } else if (name == "duration") { //duration in seconds of the run
               run->SetDuration(value.Atof());
            } else if (name == "frequency") { //number of evts per seconds (aquisition rate)
               run->SetFrequency(value.Atof());
            } else if (name == "triggerrate") { //trigger rate
               run->SetTriggerRate(value.Atof());
            } else if (name == "mtrigger") { //trigger multiplicity
               run->SetTrigger(value.Atof());
            } else if (name == "deadtime") { //deadtime of the acquisition between 0 and 1
               run->SetDeadTime(value.Atof());
            } else if (name == "trig info") { //number of trigger block in the acquisition file
               run->SetNumberOfTriggerBlocks(value.Atoi());
            } else {
               //Info("ReadNewRunList","Unknown field %s=%s",name.Data(),value.Data());
            }
         }
         delete toks;
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
void KVFAZIADB::ReadDBFile(TString file)
{
   KVFileReader fr;
   fr.OpenFileToRead(file.Data());
   KVFAZIADBRun* run = new KVFAZIADBRun();
   KVFAZIADBRun* ref = new KVFAZIADBRun();

   KVNumberList lmismatch;
   KVNumberList lok;
   KVNumberList lnotpresent;
   KVString datadir = "";

   FILE* fok = fopen("runlist_ok.dat", "w");
   FILE* fnotpresent = fopen("runlist_notpresent.dat", "w");
   FILE* fmismatch = fopen("runlist_mismatch.dat", "w");

   while (fr.IsOK()) {

      fr.ReadLine("|");
      if (fr.GetCurrentLine().BeginsWith("#")) {
         KVString line = fr.GetCurrentLine();
         line.ReplaceAll("#", "");
         line = line.StripAllExtraWhiteSpace();
         line.Begin("=");
         KVString name = line.Next();
         if (!line.End() && name == "--dir") {
            datadir = line.Next();
            Info("ReadDBFile", "acquisition file directory: %s", datadir.Data());
            fprintf(fok, "%s\n", datadir.Data());
            fprintf(fnotpresent, "%s\n", datadir.Data());
            fprintf(fmismatch, "%s\n", datadir.Data());
         }
      } else {
         if (datadir == "") {
            fr.CloseFile();
            Warning("ReadDBFile", "no data directory has been found ...");
            return;
         }
         if (fr.GetNparRead() > 0) {
            for (Int_t ii = 0; ii < fr.GetNparRead(); ii += 1) {

               KVString couple = fr.GetReadPar(ii);
               couple.Begin("=");
               KVString name = couple.Next();
               name = name.Strip(TString::kBoth);
               KVString value = "";
               if (!couple.End()) {
                  value = couple.Next();
                  value = value.Strip(TString::kBoth);
               }
               if (name == "run") {
                  run->SetNumber(value.Atoi());
               } else if (name == "nfiles") {
                  run->SetNumberOfAcqFiles(value.Atoi());
               }
            }
            if ((ref = GetRun(run->GetNumber()))) {
               if (run->GetNumberOfAcqFiles() != ref->GetNumberOfAcqFiles()) {
                  lmismatch.Add(run->GetNumber());
                  fprintf(fmismatch, "%d\n", run->GetNumber());
               } else {
                  lok.Add(run->GetNumber());
                  fprintf(fok, "%d\n", run->GetNumber());
               }
            } else {
               lnotpresent.Add(run->GetNumber());
               fprintf(fnotpresent, "%d\n", run->GetNumber());
            }
         }
      }
   }
   fr.CloseFile();

   Info("ReadDBFile", "To resume ...");
   printf("---\nruns OK:\nnumber: %d\nlist: ", lok.GetNValues());
   std::cout << lok.AsString() << std::endl;
   printf("---\nruns not present:\nnumber: %d\nlist: ", lnotpresent.GetNValues());
   std::cout << lnotpresent.AsString() << std::endl;
   printf("---\nruns with mismatch:\nnumber: %d\nlist: ", lmismatch.GetNValues());
   std::cout << lmismatch.AsString() << std::endl;

   fclose(fok);
   fclose(fnotpresent);
   fclose(fmismatch);
}

//__________________________________________________________________________________________________________________
void KVFAZIADB::ReadExceptions()
{
   if (!strcmp(GetCalibFileName("Exceptions"), "")) {
      Info("ReadExceptions()", "No file found for Exceptions");
      return;
   }

   TString fp;
   gDataSet->SearchKVFile(GetCalibFileName("Exceptions"), fp, gDataSet->GetName());

   KVFileReader fr;
   if (!fr.OpenFileToRead(fp.Data())) {
      Error("ReadExceptions()", "Error in opening file %s\n", fp.Data());
      return;
   }

   Info("ReadExceptions()", "Reading exceptions ...");
   TList* ll = new TList();
   KVNumberList lruns;
   KVDBParameterList* dbp = 0;

   ll->SetOwner(kFALSE);
   while (fr.IsOK()) {
      fr.ReadLine(":");
      if (fr.GetNparRead() == 2) {
         if (fr.GetReadPar(0) == "RunRange") {
            if (ll->GetEntries() > 0) {
               printf("\t linkage avec les runs\n");
               LinkListToRunRange(ll, lruns);
               ll->Clear();
            }
            lruns.SetList(fr.GetReadPar(1));
            printf("nouvelle plage : %s\n", lruns.AsString());
         } else {
            KVString name(fr.GetReadPar(0));
            name.Begin(".");
            KVString tel = name.Next();
            tel.Begin("-");
            KVString sdet = tel.Next();
            sdet.Prepend("-");
            sdet.Prepend(tel.Next());
            sdet.Prepend("-");
            sdet.Prepend(tel.Next());
            sdet.Prepend("-");

            KVString sig = name.Next();
            if (sig == "QH1" || sig == "QL1" || sig == "I1")   sdet.Prepend("SI1");
            else if (sig == "Q2" || sig == "I2")        sdet.Prepend("SI2");
            else if (sig == "Q3")                   sdet.Prepend("CSI");

            printf("tel=%s -> sdet=%s\n", tel.Data(), sdet.Data());

            KVString par = name.Next();
            if (!(dbp = (KVDBParameterList*)ll->FindObject(Form("%s.%s", tel.Data(), sig.Data())))) {
               dbp = new KVDBParameterList(Form("%s.%s", sdet.Data(), sig.Data()), sdet.Data());
               dbp->AddKey("Runs", "List of Runs");
               fExceptions->AddRecord(dbp);
               ll->Add(dbp);
               dbp->GetParameters()->SetValue("RunRange", lruns.AsString());
               dbp->GetParameters()->SetValue("Detector", sdet.Data());
               dbp->GetParameters()->SetValue("Signal", sig.Data());
            }
            dbp->GetParameters()->SetValue(par.Data(), fr.GetDoubleReadPar(1));

            printf("\t%s %s %s %lf\n", tel.Data(), sig.Data(), par.Data(), fr.GetDoubleReadPar(1));
         }
      }
   }

   if (ll->GetEntries() > 0) {
      printf("\t linkage avec les runs\n");
      LinkListToRunRange(ll, lruns);
      ll->Clear();
   }

   delete ll;

}
//__________________________________________________________________________________________________________________
void KVFAZIADB::ReadComments()
{

   if (!strcmp(GetCalibFileName("Comments"), "")) {
      Info("ReadComments()", "No file foud for Comments");
      return;
   }
   TString fp;
   gDataSet->SearchKVFile(GetCalibFileName("Comments"), fp, gDataSet->GetName());


   KVFileReader fr;
   if (!fr.OpenFileToRead(fp.Data())) {
      Error("ReadComments()", "Error in opening file %s\n", fp.Data());
      return;
   }

   Info("ReadComments()", "Reading comments ...");
   KVFAZIADBRun* dbrun = 0;
   while (fr.IsOK()) {
      fr.ReadLine("|");
      if (fr.GetCurrentLine().BeginsWith("#")) {

      } else if (fr.GetCurrentLine() == "") {

      } else {
         if (fr.GetNparRead() == 2) {
            KVString srun(fr.GetReadPar(0));
            srun.Begin("=");
            srun.Next();
            KVNumberList lruns(srun.Next());
            KVString comments(fr.GetReadPar(1));
            lruns.Begin();
            while (!lruns.End()) {
               Int_t run = lruns.Next();
               dbrun = (KVFAZIADBRun*)GetRun(run);
               if (dbrun)
                  dbrun->SetComments(comments.Data());
            }
         }
      }
   }

}
//__________________________________________________________________________________________________________________
void KVFAZIADB::ReadCalibrationFiles()
{

   if (!strcmp(GetCalibFileName("CalibrationFiles"), "")) {
      Info("ReadCalibrationFiles()", "No file foud for CalibrationFiles");
      return;
   }
   TString fp;
   gDataSet->SearchKVFile(GetCalibFileName("CalibrationFiles"), fp, gDataSet->GetName());


   KVFileReader fr;
   if (!fr.OpenFileToRead(fp.Data())) {
      Error("ReadCalibrationFiles()", "Error in opening file %s\n", fp.Data());
      return;
   }

   Info("ReadCalibrationFiles()", "Reading calibration files");
   while (fr.IsOK()) {
      fr.ReadLine(0);
      if (fr.GetCurrentLine().BeginsWith("#") || fr.GetCurrentLine() == "") {}
      else {
         ReadCalibFile(fr.GetCurrentLine().Data());
      }
   }
   fr.CloseFile();

}
//__________________________________________________________________________________________________________________
void KVFAZIADB::ReadCalibFile(const Char_t* filename)
{

   if (gSystem->Exec(Form("test -e %s/%s", GetDataSetDir(), filename)) != 0) {
      Info("ReadCalibFile", "%s/%s do not exist", GetDataSetDir(), filename);
      return;
   }

   TEnv env;
   env.ReadFile(Form("%s/%s", GetDataSetDir(), filename), kEnvAll);
   TIter next(env.GetTable());
   TEnvRec* rec = 0;
   KVDBParameterSet* par = 0;
   KVNumberList default_run_list;
   default_run_list.SetMinMax(GetFirstRun(), GetLastRun());

   while ((rec = (TEnvRec*)next())) {
      TString sdet(rec->GetName());
      TString sval(rec->GetValue());
      par = new KVDBParameterSet(sdet.Data(), "Calibration", 1);
      par->SetParameter(sval.Atof());
      fCalibrations->AddRecord(par);
      LinkRecordToRunRange(par, default_run_list);
   }

}
//__________________________________________________________________________________________________________________

void KVFAZIADB::PrintRuns(KVNumberList& nl) const
{
   // Print compact listing of runs in the number list like this:
   //
   // root [9] gFaziaDB->PrintRuns("8100-8120")
   // RUN     SYSTEM                          TRIGGER         EVENTS          COMMENTS
   // ------------------------------------------------------------------------------------------------------------------
   // 8100    129Xe + 58Ni 8 MeV/A            M>=2            968673
   // 8101    129Xe + 58Ni 8 MeV/A            M>=2            969166
   // 8102    129Xe + 58Ni 8 MeV/A            M>=2            960772
   // 8103    129Xe + 58Ni 8 MeV/A            M>=2            970029
   // 8104    129Xe + 58Ni 8 MeV/A            M>=2            502992          disjonction ht chassis 1
   // 8105    129Xe + 58Ni 8 MeV/A            M>=2            957015          intensite augmentee a 200 pA

   printf("RUN\tSYSTEM\t\t\t\tTRIGGER\t\tEVENTS\t\tCOMMENTS\n");
   printf("------------------------------------------------------------------------------------------------------------------\n");
   nl.Begin();
   while (!nl.End()) {
      KVFAZIADBRun* run = GetRun(nl.Next());
      if (!run) continue;
      printf("%4d\t%-30s\t%s\t\t%d\t\t%s\n",
             run->GetNumber(), (run->GetSystem() ? run->GetSystem()->GetName() : "            "), run->GetTriggerString(),
             run->GetEvents(), run->GetComments());
   }
}

void KVFAZIADB::BuildQuickAndDirtyDataBase(TString acqfiles_dir)
{


   if (gSystem->Exec(Form("test -d %s", acqfiles_dir.Data())) != 0) {
      printf("Error in KVFAZIADB::BuildQuickAndDirtyDataBase: %s is not an existing directory\n", acqfiles_dir.Data());
      return;
   }

   FILE* fout = fopen("runlist.dat", "w");
   fprintf(fout, "# Runlist generated by KVFAZIADB::BuildQuickAndDirtyDataBase from files located at:\n");
   fprintf(fout, "# --dir=%s\n", acqfiles_dir.Data());

   Int_t run, nfiles;
   Long64_t size = 0;
   Long64_t totalsize = 0;
   Int_t date, idx;
   KVDatime kvdate;

   KVString ldir = gSystem->GetFromPipe(Form("du %s", acqfiles_dir.Data()));
   ldir.Begin("\n");
   Int_t numberofruns = 0;
   KVNumberList lruns;

   while (!ldir.End()) {
      KVString sdir = ldir.Next();
      sdir.Begin("\t");
      size = sdir.Next().Atoll(); // /TMath::Power(2,10)+1; //read values are in KB
      sdir = gSystem->BaseName(sdir.Next());
      if (sscanf(sdir.Data(), "run%d", &run) == 1) {
         totalsize += size;
         Int_t dmin = 0;
         Int_t dmax = 0;
         KVString lfile = gSystem->GetFromPipe(Form("ls %s/%s", acqfiles_dir.Data(), sdir.Data()));
         lfile.Begin("\n");
         nfiles = 0;
         while (!lfile.End()) {
            KVString sfile = lfile.Next();
            //FzEventSet-1434434675-41784.pb
            if (sscanf(sfile.Data(), "FzEventSet-%d-%d.pb", &date, &idx) == 2) {
               if (dmin == 0) {
                  dmin = dmax = date;
               } else {
                  if (date < dmin) dmin = date;
                  if (date > dmax) dmax = date;
               }
               nfiles += 1;
            }
         }
         if (nfiles == 0 || size == 0) {
            printf("Warning in KVFAZIADB::BuildQuickAndDirtyDataBase: %d -> empty run\n", run);
         }
         kvdate.Set(dmin);
         fprintf(fout, "run=%d | starting date=%s | nfiles=%d | size(GB)=%1.2lf\n", run, kvdate.AsString(), nfiles, size * TMath::Power(2., -20));
         numberofruns += 1;
         lruns.Add(run);
      } else {
         Double_t conv = TMath::Power(2., -30);
         printf("Info in KVFAZIADB::BuildQuickAndDirtyDataBase: total size %lld - %lld\n", size, totalsize);
         printf("Info in KVFAZIADB::BuildQuickAndDirtyDataBase: total size (TB) %1.2lf - %1.2lf\n", size * conv, totalsize * conv);
         printf("Info in KVFAZIADB::BuildQuickAndDirtyDataBase: number of runs %d\n", numberofruns);
         printf("Info in KVFAZIADB::BuildQuickAndDirtyDataBase: last run %d done at %s\n", lruns.Last(), kvdate.AsString());
      }
   }

   fclose(fout);
}
