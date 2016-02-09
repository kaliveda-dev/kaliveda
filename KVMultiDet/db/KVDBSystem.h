/***************************************************************************
$Id: KVDBSystem.h,v 1.12 2008/03/07 15:01:34 ebonnet Exp $
                          KVDBSystem.h  -  description
                             -------------------
    begin                : jeu f� 13 2003
    copyright            : (C) 2003 by Alexis Mignon
    email                : mignon@ganil.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef KV_DB_SYSTEM_H
#define KV_DB_SYSTEM_H

#include "KVDataBase.h"
#include "Riostream.h"
#include "KVTarget.h"
#include "KVList.h"

class KV2Body;
class KVNumberList;

class KVDBSystem: public KVDBRecord {

private:

   KV2Body* fCinema;            //! used to calculate kinematics of entrance channel

   KVTarget* fTarget;           //-> physical target used for experiment run

   KVList* fRunlist;             //!used to store pointer to sorted list of runs
   Int_t fRuns;                 //!temporary variable used to stock number of associated runs

protected:
   UInt_t fZbeam;              // charge of the projectile nucleus
   UInt_t fAbeam;               // Mass of the projectile nucleus
   UInt_t fZtarget;             // charge of the target nucleus
   UInt_t fAtarget;             // Mass of the target nucleus
   Float_t fEbeam;              // Energy of the beam in MeV/nucleon

   KVList* _GetRuns();

   KVDBTable* GetRunsTable();

public:
   KVDBSystem();
   KVDBSystem(const Char_t* name);
   virtual ~ KVDBSystem();

   KVTarget* GetTarget() const
   {
      return fTarget;
   };
   void SetTarget(KVTarget* targ)
   {
      fTarget = targ;
   };

   virtual void ls(Option_t* option = "*") const;
   UInt_t GetZtarget() const;
   UInt_t GetAtarget() const;
   UInt_t GetZbeam() const;
   UInt_t GetAbeam() const;
   UInt_t GetZproj() const;
   UInt_t GetAproj() const;
   Float_t GetEbeam() const;
   Float_t GetEproj() const;
   Float_t GetTargetThickness() const;

   UInt_t GetZtot() const;
   UInt_t GetAtot() const;
   Double_t GetZVtot() const;
   Double_t GetPtot() const;
   Double_t GetEtot() const;
   Double_t GetECM() const;

   Bool_t IsCollision() const;

   void SetZtarget(UInt_t z);
   void SetAtarget(UInt_t a);
   void SetZbeam(UInt_t z);
   void SetAbeam(UInt_t a);
   void SetEbeam(Float_t energy);

   void SetBeam(UInt_t z, UInt_t a, Float_t energy);

   //Returns a sorted list of all the runs associated with this system
   KVList* GetRuns() const
   {
      return const_cast <KVDBSystem* >(this)->_GetRuns();
   };

   virtual void GetRunList(KVNumberList&) const;
   virtual void Save(std::ostream&) const;
   virtual void Load(std::istream&);

   Int_t Compare(const TObject*) const;

   KV2Body* GetKinematics();

   virtual void Print(Option_t* option = "") const;

   //set number of runs associated to this system
   void SetNumberRuns(Int_t n)
   {
      fRuns = n;
   };
   //get number of runs previously set by SetNumberRuns.
   //WARNING: for total number of runs associated to this system in database,
   //use GetRuns()->GetEntries()
   Int_t GetNumberRuns()
   {
      return fRuns;
   };

   void SetRuns(KVNumberList&);
   void RemoveRun(KVDBRecord*);
   void RemoveRun(Int_t);
   void AddRun(KVDBRecord*);
   void AddRun(Int_t);
   void RemoveAllRuns();
   virtual const Char_t* GetBatchName();

   ClassDef(KVDBSystem, 2)      // System class
};

//.............. inline functions ...............
inline UInt_t KVDBSystem::GetZtarget() const
{
   return fZtarget;
}

inline UInt_t KVDBSystem::GetAtarget() const
{
   return fAtarget;
}

inline UInt_t KVDBSystem::GetZtot() const
{
   return fZtarget + fZbeam;
}

inline UInt_t KVDBSystem::GetAtot() const
{
   return fAtarget + fAbeam;
}

inline UInt_t KVDBSystem::GetZbeam() const
{
   return fZbeam;
}

inline UInt_t KVDBSystem::GetAbeam() const
{
   return fAbeam;
}

inline UInt_t KVDBSystem::GetZproj() const
{
   return fZbeam;
}

inline UInt_t KVDBSystem::GetAproj() const
{
   return fAbeam;
}

inline Float_t KVDBSystem::GetEbeam() const
{
   return fEbeam;
}

inline Float_t KVDBSystem::GetEproj() const
{
   return fEbeam;
}

inline Float_t KVDBSystem::GetTargetThickness() const
{
   return (GetTarget() ? GetTarget()->GetTotalThickness() : 0.);
}

inline void KVDBSystem::SetZtarget(UInt_t z)
{
   fZtarget = z;
}

inline void KVDBSystem::SetAtarget(UInt_t a)
{
   fAtarget = a;
}

inline void KVDBSystem::SetZbeam(UInt_t z)
{
   fZbeam = z;
}

inline void KVDBSystem::SetAbeam(UInt_t a)
{
   fAbeam = a;
}

inline void KVDBSystem::SetEbeam(Float_t energy)
{
   fEbeam = energy;
}

inline void KVDBSystem::SetBeam(UInt_t z, UInt_t a, Float_t energy)
{
   fZbeam = z;
   fAbeam = a;
   fEbeam = energy;
}
#endif
