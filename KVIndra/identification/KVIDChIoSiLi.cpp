//Created by KVClassFactory on Mon Jan 16 13:55:08 2012
//Author: bonnet

#include "KVIDChIoSiLi.h"

ClassImp(KVIDChIoSiLi)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVIDChIoSiLi</h2>
<h4>Identification in ChIo-SiLi matrices of INDRA</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVIDChIoSiLi::KVIDChIoSiLi()
{
   // Default constructor
   fECode = kECode1;
}

//________________________________________________________________

KVIDChIoSiLi::KVIDChIoSiLi(const KVIDChIoSiLi& obj)
{
   // Copy constructor
   // This ctor is used to make a copy of an existing object (for example
   // when a method returns an object), and it is always a good idea to
   // implement it.
   // If your class allocates memory in its constructor(s) then it is ESSENTIAL :-)

   obj.Copy(*this);
}

KVIDChIoSiLi::~KVIDChIoSiLi()
{
   // Destructor
}

//________________________________________________________________

void KVIDChIoSiLi::Copy(TObject& obj) const
{
   // This method copies the current state of 'this' object into 'obj'
   // You should add here any member variables, for example:
   //    (supposing a member variable KVIDChIoSiLi::fToto)
   //    CastedObj.fToto = fToto;
   // or
   //    CastedObj.SetToto( GetToto() );

   KVIDChIoSi::Copy(obj);
   //KVIDChIoSiLi& CastedObj = (KVIDChIoSiLi&)obj;
}

