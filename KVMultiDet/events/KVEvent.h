#ifndef KVBASEEVENT_H
#define KVBASEEVENT_H

#include <KVBase.h>
#include <TTree.h>
#include "KVNameValueList.h"
#include "TClonesArray.h"
#include "KVNucleus.h"
class KVFrameTransform;

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
   TObject* ConstructedAt(Int_t idx)
   {
      // Get an object at index 'idx' that is guaranteed to have been constructed.
      // It might be either a freshly allocated object or one that had already been
      // allocated (and assumingly used).  In the later case, it is the callers
      // responsability to insure that the object is returned to a known state,
      // usually by calling the Clear method on the TClonesArray.
      //
      // Tests to see if the destructor has been called on the object.
      // If so, or if the object has never been constructed the class constructor is called using
      // New().  If not, return a pointer to the correct memory location.
      // This explicitly to deal with TObject classes that allocate memory
      // which will be reset (but not deallocated) in their Clear()
      // functions.

      TObject* obj = (*fParticles)[idx];
      if (obj && obj->TestBit(TObject::kNotDeleted)) {
         return obj;
      }
      return (fParticles->GetClass()) ? static_cast<TObject*>(fParticles->GetClass()->New(obj)) : 0;
   }
   //______________________________________________________________________________
   TObject* ConstructedAt(Int_t idx, Option_t* clear_options)
   {
      // Get an object at index 'idx' that is guaranteed to have been constructed.
      // It might be either a freshly allocated object or one that had already been
      // allocated (and assumingly used).  In the later case, the function Clear
      // will be called and passed the value of 'clear_options'
      //
      // Tests to see if the destructor has been called on the object.
      // If so, or if the object has never been constructed the class constructor is called using
      // New().  If not, return a pointer to the correct memory location.
      // This explicitly to deal with TObject classes that allocate memory
      // which will be reset (but not deallocated) in their Clear()
      // functions.

      TObject* obj = (*fParticles)[idx];
      if (obj && obj->TestBit(TObject::kNotDeleted)) {
         obj->Clear(clear_options);
         return obj;
      }
      return (fParticles->GetClass()) ? static_cast<TObject*>(fParticles->GetClass()->New(obj)) : 0;
   }
#endif

public:
   KVEvent(const TClass* particle_class, Int_t mult = 50)
      :  fParticles(new TClonesArray(particle_class, mult)),
         fParameters("EventParameters", "Parameters associated with an event")
   {
      CustomStreamer();
   }
   virtual ~KVEvent()
   {
      //Destructor. Destroys all objects stored in TClonesArray and releases
      //allocated memory.

      fParticles->Delete();
      SafeDelete(fParticles);
   }

   void Copy(TObject& obj) const
   {
      //Copy this to obj
      KVBase::Copy(obj);
      fParameters.Copy(((KVEvent&)obj).fParameters);
      Int_t MTOT = fParticles->GetEntriesFast();
      for (Int_t nn = 0; nn < MTOT; nn += 1) {
         GetParticle(nn + 1)->Copy(*((KVEvent&) obj).AddParticle());
      }
   }

   virtual Int_t GetMult(Option_t* = "") const
   {
      // Returns multiplicity (number of particles) of event.
      //
      // Optional argument not used here

      return fParticles->GetEntriesFast();
   }
   virtual KVNucleus* GetNextParticle(Option_t* = "") const = 0;
   virtual void ResetGetNextParticle() const = 0;
   virtual KVNucleus* GetParticle(Int_t npart) const = 0;
   virtual KVNucleus* AddParticle() = 0;
   virtual void SetFrame(const Char_t*, const KVFrameTransform&) = 0;
   virtual void SetFrame(const Char_t*, const Char_t*, const KVFrameTransform&) = 0;
   virtual Bool_t IsOK() const
   {
      // Returns kTRUE if the event is OK for analysis.
      // This means there must be at least MOKmin particles with IsOK()=kTRUE,
      // where MOKmin is set by calling SetMinimumOKMultiplicity(Int_t)
      // (value stored in parameter MIN_OK_MULT)

      return (GetMult("ok") >= GetMinimumOKMultiplicity());
   }
   void SetMinimumOKMultiplicity(Int_t x)
   {
      // Set minimum number of particles with IsOK()=kTRUE in event for
      // it to be considered 'good' for analysis
      SetParameter("MIN_OK_MULT", x);
   }
   Int_t GetMinimumOKMultiplicity() const
   {
      // Get minimum number of particles with IsOK()=kTRUE in event for
      // it to be considered 'good' for analysis
      // NB: if no minimum has been set, we return 1
      Int_t x = GetParameters()->GetIntValue("MIN_OK_MULT");
      if (x == -1) return 1;
      return x;
   }
   virtual void MergeEventFragments(TCollection* events, Option_t* opt = "")
   {
      // Merge all events in the list into one event (this one)
      // We also merge/sum the parameter lists of the events
      // First we clear this event, then we fill it with copies of each particle in each event
      // in the list.
      // If option "opt" is given, it is given as argument to each call to
      // KVEvent::Clear() - this option is then passed on to the Clear()
      // method of each particle in each event.
      // NOTE: the events in the list will be empty and useless after this!

      Clear(opt);
      TIter it(events);
      KVEvent* e;
      while ((e = (KVEvent*)it())) {
         KVNucleus* n;
         e->ResetGetNextParticle();
         while ((n = e->GetNextParticle())) {
            n->Copy(*AddParticle());
         }
         GetParameters()->Merge(*(e->GetParameters()));
         e->Clear(opt);
      }
   }

   void CustomStreamer()
   {
      fParticles->BypassStreamer(kFALSE);
   }

   KVNameValueList* GetParameters() const
   {
      return (KVNameValueList*)&fParameters;
   }

   virtual Double_t GetSum(const Char_t*, Option_t* = "") = 0;
   virtual Double_t GetChannelQValue() const = 0;
   virtual void SetFrameName(const KVString& name) = 0;
   const Char_t* GetFrameName() const
   {
      // Returns name of default kinematical frame for particles in event, if set
      // (see KVEvent::SetFrameName)

      return (GetParameters()->HasStringParameter("defaultFrame") ?
              GetParameters()->GetStringValue("defaultFrame") : "");
   }
   template<typename ValType> void SetParameter(const Char_t* name, ValType value) const
   {
      GetParameters()->SetValue(name, value);
   }
   virtual void GetMasses(std::vector<Double_t>&) = 0;

// avoid breaking code with the change of the following interface
#define KVEVENT_MAKE_EVENT_BRANCH_NO_VOID_PTR 1
   template<typename T>
   static void MakeEventBranch(TTree* tree, const TString& branchname, const TString& classname, T& event, Int_t bufsize = 10000000)
   {
      // Use this method when adding a branch to a TTree to store KVEvent-derived objects.
      //
      // \param[in] tree pointer to TTree
      // \param[in] branchname name of branch to create
      // \param[in] classname name of actual class of object pointed to by event
      // \param[in] event pointer to a valid (constructed) KVEvent-derived object
      // \param[in] bufsize size of buffer to use for branch [default: 10000000]

      tree->Branch(branchname, classname, &event, bufsize, 0)->SetAutoDelete(kFALSE);
   }
   static KVEvent* Factory(const char* plugin)
   {
      // Create and return pointer to new event of class given by plugin

      TPluginHandler* ph = LoadPlugin("KVEvent", plugin);
      if (ph) {
         return (KVEvent*)ph->ExecPlugin(0);
      }
      return nullptr;
   }
   void Clear(Option_t* opt = "")
   {
      // Reset the event to zero ready for new event.
      // Any option string is passed on to the Clear() method of the particle
      // objects in the TClonesArray fParticles.

      if (strcmp(opt, "")) {
         // pass options to particle class Clear() method
         TString Opt = Form("C+%s", opt);
         fParticles->Clear(Opt);
      }
      else
         fParticles->Clear("C");
      fParameters.Clear();
      ResetGetNextParticle();
   }

   ClassDef(KVEvent, 4)
};

#endif // KVBASEEVENT_H
