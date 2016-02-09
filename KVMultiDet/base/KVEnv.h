//Created by KVClassFactory on Mon Jan 14 12:23:45 2013
//Author: bonnet

#ifndef __KVENV_H
#define __KVENV_H

#include "TEnv.h"
#include "TList.h"

class KVEnv : public TEnv {
protected:
   TList fComments;

public:

   KVEnv(const KVEnv& obj);
   KVEnv(const char* name = "");
   virtual ~KVEnv();
   void Copy(TObject& obj) const;
   void CopyTable(TEnv& env);

   virtual Int_t  WriteFile(const char* fname, EEnvLevel level = kEnvAll);
   void AddCommentLine(const Char_t* line);
   void AddComments(const Char_t* comments);
   void ClearComments();
   void PrintComments();

   ClassDef(KVEnv, 1) //child class of TEnv to allow the writing of comments in the file
};

#endif
