/*
$Id: KVRemoteDataRepository.cpp,v 1.6 2007/11/21 11:34:41 franklan Exp $
$Revision: 1.6 $
$Date: 2007/11/21 11:34:41 $
*/

//Created by KVClassFactory on Thu Jul 13 23:44:40 2006
//Author: John Frankland

#include "KVRemoteDataRepository.h"
#include "KVRemoteDataSetManager.h"
#include "TSocket.h"

using namespace std;

ClassImp(KVRemoteDataRepository)

KVRemoteDataRepository::KVRemoteDataRepository()
{
   //Default constructor

   SetType("remote");
}

KVRemoteDataRepository::~KVRemoteDataRepository()
{
   //Destructor
}

//___________________________________________________________________________

Bool_t KVRemoteDataRepository::CheckSubdirExists(const Char_t*,
      const Char_t*)
{
   //Returns kTRUE if the following path is valid
   //      /root_of_data_repository/dir/[subdir]
   //For a remote data repository, this is not possible.
   //Always returns kFALSE.
   Warning("CheckSubdirExists",
           "Always returns false for remote data repository");
   return kFALSE;
}

//___________________________________________________________________________

Bool_t KVRemoteDataRepository::GetFileInfo(const KVDataSet*,
      const Char_t*,
      const Char_t*,
      FileStat_t&)
{
   //Checks if the run file of given type is physically present in dataset subdirectory,
   //i.e. (schematically), if
   //
   //      /root_of_data_repository/[datasetdir]/[datatype]/[runfile]
   //
   //exists. If it does, the returned value is kTRUE (=1), in which case the FileStat_t object
   //contains information about the file.
   //For a remote data repository, this is not possible.
   //Always returns kFALSE.
   Warning("GetFileInfo",
           "Always returns false for remote data repository");
   return kFALSE;
}

//___________________________________________________________________________

Bool_t KVRemoteDataRepository::CheckFileStatus(const KVDataSet*,
      const Char_t*,
      const Char_t*)
{
   //Checks if the run file of given type is physically present in dataset subdirectory,
   //i.e. (schematically), if
   //
   //      /root_of_data_repository/[datasetdir]/[datatype]/[runfile]
   //
   //exists. If it does, the returned value is kTRUE (=1).
   //For a remote data repository, this is not possible.
   //Always returns kFALSE.
   Warning("CheckFileStatus",
           "Always returns false for remote data repository");
   return kFALSE;
}

//___________________________________________________________________________

void KVRemoteDataRepository::CopyFileFromRepository(const KVDataSet*,
      const Char_t*,
      const Char_t*,
      const Char_t*)
{
   //Copy file [datasetdir]/[datatype]/[filename] from the repository to [destination]
   //We check if the file to copy exists.
   //For remote repositories, this uses the bbftp protocol (if available).
   //NEEDS IMPLEMENTATION.
   Info("CopyFileFromRepository", "Not implemented yet");
}

//___________________________________________________________________________

int KVRemoteDataRepository::CopyFileToRepository(const Char_t*,
      const KVDataSet*,
      const Char_t*,
      const Char_t*)
{
   //Copy file [source] to [datasetdir]/[datatype]/[filename] in the repository
   //For remote repositories, this uses the bbftp protocol (if available).
   //NEEDS IMPLEMENTATION.
   Info("CopyFileToRepository", "Not implemented yet");
   return -1;
}

//___________________________________________________________________________

void KVRemoteDataRepository::MakeSubdirectory(const KVDataSet*,
      const Char_t*)
{
   //Create a new subdirectory in the repository
   //Impossible on distant data repositories.
   Info("MakeSubdirectory", "Impossible on remote data repositories");
}

//___________________________________________________________________________

KVUniqueNameList* KVRemoteDataRepository::GetDirectoryListing(const KVDataSet*,
      const Char_t*, const Char_t*)
{
   //Impossible on distant data repositories.
   //Will return 0 (null pointer).
   Info("GetDirectoryListing", "Impossible on remote data repositories");
   return 0;
}

//___________________________________________________________________________

void KVRemoteDataRepository::DeleteFile(const KVDataSet*,
                                        const Char_t*,
                                        const Char_t*,
                                        Bool_t)
{
   //Delete repository file [datasetdir]/[datatype]/[filename]
   //Impossible on distant data repositories.
   Info("DeleteFile", "Impossible on remote data repositories");
}

//___________________________________________________________________________

KVDataSetManager* KVRemoteDataRepository::NewDataSetManager()
{
   //Create and return pointer to new data set manager
   return (new KVRemoteDataSetManager);
}

//___________________________________________________________________________

const Char_t* KVRemoteDataRepository::GetFullPathToTransferFile(const KVDataSet* dataset,
      const
      Char_t*
      datatype,
      const
      Char_t*
      runfile)
{
   //Used by KVDataTransfer.
   //Returns the full path needed to transfer a runfile belonging to the given dataset
   //either from or to the repository, using sftp or bbftp etc.
   //This is a concatenation of the repository root directory with the dataset
   //subdirectories and filename.
   //However, for a remote data repository, the type of operating system may be different
   //to gSystem, i.e. the system on the local machine. In this case, gSystem->ConcatFileName
   //may not necessarily give the right paths for the remote machine: e.g. local machine
   //is Windows, remote repository is Linux, paths for Linux machine will have a mix of
   //"/" and "\" in them.
   //We try to remedy this by looking at fLocalrootdir: if it contains "/" we take the result
   //of the concatenation performed by KVDataRepository::GetFullPathToTransferFile
   //and replace any "\" by "/", and vice versa.

   static TString path;
   path =
      KVDataRepository::GetFullPathToTransferFile(dataset, datatype,
            runfile);
   if (fLocalrootdir.Contains("/"))
//      path.ReplaceAll("\\", "/");
      path = gSystem->UnixPathName(path.Data());
   else if (fLocalrootdir.Contains("\\"))
      path.ReplaceAll("/", "\\");
   return path.Data();
}

//___________________________________________________________________________

const Char_t* KVRemoteDataRepository::GetFullPathToOpenFile(const KVDataSet* dataset,
      const Char_t*
      datatype,
      const Char_t*
      runfile)
{
   //When accessing remote data repositories, the operating system corresponding
   //to the remote file system may not be the same as the local operating system
   //(corresponding to gSystem). Therefore use of gSystem->ConcatFileName may lead
   //to paths which mix '/' and '\'.
   //We check for this here by converting the result of KVDataRepository::GetFullPath
   //depending on whether the root directory of this repository contains '/' or '\'
   static TString path;
   path =
      KVDataRepository::GetFullPathToOpenFile(dataset, datatype,
            runfile);
   if (fLocalrootdir.Contains("/"))
      path = gSystem->UnixPathName(path.Data());
   else if (fLocalrootdir.Contains("\\"))
      path.ReplaceAll("/", "\\");
   return path.Data();
}

//______________________________________________________________________________________________//

Bool_t KVRemoteDataRepository::IsConnected()
{
   //Before reading data in a remote data repository, it may be necessary to 'connect' to the repository
   //in some way: e.g. we may need to open an SSH tunnel to an xrootd server.
   //Call this method to make sure any required connection has been made, and if successful
   //the method returns kTRUE; otherwise return kFALSE.
   //
   //XROOTD via SSH tunnel
   //===================
   //Configure using the following environment variables:
   //
   // ccali.DataRepository.XRDTunnel.host:       ccali.in2p3.fr
   // ccali.DataRepository.XRDTunnel.port:          10000
   //
   //The host and port to use are obligatory. Optionally you may also change the following:
   //
   // ccali.DataRepository.XRDTunnel.retry:  #max number of tries before giving up on connection, default = 30
   //
   // ccali.DataRepository.XRDTunnel.user:  #username for login to tunnel host. default is local username.

   if (fXRDtunnel) {
      //check existence of SSH tunnel to xrootd server
      TSocket* sock = new TSocket("localhost", fXRDtunPort);
      Int_t iRetry = 0;
      if (!sock->IsValid()) {
         //open tunnel
         TString command = "xterm -e \"ssh -X -L";
         command += fXRDtunSpec;
         command += " ";
         command += Form("%s@%s\" &", fXRDtunUser.Data(), fXRDtunHost.Data());
         gSystem->Exec(command.Data());
         cout << "Waiting for connection to xrootd server via SSH tunnel ";
         while (!sock->IsValid() && iRetry < fXRDtunRetry) {
            cout << ".";
            cout.flush();
            gSystem->Sleep(2000);
            sock->Close();
            delete sock;
            sock = new TSocket("localhost", fXRDtunPort);
            iRetry++;
         }
         cout << endl;
      }
      sock->Close();
      delete sock;
      if (iRetry == fXRDtunRetry) {
         cout << "Connection timeout " << endl;
         return kFALSE;
      }
   }
   return kTRUE;
}
