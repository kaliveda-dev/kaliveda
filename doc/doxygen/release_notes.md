\page release_notes Release Notes for KaliVeda

Last update: 21st September 2020

## Version 1.11/01 (current development focus)

__Changes 21/9/2020 in__ \ref GlobalVariables

Event selection can be performed automatically based on the values of the global variables in a KVGVList.
This is implemented for example in KVEventSelector, the base class for all analysis classes.

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

__________________________

__Changes 7/8/2020 in__ \ref NucEvents

Changes to KVEvent::Iterator
 + there was a bug with the implementation of the iterator, methods pointer() and reference() clashed with implicitly-declared
member types from std::iterator. Names of methods changed to KVEvent::Iterator::get_pointer() and KVEvent::Iterator::get_reference().

__________________________

__Changes 27/7/2020 in__ \ref Analysis

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
