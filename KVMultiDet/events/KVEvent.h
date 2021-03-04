/***************************************************************************
                          kvevent.h  -  description
                             -------------------
    begin                : Sun May 19 2002
    copyright            : (C) 2002 by J.D. Frankland
    email                : frankland@ganil.fr

$Id: KVEvent.h,v 1.29 2008/12/17 11:23:12 ebonnet Exp $
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KVEVENT_H
#define KVEVENT_H

#define KVEVENT_PART_INDEX_OOB "Particle index %d out of bounds [1,%d]"

#include "TTree.h"
#include "TVector3.h"
#include "TClonesArray.h"
#include "KVNucleus.h"
#include "KVBase.h"
#include "KVConfig.h"
#include "TRotation.h"
#include "TLorentzRotation.h"
#include "KVParticleCondition.h"
#include "KVNameValueList.h"
#include "TMethodCall.h"

#include <TH1.h>
#include <iterator>

class KVIntegerList;

/**
  \class KVEvent
  \brief Base class container for multi-particle events
  \ingroup NucEvents

The main business of KaliVeda is the analysis of multi-body events produced in heavy-ion reactions,
therefore it is no surprise that a central role is played by the class KVEvent which can be thought of as a
container class for particles and nuclei (see KVParticle & KVNucleus).

In addition to containing a list of particles/nuclei, each event class also has in common the following functionality:

 - an associated list of parameters, accessible through the GetParameters() and SetParameter() methods;
 - iterators for looping over all or a subset of the particles of the event;
 - methods for defining named subsets ('groups') of particles according to various selection criteria;
 - methods for defining/modifying different relativistic reference frames in which to 'view' the particles of the event

Let us mention in passing the associated KVEventViewer class which can be used to produce 3D images of events using the ROOT OpenGL backend.

See the chapter in the User's Guide for more details: http://indra.in2p3.fr/kaliveda/UsersGuide/events.html

### Kinematical reference frames

See also KVParticle for accessing/changing reference frames of individual particles.

#### 1. Defining and accessing different reference frames for all particles of an event
You can define and use several
different reference frames for the particles in an event. Each
frame can be used independently, and new frames can be defined based on any of the
existing frames:

__Example:__ (for an event accessed through pointer `KVEvent* e`):
 - define a new frame moving at 5 cm/ns in the beam direction:

~~~~~~~~~~~~~~~~~~{.cpp}
            e->SetFrame("moving_frame", TVector3(0,0,5));
~~~~~~~~~~~~~~~~~~

 - define a rotated coordinate frame in the "moving_frame", rotated by \f$90^o\f$ clockwise around the +ve beam direction:

~~~~~~~~~~~~~~~~~~{.cpp}
            TRotation rot;
            rot.RotateZ(TMath::PiOver2());
            e->SetFrame("rotated_moving_frame", "moving_frame", rot);
~~~~~~~~~~~~~~~~~~

  Note that the same frame can be defined directly from the original frame of all particles in the event by using a combined boost-then-rotation transform:

~~~~~~~~~~~~~~~~~~{.cpp}
            e->SetFrame("rotated_moving_frame", KVFrameTransform(TVector3(0,0,5),rot));

            --//-- the following only works with C++11 and later
            e->SetFrame("rotated_moving_frame", {{0,0,5},rot});
~~~~~~~~~~~~~~~~~~

 - define a similarly rotated coordinate frame in the original (default) reference frame:

~~~~~~~~~~~~~~~~~~{.cpp}
            e->SetFrame("rotated_frame", rot);
~~~~~~~~~~~~~~~~~~

 - access kinematical information in any of these frames for any of the particles in the event:

~~~~~~~~~~~~~~~~~~{.cpp}
            e->GetParticle(i)->GetFrame("moving_frame")->GetVpar();
            e->GetParticle(i)->GetFrame("rotated_frame")->GetPhi();
            e->GetParticle(i)->GetFrame("rotated_moving_frame")->GetTransverseEnergy();
~~~~~~~~~~~~~~~~~~

Note that the frame `"rotated_moving_frame"` is directly accessible even if it is defined in two
steps as a rotation of the `"moving_frame"`.

#### 2. Changing the default reference frame for all particles in an event
Let us consider an event for which the different reference frames in the previous paragraph have been defined.
Calling method Print() will show all reference frames defined for each particle:

~~~~~~~~~~~~~~~~~~{.cpp}
e->Print()

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
e->SetFrameName("lab");
~~~~~~~~~~~~~~~~~~

Now if we want to change the default kinematical frame for the event by using ChangeDefaultFrame():

~~~~~~~~~~~~~~~~~~{.cpp}
e->ChangeDefaultFrame("rotated_moving_frame");

e->Print();

KVParticle mass=939 Theta=85.1751 Phi=270 KE=16.6117 Vpar=0.468125
         moving_frame:  Theta=85.1751 Phi=0 KE=16.6117 Vpar=0.468125
                 lab:  Theta=45 Phi=0 KE=32.7103 Vpar=5.45392
                         rotated_frame:  Theta=45 Phi=270 KE=32.7103 Vpar=5.45392
KVNameValueList::ParticleParameters : Parameters associated with a particle in an event (0x7f5a1ff8b1b8)
 <frameName=rotated_moving_frame>
~~~~~~~~~~~~~~~~~~

Note that the name of the default kinematics is stored as a parameter `"frameName"` and can be retrieved with method GetFrameName().
Note also how the relationships between frames are preserved, i.e. if we present the frames as graphs:

with "lab" as default frame:
~~~~
          lab
           |
           +--moving_frame
           |        |
           |        +--rotated_moving_frame
           |
           +--rotated_frame
~~~~
with "rotated_moving_frame" as default frame:
~~~~
   rotated_moving_frame
           |
           +--moving_frame
                    |
                    +--lab
                        |
                        +--rotated_frame
~~~~

 */
class KVEvent: public KVBase {

protected:

   TClonesArray* fParticles;    //->array of particles in event
   KVNameValueList fParameters;//general-purpose list of parameters
#ifdef __WITHOUT_TCA_CONSTRUCTED_AT
   TObject* ConstructedAt(Int_t idx);
   TObject* ConstructedAt(Int_t idx, Option_t* clear_options);
#endif
public:

   class Iterator {
   public:
      typedef std::forward_iterator_tag iterator_category;
      typedef KVNucleus value_type;
      typedef std::ptrdiff_t difference_type;
      typedef KVNucleus* pointer;
      typedef KVNucleus& reference;

      enum Type {    // type of iterator
         Null,      // null value
         All,       // include all particles
         OK,        // include particles which are "OK"
         Group      // include particles belonging to group
      };

   private:
      TIter   fIter;//iterator over TClonesArray
      Type    fType;//iterator type
      mutable Bool_t  fIterating;//=kTRUE when iteration in progress
      TString fGroup;//groupname for group iterations
      Bool_t AcceptableIteration()
      {
         // Returns kTRUE if the current particle in the iteration
         // corresponds to the selection criteria (if any)

         switch (fType) {
            case OK:
               return current()->IsOK();
               break;
            case Group:
               return current()->BelongsToGroup(fGroup);
               break;
            case All:
            default:
               return kTRUE;
         }
         return kTRUE;
      }
      KVNucleus* current() const
      {
         // Returns pointer to current particle in iteration
         // Resets fIterating flag to kFALSE at end of list
         return reinterpret_cast<KVNucleus*>(*fIter);
      }
   public:
      Iterator()
         : fIter(static_cast<TIterator*>(nullptr)),
#ifdef WITH_CPP11
           fType(Type::Null),
#else
           fType(Null),
#endif
           fIterating(kFALSE),
           fGroup()
      {}
      Iterator(const Iterator& i)
         : fIter(i.fIter),
           fType(i.fType),
           fIterating(i.fIterating),
           fGroup(i.fGroup)
      {}

#ifdef WITH_CPP11
      Iterator(const KVEvent* e, Type t = Type::All, const TString& grp = "")
#else
      Iterator(const KVEvent* e, Type t = All, const TString& grp = "")
#endif
         : fIter(e->fParticles), fType(t), fIterating(kTRUE), fGroup(grp)
      {
         // Construct an iterator object to read in sequence the particles in event *e.
         // By default, opt="" and all particles are included in the iteration.
         // opt is a case-insensitive option controlling the iteration:
         //   opt="ok"/"OK":  only particles whose KVParticle::IsOK() method returns kTRUE
         //                   are iterated over
         //   opt=any other non-empty string:  only particles belonging to the given group
         //                                    i.e. KVParticle::BelongsToGroup(opt) returns
         //                                    kTRUE

         // set iterator to first particle of event corresponding to selection
         fIter.Begin();
         while ((current() != nullptr) && !AcceptableIteration()) ++fIter;
      }

#ifdef WITH_CPP11
      Iterator(const KVEvent& e, Type t = Type::All, const TString& grp = "")
#else
      Iterator(const KVEvent& e, Type t = All, const TString& grp = "")
#endif
         : fIter(e.fParticles), fType(t), fIterating(kTRUE), fGroup(grp)
      {
         // Construct an iterator object to read in sequence the particles in event *e.
         // By default, opt="" and all particles are included in the iteration.
         // opt is a case-insensitive option controlling the iteration:
         //   opt="ok"/"OK":  only particles whose KVParticle::IsOK() method returns kTRUE
         //                   are iterated over
         //   opt=any other non-empty string:  only particles belonging to the given group
         //                                    i.e. KVParticle::BelongsToGroup(opt) returns
         //                                    kTRUE

         // set iterator to first particle of event corresponding to selection
         fIter.Begin();
         while ((current() != nullptr) && !AcceptableIteration()) ++fIter;
      }

      KVNucleus& operator* () const
      {
         // Returns reference to current particle in iteration

         return *(current());
      }
      template<typename PointerType>
      PointerType* get_pointer() const
      {
         return dynamic_cast<PointerType*>(current());
      }
      template<typename ReferenceType>
      ReferenceType& get_reference() const
      {
         return dynamic_cast<ReferenceType&>(*current());
      }
      Bool_t operator!= (const Iterator& it) const
      {
         // returns kTRUE if the 2 iterators are not pointing to the same particle
         return current() != it.current();
      }
      Bool_t operator== (const Iterator& it) const
      {
         // returns kTRUE if the 2 iterators are pointing to the same particle
         return current() == it.current();
      }
      const Iterator& operator++ ()
      {
         // Prefix ++ operator
         // Advance iterator to next particle in event compatible with selection
         if (current() == nullptr) fIterating = kFALSE;
         ++fIter;
         while (current() != nullptr && !AcceptableIteration()) ++fIter;
         return *this;
      }
      Iterator operator++ (int)
      {
         // Postfix ++ operator
         // Advance iterator to next particle in event compatible with selection
         Iterator tmp(*this);
         operator++();
         return tmp;
      }
      Iterator& operator= (const Iterator& rhs)
      {
         // copy-assignment operator
         if (this != &rhs) { // check self-assignment based on address of object
            fIter = rhs.fIter;
            fType = rhs.fType;
            fGroup = rhs.fGroup;
            fIterating = rhs.fIterating;
         }
         return *this;
      }

      static Iterator End()
      {
         return Iterator();
      }
      virtual ~Iterator() {}
#ifdef WITH_CPP11
      void Reset(Type t = Type::Null, TString grp = "")
#else
      void Reset(Type t = Null, TString grp = "")
#endif
      {
         // Reuse iterator, start iteration again
         // Reset() - use same selection criteria
         // Reset(Type t[, TString gr]) - change selection criteria
#ifdef WITH_CPP11
         if (t != Type::Null) {
#else
         if (t != Null) {
#endif
            fType = t;
            fGroup = grp;
         }
         fIter.Begin();
         fIterating = kTRUE;
         while ((current() != nullptr) && !AcceptableIteration()) ++fIter;
      }
      Bool_t IsIterating() const
      {
         // returns kTRUE if iteration is in progress
         return fIterating;
      }
      void SetIsIterating(Bool_t on = kTRUE)
      {
         // set fIterating flag
         fIterating = on;
      }

      ClassDef(Iterator, 0) //Iterator class for KVEvent
   };
protected:
   mutable Iterator fIter;//! internal iterator used by GetNextParticle()

public:
   Iterator GetNextParticleIterator(Option_t* opt) const;
   KVNameValueList* GetParameters() const
   {
      return (KVNameValueList*)&fParameters;
   }

   KVEvent(Int_t mult = 50, const char* classname = "KVNucleus");
   virtual ~ KVEvent();

#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   virtual void Copy(TObject& obj) const;
#else
   virtual void Copy(TObject& obj);
#endif

   KVNucleus* AddParticle();
   KVNucleus* GetParticle(Int_t npart) const;
   virtual Int_t GetMult(Option_t* opt = "") const;
   Int_t GetMultiplicity(Int_t Z, Int_t A = 0, Option_t* opt = "");
   void GetMultiplicities(Int_t mult[], const TString& species, Option_t* opt = "");
   Double_t GetSum(const Char_t* KVNucleus_method, Option_t* opt = "");
   Double_t GetSum(const Char_t* KVNucleus_method, const Char_t* method_prototype, const Char_t* args, Option_t* opt = "");
   void FillHisto(TH1* h, const Char_t* KVNucleus_method, Option_t* opt = "");
   void FillHisto(TH1* h, const Char_t* KVNucleus_method, const Char_t* method_prototype, const Char_t* args, Option_t* opt = "");
   virtual void Clear(Option_t* opt = "");
   virtual void Print(Option_t* t = "") const;
   virtual void ls(Option_t* t = "") const
   {
      Print(t);
   }
   KVNucleus* GetParticleWithName(const Char_t* name) const;
   KVNucleus* GetParticle(const Char_t* group_name) const;

   Iterator begin() const
   {
      // return iterator to beginning of event
      return Iterator(this);
   }
   Iterator end() const
   {
      // return iterator to end of event (a nullptr)
      return Iterator::End();
   }

   KVNucleus* GetNextParticle(Option_t* opt = "") const;
   void ResetGetNextParticle() const;

   void ResetEnergies();

   virtual Bool_t IsOK();
   void SetMinimumOKMultiplicity(Int_t);
   Int_t GetMinimumOKMultiplicity() const;

   void CustomStreamer()
   {
      fParticles->BypassStreamer(kFALSE);
   }

   void DefineGroup(const Char_t* groupname, const Char_t* from = "");
   void DefineGroup(const Char_t* groupname, KVParticleCondition* cond, const Char_t* from = "");

   void SetFrame(const Char_t* frame, const KVFrameTransform& ft);
   void SetFrame(const Char_t* newframe, const Char_t* oldframe, const KVFrameTransform& ft);
   void ChangeFrame(const KVFrameTransform&, const KVString& = "");
   void ChangeDefaultFrame(const Char_t*, const Char_t* defname = "");
   void UpdateAllFrames();

   virtual void FillArraysP(Int_t& mult, Int_t* Z, Int_t* A, Double_t* px, Double_t* py, Double_t* pz, const TString& frame = "", const TString& selection = "");
   virtual void FillArraysV(Int_t& mult, Int_t* Z, Int_t* A, Double_t* vx, Double_t* vy, Double_t* vz, const TString& frame = "", const TString& selection = "");
   virtual void FillArraysEThetaPhi(Int_t& mult, Int_t* Z, Int_t* A, Double_t* E, Double_t* Theta, Double_t* Phi, const TString& frame = "", const TString& selection = "");
   virtual void FillArraysPtRapPhi(Int_t& mult, Int_t* Z, Int_t* A, Double_t* Pt, Double_t* Rap, Double_t* Phi, const TString& frame = "", const TString& selection = "");

   virtual void FillIntegerList(KVIntegerList*, Option_t* opt);

   virtual void GetMasses(std::vector<Double_t>&);
   virtual void GetGSMasses(std::vector<Double_t>&);
   Double_t GetChannelQValue() const;
   Double_t GetGSChannelQValue() const;
   const Char_t* GetPartitionName();

// avoid breaking code with the change of the following interface
#define KVEVENT_MAKE_EVENT_BRANCH_NO_VOID_PTR 1
   static void MakeEventBranch(TTree* tree, const TString& branchname, const TString& classname, KVEvent* event, Int_t bufsize = 10000000)
   {
      // Use this method when adding a branch to a TTree to store KVEvent-derived objects.
      //
      // \param[in] tree pointer to TTree
      // \param[in] branchname name of branch to create
      // \param[in] classname name of actual class of object pointed to by event
      // \param[in] event pointer to a valid (constructed) KVEvent-derived object
      // \param[in] bufsize size of buffer to use for branch [default: 10000000]

      tree->Branch(branchname, classname, (void*)&event, bufsize, 0)->SetAutoDelete(kFALSE);
   }

   virtual void MergeEventFragments(TCollection*, Option_t* opt = "");
   static KVEvent* Factory(const char*);
   void SetFrameName(const KVString&);
   template<typename ValType> void SetParameter(const Char_t* name, ValType value) const
   {
      GetParameters()->SetValue(name, value);
   }
   const Char_t* GetFrameName() const
   {
      // Returns name of default kinematical frame for particles in event, if set
      // (see KVEvent::SetFrameName)

      return (GetParameters()->HasStringParameter("defaultFrame") ?
              GetParameters()->GetStringValue("defaultFrame") : "");
   }

   ClassDef(KVEvent, 4)         //Base class for all types of multiparticle event
};

struct EventIterator {
   KVEvent::Iterator it;
#ifdef WITH_CPP11
   EventIterator(KVEvent& event, KVEvent::Iterator::Type t = KVEvent::Iterator::Type::All, const TString& grp = "")
#else
   EventIterator(KVEvent& event, KVEvent::Iterator::Type t = KVEvent::Iterator::All, const TString& grp = "")
#endif
      : it(event, t, grp)
   {}
   KVEvent::Iterator begin() const
   {
      return it;
   }
   KVEvent::Iterator end() const
   {
      return KVEvent::Iterator::End();
   }
};

struct OKEventIterator : public EventIterator {
   OKEventIterator(KVEvent& event) :
#ifdef WITH_CPP11
      EventIterator(event, KVEvent::Iterator::Type::OK)
#else
      EventIterator(event, KVEvent::Iterator::OK)
#endif
   {}
};

struct GroupEventIterator : public EventIterator {
   GroupEventIterator(KVEvent& event, const TString& grp) :
#ifdef WITH_CPP11
      EventIterator(event, KVEvent::Iterator::Type::Group, grp)
#else
      EventIterator(event, KVEvent::Iterator::Group, grp)
#endif
   {}
};
#endif
