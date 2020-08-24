//Created by KVClassFactory on Thu Oct 18 10:38:52 2012
//Author: John Frankland

#include "KVDMSDataRepository.h"
#include "KVDMS.h"
#include "KVDataSet.h"

using namespace std;

ClassImp(KVDMSDataRepository)

KVDMSDataRepository::KVDMSDataRepository()
{
   // Default constructor
   fDMS = NULL;
   SetType("dms");
}

KVDMSDataRepository::~KVDMSDataRepository()
{
   // Destructor
   SafeDelete(fDMS);
}

KVUniqueNameList* KVDMSDataRepository::GetDirectoryListing(const KVDataSet* ds,
      const Char_t* datatype, const Char_t* subdir)
{
   //Use the DMS catalogue in order to examine the directory
   //
   //      /root_of_data_repository/[datasetdir]/[datatype]/[subdir]
   //      /root_of_data_repository/[datasetdir]/[datatype]         (if subdir="", default value)
   //      /root_of_data_repository/[datasetdir]                    (if datatype="", default value)
   //
   //and fill a TList with one KVDMSFile_t object for each entry in the directory,
   //User must delete the TList after use (list will delete its members)

   TString path, tmp;
   AssignAndDelete(path,
                   gSystem->ConcatFileName(fAccessroot.Data(),
                                           ds->GetDataPathSubdir()));
   if (strcmp(datatype, "")) {
      AssignAndDelete(tmp, gSystem->ConcatFileName(path.Data(), ds->GetDataTypeSubdir(datatype)));
      path = tmp;
      if (strcmp(subdir, "")) {
         AssignAndDelete(tmp, gSystem->ConcatFileName(path.Data(), subdir));
         path = tmp;
      }
   }

   return fDMS->GetFullListing(path.Data());
}

//___________________________________________________________________________

Bool_t KVDMSDataRepository:: CheckSubdirExists(const Char_t* dir,
      const Char_t* subdir)
{
   //Returns kTRUE if the following path is valid
   //      /root_of_data_repository/dir/[subdir]

   TString dirname, path;
   if (subdir) {
      AssignAndDelete(path, gSystem->ConcatFileName(fAccessroot.Data(), dir));
      dirname = subdir;
   }
   else {
      path = fAccessroot.Data();
      dirname = dir;
      // check that 'dir' is not a path containing several directories
      // in this case all but the last have to be put in 'path'
      if (dirname.Contains("/")) {
         AssignAndDelete(path, gSystem->ConcatFileName(fAccessroot.Data(), gSystem->DirName(dir)));
         dirname = gSystem->BaseName(dir);
      }
   }
   return fDMS->DirectoryContains(dirname.Data(), path.Data());
}

//___________________________________________________________________________

void KVDMSDataRepository::CopyFileFromRepository(const KVDataSet* ds,
      const Char_t* datatype,
      const Char_t* filename,
      const Char_t* destination)
{
   //Copy file [datasetdir]/[datatypedir]/[filename] from the repository to [destination]
   //We check if the file to copy exists.

   if (CheckFileStatus(ds, datatype, filename)) {
      TString path, tmp;
      AssignAndDelete(path,
                      gSystem->ConcatFileName(fAccessroot.Data(), ds->GetDataPathSubdir()));
      AssignAndDelete(tmp, gSystem->ConcatFileName(path.Data(), ds->GetDataTypeSubdir(datatype)));
      AssignAndDelete(path, gSystem->ConcatFileName(tmp.Data(), filename));
      //copy file
      fDMS->get(path.Data(), destination);
   }
}

//___________________________________________________________________________

int KVDMSDataRepository::CopyFileToRepository(const Char_t* source,
      const KVDataSet* ds,
      const Char_t* datatype,
      const Char_t* filename)
{
   // Copy file [source] to [datasetdir]/[datatypedir]/[filename] in the repository
   // Returns status of file transfer command

   TString path, tmp;
   AssignAndDelete(path,
                   gSystem->ConcatFileName(fAccessroot.Data(), ds->GetDataPathSubdir()));
   AssignAndDelete(tmp, gSystem->ConcatFileName(path.Data(), ds->GetDataTypeSubdir(datatype)));
   AssignAndDelete(path, gSystem->ConcatFileName(tmp.Data(), filename));

   //copy file
   return fDMS->put(source, path.Data());
}

//___________________________________________________________________________

Bool_t KVDMSDataRepository::CheckFileStatus(const KVDataSet* ds,
      const Char_t* datatype,
      const Char_t* runfile)
{
   //Checks if the run file of given type is physically present in dataset subdirectory,
   //i.e. (schematically), if
   //
   //      /root_of_data_repository/[datasetdir]/[datatypedir]/[runfile]
   //
   //exists. If it does, the returned value is kTRUE (=1).

   TString path, tmp;
   AssignAndDelete(tmp,
                   gSystem->ConcatFileName(fAccessroot.Data(), ds->GetDataPathSubdir()));
   AssignAndDelete(path, gSystem->ConcatFileName(tmp.Data(), ds->GetDataTypeSubdir(datatype)));
   return fDMS->DirectoryContains(runfile, path.Data());
}

//___________________________________________________________________________

void KVDMSDataRepository::MakeSubdirectory(const KVDataSet* ds,
      const Char_t* datatype)
{
   // Overrides KVDataRepository method.
   TString path, tmp;
   AssignAndDelete(tmp,
                   gSystem->ConcatFileName(fAccessroot.Data(), ds->GetDataPathSubdir()));
   AssignAndDelete(path, gSystem->ConcatFileName(tmp.Data(), ds->GetDataTypeSubdir(datatype)));
   fDMS->mkdir(path.Data());
}

//___________________________________________________________________________

void KVDMSDataRepository::DeleteFile(const KVDataSet* ds,
                                     const Char_t* datatype,
                                     const Char_t* filename, Bool_t confirm)
{
   //Delete repository file [datasetdir]/[datatypedir]/[filename]
   //
   //By default (confirm=kTRUE) we ask for confirmation before deleting.
   //Set confirm=kFALSE to delete without confirmation (DANGER!!!)

   TString path, tmp;
   AssignAndDelete(path,
                   gSystem->ConcatFileName(fAccessroot.Data(), ds->GetDataPathSubdir()));
   AssignAndDelete(tmp, gSystem->ConcatFileName(path.Data(), ds->GetDataTypeSubdir(datatype)));
   AssignAndDelete(path, gSystem->ConcatFileName(tmp.Data(), filename));
   TString cmd;
   cout << "Deleting file from repository: " << filename << endl;
   if (confirm) {
      cout <<
           "Are you sure you want to delete this file permanently ? (y/n)"
           << endl;
      TString answer;
      cin >> answer;
      answer.ToUpper();
      if (!answer.BeginsWith("Y")) {
         cout << "File not deleted" << endl;
         return;
      }
   }
   fDMS->forcedelete(path.Data());
}

//___________________________________________________________________________

int KVDMSDataRepository::Chmod(const char* file, UInt_t mode)
{
   // Overrides KVDataRepository method.
   return fDMS->chmod(file, mode);
}
//___________________________________________________________________________

Bool_t KVDMSDataRepository::GetFileInfo(const KVDataSet* ds,
                                        const Char_t* datatype,
                                        const Char_t* runfile,
                                        FileStat_t& fs)
{
   //Checks if the run file of given type is physically present in dataset subdirectory,
   //i.e. (schematically), if
   //
   //      /root_of_data_repository/[datasetdir]/[datatypedir]/[runfile]
   //
   //exists. If it does, the returned value is kTRUE (=1), in which case the FileStat_t object
   //contains information about the file:
   // WARNING:
   //   only  fs.fSize and fs.fMtime are used
   // in addition, the modification time corresponds to the last time that
   // the DMS declaration of the file was changed, not the last physical
   // modification of the file, i.e. it will be the date at which the file
   // was imported into the DMS catalogue, the file may be much older.

   TString path, tmp;
   AssignAndDelete(path,
                   gSystem->ConcatFileName(fAccessroot.Data(), ds->GetDataPathSubdir()));
   AssignAndDelete(tmp, gSystem->ConcatFileName(path.Data(), ds->GetDataTypeSubdir(datatype)));
   AssignAndDelete(path, gSystem->ConcatFileName(tmp.Data(), runfile));

   DMSFile_t DMSfile;
   if (fDMS->GetPathInfo(path.Data(), DMSfile)) {
      fs.fSize = DMSfile.GetSize();
      fs.fMtime = DMSfile.GetModTime().Convert();
      return kTRUE;
   }
   return kFALSE;
}

const Char_t* KVDMSDataRepository::GetFullPathToOpenFile(const KVDataSet* dataset, const Char_t* datatype, const Char_t* runfile)
{
   // Redefinition of KVDataRepository::GetFullPathToOpenFile
   //
   // If ReadProtocol != "root" then we use the DMS to download copies of data repository
   // files to the user's temporary directory for analysis - see OpenDataSetRunFile.
   // In this case, the full path to open the given file is that of the temporary directory copy

   // get read protocol for dataset and datatype
   TString RP = GetReadProtocol(dataset->GetName(), datatype);
   if (RP == "root") return KVDataRepository::GetFullPathToOpenFile(dataset, datatype, runfile);

   // look for file in temp dir
   static TString tmpdir_filepath;
   AssignAndDelete(tmpdir_filepath, gSystem->ConcatFileName(gSystem->TempDirectory(), runfile));
   return tmpdir_filepath.Data();
}

TObject* KVDMSDataRepository::OpenDataSetRunFile(const KVDataSet* ds, const Char_t* type, Int_t run, Option_t* opt)
{
   // Overrides KVDataRepository method
   //
   // If ReadProtocol == "root" we use KVDataRepository method to open file via xrootd server
   //
   // If ReadProtocol != "root" (default if user does not specify [name].DataRepository.ReadProtocol:root):
   //
   //    1) we look for a copy of the file in the user's temporary directory (gSystem->TempDirectory());
   //    2) if no copy exists, we download the file to the user's temporary directory
   //    3) we open the file in the user's temporary directory

   // get read protocol for dataset and datatype
   TString RP = GetReadProtocol(ds->GetName(), type);
   if (RP == "root") return KVDataRepository::OpenDataSetRunFile(ds, type, run, opt);

   // get name of file
   TString filename = ds->GetRunfileName(type, run);
   if (filename == "") {
      Error("OpenDataSetRunFile", "No file found for run %d of data-type %s", run, type);
      return nullptr;
   }

   // look for file in temp dir
   TString tmpdir_filepath;
   AssignAndDelete(tmpdir_filepath, gSystem->ConcatFileName(gSystem->TempDirectory(), filename.Data()));

   if (gSystem->AccessPathName(tmpdir_filepath)) {
      // copy file to temp dir
      CopyFileFromRepository(ds, type, filename, tmpdir_filepath);
   }

   return OpenDataSetFile(ds, type, tmpdir_filepath, opt);
}

