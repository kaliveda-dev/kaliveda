//Created by KVClassFactory on Tue Jul  6 17:30:28 2010
//Author: Eric Bonnet

#include "KVSimReader_MMM_asym.h"

ClassImp(KVSimReader_MMM_asym)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVSimReader_MMM_asym</h2>
<h4>Read ascii file for asymptotic events of the MMM code after deexcitation</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVSimReader_MMM_asym::KVSimReader_MMM_asym()
{
   // Default constructor
   init();
}

KVSimReader_MMM_asym::KVSimReader_MMM_asym(KVString filename)
{
   init();
   if (!OpenFileToRead(filename)) return;
   Run();
   CloseFile();
}


KVSimReader_MMM_asym::~KVSimReader_MMM_asym()
{
   // Destructor
}


void KVSimReader_MMM_asym::ReadFile()
{

   while (IsOK()) {
      while (ReadEvent()) {
         if (nevt % 1000 == 0) Info("ReadFile", "%d evts lus", nevt);
         if (HasToFill()) FillTree();
      }
   }

}


Bool_t KVSimReader_MMM_asym::ReadEvent()
{

   evt->Clear();
   Int_t mult = 0, natt = 0;

   ReadLine(" ");

   Int_t res = GetNparRead();
   switch (res) {
      case 0:
         return kFALSE;
      default:
         //nlus = toks->GetEntries();
         idx = 0;
         mult = GetIntReadPar(idx++);
         natt = 5 * mult + 1;
         if (natt != res) {
            Info("ReadEvent", "Nombre de parametres (%d) different de celui attendu (%d)", res, natt);
            return kFALSE;
         }
         for (Int_t mm = 0; mm < mult; mm += 1) {
            nuc = (KVSimNucleus*)evt->AddParticle();
            ReadNucleus();
         }

         evt->SetNumber(nevt);
         nevt += 1;
         return kTRUE;
   }

}

Bool_t KVSimReader_MMM_asym::ReadNucleus()
{

   Int_t aa = GetIntReadPar(idx++);
   nuc->SetZ(GetIntReadPar(idx++));
   nuc->SetA(aa);
   nuc->SetPx(GetDoubleReadPar(idx++));
   nuc->SetPy(GetDoubleReadPar(idx++));
   nuc->SetPz(GetDoubleReadPar(idx++));

   return kTRUE;

}

