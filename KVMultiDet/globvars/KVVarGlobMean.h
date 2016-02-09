//
// D.Cussol
//
// 17/02/2004:
// Creation d'une classe Variable Globale
//

#ifndef KVVarGlobMean_h
#define KVVarGlobMean_h
#include "KVVarGlob1.h"

//#define DEBUG_KVVarGlobMean

class KVVarGlobMean: public KVVarGlob1 {
public:
// Champs Statiques:
   static Int_t nb;
   static Int_t nb_crea;
   static Int_t nb_dest;
// Champ propre:
protected:
   Double_t ect;
   Double_t svar2;
   Double_t svar;
   Double_t sw;
   Double_t min;
   Double_t max;
   Int_t calc;
   Double_t fTab[7];//! used by GetValuePtr

// Methodes
   virtual Double_t getvalue_void(void) const;
   virtual Double_t getvalue_int(Int_t i);

public:
   Int_t GetCALC(void) const
   {
      return calc;
   };
   void SetECT(Double_t x)
   {
      ect = x;
   };
   void SetSVAR2(Double_t x)
   {
      svar2 = x;
   };
   void SetSVAR(Double_t x)
   {
      svar = x;
   };
   void SetSW(Double_t x)
   {
      sw = x;
   };
   void SetMIN(Double_t x)
   {
      min = x;
   };
   void SetMAX(Double_t x)
   {
      max = x;
   };
   void SetCALC(Int_t x)
   {
      calc = x;
   };
protected:
   void init(void);
   void CalcVar(void);
   void FillVar(Double_t v, Double_t w);
   void FillVar(Double_t v);
   static void FillMethodBody(KVString& body, int type);

public:
   KVVarGlobMean(void);         // constructeur par defaut
   KVVarGlobMean(Char_t* nom);
   KVVarGlobMean(const KVVarGlobMean& a);       // constructeur par Copy

   virtual ~ KVVarGlobMean(void);       // destructeur

#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   virtual void Copy(TObject& obj) const;
#else
   virtual void Copy(TObject& obj);
#endif

   KVVarGlobMean& operator =(const KVVarGlobMean& a);   // operateur =


   virtual void Init(void);     // methode d'initialisation des
   // variables internes
   virtual void Reset(void);    // Remise a zero avant le

   virtual Double_t* GetValuePtr(void);

   static void MakeClass(const Char_t* classname, const Char_t* classdesc, int type = KVVarGlob::kOneBody);

   ClassDef(KVVarGlobMean, 1)   // Base class for global variables computing mean values
};
#endif
