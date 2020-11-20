/*
$Id: KVElasticScatterEvent.cpp,v 1.5 2009/01/14 15:35:50 ebonnet Exp $
$Revision: 1.5 $
$Date: 2009/01/14 15:35:50 $
*/

//Created by KVClassFactory on Thu Dec 11 14:45:29 2008
//Author: eric bonnet,,,

#include "KVElasticScatterEvent.h"
#include "KVPosition.h"
#include "KVASMultiDetArray.h"
#include "KVASGroup.h"
#include "KVDetector.h"
#include "KVTelescope.h"
#include "TH2F.h"
#include "TH1F.h"
#include "KVNamedParameter.h"
#include "KVUnits.h"

using namespace std;

ClassImp(KVElasticScatterEvent)



//_______________________________________________________________________
KVElasticScatterEvent::KVElasticScatterEvent()
{
   // Default constructor
   init();

}

//_______________________________________________________________________
KVElasticScatterEvent::~KVElasticScatterEvent()
{
   // Destructor
   delete proj;
   delete targ;
   if (kb2) delete kb2;

   if (sim_evt) delete sim_evt;
   if (rec_evt) delete rec_evt;

   ClearHistos();
   ClearTrees();
}

//_______________________________________________________________________
void KVElasticScatterEvent::init()
{
   //Initialisation des variables
   //par defaut
   //theta 0,180 et phi 0,360
   //tirage isotropique
   //noyau de reference pour la direction de diffusion Projectile

   kIPPVector.SetXYZ(0, 0, 0);

   kchoix_layer = -1;
   kXruth_evt = 0;
   kTreatedNevts = 0;

   th_min = 1e-3;
   th_max = 180;
   phi_min = 0;
   phi_max = 360;

   lhisto = 0;
   ltree = 0;

   targ = new KVNucleus();
   proj = new KVNucleus();

   rec_evt = 0;
   sim_evt = 0;
   ktarget = 0;
   kb2 = 0;

   ResetBit(kProjIsSet);
   ResetBit(kTargIsSet);
   ResetBit(kHasTarget);
   ResetBit(kIsUpdated);
   ResetBit(kIsDetectionOn);

   SetDiffNucleus("PROJ");
   SetRandomOption("isotropic");

   SetDetectionOn(kFALSE);
   ChooseKinSol(1);

}

//_______________________________________________________________________
void KVElasticScatterEvent::Reset()
{
   //Set contents/entries to zero for predifined histograms, trees
   kTreatedNevts = 0;
   ResetHistos();
   ResetTrees();

}

//_______________________________________________________________________
void KVElasticScatterEvent::SetSystem(KVDBSystem* sys)
{
   //Define the entrance channel using KVDBSystem object
   //Get projectile and target via KVDBSystem::GetKinematics()
   //Get target material using KVDBSystem::GetTarget()
   if (sys->GetKinematics()) {
      SetProjNucleus(sys->GetKinematics()->GetNucleus(1));
      SetTargNucleus(sys->GetKinematics()->GetNucleus(2));
      SetTargetMaterial(sys->GetTarget());
   }
   else {
      Error("SetSystem", "KVDBSystem pointer is not valid");
      return;
   }
}

//_______________________________________________________________________
void KVElasticScatterEvent::SetSystem(Int_t zp, Int_t ap, Double_t ekin, Int_t zt, Int_t at)
{
   //Define the entrance channel
   //zp, ap, ekin, atomic number, mass number and kinetic energy (MeV) of the projectile
   //zt, at, atomic number, mass number of the target
   KVNucleus nn(zp, ap, ekin);
   SetProjNucleus(&nn);
   nn.SetZAandE(zt, at, 0.0);
   SetTargNucleus(&nn);

}

//_______________________________________________________________________
void KVElasticScatterEvent::SetTargNucleus(KVNucleus* nuc)
{
   //Define new target nucleus
   if (!nuc) return;
   nuc->Copy(*targ);
   targ->SetName("TARG");
   SetBit(kTargIsSet);
   ResetBit(kIsUpdated);

}

//_______________________________________________________________________
void KVElasticScatterEvent::SetDiffNucleus(KVString name)
{
   //Defini le noyau auquel se réfère la direction de diffusion (theta, phi)
   //name="PROJ" (default)    diffusion du projectile
   //name="TARG"              diffusion de la cible

   if (name == "TARG") {
      kDiffNuc = 4;
   }
   else {
      kDiffNuc = 3;
      if (name != "PROJ") {
         Warning("SetDiffNucleus", "%s Le nom en argument doit etre PROJ ou TARG, par defaut on choisit le projectile", name.Data());
      }
   }
}

//_______________________________________________________________________
void KVElasticScatterEvent::SetRandomOption(Option_t* opt)
{
   //Defini le mode de tirage aleatoire pour l'angle polaire
   //opt="isotropic" ou "" (defaut) ou "random"
   //voir KVPosition::GetRandomDirection()
   kRandomOpt = opt;

}

//_______________________________________________________________________
Bool_t KVElasticScatterEvent::IsIsotropic() const
{

   //retoune kTRUE si l'option choisi est isotrope
   return strcmp(kRandomOpt, "random");

}

//_______________________________________________________________________
void KVElasticScatterEvent::ChooseKinSol(Int_t choix)
{
   //Dans le cas d'une cinematique inverse (Zproj>Ztarget)
   //deux solutions cinetiques sont possibles pour un meme angle de
   //diffusion du projectile
   //choix=1, on traite seulement la premiere solution ie diffusion a l arriere de la cible
   //choix=2, on traite seulement la deuxieme solution ie diffusion a l avant de la cible
   //choix=0, les deux solution sont traitees avec la meme probabilite
   if (choix >= 0 && choix <= 2)
      kChoixSol = choix;

}

//_______________________________________________________________________
void KVElasticScatterEvent::SetProjNucleus(KVNucleus* nuc)
{
   //Define new projectile nucleus
   if (!nuc) return;
   nuc->Copy(*proj);
   proj->SetName("PROJ");
   SetBit(kProjIsSet);
   ResetBit(kIsUpdated);
   proj->SetE0();

}

//_______________________________________________________________________
KVNucleus* KVElasticScatterEvent::GetNucleus(const Char_t* name) const
{
   //return the current projectile ("PROJ") or the target ("TARG") nucleus
   KVString sname(name);
   if (sname == "PROJ")      return proj;
   else if (sname == "TARG") return targ;
   else return 0;

}

//_______________________________________________________________________
KVNucleus* KVElasticScatterEvent::GetNucleus(Int_t ii) const
{
   //return the current projectile (ii=1) or the target (ii==2) nucleus
   if (ii == 1)        return proj;
   else if (ii == 2)   return targ;
   else return 0;

}

//_______________________________________________________________________
void KVElasticScatterEvent::SetTargetMaterial(KVTarget* targ, Bool_t IsRandomized)
{
   //Define a new target material where the nuclei will be propagated
   // if IsRandomized=kTRUE, the interaction point are randomly determined
   if (!targ) return;
   if (ktarget) delete ktarget;

   ktarget = new KVTarget(*targ);
   ktarget->SetRandomized(IsRandomized);
   SetBit(kHasTarget);
   ResetBit(kIsUpdated);
}

//_______________________________________________________________________
void KVElasticScatterEvent::SetDetectionOn(Bool_t On)
{

   if (gMultiDetArray) {
      gMultiDetArray->SetSimMode(On);
      gMultiDetArray->SetFilterType(KVMultiDetArray::kFilterType_Geo);
      SetBit(kIsDetectionOn, On);
      ResetBit(kIsUpdated);

   }
   else {
      if (On)
         Warning("MakeDetection", "Detection asked but no multiDetArray defined");
   }

}

//_______________________________________________________________________
KVTarget* KVElasticScatterEvent::GetTarget() const
{
   //return the current target material
   return ktarget;

}

//_______________________________________________________________________
void KVElasticScatterEvent::GenereKV2Body()
{
   //Protected Method called by ValidateEntrance() method
   //Genere the KV2Body object which manage the 2 body kinematics
   //for the elastik scatter
   //
   //Store the original momentum of the projectile nuclei
   //
   //
   //Define the KVEvent and KVReconstructedEvent pointer
   //where are stored the projectile/target nuclei couple after diffusion / detection
   //StartEvents() methods
   //
   //Make a copy of projectile and target nuclei for the KVEvent
   //

   Info("GenereKV2Body", "On genere le KV2Body");
   if (kb2) delete kb2;
   kb2 = new KV2Body(*proj, *targ);
   kb2->CalculateKinematics();

   //GetNucleus("PROJ")->SetE0();

   //Creer ou clear les deux pointeurs associes aux evts simules et reconstruits
   StartEvents();

   GetNucleus("PROJ")->Copy(*sim_evt->AddParticle());
   GetNucleus("TARG")->Copy(*sim_evt->AddParticle());
}

//_______________________________________________________________________
void KVElasticScatterEvent::StartEvents()
{
   //Define the KVEvent and KVReconstructedEvent pointer
   //where are stored the projectile/target nuclei couple after diffusion / detection
   if (!sim_evt)  sim_evt = new KVSimEvent();
   else           sim_evt->Clear();

   if (!rec_evt)  rec_evt = new KVReconstructedEvent();
   else           rec_evt->Clear();

}

//_______________________________________________________________________
Bool_t KVElasticScatterEvent::DefineTargetNucleusFromLayer(KVString layer_name)
{

   //Define the nucleus target from the type
   //of the given layer which belongs to the predefined target
   //layer_name has to be the chemical symbol of the material

   if (!IsTargMatSet())
      return kFALSE;

   if (GetTarget()->GetLayers()->GetEntries() == 0) return kFALSE;
   KVMaterial* mat = 0;
   if (layer_name == "") {
      kchoix_layer = 1;
      mat = GetTarget()->GetLayerByIndex(kchoix_layer);
   }
   else {
      if (!(mat = GetTarget()->GetLayer(layer_name))) {
         printf("le nom du layer %s ne correspond a aucun present dans le cible\n", layer_name.Data());
         printf("Attention le layer doit etre appele par son symbol (ie Calcium -> Ca)");
         ktarget->GetLayers()->Print();
         return kFALSE;
      }
      kchoix_layer = GetTarget()->GetLayerIndex(layer_name);
   }

   KVNucleus* nuc = new KVNucleus();
   nuc->SetZandA(TMath::Nint(mat->GetZ()), TMath::Nint(mat->GetMass()));

   SetTargNucleus(nuc);

   return kTRUE;

}

//_______________________________________________________________________
Bool_t KVElasticScatterEvent::ValidateEntrance()
{
   //Check if there is :
   // - one define projectile nuclei : SetProjNucleus()
   // - one define target nuclei : SetTargNucleus()
   // - one define material target : SetTargetMaterial() [Optionnel]
   //Si pas de noyau cible défini mais une cible est definie le noyau cible est definie a partir de celle-ci
   //Si la cible comporte plusieurs layers, le premier est choisi pour definir le noyau cible
   // see DefineTargetNucleusFromLayer() method
   //
   //
   //Check if the gMultiDetArray is valid, put it in SimMode in order to able the detection and reconstruction
   //If GetTarget() return kTRUE, put it in the gMultiDetArray
   //If not, check if there is already one in the gMultiDetArray
   //If there is one, use it for the following
   //If there is no target material at all make diffusion without
   //
   //Generate the KV2Body object to calculate kinematics of the elastic scatter
   //
   //if histograms and trees is defined do nothing for this objects
   //if not DefineTrees() and DefineHistos() are called.
   //if you want to regenerate histograms and/or trees
   //call ClearHistos() and/or ClearTrees() before using ValidateEntrance()
   //
   //Return kTRUE if everything is ready
   //

   if (!IsProjNucSet()) {
      Error("ValidateEntrance", "Il n'y a pas de noyau projectile -> use SetProjNucleus()");
      return kFALSE;
   }

   if (!IsTargNucSet()) {
      Info("ValidateEntrance", "Il n'y a pas de noyau cible");
      if (!IsTargMatSet()) {
         Error("ValidateEntrance", "Il n'y a pas de noyau cible -> use SetTargNucleus() ou SetTargetMaterial");
         return kFALSE;
      }
      else {
         if (DefineTargetNucleusFromLayer())
            Info("ValidateEntrance", "Definition du noyau cible via DefineTargetNucleusFromLayer()");
      }
   }

   if (IsDetectionOn()) {
      if (GetTarget()) {
         if (gMultiDetArray->GetTarget() && gMultiDetArray->GetTarget() ==  ktarget) {

         }
         else {
            //Fait un clone
            gMultiDetArray->SetTarget(GetTarget());
            delete ktarget;
            ktarget = gMultiDetArray->GetTarget();
         }
      }
      else if (gMultiDetArray->GetTarget()) {
         ktarget = gMultiDetArray->GetTarget();
      }
      else {
         Warning("ValidateEntrance", "Pas de calcul de perte dans la cible ... ");
      }
      //DefineAngularRange(gMultiDetArray);
   }
   else {
      Info("ValidateEntrance", "The elastic scatter events will not be detected/filtered");
   }

   GenereKV2Body();
   Info("ValidateEntrance", "Fin de GenereKV2Body");
   //Define histograms/trees only at the first call
   //of validate entrance

   if (!ltree) DefineTrees();

   ClearHistos();
   DefineHistos();

   SetBit(kIsUpdated);

   return kTRUE;

}


//_______________________________________________________________________
void KVElasticScatterEvent::Process(Int_t ntimes, Bool_t reset)
{
   //process ntimes elastic scatter
   //if reset=kTRUE, reset histograms, trees and counter before
   Info("Process", "Je Suis dans Process ... Youpee");

   if (!IsUpdated())
      if (!ValidateEntrance()) return;

   if (reset) Reset();
   Int_t nn = 0;
   while (nn < ntimes) {
      MakeDiffusion();
      nn += 1;
      if ((nn % 1000) == 0) {
         Info("Process", "%d/%d diffusions treated", nn, ntimes);
      }
   }
   Info("Process", "End of process : %d diffusions performed", kTreatedNevts);
}

//_______________________________________________________________________
void KVElasticScatterEvent::MakeDiffusion()
{
   //
   //Propagation dans la cible du projectile jusqu'au point d interaction
   //       PropagateInTargetLayer();
   //Tirage aleatoire d'un couple theta phi pour la direction de diffusion du projectile
   //Determination de la cinematique de la voie de sortie
   //    SetAnglesForDiffusion(the,phi);
   //Filtre si un multidetecteur est defini
   //    Filter
   //Traitement de l evt (remplissage d'arbre ou d'histo
   //    TreateEvent();

   KVNucleus* knuc = 0;
   sim_evt->GetParameters()->Clear();
   while ((knuc = sim_evt->GetNextParticle())) {
      knuc->GetParameters()->Clear();
      knuc->RemoveAllGroups();
   }

   //-------------------------
   if (IsTargMatSet()) {
      NewInteractionPointInTargetLayer();
      PropagateInTargetLayer();
   }
   //-------------------------


   Double_t tmin = GetTheta("min");
   if (tmin >= kb2->GetMaxAngleLab(kDiffNuc)) {
      GetNucleus("PROJ")->SetMomentum(*GetNucleus("PROJ")->GetPInitial());
      return;
   }
   Double_t tmax = GetTheta("max");

   if (tmax <= kb2->GetMinAngleLab(kDiffNuc)) {
      GetNucleus("PROJ")->SetMomentum(*GetNucleus("PROJ")->GetPInitial());
      return;
   }

   if (tmin < kb2->GetMinAngleLab(kDiffNuc)) tmin = kb2->GetMinAngleLab(kDiffNuc);
   if (tmax > kb2->GetMaxAngleLab(kDiffNuc)) tmax = kb2->GetMaxAngleLab(kDiffNuc);

   Double_t pmin = GetPhi("min");
   Double_t pmax = GetPhi("max");

   kposalea.SetPolarMinMax(tmin, tmax);
   kposalea.SetAzimuthalMinMax(pmin, pmax);

   Double_t th_deg, ph_deg;
   kposalea.GetRandomAngles(th_deg, ph_deg, kRandomOpt);

   SetAnglesForDiffusion(th_deg, ph_deg);

   ((TH2F*)lhisto->FindObject("phi_theta"))->Fill(th_deg, ph_deg);
   ((TH1F*)lhisto->FindObject("costheta"))->Fill(TMath::Cos(TMath::DegToRad()*th_deg));


   if (IsDetectionOn()) {
      Filter();
   }
   else {
      if (IsTargMatSet())
         SortieDeCible();
   }

   TreateEvent();

   kTreatedNevts += 1;
}

//_______________________________________________________________________
void KVElasticScatterEvent::NewInteractionPointInTargetLayer()
{
   //Choose a new interaction point in the current target layer
   //This position can be read via the GetInteractionPointInTargetLayer()
   //method
   if (kchoix_layer != -1) {
      TVector3 dir = GetNucleus("PROJ")->GetMomentum();
      ktarget->SetInteractionLayer(kchoix_layer, dir);
      //kIPPVector = ktarget->GetInteractionPoint();
   }
   kIPPVector = ktarget->GetInteractionPoint(GetNucleus("PROJ"));
   ((TH1F*)lhisto->FindObject("target_layer_depth"))->Fill(kIPPVector.Z());
}

//_______________________________________________________________________
void KVElasticScatterEvent::PropagateInTargetLayer()
{
   //Apply Energy loss calculation to the entering projectile
   //along its path in the target layer to the interation point
   //
   //if a gMultiDetArray is defined the outgoing (after diffusion) pathes are not treated here but
   //in the Filter() method
   //
   //if not is treated in the SortieDeCible method

   Double_t eLostInTarget = GetNucleus("PROJ")->GetKE();
   ktarget->SetIncoming(kTRUE);
   ktarget->DetectParticle(GetNucleus("PROJ"), 0);
   eLostInTarget -= GetNucleus("PROJ")->GetKE();

   //Energie perdue jusqu'au point d interaction
   sim_evt->GetParticleWithName("PROJ")->GetParameters()->SetValue("TARGET In", eLostInTarget);
   //On modifie l'energie du projectile dans KV2Body
   //pour prendre en compte l energie deposee dans la cible
   //avant de faire le calcul de la cinematique
   if (GetNucleus("PROJ")->GetKE() == 0) {
      GetNucleus("PROJ")->Print();
      printf("%lf / %lf\n", eLostInTarget, proj->GetKE());
   }
   kb2->GetNucleus(1)->SetKE(GetNucleus("PROJ")->GetKE());
   kb2->CalculateKinematics();

   ktarget->SetIncoming(kFALSE);

}

//_______________________________________________________________________
TVector3& KVElasticScatterEvent::GetInteractionPointInTargetLayer()
{
   //return the last interaction point in the target
   return kIPPVector;

}


//_______________________________________________________________________
void KVElasticScatterEvent::SortieDeCible()
{
   //Apply Energy loss calculation in target material for the outgoing projectile
   //and target
   //

   ktarget->SetOutgoing(kTRUE);
   KVNucleus* knuc = 0;
   while ((knuc = sim_evt->GetNextParticle())) {
      knuc->SetE0();
      Double_t eLostInTarget = knuc->GetKE();
      ktarget->DetectParticle(knuc, 0);
      eLostInTarget -= knuc->GetKE();
      knuc->GetParameters()->SetValue("TARGET Out", eLostInTarget);
      knuc->SetMomentum(*knuc->GetPInitial());
   }

   ktarget->SetOutgoing(kFALSE);

}


//_______________________________________________________________________
void KVElasticScatterEvent::SetAnglesForDiffusion(Double_t theta, Double_t phi)
{
   //Determination a partir du theta choisi
   //de l'energie cinetique du projectile diffuse
   //All kinematics properties calculated in the KV2Body class
   //are accessible via the KV2Body& GetKinematics() method
   //
   // WARNING: in inverse kinematics, there are two projectile energies
   // for each lab angle.
   Double_t ediff;
   Double_t ediff1, ediff2;
   Int_t nsol_kin;

   nsol_kin = kb2->GetELab(kDiffNuc, theta, kDiffNuc, ediff1, ediff2);

   if (nsol_kin == 1) {
      ediff = ediff1;
   }
   else {
      if (kChoixSol == 1) ediff = ediff1;
      else if (kChoixSol == 2) {
         ediff = ediff2;
      }
      else {
         Int_t choix = TMath::Nint(gRandom->Uniform(0.5, 2.5));
         if (choix == 2)  ediff = ediff2;
         else           ediff = ediff1;
      }
   }
   Bool_t sol2 = (ediff == ediff2);

   kXruth_evt = kb2->GetXSecRuthLab(theta, kDiffNuc);

   //On modifie l energie et les angles du projectile ou cible diffusé(e)
   //puis par conservation, on deduit ceux du noyau complementaire
   KVNucleus* knuc = 0;

   if (kDiffNuc == 3)
      knuc = sim_evt->GetParticleWithName("PROJ");
   else
      knuc = sim_evt->GetParticleWithName("TARG");

   knuc->SetKE(ediff);
   knuc->SetTheta(theta);
   knuc->SetPhi(phi);
   ((TH2F*)lhisto->FindObject("ek_theta"))->Fill(knuc->GetTheta(), knuc->GetKE());

   //Conservation de l impulsion
   //Conservation de l energie tot
   TVector3 ptot = proj->Vect() + targ->Vect();
   Double_t etot = proj->E() + targ->E();
   //on retire la contribution du noyau diffusé
   ptot -= knuc->Vect();
   etot -= knuc->E();
   //on met a jour les pptés la cible ou projectile diffusé(e)

   if (kDiffNuc == 3)
      knuc = sim_evt->GetParticleWithName("TARG");
   else
      knuc = sim_evt->GetParticleWithName("PROJ");

   TLorentzVector q(ptot, etot);
   knuc->Set4Mom(q);

   ((TH2F*)lhisto->FindObject("ek_theta"))->Fill(knuc->GetTheta(), knuc->GetKE());

   sim_evt->SetNumber(kTreatedNevts);

   sim_evt->GetParameters()->SetValue("XRuth", kXruth_evt);
   sim_evt->GetParameters()->SetValue("ThDiff", theta);
   sim_evt->GetParameters()->SetValue("EkDiff", ediff1);
   sim_evt->GetParameters()->SetValue("IPz", kIPPVector.Z());

   if (sol2)
      sim_evt->GetParameters()->SetValue("Sol2", 1);

   //L' energie cinetique du projectile est reinitialisee
   //pour la prochaine diffusion
   GetNucleus("PROJ")->SetMomentum(*GetNucleus("PROJ")->GetPInitial());

}

//_______________________________________________________________________
void KVElasticScatterEvent::Filter()
{
   //Simulate passage of the projectile/target couple
   //through the multidetector refered by the gMultiDetArray pointer
   //if it is not valid do nothing

   gMultiDetArray->DetectEvent(sim_evt, rec_evt);


}

//_______________________________________________________________________
void KVElasticScatterEvent::TreateEvent()
{
   //Rempli l'arbre ElasticScatter
   //Boucle sur tous les parametres associés a l evt (KVEvent::GetParameters() et au projectiles et cible
   //qui le constituent GetParticle(1)->GetParameters()
   // Chaque parametre devient un alias de l'arbre ElasticScatter
   // pour une utilisation a posteriori plus facile.
   // -  pour les parametres de l'evt, on donne directement le nom du parametre
   // -  pour les particule :
   //    N1_[nom_du parametre] pour les projectiles et N2_[nom_du parametre] pour les cibles diffusés
   //
   // Exemple avec l'utilisation de TTree::Draw
   // Si on veut voir le spectre en energie laissé par les projectiles diffuses dans la "CI_0601"
   // au lieu de faire
   //    GetTree("Simulated_evts")->Draw("Simulated_evts->GetParticle(1)->GetParameters()->GetValue(\"CI_0601\")")
   // on fera
   //    GetTree("Simulated_evts")->Draw("N1_CI_0601")
   //
   // Generation des correlation Energie Cinetique (Ek) vs Angle de diffusion (theta)
   // pour tous les cas de détection

   if (rec_evt->GetMult() > 0) {
      TTree* tt = (TTree*)ltree->FindObject("ElasticScatter");
      tt->Fill();
   }
   /*
   TString snom,stit;
   KVNamedParameter* nm = 0;

   TIter it(sim_evt->GetParameters()->GetList());
   while ( (nm = (KVNamedParameter* )it.Next()) ){
      snom.Form("%s",nm->GetName());
      if (snom.Contains(" ")) snom.ReplaceAll(" ","_");
      stit.Form("Simulated_evts->GetParameters()->GetDoubleValue(\"%s\")",nm->GetName());
      tt->SetAlias(snom.Data(),stit.Data());
   }
   TIter it1(sim_evt->GetParticle(1)->GetParameters()->GetList());
   while ( (nm = (KVNamedParameter* )it1.Next()) ){
      snom.Form("N1_%s",nm->GetName());
      if (snom.Contains(" ")) snom.ReplaceAll(" ","_");
      stit.Form("Simulated_evts->GetParticle(1)->GetParameters()->GetDoubleValue(\"%s\")",nm->GetName());
      tt->SetAlias(snom.Data(),stit.Data());
   }
   TIter it2(sim_evt->GetParticle(2)->GetParameters()->GetList());
   while ( (nm = (KVNamedParameter* )it2.Next()) ){
      snom.Form("N2_%s",nm->GetName());
      if (snom.Contains(" ")) snom.ReplaceAll(" ","_");
      stit.Form("Simulated_evts->GetParticle(2)->GetParameters()->GetDoubleValue(\"%s\")",nm->GetName());
      tt->SetAlias(snom.Data(),stit.Data());
   }

   if (IsDetectionOn()){
   TH2F* hh = 0;
   KVNucleus* knuc = 0;
   while ( (knuc = sim_evt->GetNextParticle()) ){

      if ( knuc->GetParameters()->HasParameter("DETECTED") ){
         if ( !strcmp(knuc->GetParameters()->GetStringValue("DETECTED"),"") ){
            snom.Form("ek_theta_DETECTED");
            if ( !(hh = (TH2F* )lhisto->FindObject(snom.Data())) ){
               Double_t totalE = GetNucleus(1)->GetKE();
               lhisto->Add(new TH2F(snom.Data(),"DETECTED",180,0,180,TMath::Nint(totalE*11),0,totalE*1.1));
               hh = (TH2F* )lhisto->Last(); hh->SetXTitle("#theta_{ lab}"); hh->SetYTitle("Ek_{ lab}");
            }
         }
         else {
            snom.Form("ek_theta_%s",knuc->GetParameters()->GetStringValue("DETECTED"));
            if ( !(hh = (TH2F* )lhisto->FindObject(snom.Data())) ){
               Double_t totalE = GetNucleus(1)->GetKE();
               lhisto->Add(new TH2F(snom.Data(),knuc->GetParameters()->GetStringValue("DETECTED"),180,0,180,TMath::Nint(totalE*11),0,totalE*1.1));
               hh = (TH2F* )lhisto->Last(); hh->SetXTitle("#theta_{ lab}"); hh->SetYTitle("Ek_{ lab}");
            }
         }
      }
      else if ( knuc->GetParameters()->HasParameter("UNDETECTED") ){
         if ( !strcmp(knuc->GetParameters()->GetStringValue("UNDETECTED"),"") ){
            snom.Form("ek_theta_UNDETECTED");
            if ( !(hh = (TH2F* )lhisto->FindObject(snom.Data())) ){
               Double_t totalE = GetNucleus(1)->GetKE();
               lhisto->Add(new TH2F(snom.Data(),"UNDETECTED",180,0,180,TMath::Nint(totalE*11),0,totalE*1.1));
               hh = (TH2F* )lhisto->Last(); hh->SetXTitle("#theta_{ lab}"); hh->SetYTitle("Ek_{ lab}");
            }
         }
         else {
            snom.Form("ek_theta_%s",knuc->GetParameters()->GetStringValue("UNDETECTED"));
            if ( !(hh = (TH2F* )lhisto->FindObject(snom.Data())) ){
               Double_t totalE = GetNucleus(1)->GetKE();
               lhisto->Add(new TH2F(snom.Data(),knuc->GetParameters()->GetStringValue("UNDETECTED"),180,0,180,TMath::Nint(totalE*11),0,totalE*1.1));
               hh = (TH2F* )lhisto->Last(); hh->SetXTitle("#theta_{ lab}"); hh->SetYTitle("Ek_{ lab}");
            }
         }
      }
      else {
         knuc->Print();
      }
      if (hh)
         hh->Fill(knuc->GetTheta(),knuc->GetKE());
   }
   }
   */

}

//_______________________________________________________________________
void KVElasticScatterEvent::Print(Option_t* /*opt*/) const
{

   kb2->Print();

   printf("#####################\n");
   printf("## KVElasticScatterEvent::Print() ##\n");
   printf("# Diffusion elastique traitee :\n");
   printf("# %s+%s@%1.2lf MeV/A\n",
          GetNucleus(1)->GetSymbol(),
          GetNucleus(2)->GetSymbol(),
          GetNucleus(1)->GetKE() / GetNucleus(1)->GetA()
         );
   if (IsTargMatSet()) {
      printf("-------------------------\n");
      printf("# Propagation dans une cible de:\n");
      for (Int_t nn = 0; nn < GetTarget()->GetLayers()->GetEntries(); nn += 1) {
         Double_t epaiss = GetTarget()->GetLayerByIndex(nn + 1)->GetAreaDensity() / (KVUnits::mg / pow(KVUnits::cm, 2));
         printf("#\ttype:%s epaisseur:%lf (mg/cm**2) / %lf\n",
                GetTarget()->GetLayerByIndex(nn + 1)->GetType(),
                epaiss,
                GetTarget()->GetLayerByIndex(nn + 1)->GetThickness()
               );
      }
   }
   printf("-------------------------\n");
   if (IsDetectionOn()) {
      printf("# Detection par %s\n", gMultiDetArray->GetName());
   }
   printf("#####################\n");

}
//_______________________________________________________________________
void KVElasticScatterEvent::DefineHistos()
{
   //Definition of control histograms
   //- phi_theta : filled with angles choosen to determine the direction of the diffused projectile
   //- target_layer_depth : interaction point position in the target
   //- ek_theta : filled with energies and polar angles of projectile and target nuclei after diffusion
   //they are detected by the multidetarray
   Info("DefineHistos", "DefineHistos");
   lhisto = new KVHashList();
   lhisto->SetOwner(kTRUE);
   TH2F* h2 = 0;
   TH1F* h1 = 0;

   lhisto->Add(new TH2F("phi_theta", "phi_theta", 180, 0, 180, 360, 0, 360));
   h2 = (TH2F*)lhisto->Last();
   h2->SetXTitle("#theta_{ lab}");
   h2->SetYTitle("#phi_{ lab}");
   lhisto->Add(new TH1F("costheta", "costheta", 200, -1, 1));
   h1 = (TH1F*)lhisto->Last();
   h1->SetXTitle("cos #theta_{ lab}");
   if (IsTargMatSet()) {
      Info("DefineHistos", "Creation de l histo interaction dans la cible");
      Float_t thickness = GetTarget()->GetThickness();
      lhisto->Add(new TH1F("target_layer_depth", "target_layer_depth", TMath::Nint(thickness * 110), 0, thickness * 1.1));
      h1 = (TH1F*)lhisto->Last();
      h1->SetXTitle("IP position (mg / cm**2)");
   }
   Float_t totalE = GetNucleus(1)->GetKE();
   lhisto->Add(new TH2F("ek_theta", "ek_theta", 180, 0, 180, TMath::Nint(totalE * 11), 0, totalE * 1.1));
   h2 = (TH2F*)lhisto->Last();
   h2->SetXTitle("#theta_{ lab}");
   h2->SetYTitle("Ek_{ lab}");

}

//_______________________________________________________________________
KVHashList* KVElasticScatterEvent::GetHistos() const
{
   //return the list where histo are stored
   return lhisto;

}
//_______________________________________________________________________
void KVElasticScatterEvent::ResetHistos()
{

   //Reset histo in the list
   lhisto->Execute("Reset", "");

}
//_______________________________________________________________________
void KVElasticScatterEvent::ClearHistos()
{
   //Efface la liste des histo et leur contenu et met le pointeur a zero
   if (lhisto) delete lhisto;
   lhisto = 0;

}

//_______________________________________________________________________
void KVElasticScatterEvent::DefineTrees()
{
   //Definition of trees
   //
   //Par defaut un seul arbre : ElasticScatter where simulated events are stored
   //sous forme de KVEvent
   //Lors du remplissage de l arbre ( methode TreateEvent)
   //les parametres associes au KVEvent::GetParameters et au KVNucleus::GetParameters (projectile et cible)
   //sont scannés et leur nom et le moyen d'y accéder ajouté aux alias de l arbre pour une utilisation
   //plus aisé de celui_ci
   //

   Info("DefineTrees", "DefineTrees");
   ltree = new KVHashList();
   ltree->SetOwner(kTRUE);
   TTree* tt = 0;

   tt = new TTree("ElasticScatter", IsA()->GetName());
   KVEvent::MakeEventBranch(tt, "Simulated_evts", "KVSimEvent", sim_evt);
   ltree->Add(tt);
}

//_______________________________________________________________________
KVHashList* KVElasticScatterEvent::GetTrees() const
{
   //return the list where histo are stored
   return ltree;

}

//_______________________________________________________________________
void KVElasticScatterEvent::ClearTrees()
{
   //Efface la liste des arbres et leur contenu et met le pointeur a zero
   if (ltree) delete ltree;
   ltree = 0;

}

//_______________________________________________________________________
void KVElasticScatterEvent::ResetTrees()
{
   //Reset the tree contents
   //and aliases for the "ElasticScatter" tree
   ltree->Execute("Reset", "");
   if (((TTree*)ltree->FindObject("ElasticScatter"))->GetListOfAliases())
      ((TTree*)ltree->FindObject("ElasticScatter"))->GetListOfAliases()->Clear();

}

//_______________________________________________________________________
void KVElasticScatterEvent::DefineAngularRange(Double_t tmin, Double_t tmax, Double_t pmin, Double_t pmax)
{
   //Define in which angular (polar and azimuthal) range
   //The projectile diffusion direction will be randomized
   //If this method is not used
   //Default range is \theta [0,180] and \phi [0,360]
   if (tmin != -1) th_min = tmin;
   if (tmax != -1) th_max = tmax;
   if (pmin != -1) phi_min = pmin;
   if (pmax != -1) phi_max = pmax;

}

//_______________________________________________________________________
void KVElasticScatterEvent::DefineAngularRange(TObject* obj)
{
   //Define in which angular (polar and azimuthal) range
   //The projectile diffusion direction will be randomized
   //From the geometry of the given object obj
   //This method is defined for object derived from
   // - KVPosition (ie KVTelescope KVRing KVGroup etc ...)
   // - KVDetector (in this case, the KVTelescope it belongs to is used)
   // - KVMultiDetArray

   Double_t tmin = -1, tmax = -1, pmin = -1, pmax = -1;
   if (obj->InheritsFrom("KVPosition")) {
      KVPosition* pos_obj = (KVPosition*)obj;
      tmin = pos_obj->GetThetaMin();
      tmax = pos_obj->GetThetaMax();
      pmin = pos_obj->GetPhiMin();
      pmax = pos_obj->GetPhiMax();
   }
   else if (obj->InheritsFrom("KVDetector")) {
      KVTelescope* pos_obj = (KVTelescope*)((KVDetector*)obj)->GetParentStructure("TELESCOPE");
      tmin = pos_obj->GetThetaMin();
      tmax = pos_obj->GetThetaMax();
      pmin = pos_obj->GetPhiMin();
      pmax = pos_obj->GetPhiMax();
   }
   else if (obj->InheritsFrom("KVASMultiDetArray")) {
      Warning("DefineAngularRange(KVASMultiDetArray*)", "Needs reimplementing");
      /*KVASMultiDetArray* pos_obj=(KVASMultiDetArray* )obj;
      KVSeqCollection* ll = pos_obj->GetGroups();
            KVASGroup* gr=0;
       Double_t tmin2=180,tmax2=0;
       Double_t pmin2=360,pmax2=0;
       for (Int_t nl=0;nl<ll->GetEntries();nl+=1){
                gr = (KVASGroup* )ll->At(nl);
          if (gr->GetThetaMin()<tmin2)  tmin2 = gr->GetThetaMin();
          if (gr->GetThetaMax()>tmax2)  tmax2 = gr->GetThetaMax();
          if (gr->GetPhiMin()<pmin2)    pmin2 = gr->GetPhiMin();
          if (gr->GetPhiMax()>pmax2)    pmax2 = gr->GetPhiMax();
       }
       tmin = tmin2;
       tmax = tmax2;
       pmin = pmin2;
            pmax = pmax2;*/
   }
   else {
      printf("les objects de type %s ne sont pas implemente dans KVElasticScatterEvent::DefineAngularRange\n", obj->IsA()->GetName());
      return;
   }

   DefineAngularRange(tmin, tmax, pmin, pmax);

}

//_______________________________________________________________________
Double_t KVElasticScatterEvent::GetTheta(KVString opt) const
{
   //Return the limite in theta range (polar angle)
   //opt has to be "min" or "max"
   if (opt == "min") return th_min;
   else if (opt == "max") return th_max;
   else return -1;

}

//_______________________________________________________________________
Double_t KVElasticScatterEvent::GetPhi(KVString opt) const
{

   //Return the limite in phi range (azimuthal angle)
   //opt has to be "min" or "max"
   if (opt == "min") return phi_min;
   else if (opt == "max") return phi_max;
   else return -1;

}
