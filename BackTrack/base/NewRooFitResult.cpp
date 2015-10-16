//Created by KVClassFactory on Fri Sep 11 16:26:52 2015
//Author: fable

#include "RooFit.h"
#include "Riostream.h"

#include <iomanip>
#include "TMinuit.h"
#include "TMath.h"
#include "TMarker.h"
#include "TLine.h"
#include "TBox.h"
#include "TGaxis.h"
#include "TMatrix.h"
#include "TVector.h"
#include "TDirectory.h"
#include "TClass.h"
#include "NewRooFitResult.h"
#include "RooArgSet.h"
#include "RooArgList.h"
#include "RooRealVar.h"
#include "RooPlot.h"
#include "RooEllipse.h"
#include "RooRandom.h"
#include "RooMsgService.h"
#include "TH2D.h"
#include "TText.h"
#include "TMatrixDSym.h"
#include "RooMultiVarGaussian.h"
#include "TNamed.h"
#include "RooPrintable.h"
#include "RooDirItem.h"

using namespace std;

ClassImp(NewRooFitResult)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>NewRooFitResult</h2>
<h4>Modified Roofit class NewRooFitResult in order to use it with NewRooMinimizer</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

//_____________________________________________________________________________
NewRooFitResult::NewRooFitResult(const char* name, const char* title) : RooFitResult(name, title)
{  
  // Constructor with name and title
  // coverity[UNINIT_CTOR]
}


//_____________________________________________________________________________
NewRooFitResult::NewRooFitResult(const NewRooFitResult& other) : RooFitResult(other)
{
  // Copy constructor
}



//_____________________________________________________________________________
NewRooFitResult::~NewRooFitResult() 
{
  // Destructor
}






//_____________________________________________________________________________
NewRooFitResult* NewRooFitResult::lastMinuitFit(const RooArgList& varList) 
{
  // Import the results of the last fit performed by gMinuit, interpreting
  // the fit parameters as the given varList of parameters.

  // Verify length of supplied varList
  if (varList.getSize()>0 && varList.getSize()!=gMinuit->fNu) {
    oocoutE((TObject*)0,InputArguments) << "NewRooFitResult::lastMinuitFit: ERROR: supplied variable list must be either empty " << endl 
					<< "                             or match the number of variables of the last fit (" << gMinuit->fNu << ")" << endl ;
    return 0 ;
  }

  // Verify that all members of varList are of type RooRealVar
  TIterator* iter = varList.createIterator() ;
  RooAbsArg* arg  ;
  while((arg=(RooAbsArg*)iter->Next())) {
    if (!dynamic_cast<RooRealVar*>(arg)) {
      oocoutE((TObject*)0,InputArguments) << "NewRooFitResult::lastMinuitFit: ERROR: variable '" << arg->GetName() << "' is not of type RooRealVar" << endl ;
      return 0 ;
    }
  }
  delete iter ;

  NewRooFitResult* r = new NewRooFitResult("lastMinuitFit","Last MINUIT fit") ;

  // Extract names of fit parameters from MINUIT 
  // and construct corresponding RooRealVars
  RooArgList constPars("constPars") ;
  RooArgList floatPars("floatPars") ;

  Int_t i ;
  for (i = 1; i <= gMinuit->fNu; ++i) {
    if (gMinuit->fNvarl[i-1] < 0) continue;
    Int_t l = gMinuit->fNiofex[i-1];
    TString varName(gMinuit->fCpnam[i-1]) ;
    Bool_t isConst(l==0) ;
    
    Double_t xlo = gMinuit->fAlim[i-1];
    Double_t xhi = gMinuit->fBlim[i-1];
    Double_t xerr = gMinuit->fWerr[l-1];
    Double_t xval = gMinuit->fU[i-1] ;

    RooRealVar* var ;
    if (varList.getSize()==0) {

      if ((xlo<xhi) && !isConst) {
	var = new RooRealVar(varName,varName,xval,xlo,xhi) ;
      } else {
	var = new RooRealVar(varName,varName,xval) ;
      }
      var->setConstant(isConst) ;
    } else {

      var = (RooRealVar*) varList.at(i-1)->Clone() ;
      var->setConstant(isConst) ;
      var->setVal(xval) ;
      if (xlo<xhi) {
	var->setRange(xlo,xhi) ;
      }
      if (varName.CompareTo(var->GetName())) {
	oocoutI((TObject*)0,Eval) << "NewRooFitResult::lastMinuitFit: fit parameter '" << varName 
				  << "' stored in variable '" << var->GetName() << "'" << endl ;
      }

    }

    if (isConst) {
      constPars.addOwned(*var) ;
    } else {
      var->setError(xerr) ;
      floatPars.addOwned(*var) ;
    }
  }

  Int_t icode,npari,nparx ;
  Double_t fmin,edm,errdef ;
  gMinuit->mnstat(fmin,edm,errdef,npari,nparx,icode) ;
  
  r->setConstParList(constPars) ;
  r->setInitParList(floatPars) ;
  r->setFinalParList(floatPars) ;
  r->setMinNLL(fmin) ;
  r->setEDM(edm) ; 
  r->setCovQual(icode) ;
  r->setStatus(gMinuit->fStatus) ;
  r->fillCorrMatrix() ;

  return r ;
}



//_____________________________________________________________________________
Bool_t NewRooFitResult::isIdentical(const NewRooFitResult& other, Double_t tol, Double_t tolCorr, Bool_t /*verbose*/) const 
{
  // Return true if this fit result is identical to other within tolerance 'tol' on fitted values
  // and tolerance 'tolCor' on correlation coefficients

  Bool_t ret = kTRUE ;

  if (fabs(_minNLL-other._minNLL)>=tol) {
    cout << "NewRooFitResult::isIdentical: minimized value of -log(L) is different " << _minNLL << " vs. " << other._minNLL << endl ;
    ret = kFALSE ;
  }

  for (Int_t i=0 ; i<_constPars->getSize() ; i++) {
    RooAbsReal* ov = static_cast<RooAbsReal*>(other._constPars->find(_constPars->at(i)->GetName())) ;
    if (!ov) {
      cout << "NewRooFitResult::isIdentical: cannot find constant parameter " << _constPars->at(i)->GetName() << " in reference" << endl ;
      ret = kFALSE ;
    }
    if (ov && fabs(static_cast<RooAbsReal*>(_constPars->at(i))->getVal()-ov->getVal())>=tol) {
      cout << "NewRooFitResult::isIdentical: constant parameter " << _constPars->at(i)->GetName() 
	   << " differs in value: " << static_cast<RooAbsReal*>(_constPars->at(i))->getVal() << " vs. " << ov->getVal() << endl ;
      ret = kFALSE ;
    }
  }

  for (Int_t i=0 ; i<_initPars->getSize() ; i++) {
    RooAbsReal* ov = static_cast<RooAbsReal*>(other._initPars->find(_initPars->at(i)->GetName())) ;
    if (!ov) {
      cout << "NewRooFitResult::isIdentical: cannot find initial parameter " << _initPars->at(i)->GetName() << " in reference" << endl ;
      ret = kFALSE ;
    }
    if (ov && fabs(static_cast<RooAbsReal*>(_initPars->at(i))->getVal()-ov->getVal())>=tol) {
      cout << "NewRooFitResult::isIdentical: initial parameter " << _initPars->at(i)->GetName() 
	   << " differs in value: " << static_cast<RooAbsReal*>(_initPars->at(i))->getVal() << " vs. " << ov->getVal() << endl ;
      ret = kFALSE ;
    }
  }

  for (Int_t i=0 ; i<_finalPars->getSize() ; i++) {
    RooAbsReal* ov = static_cast<RooAbsReal*>(other._finalPars->find(_finalPars->at(i)->GetName())) ;
    if (!ov) {
      cout << "NewRooFitResult::isIdentical: cannot find final parameter " << _finalPars->at(i)->GetName() << " in reference" << endl ;
      ret = kFALSE ;
    }
    if (ov && fabs(static_cast<RooAbsReal*>(_finalPars->at(i))->getVal()-ov->getVal())>=tol) {
      cout << "NewRooFitResult::isIdentical: final parameter " << _finalPars->at(i)->GetName() 
	   << " differs in value: " << static_cast<RooAbsReal*>(_finalPars->at(i))->getVal() << " vs. " << ov->getVal() << endl ;
      ret = kFALSE ;
    }
  }

  // Only examine correlations for cases with >1 floating parameter
  if (_finalPars->getSize()>1) {
    
    fillLegacyCorrMatrix() ;
    other.fillLegacyCorrMatrix() ;
    
    for (Int_t i=0 ; i<_globalCorr->getSize() ; i++) {
      RooAbsReal* ov = static_cast<RooAbsReal*>(other._globalCorr->find(_globalCorr->at(i)->GetName())) ;
      if (!ov) {
	cout << "NewRooFitResult::isIdentical: cannot find global correlation coefficient " << _globalCorr->at(i)->GetName() << " in reference" << endl ;
	ret = kFALSE ;
      }
      if (ov && fabs(static_cast<RooAbsReal*>(_globalCorr->at(i))->getVal()-ov->getVal())>=tolCorr) {
	cout << "NewRooFitResult::isIdentical: global correlation coefficient " << _globalCorr->at(i)->GetName() 
	     << " differs in value: " << static_cast<RooAbsReal*>(_globalCorr->at(i))->getVal() << " vs. " << ov->getVal() << endl ;
	ret = kFALSE ;
      }
    }
    
    for (Int_t j=0 ; j<_corrMatrix.GetSize() ; j++) {
      RooArgList* row = (RooArgList*) _corrMatrix.At(j) ;
      RooArgList* orow = (RooArgList*) other._corrMatrix.At(j) ;
      for (Int_t i=0 ; i<row->getSize() ; i++) {
	RooAbsReal* ov = static_cast<RooAbsReal*>(orow->find(row->at(i)->GetName())) ;
	if (!ov) {
	  cout << "NewRooFitResult::isIdentical: cannot find correlation coefficient " << row->at(i)->GetName() << " in reference" << endl ;
	  ret = kFALSE ;
	}
	if (ov && fabs(static_cast<RooAbsReal*>(row->at(i))->getVal()-ov->getVal())>=tolCorr) {
	  cout << "NewRooFitResult::isIdentical: correlation coefficient " << row->at(i)->GetName() 
	       << " differs in value: " << static_cast<RooAbsReal*>(row->at(i))->getVal() << " vs. " << ov->getVal() << endl ;
	  ret = kFALSE ;
	}
      }
    }
  }    

  return ret ;
}

