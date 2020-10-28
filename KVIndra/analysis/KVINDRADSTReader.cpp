#include "KVINDRADSTReader.h"
#include "KVDataRepository.h"

ClassImp(KVINDRADSTReader)

void KVINDRADSTReader::InitRun()
{
   Info("InitRun", "ds InitRun");

   KVString dst_file = gDataSet->GetFullPathToRunfile("dst", fRunNumber);
   Info("InitRun", "dst file %s", dst_file.Data());

   Info("InitRun", "%s %s", gDataSet->GetDataPathSubdir(), gDataSet->GetRunfileName("dst", fRunNumber));
   gDataRepository->CopyFileFromRepository(gDataSet, "dst", gDataSet->GetRunfileName("dst", fRunNumber), ".");

   Info("InitRun", "Starting analysis of run %d", fRunNumber);

   TDatime now1;
   Info("InitRun", "Debut lecture DST %s", now1.AsString());

   DefineSHELLVariables();

}

void KVINDRADSTReader::ProcessRun()
{
   gSystem->Exec(Form("faire_arbre_c%d", fCampNumber));
}

void KVINDRADSTReader::EndRun()
{
   TDatime now2;
   Info("EndRun", "Fin lecture DST %s", now2.AsString());

   KVString inst;
   inst.Form("rm -f %s", gDataSet->GetRunfileName("dst", fRunNumber));
   gSystem->Exec(inst);
}

void KVINDRADSTReader::SubmitTask()
{

   if (gDataSet != GetDataSet()) GetDataSet()->cd();
   SetCampagneNumber();

   if (fCampNumber == -1) return;

   //InitAnalysis();
   Info("SubmitTask", "Liste de runs : %s", GetRunList().AsString());
   //loop over runs
   Info("SubmitTask", "RunningInLaunchDirectory : %d", Int_t(RunningInLaunchDirectory()));

   GetRunList().Begin();
   while (!GetRunList().End()) {
      Info("SubmitTask", "%s", GetBatchName());
      fRunNumber = GetRunList().Next();
      Info("SubmitTask", "Traitement du Run %d", fRunNumber);
      InitRun();
      ProcessRun();
      EndRun();
   }
}

//_____________________________________________________________________________________________//
void KVINDRADSTReader::DefineSHELLVariables()
{
   KVString shell_var;

   shell_var.Form("run%d", fRunNumber);
   gSystem->Setenv("RUN_PREFIX", shell_var.Data());

   camp1 = camp2 = camp4 = kFALSE;

   if (fCampNumber == 1) {

      shell_var.Form("%s/veda%d/files/", gSystem->ExpandPathName("$THRONG_DIR"), fCampNumber);
      gSystem->Setenv("VEDA_DATA", shell_var.Data());

      camp1 = kTRUE;

   }
   else if (fCampNumber == 2) {

      shell_var.Form("%s/veda%d/data/", gSystem->ExpandPathName("$THRONG_DIR"), fCampNumber);
      gSystem->Setenv("VEDA_DATA", shell_var.Data());

      camp2 = kTRUE;

   }
   else if (fCampNumber == 4) {

      shell_var.Form("%s/veda%d/data/", gSystem->ExpandPathName("$THRONG_DIR"), fCampNumber);
      gSystem->Setenv("VEDA_DATA", shell_var.Data());

      camp4 = kTRUE;

   }

}
