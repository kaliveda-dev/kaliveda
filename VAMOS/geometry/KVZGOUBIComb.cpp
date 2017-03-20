//Created by KVClassFactory on Tue Feb 14 10:53:57 2017
//Author: Patrick St-Onge,,,

#include "KVZGOUBIComb.h"

ClassImp(KVZGOUBIComb)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVZGOUBITrajectory</h2>
<h4>Class used to access one ZGOUBI Trajectory</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVZGOUBIComb::KVZGOUBIComb()
   : KVBase()
{
   // Default constructor
   N1 = allcomb(1);
   N2 = allcomb(2);
   N3 = allcomb(3);
   N4 = allcomb(4);
   N5 = allcomb(5);
   N6 = allcomb(6);
   N7 = allcomb(7);
   N8 = allcomb(8);
   N9 = allcomb(9);
   N10 = allcomb(10);
}

//____________________________________________________________________________//


KVZGOUBIComb::KVZGOUBIComb(const KVZGOUBIComb& obj) : KVBase()
{
   // Copy constructor
   // Use this constructor to copy an existing object
   obj.Copy(*this);
}

//____________________________________________________________________________//

KVZGOUBIComb::KVZGOUBIComb(const Char_t* name, const Char_t* title)
   : KVBase(name, title)
{
   // Constructor inherited from KVBase
}

//____________________________________________________________________________//

KVZGOUBIComb::~KVZGOUBIComb()
{
   // Destructor
}

//____________________________________________________________________________//

void KVZGOUBIComb::Copy(TObject& obj) const
{
   // This method copies the current state of 'this' KVZGOUBITrajectory
   // object into 'obj'.

   KVBase::Copy(obj);
   KVZGOUBIComb& CastedObj = (KVZGOUBIComb&)obj;
}
//____________________________________________________________________________//

std::vector<std::vector<int>> KVZGOUBIComb::comb(int N, int K)
{
   std::vector<int> vectorcombination;
   std::vector<std::vector<int>> allcombinations;
   std::string bitmask(K, 1);
   bitmask.resize(N, 0);
   do {
      for (int i = 0; i < N; ++i) {
         if (bitmask[i]) vectorcombination.push_back(i);
      }
      allcombinations.push_back(vectorcombination);
      vectorcombination.clear();
   } while (std::prev_permutation(bitmask.begin(), bitmask.end()));
   return allcombinations;
}

std::vector<std::vector<int>> KVZGOUBIComb::allcomb(int N)
{
   std::vector<std::vector<int>> allcombinations_forN;
   std::vector<std::vector<int>> allcombinations;
   for (int i = 1; i <= N; i++) {
      allcombinations = comb(N, i);
      allcombinations_forN.insert(allcombinations_forN.end(), allcombinations.begin(), allcombinations.end());
   }
   return allcombinations_forN;
}

std::vector<std::vector<int>> KVZGOUBIComb::GetComb(int N)
{
   if (N == 10) {
      return N10;
   } else if (N > 10) {
      return allcomb(N);
   } else if (N == 1) {
      return N1;
   } else if (N == 2) {
      return N2;
   } else if (N == 3) {
      return N3;
   } else if (N == 4) {
      return N4;
   } else if (N == 5) {
      return N5;
   } else if (N == 6) {
      return N6;
   } else if (N == 7) {
      return N7;
   } else if (N == 8) {
      return N8;
   } else if (N == 9) {
      return N9;
   } else {
      std::vector<std::vector<int>> empty;
      return empty;
   }
}
