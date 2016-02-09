//Created by KVClassFactory on Fri Nov 25 09:53:18 2011
//Author: John Frankland

#include "KVGELogReader.h"
#include "TObjString.h"
#include "Riostream.h"
#include <TObjArray.h>

ClassImp(KVGELogReader)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVGELogReader</h2>
<h4>Read GE (Grid Engine) log files</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVGELogReader::KVGELogReader()
{
   // Default constructor
   fileCopiedtoSRB = kFALSE;
}

KVGELogReader::~KVGELogReader()
{
   // Destructor
}


void KVGELogReader::ReadLine(TString& line, Bool_t& ok)
{
   //analyse contents of line read from log file

   KVLogReader::ReadLine(line, ok);
   //if we find a seg fault after having copied a file to SRB (raw->recon, recon->ident, ident->root tasks)
   //then we ignore it: the job is good!
   if (!ok && fileCopiedtoSRB) {
      ok = kTRUE;
      fOK = kTRUE;
   }
   if (!ok) return;

   if (line.Contains("LOCAL:") && line.Contains("->SRB:")) {
      fileCopiedtoSRB = kTRUE;
   } else if (line.Contains("Cputime limit exceeded")) {
      ok = kFALSE;
      fStatus = line;
      fOK = kFALSE;
      return;
   } else if (line.Contains("Filesize limit exceeded")) {
      ok = kFALSE;
      fStatus = line;
      fOK = kFALSE;
      return;
   } else if (line.Contains("File size limit exceeded")) {
      ok = kFALSE;
      fStatus = line;
      fOK = kFALSE;
      return;
   } else if (line.Contains("cpu time:"))
      ReadCPULimit(line);
   else if (line.Contains("CpuUser ="))
      ReadKVCPU(line);
   else if (line.Contains(" vmem:"))
      ReadMemUsed(line);
   else if (line.Contains("Exit status:"))
      ReadStatus(line);
   else if (line.Contains("Jobname:"))
      ReadJobname(line);
   else if (line.Contains("DISK_REQ") || line.Contains("MEM_REQ"))
      ReadStorageReq(line);
}

void KVGELogReader::ReadKVCPU(TString& line)
{
   // update infos on CPU time, memoire & disk from lines such as
   // CpuUser = 137.8598 s.     VirtMem = 250.32647 MB      DiskUsed = 356M

   TObjArray* toks = line.Tokenize("= ");
   Int_t ntoks = toks->GetEntries();
   for (int i = 0; i < ntoks; i++) {
      KVString token = ((TObjString*)toks->At(i))->String();
      if (token == "CpuUser") {
         fCPUused = ((TObjString*)toks->At(i + 1))->String().Atof();
      } else if (token == "VirtMem") {
         fMemKB = ((TObjString*)toks->At(i + 1))->String().Atof();
         fMemKB *= GetByteMultiplier(((TObjString*)toks->At(i + 2))->String());
      } else if (token == "DiskUsed") {
         token = ((TObjString*)toks->At(i + 1))->String();
         fScratchKB = ReadStorage(token);
      }
   }
}

void KVGELogReader::ReadCPULimit(TString& line)
{
   //read line of type "*   cpu time:              693 / 15000                        *"
   //which contains total normalized used CPU time and the time limit from the job request

   TObjArray* toks = line.Tokenize("*:/ ");
   //get used CPU time (in seconds)
   KV__TOBJSTRING_TO_INT(toks, 2, sec)
   if (sec) fCPUused = sec; //if there is a GE problem, could be "0 / 3000"
   //in this case we use values printed at end of job by KV
   //get requested CPU time (in seconds)
   KV__TOBJSTRING_TO_INT(toks, 3, sec_)
   fCPUreq = sec_;
   delete toks;
}

void KVGELogReader::ReadScratchUsed(TString&)
{
   // this just sets fScratcKB=0 because there is no information on
   // scratch disk usage in the GE logfiles
   fScratchKB = 0;
}

void KVGELogReader::ReadMemUsed(TString& line)
{
   //read line of type "*   vmem:                  406.879M                           *"
   //corresponding to memory used by job

   TObjArray* toks = line.Tokenize("*: ");
   if (toks->GetEntries() < 2) {
      //in case Grid Engine goes west & doesn't write vmem in log
      delete toks;
      return;
   }
   //value read is converted to KB, depending on units
   TString vmem = ((TObjString*)toks->At(1))->String();
   TString units = vmem[vmem.Length() - 1];
   vmem.Remove(vmem.Length() - 1);
   fMemKB = vmem.Atof() * GetByteMultiplier(units);
   delete toks;
}

Int_t KVGELogReader::GetByteMultiplier(TString& unit)
{
   //unit = "K", "M" or "G"
   //value returned is 1, 2**10 or 2**20, respectively
   static Int_t KB = 1;
   static Int_t MB = 2 << 9;
   static Int_t GB = 2 << 19;
   return (unit.BeginsWith("K") ? KB : (unit.BeginsWith("M") ? MB : GB));
}

void KVGELogReader::ReadStatus(TString& line)
{
   //read line of type "* Exit status:             0                                  *"
   //with final status of job.
   //if status = "0" then JobOK() will return kTRUE
   //otherwise, JobOK() will be kFALSE

   fGotStatus = kTRUE;
   TObjArray* toks = line.Tokenize("*: ");
   fStatus = ((TObjString*) toks->At(2))->String();
   fOK = (fStatus == "0");
   delete toks;
}

Double_t KVGELogReader::ReadStorage(KVString& stor)
{
   //'stor' is a string such as "200M", "3G" etc.
   //value returned is corresponding storage space in KB
   //if no units symbol found, we assume MB (as output of 'du -h')

   static const Char_t* units[] = { "K", "M", "G" };
   Int_t i = 0, index = -1;
   while ((index = stor.Index(units[i])) < 0 && i < 2)
      i++;
   TString u;
   if (index >= 0) {
      stor.Remove(index);
      u = units[i];
   } else {
      u = "M";
   }
   return (stor.Atof() * GetByteMultiplier(u));
}

