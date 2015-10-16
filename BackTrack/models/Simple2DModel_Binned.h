//Created by KVClassFactory on Wed Mar  4 16:50:19 2015
//Author: John Frankland,,,

#ifndef __SIMPLE2DMODEL_BINNED_H
#define __SIMPLE2DMODEL_BINNED_H

#include "GenericModel_Binned.h"
#include "TH1.h"
#include "TH2.h"
#include <vector>

namespace BackTrack {

   class Simple2DModel_Binned : public GenericModel_Binned
   {
    /* Simple model to generate 2 correlated observables [obs1, obs2]    
    *  according to 2 input model parameters [par1,par2] 
    *  We generate obs1 and obs2 according to gaussian distributions of means: mean1=par1+par2  and sigma1=abs(mean1)/5
                                                                               mean2=par1-par2  and sigma2=abs(mean2)/10								      
    */

      ClassDef(Simple2DModel_Binned,1)   //Simple model to test backtrack procedures
      
    protected:

      Int_t fNGen;                      //Number of events to generate for each dataset
      TH2D  *fdistri;                   //TH2D used to set the initial guesses for the fit
      
      void InitPar();
      void InitObs();
      
            
    public:
      
      Simple2DModel_Binned();
      virtual ~Simple2DModel_Binned();
       
      void InitParObs();                                                                //Initialize parameters and observables (need to InitWorkspace before)
      void SetParamDistribution(TH2D *distri);                                          //Initialize the guess on parameters according to a 2-D distribution              
      RooDataHist* GetModelDataHist(RooArgList &par);                                   //Definition for the virtual method for GenericModel_Binned class  
      Double_t GetParamInitiWeight(RooArgList &par);                                    //To get the initial weights
      void generateEvent(const RooArgList& parameters, RooDataSet& data);               //Generate one event for the model
      Double_t generateWeight(const RooArgList& parameters, TH2D* distri);              //Generate guess/weight for the
      void SetNumGen(Int_t n) { fNGen=n; }                                              //Number of events to generate
      Int_t GetNumGen() const { return fNGen; }                                        
      TH1* GetParameterDistributions();                                                 //For drawings after the fit
   };

}
#endif
