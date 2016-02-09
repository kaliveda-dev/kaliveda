//Created by KVClassFactory on Tue Sep  8 16:14:25 2015
//Author: ,,,

#include "KVFAZIAIDSiCsI.h"

ClassImp(KVFAZIAIDSiCsI)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVFAZIAIDSiCsI</h2>
<h4>id telescope to manage FAZIA Si-CsI identification</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVFAZIAIDSiCsI::KVFAZIAIDSiCsI()
{
   // Default constructor
   SetType("Si-CsI");
}

KVFAZIAIDSiCsI::~KVFAZIAIDSiCsI()
{
   // Destructor
}

//________________________________________________________________

void KVFAZIAIDSiCsI::Copy(TObject& obj) const
{
   // This method copies the current state of 'this' object into 'obj'
   // You should add here any member variables, for example:
   //    (supposing a member variable KVFAZIAIDSiCsI::fToto)
   //    CastedObj.fToto = fToto;
   // or
   //    CastedObj.SetToto( GetToto() );

   KVFAZIAIDTelescope::Copy(obj);
   //KVFAZIAIDSiCsI& CastedObj = (KVFAZIAIDSiCsI&)obj;
}

