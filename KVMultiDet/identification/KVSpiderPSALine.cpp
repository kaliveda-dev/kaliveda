//Created by KVClassFactory on Fri Nov 30 09:01:11 2012
//Author: dgruyer

#include "KVSpiderPSALine.h"

ClassImp(KVSpiderPSALine)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVSpiderPSALine</h2>
<h4>KVSpiderLine specialized for PSA matrix</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVSpiderPSALine::KVSpiderPSALine()
{
   // Default constructor
}

//________________________________________________________________

KVSpiderPSALine::KVSpiderPSALine(const KVSpiderPSALine& obj)  : KVSpiderLine()
{
   // Copy constructor
   // This ctor is used to make a copy of an existing object (for example
   // when a method returns an object), and it is always a good idea to
   // implement it.
   // If your class allocates memory in its constructor(s) then it is ESSENTIAL :-)

   obj.Copy(*this);
}

KVSpiderPSALine::~KVSpiderPSALine()
{
   // Destructor
}

//________________________________________________________________

void KVSpiderPSALine::Copy(TObject& obj) const
{
   // This method copies the current state of 'this' object into 'obj'
   // You should add here any member variables, for example:
   //    (supposing a member variable KVSpiderPSALine::fToto)
   //    CastedObj.fToto = fToto;
   // or
   //    CastedObj.SetToto( GetToto() );

   KVSpiderLine::Copy(obj);
   //KVSpiderPSALine& CastedObj = (KVSpiderPSALine&)obj;
}

