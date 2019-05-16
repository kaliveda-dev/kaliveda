#include <KVDataRepositoryManager.h>
#include <KVDataSetManager.h>
#include <KVEventReconstructor.h>
#include <KVGANILDataReader.h>
#include <iostream>
#include "TApplication.h"
#include "TROOT.h"
#include "KVINDRAReconEvent.h"
using namespace std;

int main(int, char* [])
{
   KVBase::InitEnvironment();

   KVDataRepositoryManager drm;
   drm.Init();
   gDataSetManager->GetDataSet("INDRA_camp5")->cd();
   unique_ptr<KVGANILDataReader> raw_file(gDataSet->OpenRunfile<KVGANILDataReader>("raw", 6800));

   KVEventReconstructor erec(gMultiDetArray, new KVINDRAReconEvent);
   KVGroupReconstructor::SetDoIdentification(true);
   KVGroupReconstructor::SetDoCalibration(true);

   for (int i = 0; i < 500; ++i) {
      std::cout << std::endl << "==============================================EVENT " << i << std::endl << std::endl;
      if (!raw_file->GetNextEvent()) break;
      gMultiDetArray->HandleRawDataEvent(raw_file.get());
      erec.ReconstructEvent(raw_file->GetFiredDataParameters());
      erec.GetEvent()->SetNumber(i + 1);
      erec.GetEvent()->ls();
   }
}
