//Created by KVClassFactory on Wed Sep  9 10:51:47 2015
//Author: fable

#ifndef __NEWROOADDPDF_H
#define __NEWROOADDPDF_H

#include "RooFit.h"
#include "RooAddPdf.h"
#include "RooAddPdf.h"
#include "RooAbsReal.h"
#include "RooArgList.h"

//#include "NewRooFitResult.h"
//#include "NewRooAbsReal.h"

using namespace RooFit;

class NewRooAddPdf : public RooAddPdf
{

public:
   NewRooAddPdf();
   NewRooAddPdf(const char *name, const char *title=0);
   NewRooAddPdf(const char *name, const char *title, RooAbsPdf& pdf1, RooAbsPdf& pdf2, RooAbsReal& coef1) ;
   NewRooAddPdf(const char *name, const char *title, const RooArgList& pdfList) ;
   NewRooAddPdf(const char *name, const char *title, const RooArgList& pdfList, const RooArgList& coefList, Bool_t recursiveFraction=kFALSE) ;
  
   NewRooAddPdf(const NewRooAddPdf& other, const char* name=0) ;
   
   virtual NewRooAddPdf* clone(const char* newname) const { return new NewRooAddPdf(*this,newname) ; }
   
   virtual RooAbsReal* createNLL(RooAbsData& data, const RooLinkedList& cmdList) ;
   virtual RooAbsReal* createNLL(RooAbsData& data, const RooCmdArg& arg1=RooCmdArg::none(),  const RooCmdArg& arg2=RooCmdArg::none(),  
				const RooCmdArg& arg3=RooCmdArg::none(),  const RooCmdArg& arg4=RooCmdArg::none(), const RooCmdArg& arg5=RooCmdArg::none(),  
				const RooCmdArg& arg6=RooCmdArg::none(),  const RooCmdArg& arg7=RooCmdArg::none(), const RooCmdArg& arg8=RooCmdArg::none()) ;
   
   Bool_t addOwnedComponents(const RooArgSet& comps, Bool_t silent=kTRUE);
   
   virtual ~NewRooAddPdf();
   
   
   RooFitResult* improvedFitTo(RooDataHist& data, const RooCmdArg& arg1 = RooCmdArg::none(), const RooCmdArg& arg2 = RooCmdArg::none(), const RooCmdArg& arg3 = RooCmdArg::none(), const RooCmdArg& arg4 = RooCmdArg::none(),
                                                  const RooCmdArg& arg5 = RooCmdArg::none(), const RooCmdArg& arg6 = RooCmdArg::none(), const RooCmdArg& arg7 = RooCmdArg::none(), const RooCmdArg& arg8 = RooCmdArg::none(),
						  const RooCmdArg& arg9 = RooCmdArg::none(), const RooCmdArg& sarg10 = RooCmdArg::none(), const RooCmdArg& arg11 = RooCmdArg::none(), const RooCmdArg& arg12 = RooCmdArg::none());
						  
   RooFitResult* improvedFitTo(RooDataHist& data, const RooLinkedList& cmdList);
		   
   

   ClassDef(NewRooAddPdf,1)//Modified RooFit class NewRooAddPdf with new fit options
};

#endif
