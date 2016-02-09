#include "ParameterName.h"

ClassImp(ParameterName)

const std::size_t ParameterName::maxlen = 20;

ParameterName::ParameterName(void):
#if __cplusplus < 201103L
   Name(NULL)
#else
   // C++11 supported, use nullptr
   Name(nullptr)
#endif
{

}

ParameterName::~ParameterName()
{
   if (Name) {
      delete[] Name;
#if __cplusplus < 201103L
      Name = NULL;
#else
      Name = nullptr;
#endif
   }
}


void ParameterName::Init()
{
   Name = new char[maxlen];
   std::memset(Name, '\0', maxlen);
}

