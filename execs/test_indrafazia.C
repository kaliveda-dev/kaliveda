#include <KVDataRepositoryManager.h>
#include <KVDataSetManager.h>
#include <KVEventReconstructor.h>
#include <KVExpSetUp.h>
#include <KVMFMDataFileReader.h>
#include <iostream>
#include "TApplication.h"
#include "TROOT.h"
#include "KVINDRA.h"
#include "KVFAZIA.h"
#include "TH2.h"
using namespace std;

int main(int, char* argv[])
{
   KVBase::InitEnvironment();

   //TApplication myapp("myapp", &argc, argv);

   KVDataRepositoryManager drm;
   drm.Init();
   gDataSetManager->GetDataSet("INDRAFAZIA")->cd();

   KVMultiDetArray::MakeMultiDetector(gDataSet->GetName());

   unique_ptr<KVMFMDataFileReader> raw_file(KVMFMDataFileReader::Open(argv[1]));

   KVEventReconstructor erec(gMultiDetArray, new KVReconstructedEvent);
   KVGroupReconstructor::SetDoIdentification(false);
   KVGroupReconstructor::SetDoCalibration(false);

   int first_frame = TString(argv[2]).Atoi();
   TFile f(Form("INDRAmult_%d.root", first_frame), "recreate");
   TH1F* hmult = new TH1F("mult", "INDRA mult", 101, -.5, 100.5);
   TH1F* hmult_faz = new TH1F("mult_faz", "INDRA mult - with FAZIA", 101, -.5, 100.5);
   TH1F* hmult_nofaz = new TH1F("mult_nofaz", "INDRA mult - without FAZIA", 101, -.5, 100.5);
   TH1F* hphi_faz = new TH1F("phi_faz", "Phi dist - with FAZIA", 100, 0, 360);
   TH1F* hphi_nofaz = new TH1F("phi_nofaz", "Phi dist - without FAZIA", 100, 0, 360);
   TH2F* hthphi_faz = new TH2F("thphi_faz", "thPhi dist - with FAZIA", 100, 0, 360, 100, 10, 90);
   TH2F* hthphi_nofaz = new TH2F("thphi_nofaz", "thPhi dist - without FAZIA", 100, 0, 360, 100, 10, 90);
   TH2F* hmult_correl = new TH2F("multcorr", "FAZIA mult vs INDRA mult", 102, -1.5, 100.5, 102, -1.5, 100.5);

   int i = 0;
   KVNameValueList multiplicities;
   while (raw_file->GetNextEvent()) {
      //std::cout << std::endl << "==============================================EVENT " << i << std::endl << std::endl;
      ++i;
      if (i != first_frame) continue;
      first_frame += 20;
      gMultiDetArray->HandleRawDataEvent(raw_file.get());
      erec.ReconstructEvent(gMultiDetArray->GetFiredDataParameters());
      erec.GetEvent()->SetNumber(i + 1);

      ((KVExpSetUp*)gMultiDetArray)->GetArrayMultiplicities(erec.GetEvent(), multiplicities);
      hmult_correl->Fill(multiplicities.GetIntValue("INDRA"), multiplicities.GetIntValue("FAZIA"));

      if (gIndra->HandledRawData()) {
         hmult->Fill(erec.GetEvent()->GetMult());
         if (gFazia->HandledRawData()) {
            hmult_faz->Fill(erec.GetEvent()->GetMult());
            erec.GetEvent()->ResetGetNextParticle();
            KVReconstructedNucleus* n;
            while ((n = erec.GetEvent()->GetNextParticle())) {
               if (n->GetStoppingDetector()) {
                  double th, ph;
                  n->GetStoppingDetector()->GetRandomAngles(th, ph);
                  hphi_faz->Fill(ph);
                  hthphi_faz->Fill(ph, th);
               }
            }
         }
         else {
            hmult_nofaz->Fill(erec.GetEvent()->GetMult());
            erec.GetEvent()->ResetGetNextParticle();
            KVReconstructedNucleus* n;
            while ((n = erec.GetEvent()->GetNextParticle())) {
               if (n->GetStoppingDetector()) {
                  double th, ph;
                  n->GetStoppingDetector()->GetRandomAngles(th, ph);
                  hphi_nofaz->Fill(ph);
                  hthphi_nofaz->Fill(ph, th);
               }
            }
         }
      }
      if (!(i % 10000)) cout << "Treated " << i << " frames..." << endl;
   }

   f.Write();
}
