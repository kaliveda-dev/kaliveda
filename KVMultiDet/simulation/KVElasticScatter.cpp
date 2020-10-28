/*
$Id: KVElasticScatter.cpp,v 1.9 2007/04/04 10:39:17 ebonnet Exp $
$Revision: 1.9 $
$Date: 2007/04/04 10:39:17 $
*/

//Created by KVClassFactory on Thu Oct 19 22:31:02 2006
//Author: John Frankland

#include "KVElasticScatter.h"
#include "KVMultiDetArray.h"
#include "TH1F.h"
#include "KVDetector.h"
#include "KVTelescope.h"
#include "KVGroup.h"
#include "KVTarget.h"
#include "KV2Body.h"
#include "TObjArray.h"

using namespace std;

ClassImp(KVElasticScatter)

KVElasticScatter::KVElasticScatter(): fBeamDirection(0, 0, 1)
{
   //Default constructor
   fDepth = fTheta = 0;
   fBinE = 500;
   fEnergy = 0;
   fKinematics = 0;
   fTelescope = 0;
   fTarget = 0;
   fIntLayer = fNDets = 0;
   fDetector = 0;
   fMultiLayer = kFALSE;
   fAlignedDetectors = 0;
   fExx = 0.;
   fHistos = 0;
   fDetInd = 0;
   if (gMultiDetArray) {
      gMultiDetArray->SetSimMode(kTRUE);
   }
   else {
      Warning("KVElasticScatter", "gMultiDetArray does not refer to a valid multidetector array");
      printf("Define it before using this class, and put it in simulation mode : gMultiDetArray->SetSimMode(kTRUE)");
   }
}

//__________________________________________________________________//

KVElasticScatter::~KVElasticScatter()
{
   //Destructor
   if (fDepth)
      delete fDepth;
   if (fTheta)
      delete fTheta;
   if (fKinematics)
      delete fKinematics;
   if (fHistos)
      delete fHistos;
   if (fDetInd)
      delete fDetInd;
   gMultiDetArray->SetSimMode(kFALSE);
}

//__________________________________________________________________//

void KVElasticScatter::SetRun(Int_t run)
{
   //Set detector parameters, target, etc. for run
   gMultiDetArray->SetParameters(run);
   fTarget = gMultiDetArray->GetTarget();
   fTarget->SetRandomized();
   fIntLayer = 0;
   fMultiLayer = (fTarget->NumberOfLayers() > 1);
}

//__________________________________________________________________//

void KVElasticScatter::SetProjectile(Int_t Z, Int_t A)
{
   //Set projectile Z and A

   fProj.SetZandA(Z, A);
}

//__________________________________________________________________//

void KVElasticScatter::SetEnergy(Double_t e)
{
   //Set energy of projectile in MeV

   fProj.SetEnergy(e);
   fEnergy = e;
}

//__________________________________________________________________//

void KVElasticScatter::SetDetector(const Char_t* det)
{
   //Set name of detector which will detect particle
   fDetector = gMultiDetArray->GetDetector(det);
   fTelescope = (KVTelescope*)fDetector->GetParentStructure("TELESCOPE");
   //get list of all detectors particle must pass through to get to required detector
   fAlignedDetectors =
      fDetector->GetAlignedDetectors(KVGroup::kForwards);
   //we store the association between detector type and index in list
   if (!fDetInd)
      fDetInd = new KVNameValueList;
   else
      fDetInd->Clear();
   KVDetector* d;
   TIter n(fAlignedDetectors);
   Int_t i = 0;
   while ((d = (KVDetector*) n())) {
      //check same type is not already present
      if (fDetInd->HasParameter(d->GetType())) {
         //detetors with same type will be called "Type_1", "Type_2" etc.
         TString newname;
         int j = 1;
         newname.Form("%s_%d", d->GetType(), j++);
         while (fDetInd->HasParameter(newname.Data()))
            newname.Form("%s_%d", d->GetType(), j++);
         fDetInd->SetValue(newname.Data(), i);
      }
      else {
         fDetInd->SetValue(d->GetType(), i);
      }
      i++;
   }
   //store number of aligned detectors
   fNDets = i;
}

//__________________________________________________________________//

void KVElasticScatter::SetTargetScatteringLayer(const Char_t* name)
{
   //For multilayer targets, use this method to choose in which
   //layer the scattering will take place.
   //If name="", reset any previous choice so that scattering
   //can take place in any layer
   if (!fTarget) {
      cout <<
           "<KVElasticScatter::SetTargetScatteringLayer> : No target set. Set run first."
           << endl;
      return;
   }
   fIntLayer = fTarget->GetLayerIndex(name);
   if (fIntLayer)
      fTarget->SetInteractionLayer(fIntLayer, fBeamDirection);
}

//__________________________________________________________________//

void KVElasticScatter::SetEbinning(Int_t nbins)
{
   //Set binning of the GetEnergy histogram
   // Default value is 500
   fBinE = nbins;
}
//__________________________________________________________________//

void KVElasticScatter::CalculateScattering(Int_t N)
{
   //Perform scattering 'N' times for current values
   //of particle Z, A and energy, target excited state, and detector.

   if (!fProj.IsDefined()) {
      cout <<
           "<KVElasticScatter::CalculateScattering> : Set projectile properties first"
           << endl;
      return;
   }
   if (!fEnergy) {
      cout <<
           "<KVElasticScatter::CalculateScattering> : Set projectile energy first"
           << endl;
      return;
   }
   if (!fDetector) {
      cout <<
           "<KVElasticScatter::CalculateScattering> : Set detector first" <<
           endl;
      return;
   }
   if (!fTarget) {
      cout <<
           "<KVElasticScatter::CalculateScattering> : No target set. Set run first."
           << endl;
      return;
   }

   /* -------------------------------------------------------------------------------------------------------------------------- */

   //set up histograms

   /* -------------------------------------------------------------------------------------------------------------------------- */

   // -- histograms for debugging: depth in target and angular distribution
   if (fDepth)
      delete fDepth;
   if (fTheta)
      delete fTheta;
   fDepth =
      new TH1F("hDepth", "Depth (mg/cm2)", 500, 0.,
               fTarget->GetTotalEffectiveThickness());
   fTheta = new TH1F("hTheta", "Theta (deg.)", 500, 0., 0.);

   /* -------------------------------------------------------------------------------------------------------------------------- */

   //set up histograms for all detectors particle passes through
   if (!fHistos) {
      fHistos = new TObjArray(fAlignedDetectors->GetSize());
      fHistos->SetOwner();      //will delete histos it stores
   }
   else {
      fHistos->Clear();         //delete any previous histograms
   }
   KVDetector* d;
   TIter n(fAlignedDetectors);
   while ((d = (KVDetector*) n())) {
      fHistos->
      Add(new
          TH1F(Form("hEloss_%s", d->GetName()), "Eloss (MeV)", fBinE, 0., 0.));
   }

   /* -------------------------------------------------------------------------------------------------------------------------- */

   //set up kinematics
   if (!fKinematics)
      fKinematics = new KV2Body;
   fProj.SetEnergy(fEnergy);
   fProj.SetTheta(0);

   /* -------------------------------------------------------------------------------------------------------------------------- */

   //set random interaction point for scattering
   if (fIntLayer) {
      fTarget->SetInteractionLayer(fIntLayer, fBeamDirection);
   }
   else {
      fTarget->GetInteractionPoint(&fProj);
   }

   /* -------------------------------------------------------------------------------------------------------------------------- */

   //get target nucleus properties from scattering layer
   TVector3 IP = fTarget->GetInteractionPoint();
   KVMaterial* targ_mat = fTarget->GetLayer(IP);
   KVNucleus t;
   t.SetZ((Int_t) targ_mat->GetZ());
   t.SetA((Int_t) targ_mat->GetMass());
   fKinematics->SetTarget(t);

   /* -------------------------------------------------------------------------------------------------------------------------- */

   //set excited state of target nucleus - in other words, dissipated energy for
   //reaction due to excitation of target
   fKinematics->SetEDiss(fExx);

   /* -------------------------------------------------------------------------------------------------------------------------- */

   Double_t xsec;
   for (int i = 0; i < N; i++) {
      //calculate slowing of incoming projectile
      fTarget->SetIncoming();
      fTarget->DetectParticle(&fProj);
      fKinematics->SetProjectile(fProj);
      fKinematics->SetOutgoing(fProj);
      fKinematics->CalculateKinematics();
      //set random direction of outgoing projectile

      double th, ph;
      th = ph = 0.;
      fDetector->GetRandomAngles(th, ph);

      //set energy of scattered nucleus
      //WARNING: for inverse kinematics reactions, their are two energies for
      //each angle below the maximum scattering angle.
      //We only use the highest energy corresponding to the most forward CM angle.
      Double_t e1, e2;
      fKinematics->GetELab(3, th, 3, e1, e2);
      fProj.SetEnergy(TMath::Max(e1, e2));
      fProj.SetTheta(th);
      fProj.SetPhi(ph);
      xsec = TMath::Abs(fKinematics->GetXSecRuthLab(fProj.GetTheta()));
      fTheta->Fill(fProj.GetTheta(), xsec);
      //slowing of outgoing projectile in target
      fTarget->SetOutgoing();
      fTarget->DetectParticle(&fProj);
      //now detect particle in detector(s)
      fAlignedDetectors->R__FOR_EACH(KVDetector, DetectParticle)(&fProj);
      //fill histograms
      fDepth->Fill(IP.z());
      int j = 0;
      n.Reset();
      while ((d = (KVDetector*) n())) {
         ((TH1F*)(*fHistos)[j++])->Fill(d->GetEnergy(), xsec);
         //prepare for next round: set energy loss to zero
         d->Clear();
      }
      fProj.SetEnergy(fEnergy);
      fProj.SetTheta(0);
      fProj.GetParameters()->Clear();
      //set random interaction point for scattering
      if (fIntLayer) {
         fTarget->SetInteractionLayer(fIntLayer, fBeamDirection);
      }
      else {
         fTarget->GetInteractionPoint(&fProj);
         //if target is multilayer and the interaction layer is not fixed,
         //the layer & hence the target nucleus may change
         if (fMultiLayer) {
            targ_mat = fTarget->GetLayer(fTarget->GetInteractionPoint());
            KVNucleus t;
            t.SetZandA((Int_t) targ_mat->GetZ(), (Int_t) targ_mat->GetMass());
            fKinematics->SetTarget(t);
         }
      }
      IP = fTarget->GetInteractionPoint();
   }
}

//__________________________________________________________________//


TH1F* KVElasticScatter::GetEnergy(const Char_t* type)
{
   //Energy loss in detector of given 'type' through which scattered particle passes.
   //Warning: if there are several detectors of the same type in the list of detectors
   //through which the particle passes, the first one (as seen by the impinging
   //particle) will have type "type", the second "type_1", the third "type_2", etc.

   return (fDetInd->HasParameter(type) ? GetEnergy(fDetInd->GetIntValue(type)) : 0);
}

//__________________________________________________________________//

TH1F* KVElasticScatter::GetEnergy(Int_t index)
{
   //Energy loss in any detector through which scattered particle passes.
   //The index corresponds to the order in which detectors are crossed by the
   //particle, beginning with 0 for the first detector, and ending with
   //(GetNDets()-1)
   return (TH1F*)(*fHistos)[index];
}

//__________________________________________________________________//
