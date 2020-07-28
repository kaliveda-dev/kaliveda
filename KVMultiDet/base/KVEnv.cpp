//Created by KVClassFactory on Mon Jan 14 12:23:45 2013
//Author: bonnet

#include "KVEnv.h"
#include "TNamed.h"
#include "THashList.h"
#include "KVString.h"

ClassImp(KVEnv)


//________________________________________________________________

KVEnv::KVEnv(const KVEnv& obj) : TEnv()
{
   // Copy constructor
   // This ctor is used to make a copy of an existing object (for example
   // when a method returns an object), and it is always a good idea to
   // implement it.
   // If your class allocates memory in its constructor(s) then it is ESSENTIAL :-)

   obj.Copy(*this);
}

//________________________________________________________________

KVEnv::KVEnv(const char* name) : TEnv(name)
{
   // Write your code here
}

KVEnv::~KVEnv()
{
   // Destructor
   fComments.Clear();
}

//________________________________________________________________
void KVEnv::Copy(TObject& obj) const
{
   // This method copies the current state of 'this' object into 'obj'
   // You should add here any member variables, for example:
   //    (supposing a member variable KVEnv::fToto)
   //    CastedObj.fToto = fToto;
   // or
   //    CastedObj.SetToto( GetToto() );

   TEnv::Copy(obj);
   //KVEnv& CastedObj = (KVEnv&)obj;
}
//________________________________________________________________
void KVEnv::CopyTable(TEnv& env)
{
   //Copy table of env to this
   THashList* hl = 0;
   if ((hl = env.GetTable())) {
      TIter next(hl);
      TEnvRec* env_rec = 0;
      while ((env_rec = (TEnvRec*)next.Next())) {
         SetValue(env_rec->GetName(), env_rec->GetValue());
      }
   }

}

//________________________________________________________________
void KVEnv::AddCommentLine(const Char_t* line)
{
   //Add a comment for the current TEnv
   //the line will started with "# " prefix and will
   //be ended with "\n" postfix
   //
   fComments.Add(new TNamed(Form("# %s\n", line), ""));

}

//________________________________________________________________
void KVEnv::AddComments(const Char_t* comments)
{
   //Add comments
   //the comments can contains "\n" character
   //each line contained in the comments will
   //begin with "# " prefix and will
   //be ended with "\n" postfix

   KVString st(comments);
   st.Begin("\n");
   while (!st.End()) {
      AddCommentLine(st.Next().Data());
   }
}

//________________________________________________________________
void KVEnv::ClearComments()
{
   fComments.Clear();
}

//________________________________________________________________
void KVEnv::PrintComments()
{
   //print comments as they will be written
   //in the file
   if (fComments.GetEntries() == 0) {
      printf("No comments defined\n");
      return;
   }
   for (Int_t ii = 0; ii < fComments.GetEntries(); ii += 1) {
      printf("%s", fComments.At(ii)->GetName());
   }
}

//________________________________________________________________
Int_t KVEnv::WriteFile(const char* fname, EEnvLevel level)
{
   // Write first comments associated to the current TEnv
   // Write resourse records to file fname for a certain level. Use
   // level kEnvAll to write all resources. Returns -1 on case of error,
   // 0 in case of success.

   if (!fname || !strlen(fname)) {
      Error("WriteFile", "no file name specified");
      return -1;
   }

   if (!GetTable()) {
      Error("WriteFile", "TEnv table is empty");
      return -1;
   }

   FILE* ofp;
   if ((ofp = fopen(fname, "w"))) {
      for (Int_t ii = 0; ii < fComments.GetEntries(); ii += 1) {
         fprintf(ofp, "%s", fComments.At(ii)->GetName());
      }

      //for (Int_t jj=0;jj<GetTable()->GetEntries();jj+=1){
      // TEnvRec *er = (TEnvRec*) GetTable()->At(jj);
      TIter next(GetTable());
      TEnvRec* er;
      while ((er = (TEnvRec*) next()))
         if (er->GetLevel() == level || level == kEnvAll)
            fprintf(ofp, "%-40s %s\n", Form("%s:", er->GetName()),
                    er->GetValue());
      //}
      fclose(ofp);
      return 0;
   }

   Error("WriteFile", "cannot open %s for writing", fname);
   return -1;
}
