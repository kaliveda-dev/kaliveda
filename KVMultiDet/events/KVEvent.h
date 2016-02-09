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
class KVIntegerList;

class KVEvent: public KVBase {

protected:

   TClonesArray* fParticles;    //->array of particles in event
   TIter* fOKIter;              //!used for iterating over particles
   KVNameValueList fParameters;//general-purpose list of parameters
#ifdef __WITHOUT_TCA_CONSTRUCTED_AT
   TObject* ConstructedAt(Int_t idx);
   TObject* ConstructedAt(Int_t idx, Option_t* clear_options);
#endif
public:

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
   virtual Int_t GetMult(Option_t* opt = "");
   Int_t GetMultiplicity(Int_t Z, Int_t A = 0, Option_t* opt = "");
   void GetMultiplicities(Int_t mult[], const TString& species);
   Double_t GetSum(const Char_t* KVNucleus_method, Option_t* opt = "");
   Double_t GetSum(const Char_t* KVNucleus_method, const Char_t* method_prototype, const Char_t* args, Option_t* opt = "");
   virtual void Clear(Option_t* opt = "");
   virtual void Print(Option_t* t = "") const;
   virtual void ls(Option_t* t = "") const
   {
      Print(t);
   }
   KVNucleus* GetParticleWithName(const Char_t* name) const;
   KVNucleus* GetParticle(const Char_t* group_name) const;
   KVNucleus* GetNextParticle(Option_t* opt = "");
   void ResetGetNextParticle();

   void ResetEnergies();

   virtual Bool_t IsOK();

   void CustomStreamer()
   {
      fParticles->BypassStreamer(kFALSE);
   };

   void DefineGroup(const Char_t* groupname, const Char_t* from = "");
   void DefineGroup(const Char_t* groupname, KVParticleCondition* cond, const Char_t* from = "");

   void SetFrame(const Char_t* frame, const TVector3& boost, Bool_t beta =
                    kFALSE);
   void SetFrame(const Char_t* frame, const TLorentzRotation& rot);
   void SetFrame(const Char_t* frame, const TRotation& rot);
   void SetFrame(const Char_t* frame, const TVector3& boost, TRotation& rot,
                 Bool_t beta = kFALSE);

   void SetFrame(const Char_t* newframe, const Char_t* oldframe,
                 const TVector3& boost, Bool_t beta = kFALSE);
   void SetFrame(const Char_t* newframe, const Char_t* oldframe,
                 const TLorentzRotation& rot);
   void SetFrame(const Char_t* newframe, const Char_t* oldframe,
                 const TRotation& rot);
   void SetFrame(const Char_t* newframe, const Char_t* oldframe,
                 const TVector3& boost, TRotation& rot, Bool_t beta = kFALSE);

   virtual void FillArraysP(Int_t& mult, Int_t* Z, Int_t* A, Double_t* px, Double_t* py, Double_t* pz, const TString& frame = "", const TString& selection = "");
   virtual void FillArraysV(Int_t& mult, Int_t* Z, Int_t* A, Double_t* vx, Double_t* vy, Double_t* vz, const TString& frame = "", const TString& selection = "");
   virtual void FillArraysEThetaPhi(Int_t& mult, Int_t* Z, Int_t* A, Double_t* E, Double_t* Theta, Double_t* Phi, const TString& frame = "", const TString& selection = "");
   virtual void FillArraysPtRapPhi(Int_t& mult, Int_t* Z, Int_t* A, Double_t* Pt, Double_t* Rap, Double_t* Phi, const TString& frame = "", const TString& selection = "");

   virtual void FillIntegerList(KVIntegerList*, Option_t* opt);

   virtual void GetMasses(Double_t*);
   virtual void GetGSMasses(Double_t*);
   Double_t GetChannelQValue() const;
   Double_t GetGSChannelQValue() const;
   const Char_t* GetPartitionName();

   static void MakeEventBranch(TTree* tree, const TString& branchname, const TString& classname, void* event, Int_t bufsize = 10000000)
   {
      // Use this method when adding a branch to a TTree to store KVEvent-derived objects.
      // If (*e) points to a valid KVEvent-derived object, we use the name of the class of the object.
      // Otherwise we use the value of classname (default = "KVEvent")

      tree->Branch(branchname, classname, event, bufsize, 0)->SetAutoDelete(kFALSE);
   }

   ClassDef(KVEvent, 4)         //Base class for all types of multiparticle event
};

#endif
