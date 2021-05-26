//Created by KVClassFactory on Wed Jun 30 15:10:22 2010
//Author: bonnet

#ifndef __KVSIMEVENT_H
#define __KVSIMEVENT_H

#include "KVTemplateEvent.h"
#include "KVSimNucleus.h"

/**
  \class KVSimEvent
  \brief Container class for simulated nuclei, KVSimNucleus
  \ingroup Simulation
\ingroup NucEvents
 */
/**
  \class KVSimEvent::Iterator
  \ingroup Simulation
  \ingroup NucEvents
  \brief Iterator for simulated events
*/
class KVSimEvent : public KVTemplateEvent<KVSimNucleus> {
public:

   KVSimEvent(Int_t mult = 50)
      : KVTemplateEvent(mult)
   {}
   virtual ~KVSimEvent() {}
   virtual void Print(Option_t* t = "") const;

   Double_t GetTotalCoulombEnergy() const;

   ClassDef(KVSimEvent, 5) //Events from simulation
};
typedef KVSimEvent::EventIterator SimEventIterator;
typedef KVSimEvent::EventGroupIterator SimEventGroupIterator;
typedef KVSimEvent::EventOKIterator SimEventOKIterator;

/**
  \class SimEventIterator
  \brief Wrapper class for iterating over nuclei in KVSimEvent accessed through base pointer or reference
  \ingroup NucEvents
  \ingroup Simulation

    Iterators are not defined for the abstract base class KVEvent. This class is a wrapper for
    which allows to use iterators with simulated events passed as base references or pointers.
    The iterator returns references to KVSimNucleus objects:

    ~~~~{.cpp}
    KVEvent* event; // pointer to valid KVSimEvent object

    for(auto& nuc : SimEventIterator(event)) {  nuc.GetAngMom(); }
    ~~~~
  */
/**
  \class SimEventOKIterator
  \brief Wrapper class for iterating over "OK" nuclei in KVSimEvent accessed through base pointer or reference
  \ingroup NucEvents
  \ingroup Simulation

    Iterators are not defined for the abstract base class KVEvent. This class is a wrapper for
    which allows to use iterators with simulated events passed as base references or pointers.
    The iterator returns references to KVSimNucleus objects:

    ~~~~{.cpp}
    KVEvent* event; // pointer to valid KVSimEvent object

    for(auto& nuc : SimOKEventIterator(event)) {  nuc.GetAngMom(); }
    ~~~~
  */
/**
  \class SimEventGroupIterator
  \brief Wrapper class for iterating over nuclei in KVSimEvent accessed through base pointer or reference
  \ingroup NucEvents
  \ingroup Simulation

    Iterators are not defined for the abstract base class KVEvent. This class is a wrapper for
    which allows to use iterators with simulated events passed as base references or pointers.
    The iterator returns references to KVSimNucleus objects:

    ~~~~{.cpp}
    KVEvent* event; // pointer to valid KVSimEvent object

    for(auto& nuc : SimGroupEventIterator(event, "groupname")) {  nuc.GetAngMom(); }
    ~~~~
  */
#endif
