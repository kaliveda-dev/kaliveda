\defgroup GlobalVariables Global Variables
\brief Classes for semi-automatic calculation of global variables in event analysis

### Global variables

A global variable is an analysis tool for condensing the information in a multibody event into one or a few
characteristic values. A simple example is the event multiplicity (the number of particles in each event),
which can be used to characterize heavy-ion collision events in terms of violence or centrality.

In KaliVeda, the base class for a multibody event is KVEvent, which is basically a collection of
nuclei (base class KVNucleus). Therefore the global variable classes below can be used with any
event described by a class derived from KVEvent, containing particles described by a class which
inherits from KVNucleus.

Global variable objects, derived from KVVarGlob, can be used in the following schematic way:

#### Creation & initialisation

~~~~~~~~~~~~{.cpp}
SomeVarGlob VG("var1");   // KVVarGlob-derived class implementing 1-body global variable

VG.Init();                // perform any necessary initialisations
~~~~~~~~~~~~

#### Analysis loop

~~~~~~~~~~~~{.cpp}
while( [loop over events] )
{
   while( [loop over particles in event] )
   {
      VG.Fill( [particle] ); // calculate contribution of particle to variable
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

### Global variable lists
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

      auto valueOfvar1 = VGlist.GetGV("var1")->GetValue(); // retrieve value of "var1" for event
~~~~~~~~~~~~

The KVGVList::CalculateGlobalVariables() method is optimised to ensure that all one- and two-body variables
are calculated with a single loop over the particles in each event. See KVGVList for more details.

 See also:

  - [The Chapter about Global Variables in the Users Guide](http://indra.in2p3.fr/kaliveda/UsersGuide/globalvariables.html)
