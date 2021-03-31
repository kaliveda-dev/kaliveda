//Created by KVClassFactory on Mon Jan 14 12:10:22 2019
//Author: John Frankland,,,

#ifndef __KVProtobufDataReader_H
#define __KVProtobufDataReader_H

#include "KVRawDataReader.h"
/**
  \class KVProtobufDataReader
\brief Read Google Protobuf DAQ files
  \ingroup DAQ
*/

class KVProtobufDataReader : public KVRawDataReader {
protected:
   Int_t fBufSize;//! buffer size
   char* fBuffer;//! current buffer
#ifdef USING_ROOT6
   unique_ptr<TFile> fFile;//! TFile plugin handle
#else
   TFile* fFile;//! TFile plugin handle
#endif
   Long64_t fFileSize;//! size of file in bytes
   UInt_t fEvSize;//! size of next event in buffer
   ptrdiff_t fEvOffset;//! next position to read in buffer
   bool fNeedToReadBuffer;//! true when the buffer is empty/incomplete
   bool fReachedEndOfFile;//! true when we have read all bytes from file

   virtual bool read_buffer();

   UInt_t get_remaining_readable_buffer() const
   {
      // number of bytes remaining to read in buffer starting from current
      // position fEvOffset
      return (UInt_t)(fBufSize - fEvOffset);
   }

   virtual bool parse_event_from_message() = 0;

   void open_file(const Char_t* filepath);

public:
   KVProtobufDataReader(Int_t bufSiz = 16 * 1024 * 1024);
   KVProtobufDataReader(const Char_t* filepath, Int_t bufSiz = 16 * 1024 * 1024);
   virtual ~KVProtobufDataReader();

   Bool_t GetNextEvent();
   TString GetDataFormat() const
   {
      return "PROTOBUF";
   }
   Int_t GetRunNumberReadFromFile() const;

   ClassDef(KVProtobufDataReader, 1) //Read Google Protobuf DAQ files
};

#endif
