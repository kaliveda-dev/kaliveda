//Created by KVClassFactory on Fri Nov  2 12:13:31 2012
//Author: Guilain ADEMARD

#include "KVIDChIoSi75_e494s.h"

#include <KVINDRADetector.h>

ClassImp(KVIDChIoSi75_e494s)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVIDChIoSi75_e494s</h2>
<h4>E503/E494S experiment INDRA identification using ChIo-Si75 matrices</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVIDChIoSi75_e494s::KVIDChIoSi75_e494s()
{
   // Default constructor
}

//________________________________________________________________

KVIDChIoSi75_e494s::~KVIDChIoSi75_e494s()
{
   // Destructor
}

//________________________________________________________________

Double_t KVIDChIoSi75_e494s::GetIDMapX(Option_t*)
{
   // This method gives the X-coordinate in a 2D identification map
   // associated with the ChIo-Si75 identification telescope.
   // The X-coordinate is the Si75 current low gain coder data minus the
   // low gain pedestal correction (see KVACQParam::GetDeltaPedestal()).
   // If the high gain coder data is less than 3900 the the low gain value
   // is calculated from the current high gain coder data minus the high
   // gain pedestal correction (see KVINDRADetector::GetPGfromGG()).

   KVINDRADetector* si = (KVINDRADetector*)fsi;

   if ((-0.5 <= si->GetGG()) && (si->GetGG() <= 3900.5))
      return si->GetPGfromGG(si->GetGG() - si->GetACQParam("GG")->GetDeltaPedestal());

   return si->GetPG() - si->GetACQParam("PG")->GetDeltaPedestal();
}
//________________________________________________________________

Double_t KVIDChIoSi75_e494s::GetIDMapY(Option_t*)
{
   // This method gives the Y-coordinate in a 2D identification map
   // associated with the ChIo-Si75 identification telescope.
   // The Y-coordinate is the ChIo current low gain coder data.

   return ((KVINDRADetector*)fchio)->GetPG();
}
