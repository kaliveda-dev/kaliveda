//Created by KVClassFactory on Fri Sep  9 14:27:28 2011
//Author: eindra

#include "KVINDRAOnlineDataAnalyser.h"
#include "GSpectra.h"
#include "GNetServerRoot.h"
#include "TH1.h"
#include "KVGRUNetClientGanilReader.h"
#include "KVMultiDetArray.h"
#include "TServerSocket.h"
#include "TMonitor.h"
#include "TMessage.h"
#include "TThread.h"

ClassImp(KVINDRAOnlineDataAnalyser)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVINDRAOnlineDataAnalyser</h2>
<h4>For on-line analysis of INDRA data</h4>
<p>
We create a GRU histogram server to which spectra can be added
using method AddHistoToServer(TH1*)
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVINDRAOnlineDataAnalyser::KVINDRAOnlineDataAnalyser()
{
   fSpectraServer = 0;
   fSpectraDB = 0;
   fMessageThread = 0;
}

KVINDRAOnlineDataAnalyser::~KVINDRAOnlineDataAnalyser()
{
   // Destructor
   if (fMessageThread) {
      fMessageThread->Kill();
      TThread::Delete(fMessageThread);
      delete fMessageThread;
      fMessageThread = 0;
   }
}

void KVINDRAOnlineDataAnalyser::preInitAnalysis()
{
   // Before calling user's InitAnalysis we start the thread listening for messages on port 9091
   // The TCP/IP port connected to the server is printed.

   fStart.Set();
   Info("InitAnalysis", "Started analysis on %s", fStart.AsString());
   last_events = 0;
   fEventsRefresh = 100;

   // Test ports for availability
   TestPorts(10000);
   TString path("$(HOME)/.kvonlineanalysisrc");
   gSystem->ExpandPathName(path);
   TEnv env(path.Data());
   env.SetValue(Form("%s.%s.Messages.port", ClassName(), gSystem->HostName()), port);
   env.SaveLevel(kEnvLocal);
//
// fMessageThread = new TThread("AnalysisMess",(void(*)(void*))&ecouteSockets, (void*) this);
// Info("InitAnalysis","Started ecoutesocket on port %d", port);
// fMessageThread->Run();
}

void KVINDRAOnlineDataAnalyser::preInitRun()
{
   Info("InitRun", "Traitement du run %d", GetRunNumber());
   fSpectraDB = ((KVGRUNetClientGanilReader*)fRunFile)->GetSpectraServer();
   TString path("$(HOME)/.kvonlineanalysisrc");
   gSystem->ExpandPathName(path);
   TEnv env(path.Data());
   env.SetValue(Form("%s.%s.SpectraDB.port", ClassName(),
                     gSystem->HostName()), ((KVGRUNetClientGanilReader*)fRunFile)->GetSpectraServerPort());
   env.SaveLevel(kEnvLocal);
   addallhistostoserver(&fHistoList, fHistoList.GetName());
   events = 0;
}

void KVINDRAOnlineDataAnalyser::addallhistostoserver(TCollection* list, const TString& family_pref)
{
   TIter next(list);
   TObject* obj;
   TString family = "";
   while ((obj = next())) {
      if (obj->InheritsFrom("TCollection")) {
         family = family_pref + "/";
         family += obj->GetName();
         addallhistostoserver((TCollection*)obj, family);
      } else if (obj->InheritsFrom("TH1")) {
         if (fSpectraDB) {
            Info("KVINDRAOnlineDataAnalyser", "Adding histo %s to family %s", obj->GetName(), family_pref.Data());
            fSpectraDB->AddSpectrum((TH1*)obj, family_pref.Data());
         }
      }
   }
}

void KVINDRAOnlineDataAnalyser::postAnalysis()
{
   events++;
   if ((events - last_events) >= fEventsRefresh) PrintControlRate();
}

void KVINDRAOnlineDataAnalyser::PrintControlRate()
{
   TDatime nowtime;
   int time_passed = nowtime.Convert() - fStart.Convert();
   if (time_passed < 1.0) {
      fEventsRefresh *= 2;
      return;
   }
   // check time - we aim for a 5 second refresh
   fEventsRefresh *= 5. / time_passed;
   // minimum 5 events
   fEventsRefresh = TMath::Max(fEventsRefresh, 5);
   Float_t acq_rate = (1.0 * (events - last_events)) / (1.0 * time_passed);
   printf("Events read = %d  <=>  Control rate = %f /sec.\n", events, acq_rate);
   last_events = events;

   fStart.Set();

   if (fDumpEvents) fRunFile->GetFiredDataParameters()->ls();
}

void KVINDRAOnlineDataAnalyser::ProcessRun()
{
   // Perform treatment of a given run

   //Open data file
   fRunFile = (KVRawDataReader*)gDataSet->OpenRunfile(GetDataType().Data(), fRunNumber);
   if (fRunFile->GetStatus() == KVGRUNetClientGanilReader::kSTATUS_NOHOST) {
      // cannot connect to ACQ host
      Error("ProcessRun", "Cannot connect to acquisition host machine. Check KVGRUNetClientGanilReader.AcqHostName");
      return;
   }
   //warning! real number of run may be different from that deduced from file name
   //we get the real run number from gMultiDetArray and use it to name any new files
   fRunNumber = gMultiDetArray->GetCurrentRunNumber();

   fEventNumber = 0; //event number

   cout << endl << "Reading ONLINE events from RUN# " << fRunNumber << endl;

   cout << "Starting analysis of data on : ";
   TDatime now;
   cout <<  now.AsString() << endl << endl;

   preInitRun();
   //call user's beginning of run
   InitRun();
   postInitRun();

   fDetEv = new KVDetectorEvent;

   //loop over events in file
   fGoEventLoop = kTRUE;
   fDumpEvents = kFALSE;
   while (fGoEventLoop) {
      Bool_t gotevent = fRunFile->GetNextEvent();

      if (gotevent) {
         fEventNumber++;

         //reconstruct hit groups
         KVSeqCollection* fired = fRunFile->GetFiredDataParameters();
         gMultiDetArray->GetDetectorEvent(fDetEv, fired);

         preAnalysis();
         //call user's analysis. stop if returns kFALSE.
         if (!Analysis()) break;
         postAnalysis();

         fDetEv->Clear();

         if (!((fEventNumber) % 10000)) cout << " ++++ " << fEventNumber << " events read ++++ " << endl;
      } else {
         // got no event - why ?
         Int_t status = fRunFile->GetStatus();
         switch (status) {
            case KVGRUNetClientGanilReader::kSTATUS_NOBUFF:
               Warning("ProcessRun", "Got no buffer from ACQHOST. Sleeping for 2 seconds.");
               gSystem->Sleep(2000);
               break;

            case KVGRUNetClientGanilReader::kSTATUS_NOEVENT:
               Warning("ProcessRun", "Got no event from ACQHOST. Sleeping for 2 seconds.");
               gSystem->Sleep(2000);
               break;

            case KVGRUNetClientGanilReader::kSTATUS_NOACQ:
               Warning("ProcessRun", "Acquisition is stopped. Sleeping for 5 seconds.");
               gSystem->Sleep(5000);
               break;
         }
      }
   }

   delete fDetEv;

   cout << "Ending analysis of run " << fRunNumber << " on : ";
   TDatime now2;
   cout <<  now2.AsString() << endl << endl;
   cout << endl << "Finished reading " << fEventNumber << " events" << endl << endl;

   preEndRun();
   //call user's end of run function
   EndRun();
   postEndRun();

   if (fMessageThread) {
      fMessageThread->Kill();
      TThread::Delete(fMessageThread);
      delete fMessageThread;
      fMessageThread = 0;
   }
}

void KVINDRAOnlineDataAnalyser::ecouteSockets(void* arg)
{
   // static method, lance dans un thread
   // listen to the sockets

   TThread::Printf("<KVINDRAOnlineDataAnalyser::ecouteSockets> : beginning thread");
   KVINDRAOnlineDataAnalyser* ANA = (KVINDRAOnlineDataAnalyser*)arg;
   TServerSocket* fServer = new TServerSocket(ANA->port, kTRUE);

   if (!fServer->IsValid()) TThread::Printf("<KVINDRAOnlineDataAnalyser::ecouteSockets> : Port %d not valid", ANA->port);
   TMonitor* fMonitor = new TMonitor;
   fMonitor->Add(fServer);

   while (1) {

      TSocket* ss = fMonitor->Select();
      if (ss <= (TSocket*)0) continue;

      if (ss->IsA() == TServerSocket::Class()) {
         // new connection ?
         TSocket* s0 = fServer->Accept();
         if (s0) {
            TThread::Printf("<KVINDRAOnlineDataAnalyser::ecouteSockets> : Client found at %s",
                            s0->GetInetAddress().GetHostName());
            fMonitor->Add(s0);
         }
         continue;
      }
      if (ss->IsA() != TSocket::Class()) continue;

      TMessage* mess;
      if (ss->Recv(mess) <= 0) {
         // socket has been closed (?)
         TThread::Printf("<KVINDRAOnlineDataAnalyser::ecouteSockets> : connection from %s has been closed", ss->GetInetAddress().GetHostName());
         fMonitor->Remove(ss);
         delete ss;
         continue;
      }

      if (mess->What() == kMESS_STRING) {

         // deal with commands
         char str[256];
         mess->ReadString(str, 256);
         TThread::Printf("<KVINDRAOnlineDataAnalyser::ecouteSockets> : Received: %s", str);
         TString command(str);
         //command.ToUpper();
         ANA->HandleCommands(command, ss);

      }/* else if (mess->What() == kMESS_OBJECT) {
         TThread::Printf("<KVINDRAOnlineDataAnalyser::ecouteSockets> : Received UN P***IN d'OBJET!!!");
      } else {
         TThread::Printf("<KVINDRAOnlineDataAnalyser::ecouteSockets> : *** Unexpected message ***");
      }*/

      delete mess;
   }
}

void KVINDRAOnlineDataAnalyser::HandleCommands(TString& ordre, TSocket* theSocket)
{
   // Gestion des ordres
   // Returns kTRUE if command was understood & executed
   // Returns kFALSE if command unknown
   //
   // STOP   will stop the analysis (calls EndRun() and EndAnalysis())
   // CLEAR  RAZ de tous les spectres
   //
   // DUMP=yes  start dumping fired data parameters
   // DUMP=no   stop dumping fired data parameters
   //
   // SAVE=toto.root          will save all histograms in '${ANALYSIS_RESULTS}/toto.root' (does not stop analysis)
   // SAVE=/home/toto.root    will save all histograms in '/home/toto.root' (does not stop analysis)
   // SAVE                    will save all histograms in '${ANALYSIS_RESULTS}/[class name]_Run[run number]_[date].root' (does not stop analysis)

   KVString com(ordre.Data());
   com.Begin("=");
   ordre = com.Next();
   ordre.ToUpper();
   if (ordre == "STOP") {
      // arrete l'analyse, en appelant EndRun() et EndAnalysis()
      fGoEventLoop = kFALSE;
      theSocket->Send("ok");
      return;
   } else if (ordre == "SAVE") {
      TString tmp_fil = com.Next();
      TString fil;
      bool send_name = kFALSE;
      if (tmp_fil.Contains("/")) fil = tmp_fil;
      else if (tmp_fil == "") {
         TDatime save_time;
         save_time.Set();
         TString date = save_time.AsString();
         date.ReplaceAll(" ", "_");
         fil = Form("${ANALYSIS_RESULTS}/%s_Run%d_%s.root", GetName(), GetRunNumber(), date.Data());
         send_name = kTRUE;
      } else fil = Form("${ANALYSIS_RESULTS}/%s", tmp_fil.Data());
      SaveSpectra(fil.Data());
      if (send_name) theSocket->Send(fil.Data());
      else theSocket->Send("ok");
      return;
   } else if (ordre == "CLEAR") {
      ClearAllHistos();
      theSocket->Send("ok");
      return;
   } else if (ordre == "DUMP") {
      TString tmp_fil = com.Next();
      tmp_fil.ToLower();
      if (tmp_fil == "yes")fDumpEvents = kTRUE;
      else if (tmp_fil == "no")fDumpEvents = kFALSE;
      else {
         theSocket->Send("unknown command");
         return;
      }
      theSocket->Send("ok");
      return;
   }
   theSocket->Send("unknown command");
}
Int_t KVINDRAOnlineDataAnalyser::TestPorts(Int_t p)
{
   // Test ports for availability. Start from 'port' and go up to port+2000 at most.
   // Returns -1 if no ports available.

   GNetClientRoot testnet((char*) "localhost");
   Int_t ret;
   ret = p;

   for (int i = 0; i < 2000; i++) {
      ret = testnet.TestPortFree(p, (char*) "localhost");
      if (ret > 0)
         break;
      if ((ret <= 0))
         p++;
   }

   return ret;

}
