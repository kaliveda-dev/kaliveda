\defgroup NucEvents Nuclei & Events
\brief Classes describing atomic nuclei properties, kinematics, and multi-particle events

### Contents

  - \ref events
    - \ref evBuild
      - \ref multEv
      - \ref objOwn
      - \ref stoRef
      - \ref copyEv
      - \ref mergEv
    - \ref evTree
    - \ref iterEv
      - \ref indexLoop
      - \ref simpIt
      - \ref stlIter
      - \ref rangeIter
      - \ref baseSTL
    - \ref kinFrames
      - \ref kinFrame1
      - \ref defFrame
      
\section events 2. Events

The main business of KaliVeda is the analysis of multi-body events produced in heavy-ion reactions,
therefore it is no surprise that a central role is played by container classes for collections of nuclei (see \ref NucProp).
In fact there are three main event classes:

 - KVNucleusEvent: base class for multi-body events (container for KVNucleus objects);
 - KVSimEvent: base class for simulated multi-body events (container for KVSimNucleus objects);
 - KVReconstructedEvent: base class for reconstructed multi-body events, either experimental data or the result of "filtering" some simulated data (container for KVReconstructedNucleus objects);

Each of these classes inherits from the abstract base class KVEvent which can be used as an argument for
general functions which can manipulate any type of event:

~~~~{.cpp}
auto print_event = [](KVEvent& event){ event.Print(); };

KVReconstructedEvent recev;
print_event(recev); // OK

KVSimEvent simev;
print_event(simev); // OK
~~~~

\subsection evBuild 2.1 Event building

Events are built up by successive calls to the KVEvent::AddParticle() method, which adds a KVNucleus -derived object to the event corresponding
to the event's defined nucleus type, and returns a pointer to the new nucleus:

~~~~{.cpp}
KVReconstructedEvent recev;
auto nuc = recev.AddParticle();

std::cout << nuc->ClassName() << std::endl;
"KVReconstructedNucleus"
~~~~

A small subtlety of this (virtual) method is that, when the event is accessed through a base pointer or reference,
the pointer returned by KVEvent::AddParticle() is also a base pointer (to a KVNucleus), although the added nucleus is
of course always of the correct type for the event:

~~~~{.cpp}
KVReconstructedEvent recev;
KVEvent& eventRef = recev;
auto nuc = eventRef.AddParticle();

std::cout << nuc->ClassName() << std::endl;
"KVReconstructedNucleus"  // type of the nucleus object

std::cout << nuc->Class_Name() << std::endl;
"KVNucleus"  // type of the nucleus pointer
~~~~

Once added to an event, a nucleus cannot be removed from the event; however, all nuclei can be removed (deleted)
in order to build a new event with method Clear():

~~~~{.cpp}
KVReconstructedEvent recev;
recev.Clear();
recev.GetMult(); // size (multiplicity) of event
0
~~~~

\subsubsection multEv 2.1.1 Event multiplicities

The number of nuclei in any event (the size of the container) is called the multiplicity of the event,
and is given by the method KVEvent::GetMult():

~~~~{.cpp}
KVEvent* event; // pointer to some valid event object
event->GetMult();
2
~~~~

The multiplicity can also be restricted to consider only nuclei which are considered "OK" for analysis
(this usually concerns only reconstructed events) or counting only nuclei belonging to a previously defined
"group":

~~~~{.cpp}
KVEvent* event; // pointer to some valid event object
event->GetParticle(1)->SetIsOK(false); // note that by default all particles are considered "OK"
event->GetMult("ok"); // case insensitive: "ok" or "OK" or "oK" or "Ok" are all OK!
1

event->GetParticle(2)->AddGroup("QP"); // affect nucleus #2 to the "QP" group
event->GetMult("qP"); // group names are also case insensitive
1
~~~~

\subsubsection objOwn 2.1.2 Object ownership

Events are built up by repeated calls to the KVEvent::AddParticle() method which creates a new
nucleus object, adds it to the event, and returns a pointer to the new nucleus.
All nuclei in an event __belong__ to the event and are destroyed by the KVEvent destructor
when it goes out of scope.

Therefore a KVEvent cannot be used to store references to nuclei in another KVEvent object,
for example if one wants to handle a subset of the nuclei in the event. This is why the
iterators presented below allow to iterate only over selected subsets (groups) of nuclei if required (see \ref iterEv).

\subsubsection stoRef 2.1.3 Storing references to nuclei in an event

If you really need to store a list of references to some nuclei in an event, you can use an
STL container, TCollection or KVSeqCollection collection class to store the nuclei's pointers,
as long as the collection does not try to delete the objects when it goes out of scope
(e.g. don't use KVList which owns its objects by default).

\subsubsection copyEv 2.1.4  Making a copy of all or part of an event

You can if you wish copy all or part of an event, as long as you understand that the nuclei 
in the copy will be new independent objects; they will not change if you change the original
event after the copy (there may also be unwanted side-effects, especially for
KVReconstructedNucleus particles).

Making a complete copy of an event (included the associated list of parameters) can be done like this:

~~~~~~~~~{.cpp}
KVEvent event, event2;
// make a copy of event in event2
event2.Clear(); // reset event2 before copying
event.Copy(event2);
~~~~~~~~~

\subsubsection mergEv 2.1.5 Merging several events into one

Although it is difficult/unwise to separate events into subevents, on the other hand it is possible to merge several event fragments into one single event:

~~~~~~~~~{.cpp}
// 'event_list' is a TList (for example) containing KVEvent objects to merge
KVEvent merged_event;
merged_event.MergeEventFragments(&event_list);
~~~~~~~~~

**WARNING** after merging, the subevents in the list will be empty and useless. Do not try to use them after merging!

\subsection evTree 2.2 Writing & reading events in ROOT TTree branches

Events i.e. KVEvent-derived objects can be written in a branch of a TTree in order to store them in a ROOT file.
This is the format used for all event-based data handled (or generated) by KaliVeda.

Here is a complete example of how to write events in a TTree on disk:

~~~~{.cpp}
TFile f("events.root", "recreate");
TTree* t = new TTree;

// the event object has to be instantiated on the heap (with 'new') before branch creation
KVSimEvent* simev = new KVSimEvent;

// note that a static method is used: the event pointer is given in argument
KVEvent::MakeEventBranch(t, "event_branch", simev);

// now we can begin to fill the events, and the TTree
simev->AddParticle();
simev->AddParticle();
...etc...
t->Fill();

// and finally write to disk
f.Write()
~~~~

Reading back the events written in the branch of a TTree can be done as follows:

~~~~{.cpp}
TFile f("events.root");
f.ls();
TFile**		events.root	
 TFile*		events.root	
  KEY: TTree	tree;1	My tree

tree->Print();
******************************************************************************
*Tree    :tree      : My tree                                                *
*Entries :        1 : Total =            2317 bytes  File  Size =        819 *
*        :          : Tree compression factor =   3.37                       *
******************************************************************************
*Br    0 :event_branch : KVSimEvent                                          *
*Entries :        1 : Total  Size=       1951 bytes  File Size  =        429 *
*Baskets :        1 : Basket Size=   10000000 bytes  Compression=   3.37     *
*............................................................................*

// to connect to the branch requires a null-initialised pointer
KVSimEvent* simev = nullptr;

// NB. the address of the pointer is passed as argument
tree->SetBranchAddress("event_branch", &simev);

// read an event
tree->GetEntry(0);

// simev is now initialised and points to the event read from the TTree
simev->Print();

KVSimEvent with 3 particles :
------------------------------------
KVNucleus Z=0 A=0 E*=-nan
KVParticle mass=0 Theta=0 Phi=0 KE=0 Vpar=0
etc. etc.
~~~~

\subsection iterEv 2.3 Iterating over nuclei

There are several different ways to loop over all (or part of) the nuclei in an event and do something
useful (for analysis) with them.

\subsubsection indexLoop 2.3.1 Simple indexed loop

~~~~{.cpp}
KVSimEvent simev;

int ztot(0);
for(int i=1; i<=simev.GetMult(); ++i) ztot += simev.GetParticle(i)->GetZ();
~~~~

Note that the same result can be achieved thus:
~~~~{.cpp}
auto ztot = simev.GetSum("GetZ");
~~~~

As for the KVEvent::AddParticle() method (see above), the type of pointer returned by KVEvent::GetParticle() changes if the event
is accessed through a base pointer or reference: in this case a base KVNucleus pointer or reference is returned.
Therefore if specific child class methods are accessed for the nuclei in the event, care must be taken:

~~~~{.cpp}
KVSimEvent simev;

TVector3 total_spin;
for(int i=1; i<=simev.GetMult(); ++i)
         total_spin += simev.GetParticle(i)->GetAngMom(); // OK: returns KVSimNucleus pointer

KVEvent* event = &simev;
for(int i=1; i<=event->GetMult(); ++i)
         // total_spin += event->GetParticle(i)->GetAngMom(); => compile error: returns KVNucleus pointer
         total_spin += dynamic_cast<KVSimNucleus*>(event->GetParticle(i))->GetAngMom(); // OK
~~~~

In a simple indexed loop, if the iteration is to be restricted to only "OK" particles, or only those
belonging to a previously-defined group, the loop must still be carried out over the full
event (i.e. using KVEvent::GetMult() with no arguments), and the test performed for each particle:

~~~~{.cpp}
// sum of Z for all "OK" particles
for(int i=1; i<=simev.GetMult(); ++i)
  if(simev.GetParticle(i)->IsOK())
     ztot_ok += simev.GetParticle(i)->GetZ();

// sum of Z for all "QP" particles
for(int i=1; i<=simev.GetMult(); ++i)
  if(simev.GetParticle(i)->BelongsToGroup("QP"))
     ztot_qp += simev.GetParticle(i)->GetZ();
~~~~

Note that each of these iterations can be replaced by:
~~~~{.cpp}
// sum of Z for all "OK" particles
ztot_ok = simev.GetSum("GetZ","Ok");  // case insensitive "OK"

// sum of Z for all "QP" particles
ztot_qp = simev.GetSum("GetZ","Qp");  // groupname case insensitive
~~~~

\subsubsection simpIt 2.3.2 Simple (deprecated) iterator

The KVEvent::GetNextParticle() method can be used to easily loop over nuclei in an event:

~~~~{.cpp}
KVReconstructedEvent recev;
KVReconstructedNucleus* nuc;
// print name of detector in which each particle of event was stopped -
// this only makes sense for KVReconstructedNucleus objects, reconstructed from
// hits recorded in different detectors
while( (nuc = recev.GetNextParticle()) ) { std::cout << nuc->GetStoppingDetector()->GetName() << std::endl; }
     // note extra parentheses around assignment to avoid compiler warnings
~~~~

This method is deprecated as it uses a shared internal iterator which makes it impossible to perform
a nested loop using two such iterations; some methods such as KVEvent::GetMult() may also use the same
internal iterator, therefore using any of those inside a loop using KVEvent::GetNextParticle()
will have unexpected results!

As for the KVEvent::GetParticle() method (see above), the type of pointer returned by KVEvent::GetNextParticle() changes if the event
is accessed through a base pointer or reference: in this case a base KVNucleus pointer or reference is returned.
Therefore if specific child class methods are accessed for the nuclei in the event, care must be taken:

~~~~{.cpp}
KVEvent* event = &recev; // base class pointer to event
// while( (nuc = event->GetNextParticle()) ) { ... } => does not compile: event->GetNextParticle() returns KVNucleus*

KVNucleus* nunuc;
while( (nunuc = event->GetNextParticle()) ) // OK
// { std::cout << nunuc->GetStoppingDetector()->GetName() << std::endl; } => does not compile: KVNucleus has no "GetStoppingDetector" method

while( (nunuc = event->GetNextParticle()) )
{ std::cout << dynamic_cast<KVReconstructedNucleus*>(nunuc)->GetStoppingDetector()->GetName() << std::endl; } // => OK
~~~~

With the KVEvent::GetNextParticle() method, it is easy to restrict the loop to only nuclei which are
"OK" or only those belonging to a previously defined group:

~~~~{.cpp}
// only print stopping detector for nuclei which are "OK":
while( (nuc = recev.GetNextParticle("ok")) ) { std::cout << nuc->GetStoppingDetector()->GetName() << std::endl; }

// only print stopping detector for nuclei in group "QP":
while( (nuc = recev.GetNextParticle("qp")) ) { std::cout << nuc->GetStoppingDetector()->GetName() << std::endl; }
~~~~

Again, the name of the group or the "OK" status argument are case insensitive.

\subsubsection stlIter 2.3.3 STL-style iterators

STL containers such as std::vector can be iterated over using the std::vector::iterator class:

~~~~{.cpp}
std::vector<int> numbers {1,3,6,10};
for(std::vector<int>::iterator it = numbers.begin(); it !=  numbers.end(); ++it)
{  std::cout << (*it) << std::endl; }
1
3
6
10
~~~~

KVEvent containers can be iterated over in an analogous fashion:

~~~~{.cpp}
KVReconstructedEvent recev;
for(KVReconstructedEvent::Iterator it = recev.begin(); it!=recev.end(); ++it)
{
   std::cout << (*it).GetStoppingDetector()->GetName();
}

// or more simply:
for(auto it = recev.begin(); it!=recev.end(); ++it)
{
   std::cout << (*it).GetStoppingDetector()->GetName();
}
~~~~

Dereferencing the iterator (i.e. the result of `*it`) gives a reference to the type of nucleus
contained in the event: in this case, a reference to a KVReconstructedNucleus. The following table resumes the
different event classes & associated iterators and underlying nucleus types:

Event class          | Iterator class                 | Type of nucleus        
-------------------- | ------------------------------ | ---------------------- 
KVNucleusEvent       | KVNucleusEvent::Iterator       | KVNucleus              
KVSimEvent           | KVSimEvent::Iterator           | KVSimNucleus           
KVReconstructedEvent | KVReconstructedEvent::Iterator | KVReconstructedNucleus 

#### Iterating over "OK" particles or groups of particles

The iterator returned by KVReconstructedEvent::begin() in the previous example will include all nuclei of the event in the loop:
this Iterator is of type KVReconstructedEvent::Iterator::Type::All.
Iterators of different types can be used in order to limit loops to include only particles which are `"OK"` (used for analysis
of reconstructed nuclei):

~~~~{.cpp}
for(auto it = KVReconstructedEvent::Iterator(recev, KVReconstructedEvent::Iterator::Type::OK); it!=recev.end(); ++it)
{
  // loop over all "OK" particles
}
~~~~

Similarly, in order to loop over all particles belonging to a previously defined group,

~~~~{.cpp}
for(auto it = KVReconstructedEvent::Iterator(recev, KVReconstructedEvent::Iterator::Type::Group, "QP"); it!=recev.end(); ++it)
{
  // loop over particles in "QP" group
}
~~~~

In order to simplify the use of such iterators, we provide wrapper classes for an easier interface

~~~~{.cpp}
for(auto it = KVReconstructedEvent::EventOKIterator(event).begin(); it!=recev.end(); ++it)
{
  // loop over all "OK" particles
}
~~~~

and to further simplify the code, we provide aliases for these wrappers:

~~~~{.cpp}
for(auto it = ReconEventOKIterator(event).begin(); it!=recev.end(); ++it)
{
  // loop over all "OK" particles
}
~~~~

The following table resumes all types, wrappers and aliases for the three main event classes:

Event class          | Iterator type                         | Wrapper                            | Alias  
-------------------- | ------------------------------------- | ---------------------------------- | -------------------
KVNucleusEvent       | KVNucleusEvent::Iterator::Type::All   | KVNucleusEvent::EventIterator      | EventIterator
^                    | KVNucleusEvent::Iterator::Type::OK    | KVNucleusEvent::EventOKIterator    | EventOKIterator
^                    | KVNucleusEvent::Iterator::Type::Group | KVNucleusEvent::EventGroupIterator | EventGroupIterator
KVReconstructedEvent       | KVReconstructedEvent::Iterator::Type::All   | KVReconstructedEvent::EventIterator      | ReconEventIterator
^                    | KVReconstructedEvent::Iterator::Type::OK    | KVReconstructedEvent::EventOKIterator    | ReconEventOKIterator
^                    | KVReconstructedEvent::Iterator::Type::Group | KVReconstructedEvent::EventGroupIterator | ReconEventGroupIterator
KVSimEvent       | KVSimEvent::Iterator::Type::All   | KVSimEvent::EventIterator      | SimEventIterator
^                    | KVSimEvent::Iterator::Type::OK    | KVSimEvent::EventOKIterator    | SimEventOKIterator
^                    | KVSimEvent::Iterator::Type::Group | KVSimEvent::EventGroupIterator | SimEventGroupIterator

\subsubsection rangeIter 2.3.4 Range-based for loops

Just as STL containers can be iterated over using a range-based `for` loop,

~~~~{.cpp}
std::vector<int> numbers {1,3,6,10};
for(auto number : numbers)
{  std::cout << number << std::endl; }
1
3
6
10
~~~~

the same facility is also available for event classes, thanks to the iterator classes and wrappers presented above:

~~~~{.cpp}
KVNucleusEvent event;

for(auto& nuc : event)  { nuc.Print(); }
~~~~

Note the use of an automatic reference type for `nuc`: this is recommended to avoid copying of underlying nucleus
objects, which may have unfortunate side-effects in certain cases (KVReconstructedNucleus).

Range-based for loops limited to either "OK" particles or particles in different groups can also be easily
implemented thanks to the wrappers introduced above:

~~~~{.cpp}
KVReconstructedEvent recev;
for(auto& nuc : ReconEventOKIterator(recev))
{
  // loop over all "OK" particles
}

KVSimEvent simev;
for(auto& nuc : SimEventGroupIterator(simev,"QP"))
{
  // loop over particles in "QP" group
}
~~~~

\subsubsection baseSTL 2.3.5 STL iterators & range-based for loops with base pointers/references to events

Often, for example in analysis classes, the event to be analysed (=iterated over) is accessed through a pointer or
reference of base type KVEvent, for example via the KVEventSelector::GetEvent() method. The STL-type iterators which
allow to perform the range-based for loops presented above are not defined for KVEvent, only in daughter classes
inheriting from the templated class KVTemplateEvent. In such a case we do not necessarily know in advance what
is the actual type of the event (and hence of the contained nuclei). However, the wrapper classes presented above
require only a base pointer or reference (KVEvent* or KVEvent&), and so still can be used:

~~~~{.cpp}
// assuming we are in a KVEventSelector analysis class: GetEvent() method returns a KVEvent*
// pointer to the events read from the TTree being analysed

for(auto& nuc : ReconEventOKIterator(GetEvent()))
{
  // loop over all "OK" particles
  std::cout << nuc.GetStoppingDetector()->GetName() << std::endl;
}
~~~~

The choice of wrapper to use depends on the type of nucleus reference which is required in the loop,
which in turn depends on the methods applied to each nucleus in the code: if only those methods which are defined for KVNucleus
are required,
an EventIterator wrapper (which returns KVNucleus& references) can be used; if methods specific to KVSimNucleus are
used, a SimEventIterator wrapper should be used; etc. The following table resumes the different options:

Reference type          | Wrappers                       
-------------------- | -------------------------------------
KVNucleus&      | EventIterator
^                    | EventOKIterator
^                    | EventGroupIterator
KVReconstructedNucleus&       | ReconEventIterator
^                    | ReconEventOKIterator
^                    | ReconEventGroupIterator
KVSimNucleus&        | SimEventIterator
^                    | SimEventOKIterator
^                    | SimEventGroupIterator

#### Warning about type safety

It should be noted that these iterator wrappers are not 'type-safe': at compile time, there is no way of knowing if the event
whose pointer or reference is supplied to the wrapper at run time will contain nuclei of the class corresponding to the
wrapper (note that it is perfectly OK for the nuclei in the event to be of a class which inherits from the wrapper nucleus
type). Consider the following example:

~~~~{.cpp}
KVSimEvent simev;
KVEvent& base_ref = simev;
for(auto& nuc : ReconEventIterator(base_ref))
{
   std::cout << nuc.GetStoppingDetector()->GetName() << std::endl;
}
~~~~

The KVSimEvent contains KVSimNucleus nuclei; using the ReconEventIterator wrapper with a base reference to this event,
we attempt to use a method with these nuclei which is only defined for KVReconstructedNucleus nuclei. This would have
disastrous consequences (segmentation violation). To avoid this, a warning will
be printed if the wrapper does not correspond to the type of nuclei in the event, and no iteration will be performed: in the previous case, this will give:

~~~~{.cpp}
Warning in <KVTemplateEvent::Iterator>: KVTemplateEvent<KVReconstructedNucleus>::Iterator for KVReconstructedNucleus nuclei requested for event containing KVSimNucleus nuclei. Iteration is aborted.
~~~~

\subsection kinFrames 2.4 Defining and using different kinematical reference frames for events

\subsubsection kinFrame1 2.4.1 Defining and accessing different reference frames

You can define and use several
different reference frames for the particles in an event. Each
frame can be used independently, and new frames can be defined based on any of the
existing frames:

__Example:__
 - define a new frame moving at 5 cm/ns in the beam direction:

~~~~~~~~~~~~~~~~~~{.cpp}
KVEvent* event; // pointer to some valid event object
event->SetFrame("moving_frame", TVector3(0,0,5));
~~~~~~~~~~~~~~~~~~

 - define a rotated coordinate frame in the "moving_frame", rotated by $90^o$ clockwise around the +ve beam direction:

~~~~~~~~~~~~~~~~~~{.cpp}
KVEvent* event; // pointer to some valid event object
TRotation rot;
rot.RotateZ(TMath::PiOver2());
event->SetFrame("rotated_moving_frame", "moving_frame", rot);
~~~~~~~~~~~~~~~~~~

  Note that the same frame can be defined directly from the original frame of all particles in the event by using a combined boost-then-rotation transform:

~~~~~~~~~~~~~~~~~~{.cpp}
KVEvent* event; // pointer to some valid event object
event->SetFrame("rotated_moving_frame", KVFrameTransform(TVector3(0,0,5),rot));
// OR, with C++11 and later:
event->SetFrame("rotated_moving_frame", {{0,0,5},rot});
~~~~~~~~~~~~~~~~~~

 - define a similarly rotated coordinate frame in the original (default) reference frame:

~~~~~~~~~~~~~~~~~~{.cpp}
KVEvent* event; // pointer to some valid event object
event->SetFrame("rotated_frame", rot);
~~~~~~~~~~~~~~~~~~

 - access kinematical information in any of these frames for any of the particles in the event:

~~~~~~~~~~~~~~~~~~{.cpp}
KVEvent* event; // pointer to some valid event object
event->GetParticle(i)->GetFrame("moving_frame")->GetVpar();
event->GetParticle(i)->GetFrame("rotated_frame")->GetPhi();
event->GetParticle(i)->GetFrame("rotated_moving_frame")->GetTransverseEnergy();
~~~~~~~~~~~~~~~~~~

Note that the frame `"rotated_moving_frame"` is directly accessible even if it is defined in two
steps as a rotation of the `"moving_frame"`.

\subsubsection defFrame 2.4.2 Changing the default reference frame of an event

Let us consider an event for which the different reference frames in the previous paragraph have been defined.
Calling method KVEvent::Print() will show all reference frames defined for each particle:

~~~~~~~~~~~~~~~~~~{.cpp}
KVEvent* event; // pointer to some valid event object
event->Print()

KVParticle mass=939 Theta=45 Phi=0 KE=32.7103 Vpar=5.45392
         moving_frame:  Theta=85.1751 Phi=0 KE=16.6117 Vpar=0.468125
                 rotated_moving_frame:  Theta=85.1751 Phi=270 KE=16.6117 Vpar=0.468125
         rotated_frame:  Theta=45 Phi=270 KE=32.7103 Vpar=5.45392

etc. etc.
~~~~~~~~~~~~~~~~~~

Indentation indicates the relationships between frames: `"rotated_moving_frame"` is a child frame of `"moving_frame"`.
The first line is the default kinematics. As yet it has no name, but if we want we can set a name for the
default kinematics of each particle in the event:

~~~~~~~~~~~~~~~~~~{.cpp}
KVEvent* event; // pointer to some valid event object
 event->SetFrameName("lab");
~~~~~~~~~~~~~~~~~~

Now if we want to change the default kinematical frame for the event by using KVEvent::ChangeDefaultFrame():

~~~~~~~~~~~~~~~~~~{.cpp}
 KVEvent* event;// pointer to some valid event object
 event->ChangeDefaultFrame("rotated_moving_frame");

event->Print();

KVParticle mass=939 Theta=85.1751 Phi=270 KE=16.6117 Vpar=0.468125
         moving_frame:  Theta=85.1751 Phi=0 KE=16.6117 Vpar=0.468125
                 lab:  Theta=45 Phi=0 KE=32.7103 Vpar=5.45392
                         rotated_frame:  Theta=45 Phi=270 KE=32.7103 Vpar=5.45392
KVNameValueList::ParticleParameters : Parameters associated with a particle in an event (0x7f5a1ff8b1b8)
 <frameName=rotated_moving_frame>
~~~~~~~~~~~~~~~~~~

Note that the name of the default kinematics is stored as a parameter `"frameName"` and can be retrieved with method
KVEvent::GetFrameName().
Note also how the relationships between frames are preserved, i.e. if we present the frames as graphs:

with "lab" as default frame:

~~~~~~
lab
 |
 +--moving_frame
 |        |
 |        +--rotated_moving_frame
 |
 +--rotated_frame
~~~~~~

with "rotated_moving_frame" as default frame:

~~~~~~
rotated_moving_frame
        |
        +--moving_frame
                 |
                 +--lab
                     |
                     +--rotated_frame
~~~~~~

