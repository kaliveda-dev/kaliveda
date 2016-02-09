/*
$Id: KVClassFactory.cpp,v 1.19 2009/01/21 08:04:20 franklan Exp $
$Revision: 1.19 $
$Date: 2009/01/21 08:04:20 $
*/

//Created by KVClassFactory on Wed Jun 10 16:09:12 2009
//Author: Abdelouahao Chbihi

#include "KVCsIVamos.h"

ClassImp(KVCsIVamos)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVCsIVamos</h2>
<h4>KVCsI for Vamos</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVCsIVamos::KVCsIVamos(Float_t thick): KVCsI(thick)
{
   // Default constructor
}
KVCsIVamos::KVCsIVamos()
{
   // Default constructor
}

KVCsIVamos::~KVCsIVamos()
{
   // Destructor
}

const Char_t* KVCsIVamos::GetName() const
{
   // don t give any name, adapted for detectors of Vamos FP
   return TNamed::GetName();
}

void KVCsIVamos::SetACQParams()
{
   // Add an acquisition parameter to this detector

   if (!fACQParams) {
      fACQParams = new KVList();
   }
   // creer parametre d'acquisition avec meme nom que le detecteur
   KVACQParam* par = new KVACQParam(GetName());
   par->SetDetector(this);
   par->SetType("E");
   fACQParams->Add(par);
   fPar = par;
}
