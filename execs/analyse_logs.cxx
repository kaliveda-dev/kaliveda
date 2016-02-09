#ifdef BQS_LOG_READER
#include "KVBQSLogReader.h"
#else
#include "KVGELogReader.h"
#endif
#include "KVNumberList.h"
#include "KVNameValueList.h"
#include "Riostream.h"
#include "TSystemDirectory.h"
#include "TSystemFile.h"

using namespace std;

void UpdateLimits(KVNameValueList& limits, KVNumberList& list, Double_t dummy, const TString& max, const TString& min)
{
   if (list.GetNValues() == 1) {
      limits.SetValue(max, dummy);
      limits.SetValue(min, dummy);
   } else {
      if (dummy > limits.GetDoubleValue(max)) limits.SetValue(max, dummy);
      else if (dummy < limits.GetDoubleValue(min)) limits.SetValue(min, dummy);
   }
}

void UpdateStatistics(Int_t run, Double_t& SCRreq, const KVGELogReader& log_reader, KVNameValueList& avg,
                      KVNameValueList& limits, KVNumberList& list, Double_t& MEMreq, Double_t& CPUreq)
{
   list.Add(run);
   CPUreq = log_reader.GetCPUrequest();
   MEMreq = log_reader.GetMEMrequest();
   SCRreq = log_reader.GetSCRATCHrequest();
   avg.IncrementValue("CPU", log_reader.GetCPUused());
   avg.IncrementValue("SCR", log_reader.GetSCRATCHused());
   avg.IncrementValue("MEM", log_reader.GetMEMused());
   //max and min
   Double_t dummy = log_reader.GetCPUused();
   UpdateLimits(limits, list, dummy, "maxCPU", "minCPU");
   dummy = log_reader.GetMEMused();
   UpdateLimits(limits, list, dummy, "maxMEM", "minMEM");
   dummy = log_reader.GetSCRATCHused();
   UpdateLimits(limits, list, dummy, "maxSCR", "minSCR");
}

void PrintStatistics(Double_t MEMreq_ok, const KVNameValueList& OKavg, const KVNameValueList& OKlimits,
                     Double_t SCRreq_ok, Double_t CPUreq_ok, const KVNumberList& ok)
{
   Double_t nok = (Double_t)((KVNumberList&)ok).GetNValues();
   cout << "\n     CPU (req / min / avg / max) :   (";
   cout << CPUreq_ok << " / ";
   cout << OKlimits.GetDoubleValue("minCPU") << " / ";
   cout << OKavg.GetDoubleValue("CPU") / nok << " / ";
   cout << OKlimits.GetDoubleValue("maxCPU") << ")" << endl;
   cout << "     MEM [MB] (req / min / avg / max) :   (";
   cout << MEMreq_ok / 1024. << " / ";
   cout << OKlimits.GetDoubleValue("minMEM") / 1024. << " / ";
   cout << OKavg.GetDoubleValue("MEM") / nok / 1024. << " / ";
   cout << OKlimits.GetDoubleValue("maxMEM") / 1024. << ")" << endl;
   cout << "     SCRATCH [MB] (req / min / avg / max) :   (";
   cout << SCRreq_ok / 1024. << " / ";
   cout << OKlimits.GetDoubleValue("minSCR") / 1024. << " / ";
   cout << OKavg.GetDoubleValue("SCR") / nok / 1024. << " / ";
   cout << OKlimits.GetDoubleValue("maxSCR") / 1024. << ")" << endl << endl;
}

int main(int argc, char** argv)
{
   if (argc < 2) {
      cout << "\t\tAnalysis of ";
#ifdef BQS_LOG_READER
      cout << "BQS";
#else
      cout << "Grid Engine";
#endif
      cout << " (CCIN2P3) batch log files" << endl << endl;
      cout << "\tanalyse_logs [jobname]" << endl << endl;
      cout << "\targuments are:" << endl;
      cout << "\t\t[jobname] common root of name of all jobs to analyse" << endl;
      cout << "\t\t      e.g. \"PbAu29MeV\" for jobs with names like \"PbAu29MeV_R8213\" etc." << endl;
      cout << "\t\t      The programme will attempt to read all files with names like" << endl;
      cout << "\t\t          PbAu29MeV_R8213.o5647093" << endl;
      cout << "\t\t          PbAu29MeV_R8214.o5647094" << endl;
      cout << "\t\t      etc. in the current directory" << endl;
      return 0;
   }
#ifdef BQS_LOG_READER
   KVBQSLogReader log_reader;
#else
   KVGELogReader log_reader;
#endif

   // take root of job name and add suffix "_R"
   KVString nameForm(argv[1]);
   nameForm += "_R";
   KVString nameFormLR = nameForm + "%d"; //format for KVLogReader
   log_reader.SetNameFormat(nameFormLR.Data());

   //number lists for listing runs according to status: OK, out of time, killed, seg fault
   KVNumberList ok, oot, kill, seg;
   //max and min CPU, SCRATCH, VIRTUAL STORAGE for OK jobs
   KVNameValueList OKlimits;
   //average CPU, SCRATCH, VIRTUAL STORAGE for OK jobs
   KVNameValueList OKavg;
   Double_t Dzero = 0.;
   OKavg.SetValue("CPU", Dzero);
   OKavg.SetValue("SCR", Dzero);
   OKavg.SetValue("MEM", Dzero);
   //max and min CPU, SCRATCH, VIRTUAL STORAGE for killed jobs
   KVNameValueList KILLlimits;
   //average CPU, SCRATCH, VIRTUAL STORAGE for killed jobs
   KVNameValueList KILLavg;
   KILLavg.SetValue("CPU", Dzero);
   KILLavg.SetValue("SCR", Dzero);
   KILLavg.SetValue("MEM", Dzero);
   //max and min CPU, SCRATCH, VIRTUAL STORAGE for incomplete jobs
   KVNameValueList INCOMPlimits;
   //average CPU, SCRATCH, VIRTUAL STORAGE for incomplete jobs
   KVNameValueList INCOMPavg;
   INCOMPavg.SetValue("CPU", Dzero);
   INCOMPavg.SetValue("SCR", Dzero);
   INCOMPavg.SetValue("MEM", Dzero);
   Double_t CPUreq_ok, MEMreq_ok, SCRreq_ok;
   CPUreq_ok = MEMreq_ok = SCRreq_ok = 0;
   Double_t CPUreq_kill, MEMreq_kill, SCRreq_kill;
   CPUreq_kill = MEMreq_kill = SCRreq_kill = 0;
   Double_t CPUreq_incomp, MEMreq_incomp, SCRreq_incomp;
   CPUreq_incomp = MEMreq_incomp = SCRreq_incomp = 0;
   KVNameValueList incompleteStatus;//status of each incomplete job

   //loop over files in current directory
   TSystemDirectory thisDir(".", ".");
   TList* fileList = thisDir.GetListOfFiles();
   TIter nextFile(fileList);
   int nfile = 0;
   TSystemFile* aFile = 0;
   while ((aFile = (TSystemFile*)nextFile())) {

      // look for files with names like [nameForm]1234.o6579780
      KVString fileName = aFile->GetName();
      if (!fileName.BeginsWith(nameForm.Data())) continue;
      if (!fileName.Contains(".o")) continue;

      nfile++;

      log_reader.ReadFile(fileName.Data());

      Int_t run = log_reader.GetRunNumber();
      if (log_reader.JobOK()) {
         UpdateStatistics(run, SCRreq_ok, log_reader, OKavg, OKlimits, ok, MEMreq_ok, CPUreq_ok);
      } else if (log_reader.Killed()) {
         UpdateStatistics(run, SCRreq_kill, log_reader, KILLavg, KILLlimits, kill, MEMreq_kill, CPUreq_kill);
      } else if (log_reader.SegFault()) seg.Add(run);
      else if (log_reader.Incomplete()) {
         incompleteStatus.SetValue(Form("Run %d", run), log_reader.GetStatus());
         UpdateStatistics(run, SCRreq_incomp, log_reader, INCOMPavg, INCOMPlimits, oot, MEMreq_incomp, CPUreq_incomp);
      }
   }

#ifdef BQS_LOG_READER
   cout << "BQS log analysis==============>" << endl;
#else
   cout << "GridEngine log analysis==============>" << endl;
#endif
   cout << "Analysed " << nfile << " jobs" << endl;
   cout << endl;
   cout << "      ";
   cout << ok.GetNValues() << " jobs were OK \n\n" << ok.GetList() << endl;
   if (ok.GetNValues()) {
      PrintStatistics(MEMreq_ok, OKavg, OKlimits, SCRreq_ok, CPUreq_ok, ok);
   }
   cout << "      ";
   cout << kill.GetNValues() << " jobs were KILLED :\n\n" << kill.GetList() << endl;
   if (kill.GetNValues()) {
      PrintStatistics(MEMreq_kill, KILLavg, KILLlimits, SCRreq_kill, CPUreq_kill, kill);
   }
   cout << "      ";
   cout << oot.GetNValues() << " jobs were INCOMPLETE :\n\n" << oot.GetList() << endl << endl;
   if (oot.GetNValues()) {
      PrintStatistics(MEMreq_incomp, INCOMPavg, INCOMPlimits, SCRreq_incomp, CPUreq_incomp, oot);
      incompleteStatus.Print();
   }
   cout << "      ";
   cout << seg.GetNValues() << " jobs had SEGMENTATION FAULT :\n\n" << seg.GetList() << endl << endl;

   cout << "If you want to delete any or all of these log files, use 'delete_logs':" << endl << endl;
   cout << "\t\tdelete_logs " << argv[1] << " \"34-67 98 123-567\"" << endl << endl;
   cout << "If you want to resubmit e.g. incomplete jobs, click on the 'Runlist' button in" << endl;
   cout << "KaliVedaGUI and copy and paste the runlist which appears above into the dialogue box" << endl;
   return 0;
}
