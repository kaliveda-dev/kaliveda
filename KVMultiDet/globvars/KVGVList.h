#ifndef KVGVList_h
#define KVGVList_h
#include "KVVarGlob.h"
#include "KVUniqueNameList.h"
#include "TTree.h"

#define MAX_CAP_BRANCHES 100

/**
\class KVGVList
\brief Mangae a list of global variables
\ingroup GlobalVariables

This class allows to process in a single call many KVVargGlob instances.
The methods used to initialize and fill variables in such a list are:

~~~~~~~~~~~~
    void Init(void);                             // initializes the global variables (call once)
    void CalculateGlobalVariables(KVEvent *e);   // compute all global variables for the event
~~~~~~~~~~~~

By default the KVGVList does not own the objects it contains (they may be on the stack).
User's responsibility in this case to delete heap-based objects after use.

~~~~~~~~~~~~
    // Declarations and initialisations
    KVEkin *Sekin=new KVEkin("SEkin");
    KVZmean zmean;
    KVZmax  zmax;
    KVGVList gvlist;
    gvlist.Add(Sekin);
    gvlist.Add(&zmean);
    gvlist.Add(&zmax);
    gvlist.Init();

    // Treatment loop for each event called for each event
    gvlist.CalculateGlobalVariables(event); // with KVEvent* pointer to event to analyse

    cout << "Total kinetic energy : " << Sekin->GetValue() << endl;
    cout << "Mean charge          : " << zmean() << endl;
    cout << "Standard deviation   : " << zmean("RMS") << endl;
    cout << "Charge of the heaviest   : " << zmax() << endl;
    cout << "Vpar of the heaviest     : " << zmax.GetZmax(0)->GetVpar() << endl;
~~~~~~~~~~~~
 */
class KVGVList: public KVUniqueNameList {

   Double_t fBranchVar[MAX_CAP_BRANCHES];//! used for automatic creation & filling of TTree branches
   Int_t fIBranchVar[MAX_CAP_BRANCHES];//! used for automatic creation & filling of TTree branches
   Int_t fNbBranch;
   Int_t fNbIBranch;

   /// replace any mathematical symbols in 's' with '_'
   TString NameSanitizer(const Char_t* s) const
   {
      TString _s(s);
      _s.ReplaceAll("+", "_");
      _s.ReplaceAll("*", "_");
      _s.ReplaceAll("-", "_");
      _s.ReplaceAll("/", "_");
      _s.ReplaceAll("=", "_");
      return _s;
   }

protected:
   void init_KVGVList(void);
   TList fVG1; // one-body variables
   TList fVG2; // two-body variables
   TList fVGN; // N-body variables
   void Fill(const KVNucleus* c);
   void Fill2(const KVNucleus* c1, const KVNucleus* c2);
   void FillN(const KVEvent* e);
   void Calculate();
   void Calculate2();
   void CalculateN();

public:
   KVGVList(void);             // constructeur par defaut
   KVGVList(const KVGVList& a);        // constructeur par Copy

   virtual ~ KVGVList(void) {}

   KVVarGlob* AddGV(const Char_t* class_name, const Char_t* name);

   void Init(void);     // methode d'initialisation des variables globales
   void Reset(void);    // Remise a zero avant le

   void CalculateGlobalVariables(KVEvent* e);

   KVVarGlob* GetGV(const Char_t* nom);         //find global variable with name 'nom'

   //Returns first global variable in list with given class
   KVVarGlob* GetGVType(const Char_t* class_name)
   {
      return (KVVarGlob*)FindObjectByClass(class_name);
   }
   virtual void      Add(TObject* obj) ;

   // returns kTRUE if list contains 1-body variables
   Bool_t Has1BodyVariables()
   {
      return (fVG1.GetEntries() > 0);
   }

   // returns kTRUE if list contains 2-body variables
   Bool_t Has2BodyVariables()
   {
      return (fVG2.GetEntries() > 0);
   }

   // returns kTRUE if list contains N-body variables
   Bool_t HasNBodyVariables()
   {
      return (fVGN.GetEntries() > 0);
   }

   void MakeBranches(TTree*);
   void FillBranches();

   ClassDef(KVGVList, 3)       // List of global variables
};
#endif
