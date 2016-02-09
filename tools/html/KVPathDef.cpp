#include "KVPathDef.h"
#include "KVBase.h"
#include "TSystem.h"
#include <iostream>

ClassImp(KVPathDef)

bool KVPathDef::GetIncludeAs(TClass* cl, TString& out_include_as) const
{
   // strip off all directory information from header filename
   // this is for the '#include' directive shown in the floating
   // yellow box on each class page
   out_include_as = gSystem->BaseName(cl->GetDeclFileName());
   return kTRUE;
}
bool KVPathDef::GetDocDir(const TString& module, TString& doc_dir) const
{
   //std::cout << "looking for doc for module " << module << std::endl;
   doc_dir = KVBase::GetKVBuildDir() + TString("/");
   doc_dir += module;
   //std::cout << "look in " << doc_dir << std::endl;
   return true;
}

