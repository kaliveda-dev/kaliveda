/*******************************************************************************
$Id: KVLogReader.cpp,v 1.13 2008/12/08 14:07:37 franklan Exp $
*******************************************************************************/
#include "KVLogReader.h"
#include "Riostream.h"
#include "TObjArray.h"
#include "TObjString.h"
#include "KVConfig.h"

using namespace std;

ClassImp(KVLogReader)

KVLogReader::KVLogReader()
{
   fFMT = "%d";
   Reset();
}

void KVLogReader::Reset()
{
   //reset informations read from file
   fCPUused = fCPUreq = fSCRreq = fMEMreq = fScratchKB = fMemKB = 0;
   fStatus = "";
   fOK = kFALSE;
   fGotRequests = kFALSE;
   fGotStatus = kFALSE;
}

void KVLogReader::ReadFile(const Char_t* fname)
{
   //Open file 'fname' and read contents

   Reset();                     //clear last read infos

   ifstream filestream(fname);
   if (!filestream.good()) {
      cout << "Error in KVLogReader::ReadFile - cannot open file " << fname
           << endl;
      return;
   }
   KVString line;
   line.ReadLine(filestream);
   Bool_t ok = kTRUE;
   while (filestream.good() && ok) {
      ReadLine(line, ok);       //perform action based on content of line
      line.ReadLine(filestream);
   }
   filestream.close();
}

void KVLogReader::ReadLine(const KVString& line, Bool_t& ok)
{
   //analyse contents of line read from log file
   //
   //if line contains "segmentation" then we put ok=kFALSE to stop reading file
   //
   //status will equal the contents of the current line

   ok = kTRUE;
   if (line.Contains("egmentation")) {
      ok = kFALSE;
      fStatus = line;
      fOK = kFALSE;
   }
   else if (line.Contains("Temps limite atteint")) {
      //VEDA FORTRAN programme 'out of time' error seen during DST conversion
      ok = kFALSE;
      fStatus = "VEDA Fortran out of time";
      fOK = kFALSE;
   }
   else if (line.Contains("rfcp : Input/output error")) {
      //failure to copy new run to hpss system at ccali
      ok = kFALSE;
      fStatus = "rfcp error";
      fOK = kFALSE;
   }
   else if (line.Contains("TXNetFile") && line.Contains("open attempt failed")) {
      //failure to open recon file with xrootd
      ok = kFALSE;
      fStatus = "XROOTD error";
      fOK = kFALSE;
   }
}

Double_t KVLogReader::GetCPUratio() const
{
   //calculate ratio of used CPU to requested CPU
   return (fCPUreq ? fCPUused / fCPUreq : 0.);
}

void KVLogReader::ReadStorageReq(const KVString& line)
{
   //Read lines "DISK_REQ:                 3GB" and "MEM_REQ:         150MB"
   //and fill corresponding variable with appropriate value

   line.Begin("*:");
   KVString type = line.Next(kTRUE);
   KVString stor = line.Next(kTRUE);
   Double_t size = ReadStorage(stor);
   if (type == "DISK_REQ")
      fSCRreq = size;
   else if (type == "MEM_REQ")
      fMEMreq = size;
   // both requests must be present in the logfile otherwise
   // something very wrong has happened
   if (fSCRreq && fMEMreq) fGotRequests = kTRUE;
}

void KVLogReader::ReadJobname(const KVString& line)
{
   //read line of type "* Jobname:                 run4049                            *"
   //with name of job.
   unique_ptr<TObjArray> toks(line.Tokenize("*: "));
   fJobname = ((TObjString*) toks->At(1))->GetString();
}

Int_t KVLogReader::GetRunNumber() const
{
   //try to get run number from jobname using format string fFMT
   Int_t run;
   sscanf(fJobname.Data(), fFMT.Data(), &run);
   return run;
}

Bool_t KVLogReader::Incomplete() const
{
   // job considered incomplete if
   //  - it was not 'killed'
   //  - it did not end in segmentation fault/violation
   //  AND
   //  - the end of job status report was not found
   //  - OR the disk & memory requests were not found
   //  - OR the status indicates the job was incomplete

   return (
             (!Killed() && !SegFault())
             &&
             (
                (!fGotStatus)
                || (!fGotRequests)
                || (fStatus == "VEDA Fortran out of time")
                || (fStatus.BeginsWith("rfcp"))
             )
          );
};
