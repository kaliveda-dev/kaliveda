//Created by KVClassFactory on Tue Nov 30 16:36:28 2010
//Author: eindra

#include "KVGRUNetClientGanilReader.h"
#include "GNetClientGanil.h"
#include "GEvent.h"
#include "GDataParameters.h"
#include "GAcq.h"
#include "GSpectra.h"
#include "KVACQParam.h"
#include "KVDataSet.h"
#include "KVMultiDetArray.h"
#include "TSystem.h"

ClassImp(KVGRUNetClientGanilReader)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVGRUNetClientGanilReader</h2>
<h4>Read GANIL acquisition events over network connection</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

#define ACQ_OK  0x08018001

KVGRUNetClientGanilReader::KVGRUNetClientGanilReader()
{
   // Open connection to acquisition machine
   // hostname taken from
   //
   // KVGRUNetClientGanilReader.AcqHostName:   ganp737
   //
   // If the dataset name is not the same as the GANIL acquisition experiment name,
   // it is ESSENTIAL to give the correct name, as for exemple:
   //
   // INDRA_e613.KVGRUNetClientGanilReader.ExpName:    e613
   //
   // If hostname not defined or introuvable, GetStatus() returns KVGRUNetClientGanilReader::kSTATUS_NOHOST
   //
   // Default timeout = 1 second

   fExtParams = 0;
   fParameters = new KVHashList;
   fParameters->SetCleanup(kTRUE);
   fFired = new KVHashList;
   fTimeout = 1;
   fGRUAcq = 0;

   GNetClientGanil* client =
      new GNetClientGanil(gEnv->GetValue("KVGRUNetClientGanilReader.AcqHostName", "xxxx"));
   client->Open();
   // check connection OK
   if (!client->GetIsOpen()) {
      fStatus = kSTATUS_NOHOST;
      return;
   }

   fGRUAcq = new GAcq(client);
   TString expname = gDataSet->GetDataSetEnv("KVGRUNetClientGanilReader.ExpName", gDataSet->GetName());
   expname.ToLower();
   // read (and print) status of acquisition, including run number
   fGRUAcq->SetConfGanilAcq(kTRUE);
   //fGRUAcq->GetRunStatus(kTRUE);
   fGRUAcq->Infos();
   fGRUAcq->EventInit((char*)expname.Data());
   //fGRUAcq->SetSpectraMode(0);
   //fGRUAcq->SetUserMode(0);
   // we have to test ports for availability
   fSpecServPort = TestPorts(9090);
   fGRUAcq->SetNetMode(1, fSpecServPort);

   Int_t run = fGRUAcq->GetRunNumber();

   fReadFirstBuffer = kTRUE;

   if (!gMultiDetArray) gDataSet->BuildMultiDetector();
   if (run) gMultiDetArray->SetParameters(run);

   ConnectRawDataParameters();

   fStatus = kSTATUS_OK;
}

void KVGRUNetClientGanilReader::ConnectRawDataParameters()
{
   GEvent* the_event = fGRUAcq->GetEvent();
   GDataParameters* the_parameters = fGRUAcq->GetDataParameters();
   Int_t nbparams = the_parameters->GetNbParameters();
   TString par_name;
   for (int i = 0; i < nbparams; i++) {
      par_name = the_parameters->GetParName(i);
      KVACQParam* par = CheckACQParam(par_name.Data());
      the_event->Connect(par_name.Data(), par->ConnectData());
      par->SetNumber(the_parameters->GetIndex(par_name.Data()));
      par->SetNbBits(the_parameters->GetNbits(par_name.Data()));
      fParameters->Add(par);
   }
}

KVACQParam* KVGRUNetClientGanilReader::CheckACQParam(const Char_t* par_name)
{
   KVACQParam* par;
   if (!gMultiDetArray || !(par = gMultiDetArray->GetACQParam(par_name))) {
      //create new unknown parameter
      par = new KVACQParam;
      par->SetName(par_name);
      if (!fExtParams) {
         fExtParams = new KVHashList;
         fExtParams->SetOwner(kTRUE);
      }
      fExtParams->Add(par);
   }
   return par;
}

KVGRUNetClientGanilReader::~KVGRUNetClientGanilReader()
{
   // Destructor
   fParameters->Clear();
   delete fParameters;
   if (fExtParams) {
      fExtParams->Delete("slow");
      delete fExtParams;
   }
   delete fFired;
   if (fGRUAcq) delete fGRUAcq;
}

Bool_t KVGRUNetClientGanilReader::GetNextEvent()
{
   // Get next event from buffer
   // Returns kTRUE if an event is read.
   // Returns kFALSE if no acquisition running (check status)
   // Returns kFALSE if no buffer/event read after fTimeout seconds (check status)

   // check status of acquisition
   ((GNetClientGanil*)fGRUAcq->GetDeviceIn())->GetStatusBuffer(0);
   if (!((GNetClientGanil*)fGRUAcq->GetDeviceIn())->GetAcquisitionOn()) {
      fStatus = kSTATUS_NOACQ;
      return kFALSE;
   }
   TDatime start;
   TDatime now;
   fStatus = kSTATUS_OK;
   if (fReadFirstBuffer) {
      // first time we call, we need to read a buffer
      fReadFirstBuffer = kFALSE;
      fGRUAcq->GetDeviceIn()->ReadBuffer();
      fGRUAcq->GetEvent()->RazEvent();
      while (!fGRUAcq->GetDeviceIn()->GetCurrentBuffer()->IsAeventBuffer()) {
         fGRUAcq->GetDeviceIn()->ReadBuffer();
         now.Set();
         if ((now.Convert() - start.Convert()) >= fTimeout) {
            fStatus = kSTATUS_NOBUFF;
            return kFALSE;
         }
      }
   }
   int Status = fGRUAcq->GetEvent()->NextEvent(fGRUAcq->GetDeviceIn()->GetCurrentBuffer());
   if (Status != ACQ_OK) {
      // we have read all events in buffer. get a new buffer.
      fGRUAcq->GetDeviceIn()->ReadBuffer();
      fGRUAcq->GetEvent()->RazEvent();
      start.Set();
      now.Set();
      while (!fGRUAcq->GetDeviceIn()->GetCurrentBuffer()->IsAeventBuffer()) {
         fGRUAcq->GetDeviceIn()->ReadBuffer();
         now.Set();
         if ((now.Convert() - start.Convert()) >= fTimeout) {
            fStatus = kSTATUS_NOBUFF;
            return kFALSE;
         }
      }
      start.Set();
      now.Set();
      while (Status != ACQ_OK) {
         Status = fGRUAcq->GetEvent()->NextEvent(fGRUAcq->GetDeviceIn()->GetCurrentBuffer());
         now.Set();
         if ((now.Convert() - start.Convert()) >= fTimeout) {
            fStatus = kSTATUS_NOEVENT;
            return kFALSE;
         }
      }
   }
   FillFiredParameterList();
   return kTRUE;
}

KVGRUNetClientGanilReader* KVGRUNetClientGanilReader::Open(const Char_t*, Option_t*)
{
   return new KVGRUNetClientGanilReader;
}

void KVGRUNetClientGanilReader::FillFiredParameterList()
{
   // clears and then fills list fFired with all fired acquisition parameters in event
   fFired->Clear();
   TIter next(fParameters);
   KVACQParam* par;
   while ((par = (KVACQParam*)next())) {
      if (par->Fired()) fFired->Add(par);
   }
}

GSpectra* KVGRUNetClientGanilReader::GetSpectraServer() const
{
   return fGRUAcq->GetSpectra();
}
