/*
$Id: KVClassFactory.cpp,v 1.19 2009/01/21 08:04:20 franklan Exp $
$Revision: 1.19 $
$Date: 2009/01/21 08:04:20 $
*/

//Created by KVClassFactory on Wed Jun 10 16:07:32 2009
//Author: Abdelouahao Chbihi

#include "KVSiliconVamos.h"

ClassImp(KVSiliconVamos)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVSiliconVamos</h2>
<h4>KVSilicon for Vamos</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVSiliconVamos::KVSiliconVamos(Float_t thick): KVSilicon(thick)
{
   // Default constructor
}
KVSiliconVamos::KVSiliconVamos()
{
   // Default constructor
}

KVSiliconVamos::~KVSiliconVamos()
{
   // Destructor
}
const Char_t* KVSiliconVamos::GetName() const
{
   // don t give any name, adapted for detectors of Vamos FP
   return TNamed::GetName();
}

void KVSiliconVamos::SetACQParams()
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
}
