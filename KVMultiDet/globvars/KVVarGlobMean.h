#ifndef KVVarGlobMean_h
#define KVVarGlobMean_h
#include "KVVarGlob.h"

/**
  \class KVVarGlobMean
  \brief Abstract base class for global variables which calculate a mean value
  \ingroup GlobalVariables

This class is a base class for the management of global variables with a mean value and a standard
deviation.

A call to the GetValue() method returns the mean value of the variable.

Other values can be retrieved by index or by name.
The correspondance between indexes and names is as follows:

Name           | Index                 |  Meaning
---------------|-----------------------|------------------------------------------------------------------------
Mean           | 0                     | Mean value of var = SumVar/SumOfWeights
RMS            | 1                     | Standard deviation of var = sqrt(SumVarSquared/SumOfWeights-Mean*Mean)
SumVar         | 2                     | Sum of weighted var values
SumVarSquared  | 3                     | Sum of weighted squared var values
SumOfWeights   | 4                     | Sum of weights
Min            | 5                     | Minimum value
Max            | 6                     | Maximum value

 By default, only Mean and RMS branches are added to TTrees (see KVGVList::MakeBranches).

  \author D. Cussol (LPC Caen), J.D. Frankland (GANIL)
  \date 2004-2020
 */

class KVVarGlobMean: public KVVarGlob {

   Double_t var;
   Double_t ect;
   Double_t svar2;
   Double_t svar;
   Double_t sw;
   Double_t min;
   Double_t max;
   Int_t calc;

   void init();

protected:
   virtual Double_t getvalue_int(Int_t i) const;

   void FillVar(Double_t v, Double_t w = 1.);
   static void FillMethodBody(KVString& body, int type);

public:
   KVVarGlobMean() : KVVarGlob("KVVarGlobMean")
   {
      init();
   }
   KVVarGlobMean(const Char_t* nom) : KVVarGlob(nom)
   {
      init();
   }
   ROOT_COPY_CTOR(KVVarGlobMean, KVVarGlob)
   ROOT_COPY_ASSIGN_OP(KVVarGlobMean)
   virtual ~KVVarGlobMean() {}

   void Copy(TObject& obj) const;
   void Reset();
   void Init();
   void Calculate();

   static void MakeClass(const Char_t* classname, const Char_t* classdesc, int type = KVVarGlob::kOneBody);

   ClassDef(KVVarGlobMean, 2)// Base class for global variables computing mean values
};
#endif
