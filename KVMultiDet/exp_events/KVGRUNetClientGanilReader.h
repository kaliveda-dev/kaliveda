//Created by KVClassFactory on Tue Nov 30 16:36:28 2010
//Author: eindra

#ifndef __KVGRUNETCLIENTGANILREADER_H
#define __KVGRUNETCLIENTGANILREADER_H

#include "KVRawDataReader.h"
#include "KVHashList.h"
class GAcq;
class GSpectra;
#include "KVACQParam.h"

class KVGRUNetClientGanilReader : public KVRawDataReader {
   GAcq* fGRUAcq;
   Bool_t fReadFirstBuffer;
   KVHashList* fFired;
   KVHashList* fParameters;
   KVHashList* fExtParams;
   Int_t fTimeout; // time out in seconds to wait for buffer
   Int_t fStatus;
   Int_t fSpecServPort;

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

   KVGRUNetClientGanilReader();
   virtual ~KVGRUNetClientGanilReader();

   Bool_t GetNextEvent();
   virtual KVSeqCollection*  GetFiredDataParameters() const
   {
      return fFired;
   };
   static KVGRUNetClientGanilReader* Open(const Char_t*, Option_t* /*opt*/ = "");
   Int_t GetTimeOut() const
   {
      return fTimeout;
   };
   void SetTimeOut(Int_t sec)
   {
      // Set timeout in seconds to wait for buffer
      fTimeout = sec;
   };
   GSpectra* GetSpectraServer() const;
   Int_t GetSpectraServerPort() const
   {
      return fSpecServPort;
   };

   ClassDef(KVGRUNetClientGanilReader, 1) //Read GANIL acquisition events over network connection
};

#endif
