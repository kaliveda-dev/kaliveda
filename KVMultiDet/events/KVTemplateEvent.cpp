/***************************************************************************
$Id: KVEvent.cpp,v 1.41 2008/12/17 11:23:12 ebonnet Exp $
                          kvevent.cpp  -  description
                             -------------------
    begin                : Sun May 19 2002
    copyright            : (C) 2002 by J.D. Frankland
    email                : frankland@ganil.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "KVTemplateEvent.h"

templateClassImp(KVTemplateEvent)

////////////////////////////////////////////////////////////////////////////////
/*
<h2>KVEvent</h2>
<h4>Base class for all types of multiparticle event consisting of KVNucleus objects</h4>

Particles are stored in a TClonesArray and KVEvent provides some basic
functionality for accessing and manipulating the list.

Events can be built using any class derived from KVNucleus to represent particles.
These classes can allocate memory in their default ctor: when filling events in a loop,
the same 'particle' objects are re-used for each new event, the ctor of each object will
only be called once when the object is first created (e.g. in the first event).
The particle class Clear() method will be called before each new event.
Therefore the cycle of use of the particle objects in a loop over many events is:

~~~~~~~~~~~~~~~
<particle ctor>
   Building 1st event
   <particle Clear()>
   Building 2nd event
   <particle Clear()>
   ...
   Building last event
<particle dtor>
~~~~~~~~~~~~~~~

When writing events in a TTree, it is very important to call the TBranch::SetAutoDelete(kFALSE)
method of the branch which is used to store the event object.

If not, when the events are read back, the KVEvent constructor and destructor will be called
every time an event is read from the TTree meading to very slow reading times (& probably
memory leaks).

For this reason we provide the method:

~~~~~~~~~~~~~~~
    void MakeEventBranch(TTree*, const TString&, const TString&, void*)
~~~~~~~~~~~~~~~

which should be used whenever it is required to stock KVEvent-derived objects in a TTree.
*/
/////////////////////////////////////////////////////////////////////////////://

/** \example KVEvent_iterator_example.C
# Example of different ways to iterate over KVEvent objects

This demonstrates the use of
  - KVEvent::Iterator class
  - EventIterator wrapper class
  - KVEvent::GetNextParticle()

All of these can be used to iterate over the particles in an event, optionally applying some selection criteria.

The KVEvent::Iterator/EventIterator classes also allow to use
(a restricted set of) STL algorithms, and (from C++11 onwards)
the use of range-based for-loops.

To execute this function, do

    $ kaliveda
    kaliveda[0] .L KVEvent_iterator_example.C+
    kaliveda[1] iterator_examples()

*/
