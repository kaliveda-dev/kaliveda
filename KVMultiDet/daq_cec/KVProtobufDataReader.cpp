//Created by KVClassFactory on Mon Jan 14 12:10:22 2019
//Author: John Frankland,,,

#include "KVProtobufDataReader.h"

#include <google/protobuf/io/coded_stream.h>

ClassImp(KVProtobufDataReader)

#define KV_PROTOBUF_MSG_SIZE 4

Bool_t KVProtobufDataReader::GetNextEvent()
{
   // Prepare to read next event in buffer.
   //  - if a new buffer is needed, we read one from the file
   //  - at current position in buffer we read the size of the next protobuf message
   //  - we check if the entire message is contained within the buffer
   //    if not, the part of the message already read is moved to the start of the buffer
   //    then the rest of the buffer is filled from the file
   //  - if this method returns true, then fEvOffset and fEvSize are correctly positioned
   //    in order for the message to be read (handled by daughter class with specific
   //    protobuf message format)
   //  - if this method returns false, then either a major problem occurred when reading
   //    the file, or we have reached the end and there are no more events/messages to read

   if (fReachedEndOfFile) {
      // derived classes may override read_buffer() so that it opens a new file
      if (!read_buffer()) return false;
   }

   // do we have enough bytes left (4) in buffer to read size of next event?
   if (get_remaining_readable_buffer() < KV_PROTOBUF_MSG_SIZE) fNeedToReadBuffer = true;

   if (fNeedToReadBuffer)
      if (!read_buffer()) return false;

   // get size of next event in buffer
   fEvSize = 0;
   {
      // Read the message size (4-bytes/32-bit integer)
      google::protobuf::io::CodedInputStream codedIStream((const uint8_t*)(fBuffer + fEvOffset), KV_PROTOBUF_MSG_SIZE);
      codedIStream.ReadLittleEndian32(&fEvSize);
      if (fEvSize == 0) {
         Error("GetNextEvent", "read zero size event");
         return false;
      }
   }
   //Info("GetNextEvent", "event size=%d bytes", fEvSize);

   fEvOffset += KV_PROTOBUF_MSG_SIZE;
   // do we have enough bytes left to read the event?
   if (get_remaining_readable_buffer() < fEvSize) {
      fNeedToReadBuffer = true;
      if (!read_buffer()) return false;
   }

   if (!parse_event_from_message()) {
      // problem parsing event - try to read next event
      fEvOffset += fEvSize;
      return GetNextEvent();
   }

   fEvOffset += fEvSize;

   return true;
}

bool KVProtobufDataReader::read_buffer()
{
   // read a buffer from the file

   if (fReachedEndOfFile) {
      // last read reached end of file
      return false;
   }
   ptrdiff_t fFillOffset = 0;
   // if fEvOffset>0 we need to copy the remaining bytes of the last buffer to the
   // beginning of the buffer and adjust fFillOffset accordingly
   if (fEvOffset > 0 && get_remaining_readable_buffer()) {
      //Info("read_buffer", "Copying last %d bytes of old buffer to beginning of new buffer", get_remaining_readable_buffer());
      memcpy(fBuffer, (fBuffer + fEvOffset), get_remaining_readable_buffer());
      fFillOffset = get_remaining_readable_buffer();
   }
   Int_t bytes_to_read = fBufSize - fFillOffset;
   if (bytes_to_read > fFileSize) bytes_to_read = fFileSize;
   //Info("read_buffer", "fFillOffset:%ld      bytes to read:%d bytes", fFillOffset, bytes_to_read);
   Long64_t old_bytes = fFile->GetBytesRead();
   fFile->ReadBuffer((char*)(fBuffer + fFillOffset), bytes_to_read);
   Long64_t bytes_read = fFile->GetBytesRead() - old_bytes;
   //Info("read_buffer", "Read %d bytes from file", (Int_t)bytes_read);
   fFileSize -= bytes_read;
   if (bytes_read == 0 || fFileSize == 0 || bytes_read < (Int_t)(fBufSize - fFillOffset)) fReachedEndOfFile = true;
   fNeedToReadBuffer = false;
   fEvOffset = 0;
   return (bytes_read > 0);
}

void KVProtobufDataReader::open_file(const Char_t* filepath)
{
   TString fp(filepath);
   fp.Append("?filetype=raw");
#if ROOT_VERSION_CODE >= ROOT_VERSION(6,0,0)
   fFile.reset(TFile::Open(fp));
#else
   SafeDelete(fFile);
   fFile = TFile::Open(fp);
#endif
   fFileSize = fFile->GetSize();
   Info("open_file", "%s : size of file = %lld bytes", filepath, fFileSize);
   fEvOffset = 0;
   fNeedToReadBuffer = true;
   fReachedEndOfFile = false;
}

KVProtobufDataReader::KVProtobufDataReader(const Char_t* filepath, Int_t bufSiz)
   : KVRawDataReader(),
     fBufSize(bufSiz), fBuffer(new char[bufSiz]), fFile(nullptr), fEvSize(0), fEvOffset(0), fNeedToReadBuffer(true),
     fReachedEndOfFile(false)
{
   // Open Google protobuf file for reading. Filepath URL will be passed to TFile::Open
   // therefore can use same plugins eg. "root://" etc.
   // Default buffer size: 16MB
   // Note: buffer size given as Int_t, as this is argument type required by TFile::ReadBuffer

   open_file(filepath);
}

KVProtobufDataReader::KVProtobufDataReader(Int_t bufSiz)
   : KVRawDataReader(),
     fBufSize(bufSiz), fBuffer(new char[bufSiz]), fFile(nullptr), fFileSize(0), fEvSize(0), fEvOffset(0), fNeedToReadBuffer(true),
     fReachedEndOfFile(false)
{
   // Create file reader of given buffer size, do not open any files yet
}
//____________________________________________________________________________//

KVProtobufDataReader::~KVProtobufDataReader()
{
   // Destructor
   delete [] fBuffer;
#if ROOT_VERSION_CODE < ROOT_VERSION(6,0,0)
   SafeDelete(fFile);
#endif
}

Int_t KVProtobufDataReader::GetRunNumberReadFromFile() const
{
   return 0;
}

