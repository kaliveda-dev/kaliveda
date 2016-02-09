/***************************************************************************/
//                        GTGanilData.cpp  -  Main Header to ROOTGAnilTape
//                             -------------------
//    begin                : Thu Jun 14 2001
//    copyright            : (C) 2001 by Garp
//    email                : patois@ganil.fr


#include "Riostream.h"
#include <GEN_TYPE.H>
#include <gan_acq_buf.h>

// Problem (for me?) in gan_acq_buf :
// What the use of defining struct inside other structs ?
// Then, theyr definition remain local and I have to do it again here:
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
//}

#include <TMath.h>
#include "GTScalers.h"
#include "GTOneScaler.h"

using namespace std;

ClassImp(GTScalers)

//////////////////////////////////////////////////////////////////////////
//
// Part of ROOTGanilTape
//
// GTScalers
//
// Scaler class for scaler events.
//
//////////////////////////////////////////////////////////////////////////

//______________________________________________________________________________
GTScalers::GTScalers(void)
   : fScalerArray(100)
{
   fNbChannel = -1; // Initialisation Marker
   fScalerArray.SetOwner(kTRUE);
}

//______________________________________________________________________________
GTScalers::~GTScalers(void)
{
}


//______________________________________________________________________________
void GTScalers::Fill(void* echelle)
{

   scale* s = (scale*)echelle;
//  cout <<s->Length<<endl;
//  cout <<s->Nb_channel<<endl; // This is number of actual scalers
//  cout <<s->Acq_status<<endl;

   fNbChannel   = s->Nb_channel;
   fScalerArray.Expand(fNbChannel);

   for (Int_t i = 0; i < fNbChannel; i++) {
      GTOneScaler* scal = new GTOneScaler;
      scal->Set(&(s->jbus_scale.UnScale[i]));
      fScalerArray.AddAt(scal, i);
   }
}

//______________________________________________________________________________
void GTScalers::DumpScalers(void)
{
   // Dump scalers value on cout

   cout << "Dumping " << fNbChannel << " scalers" << endl;
   for (Int_t i = 0; i < fNbChannel; i++) {
      fScalerArray[i]->ls();
   }
}

//______________________________________________________________________________
const GTOneScaler* GTScalers::GetScalerPtr(Int_t index) const
{
   // Return a constant pointer to the Scaler Index
   return ((GTOneScaler*)fScalerArray[index]);
}
