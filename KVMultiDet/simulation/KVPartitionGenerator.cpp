//Created by KVClassFactory on Thu Jun 17 14:20:16 2010
//Author: bonnet

#include "KVPartitionGenerator.h"
#include "TMath.h"
#include "KVCouple.h"
#include "TFile.h"
#include "TTree.h"
#include "TChain.h"
#include "TDatime.h"
#include "TNamed.h"


ClassImp(KVPartitionGenerator)


//_______________________________________________________
KVPartitionGenerator::KVPartitionGenerator()
{
   // Default constructor
   init();

}

//_______________________________________________________
void KVPartitionGenerator::init()
{
   //protected method
   //Initisalisation des variables
   tabz = 0;
   ztot = mtot = 0;

   kcurrent = 0;
   kzt = kmt = 0;
   mshift = zshift = 0;

   to_be_checked = kFALSE;

   npar = npar_zt_mt = 0;
   tree = 0;
   tname = "PartitionTree";
   cname = "";

   kwriting_path = "";
   flist = 0;

}

//_______________________________________________________
KVPartitionGenerator::~KVPartitionGenerator()
{
   // Destructor
   if (kcurrent) {
      delete [] kcurrent;
      kcurrent = 0;
   }
   if (tabz) {
      delete [] tabz;
      tabz = 0;
   }

}

//_______________________________________________________
void KVPartitionGenerator::SetPathForFile(KVString path)
{
   // Defini le chemin ou les arbres seront ecris
   //prevoir des zones pouvant recevoir de gros volumes de données
   //si les calculs de partitions se fait a partir de grosse taille initiale
   kwriting_path = path;

}

//_______________________________________________________
void KVPartitionGenerator::MakePartitions(Int_t Ztot, Int_t Mtot, Int_t Zinf)
{
   //protected method
   //Cree un fichier avec nom formate :
   //    From_[ClassName]_Zt[Ztot]_Mt[Mtot]_Zm[Zinf].root
   //
   //Defini l'arbre ou seront enregistrees les partitions
   //
   //Determine toute les partitions pour un
   //couple donné de parametres : (Ztot, Mtot, Zinf)
   //avec :
   // -  Ztot -> taille totale de la partition
   // -  Mtot -> multiplicité de partition
   // -  Zinf -> taille minimale pour les fragments de la partition
   //--------------------------------
   //Enregistre l arbre et ferme le fichier
   //
   //Routine centrale de la classe appelée par toutes les methodes de type Break_Using...Criterion()
   //

   SetConditions(Ztot, Mtot, Zinf);

   PreparTree();

   Process();

   WriteTreeAndCloseFile();

}

//_______________________________________________________
void KVPartitionGenerator::SetConditions(Int_t Ztot, Int_t Mtot, Int_t Zinf)
{
   //protected method
   //Repercute les parametres passes via la methode SetConditions
   //Prepare les tableaux necessaires
   //
   kzt = Ztot;
   kmt = Mtot;
   kzinf = Zinf;

   if (kcurrent)
      delete kcurrent;
   kcurrent = new Int_t[kmt];
   for (Int_t mm = 0; mm < kmt; mm += 1)
      kcurrent[mm] = 0;

   npar_zt_mt = 0;

}

//_______________________________________________________
void KVPartitionGenerator::PreparTree()
{
   //protected method
   //Creation du fichier formate suivant les parametres passes via la methode SetConditions
   //Creation de l'arbre
   mtot = kmt + mshift;
   ztot = kzt + zshift;

   KVString snom;
   snom.Form("%sFrom_%s_Zt%d_Mt%d_Zi%d.root", kwriting_path.Data(), this->Class_Name(), ztot, mtot, kzinf);
   TFile* file = new TFile(snom.Data(), "recreate");
   flist->Add(new TNamed(file->GetName(), ""));
   tree = new TTree(tname.Data(), this->Class_Name());

   if (tabz) delete [] tabz;

   tabz = new Int_t[mtot];

   tree->Branch("ztot", &ztot,   "ztot/I");
   tree->Branch("mtot", &mtot,   "mtot/I");
   tree->Branch("tabz", tabz,    "tabz[mtot]/I");

   if (mshift) tabz[0] = zshift;

}

//_______________________________________________________
void KVPartitionGenerator::Process(void)
{
   //protected method
   //Determine toute les partitions pour un
   //couple donné de parametres : (Ztot, Mtot, Zinf)
   //avec :
   // -  Ztot -> taille totale de la partition
   // -  Mtot -> multiplicité de partition
   // -  Zinf -> taille minimale pour les fragments de la partition
   //

   if (kmt == 1) {
      tabz[0 + mshift] = kzt;
      TreatePartition();
      return;
   }

   Int_t zutilise = (kmt * kzinf);
   Int_t zdispo = kzt - zutilise;

   Int_t nb_cassure = kmt - 1;
   KVCouple* coup[nb_cassure];
   Int_t ncouple[nb_cassure];
   Int_t niter[nb_cassure];
   for (Int_t nc = 0; nc < nb_cassure; nc += 1) {
      ncouple[nc] = 0;
      coup[nc] = 0;
      niter[nc] = 0;
   }

   Int_t nc;
   Int_t zsup = zdispo;
   for (nc = 0; nc < nb_cassure; nc += 1) {

      if (!coup[nc]) {
         coup[nc] = new KVCouple(zdispo, zsup, kmt - nc);

         ncouple[nc] = coup[nc]->GetNbreCouples();
         niter[nc] = 0;
      }
      if (niter[nc] < ncouple[nc]) {

         zdispo = coup[nc]->GetZ2(niter[nc]);
         zsup = coup[nc]->GetZ1(niter[nc]);
         kcurrent[nc] = coup[nc]->GetZ1(niter[nc]);
         kcurrent[nc + 1] = coup[nc]->GetZ2(niter[nc]);

      }
   }

   Bool_t finish = kFALSE;
   while (!finish) {

      nc = nb_cassure - 1;
      while (niter[nc] < ncouple[nc]) {

         kcurrent[nc] = coup[nc]->GetZ1(niter[nc]);
         kcurrent[nc + 1] = coup[nc]->GetZ2(niter[nc]);

         for (Int_t ii = 0; ii < kmt; ii += 1)
            tabz[ii + mshift] = kcurrent[ii] + kzinf;

         if (to_be_checked) {
            if (tabz[0] >= tabz[mshift]) {
               TreatePartition();
            }
         }
         else {
            TreatePartition();
         }

         niter[nc] += 1;
      }
      delete coup[nc];
      coup[nc] = 0;

      Int_t previous = nc - 1;
      while (niter[previous] == (ncouple[previous] - 1) && previous >= 0) {

         delete coup[previous];
         coup[previous] = 0;
         previous -= 1;

         if (previous < 0) break;
      }

      if (previous < 0) {
         finish = kTRUE;
      }
      else {
         niter[previous] += 1;
         zdispo = coup[previous]->GetZ2(niter[previous]);
         zsup = coup[previous]->GetZ1(niter[previous]);

         kcurrent[previous] = coup[previous]->GetZ1(niter[previous]);

         for (Int_t ncbis = previous + 1; ncbis < nb_cassure; ncbis += 1) {
            if (!coup[ncbis]) {
               coup[ncbis] = new KVCouple(zdispo, zsup, kmt - ncbis);

               ncouple[ncbis] = coup[ncbis]->GetNbreCouples();
               niter[ncbis] = 0;
            }
            else { }
            if (niter[ncbis] < ncouple[ncbis]) {

               zdispo = coup[ncbis]->GetZ2(niter[ncbis]);
               zsup = coup[ncbis]->GetZ1(niter[ncbis]);
               kcurrent[ncbis] = coup[ncbis]->GetZ1(niter[ncbis]);
               kcurrent[ncbis + 1] = coup[ncbis]->GetZ2(niter[ncbis]);
            }
         }
      }
   }

   Info("Process", "ztot:%d en mtot:%d avec zinf:%d donne %lf combinaisons (total %lf)", kzt, kmt, kzinf, npar_zt_mt, npar);


}


//_______________________________________________________
void KVPartitionGenerator::BeforeBreak()
{
   //protected method
   //Creation de la liste pour creation de la TChain
   //stop du chrono
   //
   flist = new KVUniqueNameList();
   flist->SetOwner(kTRUE);
   Start();

}

//_______________________________________________________
void KVPartitionGenerator::AfterBreak()
{
   //protected method
   //Permet de creer une TChain recapitulant tout les arbres
   //crees et suvegardes lors de la determination de toutes les
   //partitions par une des methodes BreakUsing_[Critere]_Criterion()
   //stop du chrono
   //

   Stop();
   TFile* file = new TFile(cname.Data(), "recreate");
   TChain* ch = new TChain(tname.Data());
   TIter nf(flist);
   TNamed* tn = 0;
   while ((tn = (TNamed*)nf.Next()))
      ch->AddFile(tn->GetName());
   ch->Write();
   file->Close();
   delete flist;

}

//_______________________________________________________
void KVPartitionGenerator::BreakUsing_Ztot_Zinf_Criterion(Int_t Ztot, Int_t Zinf, KVString chain_name, Int_t min, Int_t max)
{
   //Determine toutes les partitions pour :
   // -  une taille totale de la partition donnée (Ztot)
   // -  une taille minimale pour les fragments de la partition (Zinf)
   // - le nom du fichier ou la TChain recapitulant tous les arbres crees va etre enregistree
   // (par defaut, si chain_name=="", le nom de fichier est formate en fonction des arguments
   //Les arguments min et max permettent de restreindre le calcul a une plage en multiplicité
   // si min==-1 (defaut) -> mfmin=1
   // si max==-1 (defaut) -> mfmax=Ztot/Zinf
   cname = chain_name;
   if (cname == "") cname.Form("Partitions_Zf%d_Zi%d.root", Ztot, Zinf);
   BeforeBreak();

   Int_t mtmin = 1;
   Int_t mtmax = Ztot / Zinf;

   if (min != -1 && min <= mtmax)
      mtmin = min;

   if (max != -1 && max <= mtmax && max >= mtmin)
      mtmax = max;

   for (Int_t Mtot = mtmin; Mtot <= mtmax; Mtot += 1) {
      MakePartitions(Ztot, Mtot, Zinf);
   }

   AfterBreak();
   Info("BreakUsing_Ztot_Zinf_Criterion", "%lf partitions crees en %d seconds", npar, GetDeltaTime());

}

//_______________________________________________________
void KVPartitionGenerator::BreakUsing_Ztot_Mtot_Zinf_Criterion(Int_t Ztot, Int_t Mtot, Int_t Zinf, KVString chain_name)
{
   //Determine toutes les partitions pour :
   // -  une taille totale de la partition donnée (Ztot)
   // -  une multiplicité de partition donnée (Mtot)
   // -  une taille minimale pour les fragments de la partition (Zinf)
   // - le nom du fichier ou la TChain recapitulant tous les arbres crees va etre enregistree
   // (par defaut, si chain_name=="", le nom de fichier est formate en fonction des arguments
   cname = chain_name;
   if (cname == "") cname.Form("Partitions_Zt%d_Mt%d_Zi%d.root", Ztot, Mtot, Zinf);
   BeforeBreak();

   MakePartitions(Ztot, Mtot, Zinf);

   AfterBreak();
   Info("BreakUsing_Ztot_Mtot_Zinf_Criterion", "%lf partitions crees en %d seconds", npar, GetDeltaTime());

}

//_______________________________________________________
void KVPartitionGenerator::BreakUsing_Ztot_Zmax_Zinf_Criterion(Int_t Ztot, Int_t Zmax, Int_t Zinf, KVString chain_name)
{
   //Determine toutes les partitions pour :
   // -  une taille totale de la partition donnée (Ztot)
   // -  une taille donnée du plus gros fragment de la partition (Zmax)
   // -  une taille minimale pour les fragments de la partition (Zinf)
   // - le nom du fichier ou la TChain recapitulant tous les arbres crees va etre enregistree
   // (par defaut, si chain_name=="", le nom de fichier est formate en fonction des arguments
   cname = chain_name;
   if (cname == "") cname.Form("Partitions_Zt%d_Zm%d_Zi%d.root", Ztot, Zmax, Zinf);
   BeforeBreak();

   if (Ztot == Zmax) {
      MakePartitions(Ztot, 1, Zinf);
   }
   else if (Ztot - Zmax >= Zinf) {

      Int_t mfmin = 2;
      Int_t np = 0;
      KVCouple* cp = new KVCouple(Ztot, Zmax, mfmin);
      np = cp->GetNbreCouples();
      while (np <= 0) {
         if (cp) delete cp;
         cp = 0;
         mfmin += 1;
         cp = new KVCouple(Ztot, Zmax, mfmin);
         np = cp->GetNbreCouples();
      }
      printf("a priori mfmin=%d OK\n", mfmin);
      for (Int_t ii = 0; ii < cp->GetNbreCouples(); ii += 1) {
         printf("%d %d\n", cp->GetZ1(ii), cp->GetZ2(ii));
      }
      delete cp;
      cp = 0;


      Int_t mfmax = ((Ztot - Zmax) / Zinf) + 1;
      np = 0;
      cp = new KVCouple(Ztot, Zmax, mfmax);
      np = cp->GetNbreCouples();
      while (np <= 0) {
         if (cp) delete cp;
         cp = 0;
         mfmax -= 1;
         cp = new KVCouple(Ztot, Zmax, mfmax);
         np = cp->GetNbreCouples();
      }
      printf("a priori mfmax=%d OK\n", mfmax);
      for (Int_t ii = 0; ii < cp->GetNbreCouples(); ii += 1) {
         printf("%d %d\n", cp->GetZ1(ii), cp->GetZ2(ii));
      }
      delete cp;
      cp = 0;


      for (Int_t Mtot = mfmin; Mtot <= mfmax; Mtot += 1) {
         mshift = 1;
         zshift = Zmax;
         to_be_checked = kTRUE;
         MakePartitions(Ztot - zshift, Mtot - mshift, Zinf);
      }

   }
   else {
      printf("%d %d ??? \n", Ztot - Zmax, Zinf);
   }

   AfterBreak();
   Info("BreakUsing_Ztot_Zmax_Zinf_Criterion", "%lf partitions crees en %d seconds", npar, GetDeltaTime());
}

//_______________________________________________________
void KVPartitionGenerator::BreakUsing_Mtot_Zmax_Zinf_Criterion(Int_t Mtot, Int_t Zmax, Int_t Zinf, KVString chain_name)
{
   //Determine toutes les partitions pour :
   // -  une multiplicité de partition donnée (Mtot)
   // -  une taille donnée du plus gros fragment de la partition (Zmax)
   // -  une taille minimale pour les fragments de la partition (Zinf)
   // - le nom du fichier ou la TChain recapitulant tous les arbres crees va etre enregistree
   // (par defaut, si chain_name=="", le nom de fichier est formate en fonction des arguments
   cname = chain_name;
   if (cname == "") cname.Form("Partitions_Mt%d_Zm%d_Zi%d.root", Mtot, Zmax, Zinf);
   BeforeBreak();

   Int_t ztmin = Zmax + (Mtot - 1) * Zinf;
   Int_t ztmax = Zmax * Mtot;

   for (Int_t Ztot = ztmin; Ztot <= ztmax; Ztot += 1) {
      mshift = 1;
      zshift = Zmax;
      to_be_checked = kTRUE;
      MakePartitions(Ztot - zshift, Mtot - mshift, Zinf);
   }

   AfterBreak();
   Info("BreakUsing_Mtot_Zmax_Zinf_Criterion", "%lf partitions crees en %d seconds", npar, GetDeltaTime());
}

//_______________________________________________________
void KVPartitionGenerator::TreatePartition()
{
   //Rempli l arbre avec la partition courante
   //Incremente les compteurs sur le nombre de particules creees
   tree->Fill();

   npar_zt_mt += 1;
   npar += 1;

}

//_______________________________________________________
void KVPartitionGenerator::WriteTreeAndCloseFile()
{
   //Test le fichier courant (pointeur gFile)
   //Si tout va bien
   //Ecriture de l arbre dans le fichier (tout deux definis dans PreparTree)
   //et ajout du nom de fichier
   //dans une liste permettant de generer la TChain en fin de processus
   //
   if (gFile && gFile->IsOpen()) {

      if (tree && gFile->IsWritable()) {

         Info("WriteTreeAndCloseFile", "Ecriture du fichier:\n%s avec l'arbre:\n %s (%lld entrees)", gFile->GetName(), tree->GetName(), tree->GetEntries());
         tree->ResetBranchAddresses();
         flist->Add(new TNamed(gFile->GetName(), ""));
         gFile->Write();
      }
      gFile->Close();
   }

}

//_______________________________________________________
void KVPartitionGenerator::Start()
{
   //protected method
   //Signal start
   TDatime time;
   tstart = time.GetHour() * 3600 + time.GetMinute() * 60 + time.GetSecond();

}

//_______________________________________________________
void KVPartitionGenerator::Stop()
{
   //protected method
   //Signal stop
   TDatime time;
   tstop = time.GetHour() * 3600 + time.GetMinute() * 60 + time.GetSecond();
   tellapsed = tstop - tstart;

}

//_______________________________________________________
Int_t KVPartitionGenerator::GetDeltaTime()
{
   //protected method
   //Retoune le temps ecoules (en seconde)
   //entre un appel Start() et un appel Stop()
   return tellapsed;

}
