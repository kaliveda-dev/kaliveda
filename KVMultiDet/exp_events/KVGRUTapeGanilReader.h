//Created by KVClassFactory on Tue Nov 30 16:36:28 2010
//Author: eindra

#ifndef __KVGRUTapeGanilReader_H
#define __KVGRUTapeGanilReader_H

#include "KVRawDataReader.h"
#include "KVHashList.h"
#include "GAcq.h"
#include "KVACQParam.h"

class KVGRUTapeGanilReader : public KVRawDataReader {
   GAcq* fGRUAcq;
   Bool_t fReadFirstBuffer;
   KVHashList* fFired;
   KVHashList* fParameters;
   KVHashList* fExtParams;
   Int_t fStatus;

   void ConnectRawDataParameters();
   KVACQParam* CheckACQParam(const Char_t*);
   void FillFiredParameterList();

public:

   enum {
      kSTATUS_OK,
      kSTATUS_NOHOST,
      kSTATUS_NOBUFF,
      kSTATUS_NOEVENT,
      kSTATUS_NOACQ
   };

   Int_t GetStatus() const
   {
      return fStatus;
   };

   KVGRUTapeGanilReader(const Char_t* filename);
   virtual ~KVGRUTapeGanilReader();

   Bool_t GetNextEvent();
   virtual KVSeqCollection*  GetFiredDataParameters() const
   {
      return fFired;
   };
   static KVGRUTapeGanilReader* Open(const Char_t*, Option_t* /*opt*/ = "");

   ClassDef(KVGRUTapeGanilReader, 1) //Read GANIL acquisition events from file
};

#endif
