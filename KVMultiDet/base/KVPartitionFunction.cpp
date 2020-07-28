//Created by KVClassFactory on Thu Sep 30 16:48:24 2010
//Author: John Frankland,,,,

#include "KVPartitionFunction.h"
#include "TMath.h"

ClassImp(KVPartitionFunction)



KVPartitionFunction::KVPartitionFunction()
   : fTable(10000000, 5)
{
   // Default constructor
}

KVPartitionFunction::~KVPartitionFunction()
{
   // Destructor
}

Double_t KVPartitionFunction::PartFunc(int A, int M)
{
   // recursive calculation of number of partitions of A nucleons
   // into M fragments of size 1, 2, ..., A
   // from Eq. (2.6) of Bondorf et al., Nucl. Phys. A443, 321 (1985):
   /*    "We now consider the division of the partition space into the subsets characterized
   by having a common multiplicity. Let P(A,, M) be the number of partitions in one
   of such subsets. To compute P(A,, M) it is sufficient to observe that it should be
   equal to the number of partitions having at least one fragment composed of just
   one nucleon (which number is immediately shown to equal P(A0 - 1, M - 1)) plus
   those not having any unitary fragment. The number of this last portion can be
   computed by observing that, if we take one nucleon from each of its fragments one
   has the set of all partitions of A0 - M nucleons into M fragments. We thus arrive at
   P(A0,M) = P(A0-1,M-l) + P(A0-M, M)    Eq. (2.5)"
   */
// The expression below follows from recursive use of this relation.

   if (!A) return 0;
   if (A == M) return 1;
   if (M == 1) return 1;
   Double_t p = 0;
   int klim = A / M - 1;
   for (int k = 0; k <= klim; k++) {
      p += PartFunc(A - k * M - 1, M - 1);
   }
   return p;
}

Double_t KVPartitionFunction::PartSum(int A)
{
   // calculate total number of partitions of A nucleons into
   // 1, 2, ..., A fragments.
   // from Eq. (2.7) of Bondorf et al., Nucl. Phys. A443, 321 (1985)

   Double_t p = 0.;
   for (int m = 1; m <= A; m++) p += PartFunc(A, m);
   return p;
}


Double_t KVPartitionFunction::sneppen_Nclass(int A, int Z, int M, int B)
{
   if (A > 0 && Z >= 0 && M > 0 && B >= 0) {
      Double_t snc = get_value(A, Z, M, B);
      if (snc < 0) {
         snc = calc_sneppen_Nclass(A, Z, M, B);
         store_value(snc, A, Z, M, B);
         maxvalueNclass = TMath::Max(snc, maxvalueNclass);
      }
      return snc;
   }
   return 0;
}

Double_t KVPartitionFunction::sneppen_Np(int A, int Z, int M)
{
   if (A > 0 && Z >= 0 && M > 0) {
      if (Z > A - Z) Z = A - Z; // symmetry
      Double_t snc = get_value(A, Z, M);
      if (snc < 0) {
         snc = calc_sneppen_Np(A, Z, M);
         store_value(snc, A, Z, M);
         maxvalueNp = TMath::Max(snc, maxvalueNp);
      }
      return snc;
   }
   return 0;
}

Double_t KVPartitionFunction::calc_sneppen_Nclass(int A, int Z, int M, int B)
{
   // calculates Eqs. 5a-d

   if (M <= 0) return 0;
   if (A <= 0) return 0;
   if (Z < 0) return 0;
   if (B < 0) return 0;
   if (B > A - Z) return 0;
   if (Z == 0 && B == 0) return 0;
   if ((M - 1) > (A - B)) return 0;
   if (M == A) {
      if (Z == A && B > 0) return 0;
      if (B != 1) return 0;
   }
   if (Z == 0) {
      if (A - B * M > 0) {
         Double_t sn1 = sneppen_Np(A - (B - 1) * M, 0, M);
         Double_t sn2 = sneppen_Np(A - B * M, 0, M);
         return sn1 - sn2;
      }
      return 0;
   }
   if (B == 0) {
      if (A - M >= M && Z - M >= 0) {
         Double_t snp, snc;
         snp = sneppen_Np(A - M, Z - M, M);
         snc = sneppen_Nclass(A - 1, Z - 1, M - 1, 0);
         return snp + snc;
      }
      return 0;
   }
   Double_t ncl = 0;
   if (A - B > 0 && M - 1 > 0 && B > 1) {
      for (int i = 1; i <= B - 1; i++) {
         ncl += sneppen_Nclass(A - B, Z, M - 1, i);
      }
   }
   ncl = sneppen_Np(A - B, Z, M - 1) - ncl;
   return ncl;
}

Double_t KVPartitionFunction::calc_sneppen_Np(int A, int Z, int M)
{
   // calculates Eq. 4

   if (A <= 0) return 0;
   if (M <= 0) return 0;
   if (Z < 0) return 0;
   if (Z == 0) {
      // one-component system
      return PartFunc(A, M);
   }
   if (M > A) return 0;
   if (M == 1) return 1;
   if (M == A) return 1;
   Double_t Np = 0;
   int N = A - Z;
   for (int b = 0; b <= N; b++) Np += sneppen_Nclass(A, Z, M, b);
   return Np;
}

Double_t KVPartitionFunction::PartFunc(int A, int Z, int M)
{
   // Returns the total number of partitions of Z protons and (A-Z) neutrons
   // into M fragments, using the method given by K. Sneppen
   // in Nucl. Phys. A470, 213 (1987), Eqs. (4)-(6).

   Double_t p = sneppen_Np(A, Z, M);
   /*Info("PartFunc(A,Z,M)", "p=%f array use=%f max value=%f\n",
           p, NvalsNcl/pow(SNEPPENMAXTAB,4),GetMaxValueNclass());*/
   return p;
}

Double_t KVPartitionFunction::PartSum(int A, int Z)
{
   // Returns the total number of partitions of Z protons and (A-Z) neutrons
   // summed over all multiplicities, using the method given by K. Sneppen
   // in Nucl. Phys. A470, 213 (1987), Eqs. (4)-(6).

   Double_t p = 0;
   for (int m = 1; m <= A; m++) p += sneppen_Np(A, Z, m);
   /*Info("PartSum(A,Z)", "p=%f array use=%f max value=%f\n",
           p, NvalsNcl/pow(SNEPPENMAXTAB,4),GetMaxValueNclass());*/
   return p;
}

Double_t KVPartitionFunction::MeanNA(int A0, int A)
{
   // Calculate the mean number of clusters of size A when a system of size A0
   // fragments in all possible ways with equal probability
   // Using Eq. (3) of K. Sneppen Nucl. Phys. A470, 213 (1987)
   //
   // Correction for A=A0:
   //    there is always exactly 1 partition of M=1 composed of one fragment with A=A0
   //    the mean multiplicity of A=A0 is therefore 1/PartSum(A0)

   if (A == A0) return 1. / PartSum(A0);
   Double_t NA = 0;
   Int_t imax = (Int_t)(1.*A0 / (1.*A));
   for (int i = 1; i <= imax; i++) {
      Int_t A1 = A0 - i * A;
      if (A1 > 0) NA += PartSum(A1);
   }
   NA /= PartSum(A0);
   return NA;
}

Double_t KVPartitionFunction::MeanNA_M(int A0, int A, int M)
{
   // Calculate the mean number of clusters of size A when a system of size A0
   // breaks up into M fragments, each partition having equal probability
   // Using Eqs. (2b) & (3) of K. Sneppen Nucl. Phys. A470, 213 (1987)
   //
   // Correction for M=1:
   //     if A==A0, mean multiplicity is 1; otherwise 0.
   // Correction for M=2 and even values of A0:
   //    when an odd-A0 splits into M=2, there are PartFunc(A0,M) partitions
   //    in which each A between 1 and A0-1 occurs once only, therefore
   //    the mean multiplicity for any A is 1/PartFunc(A0,M)
   //    when an even-A0 splits into M=2, one of the PartFunc(A0,M) partitions
   //    is the symmetric split (A0/2, A0/2). Therefore the mean multiplicity
   //    of A=A0/2 is twice that of the other A, i.e. 2/PartFunc(A0,M)
   // Correction for M=A0:
   //    in this case only one partition exists, made of M=A0 monomers A=1
   //    therefore mean multiplicity of A=1 is M, for all other A it is 0.

   if (M == 1) {
      if (A == A0) return 1.;
      else return 0.;
   }
   if (M == 2) {
      if (!(A0 % 2) && A == A0 / 2) return 2. / PartFunc(A0, M);
      else if (A >= 1 && A < A0) return 1. / PartFunc(A0, M);
      else return 0.;
   }
   if (M == A0) {
      if (A == 1) return M;
      else return 0.;
   }
   Double_t NA = 0;
   Int_t imax = (Int_t)(1.*A0 / (1.*A));
   imax = TMath::Min(imax, M - 1);
   for (int i = 1; i <= imax; i++) {
      Int_t A1 = A0 - i * A;
      if (A1 > 0)NA += PartFunc(A1, M - i);
   }
   NA /= PartFunc(A0, M);
   return NA;
}
/*
    Double_t MeanNA(int A0, int Z0, int A);
    Double_t MeanNZ(int A0, int Z0, int Z);*/

Double_t KVPartitionFunction::MeanNAZ(int A0, int Z0, int A, int Z)
{
   // Calculate the mean number of clusters of mass A and charge Z
   // when a two-component system (A0,Z0) fragments in all possible ways with equal probability
   // Using Eq. (7) of K. Sneppen Nucl. Phys. A470, 213 (1987)

   Double_t NAZ = 0;
   Int_t imax = (Int_t)(1.*A0 / (1.*A));
   for (int i = 1; i <= imax; i++) {
      Int_t A1 = A0 - i * A;
      Int_t Z1 = Z0 - i * Z;
      if (A1 > 0) NAZ += PartSum(A1, Z1);
   }
   NAZ /= PartSum(A0, Z0);
   return NAZ;
}

Double_t KVPartitionFunction::MeanNA(int A0, int Z0, int A)
{
   // Calculate the mean number of clusters of mass A
   // when a two-component system (A0,Z0) fragments in all possible ways with equal probability
   // This is just the sum of MeanNAZ(A0,Z0,A,Z) with 0<=Z<= min(A,Z0)

   Double_t NA = 0;
   for (Int_t Z = 0; Z <= TMath::Min(A, Z0); Z++) NA += MeanNAZ(A0, Z0, A, Z);
   return NA;
}

Double_t KVPartitionFunction::MeanNZ(int A0, int Z0, int Z)
{
   // Calculate the mean number of clusters of charge Z
   // when a two-component system (A0,Z0) fragments in all possible ways with equal probability
   // This is just the sum of MeanNAZ(A0,Z0,A,Z) with Z<=A<=(Z+A0-Z0)

   Double_t NZ = 0;
   for (Int_t A = Z; A <= (Z + A0 - Z0); A++) NZ += MeanNAZ(A0, Z0, A, Z);
   return NZ;
}

Double_t KVPartitionFunction::MeanM(int A0)
{
   // Calculate the mean multiplicity of all partitions of a single-component
   // system of mass A0.
   Double_t mult = 0.;
   Double_t part_sum = 0;
   Double_t part_func = 0;
   for (int i = 1; i <= A0; i++) {
      part_func = PartFunc(A0, i);
      mult += i * part_func;
      part_sum += part_func;
   }
   mult /= part_sum;
   return mult;
}

Double_t KVPartitionFunction::MeanA(int A0)
{
   // Calculate the mean size of clusters in all partitions of a single-component
   // system of mass A0.
   Double_t mult, moy = 0.;
   Double_t sum = 0;
   for (int i = 1; i <= A0; i++) {
      mult = MeanNA(A0, i);
      moy += i * mult;
      sum += mult;
   }
   moy /= sum;
   return moy;
}
