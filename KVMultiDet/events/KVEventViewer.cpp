//Created by KVClassFactory on Fri Apr 12 15:48:03 2013
//Author: John Frankland,,,

#include "KVEventViewer.h"
#include "TGeoMatrix.h"
#include "TGLViewer.h"
#include "TGLLightSet.h"
#include "TVirtualPad.h"
#include "TSystem.h"
#include "TRandom.h"
#include "TGLPerspectiveCamera.h"
#include <KVNucleus.h>
#include <TBranch.h>
#include "KVTemplateEvent.h"
#include "TTree.h"

ClassImp(KVEventViewer)

KVEventViewer::KVEventViewer(Int_t protoncolor, Int_t neutroncolor, Int_t highlightprotoncolor, Int_t highlightneutroncolor, Double_t freenucleonradius, Double_t nucleonradius, Double_t nuclearradiusparameter)
   : fproton_color(protoncolor), fneutron_color(neutroncolor),
     fProton_color(highlightprotoncolor), fNeutron_color(highlightneutroncolor),
     free_nucleon_radius(freenucleonradius),
     nucleon_radius(nucleonradius),
     nuclear_radius_parameter(nuclearradiusparameter)
{
   // protoncolor             = color of protons
   // neutroncolor            = color of neutrons
   // highlight...color       = color of protons/neutrons in highlighted nuclei
   // freenucleonradius       = radius of free nucleons
   // nucleonradius           = radius of nucleons in nuclei
   // nuclearradiusparameter  = r0 in expression r=r0*A**1/3

   ivol = 0;

   geom = 0;

   fMaxVelocity = -1.;
   fFixSeed = kFALSE;
   fRefresh = 0;
   fSeed = 863167;
   fXYMode = kFALSE;
   fMomentumSpace = kFALSE;
   fScaleFactor = 1.;

   fSavePicture = kFALSE;
   textInput = kFALSE;
   theEvent = 0;

   fHighlightMode = kNoHighlight;
   fAxesMode = kFALSE;

   fFixPerspective = kFALSE;
}

KVEventViewer::~KVEventViewer()
{
   // Destructor
}

void KVEventViewer::DrawNucleus(KVNucleus* nucleus, const Char_t* frame)
{
   // Draw nucleus

   Int_t N = nucleus->GetN();
   Int_t Z = nucleus->GetZ();

   TVector3 V = (fMomentumSpace ? nucleus->GetFrame(frame, kFALSE)->GetMomentum() : nucleus->GetFrame(frame, kFALSE)->GetV());

   Bool_t Highlight = SetHighlight(nucleus);

   if (nucleus->GetA() == 1) {
      TGeoVolume* ball = 0;
      if (Z == 0) ball = geom->MakeSphere("n",  Nuc,  0., free_nucleon_radius);
      else if (Z == 1) ball = geom->MakeSphere("p",  Nuc,  0., free_nucleon_radius);
      int color = 0;
      if (Z == 0) color = (Highlight ? fNeutron_color : fneutron_color);
      else if (Z == 1) color = (Highlight ? fProton_color : fproton_color);
      ball->SetLineColor(color);
      top->AddNode(ball, ivol++, new TGeoTranslation(fScaleFactor * V.X(), fScaleFactor * V.Y(), fScaleFactor * V.Z()));
      return;
   }

   double sph_rad = pow(nuclear_radius_parameter, 3.) * (N + Z);

   for (int i = 0; i < N; i++) {
      TGeoVolume* ball = geom->MakeSphere("n",  Nuc,  0., nucleon_radius);
      ball->SetLineColor(Highlight ? fNeutron_color : fneutron_color);
      double rvec = pow(gRandom->Uniform(0., sph_rad), .33333);
      double x, y, z;
      gRandom->Sphere(x, y, z, rvec);
      top->AddNode(ball, ivol++, new TGeoTranslation(fScaleFactor * V.X() + x, fScaleFactor * V.Y() + y, fScaleFactor * V.Z() + z));
   }
   for (int i = 0; i < Z; i++) {
      TGeoVolume* ball = geom->MakeSphere("p",  Nuc,  0., nucleon_radius);
      ball->SetLineColor(Highlight ? fProton_color : fproton_color);
      double rvec = pow(gRandom->Uniform(0., sph_rad), .33333);
      double x, y, z;
      gRandom->Sphere(x, y, z, rvec);
      top->AddNode(ball, ivol++, new TGeoTranslation(fScaleFactor * V.X() + x, fScaleFactor * V.Y() + y, fScaleFactor * V.Z() + z));
   }
}

void KVEventViewer::DrawEvent(KVEvent* event, const Char_t* frame)
{
   // Draw all particles in event which are "ok"

   if (geom) delete geom;
   geom = new TGeoManager(Form("Event%d", event->GetNumber()), "Event view");
   geom->SetNsegments(500);
   matEmptySpace = new TGeoMaterial("EmptySpace", 0, 0, 0);
   matNuc     = new TGeoMaterial("Nuc", .938, 1., 10000.);
   matBox    = new TGeoMaterial("Box", .938, 1., 10000.);

   EmptySpace = new TGeoMedium("Empty", 1, matEmptySpace);
   Nuc     = new TGeoMedium("Nuc", 1, matNuc);
   Box     = new TGeoMedium("Box", 1, matBox);

   top = geom->MakeBox("WORLD", EmptySpace, 30, 30, 30);
   geom->SetTopVolume(top);

   // Find biggest velocity/momentum & biggest fragment
   maxV = 0;
   maxZ = 0;
   KVNucleus* nuc;
   if (fMomentumSpace) fScaleFactor = 1.e-2;
   else fScaleFactor = 1.;
   while ((nuc = event->GetNextParticle("ok"))) {
      Double_t v = fScaleFactor * (fMomentumSpace ? nuc->GetFrame(frame, kFALSE)->GetMomentum().Mag() :
                                   nuc->GetFrame(frame, kFALSE)->GetV().Mag());
      if (v > maxV) maxV = v;
      if (nuc->GetZ() > maxZ) maxZ = nuc->GetZ();
   }
   // scale down
   maxV *= 0.5;
   if (fFixPerspective) {
      // fixed perspective mode
      // calculate maxV once (first event), store it in fMaxVelocity, then use
      // for all other events
      if (fMaxVelocity > 0) maxV = fMaxVelocity;
      else fMaxVelocity = maxV;
   }
   else if (fMaxVelocity > 0) maxV = fMaxVelocity;

   TGeoVolume* box  = geom->MakeSphere("box",  Box,  0.99 * maxV, maxV);
   box->SetLineColor(kBlack);
   box->SetTransparency(100);
   top->AddNode(box, 1000);

   // set volume (nucleon) counter to 0
   ivol = 0;

   if (fFixSeed) {
      if (!(event->GetNumber() % fRefresh)) fSeed += 7;
      gRandom->SetSeed(fSeed);
   }

   while ((nuc = event->GetNextParticle("ok"))) DrawNucleus(nuc, frame);

   geom->CloseGeometry();

   top->SetVisContainers(kTRUE);
   top->Draw("ogl");
   TGLViewer* view = (TGLViewer*)gPad->GetViewer3D();
   if (fAxesMode) view->SetGuideState(2, 1, 0, 0);
   view->SetClearColor(kWhite);
   view->SetSmoothLines(1);
   view->SetSmoothPoints(1);
   view->GetLightSet()->SetLight(TGLLightSet::kLightTop, 1);
   view->GetLightSet()->SetLight(TGLLightSet::kLightBottom, 0);
   view->GetLightSet()->SetLight(TGLLightSet::kLightLeft, 1);
   view->GetLightSet()->SetLight(TGLLightSet::kLightRight, 0);
   view->GetLightSet()->SetLight(TGLLightSet::kLightFront, 1);
   view->GetLightSet()->SetLight(TGLLightSet::kLightSpecular, 1);
   view->SetCurrentCamera(TGLViewer::kCameraOrthoZOY);
   ((TGLOrthoCamera&)view->CurrentCamera()).SetEnableRotate(kTRUE);
   if (fXYMode) view->SetOrthoCamera(TGLViewer::kCameraOrthoZOY, 0.9, 0, 0, 0, TMath::Pi() / 2.); //TMath::Pi()/8.,TMath::Pi()/8.);
   else view->SetOrthoCamera(TGLViewer::kCameraOrthoZOY, 0.9, 0, 0, TMath::Pi() / 8., TMath::Pi() / 8.);
//   if(fSavePicture) view->SavePicture(Form("Event%d.gif",event->GetNumber()));
   if (fSavePicture) view->SavePicture(Form("Event-%d.png", event->GetNumber()));
}

void KVEventViewer::DrawEvent(KVEvent* event, const Char_t* frame) const
{
   const_cast<KVEventViewer*>(this)->DrawEvent(event, frame);
}

void KVEventViewer::SetInput(TBranch* eventbranch)
{
   // Read events from branch of a TTree

   SafeDelete(theEvent);
   eventbranch->SetAddress(&theEvent);
   textInput = kFALSE;
   theTree = eventbranch->GetTree();
   NtreeEntries = theTree->GetEntries();
   eventCounter = 0;
}

void KVEventViewer::SetInput(const char* filename)
{
   // Read events from file

   eventFile.open(filename);
   if (!eventFile.good()) {
      Error("SetInput", "Problem opening %s", filename);
      return;
   }
   eventCounter = 0;
   textInput = kTRUE;
}

void KVEventViewer::ReadEvent()
{
   // Read an event from input source

   if (textInput) {
      if (!theEvent) theEvent = new KVNucleusEvent;
      ReadTextEvent();
   }
   else {
      ReadTreeEvent();
   }

}

void KVEventViewer::ReadTextEvent()
{
   // Read event from text input
   // We assume a file containing, for each event,
   // the multiplicity, N,
   // followed by N lines containing Z Vx Vy Vz
   // the atomic number and velocity components (in cm/ns)

   theEvent->Clear();

   Double_t V[3];
   Int_t Z;

   Int_t nballs;
   eventFile >> nballs;
   eventCounter++;

   theEvent->SetNumber(eventCounter);

   for (int i = 0; i < nballs; i++) {

      eventFile >> Z >> V[0] >> V[1] >> V[2];

      KVNucleus* nuc = theEvent->AddParticle();
      nuc->SetZ(Z);
      TVector3 v(V);
      nuc->SetVelocity(v);

   }
}

void KVEventViewer::ReadTreeEvent()
{
   if (eventCounter == (Int_t)NtreeEntries) {
      Warning("ReadTreeEvent", "All events have been read");
      return;
   }
   theTree->GetEntry(eventCounter++);
}

void KVEventViewer::DrawNextEvent()
{
   // Draw next event read from input source

   ReadEvent();
   DrawEvent(theEvent);
}

Bool_t KVEventViewer::SetHighlight(KVNucleus* n)
{
   // Decide whether or not to highlight this nucleus in the event display

   if (fHighlightMode == kHighlightZmax && n->GetZ() == maxZ) return kTRUE;
   else if (fHighlightMode == kHighlightParameter && n->GetParameters()->IsValue("EventViewer.Highlight", 1)) return kTRUE;
   return kFALSE;
}
