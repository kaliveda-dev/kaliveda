//Created by KVClassFactory on Tue Jul  6 16:48:05 2010
//Author: Eric Bonnet

#include "KVSimReader_MMM.h"

ClassImp(KVSimReader_MMM)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVSimReader_MMM</h2>
<h4>Read ascii file for events of the MMM code at Freeze Out</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVSimReader_MMM::KVSimReader_MMM()
{
   // Default constructor
   init();
}

KVSimReader_MMM::KVSimReader_MMM(KVString filename)
{
   init();
   if (!OpenFileToRead(filename)) return;
   Run();
   CloseFile();
}

KVSimReader_MMM::~KVSimReader_MMM()
{
   // Destructor
}

void KVSimReader_MMM::ReadFile()
{


   while (IsOK()) {
      while (ReadEvent()) {
         if (nevt % 1000 == 0) Info("ReadFile", "%d evts lus", nevt);
         if (HasToFill()) FillTree();
      }
   }

}


Bool_t KVSimReader_MMM::ReadEvent()
{

   evt->Clear();
   Int_t mult = 0, natt = 0;

   ReadLine(" ");

   Int_t res = GetNparRead();
   switch (res) {
      case 0:
         Info("ReadEvent", "case 0 line est vide");
         return kFALSE;
      default:
         idx = 0;
         mult = GetIntReadPar(idx++);
         natt = 6 * mult + 4 + 1;
         if (natt != res) {
            Info("ReadEvent", "Nombre de parametres (%d) different de celui attendu (%d)", res, natt);
            return kFALSE;
         }
         evt->GetParameters()->SetValue("Eth", GetDoubleReadPar(idx++));
         evt->GetParameters()->SetValue("Eclb", GetDoubleReadPar(idx++));
         evt->GetParameters()->SetValue("Eexci", GetDoubleReadPar(idx++));
         evt->GetParameters()->SetValue("Ebind", GetDoubleReadPar(idx++));

         for (Int_t mm = 0; mm < mult; mm += 1) {
            nuc = (KVSimNucleus*)evt->AddParticle();
            ReadNucleus();
         }

         evt->SetNumber(nevt);
         nevt += 1;
         return kTRUE;
   }

}

Bool_t KVSimReader_MMM::ReadNucleus()
{

   Int_t aa = GetIntReadPar(idx++);
   nuc->SetZ(GetIntReadPar(idx++));
   nuc->SetA(aa);
   nuc->SetExcitEnergy(GetDoubleReadPar(idx++));
   nuc->SetPx(GetDoubleReadPar(idx++));
   nuc->SetPy(GetDoubleReadPar(idx++));
   nuc->SetPz(GetDoubleReadPar(idx++));

   return kTRUE;

}
