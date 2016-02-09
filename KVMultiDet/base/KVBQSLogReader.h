//Created by KVClassFactory on Fri Nov 25 09:53:04 2011
//Author: John Frankland

#ifndef __KVBQSLOGREADER_H
#define __KVBQSLOGREADER_H

#include "KVLogReader.h"

class KVBQSLogReader : public KVLogReader {
   virtual Int_t GetByteMultiplier(TString& unit);
   virtual void ReadLine(TString& line, Bool_t&);
   virtual void ReadCPULimit(TString& line);
   virtual void ReadScratchUsed(TString& line);
   virtual void ReadMemUsed(TString& line);
   virtual void ReadStatus(TString& line);
   virtual Double_t ReadStorage(KVString& stor);

public:
   KVBQSLogReader();
   virtual ~KVBQSLogReader();


   ClassDef(KVBQSLogReader, 1) //Read BQS log files
};

#endif
