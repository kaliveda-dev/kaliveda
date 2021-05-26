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
  \brief Abstract base class container for multi-particle events
  \ingroup NucEvents

This base class defines the basic functionality and interface for all event classes, which in addition to handling a collection
of particles/nuclei, also have in common the following functionality:

 - an associated list of parameters, accessible through the GetParameters() and SetParameter() methods;
 - iterators for looping over all or a subset of the particles of the event;
 - methods for defining named subsets ('groups') of particles according to various selection criteria;
 - methods for defining/modifying different relativistic reference frames in which to 'view' the particles of the event

Concrete implementations of event classes derive from child class KVTemplateEvent.

\sa KVTemplateEvent

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
      // Copy this event into the object referenced by obj,
      // assumed to be at least derived from KVEvent.
      KVBase::Copy(obj);
      fParameters.Copy(((KVEvent&)obj).fParameters);
      Int_t MTOT = fParticles->GetEntriesFast();
      for (Int_t nn = 0; nn < MTOT; nn += 1) {
         GetParticle(nn + 1)->Copy(*((KVEvent&) obj).AddParticle());
      }
   }

   const TClonesArray* GetParticleArray() const
   {
      return fParticles;
   }
   virtual Int_t GetMult(Option_t* opt = "") const
   {
      // Returns multiplicity (number of particles) in event.
      //
      // \param[in] opt optional argument which may limit multiplicity to certain nuclei:
      //              - opt="" (default): all nuclei of event are counted
      //              - opt="OK": only nuclei for which KVNucleus::IsOK() returns true are counted
      //              - opt="group": only nuclei belonging to given group are counted
      //
      // \note Any value given for opt is case-insensitive

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
      //
      // This means there must be at least MOKmin particles for which KVNucleus::IsOK() returns kTRUE,
      // where MOKmin is set by calling SetMinimumOKMultiplicity()
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
      //
      // We also merge/sum the parameter lists of the events
      //
      // First we clear this event, then we fill it with copies of each particle in each event
      // in the list.
      //
      // If option "opt" is given, it is given as argument to each call to
      // KVEvent::Clear() - this option is then passed on to the KVNucleus::Clear()
      // method of each particle in each event.
      //
      // \param[in] events A list of events to merge
      // \param[in] opt Optional argument transmitted to KVEvent::Clear()
      //
      // \note the events in the list will be empty and useless after this!

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
   virtual KVString GetPartitionName() = 0;
   virtual void SetFrameName(const KVString& name) = 0;
   virtual void ChangeDefaultFrame(const Char_t*, const Char_t* = "") = 0;
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
      //
      // \param[in] opt option string passed on to the Clear() method of the particle objects in the TClonesArray fParticles

      if (strcmp(opt, "")) { // pass options to particle class Clear() method
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
