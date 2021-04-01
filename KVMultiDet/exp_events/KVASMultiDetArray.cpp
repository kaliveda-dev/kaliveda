/***************************************************************************
$Id: KVMultiDetArray.cpp,v 1.91 2009/04/06 11:54:54 franklan Exp $
                          KVMultiDetArray.cpp  -  description
                             -------------------
    begin                : Thu May 16 2002
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
#include "TROOT.h"
#include "KVASMultiDetArray.h"
#include "KVDetector.h"
#include "KVDetectorEvent.h"
#include "KVReconstructedEvent.h"
#include "KVEBYEDAT_ACQParam.h"
#include "KVRList.h"
#include "KVLayer.h"
#include "KVEvent.h"
#include "KVNucleus.h"
#include "KVASGroup.h"
#include "KVRing.h"
#include "KVTelescope.h"
#include "KVMaterial.h"
#include "KVTarget.h"
#include "KVIDTelescope.h"
#include "KV2Body.h"
#include <KVString.h>
#include <TObjString.h>
#include <TObjArray.h>
#include <KVIDGridManager.h>
#include "TPluginManager.h"
#include "TGeoManager.h"
#include "TGeoMedium.h"
#include "TGeoMaterial.h"
#include "KVHashList.h"
#include "KVNameValueList.h"
#include "KVUniqueNameList.h"
#include "KVIonRangeTable.h"

using namespace std;

ClassImp(KVASMultiDetArray)

KVASMultiDetArray::KVASMultiDetArray()
{
   init();
}

void KVASMultiDetArray::init()
{
   fCurrentLayerNumber = 0;
}

//___________________________________________________________________________________

KVASMultiDetArray::~KVASMultiDetArray()
{
   //destroy (delete) the MDA and all the associated structure, detectors etc.
}


//_______________________________________________________________________________________
void KVASMultiDetArray::SetGroups(KVLayer* l1, KVLayer* l2)
{
//Update the list of groups in the detector array by comparing all telescopes in two layers.

   TIter lay1_nextring(l1->GetStructures());
   TIter lay2_nextring(l2->GetStructures());
   KVRing* kring1, *kring2;
   // loop over all pairs of rings in the two layers
   while ((kring1 = (KVRing*) lay1_nextring())) {
      while ((kring2 = (KVRing*) lay2_nextring())) {
#ifdef KV_DEBUG
         cout <<
              "SetGroups(KVLayer * l1, KVLayer * l2) - Looking at rings:" <<
              endl;
         cout << "   kring1 - " << kring1->GetName() << ", " << kring1->
              GetType() << ", " << kring1->GetNumber() << endl;
         cout << "   kring2 - " << kring2->GetName() << ", " << kring2->
              GetType() << ", " << kring2->GetNumber() << endl;
#endif
         if ((kring1->IsOverlappingWith(kring2)))       // overlap between rings ?
#ifdef KV_DEBUG
            cout << "   +++ Calling UpdateGroupsInRings" << endl;
#endif
         UpdateGroupsInRings(kring1, kring2);
      }
      lay2_nextring.Reset();
   }
}

//____________________________________________________________________________
void KVASMultiDetArray::UpdateGroupsInRings(KVRing* r1, KVRing* r2)
{
//Calculate groups by comparing two rings in different layers.
   TIter r1nxtele(r1->GetTelescopes());
   TIter r2nxtele(r2->GetTelescopes());
   KVTelescope* kvt1, *kvt2;
   // loop over all pairs of telescopes in the two rings
   while ((kvt1 = (KVTelescope*) r1nxtele())) {
      while ((kvt2 = (KVTelescope*) r2nxtele())) {
         if (kvt1->IsOverlappingWith(kvt2)) {
            AddToGroups(kvt1, kvt2);
         }
      }
      r2nxtele.Reset();
   }
}



//____________________________________________________________________________________________

KVTelescope* KVASMultiDetArray::GetTelescope(const Char_t* name) const
{
   // Return pointer to telescope in array with name given by "name"
   KVSeqCollection* fLayers = GetStructureTypeList("LAYER");
   TIter nextL(fLayers);
   KVLayer* k1;
   KVRing* k2;
   KVTelescope* k3;
   while ((k1 = (KVLayer*) nextL())) {  // loop over layers
      TIter nextR(k1->GetStructures());
      while ((k2 = (KVRing*) nextR())) {        // loop over rings
         TIter nextT(k2->GetTelescopes());
         while ((k3 = (KVTelescope*) nextT())) {
            if (!strcmp(k3->GetName(), name)) {
               delete fLayers;
               return k3;
            }
            // search among telescopes of ring
         }
      }
   }
   delete fLayers;
   return 0;
}



//________________________________________________________________________________________
void KVASMultiDetArray::MakeListOfDetectors()
{
   // Set up list of all detectors in array
   // This method is called after array geometry has been defined.
   // We set the name of each detector by calling its KVDetector::GetArrayName()
   // method (or overrides in child classes), as the name may depend on the
   // position in the final array geometry.

   fDetectors.Clear();

   KVSeqCollection* fLayers = GetStructureTypeList("LAYER");
   TIter nextL(fLayers);
   KVLayer* k1;
   KVRing* k2;
   KVTelescope* k3;
   KVDetector* k4;
   while ((k1 = (KVLayer*) nextL())) {  // loop over layers
      TIter nextR(k1->GetStructures());
      while ((k2 = (KVRing*) nextR())) {        // loop over rings
         TIter nextT(k2->GetTelescopes());
         while ((k3 = (KVTelescope*) nextT())) {        // loop over telescopes
            TIter nextD(k3->GetDetectors());
            while ((k4 = (KVDetector*) nextD())) {      // loop over detectors
               if (k4->InheritsFrom("KVDetector"))
                  k4->SetName(k4->GetArrayName());  //set name of detector
               Add(k4);
            }
            ((KVUniqueNameList*)k3->GetDetectors())->Rehash();//keep consistency if any detector names changed
         }
         ((KVUniqueNameList*)k2->GetStructures())->Rehash();//keep consistency if any telescope names changed
      }
   }
   // rehash detector node lists due to change of names of all detectors
   TIter nextD(GetDetectors());
   while ((k4 = (KVDetector*)nextD())) k4->GetNode()->RehashLists();

#ifdef KV_DEBUG
   Info("MakeListOfDetectors", "Success");
#endif
   delete fLayers;
}

KVRing* KVASMultiDetArray::GetRing(const Char_t* layer,
                                   const Char_t* ring_name) const
{
   //find named ring in named layer
   KVLayer* tmp = GetLayer(layer);
   KVRing* ring = 0;
   if (tmp) {
      ring = (KVRing*)tmp->GetStructure("RING", ring_name);
   }
   return ring;
}

KVRing* KVASMultiDetArray::GetRing(const Char_t* layer, UInt_t ring_number) const
{
   //find numbered ring in named layer
   KVLayer* tmp = GetLayer(layer);
   KVRing* ring = 0;
   if (tmp) {
      ring = (KVRing*)tmp->GetStructure("RING", ring_number);
   }
   return ring;
}

KVRing* KVASMultiDetArray::GetRing(UInt_t layer, const Char_t* ring_name) const
{
   //find named ring in numbered layer
   KVLayer* tmp = GetLayer(layer);
   KVRing* ring = 0;
   if (tmp) {
      ring = (KVRing*)tmp->GetStructure("RING", ring_name);
   }
   return ring;
}

KVRing* KVASMultiDetArray::GetRing(UInt_t layer, UInt_t ring_number) const
{
   //find numbered ring in numbered layer
   KVLayer* tmp = GetLayer(layer);
   KVRing* ring = 0;
   if (tmp) {
      ring = (KVRing*)tmp->GetStructure("RING", ring_number);
   }
   return ring;
}


//_________________________________________________________________________________
void KVASMultiDetArray::CalculateGroupsFromGeometry()
{
   ClearStructures("GROUP");          // clear out (delete) old groups

   const KVSeqCollection* fLayers = GetStructures();
   ((KVUniqueNameList*)fLayers)->Sort();

   TIter nxtlay1(fLayers);
   KVLayer* l1;
   if (fLayers->GetSize() > 1) {
      fGr = 0;                  // for numbering groups
      TIter nxtlay2(fLayers);
      KVLayer* l2;

      while ((l1 = (KVLayer*) nxtlay1())) {     // loop over layers
         while ((l2 = (KVLayer*) nxtlay2())) {  // loop over layers
            if ((l1->GetNumber() < l2->GetNumber())
                  && ((UInt_t) l1->GetNumber() <
                      (UInt_t) fLayers->GetSize()))
               SetGroups(l1, l2);       // compare all telescopes in different layers
         }
         nxtlay2.Reset();
      }
   }  //if(fLayers->GetSize()>1)

   // Finally, create groups for all orphan telescopes which are not in any existing
   // group

   nxtlay1.Reset();          // reset loop over layers
   while ((l1 = (KVLayer*) nxtlay1())) {     // loop over layers
      if (l1->GetStructures()) {
         TIter nxtrng(l1->GetStructures());
         KVRing* robj;
         while ((robj = (KVRing*) nxtrng())) {       // loop over rings
            TIter nxtscp(robj->GetTelescopes());
            KVTelescope* tobj;
            while ((tobj = (KVTelescope*) nxtscp())) {       // loop over telescopes
               if (!tobj->GetParentStructure("GROUP")) {      // orphan telescope

                  KVASGroup* kvg = new KVASGroup();
                  kvg->SetNumber(++fGr);
                  kvg->Add(tobj);
                  //group dimensions determined by detector dimensions
                  kvg->SetPolarMinMax(tobj->GetThetaMin(),
                                      tobj->GetThetaMax());
                  kvg->SetAzimuthalMinMax(tobj->GetPhiMin(),
                                          tobj->GetPhiMax());
                  Add(kvg);
               }
            }
         }
      }
   }
}

void KVASMultiDetArray::AddToGroups(KVTelescope* kt1, KVTelescope* kt2)
{
// The two telescopes are in angular overlap.
// a) if neither are in groups already, create a new group to which they both belong
// b) if one of them is in a group already, add the orphan telescope to it
// c) if both are in groups already, merge the two groups
   KVASGroup* kvg;

   if (!kt1->GetParentStructure("GROUP") && !kt2->GetParentStructure("GROUP")) {  // case a)
#ifdef KV_DEBUG
      cout << "Making new Group from " << kt1->
           GetName() << " and " << kt2->GetName() << endl;
#endif
      kvg = new KVASGroup();
      kvg->SetNumber(++fGr);
      kvg->Add(kt1);
      kvg->Add(kt2);
      kvg->Sort();              // sort telescopes in list
      kvg->SetDimensions(kt1, kt2);     // set group dimensions from telescopes
      //add to list
      Add(kvg);
   }
   else if ((kvg = (KVASGroup*)(kt1->GetParentStructure("GROUP")))
            && !kt2->GetParentStructure("GROUP")) {  // case b) - kt1 is already in a group
#ifdef KV_DEBUG
      cout << "Adding " << kt2->GetName() << " to group " << kvg->
           GetNumber() << endl;
#endif
      //add kt2 to group
      kvg->Add(kt2);
      kvg->Sort();              // sort telescopes
      kvg->SetDimensions(kvg, kt2);     //adjust dimensions depending on kt2
   }
   else if ((kvg = (KVASGroup*)(kt2->GetParentStructure("GROUP")))
            && !kt1->GetParentStructure("GROUP")) {  // case b) - kt2 is already in a group
#ifdef KV_DEBUG
      cout << "Adding " << kt1->GetName() << " to group " << kvg->
           GetNumber() << endl;
#endif
      //add kt1 to group
      kvg->Add(kt1);
      kvg->Sort();              // sort telescopes
      kvg->SetDimensions(kvg, kt1);     //adjust dimensions depending on kt1
   }
   else if (kt1->GetParentStructure("GROUP") != kt2->GetParentStructure("GROUP")) {       //both telescopes already in different groups
#ifdef KV_DEBUG
//         cout << "Merging " << kt1->GetGroup()->
//         GetNumber() << " and " << kt2->GetGroup()->GetNumber() << endl;
//         cout << "because of " << kt1->GetName() << " and " << kt2->
//         GetName() << endl;
#endif
      MergeGroups((KVASGroup*)kt1->GetParentStructure("GROUP"), (KVASGroup*)kt2->GetParentStructure("GROUP"));
   }
}
//_______________________________________________________________________________________
void KVASMultiDetArray::MergeGroups(KVASGroup*, KVASGroup*)
{
   //Merge two existing groups into a new single group.

   //look through kg1 telescopes.
   //set their "group" to kg2.
   //if they are not already in kg2, add them to kg2.
//    KVTelescope *tel = 0;
//    TIter next(kg1->GetTelescopes());
//    while ((tel = (KVTelescope *) next())) {
//        if (kg2->Contains(tel)) {
//            tel->SetGroup(kg2);    //make sure telescope has right group pointer
//        } else {
//            kg2->Add(tel);
//        }
//    }
//    //remove kg1 from list and destroy it
//    fGroups->Remove(kg1);
//    delete kg1;
//    //sort merged group and calculate dimensions
//    kg2->Sort();
//    kg2->SetDimensions();
//    //renumber groups and reset group counter
//    RenumberGroups();
   Warning("KVASMultiDetArray", "Needs reimplementing");
}
//__________________________________________________________________________________
KVNameValueList* KVASMultiDetArray::DetectParticle_KV(KVNucleus* part)
{
// Simulate detection of a single particle (nucleus) by multidetector array.
// We look for the group in the array that the particle will hit
//
// If a group is found the detection of this particle
// by the different members of the group is simulated.
// The detectors concerned have their fEloss members set to the energy lost by the
// particle when it crosses them.
//
// Returns a list (KVNameValueList pointer) of the crossed detectors with their name and energy loss
// if particle hits detector in array, 0 if not (i.e. particle
// in beam pipe or dead zone of the multidetector)
// INFO User has to delete the KVNameValueList after its use
//
   //find group in array hit by particle
   KVASGroup* grp_tch = (KVASGroup*)GetGroupWithAngles(part->GetTheta(), part->GetPhi());
   if (grp_tch) {
      //simulate detection of particle by this group
      KVNameValueList* nvl = grp_tch->DetectParticle(part);
      return nvl;
   }
   return 0;
}

//_______________________________________________________________________________________

KVGroup* KVASMultiDetArray::GetGroupWithAngles(Float_t theta, Float_t phi)
{
   // return pointer to group in array according to given polar coordinates
   KVSeqCollection* fGroups = GetStructures()->GetSubListWithType("GROUP");

   TIter next(fGroups);
   KVGroup* obj;
   while ((obj = (KVGroup*) next())) {  // loop over group list
      if (((KVASGroup*)obj)->IsInPolarRange(theta)
            && ((KVASGroup*)obj)->IsInPhiRange(phi)) {
         delete fGroups;
         return obj;
      }
   }
   delete fGroups;
   return 0;                    // no group found with these coordinates
}
//_______________________________________________________________________________________

TList* KVASMultiDetArray::GetTelescopes(Float_t theta, Float_t phi)
{
   //Create and fill list of telescopes at position (theta, phi) sorted
   //according to distance from target (closest first).
   //User must delete list after use.
   KVASGroup* gr = (KVASGroup*)GetGroupWithAngles(theta, phi);
   TList* tmp = 0;
   if (gr)
      tmp = gr->GetTelescopesWithAngles(theta, phi);
   return tmp;
}

//_________________________________________________________________________________

Double_t KVASMultiDetArray::GetTotalSolidAngle(void) const
{
   // compute & return the total solid angle (msr) of the array
   // it is the sum of solid angles of all existing KVGroups

   // if ROOT geometry is used, just use base class method
   if (IsROOTGeometry()) return KVMultiDetArray::GetTotalSolidAngle();

   Double_t ftotal_solid_angle = 0.0;
   KVASGroup* grp;
   KVSeqCollection* fGroups = GetStructures()->GetSubListWithType("GROUP");

   TIter ngrp(fGroups);
   while ((grp = (KVASGroup*) ngrp())) {
      ftotal_solid_angle += grp->GetSolidAngle(); // use the KVPosition::GetSolidAngle()
   }
   delete fGroups;
   return ftotal_solid_angle;
}

void KVASMultiDetArray::GetIDTelescopesForGroup(KVGroup* grp, TCollection* tel_list)
{
   //Identify all the ways of identifying particles possible from the detectors
   //in the group, create the appropriate KVIDTelescope objects and add them to
   //the list pointed to by tel_list.
   //USER'S RESPONSIBILITY TO DELETE CONTENTS OF LIST!!
   //
   //Starting from each detector in the "detector layer" furthest from the
   //target, we build ID telescopes from all pairs of aligned detectors.
   //We then continue up through the layers of the group
   //
   //For each pair of detectors, it is KVMultiDetArray::GetIDTelescopes
   //which determines which KVIDTelescope class to use (specialise this method
   //in KVMultiDetArray child classes). It must also make sure that
   //each IDTelescope is added only once (i.e. check it is not already in the
   //list).

   for (int lay = grp->GetNumberOfDetectorLayers(); lay > 0; lay--) {

      unique_ptr<TList> det_lay(grp->GetDetectorsInLayer(lay));
      if (det_lay.get()) {

         TIter next_det(det_lay.get());
         KVDetector* det;

         while ((det = (KVDetector*) next_det())) {
            if (det->IsOK()) {
               //1st call: create ID telescopes, they will be added to the
               //gMultiDetArray list of IDTelescopes
               GetAlignedIDTelescopesForDetector(det, tel_list);
               //2nd call: set up in the detector a list of pointers to the
               //ID telescopes made up of it and all aligned detectors in front
               //of it
               GetAlignedIDTelescopesForDetector(det, 0);
            }
         }

      }
   }
}

void KVASMultiDetArray::set_up_telescope(KVDetector* de, KVDetector* e, KVIDTelescope* idt, TCollection* idtels)
{
   // Set up detectors in de-e identification telescope and add to idtels

   idt->AddDetector(de);
   idt->AddDetector(e);
   if (de->GetGroup()) {
      idt->SetGroup(de->GetGroup());
   }
   else {
      idt->SetGroup(e->GetGroup());
   }
   if (idtels->FindObject(idt->GetName())) {
      delete idt;
   }
   else {
      idtels->Add(idt);
   }
}

void KVASMultiDetArray::set_up_single_stage_telescope(KVDetector* det, KVIDTelescope* idt, TCollection* idtels)
{
   // Set up detector in single-stage identification telescope and add to idtels

   idt->AddDetector(det);
   idt->SetGroup(det->GetGroup());
   if (idtels->FindObject(idt->GetName())) {
      delete idt;
   }
   else {
      idtels->Add(idt);
   }
}

void KVASMultiDetArray::AnalyseGroupAndReconstructEvent(KVReconstructedEvent* event, KVGroup* grp)
{

   KVASGroup* ASgrp = dynamic_cast<KVASGroup*>(grp);
   if (!ASgrp) {
      Fatal("AnalyseGroupAndReconstructedEvent",
            "All groups in KVASMultiDetArray must derive from KVASGroup! %s is NOT a KVASGroup", grp->GetName());
      return;
   }
   UInt_t nLayers = ASgrp->GetNumberOfLayers();

   UInt_t initial_hits_in_group = grp->GetHits();

   if (nLayers > 1) {
      //multilayer group
      //Start with layer furthest from target and work inwards (but don't look at layer
      //nearest to target)
      for (UInt_t i = ASgrp->GetLayerFurthestTarget();
            i > ASgrp->GetLayerNearestTarget(); i--) {
         TList* kvtl = ASgrp->GetTelescopesInLayer(i);
         if (kvtl) {
            AnalyseTelescopes(event, kvtl);
            delete kvtl;
         }
      }


      //if nothing has been found, then check for particles stopping in layer nearest target
      if (grp->GetHits() == initial_hits_in_group) {
         TList* kvtl =
            ASgrp->GetTelescopesInLayer(ASgrp->GetLayerNearestTarget());
         if (kvtl) {
            AnalyseTelescopes(event, kvtl);
            delete kvtl;
         }
      }

   }
   else {
      //single layer group
#ifdef KV_DEBUG
      Info("AnalyseGroup", "Single layer group");
#endif
      //for a single layer group we should have
      //kvg->GetLayerNearestTarget() = kvg->GetLayerFurthestTarget()
      //so we can use either one as argument for kvg->GetTelescopesInLayer
      TList* kvtl =
         ASgrp->GetTelescopesInLayer(ASgrp->GetLayerNearestTarget());
      if (kvtl) {
         AnalyseTelescopes(event, kvtl);
         delete kvtl;
      }
   }

#ifdef KV_DEBUG
   Info("AnalyseGroup", "OK after analysis of hit groups");
#endif

   //perform first-order coherency analysis (set fAnalStatus for each particle)
   KVReconstructedNucleus::AnalyseParticlesInGroup(grp);
}

//_________________________________________________________________________________

void KVASMultiDetArray::AnalyseTelescopes(KVReconstructedEvent* event, TList* kvtl)
{

   KVTelescope* t;
   TIter nxt_tel(kvtl);
   //get max number of detectors in telescopes of layer
   Int_t max = 0;
   while ((t = (KVTelescope*)nxt_tel()))
      if (max < t->GetDetectors()->GetSize())
         max = t->GetDetectors()->GetSize();
   //before, we assumed all telescopes to be same in layer:
   // but in fact it's not true
   //UInt_t ndet = ((KVTelescope *) (kvtl->First()))->GetSize();
   UInt_t ndet = max;

   for (UInt_t i = ndet; i > 0; i--) {

      nxt_tel.Reset();
      TList detlist;

      //start from last detectors and move inwards
      while ((t = (KVTelescope*) nxt_tel())) {
         //loop over detectors in each telescope
         if ((UInt_t)t->GetDetectors()->GetSize() >= i) {
            KVDetector* d = t->GetDetector(i);
            if (d)
               detlist.Add(d);
            else
               Warning("AnalyseTelescopes", "pointeur KVDetector NULL");
         }
      }

      event->AnalyseDetectors(&detlist);

   }
}

