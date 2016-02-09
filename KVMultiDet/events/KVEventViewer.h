//Created by KVClassFactory on Fri Apr 12 15:48:03 2013
//Author: John Frankland,,,

#ifndef __KVEVENTVIEWER_H
#define __KVEVENTVIEWER_H

#include "KVBase.h"
#include "KVEvent.h"
#include "TGeoManager.h"
#include "TGeoMaterial.h"
#include "TGeoMedium.h"
#include "TGeoVolume.h"
#include "Riostream.h"
using namespace std;

class TBranch;
class TTree;

class KVEventViewer : public KVBase {
protected:
   Int_t fproton_color;//proton colour
   Int_t fneutron_color;//neutron colour
   Int_t fProton_color;//proton colour for highlighted nuclei
   Int_t fNeutron_color;//neutron colour for highlighted nuclei
   Double_t free_nucleon_radius;//radius of free nucleons
   Double_t nucleon_radius;//radius of nucleons in nuclei
   Double_t nuclear_radius_parameter;//to calculate radii of nuclei

   Double_t fMaxVelocity;
   Bool_t fFixSeed;
   Int_t fRefresh;
   Int_t fSeed;
   Bool_t fXYMode;
   Bool_t fMomentumSpace;//kTRUE to show event in momentum space
   Double_t fScaleFactor;

   Int_t ivol;//geovolume counter
   Bool_t fSavePicture;//kTRUE to save in GIF file

   TGeoMaterial* matEmptySpace;
   TGeoMaterial* matNuc;
   TGeoMaterial* matBox;

   TGeoMedium* EmptySpace;
   TGeoMedium* Nuc;
   TGeoMedium* Box;
   TGeoVolume* top;
   TGeoManager* geom;

   KVEvent* theEvent;

   ifstream eventFile;
   Int_t eventCounter;
   Bool_t textInput;

   TTree* theTree;
   Long64_t NtreeEntries;

   Double_t maxV;//largest velocity of event
   Int_t maxZ;//largest Z of event

   Int_t fHighlightMode;
   Bool_t fAxesMode;

public:
   enum EHighlightMode {
      kNoHighlight,
      kHighlightZmax,
      kHighlightParameter
   };
   KVEventViewer(Int_t protoncolor = kRed - 2, Int_t neutroncolor = kBlue - 2,
                 Int_t highlightprotoncolor = kOrange - 2,
                 Int_t highlightneutroncolor = kGreen - 2,
                 Double_t freenucleonradius = 0.2,
                 Double_t nucleonradius = 0.25,
                 Double_t nuclearradiusparameter = 0.2);
   virtual ~KVEventViewer();

   void SetProtonColor(Int_t c)
   {
      fproton_color = c;
   }
   void SetNeutronColor(Int_t c)
   {
      fneutron_color = c;
   }
   void SetHighlightProtonColor(Int_t c)
   {
      fProton_color = c;
   }
   void SetHighlightNeutronColor(Int_t c)
   {
      fNeutron_color = c;
   }
   void SetSavePicture(Bool_t yes = kTRUE)
   {
      fSavePicture = yes;
   }

   // Set radius used to represent free nucleons (default value = 0.2)
   void SetFreeNucleonRadius(Double_t r)
   {
      free_nucleon_radius = r;
   }
   // Set radius used to represent nucleons in nuclei (default value = 0.25)
   void SetNucleonRadius(Double_t r)
   {
      nucleon_radius = r;
   }

   void SetMaxVelocity(Int_t vel)
   {
      fMaxVelocity = vel;
   }
   void SetFixSeed(Bool_t fix, Int_t refresh = 1)
   {
      fFixSeed = fix;
      fRefresh = refresh;
   }
   void SetXYMode(Bool_t mode)
   {
      fXYMode = mode;
   }
   // Draw event in momentum space (default is velocity space)
   void SetDrawMomentumSpace(Bool_t on = kTRUE)
   {
      fMomentumSpace = on;
   }

   void DrawNucleus(KVNucleus*, const Char_t* frame = "");
   void DrawEvent(KVEvent*, const Char_t* frame = "");
   void DrawEvent(KVEvent*, const Char_t* frame = "") const;

   void SetInput(TBranch* eventbranch);
   void SetInput(const char* filename);

   void ReadEvent();
   void ReadTextEvent();
   void ReadTreeEvent();

   void DrawNextEvent();

   virtual Bool_t SetHighlight(KVNucleus* n);
   virtual void SetHighlightMode(Int_t m)
   {
      // Select nuclei to highlight
      // m = KVEventViewer::kHighlightZmax : highlight biggest fragment (Z)
      // m = KVEventViewer::kHighlightParameter
      //                                   : highlight any nucleus for which
      //                                     KVParticle::GetParameters()->IsValue("EventViewer.Highlight",1)
      //                                     returns kTRUE
      // m = KVEventViewer::kNoHighlight   : turn off highlighting
      fHighlightMode = m;
   }

   void SetAxesMode(Bool_t on = kTRUE)
   {
      // Show axes or not
      fAxesMode = on;
   }

   void DumpCurrentEvent()
   {
      if (theEvent) theEvent->Print("ok");
   }

   ClassDef(KVEventViewer, 1) //Draw events in 3D using OpenGL
};

#endif
