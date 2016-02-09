/*
$Id: KVReconIdent_e475s.cpp,v 1.2 2007/11/21 11:22:59 franklan Exp $
$Revision: 1.2 $
$Date: 2007/11/21 11:22:59 $
*/

//Created by KVClassFactory on Tue Nov 13 10:47:51 2007
//Author: Eric Bonnet

#include "KVReconIdent_e475s.h"

ClassImp(KVReconIdent_e475s)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVReconIdent_e475s</h2>
<h4>Identification of E475S events</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVReconIdent_e475s::KVReconIdent_e475s()
{
   //Default constructor
}

KVReconIdent_e475s::~KVReconIdent_e475s()
{
   //Destructor
}

//_____________________________________
Bool_t KVReconIdent_e475s::Analysis(void)
{
   //For each event we:
   //     perform primary event identification and calibration and fill tree
   //Events with zero reconstructed multiplicity are excluded, but no selection is made
   //based on the results of the identification/calibration

   fEventNumber = GetEvent()->GetNumber();
   if (GetEvent()->GetMult() > 0) {
      GetEvent()->IdentifyEvent();
      // traitement specifique a E475S...
      //
      //
      GetEvent()->CalibrateEvent();
      fIdentTree->Fill();
   }
   return kTRUE;
}

