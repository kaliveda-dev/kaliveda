#ifndef KVVarGlob1_h
#define KVVarGlob1_h
#include "KVVarGlob.h"

/**
  \class KVVarGlob1
  \brief Abstract base class for global variables which calculate a single value
  \ingroup GlobalVariables

  \author D. Cussol
  \date 2004
 */
class KVVarGlob1: public KVVarGlob {

   Double_t var;// the quantity to be calculated
   void init(void)
   {
      var = 0;
   }

protected:
   void FillVar(Double_t val)
   {
      var += val;
   }
   void SetValue(Double_t val)
   {
      var = val;
   }
   static void FillMethodBody(KVString& body, int type);

   Double_t getvalue_int(Int_t) const
   {
      return var;
   }

public:
   KVVarGlob1() : KVVarGlob("KVVarGlob1")
   {
      init();
   }
   KVVarGlob1(const Char_t* nom) : KVVarGlob(nom)
   {
      init();
   }
   ROOT_COPY_CTOR(KVVarGlob1, KVVarGlob)
   ROOT_COPY_ASSIGN_OP(KVVarGlob1)
   virtual ~KVVarGlob1() {}

   void Copy(TObject& obj) const
   {
      KVVarGlob::Copy(obj);
      ((KVVarGlob1&)obj).var = var;
   }
   void Init()
   {
      init();
   }
   void Reset()
   {
      init();
   }

   static void MakeClass(const Char_t* classname,
                         const Char_t* classdesc, int type = KVVarGlob::kOneBody);
   Int_t GetNumberOfValues() const
   {
      return 1;
   }

   ClassDef(KVVarGlob1, 1)      // Base class for global variables with one value
};
#endif
