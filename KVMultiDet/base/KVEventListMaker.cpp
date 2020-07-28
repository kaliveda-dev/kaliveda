/*
$Id: KVEventListMaker.cpp,v 1.3 2009/01/30 09:27:06 ebonnet Exp $
$Revision: 1.3 $
$Date: 2009/01/30 09:27:06 $
*/

//Created by KVClassFactory on Thu Mar 20 11:58:48 2008
//Author: Eric Bonnet

#include "KVEventListMaker.h"
#include "TFile.h"
#include "TTree.h"
#include "TEventList.h"
#include "KVList.h"
#include "KVString.h"

ClassImp(KVEventListMaker)

void KVEventListMaker::Process()
{

   if (!IsReady()) return;

//open file where tree is stored
   TFile file(GetFileName().Data(), "update");
   KVList kevtlist;

   TTree* tt = NULL;

//just count the number of branches
   KVString lname = GetBranchName();
   Int_t  nbre = 0;
   lname.Begin(" ");
   while (!lname.End()) {
      KVString stamp = lname.Next();
      nbre += 1;
   }
   if (nbre == 0) return;

   std::vector<Int_t> variable(nbre);
   KVString evtname;
   Bool_t ok = kFALSE;
//check if the tree are there
   if ((tt = (TTree*)file.Get(GetTreeName().Data()))) {
      Int_t nentries = tt->GetEntries();
      printf("nbre d entree %d\n", nentries);
      TBranch* bb = NULL;

      nbre = 0;
      lname.Begin(" ");
      //loop on branche names validity, if there is one wrong name
      //the program will exit
      while (!lname.End()) {
         KVString stamp = lname.Next();
         if ((bb = tt->GetBranch(stamp.Data()))) {
            tt->SetBranchAddress(stamp.Data(), &variable[nbre]);
            nbre += 1;
            ok = kTRUE;
         }
      }

      if (!ok) return;

      TEventList* el = NULL;
      //loop on tree entries
      for (Int_t kk = 0; kk < nentries; kk += 1) {
         if (kk % 10000 == 0) printf("%d evts lus\n", kk);

         tt->GetEntry(kk);
         evtname = "";
         nbre = 0;
         lname.Begin(" ");
         //compute the name of the TEventList
         //using branch name and branch value
         while (!lname.End()) {
            KVString stamp = lname.Next();
            if ((bb = tt->GetBranch(stamp.Data()))) {
               KVString val;
               val.Form("%s_%d_", stamp.Data(), variable[nbre]);
               evtname += val;
               nbre += 1;
            }
         }

         if (!(el = kevtlist.get_object<TEventList>(evtname.Data()))) {
            printf("creation de %s \n", evtname.Data());
            kevtlist.Add(new TEventList(evtname.Data()));
            el = (TEventList*)kevtlist.Last();
         }
         el->Enter(kk);
      }

      //write TEventList created
      if (ktag_tree) {
         for (Int_t nn = 0; nn < kevtlist.GetEntries(); nn += 1) {
            KVString tampname;
            tampname.Form("%s_%s", GetTreeName().Data(), kevtlist.At(nn)->GetName());
            ((TEventList*) kevtlist.At(nn))->SetName(tampname.Data());
            kevtlist.At(nn)->Write();
         }
      }
      else {
         kevtlist.Write();
      }
   }
   else printf("%s n existe pas\n", GetTreeName().Data());
}
