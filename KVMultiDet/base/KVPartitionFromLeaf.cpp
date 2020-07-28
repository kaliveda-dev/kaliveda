//Created by KVClassFactory on Fri May  7 11:30:10 2010
//Author: bonnet

#include "KVPartitionFromLeaf.h"
#include "TMath.h"

ClassImp(KVPartitionFromLeaf)



KVPartitionFromLeaf::KVPartitionFromLeaf()
{
   // Default constructor
   linked_leaf = 0;
   zmin = -1;
   zmax = -1;
   select_min = kFALSE;
   select_max = kFALSE;

}

KVPartitionFromLeaf::~KVPartitionFromLeaf()
{
   // Destructor
}

void KVPartitionFromLeaf::ConnectToLeaf(TLeaf* lf)
{

   linked_leaf = lf;
   Int_t cmax;
   (lf->GetLeafCount() ? cmax = lf->GetLeafCount()->GetMaximum() : cmax = lf->GetNdata());

}

void KVPartitionFromLeaf::ReadEntry(Long64_t entry)
{

   Clear();
   if (entry != -1) linked_leaf->GetBranch()->GetEntry(entry);

   Int_t mult;
   (linked_leaf->GetLeafCount() ? mult = linked_leaf->GetLeafCount()->GetValue(0) : mult = linked_leaf->GetNdata());

   for (Int_t mm = 0; mm < mult; mm += 1) {
      Int_t val = TMath::Nint(linked_leaf->GetValue(mm));
      if ((!select_min || (select_min && val >= zmin)) && (!select_max || (select_max && val <= zmax))) {
         Add(val);
      }
   }
   CheckForUpdate();

}

void KVPartitionFromLeaf::SetZmin(Int_t val)
{

   zmin = val;
   select_min = ((zmin == -1) ? kFALSE : kTRUE);

}
void KVPartitionFromLeaf::SetZmax(Int_t val)
{

   zmax = val;
   select_max = ((zmax == -1) ? kFALSE : kTRUE);

}
void KVPartitionFromLeaf::SetZminZmax(Int_t vmin, Int_t vmax)
{

   SetZmin(vmin);
   SetZmax(vmax);

}
