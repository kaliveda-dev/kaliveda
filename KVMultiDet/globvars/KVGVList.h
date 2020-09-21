#ifndef KVGVList_h
#define KVGVList_h
#include "KVVarGlob.h"
#include "KVUniqueNameList.h"
#include "TTree.h"

#define MAX_CAP_BRANCHES 100

/**
\class KVGVList
\brief Manage a list of global variables
\ingroup GlobalVariables

The KVGVList class handles a list of global variables. A list can be used in the following
schematic way to calculate several global variables at once:

#### Creation & initialisation

~~~~~~~~~~~~{.cpp}
      KVVGList VGlist;
      VGlist.Add( new SomeVarGlob("var1") );    // add variable using explicit call to constructor
      VGlist.AddGV("SomeOtherVarGlob", "var2"); // add variable using class name
      ...
      VGlist.Init(); // initialise all variables
~~~~~~~~~~~~

#### Treatment of 1 event

~~~~~~~~~~~~{.cpp}
      VGlist.CalculateGlobalVariables( [event] );          // calculate contribution of each particle to each variable
      if( !VGlist.AbortEventAnalysis() ) // in case cuts for event selection were set - see below
      {
          auto valueOfvar1 = VGlist.GetGV("var1")->GetValue(); // retrieve value of "var1" for event
      }
~~~~~~~~~~~~

The CalculateGlobalVariables() method is optimised to ensure that all one- and two-body variables
are calculated with a single loop over the particles in each event.

#### Event selection criteria
Conditions ('cuts') can be set on each variable which
decide whether or not to retain an event for analysis. If any variable in the list fails the
test, processing of the list is abandoned.

Selection criteria are set using lambda expressions. In this example, the variable "mtot"
must have a value of at least 4 for the event to be retained:
~~~~{.cpp}
   KVGVList vglist;
   auto mtot = vglist.AddGV("KVMult","mtot");
   mtot->SetEventSelection([](const KVVarGlob* v){ return v->GetValue()>=4; });
~~~~

Any event selection criterion is tested as soon as each variable has been tested. If the test
fails, no further variables are calculated and the KVGVList goes into 'abort event' mode:
~~~~{.cpp}
    KVEvent* event_to_analyse;
    vglist.CalculateGlobalVariables( event_to_analyse );
    if( !vglist.AbortEventAnalysis() )
    {
       ... do further analysis, mtot is >=4
    }
~~~~
 */
class KVGVList: public KVUniqueNameList {

   Double_t fBranchVar[MAX_CAP_BRANCHES];//! used for automatic creation & filling of TTree branches
   Int_t fIBranchVar[MAX_CAP_BRANCHES];//! used for automatic creation & filling of TTree branches
   Int_t fNbBranch;
   Int_t fNbIBranch;
   bool fAbortEventAnalysis;// set to false if a global variable fails its own event selection criterion

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

   bool AbortEventAnalysis() const
   {
      return fAbortEventAnalysis;
   }

   ClassDef(KVGVList, 3)       // List of global variables
};
#endif
