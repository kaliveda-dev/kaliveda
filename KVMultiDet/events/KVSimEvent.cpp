//Created by KVClassFactory on Wed Jun 30 15:10:22 2010
//Author: bonnet

#include "KVSimEvent.h"

ClassImp(KVSimEvent)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVSimEvent</h2>
<h4>Classe dérivée de KVEvent pour la gestion d'événements issus de simulations</h4>
<!-- */
// --> END_HTML
//Cette classe est couplée à KVSimNucleus
////////////////////////////////////////////////////////////////////////////////

//___________________________
KVSimEvent::KVSimEvent(Int_t mult, const char* classname): KVEvent(mult, classname)
{
   // Default constructor
}

//___________________________
KVSimEvent::~KVSimEvent()
{
   //destructeur
}
