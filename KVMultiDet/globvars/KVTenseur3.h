//
// D.Cussol 18/12/2004
//
// Developpement d'un classe KVenseur3 permettant de calculer les axes
// propres d'un evenement
//

#ifndef ROOT_KVTenseur3
#define ROOT_KVTenseur3
#include "KVBase.h"
#include "TVector3.h"
#include "KVNucleus.h"
#include "TRotation.h"

class KVTenseur3: public KVBase {
public:
//
// Champs statiques
//
   static Int_t nb_KVTenseur3;

//
// Champs non statiques
//
private:
   Double_t tenseur[9];
   Double_t vap[3];
   Double_t vep[9];
   Int_t is_diago;

//
// Methodes
//
private:
   void init_KVTenseur3(void);
   void Diago(void);

public:
   KVTenseur3(void);
   KVTenseur3(Char_t* nom);
   KVTenseur3(const KVTenseur3& t);

   virtual ~ KVTenseur3(void);

#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   virtual void Copy(TObject& obj) const;
#else
   virtual void Copy(TObject& obj);
#endif

   KVTenseur3& operator =(const KVTenseur3& a);        // operateur =

   virtual void Print(Option_t* opt = "") const;

   virtual void Fill(const TVector3& v, Double_t w = 1.);
   virtual void Fill(const KVNucleus& c, Double_t w = 1.);
   virtual void Reset(void);
   virtual Double_t GetThetaFlot(void);
   virtual Double_t GetPhiPlan(void);
   virtual Double_t GetSphericite(void);
   virtual Double_t GetCoplanarite(void);
   virtual Double_t GetDirectivite(void);
   virtual Double_t GetElem(Int_t i, Int_t j) const;
   virtual Double_t GetVap(Int_t i);
   virtual void SetElem(Int_t i, Int_t j, Double_t x);
   virtual void SetVap(Int_t i, Double_t x);
   virtual Double_t GetVapNorm(Int_t i);
   virtual TVector3 GetVep(Int_t i);
   virtual Double_t GetVep(Int_t i, Int_t j);
   virtual void SetVep(Int_t i, Int_t j, Double_t x);

   virtual void GetRotation(TRotation&);

   ClassDef(KVTenseur3, 1)     // Base class for tensor management
};
#endif
