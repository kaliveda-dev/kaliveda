/***************************************************************************
                          kvtestevent.h  -  description
                             -------------------
    begin                : Sun May 19 2002
    copyright            : (C) 2002 by J.D. Frankland
    email                : frankland@ganil.fr

$Id: KVTestEvent.h,v 1.8 2006/10/19 14:32:43 franklan Exp $
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KVTESTEVENT_H
#define KVTESTEVENT_H

#include "KVSimEvent.h"

class TRandom3;
class KVNucleus;

/**
  \class KVTestEvent
 \brief A simple event generator for testing charged particle array response
 \ingroup Simulation
 \ingroup NucEvents

Each event consists of N KVSimNucleus nuclei with randomly drawn atomic number
Z, kinetic energy and direction.

To use, first set the multiplicity:
   KVTestEvent::KVTestEvent(UInt_t n, Option_t * t)
or KVTestEvent::SetMult(UInt_t mult) ... after default ctor

Then set the range in Z and the range in energy for the generated nuclei:
KVTestEvent::SetZRange(UInt_t zlo, UInt_t zhi)
KVTestEvent::SetERange(Float_t elo, Float_t ehi) (in MeV)
or KVTestEvent::SetERangeAMeV with limits in MeV/nucleon

The masses of the nuclei are calculated from the Z assuming the valley-
of-stability isotope.

By default angular ranges cover the full 4pi range, but this can be changed
with KVTestEvent::SetThetaRange and KVTestEvent::SetPhiRange.
Also by default directions are drawn isotropically within the set angular
ranges (Option_t *t="isotropic") but by choosing option "random" this can be
replaced with a flat angular distribution (see KVPosition::GetRandomDirection)
*/

class KVTestEvent: public KVSimEvent {

   UInt_t fUpper_Limit_Z;       //!max Z of nuclei
   UInt_t fLower_Limit_Z;       //! min Z of nuclei
   Float_t fUpper_Limit_E;      //! max E of nuclei
   Float_t fLower_Limit_E;      //!min E of nuclei
   Bool_t kAMeV;                //! true if E limits are in MeV/nucleon
   Float_t fUpper_Limit_theta;  //! max theta of nuclei
   Float_t fLower_Limit_theta;  //!min theta of nuclei
   Float_t fUpper_Limit_phi;    //! max phi of nuclei
   Float_t fLower_Limit_phi;    //!min phi of nuclei
   Char_t fOption[12];          //![12] option string
   UInt_t fGenMult;             //!multiplicity of nuclei to generate

public:

   KVTestEvent();
   virtual ~ KVTestEvent();
   void init();
   KVTestEvent(UInt_t mult, Option_t* t = "isotropic");

   void SetZRange(UInt_t zlo, UInt_t zhi);
   void SetERange(Float_t elo, Float_t ehi);
   void SetERangeAMeV(Float_t elo, Float_t ehi);
   void SetThetaRange(Float_t thmin, Float_t thmax);
   void SetPhiRange(Float_t phmin, Float_t phmax);
   void SetMult(UInt_t m)
   {
      fGenMult = m;
   }
   void Generate();
   virtual void Print(Option_t* t = "") const;

   ClassDef(KVTestEvent, 1)     //A simple event generator to test charged particle array response
};

#endif
