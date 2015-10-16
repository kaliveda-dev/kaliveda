//Created by KVClassFactory on Wed Sep 16 16:37:51 2015
//Author: fable

#ifndef __NEWROOADDITION_H
#define __NEWROOADDITION_H

#include "RooAddition.h"

class NewRooAddition : public RooAddition
{

public:
   NewRooAddition();
   NewRooAddition(const char *name, const char *title, const RooArgList& sumSet, Bool_t takeOwnerShip=kFALSE) ;
   NewRooAddition(const char *name, const char *title, const RooArgList& sumSet1, const RooArgList& sumSet2, Bool_t takeOwnerShip=kFALSE) ;
   virtual ~NewRooAddition() ;

   NewRooAddition(const NewRooAddition& other, const char* name = 0);
   virtual TObject* clone(const char* newname) const { return new NewRooAddition(*this, newname); }
   
protected:
    
   //New method 
   void enableSilentOffsetting(Bool_t flag);  

ClassDef(NewRooAddition,1)// Derived class of RooAddition with addition of enableSilentOffsetting() method
};

#endif
