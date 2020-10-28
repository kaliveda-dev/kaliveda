//Created by KVClassFactory on Wed Jan 11 10:29:05 2017
//Author: John Frankland,,,

#include "KVSimReader_ELIE_asym.h"

ClassImp(KVSimReader_ELIE_asym)



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

