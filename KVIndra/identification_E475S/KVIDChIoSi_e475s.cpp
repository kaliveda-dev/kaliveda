/*
$Id: KVIDChIoSi_e475s.cpp,v 1.1 2008/02/19 15:01:11 ebonnet Exp $
$Revision: 1.1 $
$Date: 2008/02/19 15:01:11 $
*/

//Created by KVClassFactory on Tue Feb 19 15:56:56 2008
//Author: eric bonnet,,,

#include "KVIDChIoSi_e475s.h"

ClassImp(KVIDChIoSi_e475s)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVIDChIoSi_e475s</h2>
<h4>derivation of KVIDChIoSi class for E475s experiment</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVIDChIoSi_e475s::KVIDChIoSi_e475s()
{
   // Default constructor
}

KVIDChIoSi_e475s::~KVIDChIoSi_e475s()
{
   // Destructor
}

Double_t KVIDChIoSi_e475s::GetIDMapX(Option_t*)
{
   //X coordinate for ChIo-Si identification is silicon pedestal-corrected 'PG' channel
   //return GetDetector(2)->GetACQData("PG") - GetDetector(2)->GetPedestal("PG");
   return GetDetector(2)->GetEnergy();
}

Double_t KVIDChIoSi_e475s::GetIDMapY(Option_t*)
{
   //Y coordinate for ChIo-Si identification is chio pedestal-corrected 'PG' channel
   //return  GetDetector(1)->GetACQData("PG") - GetDetector(1)->GetPedestal("PG");
   return GetDetector(1)->GetEnergy();
}
