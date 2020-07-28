//Created by KVClassFactory on Mon Apr  1 10:16:46 2019
//Author: eindra

#include "KVOnlineReconDataAnalyser.h"
#include "KVMultiDetArray.h"
#include "KVDataSet.h"
#include "KVReconEventSelector.h"

#ifdef WITH_DATAFLOW
#include "dataflowports.h"

#include <KVZMQMessage.h>
#endif

ClassImp(KVOnlineReconDataAnalyser)


KVOnlineReconDataAnalyser::KVOnlineReconDataAnalyser()
   : KVReconDataAnalyser()
{
   // Default constructor
   SetStatusUpdateInterval(2.5);
}

//____________________________________________________________________________//

KVOnlineReconDataAnalyser::~KVOnlineReconDataAnalyser()
{
   // Destructor
}

//____________________________________________________________________________//

void KVOnlineReconDataAnalyser::SubmitTask()
{
   //Run the online analysis

   //make the chosen dataset the active dataset ( = gDataSet; note this also opens database
   //and positions gDataBase & gExpDB).
   GetDataSet()->cd();

   TString option = GetUserClassOptions();

   fSelector = dynamic_cast<KVReconEventSelector*>(GetInstanceOfUserClass());

   if (!fSelector || !fSelector->InheritsFrom("TSelector")) {
      std::cout << "The selector \"" << GetUserClass() << "\" is not valid." << std::endl;
      std::cout << "Process aborted." << std::endl;
      SafeDelete(fSelector);
      return;
   }

   fSelector->SetOption(GetUserClassOptions());
   fSelector->ParseOptions();

   preInitAnalysis();
   fSelector->InitAnalysis();
   if (!fSelector->IsOptGiven("dataflowhost")) {
      Warning("SubmitTask", "Did you forget to give option 'dataflowhost' in InitAnalysis()?");
   }
   postInitAnalysis();

   preInitRun();
   fSelector->InitRun();
   postInitRun();

   zmq::context_t context(1); // for ZeroMQ communications
   std::string zmq_spy_port = Form("tcp://%s:%d", fSelector->GetOpt("dataflowhost").Data(), PORT_EVENT_PUB);
   zmq::socket_t pub(context, ZMQ_SUB);
   int timeout = 500; //milliseconds
   pub.setsockopt(ZMQ_RCVTIMEO, &timeout, sizeof(int));
   try {
      pub.connect(zmq_spy_port.c_str());
   }
   catch (zmq::error_t& e) {
      Error("SubmitTask", "failed to connect socket: %s", e.what());
   }

   std::cout << "Connected to EventPublisher " << zmq_spy_port << std::endl;
   pub.setsockopt(ZMQ_SUBSCRIBE, "", 0);

   zmq::message_t event;

   int nev = 0;
   int nevt = 0;
   TDatime fTimeStamp;
   Double_t fStartTime = fTimeStamp.Convert();
   fUpdate = false;

   while (1) {

      if (pub.recv(&event)) {

         KVZMQMessage mess_event(event);
         KVReconstructedEvent* event;
         if ((event = mess_event.GetObject<KVReconstructedEvent>())) {
            ++nev;
            ++nevt;
            fSelector->SetEvent(event);
            preAnalysis();
            if (!fSelector->Analysis()) break;
            postAnalysis();
         }
      }
      fTimeStamp.Set();
      Double_t time = fTimeStamp.Convert();
      if ((time - fStartTime) >= GetStatusUpdateInterval()) {
         std::cout << "~" << (int)(nev / (time - fStartTime)) << " events/s. tot = " << nevt << std::endl;
         fStartTime = time;
         nev = 0;
         fUpdate = true;
      }

   }
   fSelector->EndRun();

   fSelector->EndAnalysis();
   delete fSelector;
}

//__________________________________________________________________________________//

void KVOnlineReconDataAnalyser::preInitAnalysis()
{
   if (!gMultiDetArray) KVMultiDetArray::MakeMultiDetector(GetDataSet()->GetName());
}

void KVOnlineReconDataAnalyser::preAnalysis()
{
   // Set minimum (trigger) multiplicity for array

   gMultiDetArray->SetMinimumOKMultiplicity(fSelector->GetEvent());
}

Bool_t KVOnlineReconDataAnalyser::CheckStatusUpdateInterval(Int_t) const
{
   // This method returns true every fStatusUpdateInterval seconds, according to the check performed in SubmitTask
   // You can change this value in your analysis class by calling
   //
   //     gDataAnalyser->SetStatusUpdateInterval(...);
   //
   // with the required value in seconds before the processing loop starts

   if (fUpdate) {
      fUpdate = false;
      return true;
   }
   return false;
}

