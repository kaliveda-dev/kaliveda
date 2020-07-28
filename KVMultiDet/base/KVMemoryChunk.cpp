//Created by KVClassFactory on Fri Oct  1 16:50:15 2010
//Author: John Frankland,,,,

#include "KVMemoryChunk.h"
#include <cstdlib>

ClassImp(KVMemoryChunk)

KVMemoryChunk::KVMemoryChunk()
   :   fMemory(0), fSize(0), fUsed(0), fNext(0)
{
   // Default constructor
}

KVMemoryChunk::KVMemoryChunk(size_t bytes) : fNext(0)
{
   // Allocate new chunk of size 'bytes'
   fMemory = (char*) malloc(bytes);
   fSize = bytes;
   fUsed = 0;
}

KVMemoryChunk::~KVMemoryChunk()
{
   // Destructor
   if (fMemory) free(fMemory);
   fMemory = 0;
}

void* KVMemoryChunk::GetMemory(size_t bytes)
{
   // Return pointer to block of memory of size 'bytes'
   // If no block of this size is available, returns 0 (test it!!)

   if (fUsed + bytes <= fSize) {
      void* p = (void*)(fMemory + fUsed);
      fUsed += bytes;
      return p;
   }
   return NULL;
}
void KVMemoryChunk::Print()
{
   printf("KVMemoryChunk: %lu bytes  (%lu used) at: %p\n",
          fSize, fUsed, fMemory);
};

