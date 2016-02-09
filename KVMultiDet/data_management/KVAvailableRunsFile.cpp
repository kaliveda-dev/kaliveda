/*
$Id: KVAvailableRunsFile.cpp,v 1.18 2009/03/12 14:01:02 franklan Exp $
$Revision: 1.18 $
$Date: 2009/03/12 14:01:02 $
*/

//Created by KVClassFactory on Fri May  5 10:46:40 2006
//Author: franklan

#include "KVAvailableRunsFile.h"
#include "KVDataBase.h"
#include "KVDBRun.h"
#include "TObjArray.h"
#include "TObjString.h"
#include "KVString.h"
#include "KVList.h"
#include "KVDataRepository.h"

//macro converting octal filemode to decimal value
//to convert e.g. 664 (=u+rw, g+rw, o+r) use CHMODE(6,6,4)
#define CHMODE(u,g,o) ((u << 6) + (g << 3) + o)

using namespace std;

ClassImp(KVAvailableRunsFile)
////////////////////////////////////////////////////////////////////////////////
//Handles text files containing list of available runs for different datasets and types of data
//
//For each type of data associated with each dataset (type="raw", "recon", "ident" or "root")
//we maintain runlist files which contain the run numbers of datafiles which are physically
//present/available, along with the date/time of last modification of each file, and the filename.
//
//These files are kept in the dataset's KVFiles subdirectory, i.e. in $KVROOT/KVFiles/[name of dataset]
//
//The name of each file has the following format:
//
//      [repository].available_runs.[dataset subdir].[type of data]
//
////////////////////////////////////////////////////////////////////////////////
KVAvailableRunsFile::KVAvailableRunsFile(const Char_t* type,
      KVDataSet*
      parent): KVBase(type)
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

   static TString filename;
   if (!fDataSet) {
      Error("GetFileName", "Dataset has not been set for this file.");
      filename = "";
   } else
      filename.Form("%s.available_runs.%s.%s", fDataSet->GetRepository()->GetName(), fDataSet->GetDataPathSubdir(),
                    fDataSet->GetDataTypeSubdir(GetDataType()));
   return filename.Data();
}

const Char_t* KVAvailableRunsFile::GetFilePath() const
{
   // Returns the full path to the directory where the available runs file is stored.
   // This is by default
   //    $KVROOT/KVFiles/[dataset]
   //
   // However by setting the config variable
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
   } else {
      TString p = gEnv->GetValue("KVAvailableRunsFile.RunFileDirectory", "");
      if (p == "") filepath = fDataSet->GetDataSetDir();
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
      gSystem->Chmod(GetFilePath(), CHMODE(6, 6, 4));
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
   //This method tests the string given as 'filename' to see if it could be the name of a runfile
   //of the datatype of this available runs file (GetDataType()), for its parent dataset, fDataSet.
   //Any protocol and/or path information in the filename is first removed, i.e. if
   //e.g. "run8805.ident.root.2006-12-06_19:41:56" is a valid run filename, then the following
   //are also valid and will return the run number 8805:
   //
   //    "/hpss/in2p3.fr/group/indra/campagne5/ident/run8805.ident.root.2006-12-06_19:41:56"
   //    "root://ccxroot.in2p3.fr:1999//hpss/in2p3.fr/group/indra/campagne5/ident/run8805.ident.root.2006-12-06_19:41:56"
   //    "rfio:cchpssindra:/hpss/in2p3.fr/group/indra/campagne5/ident/run8805.ident.root.2006-12-06_19:41:56"
   //
   //The decision as to whether the filename is valid or not is based on whether it corresponds
   //to one of the formats defined in $KVROOT/KVFiles/.kvrootrc, e.g. :
   //
   //# Default formats for runfile names (run number is used to replace integer format)
   //DataSet.RunFileName.raw:    run%d.raw
   //DataSet.RunFileName.recon:    run%d.recon.root
   //DataSet.RunFileName.ident:    run%d.ident.root
   //DataSet.RunFileName.root:    run%d.root
   //
   //We also test, for each format, "format%*", in case the filename has the file creation date
   //string appended to it.
   //
   //If a working format is found, we return the run number deduced from the filename.
   //If not, we return 0.

   //get format string for current dataset
   TString fmt =
      fDataSet->
      GetDataSetEnv(Form("DataSet.RunFileName.%s", GetDataType()));
   if (fmt == "") {
      Error("IsRunFileName", "No default format set for datatype: %s",
            GetDataType());
      return 0;
   }
   Int_t run;
   //Test format of filename
   KVString _file(gSystem->BaseName(filename));  //Remove protocol and/or path
   if (_file.Sscanf(fmt.Data(), &run))
      return run;
   //wtih date at end ?
   fmt += "%*";
   if (_file.Sscanf(fmt.Data(), &run))
      return run;
   return 0;
}

//__________________________________________________________________________________________________________________

Bool_t KVAvailableRunsFile::ExtractDateFromFileName(const Char_t* name, KVDatime& date)
{
   // We assume that 'name' is the name of a run of the type of this available runs file.
   // We attempt several methods to try to extract a date from 'name'.
   // If successful, we return kTRUE and 'date' contains the result.

   //get format string for current dataset
   TString fmt =
      fDataSet->
      GetDataSetEnv(Form("DataSet.RunFileName.%s", GetDataType()));
   Int_t rubbish = fmt.Index("%*");
   if (rubbish > -1) fmt.Remove(rubbish, fmt.Length());
   Int_t run = IsRunFileName(name);
   if (!run) return kFALSE;
   TString basename;
   basename.Form(fmt.Data(), run);
   TString fullname(name);
   if (!fullname.Contains(basename)) return kFALSE;
   if (fullname.Length() == basename.Length()) return kFALSE;
   fullname.Remove(0, basename.Length() + 1);
   fullname.ReplaceAll("_", " ");
   fullname.ToUpper();
   if (KVDatime::IsSQLFormat(fullname.Data())) {
      date.SetSQLDate(fullname.Data());
      return kTRUE;
   } else {
      fullname.ReplaceAll(".", " ");
      if (KVDatime::IsGANACQFormat(fullname.Data())) {
         date.SetGanacqDate(fullname.Data());
         return kTRUE;
      } else if (KVDatime::IsGANACQ2010Format(fullname.Data())) {
         date.SetGanacq2010Date(fullname.Data());
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
   KVDBTable* run_table = 0;
   KVDataBase* db = fDataSet->GetDataBase();
   if (!db) {
      db = new KVDataBase();
      db->AddTable("Runs", "List of Runs");
   }
   run_table = db->GetTable("Runs");
   while ((objs = (KVBase*) next())) {      // loop over all entries in directory

      Int_t run_num;
      //is this the correct name of a run in the repository ?
      Info("Update", "%s %d", objs->GetName(), IsRunFileName(objs->GetName()));
      if ((run_num = IsRunFileName(objs->GetName()))) {

         KVDBRun* run = (KVDBRun*) run_table->GetRecord(run_num);
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
                  } else {
                     // New Entry - write in temporary runlist file '[run number]|[date of modification]|[name of file]
                     tmp_file << run->GetNumber() << '|' << modt.AsSQLString() << '|' << objs->GetName() << endl;
                  }
               } else { // no previous existing file
                  // New Entry in a new file - write in temporary runlist file '[run number]|[date of modification]|[name of file]
                  tmp_file << run->GetNumber() << '|' << modt.AsSQLString() << '|' << objs->GetName() << endl;
               }
            }
         } else {
            Info("Update", "the current run [%s] is not in database", objs->GetName());
            FileStat_t fs;
            if (repository->GetFileInfo(fDataSet, GetDataType(), objs->GetName(), fs)) {
               TDatime modt(fs.fMtime);
               // New Entry in a new file - write in temporary runlist file '[run number]|[date of modification]|[name of file]
               tmp_file << run_num  << '|' << modt.AsSQLString() << '|' << objs->GetName() << endl;
            } else {
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
   } else if (filenames.GetEntries() > 1) { //several versions
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
         TObjArray* toks = line.Tokenize('|');  // split into fields
         // check date is not identical to a previous entry
         // i.e. there are spurious duplicate entries
         TObjString* rundate = (TObjString*)toks->At(1)->Clone();
         if (dates->FindObject(rundate->GetName())) {
            delete toks;
            delete rundate;
            line.ReadLine(fRunlist);
            continue;
         }
         //add date string
         dates->Add(toks->At(1)->Clone());

         //backwards compatibility
         //an old available_runs file will not have the filename field
         //in this case we assume that the name of the file is given by the
         //dataset's base file name (i.e. with no date/time suffix)
         if (toks->GetEntries() > 2) {
            files->Add(toks->At(2)->Clone());
         } else {
            files->
            Add(new
                TObjString(fDataSet->
                           GetBaseFileName(GetDataType(), run)));
         }
         delete toks;

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
   //If systol!=0 then create and fill a list of available runs for the given system
   //USER MUST DELETE THE LIST AFTER USE.
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
   TString kvversion, username;
   KVDBTable* runs_table = 0;
   if (!fDataSet->GetDataBase()) {
      runs_table = new KVDBTable("Runs");
   } else {
      runs_table = fDataSet->GetDataBase()->GetTable("Runs");
   }

   while (fRunlist.good()) {

      TObjArray* toks = fLine.Tokenize('|');    // split into fields
      if (toks->GetSize()) {

         KVString kvs(((TObjString*) toks->At(0))->GetString());

         if (kvs.IsDigit()) {

            good_lines++;

            fRunNumber = kvs.Atoi();
            kvversion = username = "";
            TString tmp = ((TObjString*) toks->At(1))->GetString();
            fDatime = TDatime(tmp.Data());
            if (toks->GetEntries() > 3) {
               kvversion = ((TObjString*) toks->At(3))->GetString();
               username = ((TObjString*) toks->At(4))->GetString();
            }

            KVDBRun* a_run = (KVDBRun*) runs_table->GetRecord(fRunNumber);

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
                  } else {
                     //another run for this system
                     sys->SetNumberRuns(sys->GetNumberRuns() + 1);
                  }
               }
            } else {
               //making a runlist
               if (systol == sys) {   //run belongs to same system
                  if (!sys_list)
                     sys_list = new TList;
                  a_run->SetDatime(fDatime);
                  a_run->SetKVVersion(kvversion);
                  a_run->SetUserName(username);

                  sys_list->Add(a_run);
               }
            }
         }
      }
      delete toks;
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

         TObjArray* toks = line.Tokenize('|');       // split into fields
         TString ReadFileName;
         //backwards compatibility
         //an old available_runs file will not have the filename field
         //in this case we assume that the name of the file is given by the
         //dataset's base file name (i.e. with no date/time suffix)
         if (toks->GetEntries() > 2) {
            ReadFileName = ((TObjString*) toks->At(2))->String();
         } else {
            ReadFileName =
               fDataSet->GetBaseFileName(GetDataType(), run);
         }

         if (ReadFileName != FileName) {
            //copy line
            tmp_file << line.Data() << endl;
         } else {
            // replace existing infos
            tmp_file << run << "|" << ((TObjString*) toks->At(1))->String() << "|" << filename << "|" << kvversion << "|" << username << endl;
         }
         delete toks;

      } else {
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
      } else {
         //filename was specified: we copy everything up to the line
         //with the right filename & number
         if (line.BeginsWith(Form("%d|", run))) {

            TObjArray* toks = line.Tokenize('|');       // split into fields
            TString ReadFileName;
            //backwards compatibility
            //an old available_runs file will not have the filename field
            //in this case we assume that the name of the file is given by the
            //dataset's base file name (i.e. with no date/time suffix)
            if (toks->GetEntries() > 2) {
               ReadFileName = ((TObjString*) toks->At(2))->String();
            } else {
               ReadFileName =
                  fDataSet->GetBaseFileName(GetDataType(), run);
            }
            delete toks;

            if (ReadFileName != FileName) {
               //copy line
               tmp_file << line.Data() << endl;
            }

         } else {
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

//    //does runlist exist ?
//    if (!OpenAvailableRunsFile()) {
//       Error("CheckMultiRunfiles", "Cannot open available runs file");
//       return 0;
//    }
//
//    TString fLine;
//    TList *run_list = new TList;
//    fLine.ReadLine(fRunlist);
//
//    KVNumberList multiruns;
//
//    Int_t fRunNumber;
//    KVDBTable *runs_table = fDataSet->GetDataBase()->GetTable("Runs");
//
//    while (fRunlist.good()) {
//
//       TObjArray *toks = fLine.Tokenize('|');    // split into fields
//       KVString kvs(((TObjString *) toks->At(0))->GetString());
//       fRunNumber = kvs.Atoi();
//       delete toks;
//
//       KVDBRun *a_run = (KVDBRun *) runs_table->GetRecord(fRunNumber);
//
//       if (!run_list->Contains(a_run)) {
//          //first time that run appears
//          run_list->Add(a_run);
//       } else {
//          //run appears >1 times
//          multiruns.Add(fRunNumber);
//       }
//
//       fLine.ReadLine(fRunlist);
//    }
//    delete run_list;
//    CloseAvailableRunsFile();
//
//    return multiruns;
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
   KVDBTable* runs_table = 0;
   KVDataBase* db = fDataSet->GetDataBase();
   if (!db) {
      db = new KVDataBase();
      db->AddTable("Runs", "List of Runs");
   }
   runs_table = db->GetTable("Runs");

   //KVDBTable *runs_table = fDataSet->GetDataBase()->GetTable("Runs");

   while (fRunlist.good()) {

      TObjArray* toks = fLine.Tokenize('|');    // split into fields
      KVString kvs(((TObjString*) toks->At(0))->GetString());
      fRunNumber = kvs.Atoi();
      delete toks;

      if (sys) {
         // check run is from right system
         KVDBRun* a_run = (KVDBRun*) runs_table->GetRecord(fRunNumber);
         if (a_run) {
            if (a_run->GetSystem() == sys)
               runs.Add(fRunNumber);
         }
      } else {
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

      TObjArray* toks = fLine.Tokenize('|');    // split into fields

      // number of fields can vary
      // nfields = 2: run number, date
      // nfields = 3: run number, date, filename
      // nfields = 5: run number, date, filename, KaliVeda version, username
      Int_t nfields = toks->GetEntries();
      KVString kvs(((TObjString*) toks->At(0))->GetString());
      fRunNumber = kvs.Atoi();
      if (nfields < 2) {
         Warning("ReadFile", "Less than 2 fields in entry for run %d (line:%d)???", fRunNumber, line_number);
         toks->ls();
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
      } else {
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
            delete toks;
            line_number++;
            fLine.ReadLine(fRunlist);
            continue;
         }
      }
      Occurs++;
      NVL->SetValue("Occurs", Occurs);

      NVL->SetValue(Form("Date[%d]", Occurs - 1), datestring.Data());

      //backwards compatibility
      //an old available_runs file will not have the filename field
      //in this case we assume that the name of the file is given by the
      //dataset's base file name (i.e. with no date/time suffix)
      KVString filename;
      if (nfields > 2) {
         filename = ((TObjString*) toks->At(2))->GetString();
      } else {
         filename = fDataSet->GetBaseFileName(GetDataType(), fRunNumber);
      }
      NVL->SetValue(Form("Filename[%d]", Occurs - 1), filename.Data());
      KVString kvversion, username;
      if (nfields > 4) {
         kvversion = ((TObjString*) toks->At(3))->GetString();
         username = ((TObjString*) toks->At(4))->GetString();
         NVL->SetValue(Form("KVVersion[%d]", Occurs - 1), kvversion.Data());
         NVL->SetValue(Form("Username[%d]", Occurs - 1), username.Data());
      }
      delete toks;

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
      if (NVL->IsValue(Form("Filename[%d]", OccNum) , filename) && NVL->IsValue(Form("Date[%d]", OccNum) , modtime.AsSQLString())) return NVL;
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
      if (NVL->IsValue(Form("Filename[%d]", OccNum) , filename)) {
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
