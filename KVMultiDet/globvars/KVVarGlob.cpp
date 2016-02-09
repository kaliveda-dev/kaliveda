//
//Author: Daniel Cussol
//
// 16/02/2004:
// Creation d'une classe Variable Globale pour KaliVeda.
// Elle est utilisee dans les analyses ROOT.
//

#include "KVVarGlob.h"
#include "KVClassFactory.h"
#include "TObjString.h"

ClassImp(KVVarGlob)
/////////////////////////////////////////////////////////////////////////////////
/*
BEGIN_HTML<h1>Global variables base class</h1>END_HTML

This class is a base class for the management of global variables.
A global variable is an analysis tool for condensing the information in a multibody event into one or a few
characteristic values. A simple example is the event multiplicity (the number of particles in each event),
which can be used to characterize heavy-ion collision events in terms of violence or centrality.

In KaliVeda, the base class for a multibody event is KVEvent, which is basically a collection of
nuclei (base class KVNucleus). Therefore the global variable classes below can be used with any
event described by a class derived from KVEvent, containing particles described by a class which
inherits from KVNucleus.

Global variable objects are used in the following schematic way:

creation & initialisation:

      KVVarGlob* VG = new KV...;
      VG->Init(); // perform any necessary initialisations

treatment of 1 event:

      VG->Reset(); // reinitialise prior to analysis
      while( [loop over particles in event] ){

          VG->Fill( [particle] ); // calculate contribution of particle to variable
      }
      Double_t valueOfVG = VG->GetValue(); // retrieve value of global variable for event

Global variables can be of different types:

     One-body global variable     (type = KVVarGlob::kOneBody)
        - the variable is computed by performing a loop over all particles in an event
          and calling the overridden Fill(KVNucleus*) method for each particle in turn.
     Two-body global variable     (type = KVVarGlob::kTwoBody)
        - the variable is computed by performing a loop over all pairs of particles in an event
          and calling the overridden Fill2(KVNucleus*,KVNucleus*) method for each pair in turn.
     N-body global variable     (type = KVVarGlob::kNBody)
        - the variable is computed from the full list of particles of the event, by defining
          the overridden FillN(KVEvent*) method.

Derived global variable classes of 2-body or N-body type must set the fType member variable
to the appropriate type (kTwoBody or kNBody) and define the Fill2(KVNucleus*,KVNucleus*)
method (for 2-body variables) or the FillN(KVEvent*) method (for N-body variables).
This is handled semi-automatically when using method
   MakeClass(const Char_t * classname, const Char_t * classdesc, int type)
to generate a skeleton '.h' and '.cpp' file for the implementation of a new global variable class.
By default, global variables are 1-body and must define the Fill(KVNucleus*) method.

begin_html<h2>Global variable lists</h2>end_html
The KVGVList class handles a list of global variables. A list can be used in the following
schematic way to calculate several global variables at once:

creation & initialisation:

      KVVGList* VGlist = new KVGVList;
      VGlist->Add( new KV...("var1") ); // add variable
      VGlist->Add( new KV...("var2") ); // add variable
      VGlist->Add( new KV...("var3") ); // add variable
      ...
      VGlist->Init(); // initialise all variables

treatment of 1 event:

      VGlist->Reset(); // reinitialise all variables prior to analysis
      while( [loop over particles in event] ){

          VGlist->Fill( [particle] ); // calculate contribution of particle to each variable
      }
      Double_t valueOfvar1 = VGlist->GetGV("var1")->GetValue(); // retrieve value of "var1" for event

begin_html<h2>Options, parameters, reference frames, particle selection, etc.</h2>
<h3>Particle selection</h3>end_html
The selection of particles which are taken into account can be handled by the variable
itself. Define a selection using class KVParticleCondition and then set it by calling
method SetSelection(KVParticleCondition&). Then, in your treatment loop for each event,
instead of calling the Fill(KVNucleus*) method, call FillWithCondition(KVNucleus*).

If you have a list of global variable objects (KVGVList), the KVGVList::Fill method
automatically calls FillWithCondition for each KVVarGlob in the list (variables for which
no particle selection is defined include all particles).

begin_html<h3>Options and parameters</h3>end_html
In order to give greater flexibility to global variable classes without the need to add
member variables and the associated Get/Set methods, we provide methods to handle
generic 'options' and 'parameters' for all variables.

An 'option' is a name-value pair, the value is a character string. Methods to use are:

   void      SetOption(const Char_t* option, const Char_t* value)
   Bool_t    IsOptionGiven(const Char_t* option)
   KVString& GetOptionString(const Char_t* option) const
   void      UnsetOption(const Char_t* opt)

A 'parameter' is a name-value pair, the value is a double-precision float value. Methods to use are:

   void     SetParameter(const Char_t* par, Double_t value)
   Bool_t   IsParameterSet(const Char_t* par)
   Double_t GetParameter(const Char_t* par)
   void     UnsetParameter(const Char_t* par)

begin_html<h3>Kinematical reference frames</h3>end_html
For global variables which use kinematical properties of particles, it can be useful to be
able to calculate the same variable in different Lorentz reference frames (see KVParticle::SetFrame
and KVEvent::SetFrame for how to define and access different frames). Therefore we
provide the methods

   void           SetFrame(const Char_t*)
   const Char_t*  GetFrame()

which allow to change the reference frame used for the calculation of the variable
(depending on the implementation of the specific class).
*/
////////////////////////////////////////////////////////////////////////////////////

Int_t KVVarGlob::nb = 0;
Int_t KVVarGlob::nb_crea = 0;
Int_t KVVarGlob::nb_dest = 0;

//_________________________________________________________________
void KVVarGlob::init(void)
{
//
// Initialisation des champs de KVVarGlob
// Cette methode privee n'est appelee que par les createurs
//
   nb++;
   nb_crea++;
   nameList.Clear();
   fSelection = 0;
   // one-body variable by default
   fType = kOneBody;
   fValueType = 'D';
   fMaxNumBranches = -1;
}

//_________________________________________________________________
KVVarGlob::KVVarGlob(void): KVBase()
{
//
// Createur par default
//
   Char_t* nom = new Char_t[80];

   init();
   sprintf(nom, "KVVarGlob_%d", nb_crea);
   SetName(nom);
   SetTitle(nom);
#ifdef DEBUG_KVVarGlob
   cout << nb << " crees...(defaut) " << endl;
#endif
   delete[]nom;
}

//_________________________________________________________________
KVVarGlob::KVVarGlob(const Char_t* nom): KVBase(nom)
{
//
// Constructeur avec un nom
//
   init();
#ifdef DEBUG_KVVarGlob
   cout << nb << " crees...(nom) " << endl;
#endif
}

//_________________________________________________________________
KVVarGlob::KVVarGlob(const KVVarGlob& a) : KVBase()
{
// Contructeur par copie

   init();
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   a.Copy(*this);
#else
   ((KVVarGlob&) a).Copy(*this);
#endif
#ifdef DEBUG_KVVarGlob
   cout << nb << " crees...(copie) " << endl;
#endif
}

//_________________________________________________________________
KVVarGlob::~KVVarGlob(void)
{
//
// Destructeur
//
#ifdef DEBUG_KVVarGlob
   cout << "Destruction de " << GetName() << "..." << endl;
#endif
   nb--;

   nb_dest++;
   SafeDelete(fSelection);
}


//_________________________________________________________________
KVVarGlob& KVVarGlob::operator =(const KVVarGlob& a)
{
//
// Operateur =
//
#ifdef DEBUG_KVVarGlob
   cout << "Copie par egalite de " << a.GetName() << "..." << endl;
#endif
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   a.Copy(*this);
#else
   ((KVVarGlob&) a).Copy(*this);
#endif
#ifdef DEBUG_KVVarGlob
   cout << "Nom de la copie par egalite: " << GetName() << endl;
#endif
   return *this;
}

//_________________________________________________________________
TObject* KVVarGlob::GetObject(void) const
{
   //
   // Retourne un object
   //
   Warning("GetObject", "Not implemented");
   return 0;
}

//_________________________________________________________________
void KVVarGlob::MakeClass(const Char_t* classname, const Char_t* classdesc, int type)
{
   // Creates skeleton '.h' and '.cpp' files for a new global variable class which
   // inherits from this class. Give a name for the new class and a short description
   // which will be used to document the class.
   //
   // By default the new class will be of type 1-body. A Fill(KVNucleus*) method
   // will be generated which the user should complete.
   // For a 2-body variable, call MakeClass with type = KVVarGlob::kTwoBody.
   // A skeleton Fill2(KVNucleus*,KVNucleus*) method will be generated.
   // For a N-body variable, call MakeClass with type = KVVarGlob::kNBody.
   // A skeleton FillN(KVEvent*) method will be generated.

   // basic class template
   KVClassFactory cf(classname, classdesc, "KVVarGlob", kTRUE);

   KVString body;

   // add 'init' method
   KVVarGlob::AddInitMethod(classname, cf, body, type);

   // add 'Fill', 'Fill2', or 'FillN' method
   KVVarGlob::AddFillMethod(cf, type);

   // body of 'Fill', 'Fill2', or 'FillN' method
   KVVarGlob::FillMethodBody(body, type);

   // add body of method
   KVVarGlob::AddFillMethodBody(cf, body, type);

   cf.GenerateCode();
}

void KVVarGlob::FillMethodBody(KVString& body, int type)
{
   // PRIVATE method used by MakeClass.
   // body of 'Fill', 'Fill2', or 'FillN' method
   switch (type) {
      case kTwoBody:
         body = "   // Calculation of contribution to 2-body global variable of pair (n1,n2) of nuclei.\n";
         body += "   // NOTE: this method will be called for EVERY pair of nuclei in the event\n";
         body += "   // (i.e. n1-n2 and n2-n1), including pairs of identical nuclei (n1 = n2).\n";
         body += "   // If you want to calculate a global variable using only each non-identical pair once,\n";
         body += "   // then make sure in your implementation that you check n1!=n2 and divide the\n";
         body += "   // contribution to any sum by 2 to avoid double-counting.\n";
         break;
      case kNBody:
         body = "   // Calculation of contribution to N-body global variable of particles in event e.\n";
         break;
      default:
         body = "   // Calculation of contribution to 1-body global variable of nucleus n\n";
   }
}

void KVVarGlob::AddInitMethod(const Char_t* classname, KVClassFactory& cf, KVString& body, int type)
{
   // PRIVATE method used by MakeClass.
   // add 'init' method
   cf.AddMethod(Form("init_%s", classname), "void", "private");
   body = "   // Private initialisation method called by all constructors.\n";
   body += "   // All member initialisations should be done here.\n";
   body += "   //\n";
   body += "   // You should also (if your variable calculates several different quantities)\n";
   body += "   // set up a correspondance between named values and index number\n";
   body += "   // using method SetNameIndex(const Char_t*,Int_t)\n";
   body += "   // in order for GetValue(const Char_t*) to work correctly.\n";
   body += "   // The index numbers should be the same as in your getvalue_int(Int_t) method.\n";
   body += "\n";
   switch (type) {
      case kTwoBody:
         body += "   fType = KVVarGlob::kTwoBody; // this is a 2-body variable\n";
         break;
      case kNBody:
         body += "   fType = KVVarGlob::kNBody; // this is a N-body variable\n";
         break;
      default:
         body += "   fType = KVVarGlob::kOneBody; // this is a 1-body variable\n";
   }
   cf.AddMethodBody(Form("init_%s", classname), body);
}

void KVVarGlob::AddFillMethod(KVClassFactory& cf, int type)
{
   // PRIVATE method used by MakeClass.
   // add 'Fill', 'Fill2', or 'FillN' method
   switch (type) {
      case kTwoBody:
         cf.AddMethod("Fill2", "void");
         cf.AddMethodArgument("Fill2", "KVNucleus*", "n1");
         cf.AddMethodArgument("Fill2", "KVNucleus*", "n2");
         break;
      case kNBody:
         cf.AddMethod("FillN", "void");
         cf.AddMethodArgument("FillN", "KVEvent*", "e");
         cf.AddHeaderIncludeFile("KVEvent.h");
         break;
      default:
         cf.AddMethod("Fill", "void");
         cf.AddMethodArgument("Fill", "KVNucleus*", "n");
   }
}

void KVVarGlob::AddFillMethodBody(KVClassFactory& cf, KVString& body, int type)
{
   // PRIVATE method used by MakeClass.
   // add body of fill method
   switch (type) {
      case kTwoBody:
         cf.AddMethodBody("Fill2", body);
         break;
      case kNBody:
         cf.AddMethodBody("FillN", body);
         break;
      default:
         cf.AddMethodBody("Fill", body);
   }
}

//_________________________________________________________________
void KVVarGlob::SetNameIndex(const Char_t* name, Int_t index)
{
   // Make the link between a variable name and an index
   if (!(nameList.HasParameter(name))) {
      nameList.SetValue(name, index);
   } else {
      Warning("SetNameIndex(const Char_t *name,Int_t index)",
              "No link between \"%s\" and the index %d: the name already exist.",
              name, index);
   }
}

//_________________________________________________________________
Int_t KVVarGlob::GetNameIndex(const Char_t* name)
{
   // return the index corresponding to name
   Int_t index = 0;
   if (nameList.HasParameter(name)) {
      index = nameList.GetIntValue(name);
   } else {
      Warning("GetNameIndex(const Char_t *name)",
              "The parameter \"%s\" does not exist fot the Class %s.\n 0 returned.",
              name, ClassName());

   }
   return index;
}

//_________________________________________________________________

void KVVarGlob::SetFrame(const Char_t* ref)
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

//_________________________________________________________________

void KVVarGlob::SetOption(const Char_t* option, const Char_t* value)
{
   //Set a value for an option
   KVString tmp(value);
   fOptions.SetValue(option, tmp);
}

//_________________________________________________________________

Bool_t KVVarGlob::IsOptionGiven(const Char_t* opt)
{
   //Returns kTRUE if the option 'opt' has been set

   return fOptions.HasParameter(opt);
}

//_________________________________________________________________

const TString& KVVarGlob::GetOptionString(const Char_t* opt) const
{
   //Returns the value of the option

   return fOptions.GetTStringValue(opt);
}

//_________________________________________________________________

void KVVarGlob::UnsetOption(const Char_t* opt)
{
   //Removes the option 'opt' from the internal lists, as if it had never been set

   fOptions.RemoveParameter(opt);
}

//_________________________________________________________________

void KVVarGlob::SetParameter(const Char_t* par, Double_t value)
{
   //Set the value for a parameter
   fParameters.SetValue(par, value);
}

//_________________________________________________________________

Bool_t KVVarGlob::IsParameterSet(const Char_t* par)
{
   //Returns kTRUE if the parameter 'par' has been set

   return fParameters.HasParameter(par);
}

//_________________________________________________________________

Double_t KVVarGlob::GetParameter(const Char_t* par)
{
   //Returns the value of the parameter 'par'

   return fParameters.GetDoubleValue(par);
}

//_________________________________________________________________

void KVVarGlob::UnsetParameter(const Char_t* par)
{
   //Removes the parameter 'par' from the internal lists, as if it had never been set

   fParameters.RemoveParameter(par);
}

//_________________________________________________________________

void KVVarGlob::SetSelection(const KVParticleCondition& sel)
{
   //Use this method to define the conditions which will be applied to select
   //particles to contribute to the global variable.
   //See KVParticleCondition class.
   if (!fSelection) fSelection = new KVParticleCondition(sel);
   else *fSelection = sel;
}

//_________________________________________________________________

void KVVarGlob::Fill(KVNucleus*)
{
   // virtual method which must be overriden in child classes
   // describing one-body global variables.
   AbstractMethod("Fill(KVNucleus*)");
}

//_________________________________________________________________

void KVVarGlob::Fill2(KVNucleus*, KVNucleus*)
{
   // virtual method which must be overriden in child classes
   // describing two-body global variables.
   //
   // NOTE: this method will be called for EVERY pair of nuclei in the event
   // (i.e. n1-n2 and n2-n1), including pairs of identical nuclei (n1 = n2).
   // If you want to calculate a global variable using only each non-identical pair once,
   // then make sure in your implementation that you check n1!=n2 and divide
   // the result of summing over the pairs by 2 to avoid double-counting.
   AbstractMethod("Fill2(KVNucleus*,KVNucleus*)");
}

//_________________________________________________________________

void KVVarGlob::FillN(KVEvent*)
{
   // virtual method which must be overriden in child classes
   // describing N-body global variables.
   AbstractMethod("FillN(KVEvent*)");
}
