//Created by KVClassFactory on Tue Nov 30 16:36:28 2010
//Author: eindra

#include "KVGRUTapeGanilReader.h"
#include "GTape.h"
#include "GEvent.h"
#include "GDataParameters.h"
#include "KVACQParam.h"
#include "KVDataSet.h"
#include "KVMultiDetArray.h"
#include "TSystem.h"

ClassImp(KVGRUTapeGanilReader)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVGRUTapeGanilReader</h2>
<h4>Read GANIL acquisition events from file</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

#define ACQ_OK  0x080180e3

KVGRUTapeGanilReader::KVGRUTapeGanilReader(const Char_t* filename)
{
   fExtParams = 0;
   fParameters = new KVHashList;
   fParameters->SetCleanup(kTRUE);
   fFired = new KVHashList;
   fGRUAcq = 0;

   GTape* client = new GTape(filename);
   client->Open();
   // check connection OK
   if (!client->GetIsOpen()) {
      fStatus = kSTATUS_NOHOST;
      return;
   }

   fGRUAcq = new GAcq(client);
   TString expname = "e651";
   expname.ToLower();
   // read (and print) status of acquisition, including run number
   fGRUAcq->SetConfGanilAcq(kTRUE);
   //fGRUAcq->GetRunStatus(kTRUE);
   fGRUAcq->Infos();
   fGRUAcq->EventInit((char*)expname.Data());
   fGRUAcq->SetSpectraMode(0);
   fGRUAcq->SetUserMode(0);

   Int_t run = fGRUAcq->GetRunNumber();

   fReadFirstBuffer = kTRUE;

   ConnectRawDataParameters();

   fStatus = kSTATUS_OK;
}

void KVGRUTapeGanilReader::ConnectRawDataParameters()
{
   GEvent* the_event = (GEvent*)fGRUAcq->GetEvent();
   DataParameters* the_parameters = fGRUAcq->GetDataParameters();
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

KVACQParam* KVGRUTapeGanilReader::CheckACQParam(const Char_t* par_name)
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

KVGRUTapeGanilReader::~KVGRUTapeGanilReader()
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

Bool_t KVGRUTapeGanilReader::GetNextEvent()
{
   // Get next event from buffer
   // Returns kTRUE if an event is read.
   // Returns kFALSE if no acquisition running (check status)
   // Returns kFALSE if no buffer/event read after fTimeout seconds (check status)

   fStatus = kSTATUS_OK;
   if (fReadFirstBuffer) {
      // first time we call, we need to read a buffer
      fReadFirstBuffer = kFALSE;
      fGRUAcq->GetDeviceIn()->ReadBuffer();
      fGRUAcq->GetEvent()->RazEvent();
      Info("GetNextEvent", "ReadFirstBuffer: looking for event buffer");
      while (!fGRUAcq->GetDeviceIn()->GetCurrentBuffer()->IsAeventBuffer()) {
         fGRUAcq->GetDeviceIn()->ReadBuffer();
      }
      Info("GetNextEvent", "ReadFirstBuffer: got event buffer");
   }
   int Status = fGRUAcq->GetEvent()->NextEvent(fGRUAcq->GetDeviceIn()->GetCurrentBuffer());
   Info("GetNextEvent", "Status = %0#x", Status);
   if (Status != ACQ_OK) {
      // we have read all events in buffer. get a new buffer.
      Info("GetNextEvent", "we have read all events in buffer. get a new buffer.");
      fGRUAcq->GetDeviceIn()->ReadBuffer();
      fGRUAcq->GetEvent()->RazEvent();
      Info("GetNextEvent", "looking for event buffer");
      while (!fGRUAcq->GetDeviceIn()->GetCurrentBuffer()->IsAeventBuffer()) {
         fGRUAcq->GetDeviceIn()->ReadBuffer();
      }
      Info("GetNextEvent", "got buffer, Status = %0#x", Status);
      while (Status != ACQ_OK) {
         Status = fGRUAcq->GetEvent()->NextEvent(fGRUAcq->GetDeviceIn()->GetCurrentBuffer());
         Info("GetNextEvent", "Status = %0#x", Status);
      }
   }
   FillFiredParameterList();
   return kTRUE;
}

KVGRUTapeGanilReader* KVGRUTapeGanilReader::Open(const Char_t* filename, Option_t*)
{
   return new KVGRUTapeGanilReader(filename);
}

void KVGRUTapeGanilReader::FillFiredParameterList()
{
   // clears and then fills list fFired with all fired acquisition parameters in event
   fFired->Clear();
   TIter next(fParameters);
   KVACQParam* par;
   while ((par = (KVACQParam*)next())) {
      if (par->Fired()) fFired->Add(par);
   }
}
