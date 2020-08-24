/*
$Id: KVAvailableRunsFile.cpp,v 1.18 2009/03/12 14:01:02 franklan Exp $
$Revision: 1.18 $
$Date: 2009/03/12 14:01:02 $
*/

//Created by KVClassFactory on Fri May  5 10:46:40 2006
//Author: franklan

#include "KVAvailableRunsFile.h"
#include "KVDBRun.h"
#include "TObjArray.h"
#include "TObjString.h"
#include "KVString.h"
#include "KVList.h"
#include "KVDataRepository.h"
#include "KVRunFile.h"

//macro converting octal filemode to decimal value
//to convert e.g. 664 (=u+rw, g+rw, o+r) use CHMODE(6,6,4)
#define CHMODE(u,g,o) ((u << 6) + (g << 3) + o)

using namespace std;

KVString KVAvailableRunsFile::date_read_from_filename = "";

ClassImp(KVAvailableRunsFile)

KVAvailableRunsFile::KVAvailableRunsFile(const Char_t* type,
      const KVDataSet* parent): KVBase(type)
{
   //Constructor with name of datatype and pointer to dataset to which this file belongs
   fDataSet = parent;
   //runlist lockfile
   runlist_lock.SetTimeout(60);   // 60-second timeout in case of problems
   runlist_lock.SetSuspend(5);   // suspension after timeout
   runlist_lock.SetSleeptime(1);   // try lock every second

   fAvailableRuns = 0;
}

KVAvailableRunsFile::KVAvailableRunsFile(const Char_t* type): KVBase(type)
{
   //Constructor with name of datatype
   //Dataset must be set straight away with SetDataSet(KVDataSet*)
   fDataSet = 0;
   //runlist lockfile
   runlist_lock.SetTimeout(60);   // 60-second timeout in case of problems
   runlist_lock.SetSuspend(5);   // suspension after timeout
   runlist_lock.SetSleeptime(1);   // try lock every second

   fAvailableRuns = 0;
}

KVAvailableRunsFile::KVAvailableRunsFile()
{
   //Default ctor
   fDataSet = 0;
   //runlist lockfile
   runlist_lock.SetTimeout(60);   // 60-second timeout in case of problems
   runlist_lock.SetSuspend(5);   // suspension after timeout
   runlist_lock.SetSleeptime(1);   // try lock every second

   fAvailableRuns = 0;
}

KVAvailableRunsFile::~KVAvailableRunsFile()
{
   //Destructor
   //cout << "KVAvailableRunsFile::~KVAvailableRunsFile()" << endl;
   SafeDelete(fAvailableRuns);
}

//__________________________________________________________________________________________________________________

const Char_t* KVAvailableRunsFile::GetFileName() const
{
   // Filename of text file containing information on available runs
   // i.e. [repository].available_runs.[dataset subdir].[dattype subdir]
   //
   // If [dataset subdir] contains "/" (i.e. if data is grouped into subdirectories)
   // we replace them by "_"

   static TString filename;
   if (!fDataSet) {
      Error("GetFileName", "Dataset has not been set for this file.");
      filename = "";
   }
   else {
      KVString datapath = fDataSet->GetDataPathSubdir();
      datapath.ReplaceAll("/", "_");
      filename.Form("%s.available_runs.%s.%s", fDataSet->GetRepository()->GetName(), datapath.Data(),
                    fDataSet->GetDataTypeSubdir(GetDataType()));
   }
   return filename.Data();
}

const Char_t* KVAvailableRunsFile::GetFilePath() const
{
   // Returns the full path to the directory where the available runs file is stored.
   // This is by default the same directory where the dataset files are stored.
   // However, if KaliVeda was installed using a GNU-style directory layout
   // (possibly in the system directories, e.g. Ubuntu packages)
   // then the path will be the user's working directory + dataset name
   //
   // Alternatively, by setting the config variable
   //   KVAvailableRunsFile.RunFileDirectory:   /some/path
   //
   // files will be stored/looked for in
   //   /some/path/[dataset]
   //
   // You can use environment variables - $(VAR) - in the path

   static TString filepath;
   if (!fDataSet) {
      Error("GetFilePath", "Dataset has not been set for this file.");
      filepath = "";
   }
   else {
      TString p = gEnv->GetValue("KVAvailableRunsFile.RunFileDirectory", "");
      if (p == "") {
         // no user-defined run file directory
         if (is_gnuinstall()) {
            // GNU-style install: use working directory
            filepath = GetWORKDIRFilePath(fDataSet->GetName());
         }
         else
            filepath = fDataSet->GetDataSetDir();
      }
      else {
         gSystem->ExpandPathName(p);
         AssignAndDelete(filepath, gSystem->ConcatFileName(p.Data(), fDataSet->GetName()));
      }
   }
   return filepath.Data();
}

const Char_t* KVAvailableRunsFile::GetFullPathToAvailableRunsFile() const
{
   // Return the full path on the local machine to the text file
   // used to store information on available runs

   static TString path;
   AssignAndDelete(path, gSystem->ConcatFileName(GetFilePath(), GetFileName()));
   return path.Data();
}

Bool_t KVAvailableRunsFile::CheckDirectoryForAvailableRunsFile()
{
   // Check that the directory given by GetFilePath() exists and is writable
   // If it does not exist, it will be created if possible
   // If it exists but is not writable, or cannot be created, returns kFALSE.

   if (gSystem->AccessPathName(GetFilePath(), kFileExists)) { // directory does not exist
      if (gSystem->mkdir(GetFilePath(), kTRUE) == -1) {
         Error("CheckDirectoryForAvailableRunsFile", "cannot create directory %s - check access permissions", GetFilePath());
         return kFALSE;
      }
      // set access permissions on created directory
      gSystem->Chmod(GetFilePath(), CHMODE(7, 7, 5));
   }
   if (gSystem->AccessPathName(GetFilePath(), kWritePermission)) { // directory is not writable
      Error("CheckDirectoryForAvailableRunsFile", "directory %s is not writable - check access permissions", GetFilePath());
      return kFALSE;
   }
   return kTRUE;
}

//__________________________________________________________________________________________________________________

Int_t KVAvailableRunsFile::IsRunFileName(const Char_t* filename)
{
   // This method tests the string given as 'filename' to see if it could be the name of a runfile
   // of the datatype of this available runs file (GetDataType()), for its parent dataset, fDataSet.
   // Any protocol and/or path information in the filename is first removed.
   //
   // The format for a given dataset is defined by variable
   //
   // [dataset].DataSet.RunFileName.[datatype]:    [format]
   //
   // If this format contains an index ("%I" field) then you must also define the multiplier
   // used to generate effective run numbers: run = run_number*multiplier+index:
   //
   // [dataset].DataSet.RunFileIndexMultiplier.[datatype]:   [integer]

   TString fmt = fDataSet->GetDataSetEnv(Form("DataSet.RunFileName.%s", GetDataType()), "");  //get format string for current dataset
   if (fmt == "") {
      Error("IsRunFileName", "No default format set for datatype: %s",
            GetDataType());
      return 0;
   }
   int imult = fDataSet->GetDataSetEnv(Form("DataSet.RunFileIndexMultiplier.%s", GetDataType()), -1.0);
   if (fmt.Contains("%I") && (imult < 0)) {
      Error("IsRunFileName", "No index multiplier set for datatype: %s with format: %s",
            GetDataType(), fmt.Data());
      return 0;
   }
   return IsRunFileName(fmt, filename, imult);
}

Int_t KVAvailableRunsFile::IsRunFileName(const KVString& fmt, const Char_t* filename, Int_t index_multiplier, const Char_t* separators)
{
   // This method tests the string given as 'filename' to see if it could be the name of a runfile.
   // Any protocol and/or path information in the filename is first removed.
   // The decision as to whether the filename is valid or not is based on whether it corresponds
   // to the given format string, e.g. run_%04d.dat
   // If good, we return the run number for the file (may be an effective run number in case
   // of files with an index; see below)
   // If not, we return 0.
   //
   // KNOWN CASES:
   //
   // dataset         example filename                   format
   // =======         ================                   =====================
   // INDRA_camp1     run127.raw                         run%R.raw
   // INDRA_camp2     run127.raw                         run%R.raw
   // INDRA_camp4     run127.raw                         run%R.raw
   // INDRA_camp5     run127.raw                         run%R.raw
   // INDRA_e416a     Run356_06-Oct-05_12_44_12.dat      Run%R_%D.dat
   // INDRA_e475s     run_0085.dat.13-Mar-06.12:46:24    run_%R.dat.%D1.%D2
   // INDRA_e494s     run_0026.dat.04-May-07.20:33:00    run_%R.dat.%D1.%D2
   // INDRA_e503      run_0026.dat.04-May-07.20:33:00    run_%R.dat.%D1.%D2
   // INDRA_e613      run_0022.dat.08Sep11_00h54m47s     run_%R.dat.%D
   // INDRAFAZIA.E789 run_0017.dat.04-04-19_08h25m57s.1  run_%R.dat.%D.%I
   // FAZIA*          run000345                          run%R
   //
   //    %R      :   run number
   //    %D      :   date and time
   //    %D1.%D2 :   date and time separated by '.'
   //    %I      :   file index number. WARNING: the first file has NO index, the
   //                second file has index '1', etc. In this case the index_multiplier
   //                will be used to generate (and return) an effective run number,
   //                given by index_multiplier*run + index
   //
   // The filename will be broken up according to the separators (default: ".")
   // Then the different parts will be analysed according to the given format string

   KVString _file(gSystem->BaseName(filename));  //Remove protocol and/or path

   // the filename should contain the same number of "."-separated parts as the format
   // string, unless the format contains "%I", in which case it can have one less part
   // (case of first file for run with no index)
   int np_fmt = fmt.GetNValues(separators);
   bool with_index = fmt.Contains("%I");
   bool two_part_date = false;
   bool got_date = false;
   int np_fn = _file.GetNValues(separators);
   date_read_from_filename = "";

   if (np_fmt == np_fn || (with_index && np_fn == np_fmt - 1)) {
      _file.Begin(separators);
      fmt.Begin(separators);
      int index(0), run(0);
      char date1[100], date2[100];
      if (fmt.Contains("%D1") && fmt.Contains("%D2")) two_part_date = true;

      while (!_file.End()) {
         KVString run_part = _file.Next();
         KVString fmt_part = fmt.Next();

         if (fmt_part.Contains("%R")) {
            fmt_part.ReplaceAll("%R", "%d");
            if (fmt_part.Contains("%D")) {
               fmt_part.ReplaceAll("%D", "%s");
               sscanf(run_part.Data(), fmt_part.Data(), &run, date1);
               got_date = true;
            }
            else
               sscanf(run_part.Data(), fmt_part.Data(), &run);
         }
         else if (fmt_part.Contains("%I")) {
            fmt_part.ReplaceAll("%I", "%d");
            sscanf(run_part.Data(), fmt_part.Data(), &index);
         }
         else if (fmt_part.Contains("%D1")) {
            fmt_part.ReplaceAll("%D1", "%s");
            sscanf(run_part.Data(), fmt_part.Data(), date1);
            got_date = true;
         }
         else if (fmt_part.Contains("%D2")) {
            fmt_part.ReplaceAll("%D2", "%s");
            sscanf(run_part.Data(), fmt_part.Data(), date2);
            got_date = true;
         }
         else if (fmt_part.Contains("%D")) {
            fmt_part.ReplaceAll("%D", "%s");
            sscanf(run_part.Data(), fmt_part.Data(), date1);
            got_date = true;
         }
      }
      if (with_index) run = index_multiplier * run + index;
      if (got_date) {
         if (two_part_date) date_read_from_filename.Form("%s.%s", date1, date2);
         else date_read_from_filename = date1;
      }
      return run;
   }
//   else {
//      ::Warning("KVAvailableRunsFile::IsRunFileName",
//                "%s is not a runfile name according to format %s [separators: %s]",
//                _file.Data(), fmt.Data(), separators);
//   }
   return 0;
}

//__________________________________________________________________________________________________________________

Bool_t KVAvailableRunsFile::ExtractDateFromFileName(const Char_t* name, KVDatime& date)
{
   // We assume that 'name' is the name of a run of the type of this available runs file
   // (see KVAvailableRunsFile::IsRunFileName for accepted formats for runfile names with dates).
   // We attempt several methods to try to extract a date from 'name'.
   // If successful, we return kTRUE and 'date' contains the result.

   //get format string for current dataset
   TString fmt =
      fDataSet->
      GetDataSetEnv(Form("DataSet.RunFileName.%s", GetDataType()));
   return ExtractDateFromFileName(fmt, name, date);
}

Bool_t KVAvailableRunsFile::ExtractDateFromFileName(const TString& fmt, const Char_t* name, KVDatime& date)
{
   // We assume that 'name' is the name of a runfile according to given format 'fmt'.
   // (see KVAvailableRunsFile::IsRunFileName for accepted formats for runfile names with dates).
   // We attempt several methods to try to extract a date from 'name'.
   // If successful, we return kTRUE and 'date' contains the result.

   Int_t run = IsRunFileName(fmt, name);
   if (!run) return kFALSE;
   // if runfile name contains a date, it is now in variable date_read_from_filename
   if (date_read_from_filename == "") return kFALSE;
   date_read_from_filename.ReplaceAll("_", " ");
   date_read_from_filename.ToUpper();
   if (KVDatime::IsSQLFormat(date_read_from_filename.Data())) {
      date.SetSQLDate(date_read_from_filename.Data());
      return kTRUE;
   }
   else {
      date_read_from_filename.ReplaceAll(".", " ");
      if (KVDatime::IsGANACQFormat(date_read_from_filename.Data())) {
         date.SetGanacqDate(date_read_from_filename.Data());
         return kTRUE;
      }
      else if (KVDatime::IsGANACQ2010Format(date_read_from_filename.Data())) {
         date.SetGanacq2010Date(date_read_from_filename.Data());
         return kTRUE;
      }
      else if (KVDatime::IsGANACQNarvalFormat(date_read_from_filename.Data())) {
         date.SetGanacqNarvalDate(date_read_from_filename.Data());
         return kTRUE;
      }
   }
   return kFALSE;
}

//__________________________________________________________________________________________________________________

void KVAvailableRunsFile::Update(Bool_t no_existing_file)
{
   // Examine the contents of the repository directory corresponding to this datatype
   // for parent dataset fDataSet.
   // For each file found which was not already in the list of available runs and
   // which corresponds to a run in the database gDataBase,
   // we add an entry to the available runlist file:
   //      [run number]|[date of modification]|[name of file]
   // For "old" runs we keep the existing informations (including KV version & username)
   //
   // When no_existing_file=kTRUE we are making an available runs file
   // for the first time. There is no pre-existing file.


   TString runlist = GetFullPathToAvailableRunsFile();

   if (!no_existing_file) {
      // read all existing informations
      ReadFile();
      //use "lockfile" to make sure nobody else tries to modify available_runs file
      //while we are working on it
      if (!runlist_lock.Lock(runlist.Data())) return;
   }
   //open temporary file
   TString tmp_file_path(GetFileName());
   ofstream tmp_file;
   KVBase::OpenTempFile(tmp_file_path, tmp_file);

   KVDataRepository* repository = fDataSet->GetRepository();

   cout << endl << "Updating runlist : " << flush;
   //get directory listing from repository
   KVUniqueNameList* dir_list =
      repository->GetDirectoryListing(fDataSet, GetDataType());
   if (!dir_list)
      return;

   TIter next(dir_list);
   KVBase* objs;
   //progress bar
   Int_t ntot = dir_list->GetSize();
   Int_t n5pc = TMath::Max(ntot / 20, 1);
   Int_t ndone = 0;
   unique_ptr<KVExpDB> db_garbage;
   KVExpDB* db = fDataSet->GetDataBase();
   if (!db) {
      db = new KVExpDB();
      db_garbage.reset(db);//clean up
   }
   while ((objs = (KVBase*) next())) {      // loop over all entries in directory

      Int_t run_num;
      //is this the correct name of a run in the repository ?
      if ((run_num = IsRunFileName(objs->GetName()))) {

         KVDBRun* run = db->GetDBRun(run_num);
         if (run) {
            FileStat_t fs;
            //get file modification date
            if (repository->
                  GetFileInfo(fDataSet, GetDataType(),
                              objs->GetName(), fs)) {
               //runfile exists in repository
               TDatime modt(fs.fMtime);
               if (!no_existing_file) {
                  // was there already an entry for exactly the same file in the previous file ?
                  Int_t occIdx = 0;
                  KVNameValueList* prevEntry = RunHasFileWithDateAndName(run->GetNumber(), objs->GetName(), modt, occIdx);
                  if (prevEntry) {
                     // copy infos of previous entry
                     tmp_file << run->GetNumber() << '|' << modt.AsSQLString() << '|' << objs->GetName();
                     if (prevEntry->HasParameter(Form("KVVersion[%d]", occIdx))) {
                        tmp_file << "|" << prevEntry->GetStringValue(Form("KVVersion[%d]", occIdx)) << "|" << prevEntry->GetStringValue(Form("Username[%d]", occIdx));
                     }
                     tmp_file << endl;
                  }
                  else {
                     // New Entry - write in temporary runlist file '[run number]|[date of modification]|[name of file]
                     tmp_file << run->GetNumber() << '|' << modt.AsSQLString() << '|' << objs->GetName() << endl;
                  }
               }
               else {   // no previous existing file
                  // New Entry in a new file - write in temporary runlist file '[run number]|[date of modification]|[name of file]
                  tmp_file << run->GetNumber() << '|' << modt.AsSQLString() << '|' << objs->GetName() << endl;
               }
            }
         }
         else {
            Info("Update", "the current run [%s] is not in database", objs->GetName());
            FileStat_t fs;
            if (repository->GetFileInfo(fDataSet, GetDataType(), objs->GetName(), fs)) {
               TDatime modt(fs.fMtime);
               // New Entry in a new file - write in temporary runlist file '[run number]|[date of modification]|[name of file]
               tmp_file << run_num  << '|' << modt.AsSQLString() << '|' << objs->GetName() << endl;
            }
            else {
               Warning("Update", "%s GetFileInfo return kFALSE", objs->GetName());
            }
         }
      }

      ndone++;
      if (!(ndone % n5pc))
         cout << '>' << flush;
   }

   cout << " DONE" << endl;
   delete dir_list;
   //close temp file
   tmp_file.close();

   if (CheckDirectoryForAvailableRunsFile()) {

      if (no_existing_file) {
         //use "lockfile" to make sure nobody else tries to modify available_runs file
         //while we are working on it
         if (!runlist_lock.Lock(runlist.Data())) return;
      }

      //copy temporary file to available runs file directory, overwrite previous
      gSystem->CopyFile(tmp_file_path, runlist, kTRUE);
      //set access permissions to 664
      gSystem->Chmod(runlist.Data(), CHMODE(6, 6, 4));
   }

   //remove lockfile
   runlist_lock.Release();

   //delete temp file
   gSystem->Unlink(tmp_file_path);
}

//__________________________________________________________________________________________________________________

Bool_t KVAvailableRunsFile::GetRunInfo(Int_t run, TDatime& modtime,
                                       TString& filename)
{
   //Look for a given run number in the file, and read the file's modification date/time and filename
   //If run not found, returns kFALSE
   //If available runs file does not exist, Update() is called to create it.
   //If there are multiple versions of the same run (abnormal situation),
   //we print a warning and give info on the most recent file.

   KVList filenames, dates;
   //get infos for all versions of run
   GetRunInfos(run, &dates, &filenames);
   if (filenames.GetEntries() == 1) { //only one version
      filename = ((TObjString*) filenames.First())->String();
      modtime = ((TObjString*) dates.First())->String().Data();
      return kTRUE;
   }
   else if (filenames.GetEntries() > 1) {   //several versions
      Warning("GetRunInfo",
              "Multiple versions of this runfile exist in the repository. Infos for most recent file will be returned.");
      Warning("GetRunInfo",
              "You should clean the repository using KVDataSet::CleanMultiRunfiles.");
      //find most recent version
      TDatime most_recent("1998-12-25 00:00:00");
      Int_t i_most_recent = 0;
      for (int i = 0; i < dates.GetEntries(); i++) {
         //check if run is most recent
         TDatime rundate(((TObjString*) dates.At(i))->String().Data());
         if (rundate > most_recent) {
            most_recent = rundate;
            i_most_recent = i;
         }
      }
      filename = ((TObjString*) filenames.At(i_most_recent))->String();
      modtime = ((TObjString*) dates.At(i_most_recent))->String().Data();
      return kTRUE;
   }
   return kFALSE;
}

//__________________________________________________________________________________________________________________

void KVAvailableRunsFile::GetRunInfos(Int_t run, KVList* dates,
                                      KVList* files)
{
   //Look for a given run number in the file, and read the file's modification date/time and filename
   //These informations are stored in the two TList as TObjString objects (these objects belong to the
   //lists and will be deleted by them).
   //We do not stop at the first run found, but continue until the end of the file, adding
   //informations for every occurence of the run in the file.
   //If available runs file does not exist, Update() is called to create it.

   //does runlist exist ?
   if (!OpenAvailableRunsFile()) {
      Error("GetRunInfos", "Error opening available runs file");
      return;
   }
   //clear lists - delete objects
   dates->Delete();
   files->Delete();

   //loop over lines in fRunlist file
   //look for line beginning with 'run|'
   TString line;
   line.ReadLine(fRunlist);
   while (fRunlist.good()) {
      if (line.BeginsWith(Form("%d|", run))) {

         //found it
         unique_ptr<TObjArray> toks(line.Tokenize('|'));  // split into fields
         // check date is not identical to a previous entry
         // i.e. there are spurious duplicate entries
         TObjString* rundate = (TObjString*)toks->At(1)->Clone();
         if (dates->FindObject(rundate->GetName())) {
            delete rundate;
            line.ReadLine(fRunlist);
            continue;
         }
         //add date string
         dates->Add(toks->At(1)->Clone());
         //add filename
         files->Add(toks->At(2)->Clone());
      }
      line.ReadLine(fRunlist);
   }
   CloseAvailableRunsFile();
}

//__________________________________________________________________________________________________________________

Bool_t KVAvailableRunsFile::CheckAvailable(Int_t run)
{
   //Look for a given run number in the file
   //If run not found, returns kFALSE
   //If available runs file does not exist, Update() is called to create it.

   //does runlist exist ?
   if (!OpenAvailableRunsFile()) {
      Error("CheckAvailable", "Error opening available runs file");
      return kFALSE;
   }
   //loop over lines in runlist file
   //look for line beginning with 'run|'
   Bool_t found = kFALSE;
   TString line;
   line.ReadLine(fRunlist);
   while (fRunlist.good()) {
      if (line.BeginsWith(Form("%d|", run))) {
         CloseAvailableRunsFile();
         return kTRUE;
      }
      line.ReadLine(fRunlist);
   }
   CloseAvailableRunsFile();
   return found;
}

//__________________________________________________________________________________________________________________

Int_t KVAvailableRunsFile::Count(Int_t run)
{
   //Count the number of times a given run number appears in the file
   //If available runs file does not exist, Update() is called to create it.

//    //does runlist exist ?
//    if (!OpenAvailableRunsFile()) {
//       Error("Count", "Error opening available runs file");
//       return 0;
//    }
//    //loop over lines in runlist file
//    //look for line beginning with 'run|'
//    TString line;
//    Int_t occurs = 0;
//    line.ReadLine(fRunlist);
//    while (fRunlist.good()) {
//       if (line.BeginsWith(Form("%d|", run))) {
//          occurs++;
//       }
//       line.ReadLine(fRunlist);
//    }
//    CloseAvailableRunsFile();
//    return occurs;
   ReadFile();
   KVNameValueList* nvlrun = (KVNameValueList*)fAvailableRuns->FindObject(Form("%d", run));
   if (nvlrun) return nvlrun->GetIntValue("Occurs");
   return 0;
}

//__________________________________________________________________________________________________________________

const Char_t* KVAvailableRunsFile::GetFileName(Int_t run)
{
   //Read from available runs list the name of the file used for this run
   //If run is not available, filename will be empty

   static TString fname;
   static TDatime dtime;
   if (GetRunInfo(run, dtime, fname)) {
      return fname.Data();
   }

   fname = "";
   return fname.Data();
}

//__________________________________________________________________________________________________________________

TList* KVAvailableRunsFile::GetListOfAvailableSystems(const KVDBSystem*
      systol)
{
   //Create and fill a sorted list of available systems based on the runs in the available runs file.
   //If systol!=0 then create and fill a list of available runs (KVRunFile objects) for the given system.
   //USER MUST DELETE THE LIST AFTER USE.
   //  N.B. in case of list of KVRunFile, the list is the owner of the objects and will
   //       destroy them when it is destroyed
   //
   //For each system in the list we set the number of available runs : this number
   //can be retrieved with KVDBSystem::GetNumberRuns()
   //
   //If available runs file does not exist, Update() is called to create it.

   ReadFile(); // this will sanitize the file if necessary (remove duplicates)

   //does runlist exist ?
   if (!OpenAvailableRunsFile()) {
      Error("GetListOfAvailableSystems",
            "Error opening available runs file");
      return 0;
   }

   TString fLine;
   TList* sys_list = 0;
   Int_t good_lines = 0;
   fLine.ReadLine(fRunlist);

   Int_t fRunNumber;
   TDatime fDatime;
   TString kvversion, username, filename;
   KVExpDB* db = fDataSet->GetDataBase();
   unique_ptr<KVExpDB> garbage_db;
   if (!db) {
      db = new KVExpDB;
      garbage_db.reset(db);//clean up
   }

   while (fRunlist.good()) {

      unique_ptr<TObjArray> toks(fLine.Tokenize('|'));    // split into fields
      if (toks->GetSize()) {

         KVString kvs(((TObjString*) toks->At(0))->GetString());

         if (kvs.IsDigit()) {

            good_lines++;

            fRunNumber = kvs.Atoi();
            kvversion = username = "";
            TString tmp = ((TObjString*) toks->At(1))->GetString();
            fDatime = TDatime(tmp.Data());
            filename = ((TObjString*) toks->At(2))->String();

            if (toks->GetEntries() > 3) {
               kvversion = ((TObjString*) toks->At(3))->GetString();
               username = ((TObjString*) toks->At(4))->GetString();
            }

            KVDBRun* a_run = db->GetDBRun(fRunNumber);

            KVDBSystem* sys = 0;
            if (a_run) {
               sys = a_run->GetSystem();
            }
            if (!systol) {
               //making a systems list
               if (!sys_list)
                  sys_list = new TList;
               if (sys) {

                  a_run->SetDatime(fDatime);
                  a_run->SetKVVersion(kvversion);
                  a_run->SetUserName(username);

                  if (!sys_list->Contains(sys)) {
                     //new system
                     sys_list->Add(sys);
                     sys->SetNumberRuns(1);   //set run count to 1
                  }
                  else {
                     //another run for this system
                     sys->SetNumberRuns(sys->GetNumberRuns() + 1);
                  }
               }
            }
            else {
               //making a runlist
               if (systol == sys) {   //run belongs to same system
                  if (!sys_list) {
                     sys_list = new TList;
                     sys_list->SetOwner(kTRUE);//will delete objects
                  }
                  KVRunFile* rf = new KVRunFile(a_run, filename, fDatime, kvversion, username);
                  sys_list->Add(rf);
               }
            }
         }
      }
      fLine.ReadLine(fRunlist);
   }

   //sort list of systems in order of increasing run number
   //sort list of runs in order of increasing run number
   if (sys_list && sys_list->GetSize() > 1)
      sys_list->Sort();

   if (!good_lines) {
      Error("GetListOfAvailableSystems",
            "Available runs file is empty or absent");
   }
   CloseAvailableRunsFile();
   return sys_list;

}

//__________________________________________________________________________________________________________________
void KVAvailableRunsFile::UpdateInfos(Int_t run, const Char_t* filename, const Char_t* kvversion, const Char_t* username)
{
   // Call this mehod to update informations on the file "filename" corresponding to run,
   // by adding/replacing the KV version and username read from the file itself (not necessarily
   // corresponding to current KV version and username)

   //does runlist exist ?
   if (!OpenAvailableRunsFile()) {
      Error("UpdateInfos", "Error opening available runs file");
      return;
   }
   //open temporary file
   TString tmp_file_path(GetFileName());
   ofstream tmp_file;
   KVBase::OpenTempFile(tmp_file_path, tmp_file);

   TString FileName(filename);
   //loop over lines in fRunlist file
   //all lines which do not begin with 'run'| are directly copied to temp file
   TString line;
   line.ReadLine(fRunlist);
   while (fRunlist.good()) {
      //filename was specified: we copy everything up to the line
      //with the right filename & number
      if (line.BeginsWith(Form("%d|", run))) {

         unique_ptr<TObjArray> toks(line.Tokenize('|'));       // split into fields
         TString ReadFileName = ((TObjString*) toks->At(2))->String();

         if (ReadFileName != FileName) {
            //copy line
            tmp_file << line.Data() << endl;
         }
         else {
            // replace existing infos
            tmp_file << run << "|" << ((TObjString*) toks->At(1))->String() << "|" << filename << "|" << kvversion << "|" << username << endl;
         }

      }
      else {
         //copy line
         tmp_file << line.Data() << endl;
      }
      line.ReadLine(fRunlist);
   }

   CloseAvailableRunsFile();
   TString fRunlist_path = GetFullPathToAvailableRunsFile();

   //keep lock on runsfile
   if (!runlist_lock.Lock(fRunlist_path.Data())) return;

   //close temp file
   tmp_file.close();

   //copy temporary file to KVFiles directory, overwrite previous
   gSystem->CopyFile(tmp_file_path, fRunlist_path, kTRUE);
   //set access permissions to 664
   gSystem->Chmod(fRunlist_path.Data(), CHMODE(6, 6, 4));
   //delete temp file
   gSystem->Unlink(tmp_file_path);
   //unlock runsfile
   runlist_lock.Release();
}

//__________________________________________________________________________________________________________________
void KVAvailableRunsFile::Remove(Int_t run, const Char_t* filename)
{
   //Remove from the file the entry corresponding to this run
   //By default, the first occurrence of the run number in the file will be removed.
   //If "filename" is given, we look for a line corresponding to both the run number
   //and the filename (important if run appears more than once !!!) ;-)

   //does runlist exist ?
   if (!OpenAvailableRunsFile()) {
      Error("Remove", "Error opening available runs file");
      return;
   }
   //open temporary file
   TString tmp_file_path(GetFileName());
   ofstream tmp_file;
   KVBase::OpenTempFile(tmp_file_path, tmp_file);

   TString FileName(filename);
   Bool_t withFileName = (FileName != "");
   //loop over lines in fRunlist file
   //all lines which do not begin with 'run'| are directly copied to temp file
   TString line;
   line.ReadLine(fRunlist);
   while (fRunlist.good()) {
      if (!withFileName) {
         //filename not specified : we copy everything up to the first line
         //which begins with the run number we want
         if (!line.BeginsWith(Form("%d|", run))) {
            //copy line
            tmp_file << line.Data() << endl;
         }
      }
      else {
         //filename was specified: we copy everything up to the line
         //with the right filename & number
         if (line.BeginsWith(Form("%d|", run))) {

            unique_ptr<TObjArray> toks(line.Tokenize('|'));       // split into fields
            TString ReadFileName;
            ReadFileName = ((TObjString*) toks->At(2))->String();

            if (ReadFileName != FileName) {
               //copy line
               tmp_file << line.Data() << endl;
            }

         }
         else {
            //copy line
            tmp_file << line.Data() << endl;
         }
      }
      line.ReadLine(fRunlist);
   }

   CloseAvailableRunsFile();
   TString fRunlist_path = GetFullPathToAvailableRunsFile();

   //keep lock on runsfile
   if (!runlist_lock.Lock(fRunlist_path.Data())) return;

   //close temp file
   tmp_file.close();

   //copy temporary file to KVFiles directory, overwrite previous
   gSystem->CopyFile(tmp_file_path, fRunlist_path, kTRUE);
   //set access permissions to 664
   gSystem->Chmod(fRunlist_path.Data(), CHMODE(6, 6, 4));
   //delete temp file
   gSystem->Unlink(tmp_file_path);
   //unlock runsfile
   runlist_lock.Release();
}

//__________________________________________________________________________________________________________________

void KVAvailableRunsFile::Add(Int_t run, const Char_t* filename)
{
   //Add to the file an entry corresponding to this run, assumed to be present in the repository
   //with the given filename.
   //write in temporary runlist file '[run number]|[date of modification]|[name of file]|KaliVeda version|username

   //does runlist exist ?
   if (!OpenAvailableRunsFile()) {
      Error("Add", "Error opening available runs file");
      return;
   }
   //open temporary file
   TString tmp_file_path(GetFileName());
   ofstream tmp_file;
   KVBase::OpenTempFile(tmp_file_path, tmp_file);

   //copy all lines in runlist file
   TString line;
   line.ReadLine(fRunlist);
   while (fRunlist.good()) {
      tmp_file << line.Data() << endl;
      line.ReadLine(fRunlist);
   }

   CloseAvailableRunsFile();
   TString runlist_path = GetFullPathToAvailableRunsFile();

   // keep lock on runsfile
   if (!runlist_lock.Lock(runlist_path.Data())) return;

   //add entry for run
   FileStat_t fs;
   //get file modification date
   if (fDataSet->GetRepository()->
         GetFileInfo(fDataSet, GetDataType(), filename,
                     fs)) {
      //runfile exists in repository
      //write in temporary runlist file '[run number]|[date of modification]|[name of file]|KaliVeda version|username
      TDatime modt(fs.fMtime);
      UserGroup_t* userinfo = gSystem->GetUserInfo();
      tmp_file << run << '|' << modt.
               AsSQLString() << '|' << filename << '|' << GetKVVersion() << '|' << userinfo->fUser << endl;
      delete userinfo;
   }
   //close temp file
   tmp_file.close();

   //copy temporary file to KVFiles directory, overwrite previous
   gSystem->CopyFile(tmp_file_path, runlist_path, kTRUE);
   //set access permissions to 664
   gSystem->Chmod(runlist_path.Data(), CHMODE(6, 6, 4));
   //delete temp file
   gSystem->Unlink(tmp_file_path);
   //unlock runsfile
   runlist_lock.Release();
}

//__________________________________________________________________________________________________________________

Bool_t KVAvailableRunsFile::OpenAvailableRunsFile()
{
   //Initialise fRunlist so that it can be used to read the available runs file,
   //opens the file containing the list of available runs for the current dataset.
   //If the file does not exist, call Update() to create it.
   //
   //Returns kFALSE in case of problems.

   TString runlist = GetFullPathToAvailableRunsFile();

   fRunlist.clear();            // clear any error flags (EOF etc.) before trying to open file
   if (!SearchAndOpenKVFile(runlist, fRunlist, "", &runlist_lock)) {
      //no runlist exists. we therefore have to create it.
      Warning("OpenAvailableRunsFile", "runlist file does not exist...");
      Update(kTRUE);
      if (!SearchAndOpenKVFile(runlist, fRunlist, "", &runlist_lock)) {
         Error("OpenAvailableRunsFile",
               "Something weird: I just made the available runlist file, but I still can't open it!");
         return kFALSE;
      }
   }

   return kTRUE;
}

//__________________________________________________________________________________________________________________

void KVAvailableRunsFile::CloseAvailableRunsFile()
{
   //Must be called after each operation which calls OpenAvailableRunsFile
   //Not only do we close the file stream, we remove the file lock put in place
   //by OpenAvailableRunsFile in order to allow others to read/write the file.
   fRunlist.close();
   fRunlist.clear();
   runlist_lock.Release();
}

//__________________________________________________________________________________________________________________

KVNumberList KVAvailableRunsFile::CheckMultiRunfiles()
{
   //Returns a list with all runs which occur more than once in the available runs file.

   ReadFile();
   KVNumberList multiruns;
   TIter next(fAvailableRuns);
   KVNameValueList* run;
   while ((run = (KVNameValueList*)next())) {

      if (run->GetIntValue("Occurs") > 1) multiruns.Add(run->GetName());

   }
   return multiruns;
}

//__________________________________________________________________________________________________________________

KVNumberList KVAvailableRunsFile::GetRunList(const KVDBSystem* sys)
{
   //Returns list of available run numbers for this data type.
   //If 'sys' gives the address of a valid database reaction system, only runs
   //corresponding to the system will be included.

   KVNumberList runs;

   //does runlist exist ?
   if (!OpenAvailableRunsFile()) {
      Error("GetRunList", "Cannot open available runs file");
      return runs;
   }

   TString fLine;
   fLine.ReadLine(fRunlist);

   Int_t fRunNumber;
   KVExpDB* db = fDataSet->GetDataBase();
   unique_ptr<KVExpDB> garbage_db;
   if (!db) {
      db = new KVExpDB;
      garbage_db.reset(db);
   }

   while (fRunlist.good()) {

      TObjArray* toks = fLine.Tokenize('|');    // split into fields
      KVString kvs(((TObjString*) toks->At(0))->GetString());
      fRunNumber = kvs.Atoi();
      delete toks;

      if (sys) {
         // check run is from right system
         KVDBRun* a_run = db->GetDBRun(fRunNumber);
         if (a_run) {
            if (a_run->GetSystem() == sys)
               runs.Add(fRunNumber);
         }
      }
      else {
         // add all runs to list
         runs.Add(fRunNumber);
      }

      fLine.ReadLine(fRunlist);
   }

   CloseAvailableRunsFile();
   return runs;
}

//____________________________________________________________________________________________________________________

void KVAvailableRunsFile::ReadFile()
{
   // Read all infos in available runs file and store as KVNameValueList objects in fAvailableRuns.
   // For each run in the file we add a KVNameValueList with the following fields:
   //
   // Name = run number
   // Occurs = number of times run appears in file
   // Filename[0] = name of first file for run
   // Filename[1] =
   //  ...
   // Filename[Occurs-1] = name of last file for run
   // Date[0] = date & time of generation of first file etc.
   // KVVersion[0] = name of KaliVeda version used to generate first file etc. (if known)
   // Username[0] = name of user who generated first file etc. (if known)

   //does runlist exist ?
   if (!OpenAvailableRunsFile()) {
      Error("ReadFile", "Cannot open available runs file");
      return;
   }

   if (fAvailableRuns) delete fAvailableRuns;
   fAvailableRuns = new KVHashList;
   fAvailableRuns->SetOwner(kTRUE);

   TString fLine;
   Int_t line_number = 1;
   fLine.ReadLine(fRunlist);

   KVNumberList duplicate_lines;

   Int_t fRunNumber;

   while (fRunlist.good()) {

      unique_ptr<TObjArray> toks(fLine.Tokenize('|'));    // split into fields

      // number of fields can vary
      // nfields = 2: run number, date
      // nfields = 3: run number, date, filename
      // nfields = 5: run number, date, filename, KaliVeda version, username
      Int_t nfields = toks->GetEntries();
      KVString kvs(((TObjString*) toks->At(0))->GetString());
      if (kvs.Contains("/")) {
         Warning("ReadFile", "Strange '/' symbol in run number (line:%d)!!!", line_number);
         toks->ls();
         fLine.ReadLine(fRunlist);
         continue;
      }

      fRunNumber = kvs.Atoi();
      if (nfields < 2) {
         Warning("ReadFile", "Less than 2 fields in entry for run %d (line:%d)???", fRunNumber, line_number);
         toks->ls();
         fLine.ReadLine(fRunlist);
         continue;
      }
      //get date string
      KVString datestring(((TObjString*) toks->At(1))->GetString());

      // is run already in list ?
      KVNameValueList* NVL = (KVNameValueList*)fAvailableRuns->FindObject(kvs);
      Int_t Occurs = (NVL ? NVL->GetIntValue("Occurs") : 0);
      if (!NVL) {
         NVL = new KVNameValueList(kvs);
         fAvailableRuns->Add(NVL);
      }
      else {
         // check date for run is different to any others
         Bool_t ok = kTRUE;
         for (Int_t ii = 0; ii < Occurs; ii++) {
            KVString olddate = NVL->GetStringValue(Form("Date[%d]", ii));
            if (olddate == datestring) {
               ok = kFALSE;
               duplicate_lines.Add(line_number);
               break;
            }
         }
         if (!ok) {
            line_number++;
            fLine.ReadLine(fRunlist);
            continue;
         }
      }
      Occurs++;
      NVL->SetValue("Occurs", Occurs);

      NVL->SetValue(Form("Date[%d]", Occurs - 1), datestring.Data());

      KVString filename = ((TObjString*) toks->At(2))->GetString();
      NVL->SetValue(Form("Filename[%d]", Occurs - 1), filename.Data());
      KVString kvversion, username;
      if (nfields > 4) {
         kvversion = ((TObjString*) toks->At(3))->GetString();
         username = ((TObjString*) toks->At(4))->GetString();
         NVL->SetValue(Form("KVVersion[%d]", Occurs - 1), kvversion.Data());
         NVL->SetValue(Form("Username[%d]", Occurs - 1), username.Data());
      }

      line_number++;
      fLine.ReadLine(fRunlist);
   }

   CloseAvailableRunsFile();

   if (duplicate_lines.GetNValues()) {
      Info("ReadFile", "There were %d duplicate entries in available runs file, they will be removed", duplicate_lines.GetNValues());
      RemoveDuplicateLines(duplicate_lines);
   }

}

KVNameValueList* KVAvailableRunsFile::RunHasFileWithDateAndName(Int_t run, const Char_t* filename, TDatime modtime, Int_t& OccNum)
{
   // look in previously read infos (see ReadFile) to see if, for a given run, there is a file with the
   // given name and modification date/time
   // if so, returns the address of the KVNameValueList for the run & sets OccNum to the index number of
   // the corresponding entry (in case of several files for the run)
   // if not, returns NULL

   if (!fAvailableRuns) return NULL;
   // is run already in list ?
   KVNameValueList* NVL = (KVNameValueList*)fAvailableRuns->FindObject(Form("%d", run));
   if (!NVL) return NULL;
   Int_t Occurs = NVL->GetIntValue("Occurs");
   for (OccNum = 0; OccNum < Occurs; OccNum++) {
      if (NVL->IsValue(Form("Filename[%d]", OccNum), filename) && NVL->IsValue(Form("Date[%d]", OccNum), modtime.AsSQLString())) return NVL;
   }
   return NULL;
}

Bool_t KVAvailableRunsFile::InfosNeedUpdate(Int_t run, const Char_t* filename)
{
   // return kTRUE if the given file for this run is lacking some information
   // e.g. the KV version and username
   // N.B.: if no file is known for this run, we return kFALSE

   ReadFile();
   // is run already in list ?
   KVNameValueList* NVL = (KVNameValueList*)fAvailableRuns->FindObject(Form("%d", run));
   if (!NVL) return kFALSE;
   Int_t Occurs = NVL->GetIntValue("Occurs");
   for (Int_t OccNum = 0; OccNum < Occurs; OccNum++) {
      if (NVL->IsValue(Form("Filename[%d]", OccNum), filename)) {
         // infos need update if no KV version has been set
         return (!NVL->HasParameter(Form("KVVersion[%d]", OccNum)));
      }
   }
   return kFALSE;
}

//__________________________________________________________________________________________________________________

void KVAvailableRunsFile::RemoveDuplicateLines(KVNumberList lines_to_be_removed)
{
   // Remove from available runs file all lines whose numbers are in the list

   //does runlist exist ?
   if (!OpenAvailableRunsFile()) {
      Error("Remove", "Error opening available runs file");
      return;
   }
   //open temporary file
   TString tmp_file_path(GetFileName());
   ofstream tmp_file;
   KVBase::OpenTempFile(tmp_file_path, tmp_file);

   //loop over lines in fRunlist file
   //all lines which are not in list are directly copied to temp file
   TString line;
   Int_t line_number = 1;
   line.ReadLine(fRunlist);

   lines_to_be_removed.Begin();
   Int_t next_line_to_remove = 0;
   if (!lines_to_be_removed.End()) next_line_to_remove = lines_to_be_removed.Next();

   while (fRunlist.good()) {

      if (line_number != next_line_to_remove)
         tmp_file << line.Data() << endl;
      else {
         if (!lines_to_be_removed.End()) next_line_to_remove = lines_to_be_removed.Next();
      }
      line_number++;
      line.ReadLine(fRunlist);

   }

   CloseAvailableRunsFile();
   TString fRunlist_path = GetFullPathToAvailableRunsFile();

   //keep lock on runsfile
   if (!runlist_lock.Lock(fRunlist_path.Data())) return;

   //close temp file
   tmp_file.close();

   //copy temporary file to KVFiles directory, overwrite previous
   gSystem->CopyFile(tmp_file_path, fRunlist_path, kTRUE);
   //set access permissions to 664
   gSystem->Chmod(fRunlist_path.Data(), CHMODE(6, 6, 4));
   //delete temp file
   gSystem->Unlink(tmp_file_path);
   //unlock runsfile
   runlist_lock.Release();
}

//__________________________________________________________________________________________________________________
