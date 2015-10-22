/*
 * GenericModel_Binned.h
 *
 *  Created on: Mar 24, 2015
 *      Author: john, quentin
 */

#ifndef __GENERICMODEL_BINNED_H_
#define __GENERICMODEL_BINNED_H_

#include "KVBase.h"
#include "RooAbsReal.h"
#include "RooArgList.h"
#include "RooRealVar.h"
#include "RooDataSet.h"
#include "RooDataSet.h"
#include "RooGaussian.h"
#include "RooConstVar.h"
#include "RooPolynomial.h"
#include "RooKeysPdf.h"
#include "RooNDKeysPdf.h"
#include "RooProdPdf.h"
#include "RooPlot.h"
#include "NewRooAddPdf.h"
#include "RooFitResult.h"
#include "RooHistPdf.h"
#include "RooChi2Var.h"
#include "NewRooMinuit.h"
#include "RooExtendPdf.h"
#include "TFile.h"
#include "RooWorkspace.h"
#include "KVNumberList.h"
#include "TH2.h"

using namespace RooFit ;

namespace BackTrack {

   class GenericModel_Binned : public KVBase
    {
      ClassDef(GenericModel_Binned,1)//Generic model for backtracing studies
      

    protected:
      
      RooDataHist*  fDataHist;            // experimental RooDataHist to fit
      RooWorkspace* fWorkspace;           // workspace for the fit
      char* fwk_name;                     // name of the initial saved workspace  
       
      Int_t   fexpdatahist_counts;        // number of counts in the experimental RooDataHist for an extended fit       
      Int_t   fNpdf;	                  // internal counter of model RooHistPdf used for the fit  
      Int_t   fSmoothing;	          // histpdf smoothing factor 
      Bool_t  fBool_expdatahist_set;      // verify if the experimental RooDataHist was given
      Bool_t  fBool_extended;	          // extended/not extended fit
      Bool_t  fBool_uniform;              // uniform/not uniform guess for parameters 
      Bool_t  fBool_numint;               // to force integrals to be calculated numerically for the pseudo-PDF
      Bool_t  fBool_provided;             // to control the ImportModelData()
      Bool_t  fBool_saved;	          // to know if workspace is indeed saved with the SaveWorkspace() method    
        
      RooArgList        fWeights;              // fitted weight of each pseudo-pdf in result
      NewRooAddPdf*     fModelPseudoPDF;       // pseudo-pdf for model to fit data
      RooArgList        fFractions;            // weights of each kernel in pseudo-pdf (only used for pseudo-pdf construction)
      vector<Double_t>* fInitWeights;          // initital weights 
      RooFitResult*     fLastFit;              // result of last fit
      RooHistPdf*       fParameterPDF;         // pdf for parameters after fit to data
      RooDataHist*      fParamDataHist;        // binned parameter dataset used to construct fParameterPDF   
      TObjArray         fHistPdfs;             // pseudo-pdfs for each dataset
      KVNumberList*     fRefusedDataSets;      // to list the refused RooHistPdf numbers (if the corresponding RooDataHist has no events
         
	 
       
      // Internal checking  
      Bool_t IsExpDataHistSet() const {return fBool_expdatahist_set; }   
      Bool_t IsExtended() const { return fBool_extended; }
      Bool_t IsUniformInitWeight() const { return fBool_uniform; } 
      Bool_t IsNumInt() const { return fBool_numint; }
      Bool_t VerifyWorkspace(Bool_t debug=kFALSE);		   
      Bool_t IsWorkspaceImported() const { return fBool_provided; }
      Bool_t IsWorkspaceSaved() const { return fBool_saved; } 
      
      // For uniform guess: will take into account the experimental number of events for an extended fit
      void CreateUniformInitWeight(Double_t exp_counts); 
      
      // Save
      void CreateWorkspaceSaving(char *file);
//      void SavePseudoPDF(char* file); 

      // For data importation
      void ImportModelData(Int_t parameter_num = 0, RooArgList* plist = 0);     
      void ImportParamInitWeight(Int_t parameter_num = 0, RooArgList* plist = 0);      
	    
	                
    public:
      
      GenericModel_Binned();
      virtual ~GenericModel_Binned();
      
      // Fit caracteristics
      void SetExperimentalDataHist(RooDataHist& data);
      const RooDataHist* GetExperimentalDataHist() const { return fDataHist; }
      Int_t GetExpectedCounts() { return fexpdatahist_counts; }
      void SaveInitWorkspace(char* file=0);
      char* GetInitWorkspaceFileName() { return fwk_name; }   
      void SetExtended(Bool_t extended=kFALSE);
      void SetNumInt(Bool_t numint=kFALSE);      
      void InitWorkspace(Bool_t debug=kFALSE); 
      void ImportWorkspace(RooWorkspace* w, Bool_t debug=kTRUE);
              
       
      // Add parameters/Observables
      void AddParameter(const char* name, const char* title, Double_t min, Double_t max, Int_t nbins);
      void AddParameter(const RooRealVar&, Int_t nbins);
      void AddObservable(const char* name, const char* title, Double_t min, Double_t max, Int_t nbins);
      void AddObservable(const RooRealVar&, Int_t nbins);
      RooRealVar* GetParameter(const char* name) { return dynamic_cast<RooRealVar*> (GetParameters().find(name)); }
      RooRealVar* GetParameter(int num) { return dynamic_cast<RooRealVar*> (GetParameters().at(num)); }	   
      RooRealVar* GetObservable(const char* name) { return  dynamic_cast<RooRealVar*> (GetObservables().find(name)); }
      RooRealVar* GetObservable(int num) { return dynamic_cast<RooRealVar*> (GetObservables().at(num)); } 
      Int_t GetNumberOfParameters();
      Int_t GetNumberOfObservables();	  
      const RooArgList& GetParameters();
      const RooArgList& GetObservables();
      const RooArgList& GetParObs();
      
      // Model data      
      void AddModelData(RooArgList& params, RooDataHist* data);
      virtual RooDataHist* GetModelDataHist(RooArgList& par);
      void ImportAllModelData();
      
      // Initial guess/weights for the fit
      void AddParamInitWeight(RooArgList& params, Double_t weight);
      virtual Double_t GetParamInitWeight(RooArgList& par);
      void ImportAllParamInitWeight(Bool_t uniform=kFALSE);
      
            
      Int_t GetNumberOfDataSets();
      TObjArray* GetDataSetParametersList();
      RooArgList* GetParametersForDataset(Int_t i);

      const TObjArray*   GetDataHistsList();
      const RooDataHist* GetDataHist(Int_t i);
      
      const TObjArray* GetKernelsList();
      RooHistPdf* GetKernel(Int_t i);


      // PseudoPDF and Fit
      void  ConstructPseudoPDF(Bool_t debug=kFALSE);   
      const NewRooAddPdf* GetPseudoPDF() const { return fModelPseudoPDF; }
      const RooArgList& GetPseudoPDFFractions() const { return fFractions; }
      const RooHistPdf* GetParameterPDF() const { return fParameterPDF; }
      const RooDataHist* GetParamDataHist() const { return fParamDataHist; }
      const RooArgList& GetWeights() const { return fWeights; }
      const KVNumberList* GetRefusedSetsList() const { return fRefusedDataSets; }
      
      RooFitResult* fitTo(RooDataHist& data, const RooCmdArg& arg1 = RooCmdArg::none(), const RooCmdArg& arg2 = RooCmdArg::none(), const RooCmdArg& arg3 = RooCmdArg::none(), const RooCmdArg& arg4 = RooCmdArg::none(),
                                                const RooCmdArg& arg5 = RooCmdArg::none(), const RooCmdArg& arg6 = RooCmdArg::none(), const RooCmdArg& arg7 = RooCmdArg::none(), const RooCmdArg& arg8 = RooCmdArg::none(),
						const RooCmdArg& arg9 = RooCmdArg::none(), const RooCmdArg& sarg10 = RooCmdArg::none(), const RooCmdArg& arg11 = RooCmdArg::none(), const RooCmdArg& arg12 = RooCmdArg::none()); 
      
      RooFitResult* chi2FitTo(RooDataHist& data, const RooCmdArg& arg1 = RooCmdArg::none(), const RooCmdArg& arg2 = RooCmdArg::none(), const RooCmdArg& arg3 = RooCmdArg::none(), const RooCmdArg& arg4 = RooCmdArg::none(),
                                                const RooCmdArg& arg5 = RooCmdArg::none(), const RooCmdArg& arg6 = RooCmdArg::none(), const RooCmdArg& arg7 = RooCmdArg::none(), const RooCmdArg& arg8 = RooCmdArg::none());						
						     						 
      RooFitResult* GetLastFit() const { return fLastFit; }
      
      void plotOn(RooPlot*);
   };

} /* namespace BackTrack */

#endif /* GENERICMODEL_BINNED_H_ */
