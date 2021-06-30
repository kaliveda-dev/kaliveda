\page release_notes Release Notes for KaliVeda

Last update: 30th June 2021

## Version 1.12/04 (current development version - dev branch)

__Changes 30/6/2021 in Build system__

**MAJOR** The minimum required version of ROOT is now 6.18 or later.

This is due to unconstrained use of C++11 syntax and later in KaliVeda classes, especially member variables
where the rootcint dictionary generator of ROOT 5 is not capable of handling the syntax.

__Changes 24/6/2021 in__ \ref GlobalVariables

The following improvements have been made:

  * All global variables can now 'tag' the particles which pass their selection criteria, by creating groups which, by
    default have the name of the variable (an alternative name can be given). This is handled by KVVarGlob::SetDefineGroup(),
    which can be used as in the following example:
    
~~~~{.cpp}
[in the InitAnalysis() method of some analysis class]
auto gv = AddGV("KVMult","alpha");
gv->SetSelection({"ALPHA", [](const KVNucleus* n){
                return dynamic_cast<const KVReconstructedNucleus*>(n)->IsAMeasured()
                 && n->IsIsotope(2,4);  // count isotopically identified alpha particles
                };
gv->SetDefineGroup();  // create group "alpha" for these particles

[in the Analysis() method of the class]
for(auto& n : ReconEventGroupIterator(GetEvent(), "alpha"))
{
    // loop over the isotopically identified alpha particles
}
~~~~

  * Note that it was previously stated in the class documentation for KVSource that the variable tagged each particle
    used to reconstruct the source: in fact this was not true, but has now been correctly implemented. The tagging is
    done by default, there is no need to call KVVarGlob::SetDefineGroup() for this variable (unless you want to change
    the default name used for the group).

  * It is now possible to classify events using mathematical expressions involving the different values of multi-valued
    global variables with KVEventClassifier. For example, for a KVSource global variable, which provides the values
    "Ex" and "A" (excitation energy and mass of the reconstructed source) it is now possible to
    sort events according to excitation energy per nucleon by doing the following:
    
~~~~{.cpp}
    KVGVList list_of_variables;
    list_of_variables.AddGV("KVSource", "QP");
    auto ec = list_of_variables.AddEventClassifier("QP", "Ex/A");
~~~~

__Changes 21/6/2021 in__ \ref NucEvents : Major improvements to handling of kinematical frames

Including:

  * _Bugfix_ : when a nucleus was accessed in a reference frame that was not its default frame,
  attributing a group to the particle would only affect the specific frame, and not the nucleus _per se_:
  this affected especially global variable classes whose KVVarGlob::fill() method acts on a pointer to
  the nucleus in the reference frame given by KVVarGlob::SetFrame() method. This has now been solved,
  so that all non-kinematical properties of particles (associated parameters, groups) are now the same
  in all reference frames.
  
  * It is now possible to access the default kinematics of a particle using any of its defined kinematical frames,
  using KVParticle::GetCurrentDefaultKinematics().

__Changes 29/5/2021 in__ \ref Simulation : __rotational energy in KVGemini__

In the KVGemini interface to the GEMINI++ statistical decay code, we now add systematically any rotational energy of
the nuclei to deexcite to their internal excitation energy (which normally only accounts for thermal excitation)
before calculating the decay. This is because GEMINI++ expects \f$E*=U+E_{rot}\f$ and subtracts \f$E_{rot}\f$
from \f$E*\f$ in order to calculate the thermal excitation energy \f$U\f$ available for decay. To ensure consistency
the \f$E_{rot}\f$ added by KVGemini is calculated according to the GEMINI++ prescription.

See methods KVGemini::DecayEvent() and KVGemini::DecaySingleNucleus().

__Changes 28/5/2021 in Build system__

**MAJOR** The minimum required version for cmake is now 3.5 (version by default in Ubuntu 16.04)

**MAJOR** C++11 support is enabled by default for all builds if it was not already enabled by ROOT. Note that although this now means that even when using ROOT5
auto variables, range-based for loops and lambda functions can be used, as C++11 support in the ROOT5 dictionary generator and interpreter is
very limited (or non-existent) many new features of KaliVeda reliant on C++11 are still only enabled when building with ROOT6.

__Changes 28/5/2021 in__ \ref NucEvents : __Templated event classes__

In the C++ standard library, containers are generally used to store values and objects whose type is defined at compile time
through template parameters: std::vector<int>, std::unordered_map<std::string, std::thread>, etc.

In ROOT (and in KaliVeda), containers such as TList handle pointers to objects which may be of any type
derived from TObject, and the actual type of the objects in any list is usually determined at runtime.

A great advantage of the standard containers is they provide iterators which make it possible to loop
very simply over their contents, with a range-based for loop:

~~~~{.cpp}
 std::vector<int> V;
 for(auto v : V) { std::cout << v << std::endl; }
~~~~

The type of the `auto v` variable is the type of the objects in the vector (`int` in this case).

STL-style iterators were already added to KVEvent some time ago to enable the use of range-based for loops with all event classes,
but they had the following small drawback:

~~~~{.cpp}
 KVReconstructedEvent R;
 for(auto& n :  R) { std::cout << n.GetIDCode() << std::endl; }
~~~~

This code would not compile, because although we would expect `n` to be of type `KVReconstructedNucleus&`, in fact for all
event classes the iterators returned base class references `KVNucleus&`.

This has now been resolved by realising that the event classes are in fact STL-style containers for particle/nucleus types:
a KVReconstructedEvent contains KVReconstructedNucleus objects, a KVSimEvent contains KVSimNucleus objects. Regarding the original
KVEvent base class for all events, it is now an abstract base class and KVEvent objects cannot be instantiated. An event of
KVNucleus objects is now called KVNucleusEvent - __this could be a major code-breaker__ but was necessary to ensure backwards
compatibility (still able to read existing data).

See \ref NucEvents for a full explanation of how to use the new event classes and iterators.

__Changes 3/5/2021 in__ \ref Analysis \ref Infrastructure : __New multicore "batch" system__

On multicore machines, PROOFLite can be used for analysis of any data contained in a TTree
(e.g. reconstructed data). However raw data is usually contained in some other type of file
and PROOFLite cannot be used. New "batch" system KVXtermBatch has been developed to remedy this.

When analysing or reconstructing raw data not contained in a TFile on a multicore machine,
KaliVedaGUI will automatically switch to the new batch system in order to exploit all cores of
the PC. When N runs are to be analysed on an M-core machine, this batch system will run
M jobs concurrently with the N runs shared as evenly as possible among the jobs.

## Version 1.12/03 (Released: 04/5/2021)

__Bugfixes__

  * correct bug in KVParticleCondition when using (old-style) pseudo-code (not lambdas);
  
  * fix bug in KVEvent::MakeEventBranch which could either explode or (silently) not write any events in a TTree;

  * fix (potential) bug with gamma multiplicities in reconstructed events.

## Versions 1.12/01 (Released: 19/2/2021) &  1.12/02 (Released: 01/4/2021)

__Changes 19/2/2021 in__ \ref Analysis : __Reusable analysis classes__

As part of ongoing efforts to make analysis classes more flexible and efficient, it is now possible to use the same analysis class to analyse several different types of data.
Any analysis derived from KVReconEventSelector (generic reconstructed event analysis class) can now be used:
 
  * analyse generic reconstructed data [this was already the case];
  
  * analyse reconstructed INDRA data [previously only possible with a specific class derived from KVINDRAEventSelector];
  
  * analyse filtered simulated data
  
__Changes 28/1/2021 in__ \ref Analysis : __Rejecting events based on DAQ trigger conditions__

Rejection of reconstructed events which are not consistent with the online DAQ trigger of each run
is now handled by a new class KVTriggerConditions. This is in order to be able to handle situations which
are more complicated than a simple minimum global multiplicity.

In analysis classes, the rejection is handled by calling KVEventSelector::SetTriggerConditionsForRun() in the InitRun() method of the analysis class.
This replaces the condition

~~~{.cpp}
if( !GetEvent()->IsOK() ) return kFALSE;
~~~

which was previously used at the beginning of the Analysis() method. The new mechanism is implemented by
default in the new examples and templates for automatically-generated user analysis classes. For the moment, trigger conditions for INDRA data
are handled; the implementation for other data will follow shortly.

__Changes 22/1/2021 in__ \ref GlobalVariables

Modification required to plugin declaration for any user-defined global variable classes,
constructor with `const char*` argument (variable name) must be used, like so:

~~~~{.cpp}
+Plugin.KVVarGlob:    MyNewVarGlob    MyNewVarGlob     MyNewVarGlob.cpp+   "MyNewVarGlob(const char*)"
~~~~

__Changes 11/12/2020 in__ \ref GlobalVariables : __Definition of new frames using global variables__

Global variables in a KVGVList can be used to define new kinematical reference frames which are available for
all variables which come later in the list.

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

__Changes 21/9/2020 in__ \ref GlobalVariables : __Event selection using global variables__

Event selection can be performed automatically based on the values of the global variables in a KVGVList.
This is implemented for example in KVEventSelector, the base class for all analysis classes. This can
improve the speed of analyses, as the conditions are tested for each global variable as soon as they
are calculated, and processing of the event aborted if it fails. Variables used for event selection should
added to the list of gobal variables before any others in order to optimise the speed of analysis.

For example, to retain for analysis only events with a total measured charge in the forward c.m.
hemisphere which is at least equal to 80% of the charge of the projectile:

~~~~{.cpp}
int UserAnalysis::fZproj;// member variable (in .h file)

void UserAnalysis::InitAnalysis()
{
   auto vg = AddGV("KVZtot", "ztot");
   vg->SetFrame("cm");
   vg->SetSelection("Vcm>0", [](const KVNucleus* n){ return n->GetVpar()>0; });
   // note capture by reference in order to use value of fZproj (not defined yet)
   vg->SetEventSelection([&](const KVVarGlob* v){ return v->GetValue()>0.8*fZproj; });
}

void UserAnalysis::InitRun()
{
   // initialize fZproj for current run
   fZproj = GetCurrentRun()->GetSystem()->GetZproj();
}
~~~~

__Changes 11/8/2020 in__ \ref Core

Added STL-style iterator to KVNameValueList. It is now possible to do the following (with C++11 support enabled):
~~~~{.cpp}
KVNameValueList p;
p.SetValue("X",3.6);
p.SetValue("Y",false);
p.SetValue("Z","hello");

for(auto& d:p) { d.Print(); }

Info in <KVNamedParameter::Print>: Name = X type = Double_t value = 3.600000
Info in <KVNamedParameter::Print>: Name = Y type = Bool_t value = false
Info in <KVNamedParameter::Print>: Name = Z type = string value = hello
~~~~

__Changes 9/8/2020 in__ \ref GlobalVariables

Major rewrite of global variable classes.

Previously, there was much source of confusion as
different variables could have specific ways of defining which nuclei they would include,
in which frame, etc., while the base methods of KVVarGlob for defining particle selection
and kinematical frames were not always respected by all classes.

Now, the same logic is applied to all global variable classes:

  * individual variable classes define *only* how they are filled from nuclei
    by overriding KVVarGlob::fill(),
    KVVarGlob::fill2() or KVVarGlob::fillN() method, depending on whether they are 1-, 2- or
    \f$N\f$-body observables, respectively;

  * particle selection is handled *only* by use of
    KVVarGlob::SetSelection() and (new) KVVarGlob::AddSelection() methods; the latter
    adds a selection which will operate in addition to any existing selection (logical AND);

  * kinematic frame selection is handled *only* by use of KVVarGlob::SetFrame(); note that
  the same frame will also be used for any kinematic quantities used to select particles.

As an example, consider the KVEtrans variable, which calculates the sum of transverse kinetic
energies for each event. Without writing a new class, the same variable can be used
in very different ways:

~~~~~~{.cpp}
KVEtrans e1("et"); // total transverse energy of all particles

KVEtrans e2("et_imf");
e2.SetSelection("_NUC_->GetZ()>2"); // total transverse energy of fragments

KVEtrans e3("et_imf_fwcm");
e3.SetSelection("_NUC_->GetZ()>2");
e3.AddSelection("_NUC_->GetVpar()>0");
e3.SetFrame("CM"); // total transverse energy of fragments in forward CM hemisphere
~~~~~~

In addition, as part of this rationalization, all existing global variables calculating
multiplicities, or sums or mean values of scalar quantities have been reimplemented
using KVVGSum, which vastly reduces code replication.

All 'Av' variants (calculating various multiplicities or sums in the "forward" hemisphere)
have been removed, as they can all be implemented using existing classes just by applying
particle selection criterion
~~~~~{.cpp}
vg.AddSelection([](const KVNucleus*n){ n->GetVpar()>0; }
~~~~~
and optionally defining the correct frame in which to apply it:
~~~~~{.cpp}
vg.SetFrame("CM");
~~~~~

__Removed classes__ :   KVZboundMean, KVTenseur3, KVTensP, KVTensE, KVTensPCM, KVMultAv, KVMultLegAv,
KVMultIMFAv, KVZtotAv, KVRisoAv

__ALL EXISTING USER GLOBAL VARIABLES NEED TO BE REWRITTEN TO RESPECT THE NEW FRAMEWORK__
__________________________

__Changes 7/8/2020 in__ \ref NucEvents

Changes to KVEvent::Iterator
 + there was a bug with the implementation of the iterator, methods pointer() and reference() clashed with implicitly-declared
member types from std::iterator. Names of methods changed to KVEvent::Iterator::get_pointer() and KVEvent::Iterator::get_reference().

__________________________

__Changes 27/7/2020 in__ \ref Analysis : __Particle selection using lambda captures (C++11..)__

KVParticleCondition has been extended to use lambda expressions (if KaliVeda is compiled with ROOT
version 6 or later)

The lambda must take a `const KVNucleus*` pointer as argument and return a boolean:
~~~~~~{.cpp}
KVParticleCondition l1("Z>2", [](const KVNucleus* nuc){ return nuc->GetZ()>2; });
KVParticleCondition l2("Vpar>0", [](const KVNucleus* nuc){ return nuc->GetVpar()>0; });
~~~~~~
Note the first argument to the constructor is a name which the user is free to define
in order to remember what the condition does.

Like any lambda expressions, variables can be 'captured' from the surrounding scope, which
can be useful in some situations. For example, given the following definitions:
~~~~~~{.cpp}
int zmin = 3;
KVParticleCondition l3("Z>zmin", [&](const KVNucleus* nuc){ return nuc->GetZ()>=zmin; });
~~~~~~
then the limit for the selection can be changed dynamically like so:
~~~~~~{.cpp}
KVNucleus N("7Li");
l3.Test(&N);      ==> returns true
zmin=5;
l3.Test(&N);      ==> returns false
~~~~~~

___________________

## Version 1.11/00

Released: 9th March 2020 
