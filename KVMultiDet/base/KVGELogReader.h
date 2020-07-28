//Created by KVClassFactory on Fri Nov 25 09:53:18 2011
//Author: John Frankland

#ifndef __KVGELOGREADER_H
#define __KVGELOGREADER_H

#include "KVLogReader.h"

/**
   \class KVGELogReader
   \brief Read GE (Grid Engine) log files to extract status of batch jobs
   \ingroup Core

Updated to handle correctly new format of end-of-job informations:

~~~~~~~~
 **********************************************************************
 * Submitted on:                   Tue Apr 14 11:27:31 CEST 2020      *
 * Started on:                     Tue Apr 14 11:27:42 CEST 2020      *
 * Ended on:                       Tue Apr 14 12:02:19 CEST 2020      *
 * Exit status:                    0                                  *
 **********************************************************************
 * Requested                                                          *
 *   CPU cores:                    1 core(s)                          *
 *   CPU time:                     00:33:20 (2000 seconds)            *
 **********************************************************************
 * Consumed                                                           *
 *   wallclock:                    00:34:37 (2077 seconds)            *
 *   CPU time:                     00:32:14 (1934 seconds)            *
 *   CPU scaling factor:           10.430000                          *
 *   normalized CPU time:          05:36:18 (20178 HS06 seconds)      *
 *   CPU efficiency:               93 % (1)                           *
 *   vmem:                         1.284 GB (2)                       *
 *   maxvmem:                      1.284 GB (2)                       *
 *   maxrss:                       407.348 MB (2)                     *
 **********************************************************************
~~~~~~~~
*/
class KVGELogReader : public KVLogReader {
   virtual Int_t GetByteMultiplier(const KVString& unit);
   virtual void ReadLine(const KVString& line, Bool_t&);
   virtual void ReadCPU(const KVString& line);
   virtual void ReadScratchUsed(const KVString& line);
   virtual void ReadMemUsed(const KVString& line);
   virtual void ReadStatus(const KVString& line);
   virtual Double_t ReadStorage(const KVString& stor);
   virtual void ReadKVCPU(const KVString& line);

   Bool_t fInRequested;
   Bool_t fInConsumed;

public:
   KVGELogReader()
      : fInRequested(kFALSE), fInConsumed(kFALSE)
   {}
   virtual ~KVGELogReader() {}
   virtual void Reset()
   {
      KVLogReader::Reset();
      fInRequested = fInConsumed = kFALSE;
   }

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

   }

   ClassDef(KVGELogReader, 1) //Read GE (Grid Engine) log files
};

#endif
