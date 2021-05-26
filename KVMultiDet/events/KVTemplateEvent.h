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
#include "KVEvent.h"
#include "KVConfig.h"
#include "TRotation.h"
#include "TLorentzRotation.h"
#include "KVParticleCondition.h"
#include "KVNameValueList.h"
#include "TMethodCall.h"

#include <KVFrameTransform.h>
#include <KVIntegerList.h>
#include <TH1.h>
#include <TObjString.h>
#include <iterator>
#include "KVNucleus.h"

class KVIntegerList;

/**
 \class KVTemplateEvent
 \brief Base class for event classes as containers of different types of nucleus objects
 \ingroup NucEvents
 */
template <typename Nucleus>
class KVTemplateEvent: public KVEvent {

public:
   /**
    \class Iterator
    \brief Class for iterating over nuclei in events
    \ingroup NucEvents
    */
   class Iterator {
   public:
      typedef std::forward_iterator_tag iterator_category;
      typedef Nucleus value_type;
      typedef std::ptrdiff_t difference_type;
      typedef Nucleus* pointer;
      typedef Nucleus& reference;

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
      Nucleus* current() const
      {
         // Returns pointer to current particle in iteration
         // Resets fIterating flag to kFALSE at end of list
         return static_cast<Nucleus*>(*fIter);
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
         : fIter(e->GetParticleArray()), fType(t), fIterating(kTRUE), fGroup(grp)
      {
         // Construct an iterator object to read in sequence the particles in event *e.
         // By default, opt="" and all particles are included in the iteration.
         // opt is a case-insensitive option controlling the iteration:
         //   opt="ok"/"OK":  only particles whose KVParticle::IsOK() method returns kTRUE
         //                   are iterated over
         //   opt=any other non-empty string:  only particles belonging to the given group
         //                                    i.e. KVParticle::BelongsToGroup(opt) returns
         //                                    kTRUE

         if (!e->GetParticleArray()->GetClass()->InheritsFrom(Nucleus::Class()))
            ::Warning("KVTemplateEvent::Iterator", "KVTemplateEvent<%s>::Iterator for %s nuclei requested for event containing %s nuclei",
                      Nucleus::Class()->GetName(), Nucleus::Class()->GetName(), e->GetParticleArray()->GetClass()->GetName());
         // set iterator to first particle of event corresponding to selection
         fIter.Begin();
         while ((current() != nullptr) && !AcceptableIteration()) ++fIter;
      }

#ifdef WITH_CPP11
      Iterator(const KVEvent& e, Type t = Type::All, const TString& grp = "")
#else
      Iterator(const KVEvent& e, Type t = All, const TString& grp = "")
#endif
         : fIter(e.GetParticleArray()), fType(t), fIterating(kTRUE), fGroup(grp)
      {
         // Construct an iterator object to read in sequence the particles in event *e.
         // By default, opt="" and all particles are included in the iteration.
         // opt is a case-insensitive option controlling the iteration:
         //   opt="ok"/"OK":  only particles whose KVParticle::IsOK() method returns kTRUE
         //                   are iterated over
         //   opt=any other non-empty string:  only particles belonging to the given group
         //                                    i.e. KVParticle::BelongsToGroup(opt) returns
         //                                    kTRUE

         if (!e.GetParticleArray()->GetClass()->InheritsFrom(Nucleus::Class()))
            ::Warning("KVTemplateEvent::Iterator", "KVTemplateEvent<%s>::Iterator for %s nuclei requested for event containing %s nuclei",
                      Nucleus::Class()->GetName(), Nucleus::Class()->GetName(), e.GetParticleArray()->GetClass()->GetName());
         // set iterator to first particle of event corresponding to selection
         fIter.Begin();
         while ((current() != nullptr) && !AcceptableIteration()) ++fIter;
      }

      Nucleus& operator* () const
      {
         // Returns reference to current particle in iteration

         return *(current());
      }
      template<typename PointerType = Nucleus>
      PointerType * get_pointer() const
      {
         return dynamic_cast<PointerType*>(current());
      }
      template<typename ReferenceType = Nucleus>
      ReferenceType & get_reference() const
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
   };
protected:
   mutable Iterator fIter;//! internal iterator used by GetNextParticle()

public:
   KVTemplateEvent(Int_t mult = 50)
      : KVEvent(Nucleus::Class(), mult)
   {}
   virtual ~ KVTemplateEvent()
   {}

   Nucleus* AddParticle()
   {
      // Add a nucleus to the event and return a pointer to it.
      //
      // All nuclei belong to the event and will be deleted either by the event
      // destructor or when method Clear() is called.

      Int_t mult = GetMult();
#ifdef __WITHOUT_TCA_CONSTRUCTED_AT
      Nucleus* tmp = (Nucleus*) ConstructedAt(mult, "C");
#else
      Nucleus* tmp = (Nucleus*) fParticles->ConstructedAt(mult, "C");
#endif
      if (!tmp) {
         Error("AddParticle", "Allocation failure, Mult=%d", mult);
         return 0;
      }
      return tmp;
   }
   Nucleus* GetParticle(Int_t npart) const
   {
      //Access to event member with index npart (1<=npart<=GetMult() : error if out of bounds)

      if (npart < 1 || npart > fParticles->GetEntriesFast()) {
         Error("GetParticle", KVEVENT_PART_INDEX_OOB, npart,
               fParticles->GetEntriesFast());
         return 0;
      }

      return (Nucleus*)((*fParticles)[npart - 1]);
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

      if (TString(opt) == "") return KVEvent::GetMult();
      Int_t fMultOK = 0;
      for (Iterator it = GetNextParticleIterator(opt); it != end(); ++it) ++fMultOK;
      return fMultOK;
   }
   Int_t GetMultiplicity(Int_t Z, Int_t A = 0, Option_t* opt = "")
   {
      // Calculate the multiplicity of nuclei given Z (if A not given)
      // or of nuclei with given Z & A (if given)
      //
      //If opt = "ok" only particles with IsOK()==kTRUE are considered.
      //If opt = "name" only particles belonging to group "name" are considered.

      if (A > 0) return (Int_t)GetSum("IsIsotope", "int,int", Form("%d,%d", Z, A), opt);
      return (Int_t)GetSum("IsElement", "int", Form("%d", Z), opt);
   }
   void GetMultiplicities(Int_t mult[], const TString& species, Option_t* opt = "")
   {
      // Fill array mult[] with the number of each nuclear species in the
      // comma-separated list in this event. Make sure that mult[] is
      // large enough for the list.
      //
      // Example:
      //   Int_t mult[4];
      //   event.GetMultiplicities(mult, "1n,1H,2H,3H");
      //
      // N.B. the species name must correspond to that given by KVNucleus::GetSymbol
      //
      // If given, "opt" will be used to select particles ("OK" or groupname)

      unique_ptr<TObjArray> spec(species.Tokenize(", "));// remove any spaces
      Int_t nspec = spec->GetEntries();
      memset(mult, 0, nspec * sizeof(Int_t)); // set multiplicities to zero
      for (Iterator it = GetNextParticleIterator(opt); it != end(); ++it) {
         for (int i = 0; i < nspec; i++) {
            if (((TObjString*)(*spec)[i])->String() == (*it).GetSymbol()) mult[i] += 1;
         }
      }
   }
   Double_t GetSum(const Char_t* Nucleus_method, Option_t* opt = "")
   {
      //Returns sum over particles of the observable given by the indicated Nucleus_method
      //for example
      //if  the method is called this way GetSum("GetZ"), it returns the sum of the charge
      //of particles in the current event
      //
      //If opt = "ok" only particles with IsOK()==kTRUE are considered.
      //If opt = "name" only particles belonging to group "name" are considered.

      Double_t fSum = 0;
      TMethodCall mt;
      mt.InitWithPrototype(Nucleus::Class(), Nucleus_method, "");

      if (mt.IsValid()) {
         Iterator it = GetNextParticleIterator(opt);
         if (mt.ReturnType() == TMethodCall::kLong) {
            Long_t ret;
            for (; it != end(); ++it) {
               Nucleus* tmp = it.get_pointer();
               mt.Execute(tmp, "", ret);
               fSum += ret;
            }
         }
         else if (mt.ReturnType() == TMethodCall::kDouble) {
            Double_t ret;
            for (; it != end(); ++it) {
               Nucleus* tmp = it.get_pointer();
               mt.Execute(tmp, "", ret);
               fSum += ret;
            }
         }
      }

      return fSum;
   }
   Double_t GetSum(const Char_t* Nucleus_method, const Char_t* method_prototype, const Char_t* args, Option_t* opt = "")
   {
      //Returns sum over particles of the observable given by the indicated Nucleus_method
      // with given prototype (e.g. method_prototype="int,int") and argument values
      // e.g. args="2,4")
      //
      //If opt = "ok" only particles with IsOK()==kTRUE are considered.
      //If opt = "name" only particles belonging to group "name" are considered.

      Double_t fSum = 0;
      TMethodCall mt;
      mt.InitWithPrototype(Nucleus::Class(), Nucleus_method, method_prototype);

      if (mt.IsValid()) {
         Iterator it = GetNextParticleIterator(opt);
         if (mt.ReturnType() == TMethodCall::kLong) {
            Long_t ret;
            for (; it != end(); ++it) {
               Nucleus* tmp = it.get_pointer();
               mt.Execute(tmp, args, ret);
               fSum += ret;
            }
         }
         else if (mt.ReturnType() == TMethodCall::kDouble) {
            Double_t ret;
            for (; it != end(); ++it) {
               Nucleus* tmp = it.get_pointer();
               mt.Execute(tmp, args, ret);
               fSum += ret;
            }
         }
      }

      return fSum;
   }
   void FillHisto(TH1* h, const Char_t* Nucleus_method, Option_t* opt = "")
   {
      // Fill histogram with values of the observable given by the indicated Nucleus_method.
      // For example: if  the method is called this way - FillHisto(h,"GetZ") - it fills histogram
      // with the charge of all particles in the current event.
      //
      // If opt = "ok" only particles with IsOK()==kTRUE are considered.
      // If opt = "name" only particles belonging to group "name" are considered.

      TMethodCall mt;
      mt.InitWithPrototype(Nucleus::Class(), Nucleus_method, "");

      if (mt.IsValid()) {
         Iterator it = GetNextParticleIterator(opt);
         if (mt.ReturnType() == TMethodCall::kLong) {
            Long_t ret;
            for (; it != end(); ++it) {
               Nucleus* tmp = it.get_pointer();
               mt.Execute(tmp, "", ret);
               h->Fill((Double_t)ret);
            }
         }
         else if (mt.ReturnType() == TMethodCall::kDouble) {
            Double_t ret;
            for (; it != end(); ++it) {
               Nucleus* tmp = it.get_pointer();
               mt.Execute(tmp, "", ret);
               h->Fill(ret);
            }
         }
      }
   }
   void FillHisto(TH1* h, const Char_t* Nucleus_method, const Char_t* method_prototype, const Char_t* args, Option_t* opt = "")
   {
      // Fill histogram with values of given method with given prototype (e.g. method_prototype="int,int") and argument values
      // e.g. args="2,4") for each particle in event.
      //
      // If opt = "ok" only particles with IsOK()==kTRUE are considered.
      // If opt = "name" only particles belonging to group "name" are considered.

      TMethodCall mt;
      mt.InitWithPrototype(Nucleus::Class(), Nucleus_method, method_prototype);

      if (mt.IsValid()) {
         Iterator it = GetNextParticleIterator(opt);
         if (mt.ReturnType() == TMethodCall::kLong) {
            Long_t ret;
            for (; it != end(); ++it) {
               Nucleus* tmp = it.get_pointer();
               mt.Execute(tmp, args, ret);
               h->Fill((Double_t)ret);
            }
         }
         else if (mt.ReturnType() == TMethodCall::kDouble) {
            Double_t ret;
            for (; it != end(); ++it) {
               Nucleus* tmp = it.get_pointer();
               mt.Execute(tmp, args, ret);
               h->Fill(ret);
            }
         }
      }
   }
   virtual void Print(Option_t* t = "") const
   {
      //Print a list of all particles in the event with some characteristics.
      //Optional argument t can be used to select particles (="ok", "groupname", ...)

      std::cout << "\nKVEvent with " << GetMult(t) << " particles :" << std::endl;
      std::cout << "------------------------------------" << std::endl;
      fParameters.Print();
      for (Iterator it = GetNextParticleIterator(t); it != end(); ++it)(*it).Print();
   }
   virtual void ls(Option_t* t = "") const
   {
      Print(t);
   }
   Nucleus* GetParticleWithName(const Char_t* name) const
   {
      //Find particle using its name (SetName()/GetName() methods)
      //In case more than one particle has the same name, the first one found is returned.

      Nucleus* tmp = (Nucleus*)fParticles->FindObject(name);
      return tmp;
   }
   Nucleus* GetParticle(const Char_t* group_name) const
   {
      // Find first particle in event belonging to group with name "group_name"

      Iterator it = GetNextParticleIterator(group_name);
      Nucleus* tmp = it.get_pointer();
      if (!tmp) Warning("GetParticle", "Particle not found: %s", group_name);
      return tmp;
   }

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

   Nucleus* GetNextParticle(Option_t* opt = "") const
   {
      // Use this method to iterate over the list of particles in the event
      // After the last particle GetNextParticle() returns a null pointer and
      // resets itself ready for a new iteration over the particle list.
      //
      // If opt="" all particles are included in the iteration.
      // If opt="ok" or "OK" only particles whose IsOK() method returns kTRUE are included.
      //
      // Any other value of opt is interpreted as a (case-insensitive) particle group name: only
      // particles with BelongsToGroup(opt) returning kTRUE are included.
      //
      // If you want to start from the beginning again before getting to the end
      // of the list, especially if you want to change the selection criteria,
      // call method ResetGetNextParticle() before continuing.
      // If you interrupt an iteration before the end, then start another iteration
      // without calling ResetGetNextParticle(), even if you change the argument of
      // the call to GetNextParticle(), you will repeat exactly the same iteration
      // as the previous one.
      //
      // WARNING: Only one iteration at a time over the event can be performed
      //          using this method. If you want/need to perform several i.e. nested
      //          iterations, use the KV*Event::Iterator classes

      TString Opt(opt);
      Opt.ToUpper();

      // continue iteration
      if (fIter.IsIterating()) return &(*(fIter++));

      // start new iteration
      Bool_t ok_iter = (Opt == "OK");
      Bool_t grp_iter = (!ok_iter && Opt.Length());

#ifdef WITH_CPP11
      if (ok_iter) fIter = Iterator(this, Iterator::Type::OK);
      else if (grp_iter) fIter = Iterator(this, Iterator::Type::Group, Opt);
      else fIter = Iterator(this, Iterator::Type::All);
#else
      if (ok_iter) fIter = Iterator(this, Iterator::OK);
      else if (grp_iter) fIter = Iterator(this, Iterator::Group, Opt);
      else fIter = Iterator(this, Iterator::All);
#endif
      return &(*(fIter++));
   }
   void ResetGetNextParticle() const
   {
      // Reset iteration over event particles so that next call
      // to GetNextParticle will begin a new iteration (possibly with
      // different criteria).

      fIter.SetIsIterating(kFALSE);
   }

   void ResetEnergies()
   {
      //Used for simulated events after "detection" by some multidetector array.
      //
      //The passage of the event's particles through the different absorbers modifies their
      //kinetic energies, indeed all those which are correctly identified by the detector
      //actually stop. Calling this method will reset all the particles' energies to their
      //initial value i.e. before they entered the first absorber.
      //Particles which have not encountered any absorbers/detectors are left as they are.

      for (Iterator it = begin(); it != end(); ++it) {
         (*it).ResetEnergy();
      }
   }

   void DefineGroup(const Char_t* groupname, const Char_t* from = "")
   {
      //In the same philosophy as KVINDRAReconEvent::AcceptIDCodes() method
      // allow to affiliate a group name to particles of the event
      // if "from" is not null, a test of previously stored group name
      // such as "OK" is checked

      for (Iterator it = GetNextParticleIterator(from); it != end(); ++it) {
         (*it).AddGroup(groupname);
      }
   }
   void DefineGroup(const Char_t* groupname, KVParticleCondition* cond, const Char_t* from = "")
   {
      // allow to affiliate using a KVParticleCondition a group name to particles of the event
      // if "from" is not null, a test of previously stored group name
      // such as "OK" is checked
      // the method used in KVParticleCondition has to be compatible with the KVNucleus
      // concerned class.
      // This can be done using first KVParticleCondition::SetParticleClassName(const Char_t* cl)

      for (Iterator it = GetNextParticleIterator(from); it != end(); ++it) {
         (*it).AddGroup(groupname, cond);
      }
   }

   void SetFrame(const Char_t* frame, const KVFrameTransform& ft)
   {
      //Define a Lorentz-boosted and/or rotated frame for all "ok" particles in the event.
      //See KVParticle::SetFrame() for details.
      //
      //In order to access the kinematics in the boosted frame, use the GetFrame() method of the
      //individual particles (see KVParticle::GetFrame()).

      for (Iterator it = GetNextParticleIterator("ok"); it != end(); ++it) {
         (*it).SetFrame(frame, ft);
      }
   }
   void SetFrame(const Char_t* newframe, const Char_t* oldframe, const KVFrameTransform& ft)
   {
      //Define a Lorentz-boosted frame "newframe" for all "ok" particles in the event.
      //The transformation is applied to the particle coordinates in the existing frame "oldframe"
      //
      //See KVParticle::SetFrame() for details.
      //
      //In order to access the kinematics in the boosted frame, use the GetFrame() method of the
      //individual particles in either of these ways :
      //      KVParticle* newframe = particle->GetFrame("newframe");
      //      KVParticle* newframe = particle->GetFrame("oldframe")->GetFrame("newframe");

      for (Iterator it = GetNextParticleIterator("ok"); it != end(); ++it) {
         (*it).SetFrame(newframe, oldframe, ft);
      }
   }
   void ChangeFrame(const KVFrameTransform& ft, const KVString& name = "")
   {
      //Permanently change the reference frame used for particle kinematics in the event.
      //The transformation is applied to all "ok" particles in the event.
      //You can optionally set the name of this new default kinematical reference frame.
      //
      //See KVParticle::ChangeFrame() and KVParticle::SetFrame() for details.


      for (Iterator it = GetNextParticleIterator("ok"); it != end(); ++it) {
         (*it).ChangeFrame(ft, name);
      }
      if (name != "") SetParameter("defaultFrame", name);
   }
   void ChangeDefaultFrame(const Char_t* newdef, const Char_t* defname = "")
   {
      // Make existing reference frame 'newdef' the new default frame for particle kinematics.
      // The current default frame will then be accessible from the list of frames
      // using its name (previously set with SetFrameName). You can change this name with 'defname'.
      //
      //See KVParticle::ChangeDefaultFrame() and KVParticle::SetFrame() for details.

      for (Iterator it = GetNextParticleIterator("ok"); it != end(); ++it) {
         (*it).ChangeDefaultFrame(newdef, defname);
      }
      SetParameter("defaultFrame", newdef);
   }
   void UpdateAllFrames()
   {
      // If the kinematics of particles in their default reference frame have been modified,
      // call this method to update the kinematics in all defined reference frames.
      //
      //See KVParticle::UpdateAllFrames() for details.

      for (Iterator it = GetNextParticleIterator("ok"); it != end(); ++it) {
         (*it).UpdateAllFrames();
      }
   }

   virtual void FillArraysP(Int_t& mult, Int_t* Z, Int_t* A, Double_t* px, Double_t* py, Double_t* pz, const TString& frame = "", const TString& selection = "")
   {
      // "Translate" this event into a simple array form
      // mult will be set to number of nuclei in event
      // (px,py,pz) momentum components in MeV/c
      // frame = optional name of reference frame (see SetFrame methods)
      // selection = selection i.e. "OK"

      Int_t i = 0;
      for (Iterator it = GetNextParticleIterator(selection); it != end(); ++it) {
         Nucleus* nuc = (Nucleus*)(*it).GetFrame(frame, kFALSE);
         Z[i] = nuc->GetZ();
         A[i] = nuc->GetA();
         px[i] = nuc->Px();
         py[i] = nuc->Py();
         pz[i] = nuc->Pz();
         i++;
      }
      mult = i;
   }
   virtual void FillArraysV(Int_t& mult, Int_t* Z, Int_t* A, Double_t* vx, Double_t* vy, Double_t* vz, const TString& frame = "", const TString& selection = "")
   {
      // "Translate" this event into a simple array form
      // mult will be set to number of nuclei in event
      // (vx,vy,vz) velocity components in cm/ns
      // frame = optional name of reference frame (see SetFrame methods)
      // selection = optional selection e.g. "OK"

      Int_t i = 0;
      for (Iterator it = GetNextParticleIterator(selection); it != end(); ++it) {
         Nucleus* nuc = (Nucleus*)(*it).GetFrame(frame, kFALSE);
         Z[i] = nuc->GetZ();
         A[i] = nuc->GetA();
         vx[i] = nuc->GetVelocity().X();
         vy[i] = nuc->GetVelocity().Y();
         vz[i] = nuc->GetVelocity().Z();
         i++;
      }
      mult = i;
   }
   virtual void FillArraysEThetaPhi(Int_t& mult, Int_t* Z, Int_t* A, Double_t* E, Double_t* Theta, Double_t* Phi, const TString& frame = "", const TString& selection = "")
   {
      // "Translate" this event into a simple array form
      // mult will be set to number of nuclei in event
      // E = kinetic energy in MeV
      // Theta,Phi in degrees
      // frame = optional name of reference frame (see SetFrame methods)
      // selection = optional selection e.g. "OK"

      Int_t i = 0;
      for (Iterator it = GetNextParticleIterator(selection); it != end(); ++it) {
         Nucleus* nuc = (Nucleus*)(*it).GetFrame(frame, kFALSE);
         Z[i] = nuc->GetZ();
         A[i] = nuc->GetA();
         E[i] = nuc->GetEnergy();
         Theta[i] = nuc->GetTheta();
         Phi[i] = nuc->GetPhi();
         i++;
      }
      mult = i;
   }
   virtual void FillArraysPtRapPhi(Int_t& mult, Int_t* Z, Int_t* A, Double_t* Pt, Double_t* Rap, Double_t* Phi, const TString& frame = "", const TString& selection = "")
   {
      // "Translate" this event into a simple array form
      // mult will be set to number of nuclei in event
      // Pt = transverse momentum (perpendicular to z-axis)
      // Rap = rapidity along z-axis
      // phi = azimuthal angle around z-axis (x-axis=0 deg.)
      // frame = optional name of reference frame (see SetFrame methods)
      // selection = optional selection e.g. "OK"

      Int_t i = 0;
      for (Iterator it = GetNextParticleIterator(selection); it != end(); ++it) {
         Nucleus* nuc = (Nucleus*)(*it).GetFrame(frame, kFALSE);
         Z[i] = nuc->GetZ();
         A[i] = nuc->GetA();
         Pt[i] = nuc->Pt();
         Rap[i] = nuc->Rapidity();
         Phi[i] = nuc->GetPhi();
         i++;
      }
      mult = i;
   }

   void FillIntegerList(KVIntegerList* IL, Option_t* opt)
   {
      // Clear & fill the KVIntegerList with the contents of this event,
      // the option will be passed to GetNextParticle(opt).
      // IntegerList is then 'Update()'d.
      // (This method was originally KVIntegerList::Fill(KVEvent*,Option_t*),
      // it was moved here in order to make KVIntegerList a base class)

      IL->Clear();
      for (Iterator it = GetNextParticleIterator(opt); it != end(); ++it) IL->Add((*it).GetZ());
      IL->SetPopulation(1);
      IL->CheckForUpdate();
   }

   void GetMasses(std::vector<Double_t>& mass)
   {
      // Fill vector with mass of each nucleus of event (in MeV) [note: this is the mass including any excitation energy, not ground state]

      mass.clear();
      mass.reserve(GetMult());
      int i = 0;
      for (Iterator it = begin(); it != end(); ++it) mass[i++] = (*it).GetMass();
   }
   void GetGSMasses(std::vector<Double_t>& mass)
   {
      // Fill vector with ground state mass of each nucleus of event (in MeV).

      mass.clear();
      mass.reserve(GetMult());
      int i = 0;
      for (Iterator it = begin(); it != end(); ++it) mass[i++] = (*it).GetMassGS();
   }
   Double_t GetChannelQValue() const
   {
      // Calculate the Q-value [MeV] for this event as if all nuclei were produced by
      // the decay of an initial compound nucleus containing the sum of all nuclei
      // in the event, i.e.
      //    A -> a1 + a2 + a3 + ...
      // We take into account any excitation energy of the nuclei of the event
      // (see GetGSChannelQValue() for an alternative), i.e. we calculate
      //    Q = M(A) - ( m(a1) + m(a2) + m(a3) + ... )
      // where
      //   M(X) = ground state mass of X
      //   m(X) = M(X) + E*(X)
      // If Q<0, the excitation energy of the initial compound nucleus, A,
      // would have to be at least equal to (-Q) in order for the decay to occur.
      // i.e. decay is possible if
      //   E*(A) > -Q

      Double_t sumM = 0;
      Nucleus CN;
      Int_t M = GetMult();
      for (int i = 1; i <= M; i++) {
         sumM += GetParticle(i)->GetMass();
         CN += *(GetParticle(i));
      }
      return CN.GetMassGS() - sumM;
   }
   Double_t GetGSChannelQValue() const
   {
      // Calculate the Q-value [MeV] for this event as if all nuclei were produced by
      // the decay of an initial compound nucleus containing the sum of all nuclei
      // in the event, i.e.
      //    A -> a1 + a2 + a3 + ...
      // i.e. we calculate
      //    Q = M(A) - ( M(a1) + M(a2) + M(a3) + ... )
      // where
      //   M(X) = ground state mass of X
      // If Q<0, the excitation energy of the initial compound nucleus, A,
      // would have to be at least equal to (-Q) in order for the decay to occur.
      // i.e. decay is possible if
      //   E*(A) > -Q

      Double_t sumM = 0;
      Nucleus CN;
      Int_t M = GetMult();
      for (int i = 1; i <= M; i++) {
         sumM += GetParticle(i)->GetMassGS();
         CN += *(GetParticle(i));
      }
      return CN.GetMassGS() - sumM;
   }
   KVString GetPartitionName()
   {
      //
      //return list of isotopes of the event with the format :
      // symbol1(population1) symbol2(population2) ....
      // if population==1, it is not indicated :
      // Example :
      // 15C 12C(2) 4He 3He 1H(4) 1n(3)
      //
      fParticles->Sort();
      KVString partition;

      KVNameValueList nvl;
      partition = "";
      for (Iterator it = begin(); it != end(); ++it) {
         TString st = (*it).GetSymbol();
         Int_t pop = TMath::Max(nvl.GetIntValue(st.Data()), 0);
         pop += 1;
         nvl.SetValue(st.Data(), pop);
      }
      for (Int_t ii = 0; ii < nvl.GetEntries(); ii += 1) {
         Int_t pop = nvl.GetIntValue(ii);
         if (pop == 1) partition += nvl.GetNameAt(ii);
         else        partition += Form("%s(%d)", nvl.GetNameAt(ii), pop);
         if (ii < nvl.GetEntries() - 1) partition += " ";
      }
      return partition;
   }

   void SetFrameName(const KVString& name)
   {
      // Set name of default frame for all particles in event
      // After using this method, calls to
      //    KVParticle::GetFrame(name)
      // will return the address of the particle in question, i.e.
      // its default kinematics
      // The default frame name is stored as a parameter "defaultFrame"

      for (Iterator it = begin(); it != end(); ++it) {
         (*it).SetFrameName(name);
      }
      SetParameter("defaultFrame", name);
   }

   /**
    \class EventIterator
    \brief Class for iterating over nuclei in events accessed through base pointer/reference
    \ingroup NucEvents

    Iterators are not defined for the abstract base class KVEvent. This class is a wrapper for
    the Iterator class which allows to use iterators with events passed as base references or pointers:

    ~~~~{.cpp}
    KVEvent* event; // pointer to valid event object

    for(auto& nuc : EventIterator(event)) { // loop over nuclei in event }
    ~~~~
    */
   struct EventIterator {
      Iterator it;
#ifdef WITH_CPP11
      EventIterator(const KVEvent& event, typename Iterator::Type t = Iterator::Type::All, const TString& grp = "")
#else
      EventIterator(const KVEvent& event, typename Iterator::Type t = Iterator::All, const TString& grp = "")
#endif
         : it(event, t, grp)
      {}
#ifdef WITH_CPP11
      EventIterator(const KVEvent* event, typename Iterator::Type t = Iterator::Type::All, const TString& grp = "")
#else
      EventIterator(const KVEvent* event, typename Iterator::Type t = Iterator::All, const TString& grp = "")
#endif
         : it(event, t, grp)
      {}
      Iterator begin() const
      {
         return it;
      }
      Iterator end() const
      {
         return Iterator::End();
      }
   };

   /**
    \class EventOKIterator
    \brief Class for iterating over "OK" nuclei in events accessed through base pointer/reference
    \ingroup NucEvents

    Iterators are not defined for the abstract base class KVEvent. This class is a wrapper for
    the Iterator class which allows to use iterators with events passed as base references or pointers:

    ~~~~{.cpp}
    KVEvent* event; // pointer to valid event object

    for(auto& nuc : EventOKIterator(event)) { // loop over OK nuclei in event }
    ~~~~
    */
   struct EventOKIterator : public EventIterator {
      EventOKIterator(const KVEvent& event) :
#ifdef WITH_CPP11
         EventIterator(event, Iterator::Type::OK)
#else
         EventIterator(event, Iterator::OK)
#endif
      {}
      EventOKIterator(const KVEvent* event) :
#ifdef WITH_CPP11
         EventIterator(event, Iterator::Type::OK)
#else
         EventIterator(event, Iterator::OK)
#endif
      {}
   };

   /**
    \class EventGroupIterator
    \brief Class for iterating over nuclei of given group in events accessed through base pointer/reference
    \ingroup NucEvents

    Iterators are not defined for the abstract base class KVEvent. This class is a wrapper for
    the Iterator class which allows to use iterators with events passed as base references or pointers:

    ~~~~{.cpp}
    KVEvent* event; // pointer to valid event object

    for(auto& nuc : EventGroupIterator(event,"groupname")) { // loop over nuclei of group "groupname" in event }
    ~~~~
    */
   struct EventGroupIterator : public EventIterator {
      EventGroupIterator(const KVEvent& event, const TString& grp) :
#ifdef WITH_CPP11
         EventIterator(event, Iterator::Type::Group, grp)
#else
         EventIterator(event, Iterator::Group, grp)
#endif
      {}
      EventGroupIterator(const KVEvent* event, const TString& grp) :
#ifdef WITH_CPP11
         EventIterator(event, Iterator::Type::Group, grp)
#else
         EventIterator(event, Iterator::Group, grp)
#endif
      {}
   };

   Iterator GetNextParticleIterator(Option_t* opt) const
   {
      // Provide correct iterator using same options as for GetNextParticle() method:
      //
      //  - if opt="" (default) => iterator over all particles
      //  - if opt="ok"/"OK" => iterator for all "OK" particles
      //  - if opt!="" && opt!="ok"/"OK" => iterator for all particles in group with name given by opt (case-insensitive)

      TString Opt(opt);
      Opt.ToUpper();
      Bool_t ok_iter = (Opt == "OK");
      Bool_t grp_iter = (!ok_iter && Opt.Length());

      if (ok_iter) return EventOKIterator(this).begin();
      else if (grp_iter) return EventGroupIterator(this, Opt).begin();
      return EventIterator(this).begin();
   }

   ClassDef(KVTemplateEvent, 0)         //Base class for all types of multiparticle event
};

typedef KVTemplateEvent<KVNucleus> KVNucleusEvent;

/**
  \class KVNucleusEvent
  \brief A container for KVNucleus objects
    \ingroup NucEvents

  This is a typedef for KVTemplateEvent<KVNucleus>.
 */
/**
  \class KVNucleusEvent::Iterator
  \brief Iterate over the KVNucleus objects contained in a KVNucleusEvent
    \ingroup NucEvents

 */
typedef KVTemplateEvent<KVNucleus>::EventIterator EventIterator;
typedef KVTemplateEvent<KVNucleus>::EventGroupIterator EventGroupIterator;
typedef KVTemplateEvent<KVNucleus>::EventOKIterator EventOKIterator;
#endif
