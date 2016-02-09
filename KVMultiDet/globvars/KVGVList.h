//
// D.Cussol
//
// 18/02/2004:
// Creation d'une classe Liste de variables globales
//
//

#ifndef KVGVList_h
#define KVGVList_h
#include "KVVarGlob.h"
#include "KVList.h"
#include "TTree.h"

//#define DEBUG_KVGVList

#define MAX_CAP_BRANCHES 100

class KVGVList: public KVList {

   Double_t fBranchVar[MAX_CAP_BRANCHES];//! used for automatic creation & filling of TTree branches
   Int_t fIBranchVar[MAX_CAP_BRANCHES];//! used for automatic creation & filling of TTree branches
   Int_t fNbBranch;
   Int_t fNbIBranch;

public:
// Champs Statiques:
   static Int_t nb;
   static Int_t nb_crea;
   static Int_t nb_dest;
// Methodes
protected:
   void init_KVGVList(void);
   TList fVG1; // one-body variables
   TList fVG2; // two-body variables
   TList fVGN; // N-body variables

public:
   KVGVList(void);             // constructeur par defaut
   KVGVList(const KVGVList& a);        // constructeur par Copy

   virtual ~ KVGVList(void);   // destructeur

   virtual void Init(void);     // methode d'initialisation des
   // variables globales
   virtual void Reset(void);    // Remise a zero avant le

   virtual void Fill(KVNucleus* c);
   virtual void Fill2(KVNucleus* c1, KVNucleus* c2);
   virtual void FillN(KVEvent* e);
   void CalculateGlobalVariables(KVEvent* e);

   KVVarGlob* GetGV(const Char_t* nom);         //find global variable with name 'nom'

   //Returns first global variable in list with given class
   KVVarGlob* GetGVType(const Char_t* class_name)
   {
      return (KVVarGlob*)FindObjectByClass(class_name);
   };
   virtual void      Add(TObject* obj) ;

   // returns kTRUE if list contains 1-body variables
   Bool_t Has1BodyVariables()
   {
      return (fVG1.GetEntries() > 0);
   };

   // returns kTRUE if list contains 2-body variables
   Bool_t Has2BodyVariables()
   {
      return (fVG2.GetEntries() > 0);
   };

   // returns kTRUE if list contains N-body variables
   Bool_t HasNBodyVariables()
   {
      return (fVGN.GetEntries() > 0);
   };

   TObject** GetGVRef(const Char_t* name);

   virtual void MakeBranches(TTree*);
   virtual void FillBranches();

   ClassDef(KVGVList, 3)       // List of global variables
};
#endif
