//Created by KVClassFactory on Thu Sep 30 16:48:24 2010
//Author: John Frankland,,,,

#ifndef __KVPARTITIONFUNCTION_H
#define __KVPARTITIONFUNCTION_H

#include "TNamed.h"
#include "TString.h"
#include "THashTable.h"

/**
\class KVPartitionFunction
\brief Calculates number of partitions of (A,Z,M)
\ingroup Analysis

These functions calculate the total number of partitions and the number of partitions
per multiplicity either for a single component system (corresponding to the partition function
of number theory, i.e. the number of ways to decompose an integer into distinct sums of other
integers), or for a system made of two kinds of objects, i.e. protons and neutrons, with A=N+Z.

These are exact results, the numbers correspond to each and every partition appearing
once and only once.

#### Single component system
The method
~~~~{.cpp}
Double_t PartFunc(Int_t A, Int_t M)
~~~~
calculates the total number of ways to partition an integer
A into the sum of M integers (\f$1\leq M\leq A\f$). We use the recursion relation given by J.P. Bondorf in
Nucl. Phys. A443, 321 (1985), Eq. (2.6).

The method
~~~~{.cpp}
Double_t PartSum(Int_t A)
~~~~
returns the total number of partitions summed over all multiplicities.

#### Two component system
The method
~~~~{.cpp}
Double_t PartFunc(Int_t A, Int_t Z, Int_t M)
~~~~
calculates the total number of ways to
partition Z protons and (A-Z) neutrons into M fragments, using the method given by K. Sneppen
in Nucl. Phys. A470, 213 (1987), Eqs. (4)-(6).

The method
~~~~{.cpp}
Double_t PartSum(Int_t A, Int_t Z)
~~~~
returns the total number of partitions summed over all multiplicities.
*/
class KVPartitionFunction : public TObject {
   THashTable fTable;

   Double_t maxvalueNp;
   Double_t maxvalueNclass;
   Double_t NvalsNp;
   Double_t NvalsNcl;

   Double_t sneppen_Nclass(int A, int Z, int M, int B);
   Double_t sneppen_Np(int A, int Z, int M);
   Double_t calc_sneppen_Nclass(int A, int Z, int M, int B);
   Double_t calc_sneppen_Np(int A, int Z, int M);

   const Char_t* name_value(Int_t A, Int_t Z, Int_t M, Int_t B = -1)
   {
      if (B > -1) return Form("A%dZ%dM%dB%d", A, Z, M, B);
      return Form("A%dZ%dM%d", A, Z, M);
   }

   void store_value(Double_t val, Int_t A, Int_t Z, Int_t M, Int_t B = -1)
   {
      TString name = name_value(A, Z, M, B);
      TNamed* o = new TNamed(name.Data(), "");
      o->SetTitle(Form("%f", val));
      fTable.Add(o);
   }

   Double_t get_value(Int_t A, Int_t Z, Int_t M, Int_t B = -1)
   {
      TString name = name_value(A, Z, M, B);
      TNamed* o = (TNamed*)fTable.FindObject(name.Data());
      if (!o) return -1.;
      Double_t val;
      sscanf(o->GetTitle(), "%lf", &val);
      return val;
   }

public:
   KVPartitionFunction();
   virtual ~KVPartitionFunction();

   Double_t PartFunc(int A, int M);
   Double_t PartSum(int A);
   Double_t PartFunc(int A, int Z, int M);
   Double_t PartSum(int A, int Z);

   Double_t GetMaxValueNp() const
   {
      return maxvalueNp;
   };
   Double_t GetMaxValueNclass() const
   {
      return maxvalueNclass;
   };

   Double_t MeanNA(int A0, int A);
   Double_t MeanNA_M(int A0, int A, int M);
   Double_t MeanNA(int A0, int Z0, int A);
   Double_t MeanNZ(int A0, int Z0, int Z);
   Double_t MeanNAZ(int A0, int Z0, int A, int Z);

   Double_t MeanM(int A0);
   Double_t MeanA(int A0);

   ClassDef(KVPartitionFunction, 0) //Calculates number of partitions of A or (A,Z) into fragments
};

#endif
