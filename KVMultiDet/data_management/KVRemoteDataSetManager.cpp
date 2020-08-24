/*
$Id: KVRemoteDataSetManager.cpp,v 1.5 2007/09/20 11:30:17 franklan Exp $
$Revision: 1.5 $
$Date: 2007/09/20 11:30:17 $
*/

//Created by KVClassFactory on Thu Sep  7 15:16:29 2006
//Author: John Frankland

#include "KVRemoteDataSetManager.h"
#include "KVDataRepository.h"
#include "TError.h"
#include "TObjString.h"
#include "TObjArray.h"
#include "TEnv.h"

using namespace std;

ClassImp(KVRemoteDataSetManager)

KVRemoteDataSetManager::KVRemoteDataSetManager()
{
   //Default constructor
}

KVRemoteDataSetManager::~KVRemoteDataSetManager()
{
   //Destructor
}

Bool_t KVRemoteDataSetManager::OpenAvailableDatasetsFile()
{
   //Transfers file $KVROOT/KVFiles/[repository name].available.datasets
   //from remote machine containing info on available datasets and
   //associated subdirectories in remote data repository.
   //Copies to local working directory and opens for reading, if all goes
   //well (returns kTRUE).
   //Returns kFALSE in case of problems.
   //
   //N.B. If 'curl' is used to transfer the file, we check for HTML code in the file
   //It can happen that 'curl' itself does not give an error when the web server
   //is down, but transfers a file which looks like this:
// <!DOCTYPE HTML PUBLIC "-//IETF//DTD HTML 2.0//EN">
// <html><head>
// <title>403 Forbidden</title>
// </head><body>
// <h1>Forbidden</h1>
// <p>You don't have permission to access /KaliVedaDoc/ccali.available.datasets
// on this server.</p>
// <p>Additionally, a 403 Forbidden
// error was encountered while trying to use an ErrorDocument to handle the request.</p>
// <hr>
// <address>Apache/2.0.53 (Unix) mod_ssl/2.0.53 OpenSSL/0.9.7a DAV/2 PHP/5.1.6 mod_python/3.2.8 Python/2.4.2 mod_jk/1.2.15 mod_perl/2.0.2 Perl/v5.8.7 Server at indra.in2p3.fr Port 80</address>
// </body></html>


   TString http =
      gEnv->GetValue(Form("%s.DataRepository.RemoteAvailableRuns.url",
                          fRepository->GetName()), "");
   if (http == "") {
      Warning("OpenAvailableDatasetsFile()",
              "%s.DataRepository.RemoteAvailableRuns.url is not defined. See $KVROOT/KVFiles/.kvrootrc",
              fRepository->GetName());
      return kFALSE;
   }
   TString url;
   url.Form("%s/%s", http.Data(), fCacheFileName.Data());

   //Find executable 'curl' or programme indicated by
   //value of DataRepository.RemoteAvailableRuns.protocol
   fCurl = gEnv->GetValue(Form
                          ("%s.DataRepository.RemoteAvailableRuns.protocol",
                           fRepository->GetName()), "curl");
   if (!KVBase::FindExecutable(fCurl)) {
      Error("OpenAvailableDatasetsFile",
            "Executable for file transfer \"%s\" not found. You cannot use remote data repository.",
            fCurl.Data());
      return kFALSE;
   }
   TString cmd;
   cmd.Form("%s -oremote.available.datasets %s", fCurl.Data(), url.Data());
   if (gSystem->Exec(cmd.Data()))
      return kFALSE;            // problem with curl
   fDatasets.open("remote.available.datasets");
   if (fCurl.Contains("curl")) {
      //check contents of file for HTML code
      KVString line;
      line.ReadLine(fDatasets);
      if (line.Contains("<")) {
         Error("OpenAvailableDatasetsFile",
               "Remote available datasets file transferred by curl contains HTML code :");
         //print contents of file
         cout << line.Data() << endl;
         line.ReadLine(fDatasets);
         while (fDatasets.good()) {
            cout << line.Data() << endl;
            line.ReadLine(fDatasets);
         }
         Error("OpenAvailableDatasetsFile",
               "You cannot use this remote data repository");
         return kFALSE;
      }
      else {   // no HTML in file : OK
         //reset stream to beginning of file
         fDatasets.clear();         // clear any error flags (EOF etc.)
         fDatasets.seekg(0, ios::beg);      // set file buffer pointer to beginning of file
      }
   }
   return kTRUE;
}

void KVRemoteDataSetManager::CheckAvailability()
{
   //Check availability of datasets in repository associated to this data set manager
   //For remote data sets/repositories, this is done by transferring from the remote
   //machine the file $KVROOT/KVFiles/[repository name].available.datasets and
   //reading from it the datasets and subdirectories which are present on the remote
   //machine.
   //Example file:
   //INDRA_camp1 : root
   //INDRA_camp2 :
   //INDRA_camp4 :
   //INDRA_camp5 : raw,recon,ident,root
   //INDRA_e416a : raw,
   //INDRA_e475s :

   fNavailable = 0;
   ReadAvailableDatasetsFile();
}
