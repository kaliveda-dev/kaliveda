//Created by KVClassFactory on Fri Sep 11 23:31:56 2020
//Author: henri

#include "KVSimReader_DIT.h"

ClassImp(KVSimReader_DIT)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVSimReader_DIT</h2>
<h4></h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVSimReader_DIT::KVSimReader_DIT(KVString filename, KVString proj, KVString targ, Double_t ener) : KVSimReader()
{
   projectile = proj;
   target = targ;
   energy = ener;
   //
   InitReader();
   ConvertEventsInFile(filename);
   SaveTree();
}

//____________________________________________________________________________//

KVSimReader_DIT::~KVSimReader_DIT()
{
   // Destructor
}

//____________________________________________________________________________//

void KVSimReader_DIT::ConvertEventsInFile(KVString filename)
{
   if (!OpenFileToRead(filename)) return;

   SkipFirstLine();
   Run();
   CloseFile();
}

//____________________________________________________________________________//

void KVSimReader_DIT::ReadFile()
{
   while (IsOK()) {
      while (ReadEvent()) {
         if ((nevt % 10000) == 0) Info("ReadFile", "%d events read", nevt);
         if (HasToFill()) FillTree();
      }
   }
}

//____________________________________________________________________________//

Bool_t KVSimReader_DIT::ReadEvent()
{
   evt->Clear();
   //
   Int_t res = ReadLineAndCheck(21, " ");

   switch (res) {
      case 0 :
         Info("ReadEvent", "Line is empty");
         return kFALSE;

      case 1 :
         evt->SetNumber(nevt);
         StoreEventInVariables();
         FillEvent();
         break;

      default :
         return kFALSE;
   }

   nevt++;

   return kTRUE;
}

//____________________________________________________________________________//

void KVSimReader_DIT::StoreEventInVariables()
{
   weight = GetDoubleReadPar(0);
   inicident_partial_wave = GetDoubleReadPar(1);
   impact_parameter = GetDoubleReadPar(2);
   itype = GetIntReadPar(3);
   a_proj_like = GetIntReadPar(4);
   a_targ_like = GetIntReadPar(5);
   z_proj_like = GetIntReadPar(6);
   z_targ_like = GetIntReadPar(7);
   excitation_energy_proj_like = GetDoubleReadPar(8);
   excitation_energy_targ_like = GetDoubleReadPar(9);
   spin_proj_like = GetDoubleReadPar(10);
   spin_targ_like = GetDoubleReadPar(11);
   energy_proj_like = GetDoubleReadPar(12);
   energy_targ_like = GetDoubleReadPar(13);
   theta_proj_like = GetDoubleReadPar(14);
   theta_targ_like = GetDoubleReadPar(15);
   theta_cm = GetDoubleReadPar(16);
   q_reaction = GetDoubleReadPar(17);
   k_n = GetIntReadPar(18);
   k_p = GetIntReadPar(19);
   min_dist = GetIntReadPar(20);
}

//____________________________________________________________________________//

void KVSimReader_DIT::FillEvent()
{
   // Fill projectile-like nucleus
   nuc = static_cast<KVSimNucleus*>(evt->AddParticle());

   nuc->SetZandA(z_proj_like, a_proj_like);
   nuc->SetE(energy_proj_like);
   nuc->SetTheta(TMath::Abs(theta_proj_like));

   if (itype == DITCollType::DITCollOk) {
      if (theta_proj_like < 0.) {
         nuc->SetPhi(270.);
      }
      else {
         nuc->SetPhi(90.);
      }
   }
   else {
      nuc->SetPhi(0.);
   }

   nuc->SetExcitEnergy(excitation_energy_proj_like);
   nuc->SetSpin(spin_proj_like, 0., 0.); // Spin component given by the model is normal to the reaction plane, this last being (YZ), thus the spin is on the X-axis
   //
   // Fill target-like nucleus if it exists (meaning he itype code is 0, a DIC event and not a fusion
   if (itype == DITCollType::DITCollOk) {
      nuc = static_cast<KVSimNucleus*>(evt->AddParticle());

      nuc->SetZandA(z_targ_like, a_targ_like);
      nuc->SetE(energy_targ_like);
      nuc->SetTheta(TMath::Abs(theta_targ_like));

      if (theta_targ_like < 0.) {
         nuc->SetPhi(270.);
      }
      else {
         nuc->SetPhi(90.);
      }

      nuc->SetExcitEnergy(excitation_energy_targ_like);
      nuc->SetSpin(spin_targ_like, 0., 0.); // Spin component given by the model is normal to the reaction plane, this last being (YZ), thus the spin is on the X-axis
   }
   //
   // Add parameters to event
   evt->SetParameter("weight", weight);
   evt->SetParameter("inicident_partial_wave", inicident_partial_wave);
   evt->SetParameter("impact_parameter", impact_parameter);
   evt->SetParameter("itype", itype);
   evt->SetParameter("theta_cm", theta_cm);
   evt->SetParameter("q_reaction", q_reaction);
   evt->SetParameter("k_n", k_n);
   evt->SetParameter("k_p", k_p);
   evt->SetParameter("min_dist", min_dist);
}

//____________________________________________________________________________//

void KVSimReader_DIT::InitReader()
{
   tree_name = "dit_events";
   tree_title = "dit_events";
   branch_name = "DITKVSimEvents";
   root_file_name = Form("DIT_%s+%s@%.1fMeV.root", projectile.Data(), target.Data(), energy);
}

//____________________________________________________________________________//

void KVSimReader_DIT::SkipFirstLine()
{
   //
   // Skip first line of the file events.dat, which should be :
   // WEIGHT/F:ANGM0:IMPAR:ITYPE/I:A[2]:Z[2]:EX[2]/F:SPIN[2]:ELAB[2]:THETA[2]:THCM:Q:KN/I:KP:SMIN/F
   //
   ReadLine(0);
   Info("SkipFirstLine", "Skipping firt line of simulated data file!");
}

//____________________________________________________________________________//
