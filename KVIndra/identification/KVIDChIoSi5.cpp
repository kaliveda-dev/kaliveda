/*
$Id: KVIDChIoSi5.cpp,v 1.2 2009/04/06 15:20:28 franklan Exp $
$Revision: 1.2 $
$Date: 2009/04/06 15:20:28 $
*/

//Created by KVClassFactory on Mon Dec 10 12:58:26 2007
//Author: franklan

#include "KVIDChIoSi5.h"

ClassImp(KVIDChIoSi5)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVIDChIoSi5</h2>
<h4>INDRA 5th campaign ChIo-Si identification</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVIDChIoSi5::KVIDChIoSi5()
{
   // Default constructor
}

KVIDChIoSi5::~KVIDChIoSi5()
{
   // Destructor
}

//________________________________________________________________________________________//

Double_t KVIDChIoSi5::GetIDMapX(Option_t*)
{
   //X coordinate for 5th campaign ChIo-Si identification is KVSilicon::GetEnergy
   return (fsi->Fired() ? fsi->GetEnergy() : -999);
}

Double_t KVIDChIoSi5::GetIDMapY(Option_t*)
{
   //Y coordinate for 5th campaign ChIo-Si identification is KVChIo::GetEnergy
   return (fchio->Fired() ? fchio->GetEnergy() : -999);
}

