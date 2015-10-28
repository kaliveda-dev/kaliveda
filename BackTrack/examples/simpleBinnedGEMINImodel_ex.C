/* simpleBinnedGeminimodel_ex.C
 *
 * Example of use of class BackTrack::SimpleGeminiModel_Binned in order to demonstrate
 * BackTracking fit of model parameter distribution using binned data.
 * BackTracking is then performed in order to retrieve the initial parameter distribution
*/

#include "RooRealVar.h"
#include "RooConstVar.h"
#include "RooDataSet.h"
#include "RooGenericPdf.h"
#include "RooProdPdf.h"
#include "RooPlot.h"
#include "RooPolyVar.h"
#include "RooGaussian.h"
#include "RooUniform.h"
#include "RooExponential.h"
#include "TCanvas.h"
#include "TH1.h"
#include "TH2.h"
#include "TF2.h"
#include "TMath.h"
#include <boost/progress.hpp>

#include "SimpleGeminiModel_Binned.h"
#include "NewRooGlobalFunc.h"     // To add new RooCmd

using namespace RooFit;

BackTrack::SimpleGeminiModel_Binned* model=0;


////////////////////////////////////////////////////////////////
//Correlated gaussian 2D
Double_t corr_gaus2D(Double_t *xx, Double_t *par)
{
  Double_t e2     =  xx[0];
  Double_t e1     =  xx[1];

  Double_t E2     =  par[0]; 
  Double_t sigE2  =  par[1]; 
  Double_t E1     =  par[2]; 
  Double_t sigE1  =  par[3]; 
  Double_t norma  =  par[4]; 
  Double_t rho    =  par[5]; 

  Double_t dE1    =  E1-e1;
  Double_t dE2    =  E2-e2;

  Double_t A   = -0.5/( (1.-TMath::Power(rho,2.))*TMath::Power(sigE1*sigE2,2.) )*( TMath::Power(dE1*sigE2,2.) + TMath::Power(dE2*sigE1,2.) - 2.*rho*dE1*dE2*sigE1*sigE2 );
  Double_t det = (1.-TMath::Power(rho,2.))*TMath::Power(sigE1*sigE2,2.);
  Double_t Q   = norma * (1./det)*exp(A);

  return Q;
}   



void simpleGEMINImodel(Int_t statexp, Int_t statmod)
{   
   //------------Suppress RooFit info messages-----------
   RooMsgService::instance().setGlobalKillBelow(RooFit::WARNING);

   //If RooWorkspace already generated
   TFile *file = new TFile("./workspace_PARA_APrimary[30.0,50.0,20]_EStarPrimary[0.5,6.5,12]_OBS_APrimaryWoN[25.0,45.0,20]_EStarPrimaryWoN[20.0,200.0,90].root");
   RooWorkspace *ww = (RooWorkspace *) file->Get("init_workspace");
   

   model = new BackTrack::SimpleGeminiModel_Binned(); 
   if(ww==0)
      {
        model->InitWorkspace();
        model->InitParObs();
      } 
      
   else model->ImportWorkspace(ww);  
   
   model->SetSourceCharge(15);
   model->SetSourceSpin(0, 0, 0);     //hbar unit
   model->SetSourceVelocity(0, 0, 8);  //cm/ns unit
   
   //===========================
   //==     "EXP" DATASET     ==
   //===========================

   //Generate data with GEMINI according to a correlated gaussian-distribution for parameters Apr and E*/Apr
   model->SetNumGen(statexp);
   RooDataSet data("data","data to be fitted",model->GetObservables());
   RooDataSet params("params","input parameters",model->GetParameters());
   RooRealVar& PAR1 = *(model->GetParameter("APrimary"));
   RooRealVar& PAR2 = *(model->GetParameter("EStarPrimary"));
   
   //Create the TH2D of the 2D correlated Gaussian
   TF2 *f2 = new TF2("mygaus2D", corr_gaus2D, 30, 50, 0, 7, 6);
   f2->SetParameters(35.,2.,3.0,0.5,1.,0.5);   // (<A>,sA,<E>,sE, norm, rho)
   f2->SetNpx(1000);
   f2->SetNpy(1000);
   f2->Draw("colz");
   Double_t par1 = 0;  //Apr
   Double_t par2 = 0;  //E*/Apr
   Int_t    par3 = 0;
    
   //Infos
   printf("Generating data with %d events\n", model->GetNumGen()); 
   //Counter
   boost::progress_display *prd = new boost::progress_display(model->GetNumGen());    
              
   for(int i=0;i<model->GetNumGen();i++)
   {
     ++(*prd);
   
     f2->GetRandom2(par1, par2);
     par3  = (int) par1;  //Give an integer value to par1
     //printf("par1=%e, par2=%e, par3=%d\n", par1, par2, par3);
     PAR1.setVal(par3);
     PAR2.setVal(par2);
     model->generateEvent(RooArgList(PAR1,PAR2),data);
     params.add(model->GetParameters());
   }
   
   delete prd;
   
   //Create the "experimental" RooDataHist used for the fit  with the RooDataSet 
   //generated just before
   RooDataHist datahist("datahist_exp","input parameters", model->GetObservables(), data);
   
//    Modify the ranges for the fit
//    model->GetObservable("obs1")->setRange("RANGE",-10,32);
//    model->GetObservable("obs2")->setRange("RANGE",-20,15);
   
   //===========================
   //==    BACKTRACKING       ==
   //===========================
   //Initialise the BackTracking
   model->SetExperimentalDataHist(datahist);
   if(ww==0) model->SaveInitWorkspace();
   model->SetNumInt(kFALSE);
   model->SetExtended(kFALSE);
   model->SetNumGen(statmod);
   if(ww==0) model->ImportAllModelData();   //import model data before importing the weights (because we need the number of datasets to create the uniform weights)
   model->ImportAllParamInitWeight(kTRUE);  //uniform init values if KTRUE 
   model->ConstructPseudoPDF(kFALSE);
   
   //Fit
   //model->fitTo(datahist, Save(), NumCPU(6), SumW2Error(kTRUE), PrintLevel(1), Minimizer("TMinuit","migrad"), Extended(kFALSE), Offset(kTRUE));
   model->fitTo(datahist, Save(), NumCPU(6), SumW2Error(kTRUE), PrintLevel(1), Minimizer("TMinuit","migrad"), Extended(kFALSE), Offset(kTRUE), SetMaxIter(500000), SetMaxCalls(500000));
   //model->fitTo(datahist, Save(), NumCPU(6), SumW2Error(kTRUE), PrintLevel(1), Minimizer("GSLMultiMin","conjugatefr"), Extended(kFALSE), Offset(kTRUE), SetMaxCalls(500000), SetMaxIter(500000));   
   //model->fitTo(datahist, Save(), NumCPU(4), SumW2Error(kTRUE), PrintLevel(1), Minimizer("GSLSimAn"), Extended(kFALSE), Offset(kTRUE)); //, SetEpsilon(0.01), SetMaxCalls(500000));
   
   //Control refused sets
   KVNumberList *r = (KVNumberList*) model->GetRefusedSetsList();
   r->Begin();
   while( !r->End() )
      {
       Int_t ref_num = r->Next();
       printf("Number of refused RooDataSet=%d\n", ref_num);
      }
 
   //Look at NLL for a set of parameters
   //model->plotProfileNLL(12);
 
   //==================
   //==    PLOTS     ==
   //==================
   //---------------Results compared to experimental data----------------
   RooPlot *plot = model->GetObservable("APrimaryWoN")->frame();
   data.plotOn(plot);
   //Draw with errors
   //model->GetPseudoPDF()->plotOn(plot,VisualizeError(*(model->GetLastFit())));
   //Draw without errors
   model->GetPseudoPDF()->plotOn(plot);
   TCanvas*c = new TCanvas("Observables_results","Observables_results");
   c->Divide(2,1);
   c->cd(1);
   plot->Draw();  
   plot = model->GetObservable("EStarPrimaryWoN")->frame();
   data.plotOn(plot);
   //Draw with errors
   //model->GetPseudoPDF()->plotOn(plot,VisualizeError(*(model->GetLastFit())));
   //Draw without errors   
   model->GetPseudoPDF()->plotOn(plot);
   c->cd(2);
   plot->Draw();
   
   //For binning
   RooRealVar *o1 = dynamic_cast<RooRealVar*>(model->GetObservables().at(0));
   RooAbsBinning& bins1 = o1->getBinning();
   Int_t N1 = bins1.numBins();

   RooRealVar *o2 = dynamic_cast<RooRealVar*>(model->GetObservables().at(1));
   RooAbsBinning& bins2 = o2->getBinning();
   Int_t N2 = bins2.numBins(); 
   
   
   TH2 *hh = (TH2*) (model->GetPseudoPDF())->createHistogram("APrimaryWoN,EStarPrimaryWoN", N1, N2);
   TH2 *hhdata = (TH2*) data.createHistogram("APrimaryWoN,EStarPrimaryWoN", N1, N2);
   
   if(hh && hhdata)
       {       
   	TCanvas *ccc = new TCanvas("Observables_results_2D", "2D_Observables_results");
        ccc->Divide(2,2);
   	ccc->cd(1);
	gPad->SetLogz();
	gPad->Modified();
	ccc->Update();
   	hhdata->Draw("colz");
        ccc->cd(2);
        hhdata->Draw("lego");
        ccc->cd(3);
	gPad->SetLogz();
	gPad->Modified();
	ccc->Update();
        hh->Draw("colz");
        ccc->cd(4);
        hh->Draw("lego");
	          
        //For binning
     	RooRealVar *p1 = dynamic_cast<RooRealVar*>(model->GetParameters().at(0));
	RooAbsBinning& bins1 = p1->getBinning();
        Int_t NN1 = bins1.numBins();
	
	RooRealVar *p2 = dynamic_cast<RooRealVar*>(model->GetParameters().at(1));
	RooAbsBinning& bins2 = p2->getBinning();
        Int_t NN2 = bins2.numBins();
	

       //-----------Return input parameters---------------
       TH2D* hh_par_data = (TH2D*) params.createHistogram("APrimary,EStarPrimary",NN1,NN2) ;
       if(hh_par_data) hh_par_data->Write();   
       else printf("impossible to create hh_par_data\n");
   
   	//-------------Return fitted parameters (see SimpleGeminiModel.h")------------
       TH2D* hh_par_fit = (TH2D*) model->GetParameterDistributions(); 
       if(hh_par_fit) hh_par_fit->Write();
       else printf("impossible to create hh_par_fit\n");
   
       //Draw
       TCanvas *cc = new TCanvas("Parameters_results_2D","2D_Parameters_results");
       cc->Divide(2,2);
       cc->cd(1);
       hh_par_data->Draw("lego");
       cc->cd(2);
       hh_par_data->Draw("colz");
       cc->cd(3);
       hh_par_fit->Draw("lego");
       //hh_params->ProjectionX()->Draw();
       cc->cd(4);
       hh_par_fit->Draw("colz");
       //hh_params->ProjectionY()->Draw(); 
       
       //------------------------------------------------------------------------ 
       //------Chi2 for fit result-----
       TH2D* hh_par_data2 = new TH2D(*hh_par_data);
       TH2D* hh_par_fit2 = new TH2D(*hh_par_fit);
       hh_par_data2->Scale(1./hh_par_data2->Integral());
       Double_t chi2 = hh_par_data2->Chi2Test(hh_par_fit2,"CHI2/NDF,P");
       printf("chi2/ndf=%e\n", chi2);   
   
       //Projections for comparisons
       TH1D* projx = (TH1D*) hh_par_data2->ProjectionX();
       TH1D* projy = (TH1D*) hh_par_data2->ProjectionY();   
          
       RooPlot* pl = model->GetParameter("APrimary")->frame();
       model->GetParamDataHist()->plotOn(pl);   
       new TCanvas("Plot_result_par1","Plot PAR1");
       pl->Draw();
       projx->Draw("same");

 
       pl = model->GetParameter("EStarPrimary")->frame();
       model->GetParamDataHist()->plotOn(pl);  
       new TCanvas("Plot_result_par2","Plot PAR2");
       pl->Draw();
       projy->Draw("same");           	        
       }
   
   else
       {
   	printf("WTF, impossible to createHistogram from GetPseudoPDF ???\n");
       }               
}

