//Created by KVClassFactory on Tue Jan 13 15:06:39 2015
//Author: ,,,

#include "KVPSAResult.h"

ClassImp(KVPSAResult)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVPSAResult</h2>
<h4>class to store PSA parameters and results</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVPSAResult::KVPSAResult()
{
   // Default constructor
}

//________________________________________________________________

KVPSAResult::KVPSAResult(const Char_t* name, const Char_t* title) : KVNameValueList(name, title)
{
   // Write your code here
}

KVPSAResult::~KVPSAResult()
{
   // Destructor
}

//________________________________________________________________

void KVPSAResult::Copy(TObject& obj) const
{
   // This method copies the current state of 'this' object into 'obj'
   // You should add here any member variables, for example:
   //    (supposing a member variable KVPSAResult::fToto)
   //    CastedObj.fToto = fToto;
   // or
   //    CastedObj.SetToto( GetToto() );

   KVNameValueList::Copy(obj);
   //KVPSAResult& CastedObj = (KVPSAResult&)obj;
}

