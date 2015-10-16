//Created by KVClassFactory on Wed Sep  9 10:55:22 2015
//Author: fable

#ifndef __ROOFIT_NOROOMINIMIZER

#ifndef __NEWROOMINIMIZERFCN_H
#define __NEWROOMINIMIZERFCN_H

#include "Math/IFunction.h"
#include "Fit/ParameterSettings.h"
#include "Fit/FitResult.h"
#include "TMatrixDSym.h"
#include "RooAbsReal.h"
#include "RooArgList.h"
#include <iostream>
#include <fstream>

class NewRooMinimizer;

class NewRooMinimizerFcn : public ROOT::Math::IBaseFunctionMultiDim
{

 public:
 
  NewRooMinimizerFcn(RooAbsReal *funct, NewRooMinimizer *context, 
	       bool verbose = false);
  NewRooMinimizerFcn(const NewRooMinimizerFcn& other);
  virtual ~NewRooMinimizerFcn();

  virtual ROOT::Math::IBaseFunctionMultiDim* Clone() const;
  virtual unsigned int NDim() const { return _nDim; }

  RooArgList* GetFloatParamList() { return _floatParamList; }
  RooArgList* GetConstParamList() { return _constParamList; }
  RooArgList* GetInitFloatParamList() { return _initFloatParamList; }
  RooArgList* GetInitConstParamList() { return _initConstParamList; }

  void SetEvalErrorWall(Bool_t flag) { _doEvalErrorWall = flag ; }
  void SetPrintEvalErrors(Int_t numEvalErrors) { _printEvalErrors = numEvalErrors ; }
  Bool_t SetLogFile(const char* inLogfile);
  std::ofstream* GetLogFile() { return _logfile; }
  void SetVerbose(Bool_t flag=kTRUE) { _verbose = flag ; }

  Double_t& GetMaxFCN() { return _maxFCN; }
  Int_t GetNumInvalidNLL() { return _numBadNLL; }

  Bool_t Synchronize(std::vector<ROOT::Fit::ParameterSettings>& parameters, 
		     Bool_t optConst, Bool_t verbose);
  void BackProp(const ROOT::Fit::FitResult &results);  
  void ApplyCovarianceMatrix(TMatrixDSym& V); 

  Int_t evalCounter() const { return _evalCounter ; }
  void zeroEvalCount() { _evalCounter = 0 ; }
  
 
 protected:
 
  Double_t GetPdfParamVal(Int_t index);
  Double_t GetPdfParamErr(Int_t index);
  void SetPdfParamErr(Int_t index, Double_t value);
  void ClearPdfParamAsymErr(Int_t index);
  void SetPdfParamErr(Int_t index, Double_t loVal, Double_t hiVal);

  inline Bool_t SetPdfParamVal(const Int_t &index, const Double_t &value) const;


  virtual double DoEval(const double * x) const;  
  void updateFloatVec() ;  
  
  mutable Int_t _evalCounter ;
  
  RooAbsReal *_funct;
  NewRooMinimizer *_context;

  mutable double _maxFCN;
  mutable int _numBadNLL;
  mutable int _printEvalErrors;
  Bool_t _doEvalErrorWall;

  int _nDim;
  std::ofstream *_logfile;
  bool _verbose;

  RooArgList* _floatParamList;
  std::vector<RooAbsArg*> _floatParamVec ;
  RooArgList* _constParamList;
  RooArgList* _initFloatParamList;
  RooArgList* _initConstParamList;
  
  

 ClassDef(NewRooMinimizerFcn,1)//Modified Roofit class RooMinimizerFcn in order to use it with NewRooMinimizer
};

#endif

#endif
