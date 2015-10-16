//Created by KVClassFactory on Fri Sep 25 18:58:04 2015
//Author: fable

#ifndef __NEWROOABSDATA_H
#define __NEWROOABSDATA_H


#include "NewRooAbsOptTestStatistic.h"
#include "TNamed.h"
#include "RooPrintable.h"
#include "RooArgSet.h"
#include "RooFormulaVar.h"
#include <math.h>
#include "TMatrixDSym.h"

class RooAbsArg;
class RooAbsReal ;
class RooAbsCategory ;
class Roo1DTable ;
class RooPlot;
class RooArgList;
class TH1;
class RooAbsBinning ;
class Roo1DTable ;
class RooAbsDataStore ;

class NewRooAbsData :  public TNamed, public RooPrintable
{

  // Constructors, factory methods etc.
  NewRooAbsData() ; 
  NewRooAbsData(const char *name, const char *title, const RooArgSet& vars, RooAbsDataStore* store=0) ;
  NewRooAbsData(const NewRooAbsData& other, const char* newname = 0) ;
  virtual ~NewRooAbsData() ;
  virtual NewRooAbsData* emptyClone(const char* newName=0, const char* newTitle=0, const RooArgSet* vars=0, const char* wgtVarName=0) const = 0 ;

  // Reduction methods
  NewRooAbsData* reduce(const RooCmdArg& arg1,const RooCmdArg& arg2=RooCmdArg(),const RooCmdArg& arg3=RooCmdArg(),const RooCmdArg& arg4=RooCmdArg(),
                     const RooCmdArg& arg5=RooCmdArg(),const RooCmdArg& arg6=RooCmdArg(),const RooCmdArg& arg7=RooCmdArg(),const RooCmdArg& arg8=RooCmdArg()) ;
  NewRooAbsData* reduce(const char* cut) ;
  NewRooAbsData* reduce(const RooFormulaVar& cutVar) ;
  NewRooAbsData* reduce(const RooArgSet& varSubset, const char* cut=0) ;
  NewRooAbsData* reduce(const RooArgSet& varSubset, const RooFormulaVar& cutVar) ;

  RooAbsDataStore* store() { return _dstore ; }
  const RooAbsDataStore* store() const { return _dstore ; }
  const TTree* tree() const ;
   
  void convertToVectorStore() ;

  void attachBuffers(const RooArgSet& extObs) ;
  void resetBuffers() ;
 
  
  virtual void Draw(Option_t* option = "") ;

  void checkInit() const ; 

  // Change name of observable
  virtual Bool_t changeObservableName(const char* from, const char* to) ;

  // Add one ore more rows of data
  virtual void add(const RooArgSet& row, Double_t weight=1, Double_t weightError=0) = 0 ; // DERIVED
  virtual void fill() ; 

  // Load a given row of data
  virtual inline const RooArgSet* get() const { 
    // Return current row of dataset
    return &_vars ; 
  } 
  virtual Double_t weight() const = 0 ; // DERIVED
  virtual Double_t weightSquared() const = 0 ; // DERIVED
  virtual Bool_t valid() const { return kTRUE ; }
  enum ErrorType { Poisson, SumW2, None, Auto, Expected } ;
  virtual Double_t weightError(ErrorType etype=Poisson) const ;
  virtual void weightError(Double_t& lo, Double_t& hi, ErrorType etype=Poisson) const ; 
  virtual const RooArgSet* get(Int_t index) const ;

  virtual Int_t numEntries() const ;
  virtual Double_t sumEntries() const = 0 ;
  virtual Double_t sumEntries(const char* cutSpec, const char* cutRange=0) const = 0 ; // DERIVED
  virtual Bool_t isWeighted() const { 
    // Do events in dataset have weights?
    return kFALSE ; 
  }
  virtual Bool_t isNonPoissonWeighted() const { 
    // Do events in dataset have non-integer weights?
    return kFALSE ; 
  }
  virtual void reset() ;


  Bool_t getRange(RooRealVar& var, Double_t& lowest, Double_t& highest, Double_t marginFrac=0, Bool_t symMode=kFALSE) const ;

  // Plot the distribution of a real valued arg
  virtual Roo1DTable* table(const RooArgSet& catSet, const char* cuts="", const char* opts="") const ;
  virtual Roo1DTable* table(const RooAbsCategory& cat, const char* cuts="", const char* opts="") const ;
  virtual RooPlot* plotOn(RooPlot* frame, 
			  const RooCmdArg& arg1=RooCmdArg::none(), const RooCmdArg& arg2=RooCmdArg::none(),
			  const RooCmdArg& arg3=RooCmdArg::none(), const RooCmdArg& arg4=RooCmdArg::none(),
			  const RooCmdArg& arg5=RooCmdArg::none(), const RooCmdArg& arg6=RooCmdArg::none(),
			  const RooCmdArg& arg7=RooCmdArg::none(), const RooCmdArg& arg8=RooCmdArg::none()) const ;

  virtual RooPlot* plotOn(RooPlot* frame, const RooLinkedList& cmdList) const ;

  // WVE --- This needs to be public to avoid CINT problems
  struct PlotOpt {
   PlotOpt() : cuts(""), drawOptions("P"), bins(0), etype(NewRooAbsData::Poisson), cutRange(0), histName(0), histInvisible(kFALSE),
              addToHistName(0),addToWgtSelf(1.),addToWgtOther(1.),xErrorSize(1),refreshFrameNorm(kFALSE),correctForBinWidth(kTRUE),
              scaleFactor(1.) {} ;
   const char* cuts ;
   Option_t* drawOptions ;
   RooAbsBinning* bins ;
   NewRooAbsData::ErrorType etype ;
   const char* cutRange ;
   const char* histName ;
   Bool_t histInvisible ;
   const char* addToHistName ;
   Double_t addToWgtSelf ;
   Double_t addToWgtOther ;
   Double_t xErrorSize ;
   Bool_t refreshFrameNorm ;
   Bool_t correctForBinWidth ;
   Double_t scaleFactor ;
  } ;
	
  // Split a dataset by a category
  virtual TList* split(const RooAbsCategory& splitCat, Bool_t createEmptyDataSets=kFALSE) const ;

  // Fast splitting for SimMaster setData
  Bool_t canSplitFast() const ; 
  NewRooAbsData* getSimData(const char* idxstate) ;
			
  // Create 1,2, and 3D histograms from and fill it
  TH1 *createHistogram(const char *name, const RooAbsRealLValue& xvar,
                       const RooCmdArg& arg1=RooCmdArg::none(), const RooCmdArg& arg2=RooCmdArg::none(), 
                       const RooCmdArg& arg3=RooCmdArg::none(), const RooCmdArg& arg4=RooCmdArg::none(), 
                       const RooCmdArg& arg5=RooCmdArg::none(), const RooCmdArg& arg6=RooCmdArg::none(), 
                       const RooCmdArg& arg7=RooCmdArg::none(), const RooCmdArg& arg8=RooCmdArg::none()) const ;
  TH1* createHistogram(const char *name, const RooAbsRealLValue& xvar, const RooLinkedList& argList) const ;
  TH1 *createHistogram(const char* varNameList, Int_t xbins=0, Int_t ybins=0, Int_t zbins=0) const ;

  // Fill an existing histogram
  virtual TH1 *fillHistogram(TH1 *hist, const RooArgList &plotVars, const char *cuts= "", const char* cutRange=0) const;

  // Printing interface (human readable)
  inline virtual void Print(Option_t *options= 0) const {
    // Print contents on stdout
    printStream(defaultPrintStream(),defaultPrintContents(options),defaultPrintStyle(options));
  }

  virtual void printName(std::ostream& os) const ;
  virtual void printTitle(std::ostream& os) const ;
  virtual void printClassName(std::ostream& os) const ;
  void printMultiline(std::ostream& os, Int_t contents, Bool_t verbose=kFALSE, TString indent="") const ;

  virtual Int_t defaultPrintContents(Option_t* opt) const ;

  void setDirtyProp(Bool_t flag) ;
  
  Double_t moment(RooRealVar &var, Double_t order, const char* cutSpec=0, const char* cutRange=0) const ;
  Double_t moment(RooRealVar &var, Double_t order, Double_t offset, const char* cutSpec=0, const char* cutRange=0) const ;
  Double_t standMoment(RooRealVar &var, Double_t order, const char* cutSpec=0, const char* cutRange=0) const ;

  Double_t mean(RooRealVar& var, const char* cutSpec=0, const char* cutRange=0) const { return moment(var,1,0,cutSpec,cutRange) ; }
  Double_t sigma(RooRealVar& var, const char* cutSpec=0, const char* cutRange=0) const { return sqrt(moment(var,2,cutSpec,cutRange)) ; }
  Double_t skewness(RooRealVar& var, const char* cutSpec=0, const char* cutRange=0) const { return standMoment(var,3,cutSpec,cutRange) ; }
  Double_t kurtosis(RooRealVar& var, const char* cutSpec=0, const char* cutRange=0) const { return standMoment(var,4,cutSpec,cutRange) ; }

  Double_t covariance(RooRealVar &x,RooRealVar &y, const char* cutSpec=0, const char* cutRange=0) const { return corrcov(x,y,cutSpec,cutRange,kFALSE) ; }
  Double_t correlation(RooRealVar &x,RooRealVar &y, const char* cutSpec=0, const char* cutRange=0) const { return corrcov(x,y,cutSpec,cutRange,kTRUE) ; }

  TMatrixDSym* covarianceMatrix(const char* cutSpec=0, const char* cutRange=0) const { return covarianceMatrix(*get(),cutSpec,cutRange) ; }
  TMatrixDSym* correlationMatrix(const char* cutSpec=0, const char* cutRange=0) const { return correlationMatrix(*get(),cutSpec,cutRange) ; }
  TMatrixDSym* covarianceMatrix(const RooArgList& vars, const char* cutSpec=0, const char* cutRange=0) const { return corrcovMatrix(vars,cutSpec,cutRange,kFALSE) ; }
  TMatrixDSym* correlationMatrix(const RooArgList& vars, const char* cutSpec=0, const char* cutRange=0) const { return corrcovMatrix(vars,cutSpec,cutRange,kTRUE) ; }
  
  RooRealVar* meanVar(RooRealVar &var, const char* cutSpec=0, const char* cutRange=0) const ;
  RooRealVar* rmsVar(RooRealVar &var, const char* cutSpec=0, const char* cutRange=0) const ;

  virtual RooPlot* statOn(RooPlot* frame, 
                          const RooCmdArg& arg1=RooCmdArg::none(), const RooCmdArg& arg2=RooCmdArg::none(), 
                          const RooCmdArg& arg3=RooCmdArg::none(), const RooCmdArg& arg4=RooCmdArg::none(), 
                          const RooCmdArg& arg5=RooCmdArg::none(), const RooCmdArg& arg6=RooCmdArg::none(), 
                          const RooCmdArg& arg7=RooCmdArg::none(), const RooCmdArg& arg8=RooCmdArg::none()) ;

  virtual RooPlot* statOn(RooPlot* frame, const char *what, 
			  const char *label= "", Int_t sigDigits= 2,
			  Option_t *options= "NELU", Double_t xmin=0.15, 
			  Double_t xmax= 0.65,Double_t ymax=0.85, 
                          const char* cutSpec=0, const char* cutRange=0, 
                          const RooCmdArg* formatCmd=0);



  Bool_t hasFilledCache() const ; 

  void addOwnedComponent(const char* idxlabel, NewRooAbsData& data) ;
  static void claimVars(NewRooAbsData*) ;
  static Bool_t releaseVars(NewRooAbsData*) ;

  enum StorageType { Tree, Vector} ;

  static void setDefaultStorageType(StorageType s) ;

  static StorageType getDefaultStorageType();

protected:

  static StorageType defaultStorageType ;

  Double_t corrcov(RooRealVar &x,RooRealVar &y, const char* cutSpec, const char* cutRange, Bool_t corr) const  ;
  TMatrixDSym* corrcovMatrix(const RooArgList& vars, const char* cutSpec, const char* cutRange, Bool_t corr) const  ;

  virtual void optimizeReadingWithCaching(RooAbsArg& arg, const RooArgSet& cacheList, const RooArgSet& keepObsList) ;
  Bool_t allClientsCached(RooAbsArg*, const RooArgSet&) ;


 // PlotOn implementation
  virtual RooPlot *plotOn(RooPlot *frame, PlotOpt o) const ;
  virtual RooPlot *plotAsymOn(RooPlot* frame, const RooAbsCategoryLValue& asymCat, PlotOpt o) const ;
  virtual RooPlot *plotEffOn(RooPlot* frame, const RooAbsCategoryLValue& effCat, PlotOpt o) const ;
 
 
  // Constant term optimizer interface
  friend class RooAbsArg ;
  friend class RooAbsReal ;
  friend class RooAbsOptTestStatistic ;
  friend class NewRooAbsOptTestStatistic ;
  friend class RooAbsCachedPdf ;

  virtual void cacheArgs(const RooAbsArg* owner, RooArgSet& varSet, const RooArgSet* nset=0, Bool_t skipZeroWeights=kFALSE) ;
  virtual void resetCache() ;
  virtual void setArgStatus(const RooArgSet& set, Bool_t active) ;
  virtual void attachCache(const RooAbsArg* newOwner, const RooArgSet& cachedVars) ;

  virtual NewRooAbsData* cacheClone(const RooAbsArg* newCacheOwner, const RooArgSet* newCacheVars, const char* newName=0) = 0 ; // DERIVED
  virtual NewRooAbsData* reduceEng(const RooArgSet& varSubset, const RooFormulaVar* cutVar, const char* cutRange=0, 
	                        Int_t nStart=0, Int_t nStop=2000000000, Bool_t copyCache=kTRUE) = 0 ; // DERIVED

  RooRealVar* dataRealVar(const char* methodname, RooRealVar& extVar) const ;

  // Column structure definition
  RooArgSet _vars;         // Dimensions of this data set
  RooArgSet _cachedVars ;  //! External variables cached with this data set

  TIterator *_iterator;    //! Iterator over dimension variables
  TIterator *_cacheIter ;  //! Iterator over cached variables

  RooAbsDataStore* _dstore ; // Data storage implementation

  std::map<std::string,NewRooAbsData*> _ownedComponents ; // Owned external components

private:

   ClassDef(NewRooAbsData,1)//Derived class of NewRooAbsData modified to include NewRooAbsOptTestStatistic as a friend class
};

#endif
