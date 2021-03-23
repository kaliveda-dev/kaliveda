//Created by KVClassFactory on Fri Apr  6 16:33:15 2018
//Author: John Frankland,,,

#ifndef __KVMFMDATAFILEREADER_H
#define __KVMFMDATAFILEREADER_H

#include "KVRawDataReader.h"
#include "MFMFileReader.h"
#include "KVNameValueList.h"
#ifdef WITH_MESYTEC
#include "mesytec_buffer_reader.h"
#endif
/**
\class KVMFMDataFileReader
\brief Read MFM format acquisition data
  \ingroup DAQ

This class uses the mfmlib package available from: https://gitlab.in2p3.fr/jdfcode/mfmlib.git
(enabled by default if package found).
*/

class KVMFMDataFileReader : public KVRawDataReader, public MFMFileReader {

#ifdef WITH_MESYTEC
   mutable mesytec::buffer_reader MTEC_bufrdr;
#endif

public:
   KVMFMDataFileReader(const Char_t* filepath);
   virtual ~KVMFMDataFileReader() {}

   /// Read next frame in file. Initialise merge manager if frame is a merge frame
   Bool_t GetNextEvent()
   {
      bool ok = ReadNextFrame();
      return ok;
   }
   KVSeqCollection* GetFiredDataParameters() const
   {
      return nullptr;
   }

   static KVMFMDataFileReader* Open(const Char_t* filepath, Option_t* = "");

   TString GetDataFormat() const
   {
      return "MFM";
   }
   void SetActionsDirectory(const string&);
   void SetActionsExpName(const string&);

   Int_t GetRunNumberReadFromFile() const;

   TString GetPathToLastEbyedatActionsFile();

#ifdef WITH_MESYTEC
   void InitialiseMesytecConfig(const std::string& crate, const std::string& channels);
   mesytec::buffer_reader& GetMesytecBufferReader() const
   {
      return MTEC_bufrdr;
   }
#endif

   ClassDef(KVMFMDataFileReader, 0) //Read MFM format acquisition data
};

#endif
