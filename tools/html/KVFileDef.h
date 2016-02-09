#ifndef KVFileDEF
#define KVFileDEF

#include "THtml.h"
#include "TClass.h"

class KVFileDef : public THtml::TFileDefinition {
public:
   KVFileDef() {}
   virtual ~KVFileDef() {}

   bool  GetImplFileName(const TClass* cl, TString& out_filename, TString& out_fsys, THtml::TFileSysEntry** fse = 0) const;

   ClassDef(KVFileDef, 0)
};
#endif
