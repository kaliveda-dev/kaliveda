/*
$Id: GTGanilDataVAMOS.cpp,v 1.4 2007/11/21 11:23:00 franklan Exp $
$Revision: 1.4 $
$Date: 2007/11/21 11:23:00 $
*/

//Created by KVClassFactory on Thu May 31 15:04:01 2007
//Author: John Frankland

// ganil_tape lib headers
#include <ERR_GAN.H>
#include <gan_tape_erreur.h>
#include <gan_tape_general.h>
#include <gan_tape_param.h>
#include <gan_tape_test.h>
#include <gan_tape_alloc.h>
#include <gan_tape_mount.h>
#include <gan_tape_file.h>
#include <acq_mt_fct_ganil.h>
#include <acq_ebyedat_get_next_event.h>
#include <gan_tape_get_parametres.h>
extern "C"
{
   typedef struct SCALE {
      UNSINT32 Length; /* Nb bytes utils suivant ce mot */
      UNSINT32 Nb_channel;
      UNSINT32 Acq_status;
      UNSINT32 Reserve[2];
      union JBUS_SCALE {
         scale_struct UnScale[NB_MAX_CHANNEL];
         UNSINT16 Info_jbus  [NB_MAX_JBUS];
      } jbus_scale;
   } scale;
}

#include "GTGanilDataVAMOS.h"

//ClassImp(GTGanilDataVAMOS)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>GTGanilDataVAMOS</h2>
<h4>Reads and formats raw data from INDRA-VAMOS experiments</h4>
<p>
Based on classes from the VAMOS analysis package kindly contributed by Maurycy Rejmund (GANIL).
</p>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

bool AutoswBuf, Swbufon;
Int_t BufSize;

GTGanilDataVAMOS::GTGanilDataVAMOS(): GTGanilData()
{
   //Default constructor
   Par = new Parameters;
}

GTGanilDataVAMOS::GTGanilDataVAMOS(const TString filename): GTGanilData(filename)
{
   //Open file "filename" for reading
   Par = new Parameters;
}

GTGanilDataVAMOS::~GTGanilDataVAMOS()
{
   //Destructor
   delete Par;
}

void GTGanilDataVAMOS::ReadParameters(void)
{
   // PRIVATE
   // Read the data parameters from the current buffer, put it in the
   // parameter list.
   //This method includes the initialisation of the VAMOS parameter
   //list held in Parameters* Par

   do {
      ReadBuffer();
      if (strcmp(fHeader, PARAM_Id) == 0) {

         //fill normal parameter list
         fDataArraySize = fDataParameters->Fill(fBuffer->les_donnees.cas.Buf_param);
         //fill VAMOS parameter list
         Par->Fill(fBuffer->les_donnees.cas.Buf_param);

      }
   } while (strcmp(fHeader, PARAM_Id) == 0);

   fDataArray = new UShort_t[fDataArraySize + 1]; // Data buffer is allocated
   for (Int_t i = 1; i <= fDataArraySize; i++) {
      fDataArray[i] = (Short_t) - 1;
   }

}

//______________________________________________________________________________

bool GTGanilDataVAMOS::EventUnravelling(CTRL_EVENT* pCtrlEvent)
{
   // PRIVATE
   // If mode is variable length event, we have to reconstruct the Data buffer
   // from the given event.
   // WARNING: temporary the default: we dont check that it's really the case
   // Before reading event, all parameters have their value set to -1 (65535 - fDataArray is UShort_t)
   // Parameters which are not fired in the event will have value -1 (65535 - cast back to Short_t for real value)
   //
   //VAMOS parameters are treated here:
   //    clear all parameters
   //    check no parameters occur twice in same event (if so, the VAMOS treatment is skipped)

   Short_t* brutData     = &(pCtrlEvent->ct_par);
   Int_t eventLength = pCtrlEvent->ct_len;

   for (Int_t i = 1; i <= fDataArraySize; i++) {
      fDataArray[i] = (Short_t) - 1;
   }
   Par->Clear();

   bool fOK;

   fOK = true;

   //VAMOS double parameter check
   for (Int_t k = 0; k < eventLength - 6 - 2; k += 2) {
      for (Int_t l = k + 2; l < eventLength - 6; l += 2) {
         if (brutData[k] == brutData[l]) {
            fOK = false;
         }
      }
   }

   for (Int_t i = 0; i < eventLength; i += 2) {
      //normal GTGanilData/INDRA treatment
      if (brutData[i] <= fDataArraySize && brutData[i] >= 1) {
         fDataArray[brutData[i]] = brutData[i + 1];
      }
      //VAMOS treatment
      if ((i < (eventLength - 6)) && fOK) {
         if (brutData[i] > 0 && brutData[i + 1] > 0) {
            Par->GetData(brutData + i);
         }
      }
   }

   return (true);
}

//_______________________________________________________________________

void GTGanilDataVAMOS::SetUserTree(TTree* theTree)
{
   //Creates VAMOS vectorised data branches in the (existing) TTree
   Par->Set(theTree);
}
