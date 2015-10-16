//Created by KVClassFactory on Wed Mar  4 16:50:19 2015
//Authors: John Frankland, Quentin Fable,

#include "Simple2DModel_Binned.h"
#include "TFile.h"
#include "TTree.h"
#include "TH2.h"
#include "TF2.h"
#include "TCanvas.h"

using namespace std;

ClassImp(BackTrack::Simple2DModel_Binned)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
   <h2>Simple2DModel_Binned</h2>
   <h4>Simple model to test backtrack procedures</h4>
   <!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

namespace BackTrack {


  Simple2DModel_Binned::Simple2DModel_Binned()
  {
    fNGen     = 0;  	        
  }
      
   
  void Simple2DModel_Binned::InitParObs()
  {   
    InitPar();
    InitObs();
  }
      
   
  void Simple2DModel_Binned::InitObs()
  {
    AddObservable("obs1","observable #1",-45,45,45);
    AddObservable("obs2","observable #2",-30,30,60); 
  } 
   
   
  void Simple2DModel_Binned::InitPar()
  {
    AddParameter("par1","parameter #1",0,10,10);
    AddParameter("par2","parameter #2",-20,20,20);
  } 
   
   
//////////////////////////////////////////////////////////////////////////////////////////////  
  void Simple2DModel_Binned::SetParamDistribution(TH2D *distri)
  {
   fdistri=distri;
  }
 
   

//////////////////////////////////////////////////////////////////////////////////////////////
  void Simple2DModel_Binned::generateEvent(const RooArgList& parameters, RooDataSet& data)
  {
    // generate an event (i.e. a pair of observables) given the parameters values
    // and add it to the dataset

    Double_t par1 = ((RooRealVar*)parameters.at(0))->getVal();
    Double_t par2 = ((RooRealVar*)parameters.at(1))->getVal();
    
    Int_t obs1 = (int) gRandom->Gaus(par1+par2, abs(par1+par2)/5.);
    Double_t diff = (par1-par2);
    Double_t obs2 = gRandom->Gaus(diff,abs(diff)/10.);
    
    GetObservable("obs1")->setVal(obs1);
    GetObservable("obs2")->setVal(obs2);
    data.add(GetObservables());
  }
  

//////////////////////////////////////////////////////////////////////////////////////////////
  RooDataHist*Simple2DModel_Binned::GetModelDataHist(RooArgList& par)
  {
    // create and fill data set using uniform distributions of parameters in
    // currently defined ranges

    static int dsnum=1;
    RooDataSet* data = new RooDataSet(Form("DATA#%d",dsnum),"dataset",GetObservables());
    //Int_t numgen = gRandom->Integer(GetNumGen()) + 50.;
    Int_t numgen = GetNumGen();
    
    for(int i=0;i<numgen;i++)
      {
	RooRealVar* par1 = (RooRealVar*)par.at(0);
	RooRealVar* par2 = (RooRealVar*)par.at(1);
	Double_t P1 = gRandom->Uniform(par1->getMin(),par1->getMax());
	Double_t P2 = gRandom->Uniform(par2->getMin(),par2->getMax());
	par1->setVal(P1);
	par2->setVal(P2);
	generateEvent(par,*data);	 
      }
      
    RooDataHist* datahist = new RooDataHist(Form("DATAHIST#%d",dsnum++),"datahist",GetObservables(),*data);

    //Tip if can't initialize because of too much empty bins in the observables RooHistPdf
    //Add a very small value to all bins in the RooDataHist
    //For this we create an histogram that we fill with very small bin contents and we add it to the actual dataset
    //We add a condition on the minimum entries of the DataSet to get ride of empty ones
    
//     if(data->numEntries()>10)
//       {		            
// 	Double_t min_par1   = GetObservable(0)->getMin();
// 	Double_t max_par1   = GetObservable(0)->getMax();
// 	Double_t min_par2   = GetObservable(1)->getMin();
// 	Double_t max_par2   = GetObservable(1)->getMax();
//              
// 	Int_t    nbin_par1  = GetObservable(0)->numBins();
// 	Int_t    nbin_par2  = GetObservable(1)->numBins();
// 
// 	TH2D *hh = new TH2D("init","init", nbin_par1, min_par1, max_par1, nbin_par2, min_par2, max_par2);
//        	    
// 	for(Int_t nx=0; nx<=hh->GetNbinsX();nx++)
//           {
// 	    for(Int_t ny=0; ny<=hh->GetNbinsY();ny++)
// 	      {
// 		hh->SetBinContent(nx,ny,1./10000);
// 	      }
// 	  }
// 	 
// 	RooDataHist *datahistinit = new RooDataHist("init","init", GetObservables(), hh);  
// 	 
//	datahist->add(*datahistinit);
//	delete hh;       
//      }	 
  	                                                                   
    return datahist;
  }
  
  
    
//////////////////////////////////////////////////////////////////////////////////////////////
  Double_t Simple2DModel_Binned::generateWeight(const RooArgList& parameters, TH2D* distri)
  { 
    if(distri==0) Error("generateWeight", "...set a distribution for initial weights of parameters for the fit with SetParamDistribution() method  first...");
  
    Double_t par1 = ((RooRealVar*)parameters.at(0))->getVal();
    Double_t par2 = ((RooRealVar*)parameters.at(1))->getVal();
    
    Double_t weight = distri->GetBinContent(distri->FindBin(par1, par2));
    return weight;
  } 
  
  
//////////////////////////////////////////////////////////////////////////////////////////////  
  Double_t Simple2DModel_Binned::GetParamInitiWeight(RooArgList &par)
  {
   return generateWeight(par, fdistri);
  }


/////////////////////////////////////////////////////////////////////////////////////////
  TH1*Simple2DModel_Binned::GetParameterDistributions()
  {
    // Return 2D histo of fitted parameter distributions

    TH2F* h = (TH2F*) GetParamDataHist()->createHistogram("par1,par2",GetParameter("par1")->getBins(),GetParameter("par2")->getBins());
    h->SetName("fittedParameters");
    return h;
  }
       

/////////////////////////////////////////////////////////////////////////////////////////
  Simple2DModel_Binned::~Simple2DModel_Binned()
  {
    // Destructor
  }	                        
}
