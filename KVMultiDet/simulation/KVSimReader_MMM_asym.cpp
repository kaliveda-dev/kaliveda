//Created by KVClassFactory on Tue Jul  6 17:30:28 2010
//Author: Eric Bonnet

#include "KVSimReader_MMM_asym.h"

ClassImp(KVSimReader_MMM_asym)


KVSimReader_MMM_asym::KVSimReader_MMM_asym()
{
   // Default constructor
   init();
}

KVSimReader_MMM_asym::KVSimReader_MMM_asym(KVString filename)
{
   init();
   ConvertEventsInFile(filename);
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
      // add particle twice to the event
      // the first one (labelled "Prov=QP") with the particle's momentum
      // in the QP Frame
      // the second one (labelled "Prov=QC") with the particle's momentum
      // in the QC Frame
      nuc->ChangeFrame(KVFrameTransform(fBoostQP, kTRUE));
      nuc->GetParameters()->SetValue("Prov", "QP");

      nuc = (KVSimNucleus*)evt->AddParticle();
      nuc->SetZandA(zz, aa);
      nuc->SetMomentum(px, py, pz);
      nuc->ChangeFrame(KVFrameTransform(fBoostQC, kTRUE));
      nuc->GetParameters()->SetValue("Prov", "QC");
   }

   return kTRUE;

}

