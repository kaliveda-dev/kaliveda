/*
$Id: KVDataRepository.cpp,v 1.21 2009/01/16 14:55:20 franklan Exp $
$Revision: 1.21 $
$Date: 2009/01/16 14:55:20 $
*/

//Created by KVClassFactory on Wed Apr 26 16:17:37 2006
//Author: franklan

#include "KVDataRepository.h"
#include "KVDataSetManager.h"
#include "KVBase.h"
#include "KVList.h"
#include "TError.h"
#include "KVDataSet.h"
#include "Riostream.h"
#include "TObjString.h"
#include "TFile.h"
#include "TOrdCollection.h"
#include "TRegexp.h"
#include "TPluginManager.h"
#include "TNetFile.h"
#include "TROOT.h"
#include "KVConfig.h"

#include <TClass.h>
#include <TMethodCall.h>

//macro converting octal filemode to decimal value
//to convert e.g. 664 (=u+rw, g+rw, o+r) use CHMODE(6,6,4)
#define CHMODE(u,g,o) ((u << 6) + (g << 3) + o)

using namespace std;

ClassImp(KVDataRepository)

KVDataRepository* gDataRepository;

//___________________________________________________________________________

void KVDataRepository::cd()
{
   //Make this the 'active' or 'default' data repository,
   //i.e. gDataRepository points to this and gDataSetManager points to the
   //associated data set manager.
   gDataRepository = this;
   gDataSetManager = fDSM;
}

//___________________________________________________________________________

Bool_t KVDataRepository::Init()
{
   //Initialises data repository based on information in .kvrootrc file.
   //Each line beginning "name.DataRepository" is used for this data repository,
   //where "name" is the name of this repository.
   //
   //Then we create and initialise the data set manager for this repository.
   //
   //returns kTRUE if all goes well.
   //returns kFALSE if not (i.e. data set manager cannot be initialised, no available datasets)

   fLocalrootdir =
      gEnv->GetValue(Form("%s.DataRepository.RootDir", GetName()), "");
   if (fLocalrootdir == "") {
      Error("Init", "Top-level directory for repository %s is not defined. Set %s.DataRepository.RootDir variable in .kvrootrc",
            GetName(), GetName());
      return kFALSE;
   }
   fAccessprotocol =
      gEnv->GetValue(Form("%s.DataRepository.AccessProtocol", GetName()),
                     "local");
   fReadprotocol =
      gEnv->GetValue(Form("%s.DataRepository.ReadProtocol", GetName()),
                     "local");
   //can we create and write directly new files in the repository ?
   //for local repositories, default is yes, unless 'DataRepository.CanWrite' = 'NO'
   //for remote repositories, default is no, unless 'DataRepository.CanWrite' = 'YES'
   fCanWrite = (Bool_t)gEnv->GetValue(Form("%s.DataRepository.CanWrite", GetName()), (Int_t)(!IsRemote()));
   //xrootd protocol
   fXrootdserver =
      gEnv->GetValue(Form("%s.DataRepository.XRDServer", GetName()), "");
   fXrootdrootdir =
      gEnv->GetValue(Form("%s.DataRepository.XRDRootDir", GetName()), "");
   //access to xrootd via ssh tunnel ?
   fXRDtunnel = (fXRDtunPort = (Int_t)gEnv->GetValue(Form("%s.DataRepository.XRDTunnel.port", GetName()), 0));
   if (fXRDtunnel) {
      PrepareXRDTunnel();
   }
   //rfio protocol
   fRfioserver =
      gEnv->GetValue(Form("%s.DataRepository.RFIOServer", GetName()), "");
   fRfiorootdir =
      gEnv->GetValue(Form("%s.DataRepository.RFIORootDir", GetName()),
                     "");
   //file transfer protocol
   fTransfertype =
      gEnv->
      GetValue(Form("%s.DataRepository.FileTransfer.type", GetName()),
               "sftp");
   if (fTransfertype.Contains("bbftp")) {
      fTransferExec = fTransfertype;
      fTransfertype = "bbftp";
      if (!KVBase::FindExecutable(fTransferExec)) {
         Error("Init", "Executable for bbftp client not found. Check %s.DataRepository.FileTransfer.type",
               GetName());
      }
   }
   else if (fTransfertype.Contains("sftp")) {
      fTransferExec = fTransfertype;
      fTransfertype = "sftp";
      if (!KVBase::FindExecutable(fTransferExec)) {
         fTransferExec = "";
      }
   }
   else if (fTransfertype.Contains("root")) {
      fTransferExec = "root";
      fTransfertype = "xrd";
      if (!fXRDtunnel) fTransferExec = "";  // must have viable SSH tunnel description
   }
   fTransferserver =
      gEnv->
      GetValue(Form("%s.DataRepository.FileTransfer.server", GetName()),
               "");
   fTransferuser = gEnv->GetValue(Form("%s.DataRepository.FileTransfer.user", GetName()), "");  //if not given, we use current username
   if (fTransferuser == "") {
      UserGroup_t* user = gSystem->GetUserInfo();
      fTransferuser = user->fUser;
   }
   //set full paths to be used for checking existence of files and for opening files
   SetFullPath(fAccessroot, fAccessprotocol.Data());
   SetFullPath(fReadroot, fReadprotocol.Data());
   //data set manager
   if (fDSM) {
      delete fDSM;
   }
   fDSM = NewDataSetManager();

   //return status of dataset manager
   return fDSM->Init(this);
}

//___________________________________________________________________________

void KVDataRepository::Print(Option_t*) const
{
   //Print info on repository
   Info("Print", "configuration of repository is as follows - ");
   cout << "\tname = " << GetName() << endl;
   cout << "\tRootDir = " << fLocalrootdir.Data() << endl;
   cout << "\tAccessProtocol = " << fAccessprotocol.Data() << endl;
   cout << "\tAccessRootDir = " << fAccessroot.Data() << endl;
   cout << "\tReadProtocol = " << fReadprotocol.Data() << endl;
   cout << "\tReadRootDir = " << fReadroot.Data() << endl;
   cout << "\tCanWrite = " << fCanWrite << endl;
   cout << "\tXRDServer = " << fXrootdserver.Data() << endl;
   cout << "\tXRDRootDir = " << fXrootdrootdir.Data() << endl;
   if (fXRDtunnel) {
      cout << "\tXRDTunnel.host = " << fXRDtunHost.Data() << endl;
      cout << "\tXRDTunnel.port = " << fXRDtunPort << endl;
      cout << "\tXRDTunnel.user = " << fXRDtunUser.Data() << endl;
      cout << "\tXRDTunnel.retry = " << fXRDtunRetry << endl;
   }
   cout << "\tRFIOServer = " << fRfioserver.Data() << endl;
   cout << "\tRFIORootDir = " << fRfiorootdir.Data() << endl;
   cout << "\tTransferType = " << fTransfertype.Data() << endl;
   cout << "\tTransferServer = " << fTransferserver.Data() << endl;
   cout << "\tTransferUser = " << fTransferuser.Data() << endl;
}

//___________________________________________________________________________

KVDataRepository::KVDataRepository()
{
   //Default constructor
   fDSM = 0;
   fHelpers       = 0;
   fCommitDataSet = 0;
   SetType("local");
}

//___________________________________________________________________________

KVDataRepository::~KVDataRepository()
{
   //Destructor
   if (fDSM)
      delete fDSM;
   fDSM = 0;

   if (fHelpers) {
      fHelpers->Delete();
      SafeDelete(fHelpers);
   }

}

//___________________________________________________________________________

Bool_t KVDataRepository::CheckSubdirExists(const Char_t* dir, const Char_t* subdir)
{
   //Returns kTRUE if the following path is valid
   //      /root_of_data_repository/dir/[subdir]

   TString _dirname, tmp;
   AssignAndDelete(tmp, gSystem->ConcatFileName(fAccessroot.Data(), dir));
   if (subdir)
      AssignAndDelete(_dirname,
                      gSystem->ConcatFileName(tmp.Data(), subdir));
   else
      _dirname = tmp;
   //resolve any environment variables in path
   gSystem->ExpandPathName(_dirname);
   //if AccessPathName() returns 0, the subdir exists
   return (!gSystem->AccessPathName(_dirname.Data()));
}

//___________________________________________________________________________

void KVDataRepository::SetFullPath(TString& path, const Char_t* protocol)
{
   //Will replace contents of 'path' with the full base path needed for the given protocol.
   //
   //protocol = "local" :  path = fLocalrootdir
   //protocol = "root" :   path = "root://" + fXrootdserver + "/" + fXrootdrootdir
   //protocol = "rfio" :   path = "rfio:" + fRfioserver + ":" + fRfiorootdir

   TString tmp(protocol);
   if (tmp == "local")
      path = fLocalrootdir;
   else if (tmp == "root")
      path.Form("root://%s/%s", fXrootdserver.Data(),
                fXrootdrootdir.Data());
   else if (tmp == "rfio")
      path.Form("rfio:%s:%s", fRfioserver.Data(), fRfiorootdir.Data());
   else
      Warning("SetFullPath",
              "Unknown protocol in call to SetFullPath : %s", protocol);
}

//___________________________________________________________________________

Bool_t KVDataRepository::GetFileInfo(const KVDataSet* dataset,
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
   //contains information about the file.

   TString path, tmp;
   AssignAndDelete(path,
                   gSystem->ConcatFileName(fAccessroot.Data(), dataset->GetDataPathSubdir()));
   AssignAndDelete(tmp, gSystem->ConcatFileName(path.Data(), dataset->GetDataTypeSubdir(datatype)));
   AssignAndDelete(path, gSystem->ConcatFileName(tmp.Data(), runfile));
   return !gSystem->GetPathInfo(path.Data(), fs);
}

//___________________________________________________________________________

Bool_t KVDataRepository::CheckFileStatus(const KVDataSet* dataset,
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
   AssignAndDelete(path,
                   gSystem->ConcatFileName(fAccessroot.Data(), dataset->GetDataPathSubdir()));
   AssignAndDelete(tmp, gSystem->ConcatFileName(path.Data(), dataset->GetDataTypeSubdir(datatype)));
   AssignAndDelete(path, gSystem->ConcatFileName(tmp.Data(), runfile));
   return !gSystem->AccessPathName(path.Data());
}

//___________________________________________________________________________

const Char_t* KVDataRepository::GetFullPathToOpenFile(const KVDataSet* dataset,
      const Char_t*
      datatype,
      const Char_t*
      runfile)
{
   //Returns the full path (including protocol specification) needed by TFile::Open or TChain::Add
   //to open a runfile belonging to the given dataset.
   //The protocol depends on the value of fReadprotocol.
   //If a special protocol has been defined for the given dataset and/or datatype, as in this example
   //for "raw" data:
   //
   //ccali.DataRepository.ReadProtocol.raw:     rfio
   //
   //it will be used instead of the default (fReadprotocol).
   //If a given data type is not accessible for a repository, use "none":
   //
   //ccali.DataRepository.ReadProtocol.raw:     none
   //
   //If this is the case, an error message will be printed and an empty string returned.

   static TString path;

   TString read_proto = GetReadProtocol(dataset->GetName(), datatype);
   if (read_proto == "none") {
      Error("GetFullPathToOpenFile", "Datatype \"%s\" can not be read from the repository \"%s\"",
            datatype, GetName());
      path = "";
      return path.Data();
   }
   TString read_root;
   if (read_proto != fReadprotocol) {
      SetFullPath(read_root, read_proto.Data());
   }
   else {
      read_root = fReadroot;
   }
   TString tmp, datasetdir = dataset->GetDataPathSubdir();
   AssignAndDelete(path,
                   gSystem->ConcatFileName(read_root.Data(),
                                           datasetdir.Data()));
   AssignAndDelete(tmp, gSystem->ConcatFileName(path.Data(), dataset->GetDataTypeSubdir(datatype)));
   AssignAndDelete(path, gSystem->ConcatFileName(tmp.Data(), runfile));
   return path.Data();
}

//___________________________________________________________________________

const Char_t* KVDataRepository::GetFullPathToTransferFile(const KVDataSet* dataset,
      const Char_t*
      datatype,
      const Char_t*
      runfile)
{
   //Used by KVDataTransfer.
   //Returns the full path needed to transfer a runfile belonging to the given dataset
   //either from or to the repository, using sftp or bbftp etc.
   //This is just a concatenation of the repository root directory with the dataset
   //subdirectories and filename.

   static TString path;
   TString tmp, datasetdir = dataset->GetDataPathSubdir();
   AssignAndDelete(path,
                   gSystem->ConcatFileName(fLocalrootdir.Data(),
                                           datasetdir.Data()));
   AssignAndDelete(tmp, gSystem->ConcatFileName(path.Data(), dataset->GetDataTypeSubdir(datatype)));
   AssignAndDelete(path, gSystem->ConcatFileName(tmp.Data(), runfile));
   return path.Data();
}

//___________________________________________________________________________

const Char_t* KVDataRepository::GetReadProtocol(const Char_t* dataset,
      const Char_t* datatype)
{
   //Returns string containing protocol for reading files of the given datatype
   //belonging to the dataset whose name is given.
   //Protocol = "local", "root", "rfio" , "none"
   //(if "none", the data can not be read)
   //If a specific protocol for the dataset and/or datatype is not defined in $KVROOT/KVFiles/.kvrootrc,
   //the default protocol (=fReadprotocol) is returned.
   //Format for defining specific protocols:
   //      name_of_repository.DataRepository.ReadProtocol.dataset_name.data_type:    protocol
   //      name_of_repository.DataRepository.ReadProtocol.dataset_name:    protocol
   //      name_of_repository.DataRepository.ReadProtocol.data_type:    protocol

   static TString prot;
   prot =
      gEnv->
      GetValue(Form
               ("%s.DataRepository.ReadProtocol.%s.%s", GetName(),
                dataset, datatype), "");
   if (prot != "")
      return prot.Data();
   prot =
      gEnv->
      GetValue(Form
               ("%s.DataRepository.ReadProtocol.%s", GetName(), dataset),
               "");
   if (prot != "")
      return prot.Data();
   prot =
      gEnv->
      GetValue(Form
               ("%s.DataRepository.ReadProtocol.%s", GetName(), datatype),
               "");
   if (prot != "")
      return prot.Data();
   return fReadprotocol;
}

//___________________________________________________________________________

void KVDataRepository::CopyFileFromRepository(const KVDataSet* dataset,
      const Char_t* datatype,
      const Char_t* filename,
      const Char_t* destination)
{
   //Copy file [datasetdir]/[datatypedir]/[filename] from the repository to [destination]
   //We check if the file to copy exists.

   if (CheckFileStatus(dataset, datatype, filename)) {
      TString path, tmp;
      AssignAndDelete(path,
                      gSystem->ConcatFileName(fAccessroot.Data(), dataset->GetDataPathSubdir()));
      AssignAndDelete(tmp, gSystem->ConcatFileName(path.Data(), dataset->GetDataTypeSubdir(datatype)));
      AssignAndDelete(path, gSystem->ConcatFileName(tmp.Data(), filename));
      //copy file
      CopyFile(path.Data(), destination);
   }
}

//___________________________________________________________________________

int KVDataRepository::CopyFileToRepository(const Char_t* source,
      const KVDataSet* dataset,
      const Char_t* datatype,
      const Char_t* filename)
{
   //Copy file [source] to [datasetdir]/[datatypedir]/[filename] in the repository
   //The file access permissions are set to '664 (u:rw, g:rw, o:r)
   //Returns status of file transfer command

   TString path, tmp;
   AssignAndDelete(path,
                   gSystem->ConcatFileName(fAccessroot.Data(), dataset->GetDataPathSubdir()));
   AssignAndDelete(tmp, gSystem->ConcatFileName(path.Data(), dataset->GetDataTypeSubdir(datatype)));
   AssignAndDelete(path, gSystem->ConcatFileName(tmp.Data(), filename));

   //copy file
   int status = CopyFile(source, path.Data());
   if (status == 0) {
      //change file access permissions to 664
      Chmod(path.Data(), CHMODE(6, 6, 4));
   }
   else {
      Error("CopyFileToRepository", "Problem copying file %s to repository (%d)",
            source, status);
   }
   return status;
}

//___________________________________________________________________________

void KVDataRepository::MakeSubdirectory(const KVDataSet* dataset,
                                        const Char_t* datatype)
{
   //Create a new subdirectory in the repository:
   //      /root_of_data_repository/[datasetdir]
   //or with 'datatype' given:
   //      /root_of_data_repository/[datasetdir]/[datatypedir]
   //Set access permissions to 775 (u:rwx, g:rwx, o:rx)

   TString path, tmp;
   AssignAndDelete(tmp,
                   gSystem->ConcatFileName(fAccessroot.Data(),
                                           dataset->GetDataPathSubdir()));
   if (strcmp(datatype, ""))
      AssignAndDelete(path, gSystem->ConcatFileName(tmp.Data(), dataset->GetDataTypeSubdir(datatype)));
   else
      path = tmp;
   cout << "Creating new repository directory: " << path.Data() << endl;
   gSystem->mkdir(path.Data(), kTRUE); // create all parent directories as needed
   //change file access permissions to 775
   Chmod(path.Data(), CHMODE(7, 7, 5));
}

//___________________________________________________________________________

KVUniqueNameList* KVDataRepository::GetDirectoryListing(const KVDataSet* dataset,
      const Char_t* datatype, const Char_t* subdir)
{
   //Use the access protocol defined by DataRepository.AccessProtocol (=local by default)
   //in order to open the directory
   //
   //      /root_of_data_repository/[datasetdir]/[datatype]/[subdir]
   //      /root_of_data_repository/[datasetdir]/[datatype]         (if subdir="", default value)
   //      /root_of_data_repository/[datasetdir]                    (if datatype="", default value)
   //
   //and fill a TList with one KVBase object for each entry in the directory,
   //excluding "." and ".."
   //User must delete the KVUniqueNameList after use (list will delete its members)

   TString path, tmp;
   AssignAndDelete(path,
                   gSystem->ConcatFileName(fAccessroot.Data(),
                                           dataset->GetDataPathSubdir()));
   if (strcmp(datatype, "")) {
      AssignAndDelete(tmp, gSystem->ConcatFileName(path.Data(), dataset->GetDataTypeSubdir(datatype)));
      path = tmp;
      if (strcmp(subdir, "")) {
         AssignAndDelete(tmp, gSystem->ConcatFileName(path.Data(), subdir));
         path = tmp;
      }
   }
   //open directory
   void* dirp = gSystem->OpenDirectory(path.Data());
   if (!dirp) {
      Error("KVDataRepository::GetDirectoryListing", "Cannot open %s",
            path.Data());
      return 0;
   }

   KVUniqueNameList* dirlist = new KVUniqueNameList(kTRUE);
   dirlist->SetOwner(kTRUE);

   TObjString* direntry = new TObjString(gSystem->GetDirEntry(dirp));
   while (direntry->GetString() != "") {        //loop over all entries in directory
      if (direntry->GetString() == "." || direntry->GetString() == "..") {
         //skip "." and ".."
         delete direntry;
      }
      else {
         dirlist->Add(new KVBase(direntry->GetString().Data()));
         delete direntry;
      }
      //get next entry
      direntry = new TObjString(gSystem->GetDirEntry(dirp));
   }
   //delete last TObjString, which should contain ""
   delete direntry;
   //close directory
   gSystem->FreeDirectory(dirp);
   return dirlist;
}

//___________________________________________________________________________

TFile* KVDataRepository::CreateNewFile(const KVDataSet* dataset,
                                       const Char_t* datatype,
                                       const Char_t* filename)
{
   //This will create and open a new ROOT runfile for the dataset in the repository,
   //ready to be written.
   //
   //If the subdirectory for 'datatype' does not exist, it will be created.
   //
   //In fact, if the repository is not one in which files can be created and written
   //directly (i.e. if CanWrite() = kFALSE), the file will be created in the local working
   //directory, and only copied into the repository when CommitFile is called.

   cout << "CreateNewFile : " << filename << endl;
   if (!CanWrite()) {
      //files cannot be created and written directly in the repository
      //store path info for subsequent CommitFile
      /*
      fCommitDataSet = dataset;
      fCommitDataType = datatype;
      fCommitFileName = filename;
      */
      //create local file
      return new TFile(filename, "recreate");
   }
   //create file in local repository - make sure subdirectory exists!
   if (!CheckSubdirExists(dataset->GetDataPathSubdir(), dataset->GetDataTypeSubdir(datatype))) {
      //create subdirectory
      MakeSubdirectory(dataset, datatype);
   }
   return new TFile(GetFullPathToOpenFile(dataset, datatype, filename),
                    "recreate");
}

//___________________________________________________________________________

void KVDataRepository::CommitFile(TFile* file, const Char_t* datatype, const KVDataSet* dataset)
{
   //Add this file (previously created by a call to CreateNewFile) to the repository.
   //Any objects should be written to the file before calling this method, either by
   //calling the Write() method of each object, or by calling file->Write().
   //No file->Write() is done here: we only close (delete) the TFile.
   //
   //For repositories in which files can be created and written directly (i.e. if CanWrite() = kTRUE),
   //we just have to close the file.
   //For repositories where CanWrite() = kFALSE, we close the file, then copy it to the correct place in the repository,
   //using the previously recorded values of fCommitDataSetDir, fCommitDataType,
   //and fCommitFileName. Then we remove the local copy from disk.
   //
   //NB: after calling this method, the TFile pointer 'file' must not be used!!!

   //close ROOT file
   //if (fCommitFileName == "") {
   fCommitFileName = file->GetName();
   /*
      Info("KVDataRepository::CommitFile",
            "Recover name of the file ==> %s",fCommitFileName.Data());
   }
   */
   delete file;

   if (CanWrite()) {           //all we have to do for repositories in which files can be written directly
      return;
   }

   fCommitDataSet = dataset;
   fCommitDataType = datatype;

   //create file in local repository - make sure subdirectory exists!
   CreateAllNeededSubdirectories(fCommitDataSet, fCommitDataType);

   cout << endl << "Copying file " << fCommitFileName << " to repository"
        << endl;
   TString s;
   AssignAndDelete(s,
                   gSystem->ConcatFileName(gSystem->pwd(),
                                           fCommitFileName.Data()));
   CopyFileToRepository(s.Data(), fCommitDataSet,
                        fCommitDataType.Data(), fCommitFileName.Data());
   //delete local file
   gSystem->Unlink(fCommitFileName.Data());
   /*
   fCommitDataSet = 0;
   fCommitDataType = fCommitFileName = "";
   */
}

//___________________________________________________________________________

void KVDataRepository::DeleteFile(const KVDataSet* dataset,
                                  const Char_t* datatype,
                                  const Char_t* filename, Bool_t confirm)
{
   //Delete repository file [datasetdir]/[datatype]/[filename]
   //
   //By default (confirm=kTRUE) we ask for confirmation before deleting.
   //Set confirm=kFALSE to delete without confirmation (DANGER!!!)

   TString path, tmp;
   AssignAndDelete(path,
                   gSystem->ConcatFileName(fAccessroot.Data(), dataset->GetDataPathSubdir()));
   AssignAndDelete(tmp, gSystem->ConcatFileName(path.Data(), dataset->GetDataTypeSubdir(datatype)));
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
   gSystem->Unlink(path.Data());
}

//___________________________________________________________________________

KVDataSetManager* KVDataRepository::GetDataSetManager() const
{
   //Return pointer to data set manager for this repository
   return fDSM;
}

//___________________________________________________________________________

KVDataSetManager* KVDataRepository::NewDataSetManager()
{
   //Create and return pointer to new data set manager
   return (new KVDataSetManager);
}

//___________________________________________________________________________

TSystem* KVDataRepository::FindHelper(const char* path, void* dirptr)
{
   // Create helper TSystem to handle file and directory operations that
   // might be special for remote file access, like via rfiod or rootd.

   if (!fHelpers)
      fHelpers = new TOrdCollection;

   TPluginHandler* h;
   TSystem* helper = 0;
   TUrl url(path, kTRUE);

   // look for existing helpers
   TIter next(fHelpers);
   while ((helper = (TSystem*) next()))
      if (HelperIsConsistentWith(helper, path, dirptr))
         return helper;

   if (!path)
      return 0;

   // create new helper
   TRegexp re("^root.*:");  // also roots, rootk, etc
   TString pname = path;
   if (pname.Index(re) != kNPOS) {
      // rootd daemon ...
      if ((h = gROOT->GetPluginManager()->FindHandler("TSystem", path)) &&
            h->LoadPlugin() == 0)
         helper = (TSystem*) h->ExecPlugin(2, path, kFALSE);
      else
#ifdef __WITHOUT_TNETSYSTEM_CTOR_BOOL_T
         helper = new TNetSystem(path);
#else
         helper = new TNetSystem(path, kFALSE);
#endif
   }
   else if (!strcmp(url.GetProtocol(), "http") &&
            pname.BeginsWith("http")) {
      // http ...
      if ((h = gROOT->GetPluginManager()->FindHandler("TSystem", path)) &&
            h->LoadPlugin() == 0)
         helper = (TSystem*) h->ExecPlugin(0);
      else {
         ;  // no default helper yet
      }
   }
   else if ((h = gROOT->GetPluginManager()->FindHandler("TSystem", path))) {
      if (h->LoadPlugin() == -1)
         return 0;
      helper = (TSystem*) h->ExecPlugin(0);
   }

   if (helper)
      fHelpers->Add(helper);

   return helper;
}

//___________________________________________________________________________

int KVDataRepository::Chmod(const char* file, UInt_t mode)
{
   //Used to change file access permissions in the repository

   //do we need a special helper for this filesystem ?
   TSystem* helper = FindHelper(file);
   return (helper ? helper->Chmod(file, mode) : gSystem->Chmod(file, mode));
}

//___________________________________________________________________________

int KVDataRepository::CopyFile(const char* f, const char* t, Bool_t overwrite)
{
   //Method to copy any file from anywhere to anywhere
   //(if the necessary TFile and TSystem plugins are available).
   // Copy a file. If overwrite is true and file already exists the
   // file will be overwritten. Returns 0 when successful, -1 in case
   // of failure, -2 in case the file already exists and overwrite was false.

   if (!gSystem->AccessPathName(t) && !overwrite)
      return -2;

   //save current directory
   TDirectory* dir_sav = gDirectory;

   TUrl path(f, kTRUE);
   TUrl path2(t, kTRUE);

   path.SetOptions("filetype=raw");
   TFile* from = TFile::Open(path.GetUrl(), "READ");
   if (!from || from->IsZombie()) {
      dir_sav->cd();
      return -1;
   }
//   from->ls();

   path2.SetOptions("filetype=raw");
   TString open_option("CREATE");
   if (overwrite) open_option.Prepend("RE");
   TFile* to = TFile::Open(path2.GetUrl(), open_option.Data());
   if (!to || to->IsZombie()) {
      dir_sav->cd();
      return -2;
   }
//   to->ls();

   //set buffer size depending on size of file to copy
   //as a default use 1MB buffer (good for large files and rfio-hpss system);
   //use a 1KB buffer for files smaller than 1MB
   int bufsize = 1024 * 1024;
   Long64_t filesize = from->GetSize();
   if (filesize > 0 && filesize < 1024 * 1024) bufsize = 1024;
   char* buf = new char[bufsize];

   int ret = 0;
   Long64_t bytes_read, bytes_wrote, bytes_left, last_bytes_read, last_bytes_wrote;
   bytes_read = bytes_wrote = bytes_left = last_bytes_read = last_bytes_wrote = 0;

//   printf("Filesize=%lld Buffersize=%d\n",filesize,bufsize);

//   int op = 1;

   while (bufsize && !ret && !from->ReadBuffer(buf, bufsize)) {

      bytes_read = from->GetBytesRead() - last_bytes_read;
      last_bytes_read = from->GetBytesRead();
      ret = (int)to->WriteBuffer(buf, bytes_read);
      bytes_wrote = to->GetBytesWritten() - last_bytes_wrote;
      last_bytes_wrote = to->GetBytesWritten();
      if (bytes_wrote != bytes_read)
         ret = -3;
      //adjust buffer size
      bytes_left = filesize - last_bytes_read;
      if (bytes_left < bufsize) bufsize = (int)bytes_left;

//       printf("----------------------------------%d-----------------------------------\n",op++);
//       printf("bytes_read=%lld last_bytes_read=%lld total_bytes_read=%lld\n",
//             bytes_read, last_bytes_read, from->GetBytesRead());
//       printf("bytes_wrote=%lld last_bytes_wrote=%lld total_bytes_wrote=%lld\n",
//             bytes_wrote, last_bytes_wrote, to->GetBytesWritten());
//       printf("bytes_left=%lld bufsize=%d\n",
//             bytes_left, bufsize);
   }

//       printf("----------------------------------END-----------------------------------\n");
//       printf("bytes_read=%lld last_bytes_read=%lld total_bytes_read=%lld\n",
//             bytes_read, last_bytes_read, from->GetBytesRead());
//       printf("bytes_wrote=%lld last_bytes_wrote=%lld total_bytes_wrote=%lld\n",
//             bytes_wrote, last_bytes_wrote, to->GetBytesWritten());
//       printf("bytes_left=%lld bufsize=%d\n",
//             bytes_left, bufsize);

   delete from;
   delete to;
   delete[] buf;

   //reset working directory
   dir_sav->cd();

   return ret;
}

//___________________________________________________________________________

Bool_t KVDataRepository::HelperIsConsistentWith(TSystem* helper, const char* path, void* dirptr)
{
   // Copy of (protected) method TSystem::ConsistentWith
   // Check consistency of this helper with the one required
   // by 'path' or 'dirptr'

   Bool_t checkproto = kFALSE;
   if (path) {
      if (!helper->GetDirPtr()) {
         TUrl url(path, kTRUE);
         if (!strncmp(url.GetProtocol(), helper->GetName(), strlen(helper->GetName())))
            checkproto = kTRUE;
      }
   }

   Bool_t checkdir = kFALSE;
   if (helper->GetDirPtr() && helper->GetDirPtr() == dirptr)
      checkdir = kTRUE;

   return (checkproto || checkdir);
}

//______________________________________________________________________________________________//

void KVDataRepository::PrepareXRDTunnel()
{
   //PRIVATE method, called by constructor.
   //Called if an SSH tunnel is to be used to connect to an xrootd server, e.g. if the repository
   //configuration includes the following lines (or similar):
   //
   // #ccali.DataRepository.XRDTunnel.host:       ccali.in2p3.fr
   // #ccali.DataRepository.XRDTunnel.port:          10000
   // #ccali.DataRepository.XRDTunnel.retry:         5
   // #ccali.DataRepository.XRDTunnel.user:
   //
   //In this case we need to replace the real name & port of the xrootd server:
   //
   //#ccali.DataRepository.XRDServer:      ccxrdsn012:1999
   //
   //with the name & port of the SSH tunnel
   //
   //#ccali.DataRepository.XRDServer:      localhost:10000
   //
   //which will be used for path names in e.g. GetFullPathToOpenFile.
   //The tunnel is not created here; it should be opened when needed by calling IsConnected()

   fXRDtunHost = gEnv->GetValue(Form("%s.DataRepository.XRDTunnel.host", GetName()), "");
   if (fXRDtunHost == "") {
      Error("PrepareXRDTunnel", "Give host through which to tunnel : %s.DataRepository.XRDTunnel.host",
            GetName());
      fXRDtunnel = kFALSE;
      return;
   }
   fXRDtunUser = gEnv->GetValue(Form("%s.DataRepository.XRDTunnel.user", GetName()), "");
   if (fXRDtunUser == "") {//if not given, use local user name
      UserGroup_t* user = gSystem->GetUserInfo();
      fXRDtunUser = user->fUser;
   }
   //spec for tunnel connection
   fXRDtunSpec = Form("%d:%s", fXRDtunPort, fXrootdserver.Data());
   //max. number of retries before giving up when trying to open tunnel
   fXRDtunRetry = (Int_t)gEnv->GetValue(Form("%s.DataRepository.XRDTunnel.retry", GetName()), 30);
   //now change xrootd server/port to use for pathnames
   fXrootdserver.Form("localhost:%d", fXRDtunPort);
}

//______________________________________________________________________________________________//

KVDataRepository* KVDataRepository::NewRepository(const Char_t* type)
{
   // Create new instance of class derived from KVDataRepository.
   // Actual class of object depends on 'type' which is used to select one of the
   // Plugin.KVDataRepository's defined in .kvrootrc.

   //check and load plugin library
   TPluginHandler* ph = KVBase::LoadPlugin("KVDataRepository", type);
   if (!ph)
      return new KVDataRepository();

   //execute constructor/macro for plugin
   return ((KVDataRepository*) ph->ExecPlugin(0));
}

//______________________________________________________________________________________________//

KVAvailableRunsFile* KVDataRepository::NewAvailableRunsFile(const Char_t* data_type, const KVDataSet* ds)
{
   // Create new instance of class derived from KVAvailableRunsFile.
   // Actual class of object depends on the type of the repository,
   // which is used to select one of the
   // Plugin.KVDataAvailableRunsFile's defined in .kvrootrc.

   //check and load plugin library
   TPluginHandler* ph = KVBase::LoadPlugin("KVAvailableRunsFile", GetType());
   if (!ph)
      return new KVAvailableRunsFile(data_type, ds);

   //execute constructor/macro for plugin
   return ((KVAvailableRunsFile*) ph->ExecPlugin(2, data_type, ds));
}

//___________________________________________________________________________

TObject* KVDataRepository::OpenDataSetRunFile(const KVDataSet* ds, const Char_t* type, Int_t run, Option_t* opt)
{
   //Open a file using plugin defined in $KVROOT/KVFiles/.kvrootrc
   //The default base classes for each type are defined as in this example:
   //
   // # Default base classes for reading runfiles of different types
   // DataSet.RunFileClass.raw:    KVRawDataReader
   // DataSet.RunFileClass.recon:    TFile
   // DataSet.RunFileClass.ident:    TFile
   // DataSet.RunFileClass.root:    TFile
   //
   //A different base class can be defined for a specific dataset/datatype
   //by adding a line to your $HOME/.kvrootrc like this:
   //
   //name_of_dataset.DataSet.RunFileClass.data_type:     BaseClassName
   //
   //The actual class to be used is then defined by plugins in $KVROOT/KVFiles/.kvrootrc,
   //for example
   //
   //Plugin.KVRawDataReader:    raw.INDRA*    KVINDRARawDataReader     KVIndra    "KVINDRARawDataReader()"
   //
   //which defines the plugin for raw data for all datasets whose name begins with "INDRA"
   //If no plugin is found for the base class defined by DataSet.RunFileClass, the base class is used.
   //
   //To actually open the file, each base class & plugin must define a method
   //    static BaseClass* Open(const Char_t* path, Option_t* opt="", ...)
   //which takes the full path to the file as argument (any other arguments taking default options)
   //and returns a pointer of the BaseClass type to the created object which can be used to read the file.

   TString fname = ds->GetFullPathToRunfile(type, run);
   if (fname == "") return nullptr; //file not found

   return OpenDataSetFile(ds, type, fname, opt);
}

//___________________________________________________________________________

TObject* KVDataRepository::OpenDataSetFile(const KVDataSet* ds, const Char_t* type, const TString& fname, Option_t* opt)
{
   //Open a file using plugin defined in $KVROOT/KVFiles/.kvrootrc
   //fname is the full path required to open the file
   //The default base classes for each type are defined as in this example:
   //
   // # Default base classes for reading runfiles of different types
   // DataSet.RunFileClass.raw:    KVRawDataReader
   // DataSet.RunFileClass.recon:    TFile
   // DataSet.RunFileClass.ident:    TFile
   // DataSet.RunFileClass.root:    TFile
   //
   //A different base class can be defined for a specific dataset/datatype
   //by adding a line to your $HOME/.kvrootrc like this:
   //
   //name_of_dataset.DataSet.RunFileClass.data_type:     BaseClassName
   //
   //The actual class to be used is then defined by plugins in $KVROOT/KVFiles/.kvrootrc, for example
   //
   //name_of_dataset.DataSet.RunFileClass.data_type:     plugin_name
   //
   //where 'plugin_name' is one of the known plugins for the default base class for the given data_type,
   //i.e. if data_type="raw" we could use one of either "GANIL" or "MFM" as plugin_name:
   //
   //Plugin.KVRawDataReader:    GANIL    KVGANILDataReader     KVMultiDetexp_events    "KVGANILDataReader()"
   //+Plugin.KVRawDataReader:    MFM    KVMFMDataFileReader     KVMultiDetdaq_cec    "KVMFMDataFileReader()"
   //
   //If no plugin is found for the base class defined by DataSet.RunFileClass, the base class is used.
   //
   //To actually open the file, each base class & plugin must define a method
   //    static BaseClass* Open(const Char_t* path, Option_t* opt="", ...)
   //which takes the full path to the file as argument (any other arguments taking default options)
   //and returns a pointer of the BaseClass type to the created object which can be used to read the file.

   // check connection to repository (e.g. SSH tunnel) in case of remote repository
   if (!IsConnected()) return nullptr;

   //get class for dataset & type
   KVString plugin_class = ds->GetDataSetEnv(Form("DataSet.RunFileClass.%s", type));

   TPluginHandler* ph = nullptr;
   KVString base_class;
   if (IsThisAPlugin(plugin_class, base_class)) {

      //look for plugin specific to dataset & type
      ph = LoadPlugin(base_class, plugin_class);

   }
   else
      base_class = plugin_class;
   TClass* cl;
   if (!ph) {
      //no plugin - use base class
      cl = TClass::GetClass(base_class.Data());
   }
   else {
      cl = TClass::GetClass(ph->GetClass());
   }

   //set up call to static Open method
   TMethodCall* methcall;
   if (strcmp(opt, "")) {
      //Open with option
      methcall = new  TMethodCall(cl, "Open", Form("\"%s\", \"%s\"", fname.Data(), opt));
   }
   else {
      //Open without option
      methcall = new  TMethodCall(cl, "Open", Form("\"%s\"", fname.Data()));
   }

   if (!methcall->IsValid()) {
      if (ph) Error("OpenDataSetFile", "Open method for class %s is not valid", ph->GetClass());
      else Error("OpenDataSetFile", "Open method for class %s is not valid", base_class.Data());
      delete methcall;
      return nullptr;
   }

   //open the file
   Long_t retval;
   methcall->Execute(retval);
   delete methcall;
   return ((TObject*)(retval));
}

void KVDataRepository::CreateAllNeededSubdirectories(const KVDataSet* DataSet, const Char_t* DataType)
{
   // Ensure that all required subdirectories exist and any missing ones are created
   // in order to store a runfile of given DataType for the given DataSet

   if (!CheckSubdirExists(DataSet->GetDataPathSubdir(), DataSet->GetDataTypeSubdir(DataType))) {
      //does dataset directory exist ?
      if (!CheckSubdirExists(DataSet->GetDataPathSubdir())) {
         //create dataset directory
         MakeSubdirectory(DataSet);
      }
      //create subdirectory
      MakeSubdirectory(DataSet, DataType);
   }
}

//___________________________________________________________________________

#ifdef __CCIN2P3_RFIO
//______________________________________________________________________________________________//

ClassImp(KVRFIOSystem)

///////////////////////////////////////////////////////////////////////
//This class corrects a bug in TRFIOSystem::Unlink
//and adds a fully-functioning Chmod method and
//CopyFile which can handle either or both of source
//and target file being accessed via rfiod (the other is
//assumed to be on the local filesystem).

extern "C" {
   FILE*   rfio_fopen(const char* path, const char* mode);
   int     rfio_fclose(FILE* s);
   int     rfio_fread(void* buffer, size_t size, size_t count, FILE* stream);
   int     rfio_fwrite(const void* buffer, size_t size, size_t count, FILE* stream);
   int     rfio_feof(FILE* stream);
   int     rfio_stat(const char* path, struct stat* statbuf);
   int     rfio_rmdir(const char* path);
   int     rfio_unlink(const char* path);
   int     rfio_chmod(const char* path, mode_t mode);
};

//______________________________________________________________________________________________//

Int_t KVRFIOSystem::Unlink(const char* path)
{
   // Unlink, i.e. remove, a file or directory. Returns 0 when succesfull,
   // -1 in case of failure.

   TUrl url(path);
   struct stat finfo;
   if (rfio_stat(url.GetFile(), &finfo) < 0)
      return -1;

   if (R_ISDIR(finfo.st_mode))
      return rfio_rmdir(url.GetFile());
   else
      return rfio_unlink(url.GetFile());
}

//______________________________________________________________________________________________//

int KVRFIOSystem::Chmod(const char* file, UInt_t mode)
{
   // Set the file permission bits. Returns -1 in case or error, 0 otherwise.

   TUrl url(file);
   return rfio_chmod(url.GetFile(), mode);
}

#endif
