//Created by KVClassFactory on Fri Sep 11 16:26:52 2015
//Author: fable

#ifndef __NEWROOFITRESULT_H
#define __NEWROOFITRESULT_H

#include "Riosfwd.h"
#include "RooAbsArg.h"
#include "RooPrintable.h"
#include "RooDirItem.h"
#include "RooArgList.h"
#include "RVersion.h"
#include "TMatrixFfwd.h"
#include "TMatrixDSym.h"
#include "TRootIOCtor.h"
#include "RooFitResult.h"
#include <vector>
#include <string>
#include <map>

class RooArgSet ;
class RooAbsPdf ;
class RooPlot;
class TObject ;
class TH2 ;
typedef RooArgSet* pRooArgSet ;


class NewRooFitResult : public RooFitResult
{
public:
 
  // Constructors, assignment etc.
  NewRooFitResult(const char* name=0, const char* title=0) ;
  NewRooFitResult(const NewRooFitResult& other) ;   			     
  virtual TObject* Clone(const char* newname = 0) const { 
    NewRooFitResult* r =  new NewRooFitResult(*this) ; 
    if (newname && *newname) r->SetName(newname) ; 
    return r ; 
  }
  virtual TObject* clone() const { return new NewRooFitResult(*this); }   
  virtual ~NewRooFitResult() ;

  static NewRooFitResult* lastMinuitFit(const RooArgList& varList=RooArgList()) ;

  Bool_t isIdentical(const NewRooFitResult& other, Double_t tol=5e-5, Double_t tolCorr=1e-4, Bool_t verbose=kTRUE) const ;

protected:
  
  friend class NewRooMinuit ;
  friend class NewRooMinimizer ;


   ClassDef(NewRooFitResult,1)//Modified Roofit class NewRooFitResult in order to use it with NewRooMinimizer
};

#endif
