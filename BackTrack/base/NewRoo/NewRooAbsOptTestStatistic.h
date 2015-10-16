//Created by KVClassFactory on Wed Sep 16 14:13:58 2015
//Author: fable

#ifndef __NEWROOABSTESTSTATISTIC_H
#define __NEWROOABSTESTSTATISTIC_H

#include "RooAbsTestStatistic.h"
#include "Riosfwd.h"
#include "RooAbsTestStatistic.h"
#include "RooSetProxy.h"
#include "RooCategoryProxy.h"
#include "TString.h"

class RooArgSet ;
class RooAbsData ;
class NewRooAbsData ;
class RooAbsReal ;


class NewRooAbsOptTestStatistic : public RooAbsTestStatistic
{
public:

  // Constructors, assignment etc
  NewRooAbsOptTestStatistic() ;
  NewRooAbsOptTestStatistic(const char *name, const char *title, RooAbsReal& real, RooAbsData& data,
			 const RooArgSet& projDeps, const char* rangeName=0, const char* addCoefRangeName=0,
			 Int_t nCPU=1, RooFit::MPSplit interleave=RooFit::BulkPartition, Bool_t verbose=kTRUE, Bool_t splitCutRange=kFALSE,
			 Bool_t cloneInputData=kTRUE) ;
  NewRooAbsOptTestStatistic(const NewRooAbsOptTestStatistic& other, const char* name=0);
  virtual ~NewRooAbsOptTestStatistic();

  virtual Double_t combinedValue(RooAbsReal** gofArray, Int_t nVal) const ;

  RooAbsReal& function() { return *_funcClone ; }
  const RooAbsReal& function() const { return *_funcClone ; }

  RooAbsData& data() ;
  const RooAbsData& data() const ;


  virtual const char* cacheUniqueSuffix() const { return Form("_%lx", (ULong_t)_dataClone) ; }

  // Override this to be always true to force calculation of likelihood without parameters
  virtual Bool_t isDerived() const { return kTRUE ; }

  void seal(const char* notice="") { _sealed = kTRUE ; _sealNotice = notice ; }
  Bool_t isSealed() const { return _sealed ; }
  const char* sealNotice() const { return _sealNotice.Data() ; }

  // Added for silent mode
  void enableSilentOffsetting(Bool_t flag); 

protected:

  Bool_t setDataSlave(RooAbsData& data, Bool_t cloneData=kTRUE, Bool_t ownNewDataAnyway=kFALSE) ;
  
  //Added
  void initSlave(RooAbsReal& real, RooAbsData& indata, const RooArgSet& projDeps, const char* rangeName, 
		 const char* addCoefRangeName)  ;

  void initSlave(RooAbsReal& real, NewRooAbsData& indata, const RooArgSet& projDeps, const char* rangeName, 
		 const char* addCoefRangeName)  ;

  friend class RooAbsReal ;

  virtual Bool_t allowFunctionCache() { return kTRUE ;  }
  void constOptimizeTestStatistic(ConstOpCode opcode, Bool_t doAlsoTrackingOpt=kTRUE) ;

  virtual Bool_t redirectServersHook(const RooAbsCollection& newServerList, Bool_t mustReplaceAll, Bool_t nameChange, Bool_t isRecursive) ;
  virtual void printCompactTreeHook(std::ostream& os, const char* indent="") ;
  virtual RooArgSet requiredExtraObservables() const { return RooArgSet() ; }
  void optimizeCaching() ;
  void optimizeConstantTerms(Bool_t,Bool_t=kTRUE) ;

  //Added for silent mode
  Bool_t initializeSilent();
  void initMPModeSilent(RooAbsReal* real, RooAbsData* data, const RooArgSet* projDeps, const char* rangeName, const char* addCoefRangeName);

  RooArgSet*  _normSet ; // Pointer to set with observables used for normalization
  RooArgSet*  _funcCloneSet ; // Set owning all components of internal clone of input function
  //RooAbsData* _dataClone ; // Pointer to internal clone if input data
  NewRooAbsData* _dataClone ;
  RooAbsReal* _funcClone ; // Pointer to internal clone of input function
  RooArgSet*  _projDeps ; // Set of projected observable
  Bool_t      _ownData  ; // Do we own the dataset
  Bool_t      _sealed ; // Is test statistic sealed -- i.e. no access to data 
  TString     _sealNotice ; // User-defined notice shown when reading a sealed likelihood 
  RooArgSet*  _funcObsSet ; // List of observables in the pdf expression
  RooArgSet   _cachedNodes ; //! List of nodes that are cached as constant expressions
  
  RooAbsReal* _origFunc ; // Original function 
  RooAbsData* _origData ; // Original data 
  Bool_t      _optimized ; //!


ClassDef(NewRooAbsOptTestStatistic,1)// Derived class of RooAbsTestStatistic, inspired of RooAbsOptTestStatistic, to avoid RooArgSet checkForDup() messages in enableOffsetting method using enableSilentOffsetting()
};

#endif

