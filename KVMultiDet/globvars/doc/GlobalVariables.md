\defgroup GlobalVariables Global Variables
\brief Classes for semi-automatic calculation of global variables in event analysis

This class is a base class for the management of global variables.
A global variable is an analysis tool for condensing the information in a multibody event into one or a few
characteristic values. A simple example is the event multiplicity (the number of particles in each event),
which can be used to characterize heavy-ion collision events in terms of violence or centrality.

In KaliVeda, the base class for a multibody event is KVEvent, which is basically a collection of
nuclei (base class KVNucleus). Therefore the global variable classes below can be used with any
event described by a class derived from KVEvent, containing particles described by a class which
inherits from KVNucleus.

Global variable objects are used in the following schematic way:

#### Creation & initialisation:

~~~~~~~~~~~~{.cpp}
      KVVarGlob VG;
      VG.Init(); // perform any necessary initialisations
~~~~~~~~~~~~

#### Treatment of 1 event:

~~~~~~~~~~~~{.cpp}
      VG.Reset(); // reinitialise prior to analysis
      while( [loop over particles in event] ){

          VG.Fill( [particle] ); // calculate contribution of particle to variable
      }
      VG.Calculate();  // perform any necessary calculations
      Double_t valueOfVG = VG.GetValue(); // retrieve value of global variable for event
~~~~~~~~~~~~

Global variables can be of different types:

 - One-body global variable  (type = `KVVarGlob::kOneBody`)
    - the variable is computed by performing a loop over all particles in an event
      and calling the overridden fill(KVNucleus*) method for each particle in turn.
 - Two-body global variable  (type = `KVVarGlob::kTwoBody`)
    - the variable is computed by performing a loop over all pairs of particles in an event
      and calling the overridden fill2(KVNucleus*,KVNucleus*) method for each pair in turn.
 - N-body global variable (type = `KVVarGlob::kNBody`)
    - the variable is computed from the full list of particles of the event, by defining
      the overridden fillN(KVEvent*) method.

Derived global variable classes of 2-body or N-body type must set the `fType` member variable
to the appropriate type (`kTwoBody` or `kNBody`) and define the fill2(KVNucleus*,KVNucleus*)
method (for 2-body variables) or the fillN(KVEvent*) method (for N-body variables).

This is handled semi-automatically when using method

~~~~~~~~~~~~{.cpp}
     MakeClass(const Char_t * classname, const Char_t * classdesc, int type)
~~~~~~~~~~~~

to generate a skeleton '.h' and '.cpp' file for the implementation of a new global variable class.

By default, global variables are 1-body and must define the fill(KVNucleus*) method.

## Global variable lists
The KVGVList class handles a list of global variables. A list can be used in the following
schematic way to calculate several global variables at once:

#### Creation & initialisation

~~~~~~~~~~~~{.cpp}
      KVVGList VGlist;
      VGlist.Add( new KV...("var1") ); // add variable
      VGlist.Add( new KV...("var2") ); // add variable
      VGlist.Add( new KV...("var3") ); // add variable
      ...
      VGlist.Init(); // initialise all variables
~~~~~~~~~~~~

#### Treatment of 1 event

~~~~~~~~~~~~{.cpp}
      VGlist.CalculateGlobalVariables( [event] ); // calculate contribution of each particle to each variable
      Double_t valueOfvar1 = VGlist.GetGV("var1")->GetValue(); // retrieve value of "var1" for event
~~~~~~~~~~~~

## Options, parameters, reference frames, particle selection, etc.
### Particle selection
The selection of particles which are taken into account can be handled by the variable
itself. Define a selection using class KVParticleCondition and then set it by calling
method SetSelection(KVParticleCondition&).

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

### Kinematical reference frames
For global variables which use kinematical properties of particles, it can be useful to be
able to calculate the same variable in different Lorentz reference frames (see KVParticle::SetFrame
and KVEvent::SetFrame for how to define and access different frames). Therefore we
provide the methods

~~~~~~~~~~~{.cpp}
     void           SetFrame(const Char_t*)
     const Char_t*  GetFrame()
~~~~~~~~~~~

which allow to change the reference frame used for the calculation of the variable
(depending on the implementation of the specific class).

 See also:

  - [The Chapter about the Global Variables in the Users Guide](http://indra.in2p3.fr/kaliveda/UsersGuide/globalvariables.html)
