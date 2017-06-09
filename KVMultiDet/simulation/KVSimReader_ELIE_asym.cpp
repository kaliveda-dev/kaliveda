//Created by KVClassFactory on Wed Jan 11 10:29:05 2017
//Author: John Frankland,,,

#include "KVSimReader_ELIE_asym.h"

ClassImp(KVSimReader_ELIE_asym)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVSimReader_ELIE_asym</h2>
<h4>Read ELIE events after secondary decay</h4>
<!-- */
// --> END_HTML
// This class is for reading asymptotic events, i.e. output files with names
// like: elie_58_197_32_1000_secondary.output
// The events will be stored in a ROOT file with a name like:
// ELIE_[PROJ]_[TARG]_[EBEAM]AMeV_ASYM.root
// This file will contain a TTree called "ELIE" with a branch "Simulated_evts"
// All the informations in the header of the ELIE output file are stored in
// a KVNameValueList in the TTree:
//   ELIE->GetUserInfo()->FindObject("ELIE Parameters")->ls()
//KVNameValueList::ELIE Parameters :  (0x55b2067172f0)
// <ievt=50>
// <projectile=Ni>
// <target=Au>
// <a_proj=58>
// <z_proj=28>
// <a_targ=197>
// <z_targ=79>
// <ebeam=32>
// <ilevel=0>
// <medium_effect=0>
// <g_al=1>
// <fraction_ex=0.5>
// <tempe_limite=5.5>
// <decay_in_flight=1>
// <lab_frame=0>
// <bminimum=0>
// <bmaximum=1>
// <choix_ex=0>
// <fermi=1>
// <version=1000>
// <periph_trick=0>
// <b_limite=0.75>
// <freeze_out=3>
// <proba_free_nucleon=0.001>
// <tole_pourcent=0.01>
// <discrete=1>
// <minimisation=0>
// <geometry=0>
// <free_nucleon=0>
// <lambda_ex=1.3>
//
// If parameter lab_frame=1, we transform the events to the CM frame.
// In this way, for further analysis/filtering, ELIE events are always in the CM frame.
////////////////////////////////////////////////////////////////////////////////

void KVSimReader_ELIE_asym::define_output_filename()
{
   // ROOT file called: ELIE_[PROJ]_[TARG]_[EBEAM]AMeV_ASYM.root
   // Call after reading file header
   SetROOTFileName(Form("ELIE_%s_%s_%.1fAMeV_ASYM.root",
                        proj.GetSymbol(), targ.GetSymbol(), ebeam));
}

KVSimReader_ELIE_asym::KVSimReader_ELIE_asym()
   : KVSimReader_ELIE()
{
   // Default constructor
}

//____________________________________________________________________________//

KVSimReader_ELIE_asym::KVSimReader_ELIE_asym(KVString filename)
   : KVSimReader_ELIE()
{
   // Read file and convert to ROOT TTree
   ConvertEventsInFile(filename);
   SaveTree();
}

//____________________________________________________________________________//

KVSimReader_ELIE_asym::~KVSimReader_ELIE_asym()
{
   // Destructor
}

void KVSimReader_ELIE_asym::ConvertEventsInFile(KVString filename)
{
   if (!OpenFileToRead(filename)) return;
   if (!ReadHeader()) return;
   define_output_filename();
   tree_title.Form("ELIE secondary events %s + %s %.1f MeV/nuc.",
                   proj.GetSymbol(), targ.GetSymbol(), ebeam);
   Run();
   CloseFile();
}


Bool_t KVSimReader_ELIE_asym::ReadNucleus()
{
   //   numero_particule,z, a, teta, phi, energie, origine de la particule, energie d'ecitation finale
   //   0 27 57 3.27897003986 230.52425244 1109.37002505 0 0
   //   1 80 198 176.726338776 129.481056376 319.364098122 1 0
   //
   // The origin of secondary decay particles is stored in a parameter named "ORIGIN"
   // As particles in a KVEvent are numbered 1,2,... we add 1 to the value read in

   Int_t res = ReadLineAndCheck(8, " ");
   switch (res) {
      case 0:
         Info("ReadNucleus", "case 0 line est vide");
         return kFALSE;

      case 1:
         nuc->SetZ(GetIntReadPar(1));
         nuc->SetA(GetIntReadPar(2));
         nuc->SetExcitEnergy(GetDoubleReadPar(7));
         nuc->SetEnergy(GetDoubleReadPar(5));
         nuc->SetTheta(GetDoubleReadPar(3));
         nuc->SetPhi(GetDoubleReadPar(4));
         nuc->SetParameter("ORIGIN", GetIntReadPar(6) + 1);
         return kTRUE;
         break;

      default:
         return kFALSE;
   }

   return kFALSE;

}

//____________________________________________________________________________//

