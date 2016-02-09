#ifndef KVPATHDEF
#define KVPATHDEF

#include "THtml.h"
#include "TClass.h"

class KVPathDef : public THtml::TPathDefinition {
public:
   KVPathDef() {}
   virtual ~KVPathDef() {}

   bool GetIncludeAs(TClass* cl, TString& out_include_as) const;
   bool GetDocDir(const TString& module, TString& doc_dir) const;
   ClassDef(KVPathDef, 0)
};
#endif
