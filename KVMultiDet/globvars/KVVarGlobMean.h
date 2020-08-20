#ifndef KVVarGlobMean_h
#define KVVarGlobMean_h
#include "KVVarGlob.h"

/**
  \class KVVarGlobMean
  \brief Abstract base class for global variables which calculate a mean value
  \ingroup GlobalVariables

          *****  Base class for Global variables with   ******
                 a mean value and a standard deviation

   This class is a base abstact class for the management of global variable with a mean value and a standard
deviation. A global variable class deriving from KVVarGlobmean has to provide the following methods:
      virtual void Init(void)                 initiates the internal fields
      virtual void Reset(void)                resets internal fields before the treatment
      virtual void Fill(KVNucleus *c)         tells how to compute the global variable using a KVNucleus

 A call to the GetValue() method returns the mean value of the variable. One can also obtain the array which
 contains the values of the internal fields by a call to GetValuePtr(). Individual values can also be
 obtained by a call to GetValue(Int_t index) or by a call to  GetValue(Chat_t *name). Parenthesis can also be
 used. The correspondance between indexes ans names is as follows:

      Name            Index                   Meaning
----------------------------------------------------------------
      Mean            0       (default)       Mean value of var = SumVar/SumOfWeights
      RMS             1                       standard deviation of var = sqrt(SumVarSquared/SumOfWeights-Mean*Mean)
      SumVar          2                       Sum of weighted var values
      SumVarSquared   3                       Sum of weighted squared var values
      SumOfWeights    4                       Sum of weights
      Min             5                       Minimum value
      Max             6                       Maximum value


 Here are examples on how obtaining values.

 KVZmean *pzmean=new KVZmean();
 KVZmean zmean;
 ...
 Double_t mean=pzmean->GetValue();        |
          mean=pzmean->GetValue("Mean");  |---> Mean value
          mean=zmean();                   |
          mean=zmean("Mean");             |
 Double_t ect=zmean("RMS");---------------> Standard Deviation
 Double_t sw=pzmean->GetValue(4) ---------> Sum of weights
 Double_t *pvalues=zmean.GetValuePtr();---> Array of values

 The internal fields can be set by a call to the protected methods FillVar(Double_t v, Double_t w) or
 FillVar(Double_t v). In the latter case, the weight is set to 1.

      Method                                  Action
----------------------------------------------------------------
      FillVar(Double_t v, Double_t w)         SumVar+=w*v;
                                              SumVarSquared+=w*v*v;
                                              SumOfWeights+=w;

      FillVar(Double_t v)                     SumVar+=v;
                                              SumVarSquared+=v*v;
                                              SumOfWeights+=1;

 Mean value and standard deviation are computed by the protected CalcVar() method.
 Have a look to the KVZmean class for an example of a global variable deriving from KVVarGlobMean.
 Look at KVVarGlob class to have an example of use.

 By default, only Mean and RMS branches are added to TTrees.

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
