/* simpleBinned2Dmodel_ex.C
 *
 * Example of use of class BackTrack::Simple2DModel_Binned in order to demonstrate
 * BackTracking fit of model parameter distribution using binned data.
 *
 * Simple2DModel_Binned has 2 correlated parameters [par1,par2] and 2 correlated observables [obs1, obs2]
 * We generate obs1 and obs2 according to gaussian distributions of means: mean1=par1+par2  and sigma1=abs(mean1)/5
                                                                           mean2=par1-par2  and sigma2=abs(mean2)/10
 * In this example the "experimental" simulated DataSet is generated using Gaussian distribution for par1 and par2,
 * where par2 is correlated to par1 value. We decide here to set integer values for par1 and obs1 for testing
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

#if ROOT_VERSION_CODE < ROOT_VERSION(6,18,0)

#include "Simple2DModel_Binned.h"
#include "NewRooGlobalFunc.h"     // To add new RooCmd

using namespace RooFit;
//using namespace MyGlobalFunc;

BackTrack::Simple2DModel_Binned* model = 0;

void simple2Dmodel_ex(Int_t statexp, Int_t statmod)
{
   //------------Suppress RooFit info messages-----------
   RooMsgService::instance().setGlobalKillBelow(RooFit::WARNING);

   //If RooWorkspace already generated
//    TFile *file = new TFile("./workspace_PARA_par1[0.0,10.0,10]_par2[-20.0,20.0,20]_OBS_obs1[-45.0,45.0,45]_obs2[-30.0,30.0,60].root");
//    RooWorkspace *ww = (RooWorkspace *) file->Get("init_workspace");


   //----------Init weights histogram--------
//    TFile *fileh = new TFile("./histo_par1_par2.root");
//    TH2F *histo = (TH2F*) fileh->Get("fittedParameters");
   //TH2F *histo = (TH2F*) fileh->Get("dummy");

   model = new BackTrack::Simple2DModel_Binned();
   model->InitWorkspace();
   model->InitParObs();

   //===========================
   //==     "EXP" DATASET     ==
   //===========================

   //Generate data with correlated gaussian-distribution parameters
   model->SetNumGen(statexp);
   RooDataSet data("data", "data to be fitted", model->GetObservables());
   RooDataSet params("params", "input parameters", model->GetParameters());
   RooRealVar& PAR1 = *(model->GetParameter("par1"));
   RooRealVar& PAR2 = *(model->GetParameter("par2"));

   for (int i = 0; i < model->GetNumGen(); i++) {
      //Double_t par1 = gRandom->Gaus(3.5,1.);
      Int_t par1     = (int) gRandom->Gaus(3.5, 1.);
      Double_t par2m = 15. - 0.5 * pow(par1, 2.);
      Double_t par2 = gRandom->Gaus(par2m, 1.);
      PAR1.setVal(par1);
      PAR2.setVal(par2);
      model->generateEvent(RooArgList(PAR1, PAR2), data);
      params.add(model->GetParameters());
   }

   //Create the "experimental" RooDataHist used for the fit  with the RooDataSet
   //generated just before
   RooDataHist datahist("datahist_exp", "input parameters", model->GetObservables(), data);

//    Modify the ranges for the fit
//    model->GetObservable("obs1")->setRange("RANGE",-10,32);
//    model->GetObservable("obs2")->setRange("RANGE",-20,15);

   //===========================
   //==    BACKTRACKING       ==
   //===========================
   //Initialise the BackTracking
   model->SetExperimentalDataHist(datahist);
   model->SaveInitWorkspace();
   model->SetNumInt(kTRUE);
   model->SetExtended(kFALSE);
   model->SetUniformInitWeight(kTRUE);
   model->SetNumGen(statmod);
   model->ImportAllModelData();
   model->ImportParamInitWeight();
   model->ConstructPseudoPDF(kFALSE);
   //model->fitTo(datahist, Save(), NumCPU(6), SumW2Error(kTRUE), PrintLevel(1), Minimizer("TMinuit","migrad"), Extended(kFALSE), Offset(kTRUE));
   model->fitTo(datahist, Save(), NumCPU(6), SumW2Error(kTRUE), PrintLevel(1), Minimizer("TMinuit", "migrad"), Extended(kFALSE), Offset(kTRUE), SetMaxIter(500000), SetMaxCalls(500000));
   //model->fitTo(datahist, Save(), NumCPU(6), SumW2Error(kTRUE), PrintLevel(1), Minimizer("GSLMultiMin","conjugatefr"), Extended(kFALSE), Offset(kTRUE), SetMaxCalls(500000), SetMaxIter(500000));
   //model->fitTo(datahist, Save(), NumCPU(4), SumW2Error(kTRUE), PrintLevel(1), Minimizer("GSLSimAn"), Extended(kFALSE), Offset(kTRUE)); //, SetEpsilon(0.01), SetMaxCalls(500000));


   //==================
   //==    PLOTS     ==
   //==================
   //---------------Results compared to experimental data----------------
   RooPlot* plot = model->GetObservable("obs1")->frame();
   data.plotOn(plot);
   model->GetPseudoPDF()->plotOn(plot);
   TCanvas* c = new TCanvas("Observables_results", "Observables_results");
   c->Divide(2, 1);
   c->cd(1);
   plot->Draw();

   plot = model->GetObservable("obs2")->frame();
   data.plotOn(plot);
   model->GetPseudoPDF()->plotOn(plot);
   c->cd(2);
   plot->Draw();

   //For binning
   RooRealVar* o1 = dynamic_cast<RooRealVar*>(model->GetObservables().at(0));
   RooAbsBinning& bins1 = o1->getBinning();
   Int_t N1 = bins1.numBins();

   RooRealVar* o2 = dynamic_cast<RooRealVar*>(model->GetObservables().at(1));
   RooAbsBinning& bins2 = o2->getBinning();
   Int_t N2 = bins2.numBins();


   TH2* hh = (TH2*)(model->GetPseudoPDF())->createHistogram("obs1,obs2", N1, N2);
   TH2* hhdata = (TH2*) data.createHistogram("obs1,obs2", N1, N2);

   if (hh && hhdata) {
      TCanvas* ccc = new TCanvas("Observables_results_2D", "2D_Observables_results");
      ccc->Divide(2, 2);
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
      RooRealVar* p1 = dynamic_cast<RooRealVar*>(model->GetParameters().at(0));
      RooAbsBinning& bins1 = p1->getBinning();
      Int_t NN1 = bins1.numBins();

      RooRealVar* p2 = dynamic_cast<RooRealVar*>(model->GetParameters().at(1));
      RooAbsBinning& bins2 = p2->getBinning();
      Int_t NN2 = bins2.numBins();


      //-----------Return input parameters---------------
      TH2F* hh_par_data = (TH2F*) params.createHistogram("par1,par2", NN1, NN2) ;
      if (hh_par_data) hh_par_data->Write();
      else printf("impossible to create hh_par_data\n");

      //-------------Return fitted parameters (see Simple2DModel.h")------------
      TH2F* hh_par_fit = (TH2F*) model->GetParameterDistributions();
      if (hh_par_fit) hh_par_fit->Write();
      else printf("impossible to create hh_par_fit\n");

      //Draw
      TCanvas* cc = new TCanvas("Parameters_results_2D", "2D_Parameters_results");
      cc->Divide(2, 2);
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

      //------Chi2 for fit result
      Double_t chi2 = hh_par_data->Chi2Test(hh_par_fit, "CHI2/NDF,P");
      printf("chi2/ndf=%e\n", chi2);

   }

   else {
      printf("WTF, impossible to createHistogram from GetPseudoPDF ???\n");
   }

   //------------------------------------------------------------------------
   RooPlot* pl = model->GetParameter("par1")->frame();
   model->GetParamDataHist()->plotOn(pl);
   new TCanvas("Plot_result_par1", "Plot PAR1");
   pl->Draw();

   pl = model->GetParameter("par2")->frame();
   model->GetParamDataHist()->plotOn(pl);
   new TCanvas("Plot_result_par2", "Plot PAR2");
   pl->Draw();
}
#endif
