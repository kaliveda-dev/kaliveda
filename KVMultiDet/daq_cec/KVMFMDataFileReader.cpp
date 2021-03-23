//Created by KVClassFactory on Fri Apr  6 16:33:15 2018
//Author: John Frankland,,,

#include "KVMFMDataFileReader.h"
#include "TSystem.h"
#include "TError.h"
#include "MFMXmlFileHeaderFrame.h"

ClassImp(KVMFMDataFileReader)

KVMFMDataFileReader::KVMFMDataFileReader(const Char_t* filepath)
   : KVRawDataReader(), MFMFileReader(filepath, 100 * 1024 * 1024)
{
   // Open the datafile with given path
   // Read the first frame in the file (should be a MFMXmlFileHeaderFrame) and extract informations
   //
   // If the variable
   //    KVMFMDataFileReader.ActionsDirectory
   // has been set, it will be used to search for EBYEDAT actions files in order to
   // decode any EBYEDAT data in the file

   std::string actions_dir = gEnv->GetValue("KVMFMDataFileReader.ActionsDirectory", "");
   if (actions_dir != "") SetActionsDirectory(actions_dir);
   std::string actions_expname = gEnv->GetValue("KVMFMDataFileReader.ActionsExpName", "");
   if (actions_expname != "") SetActionsExpName(actions_expname);
   if (!ReadNextFrame()) {
      Error("KVMFMDataFileReader", "Cannot read file %s", filepath);
      MakeZombie();
      return;
   }
   if (GetFrameReadType() != MFM_XML_FILE_HEADER_FRAME_TYPE) {
      Warning("KVMFMDataFileReader", "First frame in file is not MFM_XML_FILE_HEADER_FRAME_TYPE: type is %s",
              GetFrameReadTypeSymbol().c_str());
   }
   else {
      GetFrameRead().Print();
      fRunInfos.SetValue("ExperimentName", GetFrameRead<MFMXmlFileHeaderFrame>().GetExperimentName());
      fRunInfos.SetValue("FileName", GetFrameRead<MFMXmlFileHeaderFrame>().GetFileName());
      fRunInfos.SetValue("FileCreationTime", GetFrameRead<MFMXmlFileHeaderFrame>().GetFileCreationTime());
      fRunInfos.SetValue("RunNumber", GetFrameRead<MFMXmlFileHeaderFrame>().GetRunNumber());
      fRunInfos.SetValue("RunIndex", GetFrameRead<MFMXmlFileHeaderFrame>().GetRunIndex());
      fRunInfos.SetValue("RunStartTime", GetFrameRead<MFMXmlFileHeaderFrame>().GetRunStartTime());
   }
}

KVMFMDataFileReader* KVMFMDataFileReader::Open(const Char_t* filepath, Option_t*)
{
   TString fp(filepath);
   if (fp.Contains('$')) gSystem->ExpandPathName(fp);
   ::Info("KVMFMDataFileReader::Open", "Opening file %s...", fp.Data());
   return new KVMFMDataFileReader(fp);
}

void KVMFMDataFileReader::SetActionsDirectory(const string& d)
{
   // Set directory in which to look for Ebyedat ACTIONS_* files
   // Defaults to data directory
   GetFrameLibrary().SetEbyedatActionsDirectory(d);
}

void KVMFMDataFileReader::SetActionsExpName(const string& e)
{
   // Set experiment name for Ebyedat ACTIONS_* files.
   //
   // Setting this means that a single ACTIONS file will be looked for in the actions directory
   // (set with SetActionsDirectory).
   GetFrameLibrary().SetEbyedatExpName(e);
}

Int_t KVMFMDataFileReader::GetRunNumberReadFromFile() const
{
   // Return run number of file currently being read.
   // Only call once file has been successfully opened.

   return fRunInfos.GetIntValue("RunNumber");
}

TString KVMFMDataFileReader::GetPathToLastEbyedatActionsFile()
{
   // Returns full path to last ACTIONS file used to decode Ebyedat parameters
   return GetFrameLibrary().GetLastEbyedatActionsFile().c_str();
}

#ifdef WITH_MESYTEC
void KVMFMDataFileReader::InitialiseMesytecConfig(const std::string& crate, const std::string& channels)
{
   // Read the two files whose full paths are given as argument in order to set up the
   // Mesytec crate config and module-channel-detector correspondence

   mesytec::experimental_setup exp;
   exp.read_crate_map(crate);
   exp.read_detector_correspondence(channels);
   MTEC_bufrdr.define_setup(exp);
}
#endif
