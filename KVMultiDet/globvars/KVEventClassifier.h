#ifndef __KVEVENTCLASSIFIER_H
#define __KVEVENTCLASSIFIER_H

#include "KVVarGlob1.h"
#include <iostream>

/**
 \class KVEventClassifier
 \brief Simple class for sorting events according to global variables
 \ingroup GlobVars
 Write a detailed documentation for your class here, see doxygen manual for help.

 \author John Frankland
 \date Mon Jan 25 16:25:26 2021
*/

class KVEventClassifier : public KVVarGlob1 {
   KVVarGlob* fVar;//! variable used for event classification
   std::vector<double> fCuts;//! cuts in variable to define event classes
   bool fBins;//! true if separate bins are to be used

   int calc_where() const;

public:
   KVEventClassifier() : KVVarGlob1("KVEventClassifier"), fVar(nullptr), fBins(false) {}
   KVEventClassifier(KVVarGlob* b) : KVVarGlob1(Form("%s_EC", b->GetName())), fVar(b), fBins(false) {}
   virtual ~KVEventClassifier() {}
   ROOT_COPY_CTOR(KVEventClassifier, KVVarGlob1)
   ROOT_COPY_ASSIGN_OP(KVEventClassifier)

   void Copy(TObject& other) const
   {
      // Copy this object into other
      KVVarGlob1::Copy(other);
      ((KVEventClassifier&)other).fVar = fVar;
      ((KVEventClassifier&)other).fCuts = fCuts;
      ((KVEventClassifier&)other).fBins = fBins;
   }
   Bool_t IsGlobalVariable() const
   {
      return kFALSE;
   }
   int GetEventClassification() const
   {
      // Returns event class number according to current value of global variable.
      //
      //  * WITH N CUTS: 0 for values < first cut, N for values > last cut
      //  * WITH N BINS: 0 for values in first bin, ..., N for values in last bin, -1 if not in a bin

      int where = calc_where();
      if (fBins) {
         // odd values correspond to bins
         if (where % 2) {
            return where / 2;
         }
         return -1;
      }
      return where;
   }

   void AddCut(double x)
   {
      if (fBins) {
         Error("AddCut", "You cannot mix cuts & bins (and you already defined some bins)");
         return;
      }
      fCuts.push_back(x);
   }
   void AddBin(double xmin, double xmax)
   {
      fBins = true;
      fCuts.push_back(xmin);
      fCuts.push_back(xmax);
   }

   void Init()
   {
      // Sort cuts into ascending order
      std::sort(fCuts.begin(), fCuts.end());
   }
   void Reset() {}
   void Calculate()
   {
      SetValue(GetEventClassification());
   }

   ClassDef(KVEventClassifier, 1) //Simple class for sorting events according to global variables
};

#endif
