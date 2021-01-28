#ifndef KVGVList_h
#define KVGVList_h
#include "KVVarGlob.h"
#include "KVUniqueNameList.h"
#include "TTree.h"
#include "KVEventClassifier.h"

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

Any event selection criterion is tested as soon as each variable has been calculated. If the test
fails, no further variables are calculated and the KVGVList goes into 'abort event' mode:
~~~~{.cpp}
    KVEvent* event_to_analyse;
    vglist.CalculateGlobalVariables( event_to_analyse );
    if( !vglist.AbortEventAnalysis() )
    {
       ... do further analysis, mtot is >=4
    }
~~~~

#### Definition of new kinematical frames
One or more variables in the list can be used to define new kinematical frames which can in turn be
used by any variables which occur after them in the list. In order to do so, call method SetNewFrameDefinition()
for the variable(s) in question with a lambda function having the following signature:
~~~~{.cpp}
[](KVEvent* e, const KVVarGlob* vg){ e->SetFrame("_frame_name_", ...); }
~~~~
When called (e.g. by KVGVList), the KVVarGlob pointer gives access to the global variable.

As an example of use, imagine that KVZmax is used to find the heaviest (largest Z) fragment in the
forward CM hemisphere, then the velocity of this fragment is used to define a "QP_FRAME"
in order to calculate the KVFlowTensor in this frame:

~~~~{.cpp}
    KVGVList vglist;
    auto vg = vglist.AddGV("KVZmax", "zmax");
    vg->SetFrame("CM");
    vg->SetSelection( {"V>0", [](const KVNucleus* n){ return n->GetVpar()>0; }} );
    vg->SetNewFrameDefinition(
                [](KVEvent* e, const KVVarGlob* v){
        e->SetFrame("QP_FRAME", static_cast<const KVZmax*>(v)->GetZmax(0)->GetVelocity());
    });
    vg = AddGV("KVFlowTensor", "qp_tensor");
    vg->SetFrame("QP_FRAME"); // frame will have been defined before tensor is filled
~~~~

 */
class KVGVList: public KVUniqueNameList {

   std::vector<Double_t> fBranchVar;//! used for automatic creation & filling of TTree branches
   std::vector<Int_t> fIBranchVar;//! used for automatic creation & filling of TTree branches
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
   KVVarGlob* prepareGVforAdding(const Char_t* class_name, const Char_t* name);

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
   KVVarGlob* AddGVFirst(const Char_t* class_name, const Char_t* name);

   void Init(void);     // methode d'initialisation des variables globales
   void Reset(void);    // Remise a zero avant le

   void CalculateGlobalVariables(KVEvent* e);

   KVVarGlob* GetGV(const Char_t* nom) const;         //find global variable with name 'nom'

   //Returns first global variable in list with given class
   KVVarGlob* GetGVType(const Char_t* class_name)
   {
      return (KVVarGlob*)FindObjectByClass(class_name);
   }
   virtual void      Add(TObject* obj) ;
   virtual void      AddFirst(TObject* obj) ;

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
   KVEventClassifier* AddEventClassifier(const TString& varname);

   ClassDef(KVGVList, 3)       // List of global variables
};
#endif
