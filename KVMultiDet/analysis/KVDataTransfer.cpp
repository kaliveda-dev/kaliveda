/*
$Id: KVDataTransfer.cpp,v 1.6 2007/05/31 09:59:22 franklan Exp $
$Revision: 1.6 $
$Date: 2007/05/31 09:59:22 $
*/

//Created by KVClassFactory on Mon Sep 18 14:27:21 2006
//Author: franklan

#include "KVDataTransfer.h"
#include "KVDataSet.h"
#include "KVDataRepositoryManager.h"
#include "KVDataRepository.h"
#include "KVDataSetManager.h"
#include "TError.h"
#include "TPluginManager.h"
#include "KVBase.h"
#include "KVError.h"

ClassImp(KVDataTransfer)
////////////////////////////////////////////////////////////////////////////////
// Transfers data between data repositories
//
// If the user has defined two or more repositories in her $HOME/.kvrootrc file,
// e.g. as in the example #3 given in $KVROOT/KVFiles/.kvrootrc:
//
//    #Case 3: user has data files on his machine or on a locally-mounted disk, and can access
//    #a remote data repository using xrootd via an SSH tunnel on port 10000 (the example given
//    #is for the xrootd server at ccali centre de calcul).
//
//    #definition of local repository
//    DataRepository: default
//    default.DataRepository.Type: local
//    default.DataRepository.RootDir: $(HOME)/Data
//
//    #definition of remote repository
//    +DataRepository: ccali
//    ccali.DataRepository.Type: remote
//    ccali.DataRepository.RootDir:       cchpssindra:/hpss/in2p3.fr/group/indra
//    ccali.DataRepository.ReadProtocol:     root
//    ccali.DataRepository.XRDServer:      localhost:10000
//    ccali.DataRepository.XRDRootDir:       /hpss/in2p3.fr/group/indra
//    ccali.DataRepository.RemoteAvailableRuns.protocol:  curl
//    ccali.DataRepository.RemoteAvailableRuns.url:   http://indra.in2p3.fr/KaliVedaDoc
//    ccali.DataRepository.FileTransfer.type:    bbftp
//    ccali.DataRepository.FileTransfer.server:    ccbbftp.in2p3.fr
//    #if your login name is not the same as for ccali, give it here
//    ccali.DataRepository.FileTransfer.user:
//
// then it is possible to transfer files from one to the other by running one of the following
// commands on the local machine:
//
//    KVDataTransfer::NewTransfer("ccali", "default")->Run() // transfer from ccali to local machine
//    KVDataTransfer::NewTransfer("default", "ccali")->Run() // transfer from local machine to ccali
//
// The same menu-driven approach as for submitting analysis tasks is used to
// allow the user to choose among the available datasets, systems and runs to
// transfer. The transferred files are copied in to the target repository, creating any
// missing directories which may be needed.
//
// It is also possible to use a KVDataTransfer object interactively, in this case
// you should use:
//
//    KVDataTransfer* trans = KVDataTransfer::NewTransfer("ccali", "default");
//    trans->SetDataSet(...);
//    trans->SetDataType(...);
//    trans->SetRuns(...);
//    trans->TransferRuns();
//
// To use the BBFTP transfer facility, a bbftp client must be installed on the user's machine
// If the client is not located in the user's PATH, he should give the full path to the executable
// in the configuration, e.g. on WinXP with cygwin version of bbftp client:
//
//    ccali.DataRepository.FileTransfer.type:    c:\cygwin\bin\bbftp.exe
////////////////////////////////////////////////////////////////////////////////
KVDataTransfer::KVDataTransfer()
{
   //Default constructor.
   //Use KVDataTransfer::NewTransfer to create a new data transfer object with the correct
   //properties to transfer files between two repositories.
   fOK = kTRUE;
   fSourceRep = fTargetRep = 0;
}

//_________________________________________________________________

KVDataTransfer::~KVDataTransfer()
{
   //Destructor
}

//_________________________________________________________________

KVDataTransfer* KVDataTransfer::NewTransfer(const Char_t* source_rep,
      const Char_t* target_rep)
{
   //Creates a new data transfer object to transfer files between the two named data repositories.
   //The type of the created object depends on the values of the environment variables
   //(defined in .kvrootrc file):
   //
   //source_rep.DataRepository.FileTransfer.type
   //target_rep.DataRepository.FileTransfer.type
   //
   //If either of these = bbftp, we create a KVDataTransferBBFTP object.
   //If source_rep.DataRepository.FileTransfer.type = xrd, we create a KVDataTransferXRD object.
   //By default (no type given), we use a KVDataTransferSFTP object.
   //In fact, the type of object created is defined in .kvrootrc by the following plugins:
   //
   // # Plugins for data transfer between repositories
   // # Used by KVDataTransfer::NewTransfer
   // Plugin.KVDataTransfer:   sftp    KVDataTransferSFTP   KVMultiDet   "KVDataTransferSFTP()"
   // +Plugin.KVDataTransfer:   bbftp    KVDataTransferBBFTP   KVMultiDet   "KVDataTransferBBFTP()"
   // +Plugin.KVDataTransfer:   xrd    KVDataTransferXRD   KVMultiDet   "KVDataTransferXRD()"

   KVDataRepository* SR =
      gDataRepositoryManager->GetRepository(source_rep);
   if (!SR) {
      ::Error("KVDataTransfer::NewTransfer",
              "Unknown source data repository: %s", source_rep);
      return 0;
   }
   KVDataRepository* TR =
      gDataRepositoryManager->GetRepository(target_rep);
   if (!TR) {
      ::Error("KVDataTransfer::NewTransfer",
              "Unknown target data repository: %s", target_rep);
      return 0;
   }
   TString uri = "sftp";        //default plugin
   //if either one has transfer type 'bbftp'...
   if (!strcmp(SR->GetFileTransferType(), "bbftp")
         || !strcmp(TR->GetFileTransferType(), "bbftp"))
      uri = "bbftp";
   //if source repository has transfer type 'xrd'...
   else if (!strcmp(SR->GetFileTransferType(), "xrd"))
      uri = "xrd";
   TString transfer_exec = SR->GetFileTransferExec();
   if (!strcmp(TR->GetFileTransferType(), "bbftp")) transfer_exec = TR->GetFileTransferExec();
   //if transfer_exec=="" then we cannot perform the transfer
   if (transfer_exec == "") {
      ::Error("KVDataTransfer::NewTransfer",
              "No file transfer executable to perform transfer");
      return 0;
   }

   //check and load plugin library
   TPluginHandler* ph;
   if (!(ph = KVBase::LoadPlugin("KVDataTransfer", uri)))
      return 0;

   KVDataTransfer* tran = (KVDataTransfer*) ph->ExecPlugin(0);
   //set target and source repositories
   tran->fSourceRep = SR;
   tran->fTargetRep = TR;
   //set transfer client executable path
   tran->SetTransferExec(transfer_exec);
   //initialise transfer
   tran->init();
   return tran;
}

//_________________________________________________________________

void KVDataTransfer::Run()
{
   //Perform file transfer
   //The user selects runs which are available in the source repository in the same way as
   //when performing data analysis
   //Note that this will make the 'source' repository the current active repository
   //(i.e. gDataRepository and gDataSetManager will correspond to source repository
   //after execution of this command).

   if (!fOK) {
      Error("KVDataTransfer::Run",
            "Status not OK. Data transfer aborted.");
      return;
   }
   //make 'source' repository the 'active' repository
   fSourceRep->cd();

   fMenus = kTRUE;
   fQuit = kFALSE;
   fChoozDataSet = kTRUE;
   fChoozSystem = kFALSE;
   fChoozTask = kFALSE;
   fChoozRuns = kFALSE;
   fSubmit = kFALSE;

   fDataSet = 0;
   fDataType = "";
   fRunList.Clear();
   fSystem = 0;

   //choose dataset, data type, system, runs
   while (!fQuit) {

      if (fChoozDataSet)
         ChooseDataSet();
      else if (fChoozTask)
         ChooseDataType();
      else if (fChoozSystem)
         ChooseSystem();
      else if (fChoozRuns)
         ChooseRuns();
      else if (fSubmit)
         TransferRuns();

   }
}

//_________________________________________________________________

void KVDataTransfer::TransferRuns()
{
   //Based on information gathered from user (see Run()),
   //perform the transfer of files.
   //Any missing directories in the target repository are created beforehand.
   //After transfer, the available runlist for the target repository is updated

   fSubmit = kFALSE;

   if (!fDataSet) {
      Error("KVDataTransfer::Transfer",
            "No dataset defined for transfer. Choose dataset first.");
      fChoozDataSet = kTRUE;
      return;
   }
   if (fDataType == "") {
      Error("KVDataTransfer::Transfer",
            "No data type defined for transfer. Choose type of data to transfer first.");
      fChoozTask = kTRUE;
      return;
   }
   if (fRunList.IsEmpty()) {
      Error("KVDataTransfer::Transfer",
            "No runlist defined for transfer. Choose runs to transfer first.");
      fChoozRuns = kTRUE;
      return;
   }

   CheckTargetRepository();

   WriteTransferScript();

   ExecuteCommand();

   //delete temporary command file if present
   if (fCmdFile != "") gSystem->Unlink(fCmdFile.Data());

   //update available run list for target repository
   fTargetRep->GetDataSetManager()->GetDataSet(fDataSet->GetName())->
   UpdateAvailableRuns(fDataType.Data());

   //force user to choose new system
   fChoozSystem = kTRUE;
}

//_________________________________________________________________

void KVDataTransfer::CheckTargetRepository()
{
   //Make sure that the target repository has the necessary
   //dataset/subdirectory to receive the transferred files.
   //If not, we create the new dataset/subdir.

   Bool_t update = kFALSE;
   if (!fTargetRep->GetDataSetManager()->GetDataSet(fDataSet->GetName())->
         IsAvailable()) {
      //add dataset directory to target repository
      fTargetRep->MakeSubdirectory(fDataSet);
      update = kTRUE;
   }
   if (!fTargetRep->GetDataSetManager()->GetDataSet(fDataSet->GetName())->
         HasDataType(fDataType.Data())) {
      //add subdirectory for new data type to dataset directory
      fTargetRep->MakeSubdirectory(fDataSet,
                                   fDataType.Data());
      update = kTRUE;
   }
   if (update) {
      //update dataset manager of target repository with new state of available datasets/datatypes
      fTargetRep->GetDataSetManager()->Update();
   }
}

//_________________________________________________________________

void KVDataTransfer::init()
{
   //Initialisation of data transfer.
}

//_________________________________________________________________

void KVDataTransfer::SetDataSet(const Char_t* name)
{
   //Set dataset to be analysed.
   //If 'name' is not the name of a valid and available dataset
   //in the 'source' data repository an error message is printed.

   fDataSet = 0;
   KVDataSet* ds = fSourceRep->GetDataSetManager()->GetDataSet(name);
   if (!ds) {
      Error("SetDataSet", "Unknown dataset %s", name);
   } else {
      SetDataSet(ds);
   }
}

//_________________________________________________________________

void KVDataTransfer::SetDataSet(KVDataSet* ds)
{
   //Set dataset to be used for transfer.
   //If the chosen dataset is not available, an error message is printed
   //Only datasets which are available in the source repository can be transferred
   //If the pointer actually corresponds to a dataset in the target repository,
   //we replace it with a pointer to the dataset with the same name in the source
   //repository.

   //allow user to reset dataset pointer to 0
   fDataSet = ds;
   if (!ds)
      return;

   //check repository
   if (ds->GetRepository() != fSourceRep) {
      fDataSet =
         fSourceRep->GetDataSetManager()->GetDataSet(ds->GetName());
   }
   //check availablility
   if (!ds->IsAvailable()) {
      Error("SetDataSet",
            "Dataset %s is not available for analysis", ds->GetName());
      fDataSet = 0;
   }

}
