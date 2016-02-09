//
// D.Cussol
//
// 18/02/2004:
// Creation d'une classe Variable Globale Tenseur de moments
// Basee sur KVTenseur3
//

#ifndef KVTensP_h
#define KVTensP_h
#include "KVVarGlob.h"
#include "KVTenseur3.h"

//#define DEBUG_KVTensP

class KVTensP: public KVVarGlob {
protected:
   static Int_t nb;
   static Int_t nb_crea;
   static Int_t nb_dest;
   KVTenseur3* tenseurP;

   Double_t fVal[8];//! used by GetValuePtr

   void init_KVTensP(void);
   virtual Double_t getvalue_void(void) const;
   virtual Double_t getvalue_int(Int_t i);

public:
   KVTensP(void);              // constructeur par defaut
   KVTensP(const Char_t* nom);
   KVTensP(const Char_t* nom, const Char_t* frame);
   KVTensP(const KVTensP& a);  // constructeur par Copy

   virtual ~ KVTensP(void);    // destructeur

#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   virtual void Copy(TObject& obj) const;
#else
   virtual void Copy(TObject& obj);
#endif

   KVTensP& operator =(const KVTensP& a);      // operateur =


   virtual void Init(void);     // methode d'initialisation des
   // variables internes
   virtual void Reset(void);    // Remise a zero avant le
   // traitement d'un evenement
   virtual void Fill(KVNucleus* c);     // Remplissage de la variable.

   virtual Double_t* GetValuePtr(void); // On retourne un tableau de

   virtual void SetPartGroup(const Char_t* lab)
   {
      SetLabel(lab);
   }
   virtual const Char_t* GetPartGroup() const
   {
      return GetLabel();
   }

   virtual Int_t GetZmin(void) const
   {
      return (Int_t)const_cast<KVTensP*>(this)->GetParameter("Zmin");
   };
   virtual void SetZmin(Int_t zm)
   {
      SetParameter("Zmin", zm);
   };

   KVTenseur3* GetTensor() const
   {
      return tenseurP;
   };
   void SetTensor(const KVTenseur3*);

   ClassDef(KVTensP, 3)         // Global variable Momentum tensor
};
#endif
