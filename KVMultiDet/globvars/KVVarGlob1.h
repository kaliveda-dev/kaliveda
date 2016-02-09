//
// D.Cussol
//
// 17/02/2004
// Creation d'une classe Variable Globale
//

#ifndef KVVarGlob1_h
#define KVVarGlob1_h
#include "KVVarGlob.h"

//#define DEBUG_KVVarGlob1

class KVVarGlob1: public KVVarGlob {
public:
// Champs Statiques:
   static Int_t nb;
   static Int_t nb_crea;
   static Int_t nb_dest;
// Champ propre:
protected:
   Double_t var;// the quantity to be calculated
// Methodes
protected:
   void init(void);
   void FillVar(Double_t val)
   {
      var += val;
   };
   void SetValue(Double_t val)
   {
      // On donne la valeur de la variable
      var = val;
   };
   static void FillMethodBody(KVString& body, int type);

   virtual Double_t getvalue_void(void) const;
   virtual Double_t getvalue_int(Int_t i);

public:
   KVVarGlob1(void);            // constructeur par defaut
   KVVarGlob1(Char_t* nom);
   KVVarGlob1(const KVVarGlob1& a);     // constructeur par Copy

   virtual ~ KVVarGlob1(void);  // destructeur


   KVVarGlob1& operator =(const KVVarGlob1& a);         // operateur =


#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   virtual void Copy(TObject& obj) const;
#else
   virtual void Copy(TObject& obj);
#endif
   virtual void Init(void);
   virtual void Reset(void);

   static void MakeClass(const Char_t* classname,
                         const Char_t* classdesc, int type = KVVarGlob::kOneBody);
   virtual Int_t GetNumberOfValues() const
   {
      return 1;
   }

   ClassDef(KVVarGlob1, 1)      // Base class for global variables with one value
};
#endif
