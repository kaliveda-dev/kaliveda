#ifndef __KVEVENTCLASSIFIER_H
#define __KVEVENTCLASSIFIER_H

#include "KVVarGlob1.h"

/**
 \class KVEventClassifier
 \brief Simple class for sorting events according to global variables
 \ingroup GlobalVariables
 \ingroup Analysis

 This is a simple class for sorting events according to the value of some global variable (see KVVarGlob).
 Based on either a set of \f$N\f$ cuts or a set of \f$N+1\f$ bins (defined by lower and upper bounds), this class
 will attribute an integer value \f$0,...,N\f$ to each event based on the value of the variable.

 As for any global variable, the event class value can be retrieved with the usual GetValue() method after Calculate() is called.
 It can be used for event selection (see SetEventSelection()) or be stored with other analysis results in a TTree
 (see KVGVList::MakeBranches()).

### Use in user analysis classes

In any analysis class derived from KVEventSelector, an event classifier can be added to the list of global variables
in order to define cuts or bins for any variable which is already in the list:
~~~~{.cpp}
[in the InitAnalysis() method of some analysis class]
AddGV("KVMult", "mtot");
auto mt_cuts = GetGVList()->AddEventClassifier("mtot");
~~~~
See method KVGVList::AddEventClassifier().

Additionally, if the global variable used provides several named values, a mathematical expression can
be used involving these values. For example, with the KVSource variable, it is possible to do
~~~~{.cpp}
AddGV("KVSource", "QP");
auto ec = GetGVList()->AddEventClassifier("QP", "Ex/A");
~~~~
in order to sort according to the excitation energy per nucleon of the reconstructed source.

If the global variable list is then used to create automatic branches in a TTree (KVGVList::MakeBranches()),
this event classification object will add a branch `mtot_EC` in order to store its values.

#### Using cuts
~~~~{.cpp}
     mtot_cuts->AddCut(5);
     mtot_cuts->AddCut(10);
     mtot_cuts->AddCut(15);
     mtot_cuts->AddCut(20);
     mtot_cuts->AddCut(25);
     mtot_cuts->AddCut(30);
~~~~

This will class events according to:

| mtot       | mtot_EC |
|------------|---------|
|         <5 |    0    |
|  [5, 9]    |    1    |
|  [10, 14]  |    2    |
|  [15, 19]  |    3    |
|  [20, 24]  |    4    |
|  [25, 29]  |    5    |
|  >=30      |    6    |

#### Using bins
~~~~{.cpp}
     mtot_cuts->AddBin(5,10);
     mtot_cuts->AddBin(15,20);
     mtot_cuts->AddBin(25,30);
~~~~

This will class events according to the 2 bins with the following numbers:

| mtot     |   mtot_EC   |
|----------|-------------|
|  [5, 9]  |    0        |
| [15, 19] |    1        |
| [25, 29] |    2        |
| other    |    -1       |

Note that in this case, any value outside of a defined bin is unclassified.

 \author John Frankland
 \date Mon Jan 25 16:25:26 2021
*/

class KVEventClassifier : public KVVarGlob1 {
   KVVarGlob* fVar;//! variable used for event classification
   TString fVal;//! optional name of value calculated by variable to use
   std::vector<double> fCuts;//! cuts in variable to define event classes
   bool fBcuts;//! true if cuts are to be used
   bool fBins;//! true if separate bins are to be used
   bool fWithVal;//! true if named value given
   bool fWithExpression;//! true if an expression using several values is used
   std::unique_ptr<TFormula> fFormula;//! in case a combination of values is used
   std::vector<std::string> fValues;//! indices of values in parsed expression

   int calc_where() const;

public:
   KVEventClassifier() : KVVarGlob1("KVEventClassifier"), fVar(nullptr), fVal(""), fBcuts(false),
      fBins(false), fWithVal(false), fWithExpression(false), fFormula(nullptr)
   {}
   KVEventClassifier(KVVarGlob* b, const TString& value = "")
      : KVVarGlob1((value == "") ? Form("%s_EC", b->GetName()) : Form("%s_%s_EC", b->GetName(), value.Data())),
        fVar(b), fVal(value), fBcuts(false), fBins(false), fWithVal(value != ""), fWithExpression(false), fFormula(nullptr)
   {
      // \param[in] b address of global variable to use for event classification
      // \param[in] value [optional] for multi-valued variables, you can specify which value to use by name, or a mathematical expression
      //                involving one or more of the available values
   }
   virtual ~KVEventClassifier() {}
   ROOT_COPY_CTOR(KVEventClassifier, KVVarGlob1)
   ROOT_COPY_ASSIGN_OP(KVEventClassifier)

   void Copy(TObject& other) const
   {
      // Copy this object into other
      KVVarGlob1::Copy(other);
      ((KVEventClassifier&)other).fVar = fVar;
      ((KVEventClassifier&)other).fVal = fVal;
      ((KVEventClassifier&)other).fCuts = fCuts;
      ((KVEventClassifier&)other).fBcuts = fBcuts;
      ((KVEventClassifier&)other).fBins = fBins;
      ((KVEventClassifier&)other).fWithVal = fWithVal;
      ((KVEventClassifier&)other).fWithExpression = fWithExpression;
      ((KVEventClassifier&)other).fFormula.reset(new TFormula(*(fFormula.get())));
      ((KVEventClassifier&)other).fValues = fValues;
   }
   Bool_t IsGlobalVariable() const
   {
      return kFALSE;
   }
   int GetEventClassification() const
   {
      // Returns event class number according to current value of global variable:
      //
      //  * WITH \f$N\f$ CUTS: 0 for values < first cut, \f$N\f$ for values > last cut
      //  * WITH \f$N\f$ BINS: 0 for values in first bin, ..., \f$N-1\f$ for values in last bin, -1 if not in a bin

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
   int GetMaxEventClassNumber() const
   {
      // Returns the largest possible value returned by GetEventClassification() or GetValue():
      //
      //  * for \f$N\f$ cuts, this is \f$N\f$ (\f$N+1\f$ values)
      //  * for \f$N\f$ bins, this is \f$N-1\f$ (\f$N\f$ values, plus -1 if outside all bins)

      if (fBins) return fCuts.size() / 2;
      return fCuts.size();
   }

   void AddCut(double x)
   {
      // Add a cut to be used to sort events. The order in which cuts are added is unimportant,
      // events are always classed from the lowest to the highest value of the variable.
      //
      //~~~~{.cpp}
      //    AddCut(5);
      //    AddCut(10);
      //    AddCut(15);
      //    AddCut(20);
      //    AddCut(25);
      //    AddCut(30);
      //~~~~
      //
      //This will class events according to:
      // | variable   |      EC |
      // |------------|---------|
      // |         <5 |    0    |
      // |  [5, 9]    |    1    |
      // |  [10, 14]  |    2    |
      // |  [15, 19]  |    3    |
      // |  [20, 24]  |    4    |
      // |  [25, 29]  |    5    |
      // |  >=30      |    6    |

      if (fBins) {
         Error("AddCut", "You cannot mix cuts & bins (and you already defined some bins)");
         return;
      }
      fBcuts = true;
      fCuts.push_back(x);
   }
   void AddBin(double xmin, double xmax)
   {
      // Add a bin to be used to sort events. The order in which bins are added is unimportant,
      // events are always classed from the lowest to the highest value of the variable.
      //
      //~~~~{.cpp}
      //   AddBin(5,10);
      //  AddBin(15,20);
      // AddBin(25,30);
      //~~~~
      //This will class events according to the 2 bins with the following numbers:
      // | variable |   EC        |
      // |----------|-------------|
      // |  [5, 9]  |    0        |
      // | [15, 19] |    1        |
      // | [25, 29] |    2        |
      // | other    |    -1       |
      //
      // Note that in this case, any value outside of a defined bin is unclassified.

      if (fBcuts) {
         Error("AddBin", "You cannot mix cuts & bins (and you already defined some cuts)");
         return;
      }
      fBins = true;
      fCuts.push_back(xmin);
      fCuts.push_back(xmax);
   }

   void Init();
   void Reset() {}
   void Calculate()
   {
      SetValue(GetEventClassification());
   }

   ClassDef(KVEventClassifier, 1) //Simple class for sorting events according to global variables
};

#endif
