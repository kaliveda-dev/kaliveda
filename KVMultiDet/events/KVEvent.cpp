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

#include "Riostream.h"
#include "TClass.h"
#include "KVEvent.h"
#include "TStreamerInfo.h"
#include "KVParticleCondition.h"
#include "TClass.h"
#include "KVIntegerList.h"

using namespace std;

ClassImp(KVEvent);

////////////////////////////////////////////////////////////////////////////////
/*
<h2>KVEvent</h2>

*/
/////////////////////////////////////////////////////////////////////////////://


KVEvent::Iterator KVEvent::GetNextParticleIterator(Option_t* opt) const
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
   KVEvent& event = const_cast<KVEvent&>(*this);
   if (ok_iter) return OKEventIterator(event).begin();
   else if (grp_iter) return GroupEventIterator(event, Opt).begin();
   return EventIterator(event).begin();
}

KVEvent::KVEvent(Int_t mult, const char* classname)
   : fParameters("EventParameters", "Parameters associated with an event")
{
   //Initialise KVEvent to hold mult events of "classname" objects
   //(the class must inherit from KVNucleus).
   //"mult" is the approximate maximum multiplicity of the events (if too
   //small, extra space will be allocated automatically by TClonesArray).
   //
   // Default argument :
   //     classname = "KVNucleus"
   //

   fParticles = new TClonesArray(classname, mult);
   CustomStreamer();//force use of KVEvent::Streamer function for reading/writing
}

//_______________________________________________________________________________

KVEvent::~KVEvent()
{
   //Destructor. Destroys all objects stored in TClonesArray and releases
   //allocated memory.

   fParticles->Delete();
   SafeDelete(fParticles);
}

//_______________________________________________________________________________

#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
void KVEvent::Copy(TObject& obj) const
#else
void KVEvent::Copy(TObject& obj)
#endif
{
   //Copy this to obj
   KVBase::Copy(obj);
   fParameters.Copy(((KVEvent&)obj).fParameters);
   for (Int_t nn = 0; nn < fParticles->GetEntriesFast(); nn += 1) {
      //printf("avant=%s - ",GetParticle(nn+1)->GetName());
      GetParticle(nn + 1)->Copy(*((KVEvent&) obj).AddParticle());
      //printf("apres=%s\n",((KVEvent &) obj).GetParticle(nn+1)->GetName());
   }
}
//_______________________________________________________________________________

KVNucleus* KVEvent::GetParticle(Int_t npart) const
{
   //Access to event member with index npart (1<=npart<=GetMult() : error if out of bounds)
   //This method may be overridden in event classes derived from KVEvent.

   if (npart < 1 || npart > fParticles->GetEntriesFast()) {
      Error("GetParticle", KVEVENT_PART_INDEX_OOB, npart,
            fParticles->GetEntriesFast());
      return 0;
   }

   return (KVNucleus*)((*fParticles)[npart - 1]);
}

//________________________________________________________________________________

KVNucleus* KVEvent::AddParticle()
{
   // Method used for building an event particle by particle.
   // DO NOT USE FOR READING EVENTS - use GetParticle(Int_t npart)!!
   //
   // This method increases the multiplicity fMult by one and "creates" a new particle with index (fMult-1).
   // In actual fact a new object is only created if needed i.e. if the new multiplicity is greater
   // than previously. Particle objects in the array are reused from one event to another.
   // The default constructor for the class corresponding to the "new" particle will only be called
   // once during its lifetime (i.e. if N events are generated, the particle ctor will be called only once,
   // not N times). Once created, in subsequent events we just call the particle's Clear() method
   // in order to reset its internal variables ready for a new event.

   Int_t mult = GetMult();
#ifdef __WITHOUT_TCA_CONSTRUCTED_AT
   KVNucleus* tmp = (KVNucleus*) ConstructedAt(mult, "C");
#else
   KVNucleus* tmp = (KVNucleus*) fParticles->ConstructedAt(mult, "C");
#endif
   if (!tmp) {
      Error("AddParticle", "Allocation failure, Mult=%d", mult);
      return 0;
   }
   return tmp;
}

//________________________________________________________________________________

void KVEvent::Clear(Option_t* opt)
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

//________________________________________________________________________________

void KVEvent::Print(Option_t* t) const
{
   //Print a list of all particles in the event with some characteristics.
   //Optional argument t can be used to select particles (="ok", "groupname", ...)

   cout << "\nKVEvent with " << GetMult(t) << " particles :" << endl;
   cout << "------------------------------------" << endl;
   fParameters.Print();
   for (Iterator it = GetNextParticleIterator(t); it != end(); ++it)(*it).Print();
}

//________________________________________________________________________________

KVNucleus* KVEvent::GetParticleWithName(const Char_t* name) const
{
   //Find particle using its name (SetName()/GetName() methods)
   //In case more than one particle has the same name, the first one found is returned.

   KVNucleus* tmp = (KVNucleus*)fParticles->FindObject(name);
   return tmp;
}

//________________________________________________________________________________

KVNucleus* KVEvent::GetParticle(const Char_t* group_name) const
{
   // Find first particle in event belonging to group with name "group_name"

   Iterator it = GetNextParticleIterator(group_name);
   KVNucleus* tmp = it.get_pointer<KVNucleus>();
   if (!tmp) Warning("GetParticle", "Particle not found: %s", group_name);
   return tmp;
}

//__________________________________________________________________________________

Int_t KVEvent::GetMult(Option_t* opt) const
{
   //Returns multiplicity (number of particles) of event.
   //If opt = "" (default), returns number of particles in TClonesArray* fParticles
   // i.e. the value of fParticles->GetEntriesFast() (we assume there are no gaps
   // in the list)
   //If opt = "ok" only particles with IsOK()==kTRUE are included.
   //If opt = "name" only particles belonging to group "name" are included.

   Int_t fMultOK = 0;
   if (TString(opt) == "") {
      //get total multiplicity of event
      return fParticles->GetEntriesFast();
   }
   else {
      for (Iterator it = GetNextParticleIterator(opt); it != end(); ++it) ++fMultOK;
   }
   return fMultOK;
}

//__________________________________________________________________________________

Double_t KVEvent::GetSum(const Char_t* KVNucleus_method, Option_t* opt)
{
   //Returns sum over particles of the observable given by the indicated KVNucleus_method
   //for example
   //if  the method is called this way GetSum("GetZ"), it returns the sum of the charge
   //of particles in the current event
   //
   //If opt = "ok" only particles with IsOK()==kTRUE are considered.
   //If opt = "name" only particles belonging to group "name" are considered.

   Double_t fSum = 0;
   TMethodCall mt;
   mt.InitWithPrototype(KVNucleus::Class(), KVNucleus_method, "");

   if (mt.IsValid()) {
      Iterator it = GetNextParticleIterator(opt);
      if (mt.ReturnType() == TMethodCall::kLong) {
         Long_t ret;
         for (; it != end(); ++it) {
            KVNucleus* tmp = it.get_pointer<KVNucleus>();
            mt.Execute(tmp, "", ret);
            fSum += ret;
         }
      }
      else if (mt.ReturnType() == TMethodCall::kDouble) {
         Double_t ret;
         for (; it != end(); ++it) {
            KVNucleus* tmp = it.get_pointer<KVNucleus>();
            mt.Execute(tmp, "", ret);
            fSum += ret;
         }
      }
   }

   return fSum;
}

Double_t KVEvent::GetSum(const Char_t* KVNucleus_method, const Char_t* method_prototype, const Char_t* args, Option_t* opt)
{
   //Returns sum over particles of the observable given by the indicated KVNucleus_method
   // with given prototype (e.g. method_prototype="int,int") and argument values
   // e.g. args="2,4")
   //
   //If opt = "ok" only particles with IsOK()==kTRUE are considered.
   //If opt = "name" only particles belonging to group "name" are considered.

   Double_t fSum = 0;
   TMethodCall mt;
   mt.InitWithPrototype(KVNucleus::Class(), KVNucleus_method, method_prototype);

   if (mt.IsValid()) {
      Iterator it = GetNextParticleIterator(opt);
      if (mt.ReturnType() == TMethodCall::kLong) {
         Long_t ret;
         for (; it != end(); ++it) {
            KVNucleus* tmp = it.get_pointer<KVNucleus>();
            mt.Execute(tmp, args, ret);
            fSum += ret;
         }
      }
      else if (mt.ReturnType() == TMethodCall::kDouble) {
         Double_t ret;
         for (; it != end(); ++it) {
            KVNucleus* tmp = it.get_pointer<KVNucleus>();
            mt.Execute(tmp, args, ret);
            fSum += ret;
         }
      }
   }

   return fSum;
}

void KVEvent::FillHisto(TH1* h, const Char_t* KVNucleus_method, Option_t* opt)
{
   // Fill histogram with values of the observable given by the indicated KVNucleus_method.
   // For example: if  the method is called this way - FillHisto(h,"GetZ") - it fills histogram
   // with the charge of all particles in the current event.
   //
   // If opt = "ok" only particles with IsOK()==kTRUE are considered.
   // If opt = "name" only particles belonging to group "name" are considered.

   TMethodCall mt;
   mt.InitWithPrototype(KVNucleus::Class(), KVNucleus_method, "");

   if (mt.IsValid()) {
      Iterator it = GetNextParticleIterator(opt);
      if (mt.ReturnType() == TMethodCall::kLong) {
         Long_t ret;
         for (; it != end(); ++it) {
            KVNucleus* tmp = it.get_pointer<KVNucleus>();
            mt.Execute(tmp, "", ret);
            h->Fill((Double_t)ret);
         }
      }
      else if (mt.ReturnType() == TMethodCall::kDouble) {
         Double_t ret;
         for (; it != end(); ++it) {
            KVNucleus* tmp = it.get_pointer<KVNucleus>();
            mt.Execute(tmp, "", ret);
            h->Fill(ret);
         }
      }
   }
}

void KVEvent::FillHisto(TH1* h, const Char_t* KVNucleus_method, const Char_t* method_prototype, const Char_t* args, Option_t* opt)
{
   // Fill histogram with values of given method with given prototype (e.g. method_prototype="int,int") and argument values
   // e.g. args="2,4") for each particle in event.
   //
   // If opt = "ok" only particles with IsOK()==kTRUE are considered.
   // If opt = "name" only particles belonging to group "name" are considered.

   TMethodCall mt;
   mt.InitWithPrototype(KVNucleus::Class(), KVNucleus_method, method_prototype);

   if (mt.IsValid()) {
      Iterator it = GetNextParticleIterator(opt);
      if (mt.ReturnType() == TMethodCall::kLong) {
         Long_t ret;
         for (; it != end(); ++it) {
            KVNucleus* tmp = it.get_pointer<KVNucleus>();
            mt.Execute(tmp, args, ret);
            h->Fill((Double_t)ret);
         }
      }
      else if (mt.ReturnType() == TMethodCall::kDouble) {
         Double_t ret;
         for (; it != end(); ++it) {
            KVNucleus* tmp = it.get_pointer<KVNucleus>();
            mt.Execute(tmp, args, ret);
            h->Fill(ret);
         }
      }
   }
}

Int_t KVEvent::GetMultiplicity(Int_t Z, Int_t A, Option_t* opt)
{
   // Calculate the multiplicity of nuclei given Z (if A not given)
   // or of nuclei with given Z & A (if given)
   //
   //If opt = "ok" only particles with IsOK()==kTRUE are considered.
   //If opt = "name" only particles belonging to group "name" are considered.

   if (A > 0) return (Int_t)GetSum("IsIsotope", "int,int", Form("%d,%d", Z, A), opt);
   return (Int_t)GetSum("IsElement", "int", Form("%d", Z), opt);
}

void KVEvent::GetMultiplicities(Int_t mult[], const TString& species, Option_t* opt)
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

////////////////////////////////////////////////////////////////////////////////////////

KVNucleus* KVEvent::GetNextParticle(Option_t* opt) const
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
   //          iterations, use the KVEvent::Iterator class

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

//__________________________________________________________________________________________________

void KVEvent::ResetGetNextParticle() const
{
   // Reset iteration over event particles so that next call
   // to GetNextParticle will begin a new iteration (possibly with
   // different criteria).

   fIter.SetIsIterating(kFALSE);
}

//__________________________________________________________________________________________________

Bool_t KVEvent::IsOK()
{
   // Returns kTRUE if the event is OK for analysis.
   // This means there must be at least MOKmin particles with IsOK()=kTRUE,
   // where MOKmin is set by calling SetMinimumOKMultiplicity(Int_t)
   // (value stored in parameter MIN_OK_MULT)

   return (GetMult("ok") >= GetMinimumOKMultiplicity());
}

void KVEvent::SetMinimumOKMultiplicity(Int_t x)
{
   // Set minimum number of particles with IsOK()=kTRUE in event for
   // it to be considered 'good' for analysis
   SetParameter("MIN_OK_MULT", x);
}

Int_t KVEvent::GetMinimumOKMultiplicity() const
{
   // Get minimum number of particles with IsOK()=kTRUE in event for
   // it to be considered 'good' for analysis
   // NB: if no minimum has been set, we return 1
   Int_t x = GetParameters()->GetIntValue("MIN_OK_MULT");
   if (x == -1) return 1;
   return x;
}

//__________________________________________________________________________________________

void KVEvent::ResetEnergies()
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

//______________________________________________________________________________________________

void KVEvent::DefineGroup(const Char_t* groupname, const Char_t* from)
{
   //In the same philosophy as KVINDRAReconEvent::AcceptIDCodes() method
   // allow to affiliate a group name to particles of the event
   // if "from" is not null, a test of previously stored group name
   // such as "OK" is checked

   for (Iterator it = GetNextParticleIterator(from); it != end(); ++it) {
      (*it).AddGroup(groupname);
   }
}

//______________________________________________________________________________________________

void KVEvent::DefineGroup(const Char_t* groupname, KVParticleCondition* cond, const Char_t* from)
{
   //In the same philosophy as KVINDRAReconEvent::AcceptIDCodes() method
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


//______________________________________________________________________________________________

void KVEvent::SetFrame(const Char_t* frame, const KVFrameTransform& ft)
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

//______________________________________________________________________________________________

void KVEvent::SetFrame(const Char_t* newframe, const Char_t* oldframe, const KVFrameTransform& ft)
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

void KVEvent::ChangeFrame(const KVFrameTransform& ft, const KVString& name)
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

void KVEvent::ChangeDefaultFrame(const Char_t* newdef, const Char_t* defname)
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

void KVEvent::UpdateAllFrames()
{
   // If the kinematics of particles in their default reference frame have been modified,
   // call this method to update the kinematics in all defined reference frames.
   //
   //See KVParticle::UpdateAllFrames() for details.

   for (Iterator it = GetNextParticleIterator("ok"); it != end(); ++it) {
      (*it).UpdateAllFrames();
   }
}

void KVEvent::FillArraysP(Int_t& mult, Int_t* Z, Int_t* A, Double_t* px, Double_t* py, Double_t* pz, const TString& frame, const TString& selection)
{
   // "Translate" this event into a simple array form
   // mult will be set to number of nuclei in event
   // (px,py,pz) momentum components in MeV/c
   // frame = optional name of reference frame (see SetFrame methods)
   // selection = selection i.e. "OK"

   Int_t i = 0;
   for (Iterator it = GetNextParticleIterator(selection); it != end(); ++it) {
      KVNucleus* nuc = (KVNucleus*)(*it).GetFrame(frame, kFALSE);
      Z[i] = nuc->GetZ();
      A[i] = nuc->GetA();
      px[i] = nuc->Px();
      py[i] = nuc->Py();
      pz[i] = nuc->Pz();
      i++;
   }
   mult = i;
}

//______________________________________________________________________________

void KVEvent::Streamer(TBuffer& R__b)
{
   // Customised Streamer for KVEvent.
   // This is just the automatic Streamer with the addition of a call to the Clear()
   // method before reading a new object (avoid memory leaks with lists of parameters).

   if (R__b.IsReading()) {
      Clear();
      R__b.ReadClassBuffer(KVEvent::Class(), this);
   }
   else {
      R__b.WriteClassBuffer(KVEvent::Class(), this);
   }
}

//______________________________________________________________________________

#ifdef __WITHOUT_TCA_CONSTRUCTED_AT
TObject* KVEvent::ConstructedAt(Int_t idx)
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
TObject* KVEvent::ConstructedAt(Int_t idx, Option_t* clear_options)
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

void KVEvent::FillArraysV(Int_t& mult, Int_t* Z, Int_t* A, Double_t* vx, Double_t* vy, Double_t* vz, const TString& frame, const TString& selection)
{
   // "Translate" this event into a simple array form
   // mult will be set to number of nuclei in event
   // (vx,vy,vz) velocity components in cm/ns
   // frame = optional name of reference frame (see SetFrame methods)
   // selection = optional selection e.g. "OK"

   Int_t i = 0;
   for (Iterator it = GetNextParticleIterator(selection); it != end(); ++it) {
      KVNucleus* nuc = (KVNucleus*)(*it).GetFrame(frame, kFALSE);
      Z[i] = nuc->GetZ();
      A[i] = nuc->GetA();
      vx[i] = nuc->GetVelocity().X();
      vy[i] = nuc->GetVelocity().Y();
      vz[i] = nuc->GetVelocity().Z();
      i++;
   }
   mult = i;
}

void KVEvent::FillArraysEThetaPhi(Int_t& mult, Int_t* Z, Int_t* A, Double_t* E, Double_t* Theta, Double_t* Phi, const TString& frame, const TString& selection)
{
   // "Translate" this event into a simple array form
   // mult will be set to number of nuclei in event
   // E = kinetic energy in MeV
   // Theta,Phi in degrees
   // frame = optional name of reference frame (see SetFrame methods)
   // selection = optional selection e.g. "OK"

   Int_t i = 0;
   for (Iterator it = GetNextParticleIterator(selection); it != end(); ++it) {
      KVNucleus* nuc = (KVNucleus*)(*it).GetFrame(frame, kFALSE);
      Z[i] = nuc->GetZ();
      A[i] = nuc->GetA();
      E[i] = nuc->GetEnergy();
      Theta[i] = nuc->GetTheta();
      Phi[i] = nuc->GetPhi();
      i++;
   }
   mult = i;
}

void KVEvent::FillArraysPtRapPhi(Int_t& mult, Int_t* Z, Int_t* A, Double_t* Pt, Double_t* Rap, Double_t* Phi, const TString& frame, const TString& selection)
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
      KVNucleus* nuc = (KVNucleus*)(*it).GetFrame(frame, kFALSE);
      Z[i] = nuc->GetZ();
      A[i] = nuc->GetA();
      Pt[i] = nuc->Pt();
      Rap[i] = nuc->Rapidity();
      Phi[i] = nuc->GetPhi();
      i++;
   }
   mult = i;
}

void KVEvent::FillIntegerList(KVIntegerList* IL, Option_t* opt)
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

void KVEvent::GetMasses(std::vector<Double_t>& mass)
{
   // Fill vector with mass of each nucleus of event (in MeV) [note: this is the mass including any excitation energy, not ground state]

   mass.clear();
   mass.reserve(GetMult());
   int i = 0;
   for (Iterator it = begin(); it != end(); ++it) mass[i++] = (*it).GetMass();
}

void KVEvent::GetGSMasses(std::vector<Double_t>& mass)
{
   // Fill vector with ground state mass of each nucleus of event (in MeV).

   mass.clear();
   mass.reserve(GetMult());
   int i = 0;
   for (Iterator it = begin(); it != end(); ++it) mass[i++] = (*it).GetMassGS();
}

Double_t KVEvent::GetChannelQValue() const
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
   KVNucleus CN;
   Int_t M = GetMult();
   for (int i = 1; i <= M; i++) {
      sumM += GetParticle(i)->GetMass();
      CN += *(GetParticle(i));
   }
   return CN.GetMassGS() - sumM;
}

Double_t KVEvent::GetGSChannelQValue() const
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
   KVNucleus CN;
   Int_t M = GetMult();
   for (int i = 1; i <= M; i++) {
      sumM += GetParticle(i)->GetMassGS();
      CN += *(GetParticle(i));
   }
   return CN.GetMassGS() - sumM;
}

const Char_t* KVEvent::GetPartitionName()
{
   //
   //return list of isotopes of the event with the format :
   // symbol1(population1) symbol2(population2) ....
   // if population==1, it is not indicated :
   // Example :
   // 15C 12C(2) 4He 3He 1H(4) 1n(3)
   //
   fParticles->Sort();
   static KVString partition;

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
   return partition.Data();
}

void KVEvent::MergeEventFragments(TCollection* events, Option_t* opt)
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

KVEvent* KVEvent::Factory(const char* plugin)
{
   // Create and return pointer to new event of class given by plugin

   TPluginHandler* ph = LoadPlugin("KVEvent", plugin);
   if (ph) {
      return (KVEvent*)ph->ExecPlugin(0);
   }
   return nullptr;
}

void KVEvent::SetFrameName(const KVString& name)
{
   // Set name of default frame for all particles in event
   // After using this method, calls to
   //    KVParticle::GetFrame(name)
   // will return the address of the particle in question, i.e.
   // its default kinematics
   // The default frame name is stored as a parameter "defaultFrame"

#ifdef WITH_CPP11
   for (KVEvent::Iterator it = std::begin(*this); it != std::end(*this); ++it)
#else
   for (KVEvent::Iterator it = begin(); it != end(); ++it)
#endif
   {
      (*it).SetFrameName(name);
   }
   SetParameter("defaultFrame", name);
}

ClassImp(KVEvent::Iterator)

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
