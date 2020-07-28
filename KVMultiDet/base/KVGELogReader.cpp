//Created by KVClassFactory on Fri Nov 25 09:53:18 2011
//Author: John Frankland

#include "KVGELogReader.h"
#include "TObjString.h"
#include "Riostream.h"
#include <TObjArray.h>

ClassImp(KVGELogReader)


void KVGELogReader::ReadLine(const KVString& line, Bool_t& ok)
{
   //analyse contents of line read from log file

   KVLogReader::ReadLine(line, ok);

   if (!ok) return;

   if (line.Contains("Cputime limit exceeded")) {
      ok = kFALSE;
      fStatus = line;
      fOK = kFALSE;
      return;
   }
   else if (line.Contains("Filesize limit exceeded")) {
      ok = kFALSE;
      fStatus = line;
      fOK = kFALSE;
      return;
   }
   else if (line.Contains("File size limit exceeded")) {
      ok = kFALSE;
      fStatus = line;
      fOK = kFALSE;
      return;
   }
   else if (line.Contains("Job received KILL signal")) {
      fStatus = "KILLED";
      fOK = kFALSE;
   }
   else if (line.BeginsWith("* Requested")) {
      fInRequested = kTRUE;
      fInConsumed = kFALSE;
   }
   else if (line.BeginsWith("* Consumed")) {
      fInRequested = kFALSE;
      fInConsumed = kTRUE;
   }
   else if (line.BeginsWith("*   CPU time:")) {
      ReadCPU(line);
   }
   else if (line.Contains("CpuUser ="))
      ReadKVCPU(line);
   else if (fInConsumed && line.BeginsWith("*   vmem:"))
      ReadMemUsed(line);
   else if (fStatus != "KILLED" && line.BeginsWith("* Exit status:"))
      ReadStatus(line);
   else if (line.Contains("Jobname:"))
      ReadJobname(line);
   else if (line.Contains("DISK_REQ") || line.Contains("MEM_REQ"))
      ReadStorageReq(line);
}

void KVGELogReader::ReadKVCPU(const KVString& line)
{
   // update infos on CPU time, memoire & disk from lines such as
   // "CpuSys = 7.044505  s.    CpuUser = 846.259888 s.    ResMem = 338.109375 MB   VirtMem = 1039.921875 MB      DiskUsed = 5678742 KB"

   line.Begin("= ");
   while (!line.End()) {
      KVString token = line.Next();
      if (token == "CpuUser") {
         fCPUused = line.Next().Atof();
      }
      else if (token == "VirtMem") {
         fMemKB = line.Next().Atof();
         fMemKB *= GetByteMultiplier(line.Next());
      }
      else if (token == "DiskUsed") {
         fScratchKB = ReadStorage(line.Next());
      }
   }
}

void KVGELogReader::ReadCPU(const KVString& line)
{
   //read line of type "*   CPU time:                     00:32:14 (1934 seconds)            *"
   //which is either the requested CPU time or the consumed CPU time, depending on
   //where we are in the file

   line.Begin("()");
   line.Next();
   // at this point, myline.Next() will give "1934 seconds"
   // we can directly call TString::Atoi() as it ignores any non-numeric stuff
   if (fInRequested) fCPUreq = line.Next().Atoi();
   else if (fInConsumed) fCPUused = line.Next().Atoi();
}

void KVGELogReader::ReadScratchUsed(const KVString&)
{
   // this just sets fScratcKB=0 because there is no information on
   // scratch disk usage in the GE logfiles
   fScratchKB = 0;
}

void KVGELogReader::ReadMemUsed(const KVString& line)
{
   //read line of type "*   vmem:                         1.284 GB (2)                       *"
   //corresponding to memory used by job

   if (line.GetNValues(":(") < 3) {
      //in case Grid Engine goes west & doesn't write vmem in log
      return;
   }
   line.Begin(":(");
   line.Next();
   // at this point line.Next(kTRUE) will give "1.284 GB"
   KVString vmem = line.Next(kTRUE);
   if (vmem.GetNValues(" ") < 2) {
      //something wrong here
      return;
   }
   vmem.Begin(" ");
   Double_t mem_use = vmem.Next().Atof();
   //value read is converted to KB, depending on units
   fMemKB = mem_use * GetByteMultiplier(vmem.Next());
}

Int_t KVGELogReader::GetByteMultiplier(const KVString& unit)
{
   //unit = "K", "M" or "G"
   //value returned is 1, 2**10 or 2**20, respectively
   static Int_t KB = 1;
   static Int_t MB = 2 << 9;
   static Int_t GB = 2 << 19;
   return (unit.BeginsWith("K") ? KB : (unit.BeginsWith("M") ? MB : GB));
}

void KVGELogReader::ReadStatus(const KVString& line)
{
   //read line of type
   //
   //    "* Exit status:             0                                  *"
   //
   //with final status of job.
   //
   //if status = "0" then JobOK() will return kTRUE
   //otherwise, JobOK() will be kFALSE.

   fGotStatus = kTRUE;
   line.Begin("*:");
   line.Next();
   fStatus = line.Next(kTRUE);
   fOK = (fStatus == "0");
}

Double_t KVGELogReader::ReadStorage(const KVString& stor)
{
   //'stor' is a string such as "200MB", "3GB" etc.
   //value returned is corresponding storage space in KB
   //if no units symbol found, we assume MB

   static KVString units[] = { "K", "M", "G" };
   Int_t i = 0, index = -1;
   while ((index = stor.Index(units[i])) < 0 && i < 2)
      i++;
   KVString u, STOR(stor);
   if (index >= 0) {
      STOR.Remove(index);
      u = units[i];
   }
   else {
      u = "M";
   }
   return (STOR.Atof() * GetByteMultiplier(u));
}

