//Created by KVClassFactory on Sun Jan 20 13:31:10 2013
//Author: John Frankland,,,

#include "KVSimReader_ELIE.h"

ClassImp(KVSimReader_ELIE)



void KVSimReader_ELIE::define_output_filename()
{
   // ROOT file called: ELIE_[PROJ]_[TARG]_[EBEAM]AMeV_PRIM.root
   // Call after reading file header
   SetROOTFileName(Form("ELIE_%s_%s_%.1fAMeV_PRIM.root",
                        proj.GetSymbol(), targ.GetSymbol(), ebeam));
}

void KVSimReader_ELIE::transform_to_cm()
{
   // transform all particle kinematics to CM frame from lab

   KVNucleus compound = proj + targ;
   evt->ChangeFrame(compound.GetV(), "CM");
}

KVSimReader_ELIE::KVSimReader_ELIE()
{
   // Default constructor
   init();
}

//________________________________________________________________

KVSimReader_ELIE::KVSimReader_ELIE(KVString filename) : KVSimReader()
{
   // Read file
   init();
   ConvertEventsInFile(filename);
}

KVSimReader_ELIE::~KVSimReader_ELIE()
{
   // Destructor
}

void KVSimReader_ELIE::ConvertEventsInFile(KVString filename)
{
   if (!OpenFileToRead(filename)) return;
   if (!ReadHeader()) return;
   define_output_filename();
   tree_title.Form("ELIE primary events %s + %s %.1f MeV/nuc.",
                   proj.GetSymbol(), targ.GetSymbol(), ebeam);
   Run();
   CloseFile();
}


void KVSimReader_ELIE::ReadFile()
{
   Info("ReadFile", "begins");
   while (IsOK()) {
      while (ReadEvent()) {
         if (nevt && nevt % 1000 == 0) Info("ReadFile", "%d evts lus", nevt);
         if (HasToFill()) FillTree();
      }
   }
}

Bool_t KVSimReader_ELIE::ReadHeader()
{
   // File header contains info on simulation
   //
   // 54 129 50 119 32
   // 20000
   //   ievt=50
   //   projectile=Ni
   //   target=Au
   //   a_proj=58
   //   z_proj=28
   //   a_targ=197
   //   z_targ=79
   //   [...]
   //   geometry=0
   //   free_nucleon=0
   //   lambda_ex=1.3
   //
   // The simulation parameters are stored in a KVNameValueList

   Int_t res = ReadLineAndCheck(5, " ");
   switch (res) {
      case 0:
         Info("ReadHeader", "Can't read system");
         return kFALSE;
      case 1:
         AddInfo("Aproj", GetReadPar(1));
         AddInfo("Zproj", GetReadPar(0));
         AddInfo("Atarg", GetReadPar(3));
         AddInfo("Ztarg", GetReadPar(2));
         AddInfo("Ebeam", GetReadPar(4));
         proj.SetZAandE(GetIntReadPar(0), GetIntReadPar(1), GetIntReadPar(1)*GetDoubleReadPar(4));
         targ.SetZandA(GetIntReadPar(2), GetIntReadPar(3));
         ebeam = GetDoubleReadPar(4);
         break;
      default:
         return kFALSE;
   }

   res = ReadLineAndCheck(1, " ");
   switch (res) {
      case 0:
         Info("ReadHeader", "Can't read events");
         return kFALSE;
      case 1:
         AddInfo("Nevents", GetReadPar(0));
         break;

      default:
         return kFALSE;
   }
   elie_params->Clear();
   elie_params->SetName("ELIE Parameters");
   res = ReadLineAndCheck(2, "=");
   while (res < 2) {
      if (res > 0) {
         if (GetReadPar(1).IsDigit()) elie_params->SetValue(GetReadPar(0), GetIntReadPar(1));
         else if (GetReadPar(1).IsFloat()) elie_params->SetValue(GetReadPar(0), GetDoubleReadPar(1));
         else elie_params->SetValue(GetReadPar(0), GetReadPar(1));
      }
      res = ReadLineAndCheck(2, "=");
   }
   if (elie_params->GetEntries()) AddObject(elie_params);

   return kTRUE;
}

Bool_t KVSimReader_ELIE::ReadEvent()
{
   // Event structure: (primary events) -> OLD WAY, see bellow for recent ELIE calculations
   //   nevt multiplicite b reduit
   //   0 2 0.998654688551
   //   particules dans l'evt
   //   indice_particule  charge_particule masse_particule teta_particule(degres) phi_particule(degres) indice_particule energie d'excitation totale (MeV)
   //   0 27 57 3.27897003986 230.52425244 1109.37002505 0 0.00499304975261
   //   1 80 198 176.726338776 129.481056376 319.364098122 1 0.017344278088

   //   nevt multiplicite b reduit
   //1 11 0.665404278608
   //   indice_particule  charge_particule masse_particule teta_particule(degres) phi_particule(degres) E(MeV) E*(MeV) L(hbar)
   //0 21 45 2.49354294869 78.2453656442 2180.55536792 91.0936851491 1.0
   //1 20 46 5.25547324063 79.8035650615 7.5872385507 93.1179892635 1.0


   evt->Clear();

   // first line of first event will have been read already by ReadHeader,
   // therefore we do not read a new line in this case (when nevt=0)
   Int_t res = (nevt ? ReadLineAndCheck(3, " ") : ReuseLineAndCheck(3, " "));
   Int_t mult = 0;
   switch (res) {
      case 0:
         return kFALSE;
      case 1:
         evt->SetNumber(GetIntReadPar(0));
         mult = GetIntReadPar(1);
         evt->GetParameters()->SetValue("bred", GetDoubleReadPar(2));
         evt->GetParameters()->SetValue("mult", mult);
         break;
      default:
         return kFALSE;
   }
   for (Int_t mm = 0; mm < mult; mm++) {
      nuc = (KVSimNucleus*)evt->AddParticle();
      if (!ReadNucleus()) return kFALSE;
   }
   nevt++;

   // if in lab, transform to CM frame
   if (elie_params->GetIntValue("lab_frame") > 0) transform_to_cm();
   else evt->SetFrameName("CM");

   return kTRUE;
}

Bool_t KVSimReader_ELIE::ReadNucleus()
{
   // Event structure: (primary events) -> OLD WAY, see bellow for recent ELIE calculations
   //   nevt multiplicite b reduit
   //   0 2 0.998654688551
   //   particules dans l'evt
   //   indice_particule  charge_particule masse_particule teta_particule(degres) phi_particule(degres) indice_particule energie d'excitation totale (MeV)
   //   0 27 57 3.27897003986 230.52425244 1109.37002505 0 0.00499304975261
   //   1 80 198 176.726338776 129.481056376 319.364098122 1 0.017344278088

   //   nevt multiplicite b reduit
   //1 11 0.665404278608
   //   indice_particule  charge_particule masse_particule teta_particule(degres) phi_particule(degres) E(MeV) E*(MeV) L(hbar)
   //0 21 45 2.49354294869 78.2453656442 2180.55536792 91.0936851491 1.0
   //1 20 46 5.25547324063 79.8035650615 7.5872385507 93.1179892635 1.0



   Int_t res = ReadLineAndCheck(8, " ");
   switch (res) {
      case 0:
         Info("ReadNucleus", "case 0 line est vide");
         return kFALSE;

      case 1:
         nuc->SetZ(GetIntReadPar(1));
         nuc->SetA(GetIntReadPar(2));
         if (GetDoubleReadPar(6) >= 0.) nuc->SetExcitEnergy(GetDoubleReadPar(6));
         nuc->SetEnergy(GetDoubleReadPar(5));
         nuc->SetTheta(GetDoubleReadPar(3));
         nuc->SetPhi(GetDoubleReadPar(4));
         if (nuc->GetExcitEnergy() > 0.) nuc->SetAngMom(GetDoubleReadPar(7), 0, 0);

         return kTRUE;
         break;

      default:
         return kFALSE;
   }

   return kFALSE;
}
