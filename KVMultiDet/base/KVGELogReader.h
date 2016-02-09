//Created by KVClassFactory on Fri Nov 25 09:53:18 2011
//Author: John Frankland

#ifndef __KVGELOGREADER_H
#define __KVGELOGREADER_H

#include "KVLogReader.h"

class KVGELogReader : public KVLogReader {
   virtual Int_t GetByteMultiplier(TString& unit);
   virtual void ReadLine(TString& line, Bool_t&);
   virtual void ReadCPULimit(TString& line);
   virtual void ReadScratchUsed(TString& line);
   virtual void ReadMemUsed(TString& line);
   virtual void ReadStatus(TString& line);
   virtual Double_t ReadStorage(KVString& stor);
   virtual void ReadKVCPU(TString& line);
   Bool_t fileCopiedtoSRB;

public:
   KVGELogReader();
   virtual ~KVGELogReader();
   virtual void Reset()
   {
      KVLogReader::Reset();
      fileCopiedtoSRB = kFALSE;
   };

   virtual Bool_t Incomplete() const
   {
      return (KVLogReader::Incomplete()
              ||
              ((!Killed() && !SegFault())  &&
               (
                  (fStatus == "Cputime limit exceeded")
                  || (fStatus == "Filesize limit exceeded")
               )
              )
             );

   };

   ClassDef(KVGELogReader, 1) //Read GE (Grid Engine) log files
};

#endif
