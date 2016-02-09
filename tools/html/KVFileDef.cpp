#include "KVFileDef.h"
#include "TSystem.h"
#include "KVBase.h"

ClassImp(KVFileDef)

bool KVFileDef::GetImplFileName(const TClass* cl, TString& out_filename, TString& out_fsys, THtml::TFileSysEntry** fse) const
{
   bool ok = THtml::TFileDefinition::GetImplFileName(cl, out_filename, out_fsys, fse);
   if (ok) {
      TString root = KVBase::GetKVSourceDir() + TString("/");
      out_filename = out_filename.ReplaceAll(root, "");
      //Info("GetImplFileName","ok=%d name=%s fsys=%s",ok,out_filename.Data(),out_fsys.Data());
   }
   return ok;
}
