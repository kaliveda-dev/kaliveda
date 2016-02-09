#ifndef __PARAMETER_NAME_H__
#define __PARAMETER_NAME_H__

#include "Rtypes.h"
#include <cstring>

class ParameterName {


public:

   char* Name;
   static const std::size_t maxlen;

   ParameterName();
   virtual ~ParameterName();

   void Init();

   ClassDef(ParameterName, 1)

};

#endif // __PARAMETER_NAME_H__ not defined

