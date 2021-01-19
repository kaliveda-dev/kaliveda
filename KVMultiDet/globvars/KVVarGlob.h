#ifndef KVVarGlob_h
#define KVVarGlob_h
#include "Riostream.h"
#include "KVBase.h"
#include "KVNucleus.h"
#include "KVNameValueList.h"
#include "KVString.h"
#include "KVParticleCondition.h"

class KVEvent;

/**
  \class KVVarGlob
  \ingroup GlobalVariables
  \brief Base class for all global variable implementations

Abstract base class for the management of global variables.
A global variable is an analysis tool for condensing the information in a multibody event into one or a few
characteristic values. A simple example is the event multiplicity (the number of particles in each event),
which can be used to characterize heavy-ion collision events in terms of violence or centrality.

In KaliVeda, the base class for a multibody event is KVEvent, which is basically a collection of
nuclei (base class KVNucleus). Therefore the global variable classes below can be used with any
event described by a class derived from KVEvent, containing particles described by a class which
inherits from KVNucleus.

Each global variable class implements the calculation of a given observable for any set of nuclei;
this is done independently of the selection of the nuclei in the set, or in what reference frame
kinematics should be calculated for either selection purposes or for the calculation of the observable.
The way the observable is calculated is defined by the implementation of the fill(const KVNucleus*)
(for 1-body observables: see below) and Calculate() methods.

Global variables can be of different types:

 - One-body global variable  (type = `KVVarGlob::kOneBody`)
    - the variable is computed by performing a loop over all particles in an event
      and calling the overridden fill(const KVNucleus*) method for each particle in turn.
 - Two-body global variable  (type = `KVVarGlob::kTwoBody`)
    - the variable is computed by performing a loop over all pairs of particles in an event
      and calling the overridden fill2(const KVNucleus*,const KVNucleus*) method for each pair in turn.
 - N-body global variable (type = `KVVarGlob::kNBody`)
    - the variable is computed from the full list of particles of the event, by defining
      the overridden fillN(KVEvent*) method.

Derived global variable classes of 2-body or N-body type must set the `fType` member variable
to the appropriate type (`kTwoBody` or `kNBody`) and define the fill2(const KVNucleus*,const KVNucleus*)
method (for 2-body variables) or the fillN(KVEvent*) method (for N-body variables).

This is handled semi-automatically when using method

~~~~~~~~~~~~{.cpp}
     MakeClass(const Char_t * classname, const Char_t * classdesc, int type)
~~~~~~~~~~~~

to generate a skeleton '.h' and '.cpp' file for the implementation of a new global variable class.

By default, global variables are 1-body and must define the fill(const KVNucleus*) method.

In addition, implementations in daughter classes *must* define the following methods:
 - getvalue_int(int) : return (possibly) several values calculated by the global variable,
   depending on the index.
 - Init() : initialisation of any internal variables to be performed once before beginning analysis loop.
 - Calculate() : perform any necessary calculations after filling for 1 event is finished.
 - Reset() : reset internal variables ready for another event.

### Usage

#### Creation & initialisation

~~~~~~~~~~~~{.cpp}
SomeVarGlob VG("var1");   // daughter class implementing 1-body global variable

// if required:
VG.SetSelection( [particle selection criteria] );
VG.SetFrame( [reference frame for kinematics] );

VG.Init();                // perform any necessary initialisations
~~~~~~~~~~~~

#### Analysis loop

~~~~~~~~~~~~{.cpp}
while( [loop over events] )
{
   while( [loop over particles in event] )
   {
      // calculate contribution of particle to variable, only if it satisfies
      // the selection criteria set with method SetSelection() (if any)
      VG.Fill( [particle] );
   }

   VG.Calculate();  // perform any necessary calculations

   // accessing the result
   auto vg = VG.GetValue();       // retrieve unique or principal value of variable
   auto vh = VG.GetValue(2);      // retrieve value with index 2 from multi-valued variable (implementation-dependent)
   auto vi = VG.GetValue("Toto"); // retrieve value named "Toto" from multi-valued variable (implementation-dependent)
   auto vv = VG.GetValueVector(); // retrieve vector containing all values of multi-valued variable
   for(auto V : vv)               // print all values
   {
      std::cout << V << std::endl;
   }

   VG.Reset(); // reinitialise prior to analysis of next event
}
~~~~~~~~~~~~

Note that although the Fill() method is called for all particles, only those which satsify the conditions
given to SetSelection() will be used to calculate the variable. Also, the internal fill(const KVNucleus*)
etc. methods of each global variable class actually receive a pointer to each nucleus with as default
kinematics those of the frame chosen with SetFrame().

### Global variable lists
The KVGVList class handles a list of global variables. The recommended way to use global variables
is through a list of this type (even if only one). The user analysis classes derived from KVEventSelector
all have an internal KVGVList for definition and calculation of global variables. See KVGVList and KVEventSelector class
documentation for more details.

## Options, parameters, reference frames, particle selection, etc.
### Particle selection
The selection of particles which are taken into account is handled by the variable
itself by calling method SetSelection().

### Kinematical reference frames
To change the reference frame used by the variable to calculate kinematical properties of particles
(including those used for particle selection), call method SetFrame()
(see KVEvent::SetFrame() and KVParticle::GetFrame() for how to define and access different frames).

### Options and parameters
In order to give greater flexibility to global variable classes without the need to add
member variables and the associated Get/Set methods, we provide methods to handle
generic 'options' and 'parameters' for all variables.

An 'option' is a name-value pair, the value is a character string. Methods to use are:

~~~~~~~~~~~{.cpp}
     void      SetOption(const Char_t* option, const Char_t* value)
     Bool_t    IsOptionGiven(const Char_t* option)
     KVString& GetOptionString(const Char_t* option) const
     void      UnsetOption(const Char_t* opt)
~~~~~~~~~~~

A 'parameter' is a name-value pair, the value is a double-precision float value. Methods to use are:

~~~~~~~~~~~{.cpp}
     void     SetParameter(const Char_t* par, Double_t value)
     Bool_t   IsParameterSet(const Char_t* par)
     Double_t GetParameter(const Char_t* par)
     void     UnsetParameter(const Char_t* par)
~~~~~~~~~~~

### Normalization

The value(s) returned by the variable will all be divided by the normalization factor set with one
of the following methods:
~~~~{.cpp}
SetNormalization(Double_t)
SetParameter("Normalization", Double_t)
~~~~

### Definition of event selection criteria [ROOT6]
When used in a KVGVList of global variables, conditions ('cuts') can be set on each variable which
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

This mechanism is implemented in KVEventSelector, i.e. in all user analysis classes.

### Definition of new kinematical frames [ROOT6]
When used in a KVGVList of global variables, a variable can be used to define a new kinematical frame which can in turn be
used by any variables which occur after them in the list. In order to do so, call method SetNewFrameDefinition() with
a lambda function having the following signature:
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
    vg = vglist.AddGV("KVFlowTensor", "qp_tensor");
    vg->SetFrame("QP_FRAME"); // frame will have been defined before tensor is filled
~~~~

\authors D. Cussol (LPC Caen), J.D. Frankland (GANIL)
\date 2004-2020
 */

class KVVarGlob: public KVBase {

public:
   enum {
      kOneBody, // one-body variable: Fill (KVNucleus*) must be defined
      kTwoBody, // two-body variable: Fill2 (KVNucleus*, KVNucleus*) must be defined
      kNBody      // N-body variable: FillN (KVEvent*) must be defined
   };

protected:
   Int_t fType;   // type of variable global; = kOneBody, kTwoBody or kNBody
   Char_t fValueType; // type (='I' integer or 'D' double) of global variable value
private:
   KVNameValueList nameList;//correspondence between variable name and index
   Bool_t fIsInitialized;//! flag set after initialisation
   KVString fFrame;//(optional) name of reference frame used for kinematics
   KVNameValueList fOptions;//list of options
   KVNameValueList fParameters;//list of parameters
   KVParticleCondition fSelection;//(optional) condition used to select particles
   Int_t fMaxNumBranches;// max number of branches to create for multi-valued variable
   Double_t fNormalization;// optional normalization parameter

#ifdef USING_ROOT6
   using EventSelector = std::function<bool(const KVVarGlob*)>;
   EventSelector fEventSelector;// used to select events in analysis based on value of variable
   using FrameSetter = std::function<void(KVEvent*, const KVVarGlob*)>;
   FrameSetter fFrameSetter;// used to define a new kinematical frame for event based on variable
#endif

   void init();
   static void AddExtraInitMethodComment(KVClassFactory& cf, KVString& body);

protected:
   void SetNameIndex(const Char_t* name, Int_t index);
   void ClearNameIndex()
   {
      // Delete previously defined associations between variable name and index
      nameList.Clear();
   }

   static void FillMethodBody(KVString& body, int type);
   static void AddInitMethod(KVClassFactory& cf, KVString& body);
   static void ImplementInitMethod(KVClassFactory& cf, KVString& body, int type);
   static void AddFillMethod(KVClassFactory& cf, int type);
   static void AddFillMethodBody(KVClassFactory& cf, KVString& body, int type);

   virtual Double_t getvalue_void() const
   {
      // By default, returns value with index 0
      return getvalue_int(0);
   }
   Double_t getvalue_char(const Char_t* name) const
   {
      // By default, this method returns the value of the variable "name"
      // using the name-index table set up with SetNameIndex(const Char_t*,Int_t).

      return getvalue_int(GetNameIndex(name));
   }
   virtual Double_t getvalue_int(Int_t) const = 0;
   virtual void fill(const KVNucleus*)
   {
      // abstract method which must be overriden in child classes
      // describing one-body global variables.
      AbstractMethod("fill(KVNucleus*)");
   }

   virtual void fill2(const KVNucleus*, const KVNucleus*)
   {
      // abstract method which must be overriden in child classes
      // describing two-body global variables.
      //
      // NOTE: this method will be called for EVERY pair of nuclei in the event
      // (i.e. n1-n2 and n2-n1), including pairs of identical nuclei (n1 = n2).
      // If you want to calculate a global variable using only each non-identical pair once,
      // then make sure in your implementation that you check n1!=n2 and divide
      // the result of summing over the pairs by 2 to avoid double-counting.
      AbstractMethod("fill2(KVNucleus*,KVNucleus*)");
   }

   KVNameValueList& GetParameters()
   {
      return fParameters;
   }
   const KVNameValueList& GetParameters() const
   {
      return fParameters;
   }

public:
   KVVarGlob()
      : KVBase("KVVarGlob", "KVVarGlob"), nameList("IndexList", "Correspondance variable name<->index")
   {
      init();
   }
   KVVarGlob(const Char_t* nom)
      : KVBase(nom, nom), nameList("IndexList", "Correspondance variable name<->index")
   {
      init();
   }
   ROOT_COPY_CTOR(KVVarGlob, KVBase)
   ROOT_COPY_ASSIGN_OP(KVVarGlob)
   void Copy(TObject& obj) const
   {
      // Copy this to obj
      KVBase::Copy(obj);
      KVVarGlob& vgobj = dynamic_cast<KVVarGlob&>(obj);
      nameList.Copy(vgobj.nameList);
      vgobj.fIsInitialized = fIsInitialized;
      vgobj.fFrame = fFrame;
      fOptions.Copy(vgobj.fOptions);
      fParameters.Copy(vgobj.fParameters);
      fSelection.Copy(vgobj.fSelection);
      vgobj.fType = fType;
      vgobj.fValueType = fValueType;
      vgobj.fMaxNumBranches = fMaxNumBranches;
      vgobj.fNormalization = fNormalization;
#ifdef USING_ROOT6
      vgobj.fEventSelector = fEventSelector;
      vgobj.fFrameSetter = fFrameSetter;
#endif
   }
   virtual ~KVVarGlob(void)
   {}

   Bool_t IsOneBody()
   {
      // \returns kTRUE for variables of one-body type for which Fill(KVNucleus*) method must be defined
      return fType == kOneBody;
   }

   Bool_t IsTwoBody()
   {
      // \returns kTRUE for variables of two-body type for which Fill2(KVNucleus*,KVNucleus*) method must be defined
      return fType == kTwoBody;
   }

   Bool_t IsNBody()
   {
      // \returns kTRUE for variables of N-body type for which FillN(KVEvent*) method must be defined
      return fType == kNBody;
   }

   void ListInit()
   {
      // Method called by KVGVList::Init
      // Ensures that initialisation of variable is performed only once

      if (!fIsInitialized) {
         Init();
         fIsInitialized = kTRUE;
      }
   }
   virtual void Init() = 0;
   virtual void Reset() = 0;
   virtual void Calculate() = 0;

   void Fill(const KVNucleus* c)
   {
      // Evaluate contribution of particle to variable only if it satisfies
      // the particle selection criteria given with SetSelection()/AddSelection(),
      // call fill() with particle in desired frame
      const KVNucleus* c_in_frame = dynamic_cast<const KVNucleus*>(c->GetFrame(fFrame, false));
      if (fSelection.Test(c_in_frame)) fill(c_in_frame);
   }
   void Fill2(const KVNucleus* n1, const KVNucleus* n2)
   {
      // Evaluate contribution of particles to variable only if both satisfy
      // the particle selection criteria given with SetSelection(KVParticleCondition&),
      // call fill() with particle in desired frame
      const KVNucleus* n1_in_frame = dynamic_cast<const KVNucleus*>(n1->GetFrame(fFrame, false));
      const KVNucleus* n2_in_frame = dynamic_cast<const KVNucleus*>(n2->GetFrame(fFrame, false));
      if (fSelection.Test(n1_in_frame) && fSelection.Test(n2_in_frame))
         fill2(n1_in_frame, n2_in_frame);
   }
   virtual void FillN(const KVEvent*)
   {
      // abstract method which must be overriden in child classes
      // describing N-body global variables.
      AbstractMethod("FillN(KVEvent*)");
   }
   Double_t GetValue(void) const
   {
      // \returns principal gobal variable value
      // To override behaviour of this method in child classes,
      // redefine the protected method getvalue_void()
      //
      // If a "Normalization" parameter has been set, it is applied here

      return getvalue_void() / fNormalization;
   }
   Double_t GetValue(const Char_t* name) const
   {
      // \return value of "name"
      // To override behaviour of this method in child classes,
      // redefine the protected method getvalue_char(const Char_t*)
      //
      // If a "Normalization" parameter has been set, it is applied here

      return getvalue_char(name) / fNormalization;
   }
   Double_t GetValue(Int_t i) const
   {
      // \return value with index i
      // To override behaviour of this method in child classes,
      // redefine the protected method getvalue_int(Int_t)
      //
      // If a "Normalization" parameter has been set, it is applied here

      return getvalue_int(i) / fNormalization;
   }
   virtual std::vector<Double_t> GetValueVector(void) const
   {
      // \return vector of all values calculated by variable. The order of the values in the vector is the
      // same as the indices defined by calls to SetNameIndex(), these indices correspond to those used with
      // GetValue(Int_t)
      //
      // If a "Normalization" parameter has been set, it is applied here

      std::vector<Double_t> tmp;
      for (int i = 0; i < GetNumberOfValues(); ++i) tmp.push_back(GetValue(i) / fNormalization);
      return tmp;
   }

   Double_t operator()(void) const
   {
      return GetValue();
   }

   Double_t operator()(const Char_t* name) const
   {
      return GetValue(name);
   }

   Double_t operator()(Int_t i) const
   {
      return GetValue(i);
   }

   virtual Int_t GetNameIndex(const Char_t* name) const;

   static void MakeClass(const Char_t* classname, const Char_t* classdesc, int type = kOneBody);

   void SetFrame(const Char_t* ref)
   {
      //Sets the reference frame used for kinematical calculations.
      //By default, i.e. if this method is not called, we use the default frame of particles
      //which (usually) corresponds to the 'laboratory' or 'detector' frame.
      //
      //The frame 'ref' must be defined before calculating global variables.
      //See KVParticle::SetFrame and KVEvent::SetFrame methods for defining new reference frames.
      //See KVParticle::GetFrame how to access particle kinematics in different frames.
      fFrame = ref;
   }
   const Char_t* GetFrame() const
   {
      return fFrame.Data();
   }

   void SetOption(const Char_t* option, const Char_t* value)
   {
      //Set a value for an option
      KVString tmp(value);
      fOptions.SetValue(option, tmp);
      fIsInitialized = kFALSE; //allow re-initialisation
   }

   Bool_t IsOptionGiven(const Char_t* opt)
   {
      //Returns kTRUE if the option 'opt' has been set

      return fOptions.HasParameter(opt);
   }

   TString GetOptionString(const Char_t* opt) const
   {
      //Returns the value of the option

      return fOptions.GetTStringValue(opt);
   }
   void UnsetOption(const Char_t* opt)
   {
      //Removes the option 'opt' from the internal lists, as if it had never been set

      fOptions.RemoveParameter(opt);
   }

   void SetParameter(const Char_t* par, Double_t value)
   {
      //Set the value for a parameter

      TString Par(par);
      // before v1.12, certain global variables used a parameter to select particles
      // force analysis to abort in case user has not updated an old analysis class
      if (Par == "Zmin" || Par == "Zmax" || Par == "Vmin" || Par == "Vmax")
         Fatal("SetParameter", "Particle selection for global variables must be defined using SetSelection(const KVParticleCondition&).\nUpdate the use of variable \"%s\" in your analysis.",
               GetName());
      if (Par == "Normalization") fNormalization = value;
      else fParameters.SetValue(par, value);
      fIsInitialized = kFALSE; //allow re-initialisation
   }

   void SetNormalization(Double_t norm)
   {
      fNormalization = norm;
   }

   Bool_t IsParameterSet(const Char_t* par)
   {
      //Returns kTRUE if the parameter 'par' has been set
      if (TString(par) == "Normalization") return (fNormalization != 1.0);
      return fParameters.HasParameter(par);
   }

   Double_t GetParameter(const Char_t* par) const
   {
      //Returns the value of the parameter 'par'
      if (TString(par) == "Normalization") return fNormalization;
      return fParameters.GetDoubleValue(par);
   }

   Double_t GetNormalization() const
   {
      return fNormalization;
   }

   void UnsetParameter(const Char_t* par)
   {
      //Removes the parameter 'par' from the internal lists, as if it had never been set

      if (TString(par) == "Normalization") fNormalization = 1.0;
      fParameters.RemoveParameter(par);
   }

   void SetSelection(const KVParticleCondition& sel)
   {
      // Use this method to define the condition(s) which will be applied to select
      // particles to contribute to the global variable.
      //
      // Any previously set conditions are replaced by this.
      //
      // \note if a reference frame is defined for this variable, it will automatically be applied
      // to the kinematics of the particle used for the selection. E.g. if the selection is "_NUC_->GetVpar()>0"
      // and the reference frame is "CM", the condition will be applied to the CM parallel velocities.
      //
      // \sa KVParticleCondition

      fSelection = sel;
   }

   void AddSelection(const KVParticleCondition& sel)
   {
      // Use this method to add a condition which will be applied to select
      // particles to contribute to the global variable.
      //
      // The final selection will be a logical 'AND' between any previously set
      // conditions and this one
      //
      // \sa SetSelection()

      fSelection &= sel;
   }

   Double_t AsDouble() const
   {
      return GetValue();
   }
   operator double() const
   {
      return AsDouble();
   }

   virtual Int_t GetNumberOfValues() const
   {
      // \returns number of values associated with this global variable.
      // This is the number of indices defined using SetNameIndex() method.
      return nameList.GetNpar();
   }
   Int_t GetNumberOfBranches() const
   {
      // Returns number of branches to create for this global variable (see KVGVList::MakeBranches).
      //
      // This is the same as GetNumberOfValues() unless SetMaxNumBranches() has been called with a different
      // (smaller) value.
      //
      // Note that if SetMaxNumBranches(0) is called, no branch will be created for this variable.
      return (fMaxNumBranches > -1 ? fMaxNumBranches : GetNumberOfValues());
   }
   TString GetValueName(Int_t i) const
   {
      // Returns name of value associated with index 'i',
      // as defined by using SetNameIndex method.
      for (int j = 0; j < GetNumberOfValues(); j++) {
         if (nameList.GetIntValue(j) == i) return nameList.GetParameter(j)->GetName();
      }
      return TString("unknown");
   }
   virtual Char_t GetValueType(Int_t) const
   {
      // Returns type of value associated with index i.
      //
      // This can be either 'I' (integer values) or 'D' (floating-point/double).
      // By default, this method returns the same type (value of member variable
      // fValueType) for all values of i.
      //
      // This can be overridden in child classes.

      return fValueType;
   }
   void SetMaxNumBranches(Int_t n)
   {
      // Used for automatic TTree branch creation for multi-valued variables
      // (see KVGVList::MakeBranches).
      //
      // Normally a branch will be created for each of the \f$N\f$ values declared
      // with the SetNameIndex() method, but if this method is called before
      // analysis begins with \f$n<N\f$, only the first \f$n\f$ branches will be used.
      //
      // Note that if SetMaxNumBranches(0) is called, no branch will
      // be created for this variable.
      fMaxNumBranches = (n <= GetNumberOfValues() ? n : -1);
   }
   void Print(Option_t* = "") const;

#ifdef USING_ROOT6
   void SetEventSelection(const EventSelector& f)
   {
      // Call this method with a lambda expression in order to define an event selection criterion
      // based on the value of this variable. The signature of the lambda is
      //~~~~{.cpp}
      // [](const KVVarGlob* var){ return var->GetValue()>20; }
      //~~~~
      // i.e. it evaluates to `bool` based on the value of the global variable passed to it.
      //
      // If capture by reference is used, the 'cut' can be defined later:
      //~~~~{.cpp}
      // double my_cut;
      // [&](const KVVarGlob* var){ return var->GetValue("Mean")<my_cut; }
      //~~~~
      //
      // The condition will be tested by KVGVList just after calculation of this variable;
      // if the condition is not met, no further variables will be calculated and the
      // event will be rejected for further analysis.
      fEventSelector = f;
   }
   bool TestEventSelection() const
   {
      // Called by KVGVList just after calculation of this variable;
      // if the condition is not met, no further variables will be calculated and the
      // event will be rejected for further analysis.

      return fEventSelector ? fEventSelector(this) : true;
   }
   void SetNewFrameDefinition(const FrameSetter& f)
   {
      // Call this method with a lambda expression in order to define a new frame for events
      // based on the calculated value(s) of this variable.
      //
      // The signature of the lambda is (the KVVarGlob pointer passed will be 'this')
      //~~~~{.cpp}
      // [](KVEvent* e, const KVVarGlob* vg){ e->SetFrame("_frame_name_", ...); }
      //~~~~
      fFrameSetter = f;
   }
   void DefineNewFrame(KVEvent* e) const
   {
      // If method SetFrameDefinition() was called with a valid function to define a new
      // kinematical frame for events, it will be used here to define the frame for all
      // particles in the event.
      if (fFrameSetter) fFrameSetter(e, this);
   }
#endif

   ClassDef(KVVarGlob, 7)      // Base class for global variables
};
#endif
