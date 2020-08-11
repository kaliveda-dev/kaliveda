/***************************************************************************
                          kvparticle.h  -  description
                             -------------------
    begin                : Sun May 19 2002
    copyright            : (C) 2002 by J.D. Frankland
    email                : frankland@ganil.fr

$Id: KVParticle.h,v 1.41 2008/05/21 13:19:56 ebonnet Exp $
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KVPARTICLE_H
#define KVPARTICLE_H

#include "TVector3.h"
#include "TLorentzVector.h"
#include "KVBase.h"
#include "TRef.h"
#include "TMath.h"
#include "KVList.h"
#include "KVUniqueNameList.h"
#include "TObjString.h"
#include "KVNameValueList.h"
#include "KVFrameTransform.h"

class KVKinematicalFrame;
class KVParticleCondition;

/**
  \class KVParticle
  \ingroup NucEvents
  \brief Base class for kinematics of massive particles
 */
class KVParticle: public TLorentzVector {

private:
   void print_frames(TString fmt = "") const;
   KVKinematicalFrame* get_frame(const Char_t*) const;
   KVKinematicalFrame* get_parent_frame(const Char_t*, KVKinematicalFrame* F = nullptr) const;

   TString fName;                       //!non-persistent name field - Is useful
   TString fFrameName;                  //!non-persistent frame name field, sets when calling SetFrame method
   KVList fBoosted;                     //!list of momenta of the particle in different Lorentz-boosted frames
   KVUniqueNameList fGroups;            //!list of TObjString for manage different group name
   static Double_t kSpeedOfLight;       //speed of light in cm/ns

   // TLorentzVector setters should not be used
   void SetVect(const TVector3& vect3)
   {
      TLorentzVector::SetVect(vect3);
   }
   void SetVectM(const TVector3& spatial, Double_t mass)
   {
      TLorentzVector::SetVectM(spatial, mass);
   }
   void SetVectMag(const TVector3& spatial, Double_t magnitude)
   {
      TLorentzVector::SetVectMag(spatial, magnitude);
   }
   void SetPerp(Double_t p)
   {
      TLorentzVector::SetPerp(p);
   }
   void SetPtEtaPhiE(Double_t pt, Double_t eta, Double_t phi, Double_t e)
   {
      TLorentzVector::SetPtEtaPhiE(pt, eta, phi, e);
   }
   void SetPtEtaPhiM(Double_t pt, Double_t eta, Double_t phi, Double_t m)
   {
      TLorentzVector::SetPtEtaPhiM(pt, eta, phi, m);
   }
   void SetPx(Double_t a)
   {
      TLorentzVector::SetPx(a);
   }
   void SetPy(Double_t a)
   {
      TLorentzVector::SetPy(a);
   }
   void SetPz(Double_t a)
   {
      TLorentzVector::SetPz(a);
   }
   void SetPxPyPzE(Double_t px, Double_t py, Double_t pz, Double_t e)
   {
      TLorentzVector::SetPxPyPzE(px, py, pz, e);
   }
   void SetRho(Double_t rho)
   {
      TLorentzVector::SetRho(rho);
   }
   void SetT(Double_t a)
   {
      TLorentzVector::SetT(a);
   }
   void SetX(Double_t a)
   {
      TLorentzVector::SetX(a);
   }
   void SetY(Double_t a)
   {
      TLorentzVector::SetY(a);
   }
   void SetZ(Double_t a)
   {
      TLorentzVector::SetZ(a);
   }
   void SetXYZM(Double_t x, Double_t y, Double_t z, Double_t m)
   {
      TLorentzVector::SetXYZM(x, y, z, m);
   }
   void SetXYZT(Double_t x, Double_t y, Double_t z, Double_t t)
   {
      TLorentzVector::SetXYZT(x, y, z, t);
   }

protected:

   TVector3* fE0;              //the momentum of the particle before it is slowed/stopped by an absorber
   KVNameValueList fParameters;//a general-purpose list of parameters associated with this particle

   virtual void AddGroup_Withcondition(const Char_t*, KVParticleCondition*);
   virtual void AddGroup_Sanscondition(const Char_t* groupname, const Char_t* from = "");
   void CreateGroups();
   void SetGroups(KVUniqueNameList* un);
   void AddGroups(KVUniqueNameList* un);

public:

   Bool_t HasFrame(const Char_t* frame)
   {
      // Check if a given frame has been defined
      // Note that this may be a frame defined by transformation of another frame
      // Also: frame names are case-insensitive

      return (get_frame(frame) != nullptr);
   }
   Int_t GetNumberOfDefinedFrames(void);
   Int_t GetNumberOfDefinedGroups(void);
   KVUniqueNameList* GetGroups() const;

   enum {
      kIsOK = BIT(14),          //acceptation/rejection flag
      kIsOKSet = BIT(15),       //flag to indicate flag is set
      kIsDetected = BIT(16)     //flag set when particle is slowed by some KVMaterial
   };

   static Double_t C();

   KVParticle();
   KVParticle(Double_t m, TVector3& p);
   KVParticle(Double_t m, Double_t px, Double_t py, Double_t pz);
   KVParticle(const KVParticle&);
   virtual ~ KVParticle();
   void init();
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   virtual void Copy(TObject&) const;
#else
   virtual void Copy(TObject&);
#endif
   virtual void Clear(Option_t* opt = "");

   virtual void SetMass(Double_t m)
   {
      SetXYZM(Px(), Py(), Pz(), m);
   };
   Double_t GetMass() const
   {
      return M();
   };
   void SetMomentum(const TVector3& v)
   {
      SetXYZM(v(0), v(1), v(2), M());
   };
   void SetMomentum(const TVector3* v)
   {
      SetXYZM((*v)(0), (*v)(1), (*v)(2), M());
   };
   void SetMomentum(Double_t px, Double_t py, Double_t pz, Option_t* opt =
                       "cart");
   void SetMomentum(Double_t T, TVector3 dir);
   void SetRandomMomentum(Double_t T, Double_t thmin, Double_t thmax,
                          Double_t phmin, Double_t phmax,
                          Option_t* opt = "isotropic");
   virtual void Print(Option_t* t = "") const;
   void Set4Mom(const TLorentzVector& p)
   {
      SetVect(p.Vect());
      SetT(p.E());
   }
   void SetE(Double_t a)
   {
      SetKE(a);
   };
   void SetKE(Double_t ecin);
   void SetEnergy(Double_t e)
   {
      SetKE(e);
   };
   void SetVelocity(const TVector3&);
   TVector3 GetMomentum() const
   {
      return Vect();
   };
   Double_t GetKE() const
   {
      Double_t e = E();
      Double_t m = M();
      //return (E() - M());
      return e - m;
   };
   Double_t GetEnergy() const
   {
      return GetKE();
   };
   Double_t GetTransverseEnergy() const
   {
      Double_t etran = TMath::Sin(Theta());
      etran = TMath::Power(etran, 2.0);
      etran *= GetKE();
      return etran;
   };
   Double_t GetEtran() const
   {
      return GetTransverseEnergy();
   };

   Double_t GetLongitudinalEnergy() const
   {
      Double_t elong = TMath::Cos(Theta());
      elong = TMath::Power(elong, 2.0);
      elong *= GetKE();
      return elong;
   };
   Double_t GetElong() const
   {
      return GetLongitudinalEnergy();
   };

   Double_t GetRTransverseEnergy() const
   {
      Double_t etran = Mt() - GetMass();
      return etran;
   };
   Double_t GetREtran() const
   {
      return GetRTransverseEnergy();
   };
   Double_t GetE() const
   {
      return GetKE();
   };
   Double_t GetWaveLength() const
   {
      //should be in fm
      if (GetMomentum().Mag() == 0)
         return 0;
      Double_t h = TMath::H() * TMath::C() / TMath::Qe() * 1e9; //in MeV.fm
      return h / GetMomentum().Mag();
   };
   Double_t GetThermalWaveLength(Double_t temp) const
   {
      //should be in fm
      Double_t h = TMath::H() * TMath::C() / TMath::Qe() * 1e9; //in MeV.fm
      return h / TMath::Sqrt(TMath::TwoPi() * temp * GetMass());
   };
   TVector3 GetVelocity() const;
   TVector3 GetV() const
   {
      return GetVelocity();
   };
   Double_t GetVpar() const
   {
      return GetV().z();
   };
   Double_t GetVperp() const;
   Double_t GetTheta() const
   {
      return TMath::RadToDeg() * Theta();
   };
   Double_t GetCosTheta() const
   {
      return TMath::Cos(Theta());
   }
   Double_t GetPhi() const
   {
      Double_t phi = TMath::RadToDeg() * Phi();
      return (phi < 0 ? 360. + phi : phi);
   };
   void SetTheta(Double_t theta)
   {
      TLorentzVector::SetTheta(TMath::DegToRad() * theta);
   }
   void SetPhi(Double_t phi)
   {
      TLorentzVector::SetPhi(TMath::DegToRad() * phi);
   }

   virtual Bool_t IsOK();
   void SetIsOK(Bool_t flag = kTRUE);
   void ResetIsOK()
   {
      ResetBit(kIsOKSet);
   }

   KVList* GetListOfFrames(void)
   {
      return (KVList*)&fBoosted;
   }
   void ls(Option_t* option = "") const;

   void SetE0(TVector3* e = 0)
   {
      if (!fE0)
         fE0 = new TVector3;
      if (!e) {
         *fE0 = GetMomentum();
      }
      else {
         *fE0 = *e;
      }
   };
   TVector3* GetPInitial() const
   {
      return fE0;
   };
   void SetIsDetected()
   {
      SetBit(kIsDetected);
   };
   Bool_t IsDetected()
   {
      return TestBit(kIsDetected);
   }
   KVParticle& operator=(const KVParticle& rhs);

   void ResetEnergy();

   const Char_t* GetName() const;
   void SetName(const Char_t* nom);

   void AddGroup(const Char_t* groupname, const Char_t* from = "");
   void AddGroup(const Char_t* groupname, KVParticleCondition*);

   Bool_t BelongsToGroup(const Char_t* groupname) const;
   void RemoveGroup(const Char_t* groupname);
   void RemoveAllGroups();
   void ListGroups(void) const;

   KVParticle InFrame(const KVFrameTransform&);
   void ChangeFrame(const KVFrameTransform&, const KVString& = "");
   void ChangeDefaultFrame(const Char_t*, const Char_t* defname = "");
   void SetFrame(const Char_t* frame, const KVFrameTransform&);
   void SetFrame(const Char_t* newframe, const Char_t* oldframe, const KVFrameTransform&);

   KVParticle const* GetFrame(const Char_t* frame, Bool_t warn_and_return_null_if_unknown = kTRUE) const;

   const Char_t* GetFrameName(void) const
   {
      return fFrameName;
   }
   void SetFrameName(const Char_t* framename)
   {
      // Set the (non-persistent) name of the reference frame for this particle's kinematics.
      // Also sets a (persistent) parameter "frameName"
      fFrameName = framename;
      if (fFrameName != "") SetParameter("frameName", framename);
      else GetParameters()->RemoveParameter("frameName");
   }

   KVNameValueList* GetParameters() const
   {
      return (KVNameValueList*)&fParameters;
   }
   template<typename ValType> void SetParameter(const Char_t* name, ValType value) const
   {
      GetParameters()->SetValue(name, value);
   }

   void UpdateAllFrames();

   ClassDef(KVParticle, 8)      //General base class for all massive particles
};

#endif
