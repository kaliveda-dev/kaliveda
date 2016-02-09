/*
$Id: KVDataTransferBBFTP.cpp,v 1.4 2007/03/16 15:13:16 cussol Exp $
$Revision: 1.4 $
$Date: 2007/03/16 15:13:16 $
*/

//Created by KVClassFactory on Mon Sep 18 15:28:27 2006
//Author: franklan

#include "KVDataTransferBBFTP.h"
#include "KVDataSet.h"
#include "KVDataSetManager.h"
#include "KVDataRepository.h"
#include "KVBase.h"
#include "Riostream.h"
#include "TError.h"

using namespace std;

ClassImp(KVDataTransferBBFTP)
////////////////////////////////////////////////////////////////////////////////
// File transfer between repositories using bbftp.
////////////////////////////////////////////////////////////////////////////////
KVDataTransferBBFTP::KVDataTransferBBFTP()
{
   //Default constructor
   fServer = 0;
}

KVDataTransferBBFTP::~KVDataTransferBBFTP()
{
   //Destructor
}

void KVDataTransferBBFTP::init()
{
   //Initialisation of data transfer.
   //A bbftp client must be installed on the user's machine, and the
   //executable must be located within the user's PATH.

   //Deduce from target and source repository properties which has the BBFTP server
   if (!strcmp(fSourceRep->GetFileTransferType(), "bbftp"))
      fServer = fSourceRep;
   else
      fServer = fTargetRep;
}

void KVDataTransferBBFTP::WriteTransferScript()
{
   //Write BBFTP command file to transfer all files requested by user

   fCmdFile = "bbftp.cmd";
   ofstream file;
   KVBase::OpenTempFile(fCmdFile, file);

   cout << endl << "Writing command file : " << fCmdFile.
        Data() << endl << endl;

   file << "setoption remoterfio" << endl;
   cout << "setoption remoterfio" << endl;

   //command lines begin with 'put' if target repository = bbftp server,
   //or 'get' if source repository = bbftp server
   TString getput("put ");
   if (fServer == fSourceRep)
      getput = "get ";

   //write a line for each file to transfer
   //loop over runs in runlist
   fRunList.Begin();
   TString targ_file;
   while (!fRunList.End()) {

      Int_t run = fRunList.Next();      // current run number

      file << getput.Data();
      cout << getput.Data();

      //source file name
      file << fSourceRep->GetFullPathToTransferFile(fDataSet,
            fDataType.Data(),
            fDataSet->
            GetRunfileName
            (fDataType.Data(),
             run)) << " ";
      cout << fSourceRep->GetFullPathToTransferFile(fDataSet,
            fDataType.Data(),
            fDataSet->
            GetRunfileName
            (fDataType.Data(),
             run)) << " ";

      //target file name
      //target repository dataset pointer
      KVDataSet* targ_ds =
         fTargetRep->GetDataSetManager()->GetDataSet(fDataSet->GetName());
      targ_file = fDataSet->GetRunfileName(fDataType.Data(), run);
      //if the target repository is the local repository,
      //we have to replace illegal characters like ":" in the target file name
      //to avoid problems due to the meaning of the ":" character for some
      //systems (MacOsX, Windows, some Linux distributions)
      if (!fTargetRep->IsRemote())
         targ_file.ReplaceAll(":", "_");
      file << fTargetRep->GetFullPathToTransferFile(targ_ds,
            fDataType.Data(),
            targ_file.
            Data()) << endl;
      cout << fTargetRep->GetFullPathToTransferFile(targ_ds,
            fDataType.Data(),
            targ_file.
            Data()) << endl;
   }

   file.close();
   cout << endl;
}

void KVDataTransferBBFTP::ExecuteCommand()
{
   //Performs the transfer of files by executing the command
   //
   // [bbftp command] -i [command file name] -u [user] [server]
   //
   // where
   //
   // [bbftp command] = result of searching for 'bbftp' in user's PATH
   // [command file name] = name of file written by WriteTransferScript()
   // [user] = username to login to bbftp server
   // [server] = name of bbftp server
   TString command = fTransferExec + " -V -i " + fCmdFile;
   command += " -u ";
   command += fServer->GetFileTransferUser();
   command += " ";
   command += fServer->GetFileTransferServer();
   cout << endl << "Executing: " << command.Data() << endl << endl;
   gSystem->Exec(command.Data());
   //mop up results file with name = fCmdFile + ".res"
   TString mopup = fCmdFile + ".res";
   gSystem->Unlink(mopup.Data());
}
