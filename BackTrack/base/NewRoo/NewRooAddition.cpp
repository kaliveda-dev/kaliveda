//Created by KVClassFactory on Wed Sep 16 16:37:51 2015
//Author: fable

#include "NewRooAddition.h"

ClassImp(NewRooAddition)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>NewRooAddition</h2>
<h4> Derived class of RooAddition with addition of enableSilentOffsetting() method</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

//_____________________________________________________________________________
NewRooAddition::NewRooAddition() : RooAddition()
{
   // Default constructor
}


//_____________________________________________________________________________
NewRooAddition::NewRooAddition(const char* name, const char* title, const RooArgList& sumSet, Bool_t takeOwnership) : RooAddition(name, title, sumSet, takeOwnership)
{
  // Constructor with a single set of RooAbsReals. The value of the function will be
  // the sum of the values in sumSet. If takeOwnership is true the RooAddition object
  // will take ownership of the arguments in sumSet
}



//_____________________________________________________________________________
NewRooAddition::NewRooAddition(const char* name, const char* title, const RooArgList& sumSet1, const RooArgList& sumSet2, Bool_t takeOwnership) : RooAddition(name, title, sumSet1, sumSet2, takeOwnership)
{
  // Constructor with two set of RooAbsReals. The value of the function will be
  //
  //  A = sum_i sumSet1(i)*sumSet2(i) 
  //
  // If takeOwnership is true the RooAddition object will take ownership of the arguments in sumSet
}



//_____________________________________________________________________________
NewRooAddition::NewRooAddition(const NewRooAddition& other, const char* name) : RooAddition(other, name)
{
  // Copy constructor
  
  // Member _ownedList is intentionally not copy-constructed -- ownership is not transferred
}


//_____________________________________________________________________________
NewRooAddition::~NewRooAddition()
{
   // Destructor
}


//_____________________________________________________________________________
void NewRooAddition::enableSilentOffsetting(Bool_t flag)
{
  enableOffsetting(flag);
}
