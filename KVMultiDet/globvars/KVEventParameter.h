#ifndef __KVEVENTPARAMETER_H
#define __KVEVENTPARAMETER_H

#include "KVVarGlob1.h"

#include "KVEvent.h"

/**
 \class KVEventParameter
 \brief Global variable to retrieve parameters from events
 \ingroup GlobalVariables

 Use this global variable to retrieve the values of named parameters associated
 with events. The name of the variable is the name of the parameter.

 All values are retrieved as Double_t type variables.

 \author John Frankland
 \date Wed Feb 24 08:16:23 2021
*/

class KVEventParameter: public KVVarGlob1 {

private:
   void init()
   {
      fType = KVVarGlob::kNBody; // this is a N-body variable
   }

public:
   KVEventParameter() : KVVarGlob1()
   {
      init();
   }
   KVEventParameter(const Char_t* parameter_name)
      : KVVarGlob1(parameter_name)
   {
      init();
   }
   ROOT_COPY_CTOR(KVEventParameter, KVVarGlob1)
   ROOT_COPY_ASSIGN_OP(KVEventParameter)
   virtual ~KVEventParameter(void) {}

   void Calculate() {}
   void FillN(const KVEvent* e)
   {
      // retrieve value of parameter
      SetValue(e->GetParameters()->GetDoubleValue(GetName()));
   }

   ClassDef(KVEventParameter, 1) //Global variable to retrieve parameters from events
};

#endif
