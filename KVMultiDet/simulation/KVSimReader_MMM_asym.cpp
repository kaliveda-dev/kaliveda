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


void KVSimReader_MMM_asym::SetBoost(TVector3& vQP, TVector3& vQC)
{
   fApplyBoost = kTRUE;
   fBoostQP = vQP;
   fBoostQC = vQC;
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
   Int_t zz = GetIntReadPar(idx++);

   nuc->SetZ(zz);
   nuc->SetA(aa);

   Double_t px = GetDoubleReadPar(idx++);
   Double_t py = GetDoubleReadPar(idx++);
   Double_t pz = GetDoubleReadPar(idx++);

   nuc->SetMomentum(px, py, pz);

   if (fApplyBoost) {
      nuc->SetFrame("QP", fBoostQP, kTRUE);
      TVector3 momQP = nuc->GetFrame("QP")->GetMomentum();

      nuc->SetFrame("QC", fBoostQC, kTRUE);
      TVector3 momQC = nuc->GetFrame("QC")->GetMomentum();

      nuc->Clear();
      nuc->GetParameters()->SetValue("Prov", "QP");
      nuc->SetZandA(zz, aa);
      nuc->SetMomentum(momQP.X(), momQP.Y(), momQP.Z());

      nuc = (KVSimNucleus*)evt->AddParticle();
      nuc->GetParameters()->SetValue("Prov", "QC");
      nuc->SetZandA(zz, aa);
      nuc->SetMomentum(momQC.X(), momQC.Y(), momQC.Z());
   }

   return kTRUE;

}

