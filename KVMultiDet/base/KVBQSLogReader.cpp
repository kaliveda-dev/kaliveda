//Created by KVClassFactory on Fri Nov 25 09:53:04 2011
//Author: John Frankland

#include "KVBQSLogReader.h"
#include "TObjString.h"
#include "TObjArray.h"

ClassImp(KVBQSLogReader)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVBQSLogReader</h2>
<h4>Read BQS log files</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVBQSLogReader::KVBQSLogReader()
{
   // Default constructor
}

KVBQSLogReader::~KVBQSLogReader()
{
   // Destructor
}


void KVBQSLogReader::ReadLine(TString& line, Bool_t& ok)
{
   //analyse contents of line read from log file
   //if line contains "segmentation" then we put ok=kFALSE to stop reading file
   //status will equal the contents of the current line

   KVLogReader::ReadLine(line, ok);
   if (!ok) return;

   if (line.Contains("time limit"))
      ReadCPULimit(line);
   else if (line.Contains("SCRATCH:"))
      ReadScratchUsed(line);
   else if (line.Contains("VIRTUAL STORAGE:"))
      ReadMemUsed(line);
   else if (line.Contains("with status:"))
      ReadStatus(line);
   else if (line.Contains("Jobname:"))
      ReadJobname(line);
   else if (line.Contains("DISK_REQ") || line.Contains("MEM_REQ"))
      ReadStorageReq(line);
}


void KVBQSLogReader::ReadCPULimit(TString& line)
{
   //read line of type "*     total normalized:    68:57:52 (time limit: 100:00:00)   *"
   //which contains total normalized used CPU time and the time limit from the job request
   TObjArray* toks = line.Tokenize("(:) ");

   //get used CPU time (in seconds)
   KV__TOBJSTRING_TO_INT(toks, 3, hrs)
   KV__TOBJSTRING_TO_INT(toks, 4, mins)
   KV__TOBJSTRING_TO_INT(toks, 5, sec)
   fCPUused = 3600 * hrs + 60 * mins + sec;
   //get requested CPU time (in seconds)
   KV__TOBJSTRING_TO_INT(toks, 8, hrs_)
   KV__TOBJSTRING_TO_INT(toks, 9, mins_)
   KV__TOBJSTRING_TO_INT(toks, 10, sec_)
   fCPUreq = 3600 * hrs_ + 60 * mins_ + sec_;
   delete toks;
}

void KVBQSLogReader::ReadScratchUsed(TString& line)
{
   //read line of type "* SCRATCH:                 1042 MB                            *"
   //corresponding to disk space used by job
   TObjArray* toks = line.Tokenize("*: ");
   //value read is converted to KB, depending on units
   KV__TOBJSTRING_TO_INT(toks, 1, ScratchKB_)
   fScratchKB = ScratchKB_;
   TString units = ((TObjString*) toks->At(2))->GetString();
   fScratchKB *= GetByteMultiplier(units);
   delete toks;
}

void KVBQSLogReader::ReadMemUsed(TString& line)
{
   //read line of type "* VIRTUAL STORAGE:         103 MB                             *"
   //corresponding to memory used by job
   TObjArray* toks = line.Tokenize("*: ");
   //value read is converted to KB, depending on units
   KV__TOBJSTRING_TO_INT(toks, 2, MemKB_)
   fMemKB = MemKB_;
   TString units = ((TObjString*) toks->At(3))->GetString();
   fMemKB *= GetByteMultiplier(units);
   delete toks;
}

Int_t KVBQSLogReader::GetByteMultiplier(TString& unit)
{
   //unit = "KB", "MB" or "GB"
   //value returned is 1, 2**10 or 2**20, respectively
   static Int_t KB = 1;
   static Int_t MB = 2 << 9;
   static Int_t GB = 2 << 19;
   return (unit == "KB" ? KB : (unit == "MB" ? MB : GB));
}

void KVBQSLogReader::ReadStatus(TString& line)
{
   //read line of type "* with status:             ENDED                              *"
   //with final status of job.
   //if status = "ENDED" then JobOK() will return kTRUE
   //otherwise, JobOK() will be kFALSE

   fGotStatus = kTRUE;
   TObjArray* toks = line.Tokenize("*: ");
   fStatus = ((TObjString*) toks->At(2))->GetString();
   fOK = (fStatus == "ENDED");
   delete toks;
}

Double_t KVBQSLogReader::ReadStorage(KVString& stor)
{
   //'stor' is a string such as "200MB", "3GB" etc.
   //value returned is corresponding storage space in KB
   static const Char_t* units[] = { "KB", "MB", "GB" };
   Int_t i = 0, index = -1;
   while ((index = stor.Index(units[i])) < 0 && i < 2)
      i++;
   if (index < 0)
      return 0;
   stor.Remove(index);
   TString u(units[i]);
   return (stor.Atof() * GetByteMultiplier(u));
}

