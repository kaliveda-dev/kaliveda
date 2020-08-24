/*
$Id: KVDataTransfer.h,v 1.5 2007/01/04 16:38:50 franklan Exp $
$Revision: 1.5 $
$Date: 2007/01/04 16:38:50 $
*/

//Created by KVClassFactory on Mon Sep 18 14:27:21 2006
//Author: franklan

#ifndef __KVDATATRANSFER_H
#define __KVDATATRANSFER_H

#include "KVDataSetAnalyser.h"
#include "TString.h"

class KVDataRepository;

/**
  \class KVDataTransfer
  \brief Base class for transferring data between data repositories
  \ingroup Infrastructure

 If the user has defined two or more repositories in her $HOME/.kvrootrc file,
 e.g. as in the example #3 given in $KVROOT/KVFiles/.kvrootrc:

    #Case 3: user has data files on his machine or on a locally-mounted disk, and can access
    #a remote data repository using xrootd via an SSH tunnel on port 10000 (the example given
    #is for the xrootd server at ccali centre de calcul).

    #definition of local repository
    DataRepository: default
    default.DataRepository.Type: local
    default.DataRepository.RootDir: $(HOME)/Data

    #definition of remote repository
    +DataRepository: ccali
    ccali.DataRepository.Type: remote
    ccali.DataRepository.RootDir:       cchpssindra:/hpss/in2p3.fr/group/indra
    ccali.DataRepository.ReadProtocol:     root
    ccali.DataRepository.XRDServer:      localhost:10000
    ccali.DataRepository.XRDRootDir:       /hpss/in2p3.fr/group/indra
    ccali.DataRepository.RemoteAvailableRuns.protocol:  curl
    ccali.DataRepository.RemoteAvailableRuns.url:   http://indra.in2p3.fr/KaliVedaDoc
    ccali.DataRepository.FileTransfer.type:    bbftp
    ccali.DataRepository.FileTransfer.server:    ccbbftp.in2p3.fr
    #if your login name is not the same as for ccali, give it here
    ccali.DataRepository.FileTransfer.user:

 then it is possible to transfer files from one to the other by running one of the following
 commands on the local machine:

    KVDataTransfer::NewTransfer("ccali", "default")->Run() // transfer from ccali to local machine
    KVDataTransfer::NewTransfer("default", "ccali")->Run() // transfer from local machine to ccali

 The same menu-driven approach as for submitting analysis tasks is used to
 allow the user to choose among the available datasets, systems and runs to
 transfer. The transferred files are copied in to the target repository, creating any
 missing directories which may be needed.

 It is also possible to use a KVDataTransfer object interactively, in this case
 you should use:

    KVDataTransfer* trans = KVDataTransfer::NewTransfer("ccali", "default");
    trans->SetDataSet(...);
    trans->SetDataType(...);
    trans->SetRuns(...);
    trans->TransferRuns();

 To use the BBFTP transfer facility, a bbftp client must be installed on the user's machine
 If the client is not located in the user's PATH, he should give the full path to the executable
 in the configuration, e.g. on WinXP with cygwin version of bbftp client:

    ccali.DataRepository.FileTransfer.type:    c:\cygwin\bin\bbftp.exe
*/
class KVDataTransfer: public KVDataSetAnalyser {
protected:
   KVDataRepository* fSourceRep;        // repository containing source files
   KVDataRepository* fTargetRep;        // repository where files will be copied
   TString fCmdFile;            //name of command file given to transfer agent
   TString fTransferExec;      //full path to transfer client executable

   virtual void SetTransferExec(const Char_t* path)
   {
      fTransferExec = path;
   }
   virtual void ChooseAnalysisTask()
   {
   }
   virtual void SubmitTask()
   {
   }

   virtual void ExecuteCommand() = 0;
   virtual void WriteTransferScript() = 0;

   virtual void CheckTargetRepository();

   virtual void init();
   Bool_t fOK;                  //may be set to kFALSE by init(), in which case Run() will abort
   virtual void set_dataset_pointer(KVDataSet* ds);
   virtual void set_dataset_name(const Char_t* name);

public:

   KVDataTransfer();
   virtual ~ KVDataTransfer();
   virtual void Run();
   virtual void TransferRuns();

   static KVDataTransfer* NewTransfer(const Char_t* source_rep,
                                      const Char_t* target_rep);

   ClassDef(KVDataTransfer, 0)  //Transfers data between data repositories
};

#endif
