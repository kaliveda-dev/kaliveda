//Created by KVClassFactory on Tue Oct 13 10:57:16 2015
//Author: Quentin Fable

#include "SimpleGeminiModel_Binned.h"
#include "TVector3.h"
#include "KVSimEvent.h"
#include "KVSimNucleus.h"
#include "KVCalorimetry.h"
#include "KVGemini.h"
#include <boost/progress.hpp>

using namespace std;

ClassImp(BackTrack::SimpleGeminiModel_Binned)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>SimpleGeminiModel_Binned</h2>
<h4>An example of fit with BackTrack using the GEMINI++ desexcitation code</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////


namespace BackTrack {
  
//////////////////////////////////////////////////////////////////////////////////////////////  
  SimpleGeminiModel_Binned::SimpleGeminiModel_Binned()
  {
    // Default constructor
    fNGen     = 0;  
    fZpr      = -1;
    fJpr_x    = 0.;
    fJpr_y    = 0.; 
    fJpr_z    = 0.;   	        
  }
      
//////////////////////////////////////////////////////////////////////////////////////////////     
  void SimpleGeminiModel_Binned::InitParObs()
  {   
    if(IsWorkspaceImported()==kFALSE)
       { 
        Info("InitParObs","...Creating Parameters and Observables...");
        InitPar();
        InitObs();
       }
       
    else Error("InitParObs", "...No need to create Parameters and Observables, workspace imported...");	
  }
      
   
  void SimpleGeminiModel_Binned::InitObs()
  {
    AddObservable("APrimaryWoN",         "Mass of the source without neutrons",                                       25,  45, 20);
    AddObservable("EStarPrimaryWoN",     "Excitation energy of the source by calorimetry without neutrons (MeV)",     20, 200, 90);
  } 
   
   
  void SimpleGeminiModel_Binned::InitPar()
  {
    AddParameter("APrimary",    "Mass of the source",                          30,  50,  20);
    AddParameter("EStarPrimary","Excitation energy of the source (in MeV/A)", 0.5, 6.5,  12);
  } 
  
  
//////////////////////////////////////////////////////////////////////////////////////////////    
  void SimpleGeminiModel_Binned::SetSourceCharge(Int_t zpr)
  {
    fZpr = zpr;
  } 
  
    
  void SimpleGeminiModel_Binned::SetSourceSpin(Double_t lx, Double_t ly, Double_t lz)
  {
    fJpr_x = lx;
    fJpr_y = ly;
    fJpr_z = lz;
  }
  
  
  void SimpleGeminiModel_Binned::SetSourceVelocity(Double_t vx, Double_t vy, Double_t vz)  
  { 
    TVector3 vec(vx, vy, vz);
    fVpr = vec;
  }
 
 
//////////////////////////////////////////////////////////////////////////////////////////////  
  void SimpleGeminiModel_Binned::SetParamDistribution(TH2D *distri)
  { 
    //Set the initial guesses/weights on (Apr,E*pr/Apr) for the fit
    fdistri=distri;
  }
 
  
//////////////////////////////////////////////////////////////////////////////////////////////
  void SimpleGeminiModel_Binned::generateEvent(const RooArgList& parameters, RooDataSet& data)
  {
    // generate an event (i.e. a pair of observables) given the parameters values with GEMINI++ model
    // and add it to the dataset

    //Parameters
    Double_t aa = ((RooRealVar*)parameters.at(0))->getVal();  //Apr
    Double_t ee = ((RooRealVar*)parameters.at(1))->getVal();  //E*pr/Apr in MeV/A
    
    //-----------GEMINI event-----------
    //Init
    KVGemini gem;
    KVSimEvent   *hot  = new KVSimEvent();
    KVSimEvent   *cold = new KVSimEvent();
    //Declare particle to decay  
    KVSimNucleus *nuc  = (KVSimNucleus*) hot->AddParticle();
    nuc->SetZandA(fZpr, aa);
    nuc->SetVelocity(fVpr);
    nuc->SetExcitEnergy(ee*aa);
    nuc->SetAngMom(fJpr_x, fJpr_y, fJpr_z);
    //Decay
    gem.DecayEvent(hot, cold);
    
    //---------------Compute the chosen observables [Apr-n, (E*-En)/(Apr-n)] with a KVCalorimetry-------------
    //Init
    KVCalorimetry *fcal = new KVCalorimetry();
    Int_t    obs1        = 0;  //Apr-n
    Double_t obs2        = 0.; //(E*-En)/(Apr-n)
    //For the boost
    Double_t etot = 0.; 
    TVector3 ptot(0., 0., 0.);                 
    KVSimNucleus* part = new KVSimNucleus();
    
    //Compute the boost in order to apply calorimetry on the fragments
    for(Int_t hh=1;hh<=cold->GetMult();hh++)
       {
         part = (KVSimNucleus*) cold->GetParticle(hh);       
         etot+= part->E();
	 ptot+= part->GetMomentum();
       }
    //Define center of mass frame   
    ptot*=1./etot;
    cold->SetFrame("CM", ptot, kTRUE);
    fcal->SetFrame("CM");
    
    
    //Calorimetry
    for(Int_t hh=1;hh<=cold->GetMult();hh++)
       {
         part = (KVSimNucleus*) cold->GetParticle(hh);
         if(part->GetZ() !=0) fcal->Fill(part);
       }
    fcal->Calculate();
    
    //APrimaryWoN
    obs1 = fcal->GetValue("Asum");
    GetObservable("APrimaryWoN")->setVal(obs1);       
    
    //EStarPrimaryWoN
    obs2 = fcal->GetValue("Exci");
    GetObservable("EStarPrimaryWoN")->setVal(obs2);
    
    //Fill the RooDataSet "data"
    data.add(GetObservables());
  }
  

//////////////////////////////////////////////////////////////////////////////////////////////
  RooDataHist*SimpleGeminiModel_Binned::GetModelDataHist(RooArgList& par)
  {
    // create and fill data set using uniform distributions of E*/Apr in
    // currently defined ranges

    RooRealVar* par1 = (RooRealVar*)par.at(0);
    RooRealVar* par2 = (RooRealVar*)par.at(1);

    //Infos
    Info("GetModelDataHist", "Generating %d events with Apr=%e, E*pr/Apr=%e", GetNumGen(), par1->getVal(), par2->getVal()); 
    //Counter
    boost::progress_display *pd = new boost::progress_display(GetNumGen());    

    
    static int dsnum=1;
    RooDataSet* data = new RooDataSet(Form("DATA#%d",dsnum),"dataset",GetObservables());   
    Int_t numgen = GetNumGen();
    
    for(int i=0;i<numgen;i++)
      {
        //Counter progress
        ++(*pd);      
	//Apr
	Int_t P1    = (Int_t) par1->getMin();
	//E*/Apr
	Double_t P2 = gRandom->Uniform(par2->getMin(),par2->getMax());
	par1->setVal(P1);
	par2->setVal(P2);
	generateEvent(par,*data);	 
      } 
      
    RooDataHist* datahist = new RooDataHist(Form("DATAHIST#%d",dsnum++),"datahist",GetObservables(),*data);

    //Trick if can't initialize because of too much empty bins in the observables RooHistPdf
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
// 	RooDataHist *datahistinit = new RooDataHist("init","init", GetExpectedCounts(), hh);  
// 	 
//	datahist->add(*datahistinit);
//	delete hh;       
//      }	 
  	                                                                   
    return datahist;
  }
  
  
    
//////////////////////////////////////////////////////////////////////////////////////////////
  Double_t SimpleGeminiModel_Binned::generateWeight(const RooArgList& parameters, TH2D* distri)
  { 
    if(distri==0) Error("generateWeight", "...set a distribution for initial weights of parameters for the fit with SetParamDistribution() method  first...");
  
    Double_t par1 = ((RooRealVar*)parameters.at(0))->getVal();
    Double_t par2 = ((RooRealVar*)parameters.at(1))->getVal();
    
    Double_t weight = distri->GetBinContent(distri->FindBin(par1, par2));
    return weight;
  } 
  
  
//////////////////////////////////////////////////////////////////////////////////////////////  
  Double_t SimpleGeminiModel_Binned::GetParamInitiWeight(RooArgList &par)
  {
   return generateWeight(par, fdistri);
  }


/////////////////////////////////////////////////////////////////////////////////////////
  TH1*SimpleGeminiModel_Binned::GetParameterDistributions()
  {
    // Return 2D histo of fitted parameter distributions

    TH2F* h = (TH2F*) GetParamDataHist()->createHistogram("APrimary,EStarPrimary",GetParameter("APrimary")->getBins(),GetParameter("EStarPrimary")->getBins());
    h->SetName("fittedParameters");
    return h;
  }   
   
      
////////////////////////////////////////////////////////////////////////////////////////////// 
   SimpleGeminiModel_Binned::~SimpleGeminiModel_Binned()
   {
      // Destructor
   }
   
}   
 
