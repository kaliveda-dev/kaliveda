/*
 * GenericModel_Binned.cpp
 *
 *  Created on: Mar 24, 2015
 *      Author: john, quentin
 */

#include "GenericModel_Binned.h"

#include "RooFit.h"
#include "RooCmdConfig.h"
#include "Riostream.h"
#include "TMath.h"
#include "TObjString.h"
#include "TPaveText.h"
#include "TList.h"
#include "TH1.h"
#include "TH2.h"
#include "TMatrixD.h"
#include "TMatrixDSym.h"
#include "RooAbsPdf.h"
#include "RooAbsReal.h"
#include "RooDataSet.h"
#include "RooArgSet.h"
#include "RooArgProxy.h"
#include "RooRealProxy.h"
#include "RooRealVar.h"
#include "RooGenContext.h"
#include "RooBinnedGenContext.h"
#include "RooPlot.h"
#include "RooCurve.h"
#include "RooNLLVar.h"
#include "NewRooMinuit.h"
#include "RooCategory.h"
#include "RooNameReg.h"
#include "RooCmdConfig.h"
#include "RooAddition.h"
#include "RooRandom.h"
#include "RooNumIntConfig.h"
#include "RooProjectedPdf.h"
#include "RooInt.h"
#include "RooCustomizer.h"
#include "RooConstraintSum.h"
#include "RooParamBinning.h"
#include "RooNumCdf.h"
#include "NewRooFitResult.h"
#include "RooFitResult.h"
#include "RooNumGenConfig.h"
#include "RooCachedReal.h"
#include "RooXYChi2Var.h"
#include "RooChi2Var.h"
#include "NewRooMinimizer.h"
#include "RooRealIntegral.h"
#include "Math/CholeskyDecomp.h"
#include <RooAbsBinning.h>
#include <RooUniform.h>
#include "KVNumberList.h"

ClassImp(BackTrack::GenericModel_Binned)

namespace BackTrack {

  GenericModel_Binned::GenericModel_Binned()
  {  
    fBool_expdatahist_set   =kFALSE;   
    fBool_extended          =kFALSE;    //By default not extended fit
    fBool_uniform           =kTRUE;     //By default uniform values of guess for the fit parameters
    fBool_numint            =kFALSE;    //By default integrals are not calculated numerically for the pseudo-PDF         
    fBool_saved             =kFALSE;    //By default initial worksapce will not be saved
    fBool_provided          =kFALSE;    //To control if the workspace was provided or not, if it was provided then no need to import the model data    
    
    fInitWeights = new vector<Double_t>();
    fRefusedDataSets = new KVNumberList();
    fwk_name=0;	  
  
    fDataHist           = 0;
    fWorkspace          = 0; 
    fexpdatahist_counts = -1;
    fNpdf               =0;
    fModelPseudoPDF     =0;
    fLastFit            =0;
    fParameterPDF       =0;
    fParamDataHist      =0;
    fSmoothing          =0.;
  }

  GenericModel_Binned::~GenericModel_Binned()
  {
    SafeDelete(fParameterPDF);
    SafeDelete(fParamDataHist);

    if(fModelPseudoPDF)
      {
	delete fModelPseudoPDF;
	fHistPdfs.Delete();
	fModelPseudoPDF=0;
	fFractions.removeAll();
      }      
            
    SafeDelete(fLastFit);
    SafeDelete(fWorkspace);
  }


  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  void GenericModel_Binned::SetExperimentalDataHist(RooDataHist& data)
  {
    //Set the experimental RooDataHist we want to fit (to know the expected number of events for an extended fit)
       
    fDataHist = new RooDataHist(data, "data_hist");
    fexpdatahist_counts   = data.sumEntries();
    fBool_expdatahist_set = kTRUE;
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  void GenericModel_Binned::SetExtended(Bool_t ext)
  {
    //Extended/ Not extended fit
    //By default not extended
    
    fBool_extended = ext;
  }
  
  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////  
  void GenericModel_Binned::SetNumInt(Bool_t numint)   
  {
    //Force the integrals to be calculated numerically for the pseudo-PDF construction and calculations
    //By default the integrals are not calculated numerically
    fBool_numint = numint;
  }  
  
    
  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  void GenericModel_Binned::AddParameter(const char* name, const char* title, Double_t min, Double_t max, Int_t nbins)
  {
    // Define a named parameter for the model, with its associated range
    // of values and the number of 'bins' to be used to scan these values
    {
      RooRealVar p(name,title,min,max);
      p.setBins(nbins);
	
      ((RooArgList*) fWorkspace->obj("_parameters"))->addClone(p);
      ((RooArgList*) fWorkspace->obj("_parobs"))->addClone(p);
    }   	
  }

  void GenericModel_Binned::AddParameter(const RooRealVar& var, Int_t nbins)
  {
    // Define a named parameter for the model, with its associated range
    // of values and the number of 'bins' to be used to scan these values
    ((RooArgList*) fWorkspace->obj("_parameters"))->addClone(var);
    ((RooArgList*) fWorkspace->obj("_parobs"))->addClone(var);
   
    GetParameter(var.GetName())->setBins(nbins);
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  void GenericModel_Binned::AddObservable(const char* name, const char* title, Double_t min, Double_t max, Int_t nbins)
  {
    // Define a named observable for the model, with its associated range of values
    RooRealVar p(name,title,min,max);
    p.setBins(nbins);
    ((RooArgList*) fWorkspace->obj("_observables"))->addClone(p);
    ((RooArgList*) fWorkspace->obj("_parobs"))->addClone(p);
  }

  void GenericModel_Binned::AddObservable(const RooRealVar& var, Int_t nbins)
  {
    // Define a named observable for the model, with its associated range of values
    ((RooArgList*) fWorkspace->obj("_observables"))->addClone(var);
    ((RooArgList*) fWorkspace->obj("_parobs"))->addClone(var);
    
    GetObservable(var.GetName())->setBins(nbins);
  }


  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  const RooArgList& GenericModel_Binned::GetParameters()
  {
    return *((RooArgList*) fWorkspace->obj("_parameters")); 
  }

  const RooArgList& GenericModel_Binned::GetObservables()
  {
    return *((RooArgList*) fWorkspace->obj("_observables")); 
  }

  const RooArgList& GenericModel_Binned::GetParObs() 
  {
    return *((RooArgList*) fWorkspace->obj("_parobs")); 
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  Int_t GenericModel_Binned::GetNumberOfParameters()
  {
    return GetParameters().getSize();
  } 

  Int_t GenericModel_Binned::GetNumberOfObservables()
  {
    return GetObservables().getSize();
  }
  
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  TObjArray* GenericModel_Binned::GetDataSetParametersList()
  {
    return (TObjArray*) fWorkspace->obj("_datasetparams");
  }
  
              
  RooArgList* GenericModel_Binned::GetParametersForDataset(Int_t i)
  {
    return (RooArgList*) GetDataSetParametersList()->At(i); 
  }
  
        
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  const TObjArray* GenericModel_Binned::GetDataHistsList()
  {
    // Return the kernel estimation PDF for the imported dataset
    return (TObjArray*) fWorkspace->obj("_datahistset");
  }
  
  const RooDataHist* GenericModel_Binned::GetDataHist(Int_t i)
  {
    // Return the kernel estimation PDF for the i-th imported dataset
    return (RooDataHist*) GetDataHistsList()->At(i);
  } 
    

  Int_t GenericModel_Binned::GetNumberOfDataSets() 
  {  
    Int_t size = (int) ((TObjArray*) fWorkspace->obj("_datahistset"))->GetEntriesFast();
    return size; 
  }
  
   
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  const TObjArray* GenericModel_Binned::GetKernelsList()
  {
    // Return the kernel estimation PDF for the i-th imported dataset
    return &fHistPdfs;
  }
  
  RooHistPdf* GenericModel_Binned::GetKernel(Int_t i)
  {
    // Return the kernel estimation PDF for the i-th imported dataset
    return (RooHistPdf*) GetKernelsList()->At(i);
  }   
  
  
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  void GenericModel_Binned::AddModelData(RooArgList& params, RooDataHist* data)
  {
    // Add a set of data calculated with the model for the given set of parameter values.
    // Keep a copy of the parameter values for saving the fit results
    if(!data) return;   

    ((TObjArray*) fWorkspace->obj("_datahistset"))->Add(data);
    RooArgList* ral = new RooArgList;
    ral->addClone(params);
    ((TObjArray*) fWorkspace->obj("_datasetparams"))->Add(ral);     
  }


  RooDataHist*GenericModel_Binned::GetModelDataHist(RooArgList&)
  {
    // Generate/fill unbinned dataset corresponding to model having
    // the parameter values in the list.

    AbstractMethod("GetModelDataHist");
    return nullptr;
  }


  /////////////////////////////////////////////////////////////////////////////////////////////////
  void GenericModel_Binned::ImportAllModelData()
  {
    // Import the model data: create the RooDataSets and import the corresponding RooDataHists,
    // according to the defined parameters ranges and binnings.
  
    ImportModelData();
   
    //Saving  
    if(IsWorkspaceSaved()==kTRUE) CreateWorkspaceSaving(GetInitWorkspaceFileName());	 
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////
  void GenericModel_Binned::ImportModelData(Int_t parameter_num, RooArgList* plist)
  {
    // Initial RooWorksapce already given by the user for the fit, no save or importation       
    if(IsWorkspaceImported()==kTRUE)
      {
        Info("ImportModelData", "RooWorspace for fit already provided, no importation needed !!!");		
        return;
      } 
  
    // Import all model datasets corresponding to the defined parameters, ranges & binnings  
    RooArgList PLIST;
    if(plist) PLIST.addClone(*plist);
    RooRealVar* par = GetParameter(parameter_num);
    RooAbsBinning& bins = par->getBinning();
    Int_t N = bins.numBins();
    RooRealVar* par_in_list = (RooRealVar*)PLIST.find(par->GetName());
   
    if(!par_in_list)
      {
	PLIST.addClone(RooRealVar(par->GetName(),par->GetTitle(),0.));
	par_in_list = (RooRealVar*)PLIST.find(par->GetName());
      }
   
    for(int i=0; i<N; i++)
      {
	par_in_list->setMax(bins.binHigh(i));
	par_in_list->setMin(bins.binLow(i));
	par_in_list->setVal(bins.binCenter(i));
	    	
	if((parameter_num+1)<GetNumberOfParameters()) ImportModelData(parameter_num+1,&PLIST);
	    
	else AddModelData(PLIST, GetModelDataHist(PLIST));
      }	                    
  }




  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  void GenericModel_Binned::AddParamInitWeight(RooArgList& params, Double_t weight)
  {
    // Create the initial values of the guess/weight of the each set of fit parameters.  
    fInitWeights->push_back(weight);     
  }


  Double_t GenericModel_Binned::GetParamInitWeight(RooArgList&)
  {
    // Generate/fill the initial weight corresponding to the parameter values in the list.

    AbstractMethod("GetParamInitWeight");
    return -1;
  }
  
  
  
  /////////////////////////////////////////////////////////////////////////////////////////////////
  void GenericModel_Binned::ImportParamInitWeight(Int_t parameter_num, RooArgList* plist)
  {  
    // Import all initial weights for the parameters corresponding to their ranges & binnings.  
    
    RooArgList PLIST;
    if(plist) PLIST.addClone(*plist);
    RooRealVar* par = GetParameter(parameter_num);
    RooAbsBinning& bins = par->getBinning();
    Int_t N = bins.numBins();
    RooRealVar* par_in_list = (RooRealVar*)PLIST.find(par->GetName());
   
    if(!par_in_list)
      {
    	PLIST.addClone(RooRealVar(par->GetName(),par->GetTitle(),0.));
    	par_in_list = (RooRealVar*)PLIST.find(par->GetName());
      }
   
    for(int i=0; i<N; i++)
      {
    	par_in_list->setMax(bins.binHigh(i));
    	par_in_list->setMin(bins.binLow(i));
    	par_in_list->setVal(bins.binCenter(i));
    	    
    	if((parameter_num+1)<GetNumberOfParameters()) ImportParamInitWeight(parameter_num+1,&PLIST);
    	
    	else AddParamInitWeight(PLIST, GetParamInitWeight(PLIST));
      } 			     
  } 
  
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////   
  void GenericModel_Binned::ImportAllParamInitWeight(Bool_t uniform)
  {
    // Initialize the parameters weights,
    // if uniform=kTRUE  -> will call the default uniform weight initialization (CreateUniformInitWeight())
    // if uniform=kFALSE -> wil call the ImportParamInitWeight() method
    
    fBool_uniform=uniform;
    
    if(IsUniformInitWeight()==kTRUE)
      {
        Info("ImportAllParamInitWeight", ".. setting uniform weights...");
        CreateUniformInitWeight(GetExpectedCounts());
      } 
      
    else
      {
      	Info("ImportAllParamInitWeight", ".. setting user weights...");
	ImportParamInitWeight();
      } 
      
    //debug
    printf("fIniWeight size = %d\n", (int) fInitWeights->size());
    for(std::vector<Double_t>::const_iterator i=fInitWeights->begin(); i != fInitWeights->end(); i++) cout << *i << endl;
     
  }
      

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////  
  void GenericModel_Binned::CreateUniformInitWeight(Double_t ncounts)
  {
    if(ncounts<=0)
      {
	Error("CreateUniformInitWeights", "...Warning empty experimental RooDataHist...");
	return;
      }

    Double_t integral=0;	
	     
    if(IsExtended()==kTRUE) integral=ncounts;   
    else integral=1.;

    //Loop on parameters  
    Double_t ww = integral/GetNumberOfDataSets(); 
                 
    for(Int_t ii=0; ii<GetNumberOfDataSets(); ii++) 
      {
        fInitWeights->push_back(ww);
      } 
  }
  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////  
  void GenericModel_Binned::ConstructPseudoPDF(Bool_t debug)
  {
    //Build a parameterised pseudo-PDF from all the imported model datasets (RooDataHist).
    //Each dataset will be transformed into a RooHistPdf.  
    //The user can decide to initialize the parameters values using the ImportAllParamInitWeight() method.
    //For an extended fit the weights will be created using GetExpectedCounts() method.
    
   
    //-------------Control if the user gave the experimental RooDataHist to fit------------
    if(IsExpDataHistSet()==kFALSE)
      {
        Error("ConstructPseudoPDF", "... !!! No Experimental RooDataHist for the fit !!! Import experimental RooDataHist to fit with SetExperimentalDataHist() method first...");
	return;
      }  	
    
    //-------------Control if DataSets list is given------------
    if(!GetNumberOfDataSets() || GetNumberOfDataSets()==0)
      {
	if(IsWorkspaceImported()==kFALSE) Error("ConstructPseudoPDF", "... !!! No DataSets !!! Import model datasets with ImportModelData() method first...");
	else                              Error("ConstructPseudoPDF", "... !!! No DataSets !!! Verify provided worspace...");
	
	return;
      }    
       
    //-------------For the weights initialization--------------
    Int_t size =  (int) fInitWeights->size();      
    if(size==0)  Error("ConstructPseudoPDF", "Initial guess of parameters not given... use ImportAllParamInitWeight() method first...");        


    if(fModelPseudoPDF)
      {
	delete fModelPseudoPDF;
	fModelPseudoPDF=0;
	//fHistPdfs.Delete();
	fFractions.removeAll();
      }
   
    //-----------generate kernels------------
    RooArgList kernels;

    //number of the last set added in kernels
    //used for not extended pdf to remove last coef
    Int_t last = -1;

    for(int i=0; i<GetNumberOfDataSets(); i++)
      {                    
        const RooDataHist *set = GetDataHist(i);
	
	// Check if there are events in the DataSet
	// Else error : RooFit can't create an empty RooHistPdf
	if(set->sumEntries()>0)
	  {	    
// 	    ++fNpdf;       // Counting the number of kept RooHistPdf for the fit
	    last=i;        // Remember the last kept histogram for extended fit  
      
	    RooHistPdf *pdf = new RooHistPdf(Form("HistPdf%d",i), Form("RooHistPdf from datahist#%d",i), GetObservables(), *set, fSmoothing);
	    pdf->forceNumInt(IsNumInt()); 
	 		      
	    fHistPdfs.Add(pdf);      
	    kernels.add(*pdf);
	
	    if(debug) Info("ConstructPseudoPDF", "...datahist#%d... added into kernel list (entries=%d)...", i, (int) set->sumEntries());
            
	    //Extended pdf
	    if(IsExtended()==kTRUE)
	       {
	         RooRealVar pp(Form("W%d",i),Form("fractional weight of kernel-PDF #%d for extended fit",i),(*fInitWeights)[i],0., 2.*fexpdatahist_counts);
	         fFractions.addClone(pp);		
	       }
	       	
	    //Not extended pdf
	    else
	       { 		   
	         RooRealVar pp(Form("W%d",i),Form("fractional weight of kernel-PDF #%d for not-extended fit",i),(*fInitWeights)[i],0.,1.);
	         fFractions.addClone(pp);
	       }	      		       
	      
	  } 
	 
	 
	else
	  {
	    // Keep the number of the refused RooDataHist/RooDataSet in a list
	    fRefusedDataSets->Add(i);	    
	    Info("ConstructPseudoPDF", "...datahist#%d... refused (nentries=%d)...", i, (int) set->sumEntries());	    
	  } 
      }
   
          
    //-------------Remove last coefficient from fFractions for not-extended fit-----------
    if(IsExtended()==kFALSE)
      {
	RooRealVar *p = (RooRealVar*) fFractions.find(Form("W%d",last));
	  
	if(p) 
	  {
	    fFractions.remove(*p);
	    Info("ConstructPseudoPdf", "...last dataset (dataset#%d) removed for not-extended fit...", last);
	  } 
	     
	else Error("ConstructPseudoPdf", "...error in finding the last dataset to remove for not-extended fit...");  
      }
   
   //----------------Construction of the pseufo-pdf----------------
    //Extended
    if(IsExtended()==kTRUE) fModelPseudoPDF = new NewRooAddPdf("Model", "Pseudo-PDF constructed from kernels for model datahists", kernels, fFractions, kFALSE);
    //Not extended
    else                    fModelPseudoPDF = new NewRooAddPdf("Model", "Pseudo-PDF constructed from kernels for model datahists", kernels, fFractions, kTRUE); 
    
    // Numerical integral   
    fModelPseudoPDF->forceNumInt(IsNumInt());	      
  }
    
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  RooFitResult* GenericModel_Binned::fitTo(RooDataHist& data, const RooCmdArg& arg1, const RooCmdArg& arg2, const RooCmdArg& arg3, const RooCmdArg& arg4, 
					   const RooCmdArg& arg5, const RooCmdArg& arg6, const RooCmdArg& arg7, const RooCmdArg& arg8,
					   const RooCmdArg& arg9, const RooCmdArg& arg10, const RooCmdArg& arg11, const RooCmdArg& arg12)
  {
    //Print informations
    Info("fiTo...","performing fit with %d parameters and %d observables", GetNumberOfParameters(), GetNumberOfObservables());
    for(Int_t ii=0; ii<GetNumberOfParameters(); ii++)
      {
        RooRealVar *p = dynamic_cast<RooRealVar*>(GetParameters().at(ii));
	RooAbsBinning& bins = p->getBinning();
        Int_t N = bins.numBins();
        Info("fiTo...","parameter%d: name=%s, min=%e, max=%e, nbin=%d",ii, p->GetName(),p->getMin(),p->getMax(),N); 
      }
    for(Int_t ii=0; ii<GetNumberOfObservables(); ii++)
      {
        RooRealVar *p = dynamic_cast<RooRealVar*>(GetObservables().at(ii));
	RooAbsBinning& bins = p->getBinning();
        Int_t N = bins.numBins();
        Info("fiTo...","observable%d: name=%s, min=%e, max=%e, nbin=%d",ii, p->GetName(),p->getMin(),p->getMax(),N); 
      }  
        
    //Fit
    fLastFit = fModelPseudoPDF->improvedFitTo(data, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12);
        
    //Save Coefs
    SafeDelete(fParamDataHist);
    fParamDataHist = new RooDataHist("params","params",GetParameters());
   
    fWeights.removeAll();
    const RooArgList& coefs = fModelPseudoPDF->coefList();
    
    for(int i=0;i<GetNumberOfDataSets();i++)
      { 
        // If accepted set of parameters (events in the RooDataSet)
        if(fRefusedDataSets->Contains(i)==kFALSE)
	   {        
	     RooAbsReal* coef = (RooAbsReal*)coefs.at(i);
	     RooRealVar w(Form("w%d",i),Form("Fitted weight of kernel#%d",i),coef->getVal());
	     printf("coef%d=%e\n",i, coef->getVal());
	
	     if(coef->InheritsFrom(RooRealVar::Class()))
	       {
	         w.setError(((RooRealVar*)coef)->getError());
	       }
	       
	     else
	        {
	         w.setError(coef->getPropagatedError(*fLastFit));
	        }
      
	     fWeights.addClone(w);
	     fParamDataHist->set(*GetParametersForDataset(i),w.getVal(),w.getError());	
	   } 
	   
	else
           {
             printf("coef%d not calculated because of no events in the RooDataSet", i);
           }      	    
      }
           
    SafeDelete(fParameterPDF);
    fParameterPDF = new RooHistPdf("paramPDF","paramPDF",GetParameters(),*fParamDataHist); 
     
    return fLastFit;
  }
    
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  RooFitResult* GenericModel_Binned::chi2FitTo(RooDataHist& data, const RooCmdArg& arg1, const RooCmdArg& arg2, const RooCmdArg& arg3, const RooCmdArg& arg4, 
					   const RooCmdArg& arg5, const RooCmdArg& arg6, const RooCmdArg& arg7, const RooCmdArg& arg8)
					   
  {
    //Print informations
    Info("fiTo...","performing fit with %d parameters and %d observables", GetNumberOfParameters(), GetNumberOfObservables());
    for(Int_t ii=0; ii<GetNumberOfParameters(); ii++)
      {
        RooRealVar *p = dynamic_cast<RooRealVar*>(GetParameters().at(ii));
	RooAbsBinning& bins = p->getBinning();
        Int_t N = bins.numBins();
        Info("fiTo...","parameter%d: name=%s, min=%e, max=%e, nbin=%d",ii, p->GetName(),p->getMin(),p->getMax(),N); 
      }
    for(Int_t ii=0; ii<GetNumberOfObservables(); ii++)
      {
        RooRealVar *p = dynamic_cast<RooRealVar*>(GetObservables().at(ii));
	RooAbsBinning& bins = p->getBinning();
        Int_t N = bins.numBins();
        Info("fiTo...","observable%d: name=%s, min=%e, max=%e, nbin=%d",ii, p->GetName(),p->getMin(),p->getMax(),N); 
      }  
        
    //Fit
    fLastFit = fModelPseudoPDF->chi2FitTo(data, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
        
    //Save Coefs
    SafeDelete(fParamDataHist);
    fParamDataHist = new RooDataHist("params","params",GetParameters());
   
    fWeights.removeAll();
    const RooArgList& coefs = fModelPseudoPDF->coefList();
    
    for(int i=0;i<GetNumberOfDataSets();i++)
      { 
        // If accepted set of parameters (events in the RooDataSet)
        if(fRefusedDataSets->Contains(i)==kFALSE)
	   {        
	     RooAbsReal* coef = (RooAbsReal*)coefs.at(i);
	     RooRealVar w(Form("w%d",i),Form("Fitted weight of kernel#%d",i),coef->getVal());
	     printf("coef%d=%e\n",i, coef->getVal());
	
	     if(coef->InheritsFrom(RooRealVar::Class()))
	       {
	         w.setError(((RooRealVar*)coef)->getError());
	       }
	       
// 	     else
// 	        {
// 	         w.setError(coef->getPropagatedError(*fLastFit));
// 	        }
      
	     fWeights.addClone(w);
	     fParamDataHist->set(*GetParametersForDataset(i),w.getVal(),w.getError());	
	   } 
	   
	else
           {
             printf("coef%d not calculated because of no events in the RooDataSet", i);
           }      	    
      }
           
    SafeDelete(fParameterPDF);
    fParameterPDF = new RooHistPdf("paramPDF","paramPDF",GetParameters(),*fParamDataHist); 
     
    return fLastFit;
  }
  //////////////////////////////////////////////////////////////////////////////////////////////////////
  void GenericModel_Binned::plotOn(RooPlot* frame)
  {
    //     // Add the pseudo-PDF to the RooPlot
    //     // We plot:
    //     //    - the fit result with a 1-sigma error band;
    //     //    - each of the individual kernel PDF's
    // 
    //     //plot fit result with 1-sigma error band
    //     fModelPseudoPDF->plotOn(frame, VisualizeError(*fLastFit,1),FillColor(kMagenta));
    //     fModelPseudoPDF->plotOn(frame);
    //     //plot individual kernels
    //     for(int i=0; i<GetNumberOfDataSets(); i++)
    //       {
    // 	fModelPseudoPDF->plotOn(frame,Components(*GetKernel(i)),LineStyle(kDashed),LineColor(kBlue+4*i));
    //       }
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////////
  void GenericModel_Binned::InitWorkspace(Bool_t debug)
  {   
    fBool_provided = kFALSE;
    	
    RooArgList par;
    RooArgList obs;
    RooArgList parobs;
    TObjArray  dataset;     
    TObjArray  datasetpar;   
    //TObjArray  histpdf;	   
        
    //init new workspace
    Info("InitWorkspace ...", "Creating a new workspace...");  
    
    fWorkspace = new RooWorkspace("_workspace","RooWorkspace for the fit");
    fWorkspace->import(par, "_parameters");
    fWorkspace->import(obs, "_observables");
    fWorkspace->import(parobs,"_parobs");
    fWorkspace->import(dataset,"_datahistset");
    fWorkspace->import(datasetpar,"_datasetparams");  
    
    VerifyWorkspace(debug); 
  }
  
  
  //////////////////////////////////////////////////////////////////////////////////////////////////////    
  void GenericModel_Binned::ImportWorkspace(RooWorkspace* workspace, Bool_t debug)
  {  
    fBool_provided = kTRUE;  
    fWorkspace = workspace;
    Info("ImportWorkspace ...", "Importing the given workspace...");
    VerifyWorkspace(debug);
  }


  //////////////////////////////////////////////////////////////////////////////////////////////////////////  
  Bool_t GenericModel_Binned::VerifyWorkspace(Bool_t debug)
  {
    Bool_t good_workspace = kTRUE;
  
    //If empty worksapce...
    if(fWorkspace==0)
      {
        Error("VerifyWorkspace...", "Empty workspace or no workspace provided...");
	good_workspace=kFALSE;
      } 
       
    else
      {       
	RooArgList* par    = NULL;
	RooArgList* obs    = NULL;
	RooArgList* parobs = NULL;
	TObjArray*  dat    = NULL;
	TObjArray*  datpar = NULL;        
      
	//------Parameters------       
	par  = (RooArgList*) fWorkspace->obj("_parameters");    
	if(par==0)	   
	  {
	    Error("VerifyWorkspace ...", "_parameters NOT found...");
	    good_workspace=kFALSE;
	  }
	  
	else
	  {
	    if(debug==kTRUE) Info("VerifyWorkspace ...", "_parameters found...");
	  } 	 
   
	//------Observables-----
	obs = (RooArgList*) fWorkspace->obj("_observables");
	if(obs==0)
	  {
	    Error("VerifyWorkspace ...", "_observables NOT found...");
	    good_workspace=kFALSE;
	  }
          
	else
	  {
	    if(debug==kTRUE)  Info("VerifyWorkspace ...", "_observables found...");
	  }
       	 
	//----------ParObs-------  
	parobs  = (RooArgList*) fWorkspace->obj("_parobs");
	if(parobs==0)
	  {
	    Error("VerifyWorkspace ...", "_parobs NOT found...");
	    good_workspace=kFALSE;
	  }
	   
	else 
	  {
	    if(debug==kTRUE) Info("VerifyWorkspace ...", "_parobs found...");
	  }	
   
	//-------DataSets------
	dat = (TObjArray*)  fWorkspace->obj("_datahistset");
	if(dat==0)
	  {
	    Error("VerifyWorkspace ...", "_datahistset NOT found...");
	    good_workspace=kFALSE;
	  }
	   
	else
	  {
	    if(debug==kTRUE) Info("VerifyWorkspace ...", "_datahistset found...");
	  }
	 
	//-------DataSets Parameters-------
	datpar = (TObjArray*)  fWorkspace->obj("_datasetparams");
	if(datpar==0)
	  {
	    Error("VerifyWorkspace ...", "_datasetparams NOT found...");
	    good_workspace=kFALSE;
	  }
	   
	else
	  {
	    if(debug==kTRUE) Info("VerifyWorkspace ...", "_datasetparams found...");
	  }	
      }	  
	 
    return good_workspace;	    		         
  }
  





  //////////////////////////////////////////////////////////////////////////////////////////////////////////     
  void GenericModel_Binned::SaveInitWorkspace(const char *file)
  {  
    /*
      To save an initial worksapce for further fits
      Will save the needed objects for the performing another fit
      without generating the new RooDataSets (can be long)
      will contain:
      _parameters    = RooArgList of the parameters
      _obsevables    = RooArgList of the observables
      _parobs        = RooArgList of the param+observables (mandatory for cuts in the RooDataHist)
      _datahistset   = TObjArray of the  model RooDataHist
      _datasetparams = TObjArray of the model RooDataHist parameters
      This workspace can then be used with SetWorkspace() method	   
      By default not saved and saved in a file which name will contain
      observables informations
      If workspace already provided, will not save it again   
    */ 
     
    fBool_saved = kTRUE; 
    fwk_name    = file;           
  }
  
  
  //////////////////////////////////////////////////////////////////////////////////////////////////////////   
  void GenericModel_Binned::CreateWorkspaceSaving(const char *file)
  {      
    RooWorkspace *InitWorkspace = new RooWorkspace(Form("init_workspace"),"RooWorkspace prepared for fit");

    RooArgList par     = *((RooArgList*) fWorkspace->obj("_parameters"));
    RooArgList obs     = *((RooArgList*) fWorkspace->obj("_observables"));
    RooArgList parobs  = *((RooArgList*) fWorkspace->obj("_parobs"));
    TObjArray dat      = *((TObjArray*)  fWorkspace->obj("_datahistset"));
    TObjArray datpar   = *((TObjArray*)  fWorkspace->obj("_datasetparams"));
    		    
    InitWorkspace->import(par, "_parameters");
    InitWorkspace->import(obs, "_observables");
    InitWorkspace->import(parobs,"_parobs");
    InitWorkspace->import(dat,"_datahistset");
    InitWorkspace->import(datpar,"_datasetparams");
    	   
    if(file==0)
      {
    	//For name
    	TString ss0("_PARA");
    	for(Int_t ii=0;ii<GetNumberOfParameters();ii++)
    	  {	      
    	    RooArgList ll = GetParameters();
    	    RooRealVar vv = *((RooRealVar*) ll.at(ii));
    	    TString ss1(Form("_%s[%.1lf,%.1lf,%d]",vv.GetName(),vv.getMin(),vv.getMax(),vv.getBins()));
    	    ss0+=ss1;		
    	  }

    	TString ss2("_OBS");  
    	for(Int_t ii=0;ii<GetNumberOfObservables();ii++)
    	  {	      
    	    RooArgList ll = GetObservables();
    	    RooRealVar vv = *((RooRealVar*) ll.at(ii));
    	    TString ss3(Form("_%s[%.1lf,%.1lf,%d]",vv.GetName(),vv.getMin(),vv.getMax(),vv.getBins()));
    	    ss2+=ss3;		
    	  }  

    	ss0+=ss2;
    	
	Info("SaveWorkspace", "Saving created RooDataSet workspace in file 'workspace%s.root'", ss0.Data());			      
    	InitWorkspace->writeToFile(Form("workspace%s.root",ss0.Data()),"recreate");
	Info("SaveWorkspace", "Save done...");
      }

    else
      {    		    
    	Info("SaveWorkspace", "Saving Created RooDatSet workspace in file '%s.root'", file);
	InitWorkspace->writeToFile(Form("%s.root",file),"recreate");
        Info("SaveWorkspace", "Save done...");	    
      }          
  }       
} 
