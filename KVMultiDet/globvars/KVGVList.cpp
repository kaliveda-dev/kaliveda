// Author: Daniel Cussol
//
// 17/02/2004
#include "Riostream.h"
#include "KVGVList.h"

#include <KVEvent.h>

ClassImp(KVGVList)


//_________________________________________________________________
void KVGVList::init_KVGVList(void)
{
   fNbBranch = 0;
   fNbIBranch = 0;
}

//_________________________________________________________________
KVGVList::KVGVList(void): KVUniqueNameList()
{
   init_KVGVList();
}


//_________________________________________________________________
KVGVList::KVGVList(const KVGVList& a) : KVUniqueNameList()
{
   init_KVGVList();
   a.Copy(*this);
}

//_________________________________________________________________
void KVGVList::Init(void)
{
   // Initialisation of all global variables in list
   // As this method may be called several times we ensure that
   // variables are only initialised once

   this->R__FOR_EACH(KVVarGlob, ListInit)();
}

//_________________________________________________________________
void KVGVList::Reset(void)
{
   // Reset all variables before treating an event
   this->R__FOR_EACH(KVVarGlob, Reset)();
   fAbortEventAnalysis = false;
}

//_________________________________________________________________
void KVGVList::Fill(const KVNucleus* c)
{
   // Calls KVVarGlob::Fill(KVNucleus*) method of all one-body variables in the list
   // for all KVNucleus satisfying the KVParticleCondition given to
   // KVVarGlob::SetSelection() (if no selection given, all nuclei are used).

   fVG1.R__FOR_EACH(KVVarGlob, Fill)(c);
}


//_________________________________________________________________
void KVGVList::Fill2(const KVNucleus* c1, const KVNucleus* c2)
{
   // Calls KVVarGlob::Fill(KVNucleus*,KVNucleus*) method of all two-body variables in the list
   // for all pairs of KVNucleus (c1,c2) satisfying the KVParticleCondition given to
   // KVVarGlob::SetSelection() (if no selection given, all nuclei are used).

   fVG2.R__FOR_EACH(KVVarGlob, Fill2)(c1, c2);
}

//_________________________________________________________________
void KVGVList::FillN(const KVEvent* r)
{
   // Calls KVVarGlob::FillN(KVEvent*) method of all N-body variables in the list
   fVGN.R__FOR_EACH(KVVarGlob, FillN)(r);
}

void KVGVList::Calculate()
{
   // Calculate all 1-body observables after filling
   TIter it(&fVG1);
   KVVarGlob* vg;
   while ((vg = (KVVarGlob*)it())) {
      vg->Calculate();
#ifdef USING_ROOT6
      if (!vg->TestEventSelection()) {
         fAbortEventAnalysis = true;
         break;
      }
#endif
   }
}

void KVGVList::Calculate2()
{
   // Calculate all 2-body observables after filling
   TIter it(&fVG2);
   KVVarGlob* vg;
   while ((vg = (KVVarGlob*)it())) {
      vg->Calculate();
#ifdef USING_ROOT6
      if (!vg->TestEventSelection()) {
         fAbortEventAnalysis = true;
         break;
      }
#endif
   }
}

void KVGVList::CalculateN()
{
   // Calculate all N-body observables after filling
   TIter it(&fVGN);
   KVVarGlob* vg;
   while ((vg = (KVVarGlob*)it())) {
      vg->Calculate();
#ifdef USING_ROOT6
      if (!vg->TestEventSelection()) {
         fAbortEventAnalysis = true;
         break;
      }
#endif
   }
}

void KVGVList::CalculateGlobalVariables(KVEvent* e)
{
   // This method will calculate all global variables defined in the list for the event 'e'.
   //
   // Note that for 2-body variables, the Fill2 method will be called for each distinct pair of particles
   // in the event, plus each pair made up of a particle and itself.
   //
   // For each variable for which an event selection condition was set (see KVVarGlob::SetEventSelection())
   // the condition is tested as soon as the variable is calculated. If the condition is not satisfied,
   // calculation of the other variables is abandonded and method AbortEventAnalysis() returns kTRUE.

   Reset();

   TIter it(this);
   KVVarGlob* vg;
   while ((vg = (KVVarGlob*)it())) {

      if (vg->IsGlobalVariable()) {
         if (vg->IsNBody()) vg->FillN(e);
         else {
            Int_t mult = e->GetMult();
            for (int i = 1; i <= mult; ++i) {
               KVNucleus* nuc_i = e->GetParticle(i);
               if (nuc_i->IsOK()) {
                  if (vg->IsTwoBody()) {
                     for (int j = i; j <= mult; ++j) {
                        KVNucleus* nuc_j = e->GetParticle(j);
                        if (nuc_j->IsOK()) {
                           // we use every distinct pair of particles (including identical pairs) in the event
                           vg->Fill2(nuc_i, nuc_j);
                        }
                     }
                  }
                  else {
                     vg->Fill(nuc_i);
                  }
               }
            }
         }
         vg->Calculate();
#ifdef USING_ROOT6
         if ((fAbortEventAnalysis = !vg->TestEventSelection())) {
            return;
         }
         vg->DefineNewFrame(e);
#endif
      }
   }
}

//_________________________________________________________________
KVVarGlob* KVGVList::GetGV(const Char_t* nom) const
{
   //Return pointer to global variable in list with name 'nom'

   return (KVVarGlob*) FindObject(nom);
}

//_________________________________________________________________
void KVGVList::Add(TObject* obj)
{
   // Overrides KVUniqueNameList::Add(TObject*) so that global variable pointers are sorted
   // between the 3 lists used for 1-body, 2-body & N-body variables.
   //
   // We also print a warning in case the user tries to add a global variable with the
   // same name as one already in the list. In the case we retain only the first global variable,
   // any others with the same name are ignored

   KVUniqueNameList::Add(obj);   // add object to main list, check duplicates
   if (!ObjectAdded()) {
      Warning("Add", "You tried to add a global variable with the same name as one already in the list");
      Warning("Add", "Only the first variable added to the list will be used: name=%s class=%s",
              GetGV(obj->GetName())->GetName(), GetGV(obj->GetName())->ClassName());
      Warning("Add", "The following global variable (the one you tried to add) will be ignored:");
      printf("\n");
      obj->Print();
      return;
   }
   if (obj->InheritsFrom("KVVarGlob")) {
      // put global variable pointer in appropriate list
      KVVarGlob* vg = (KVVarGlob*)obj;
      if (vg->IsOneBody()) fVG1.Add(vg);
      else if (vg->IsTwoBody()) fVG2.Add(vg);
      else if (vg->IsNBody()) fVGN.Add(vg);
   }
}
void KVGVList::AddFirst(TObject* obj)
{
   // Overrides KVUniqueNameList::AddFirst(TObject*) so that global variable pointers are sorted
   // between the 3 lists used for 1-body, 2-body & N-body variables.
   //
   // We also print a warning in case the user tries to add a global variable with the
   // same name as one already in the list. In the case we retain only the first global variable,
   // any others with the same name are ignored

   KVUniqueNameList::AddFirst(obj);   // add object to main list, check duplicates
   if (!ObjectAdded()) {
      Warning("AddFirst", "You tried to add a global variable with the same name as one already in the list");
      Warning("AddFirst", "Only the first variable added to the list will be used: name=%s class=%s",
              GetGV(obj->GetName())->GetName(), GetGV(obj->GetName())->ClassName());
      Warning("AddFirst", "The following global variable (the one you tried to add) will be ignored:");
      printf("\n");
      obj->Print();
      return;
   }
   if (obj->InheritsFrom("KVVarGlob")) {
      // put global variable pointer in appropriate list
      KVVarGlob* vg = (KVVarGlob*)obj;
      if (vg->IsOneBody()) fVG1.Add(vg);
      else if (vg->IsTwoBody()) fVG2.Add(vg);
      else if (vg->IsNBody()) fVGN.Add(vg);
   }
}

//_________________________________________________________________

void KVGVList::MakeBranches(TTree* tree)
{
   // Create a branch in the TTree for each global variable in the list.
   // A leaf with the name of each global variable will be created to hold the
   // value of the variable (result of KVVarGlob::GetValue() method).
   // For multi-valued global variables we add a branch for each value with name
   //
   //    `GVname.ValueName`
   //
   // To avoid problems with TTree::Draw, 'GVName' will be sanitized i.e. any
   // mathematical symbols will be replaced by '_'.
   //
   // Any variable for which KVVarGlob::SetMaxNumBranches() was called with argument `0`
   // will not be added to the TTree.

   if (!tree) return;

   // Make sure all variables are initialised before proceeding
   Init();

   fNbBranch = 0;
   fNbIBranch = 0;
   fBranchVar.clear();
   fIBranchVar.clear();

   // calculate the number of variables which will be stored in each vector
   // the space has to be pre-reserved not added by successive "push_back"s, otherwise references to vector elements
   // are invalidated every time the vector reallocates space

   TIter next(this);
   KVVarGlob* ob;
   while ((ob = (KVVarGlob*)next())) {
      if (ob->GetNumberOfBranches()) { //skip variables for which KVVarGlob::SetMaxNumBranches(0) was called
         if (ob->GetNumberOfValues() > 1) {
            // multi-valued variable
            for (int i = 0; i < ob->GetNumberOfBranches(); i++) {
               if (ob->GetValueType(i) == 'I') ++fNbIBranch;
               else ++fNbBranch;
            }
         }
         else {
            if (ob->GetValueType(0) == 'I') ++fNbIBranch;
            else ++fNbBranch;
         }
      }
   }

   fBranchVar.reserve(fNbBranch);
   fIBranchVar.reserve(fNbIBranch);
   fNbBranch = 0;
   fNbIBranch = 0;

   next.Reset();

   while ((ob = (KVVarGlob*)next())) {
      if (ob->GetNumberOfBranches()) { //skip variables for which KVVarGlob::SetMaxNumBranches(0) was called
         TString sane_varname = NameSanitizer(ob->GetName());
         if (ob->GetNumberOfValues() > 1) {
            // multi-valued variable
            for (int i = 0; i < ob->GetNumberOfBranches(); i++) {
               // replace any nasty mathematical symbols which could pose problems
               // in names of TTree leaves/branches
               TString sane_name(ob->GetValueName(i));
               sane_name.ReplaceAll("*", "star");
               if (ob->GetValueType(i) == 'I') {
                  tree->Branch(Form("%s.%s", sane_varname.Data(), sane_name.Data()), &fIBranchVar[ fNbIBranch ], Form("%s.%s/I", sane_varname.Data(), sane_name.Data()));
                  ++fNbIBranch;
               }
               else {
                  tree->Branch(Form("%s.%s", sane_varname.Data(), sane_name.Data()), &fBranchVar[ fNbBranch ], Form("%s.%s/D", sane_varname.Data(), sane_name.Data()));
                  ++fNbBranch;
               }
            }
         }
         else {
            if (ob->GetValueType(0) == 'I') {
               tree->Branch(sane_varname, &fIBranchVar[ fNbIBranch ], Form("%s/I", sane_varname.Data()));
               ++fNbIBranch;
            }
            else {
               tree->Branch(sane_varname, &fBranchVar[ fNbBranch ], Form("%s/D", sane_varname.Data()));
               ++fNbBranch;
            }
         }
      }
   }
}

//_________________________________________________________________

void KVGVList::FillBranches()
{
   // Use this method ONLY if you first use MakeBranches(TTree*) in order to
   // automatically create branches for your global variables.
   // Call this method for each event in order to put the values of the variables
   // in the branches ready for TTree::Fill to be called (note that TTree::Fill is not
   // called in this method: you should do it after this).

   if (!fNbBranch && !fNbIBranch) return;  // MakeBranches has not been called

   int INT_index = 0;
   int FLT_index = 0;
   TIter next(this);
   KVVarGlob* ob;
   while ((ob = (KVVarGlob*)next())) {
      if (ob->GetNumberOfBranches()) { //skip variables for which KVVarGlob::SetMaxNumBranches(0) was called

         if (ob->GetNumberOfValues() > 1) {
            // multi-valued variable
            for (int j = 0; j < ob->GetNumberOfBranches(); j++) {
               if (ob->GetValueType(j) == 'I') {
                  fIBranchVar[ INT_index ] = (Int_t)ob->GetValue(j);
                  ++INT_index;
               }
               else {
                  fBranchVar[ FLT_index ] = ob->GetValue(j);
                  ++FLT_index;
               }
            }
         }
         else {
            if (ob->GetValueType(0) == 'I') {
               fIBranchVar[ INT_index ] = (Int_t)ob->GetValue();
               ++INT_index;
            }
            else {
               fBranchVar[ FLT_index ] = ob->GetValue();
               ++FLT_index;
            }
         }

      }
   }
}

KVEventClassifier* KVGVList::AddEventClassifier(const TString& varname)
{
   // Add an event classification object to the list, based on the named global variable
   // (which must already be in the list).
   //
   // Returns a pointer to the object, in order to add either cuts or bins like so:
   //
   // #### Using cuts
   //~~~~{.cpp}
   //        mtot_cuts->AddCut(5);
   //        mtot_cuts->AddCut(10);
   //        mtot_cuts->AddCut(15);
   //        mtot_cuts->AddCut(20);
   //        mtot_cuts->AddCut(25);
   //        mtot_cuts->AddCut(30);
   //~~~~
   //   This will class events according to:
   //   | mtot       | mtot_EC |
   //   |------------|---------|
   //   |         <5 |    0    |
   //   |  [5, 9]    |    1    |
   //   |  [10, 14]  |    2    |
   //   |  [15, 19]  |    3    |
   //   |  [20, 24]  |    4    |
   //   |  [25, 29]  |    5    |
   //   |  >=30      |    6    |
   //
   // `mtot_EC` is the default name for an event classification based on `mtot` and will be
   // used for the branch added to the user's analysis TTree by method MakeBranches().
   //
   // #### Using bins
   // ~~~~{.cpp}
   //              mtot_cuts->AddBin(5,10);
   //              mtot_cuts->AddBin(15,20);
   //              mtot_cuts->AddBin(25,30);
   //~~~~
   //This will class events according to the 2 bins with the following numbers:
   // | mtot     |   mtot_EC   |
   // |----------|-------------|
   // |  [5, 9]  |    0        |
   // | [15, 19] |    1        |
   // | [25, 29] |    2        |
   // | other    |    -1       |
   //
   // Note that in this case, any value outside of a defined bin is unclassified.

   KVVarGlob* gv = GetGV(varname);
   if (!gv) {
      Warning("AddEventClassifier", "Variable %s not found in list. No classification possible.", varname.Data());
   }
   KVEventClassifier* ec = new KVEventClassifier(gv);
   Add(ec);
   return ec;
}

KVVarGlob* KVGVList::AddGV(const Char_t* class_name, const Char_t* name)
{
   //Add a global variable to the list.
   //
   //`"class_name"` must be the name of a valid class inheriting from KVVarGlob, e.g. any of the default global
   //variable classes defined as part of the standard KaliVeda package (see the [Global Variables module](group__GlobalVariables.html)),
   //or the name of a user-defined class (see below).
   //
   //`"name"` is a unique name for the new global variable object which will be created and added to the
   //list of global variables. This name can later be used to retrieve the object (see GetGV()).
   //
   //Returns pointer to new global variable object in case more than the usual default initialisation is necessary.
   //
   //### User-defined global variables
   //The user may use her own global variables, without having to add them to the main libraries.
   //If the given class name is not known, it is assumed to be a user-defined class and we attempt to compile and load
   //the class from the user's source code. For this to work, the user must:
   //
   //  1. add to the ROOT macro path the directory where her class's source code is kept, e.g. in `$HOME/.rootrc` add the following line:
   //
   //~~~~~~~~~~~~~~~
   //              +Unix.*.Root.MacroPath:      $(HOME)/myVarGlobs
   //~~~~~~~~~~~~~~~
   //
   //  2. for each user-defined class, add a line to $HOME/.kvrootrc to define a "plugin". E.g. for a class called MyNewVarGlob,
   //
   //~~~~~~~~~~~~~~~
   //              +Plugin.KVVarGlob:    MyNewVarGlob    MyNewVarGlob     MyNewVarGlob.cpp+   "MyNewVarGlob(const char*)"
   //~~~~~~~~~~~~~~~
   //
   //  It is assumed that `MyNewVarGlob.h` and `MyNewVarGlob.cpp` will be found in `$HOME/myVarGlobs` (in this example).
   // The constructor taking a single character string argument (name of the variable) must be defined in the class.

   KVVarGlob* vg = prepareGVforAdding(class_name, name);
   if (vg) Add(vg);
   return vg;
}

KVVarGlob* KVGVList::prepareGVforAdding(const Char_t* class_name, const Char_t* name)
{
   KVVarGlob* vg = nullptr;
   TClass* clas = TClass::GetClass(class_name);
   if (!clas) {
      //class not in dictionary - user-defined class ? Look for plugin.
      TPluginHandler* ph = KVBase::LoadPlugin("KVVarGlob", class_name);
      if (!ph) {
         //not found
         Error("AddGV(const Char_t*,const Char_t*)",
               "Called with class_name=%s.\nClass is unknown: not in standard libraries, and plugin (user-defined class) not found",
               class_name);
         return 0;
      }
      else {
         vg = (KVVarGlob*) ph->ExecPlugin(1, name);
      }
   }
   else if (!clas->InheritsFrom("KVVarGlob")) {
      Error("AddGV(const Char_t*,const Char_t*)",
            "%s is not a valid class deriving from KVVarGlob.",
            class_name);
      return nullptr;
   }
   else {
      // need to use plugins to create even known classes, in order to call ctor with name
      TPluginHandler* ph = KVBase::LoadPlugin("KVVarGlob", class_name);
      if (!ph) {
         // define plugin handler for known class
         gPluginMgr->AddHandler("KVVarGlob", class_name, class_name, "KVMultiDetglobvars", Form("%s(const char*)", class_name));
         ph = KVBase::LoadPlugin("KVVarGlob", class_name);
      }
      vg = (KVVarGlob*) ph->ExecPlugin(1, name);
   }
   return vg;
}

KVVarGlob* KVGVList::AddGVFirst(const Char_t* class_name, const Char_t* name)
{
   // Add a global variable at the beginning of the list.
   //
   // See AddGV() for details.

   KVVarGlob* vg = prepareGVforAdding(class_name, name);
   if (vg) AddFirst(vg);
   return vg;
}
