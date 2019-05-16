#include <KVDataRepositoryManager.h>
#include <KVDataSetManager.h>
#include <KVEventReconstructor.h>
#include <KVFAZIADetector.h>
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
   gDataSetManager->GetDataSet("INDRAFAZIA.E789_test")->cd();

   KVMultiDetArray::MakeMultiDetector(gDataSet->GetName());
   gMultiDetArray->InitializeIDTelescopes();

   unique_ptr<KVMFMDataFileReader> raw_file(KVMFMDataFileReader::Open(argv[1]));

   KVEventReconstructor erec(gMultiDetArray, new KVReconstructedEvent);
   KVGroupReconstructor::SetDoIdentification(false);
   KVGroupReconstructor::SetDoCalibration(false);

   int first_frame = TString(argv[2]).Atoi();
   TFile f(Form("idmaps_%d.root", first_frame), "recreate");

   KVHashList my_hists;
//   TString idtypes[] = {"Si-Si", "Si-CsI", "CsI", "CSI", "SI-CSI", ""};
   TString idtypes[] = {"Si-CsI", ""};
   int iid = 0;
   while (idtypes[iid] != "") {
      f.mkdir(idtypes[iid]);
      f.cd(idtypes[iid]);
      unique_ptr<KVSeqCollection> idl(gMultiDetArray->GetIDTelescopesWithType(idtypes[iid]));
      if (idl.get()) {
         TIter nxt_csi(idl.get());
         KVIDTelescope* idt;
         while ((idt = (KVIDTelescope*)nxt_csi())) {
            if (idt->GetDetector(1)->InheritsFrom("KVFAZIADetector")) {
               if (1) {

                  if (idtypes[iid] == "Si-Si") my_hists.Add(new TH2F(idt->GetName(), idt->GetName(), 2000, 0, 1000, 2000, 0, 1000));
                  else if (idtypes[iid] == "Si-CsI") my_hists.Add(new TH2F(idt->GetName(), idt->GetName(), 2000, 0, 4000, 2000, 0, 500));
                  else my_hists.Add(new TH2F(idt->GetName(), idt->GetName(), 4000, 0, 2000, 4000, 0, 3000));
               }
            }
            else
               my_hists.Add(new TH2F(idt->GetName(), idt->GetName(), 2500, 0, 2500, 2500, 0, 2500));
         }
      }
      ++iid;
      f.cd();
   }
   //f.ls();
   int i = 0;
   int bad = 0;
   while (raw_file->GetNextEvent()) {
      ++i;
      if (gMultiDetArray->HandleRawDataEvent(raw_file.get())) {
         erec.ReconstructEvent(gMultiDetArray->GetFiredDataParameters());
         erec.GetEvent()->SetNumber(i + 1);
         if (erec.GetEvent()->GetMult()) {
            for (KVEvent::Iterator it = erec.GetEvent()->begin(); it != erec.GetEvent()->end(); ++it) {
               KVReconstructedNucleus& n = it.reference<KVReconstructedNucleus>();
               TIter nxt_id(n.GetReconstructionTrajectory()->GetIDTelescopes());
               KVIDTelescope* idt;
               while ((idt = (KVIDTelescope*)nxt_id())) {
                  TH2F* h = (TH2F*)my_hists.FindObject(idt->GetName());
                  if (h) {
                     //cout << idt->GetName() << endl; idt->GetIDMapX();
                     if (idt->IsType("SI_CSI")) h->Fill(idt->GetIDMapX("L"), idt->GetIDMapY("GG"));
                     else h->Fill(idt->GetIDMapX(), idt->GetIDMapY());
                  }
               }
            }
         }
      }
      else {
//         cout << "Frame " << i << " not handled by anybody:" << endl;
//         raw_file->PrintFrameRead();
         bad++;
      }
      if (!(i % 1000)) cout << "Treated " << i << " frames (" << bad << " bad)" << endl;
   }

   f.Write();
}
