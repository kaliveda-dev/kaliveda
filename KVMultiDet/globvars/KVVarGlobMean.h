//
// D.Cussol
//
// 17/02/2004:
// Creation d'une classe Variable Globale
//

#ifndef KVVarGlobMean_h
#define KVVarGlobMean_h
#include "KVVarGlob.h"

class KVVarGlobMean: public KVVarGlob {

   Double_t var;
   Double_t ect;
   Double_t svar2;
   Double_t svar;
   Double_t sw;
   Double_t min;
   Double_t max;
   Int_t calc;

protected:
   virtual Double_t getvalue_void() const;
   virtual Double_t getvalue_int(Int_t i) const;

   void init();
   void FillVar(Double_t v, Double_t w);
   void FillVar(Double_t v);
   static void FillMethodBody(KVString& body, int type);

public:
   KVVarGlobMean() : KVVarGlob("KVVarGlobMean")
   {
      init();
   }
   KVVarGlobMean(const Char_t* nom) : KVVarGlob(nom)
   {
      init();
   }
   ROOT_COPY_CTOR(KVVarGlobMean, KVVarGlob)
   virtual ~KVVarGlobMean() {}
   ROOT_COPY_ASSIGN_OP(KVVarGlobMean)

   virtual void Copy(TObject& obj) const;
   virtual void Reset();
   void Init();
   void Calculate();

   static void MakeClass(const Char_t* classname, const Char_t* classdesc, int type = KVVarGlob::kOneBody);

   ClassDef(KVVarGlobMean, 2)// Base class for global variables computing mean values
};
#endif
