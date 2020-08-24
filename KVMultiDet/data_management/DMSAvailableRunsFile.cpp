//Created by KVClassFactory on Mon Jul 20 11:33:43 2009
//Author: John Frankland

#include "DMSAvailableRunsFile.h"
#include "KVDMS.h"
#include "KVDataRepository.h"
#include "KVDataSet.h"
#include "KVDBRun.h"
#include "KVUniqueNameList.h"
#include "TClass.h"

//macro converting octal filemode to decimal value
//to convert e.g. 664 (=u+rw, g+rw, o+r) use CHMODE(6,6,4)
#define CHMODE(u,g,o) ((u << 6) + (g << 3) + o)

using namespace std;

ClassImp(DMSAvailableRunsFile)


DMSAvailableRunsFile::DMSAvailableRunsFile()
{
   // Default constructor
}

DMSAvailableRunsFile::DMSAvailableRunsFile(const Char_t* type, KVDataSet* ds)
   : KVAvailableRunsFile(type, ds)
{
   // Constructor with data-type
}

DMSAvailableRunsFile::~DMSAvailableRunsFile()
{
   // Destructor
}


void DMSAvailableRunsFile::Update(Bool_t no_existing_file)
{
   //Examine the contents of the DMS container corresponding to this datatype
   //for parent dataset fDataSet.
   //For each file found which was not already in the list of available runs and
   //which corresponds to a run in the database gDataBase,
   //we add an entry to the available runlist file:
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

   KVDataRepository* repository = GetDataSet()->GetRepository();
   //Info("Update","repository : %s",repository->IsA()->GetName());
   cout << endl << "Updating runlist : " << flush;
   //get directory listing from repository
   KVUniqueNameList* dir_list =
      repository->GetDirectoryListing(GetDataSet(), GetDataType());
   if (!dir_list) {
      //Info("Update","Sort au milieu");
      return;
   }
   TIter next(dir_list);
   DMSFile_t* objs;
   //progress bar
   Int_t ntot = dir_list->GetSize();
   Int_t n5pc = TMath::Max(ntot / 20, 1);
   Int_t ndone = 0;
   KVNumberList lnewruns;  //list containing run with a valid file but not store  in database
   while ((objs = (DMSFile_t*) next())) {      // loop over all entries in directory

      //Info("Update","objs->GetName()=%s",objs->GetName());
      Int_t run_num;
      //is this the correct name of a run in the repository ?
      if ((run_num = IsRunFileName(objs->GetName()))) {

         KVDatime filedate;
         KVDBRun* run = GetDataSet()->GetDataBase()->GetDBRun(run_num);
         if (run) {
            //runfile exists in repository
            //check in case it is possible to extract a date from the name of the file
            //the file may be much older than its DMS modtime (=date of DMS import)
            if (!ExtractDateFromFileName(objs->GetName(), filedate))
               filedate = objs->GetModTime();

            if (!no_existing_file) {
               // was there already an entry for exactly the same file in the previous file ?
               Int_t occIdx = 0;
               KVNameValueList* prevEntry = RunHasFileWithDateAndName(run->GetNumber(), objs->GetName(), filedate, occIdx);
               if (prevEntry) {
                  // copy infos of previous entry
                  tmp_file << run->GetNumber() << '|' << filedate.AsSQLString() << '|' << objs->GetName();
                  if (prevEntry->HasParameter(Form("KVVersion[%d]", occIdx))) {
                     tmp_file << "|" << prevEntry->GetStringValue(Form("KVVersion[%d]", occIdx)) << "|" << prevEntry->GetStringValue(Form("Username[%d]", occIdx));
                  }
                  tmp_file << endl;
               }
               else {
                  // New Entry - write in temporary runlist file '[run number]|[date of modification]|[name of file]
                  tmp_file << run->GetNumber() << '|' << filedate.AsSQLString() << '|' << objs->GetName() << endl;
               }
            }
            else {   // no previous existing file
               // New Entry in a new file - write in temporary runlist file '[run number]|[date of modification]|[name of file]
               tmp_file << run->GetNumber() << '|' << filedate.AsSQLString() << '|' << objs->GetName() << endl;
            }
         }
         else {
            lnewruns.Add(run_num);
            filedate = objs->GetModTime();
            tmp_file << run_num << '|' << filedate.AsSQLString() << '|' << objs->GetName() << endl;
         }
      }

      ndone++;
      if (!(ndone % n5pc))
         cout << '>' << flush;
   }

   cout << " DONE" << endl;

   // If runs not present in the database were found, emit an informational message
   if (lnewruns.GetNValues())
      Info("Update", "Found %d runs not in database :\n\tlist:%s", lnewruns.GetNValues(), lnewruns.AsString());

   delete dir_list;
   //close temp file
   tmp_file.close();

   if (CheckDirectoryForAvailableRunsFile()) {
      if (no_existing_file) {
         //use "lockfile" to make sure nobody else tries to modify available_runs file
         //while we are working on it
         if (!runlist_lock.Lock(runlist.Data())) return;
      }

      //copy temporary file to KVFiles directory, overwrite previous
      gSystem->CopyFile(tmp_file_path, runlist, kTRUE);
      //set access permissions to 664
      gSystem->Chmod(runlist.Data(), CHMODE(6, 6, 4));
   }
   //remove lockfile
   runlist_lock.Release();

   //delete temp file
   gSystem->Unlink(tmp_file_path);
}
