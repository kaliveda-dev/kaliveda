//Created by KVClassFactory on Mon Jul  5 16:21:41 2010
//Author: bonnet

#include "KVSimReader_SMF.h"
#include "KVString.h"

ClassImp(KVSimReader_SMF)


//________________________________________________________________
KVSimReader_SMF::KVSimReader_SMF()
{
   // Default constructor
   init();
}

//________________________________________________________________
KVSimReader_SMF::KVSimReader_SMF(KVString filename)
{
   init();
   ConvertEventsInFile(filename);
}

//________________________________________________________________
KVSimReader_SMF::~KVSimReader_SMF()
{
   // Destructor
}


//________________________________________________________________
void KVSimReader_SMF::ReadFile()
{

   while (IsOK()) {
      if (ReadHeader() && ReadEvent()) {
         if (nevt % 1000 == 0) Info("ReadFile", "%d evts lus", nevt);
         if (HasToFill()) FillTree();
      }
   }

}

//________________________________________________________________
Bool_t KVSimReader_SMF::ReadHeader()
{

   KVString snom;
   Int_t res = ReadLineAndCheck(1, " ");
   switch (res) {
      case 0:
         return kFALSE;
      case 1:
         snom.Form("%s", GetReadPar(0).Data());
         snom.ReplaceAll("evt_", "");
         //Info("ReadHeader","lecture %d",snom.Atoi());
         nv->SetValue("event_number", snom.Atoi());

         return kTRUE;
      default:

         return kFALSE;
   }

}

//________________________________________________________________
Bool_t KVSimReader_SMF::ReadEvent()
{

   evt->Clear();

   Int_t mult = 0;
   Int_t res = ReadLineAndCheck(1, " ");
   switch (res) {
      case 0:
         return kFALSE;
      case 1:
         mult = GetIntReadPar(0);
         break;

      default:
         return kFALSE;
   }

   evt->SetNumber(nv->GetIntValue("event_number"));
   //Info("ReadEvent","       Lecture evt %d -> mult %d",evt->GetNumber(),mult);
   for (Int_t mm = 0; mm < mult; mm += 1) {
      nuc = (KVSimNucleus*)evt->AddParticle();
      if (!ReadNucleus()) return kFALSE;
   }

   nevt += 1;

   return kTRUE;

}

//________________________________________________________________
Bool_t KVSimReader_SMF::ReadNucleus()
{

   ReadLine(" ");
   Int_t res = GetNparRead();
   Int_t npar = 0;
   switch (res) {
      case 0:
         Info("ReadNucleus", "case 0 line est vide");
         return kFALSE;

      default:

         ReadLineAndAdd(" ");
         npar = GetNparRead();
         if (npar != 12) {
            Info("ReadNucleus", "Nombre de parametres (%d) different de celui attendu(%d)", npar, 12);
            return kFALSE;
         }
         nuc->SetZ(TMath::Nint(GetDoubleReadPar(1)));
         nuc->SetA(TMath::Nint(GetDoubleReadPar(0)));
         nuc->GetParameters()->SetValue("density", GetDoubleReadPar(2));
         nuc->SetExcitEnergy(GetDoubleReadPar(3));
         nuc->GetParameters()->SetValue("spin", GetDoubleReadPar(4));
         //Axe "faisceau dans SMF z"
         nuc->SetMomentum(GetDoubleReadPar(5), GetDoubleReadPar(6), GetDoubleReadPar(7));
         nuc->SetPosition(GetDoubleReadPar(8), GetDoubleReadPar(9), GetDoubleReadPar(10));
         nuc->GetParameters()->SetValue("avoir", GetDoubleReadPar(11));

         return kTRUE;
   }

}
