/***************************************************************************
                          KVMultiDetArray.h  -  description
                             -------------------
    begin                : Thu May 16 2002
    copyright            : (C) 2002 by J.D. Frankland
    email                : frankland@ganil.fr

$Id: KVMultiDetArray.h,v 1.55 2009/03/03 14:27:15 franklan Exp $
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KVASMultiDetArray_H
#define KVASMultiDetArray_H

#include "KVMultiDetArray.h"
#include "KVLayer.h"
#include "KVRing.h"
class KVASGroup;

/**
  \class KVASMultiDetArray
  \ingroup Geometry
  \brief Base class for azimuthally-symmetric multidetector arrays (obsolete)
 */
class KVASMultiDetArray : public KVMultiDetArray {

protected:
   UInt_t fCurrentLayerNumber; //! used to number layers
   UInt_t fGr;                 //!used to number groups

   virtual void AddToGroups(KVTelescope* kt1, KVTelescope* kt2);
   void SetGroups(KVLayer*, KVLayer*);
   void UpdateGroupsInRings(KVRing* r1, KVRing* r2);
   void MakeListOfDetectors();
   virtual void MergeGroups(KVASGroup* kg1, KVASGroup* kg2);
   KVNameValueList* DetectParticle_KV(KVNucleus* part);
   void GetIDTelescopesForGroup(KVGroup* grp, TCollection* tel_list);
   virtual void set_up_telescope(KVDetector* de, KVDetector* e, KVIDTelescope* idt, TCollection* l);
   virtual void set_up_single_stage_telescope(KVDetector* det, KVIDTelescope* idt, TCollection* l);
public:
   KVASMultiDetArray();
   virtual ~ KVASMultiDetArray();
   void init();
   virtual KVGroup* GetGroupWithAngles(Float_t theta, Float_t phi);
   TList* GetTelescopes(Float_t theta, Float_t phi);
   virtual KVNameValueList* DetectParticle(KVNucleus* part)
   {
      // Simulate detection of a charged particle by the array.
      // The actual method called depends on the value of fROOTGeometry:
      //   fROOTGeometry=kTRUE:  calls DetectParticle_TGEO, particle propagation performed using
      //               TGeo description of array and algorithms from ROOT TGeo package
      //   fROOTGeometry=kFALSE:  calls DetectParticle_KV, uses simple KaliVeda geometry
      //                to simulate propagation of particle
      //
      // The default value is given in .kvrootrc by variable KVASMultiDetArray.FilterUsesROOTGeometry
      return (IsROOTGeometry() ? DetectParticle_TGEO(part) : DetectParticle_KV(part));
   };

   void CalculateGroupsFromGeometry();
   KVLayer* GetLayer(const Char_t* name) const
   {
      //find layer with name
      return (KVLayer*)GetStructure("LAYER", name);
   }
   KVLayer* GetLayer(Int_t num) const
   {
      //find layer with number
      return (KVLayer*)GetStructure("LAYER", num);
   }
   KVTelescope* GetTelescope(const Char_t* name) const;
   KVRing* GetRing(const Char_t* layer, const Char_t* ring_name) const;
   KVRing* GetRing(const Char_t* layer, UInt_t ring_number) const;
   KVRing* GetRing(UInt_t layer, const Char_t* ring_name) const;
   KVRing* GetRing(UInt_t layer, UInt_t ring_number) const;

   virtual Double_t GetSolidAngleByLayerAndRing(const Char_t* layer, UInt_t ring_number)
   {
      // return the solid angle (msr) for a given KVRing conditioned by the chosen KVLayer
      return GetRing(layer, ring_number)->GetSolidAngle();
   }
   virtual Double_t GetSolidAngleByLayer(const Char_t* layer)
   {
      // return the solid angle (msr) of all the KVRing conditioned by the chosen KVLayer
      Double_t sol_ang = 0;
      KVRing* cou;
      TIter nxtcou(GetLayer(layer)->GetStructures());
      while ((cou = (KVRing*) nxtcou())) sol_ang += cou->GetSolidAngle();
      return sol_ang;
   }

   virtual Double_t GetTotalSolidAngle(void) const;
   void AnalyseGroupAndReconstructEvent(KVReconstructedEvent* event, KVGroup* grp);
   void AnalyseTelescopes(KVReconstructedEvent* event, TList* kvtl);

   ClassDef(KVASMultiDetArray, 1) //Azimuthally-symmetric multidetector arrays
};

#endif
