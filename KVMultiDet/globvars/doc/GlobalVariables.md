\defgroup GlobalVariables Global Variables
\brief Classes for semi-automatic calculation of global variables in event analysis

### Global variables

A global variable is an analysis tool for condensing the information in a multibody event into one or a few
characteristic values. A simple example is the event multiplicity (the number of particles in each event),
which can be used to characterize heavy-ion collision events in terms of violence or centrality.

In KaliVeda, the base class for a multibody event is KVEvent, which is basically a collection of
nuclei (base class KVNucleus). Therefore the global variable classes can be used with any
event described by a class derived from KVEvent, containing particles described by a class which
inherits from KVNucleus.

For more information, see KVVarGlob and KVGVList.

 See also:

  - [The Chapter about Global Variables in the Users Guide](http://indra.in2p3.fr/kaliveda/UsersGuide/globalvariables.html)
