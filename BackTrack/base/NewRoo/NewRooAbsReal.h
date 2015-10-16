//Created by KVClassFactory on Wed Sep 16 16:53:40 2015
//Author: fable

#ifndef __NEWROOABSREAL_H
#define __NEWROOABSREAL_H

#include "RooAbsReal.h"

class NewRooAbsReal : public RooAbsReal
{

   public:
   NewRooAbsReal();
   virtual ~NewRooAbsReal();
   
   virtual void enableSilentOffsetting(Bool_t flag) {} ;

   ClassDef(NewRooAbsReal,1)// Derived class of RooAbsReal to avoid RooArgSet checkForDup() messages
};

#endif
