//Created by KVClassFactory on Fri Oct  1 16:50:15 2010
//Author: John Frankland,,,,

#ifndef __KVMEMORYCHUNK_H
#define __KVMEMORYCHUNK_H
#include "Rtypes.h"
#include <cstdio>

/**
  \class KVMemoryChunk
  \brief Memory handled by KVMemoryPool
  \ingroup Core
 */
class KVMemoryChunk {
   char* fMemory;//pointer to start of chunk
   size_t fSize;//size of chunk in bytes
   size_t fUsed;//memory used in bytes
   KVMemoryChunk* fNext;//next chunk of memory

public:
   KVMemoryChunk();
   KVMemoryChunk(size_t);
   virtual ~KVMemoryChunk();

   void* GetMemory(size_t);

   void SetNext(KVMemoryChunk* n)
   {
      fNext = n;
   };
   KVMemoryChunk* Next() const
   {
      return fNext;
   };

   void Print();

   ClassDef(KVMemoryChunk, 0) //Memory chunk in pool
};

#endif
