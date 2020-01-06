//Created by KVClassFactory on Thu Sep 20 17:24:51 2012
//Author: Ademard Guilain

#include "KVIDChIoCorrCsI_e494s.h"

ClassImp(KVIDChIoCorrCsI_e494s)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
   <h2>KVIDChIoCorrCsI_e494s</h2>
   <h4>ChIo-CsI identification with grids for E494S</h4>
   <!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVIDChIoCorrCsI_e494s::KVIDChIoCorrCsI_e494s()
{
   // Default constructor
}
//________________________________________________________________

KVIDChIoCorrCsI_e494s::~KVIDChIoCorrCsI_e494s()
{
   // Destructor
}
//________________________________________________________________

Double_t KVIDChIoCorrCsI_e494s::GetIDMapY(Option_t* opt)
{
   // This method gives the Y-coordinate in a 2D identification map
   // associated whith the ChIo-CsI identification telescope.
   // The Y-coordinate is the ionisation chamber's current petit gain coder data minus the petit gain pedestal. If the grand gain coder
   // data is less than 3900 then the petit gain value is calculated
   // from the current grand gain coder data (see KVINDRADetector::GetPGFromGG())
   // We set the pedestal to zero in order to not cut the physics
   // as the pedestal GG seems to be too high.
   // PG-GG conversion has been calculated without ChIo GG pedestal.

   IGNORE_UNUSED(opt);

   fChIo->SetPedestal("GG", 0.);
   if (fChIo->GetGG() < 3900.) return fChIo->GetPGfromGG() - fChIo->GetPedestal("PG");
   return fChIo->GetPG() - fChIo->GetPedestal("PG");
}
