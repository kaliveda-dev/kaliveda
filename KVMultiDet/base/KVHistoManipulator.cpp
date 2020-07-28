//Created by KVClassFactory on Thu Oct 18 11:48:18 2007
//Author: Eric Bonnet

#include "KVHistoManipulator.h"
#include "Riostream.h"

#include "TProfile.h"
#include "TH1.h"
#include "TH2.h"

#include "TF1.h"
#include "TCutG.h"
#include "TList.h"
#include "TString.h"
#include "TClass.h"
#include "TRandom3.h"
#include "TGraph.h"
#include "TROOT.h"
#include "TMethodCall.h"
#include "TMath.h"

#include "KVNumberList.h"
#include "KVList.h"

#include "TSpline.h"
#include "TStyle.h"
#include "TCanvas.h"
#include "TMultiGraph.h"

#include <TObjString.h>

using namespace std;

ClassImp(KVHistoManipulator)

KVHistoManipulator* gHistoManipulator;

KVHistoManipulator::KVHistoManipulator()
{
   //Default constructor
   init();
   gHistoManipulator = this;
   fVDCanvas = nullptr;
}


KVHistoManipulator::~KVHistoManipulator()
{
   if (gHistoManipulator == this)
      gHistoManipulator = nullptr;
}

//###############################################################################################################"
//-------------------------------------------------
Int_t KVHistoManipulator::CutStatBin(TH1* hh, Int_t stat_min, Int_t stat_max)
{
//-------------------------------------------------
   // IMPORTANT l'histo est modifie
   //
   // Efface les bins dont la statistique est hors de l 'intervalle ]stat_min,stat_max[
   // l'erreur et le contenu sont mis a zero
   // le nombre d entree (GetEntries) reste inchange
   // pour les TH1 cela correspond a GetBinContent(xx)
   // pour les TH2 cela correspond a GetBinContent(xx,yy) --> cellules
   // pour les TProfile cela correspond a GetBinEntries(xx)
   // la fonction renvoie le nbre de bins ou cellules mis a zero
   // si stat_min ou stat_max sont egales à -1 la borne associée n'est pas prise en compte
   if (!hh) {
      cout << "pointeur histogramme nul" << endl;
      return -1;
   }
   Int_t nb_raz = 0;
   Bool_t raz = kFALSE;
   if (hh->InheritsFrom("TH2")) {

      for (Int_t nx = 1; nx <= hh->GetNbinsX(); nx += 1) {
         for (Int_t ny = 1; ny <= hh->GetNbinsY(); ny += 1) {
            raz = kFALSE;
            if (stat_min != -1 && hh->GetBinContent(nx, ny) < stat_min) raz = kTRUE;
            if (stat_max != -1 && hh->GetBinContent(nx, ny) > stat_max) raz = kTRUE;
            if (raz) {
               hh->SetBinContent(nx, ny, 0);
               hh->SetBinError(nx, ny, 0);
               nb_raz += 1;
            }
         }
      }
   }
   else if (hh->InheritsFrom("TProfile")) {
      TProfile* prof = (TProfile*)hh;
      for (Int_t nx = 1; nx <= prof->GetNbinsX(); nx += 1) {
         raz = kFALSE;
         if (stat_min != -1 && prof->GetBinEntries(nx) < stat_min) raz = kTRUE;
         if (stat_max != -1 && prof->GetBinEntries(nx) > stat_max) raz = kTRUE;
         if (raz) {
            prof->SetBinContent(nx, 0);
            prof->SetBinEntries(nx, 0);
            prof->SetBinError(nx, 0);
            nb_raz += 1;
         }
      }
   }
   else if (hh->InheritsFrom("TH1")) {
      for (Int_t nx = 1; nx <= hh->GetNbinsX(); nx += 1) {
         raz = kFALSE;
         if (stat_min != -1 && hh->GetBinContent(nx) < stat_min) raz = kTRUE;
         if (stat_max != -1 && hh->GetBinContent(nx) > stat_max) raz = kTRUE;
         if (raz) {
            hh->SetBinContent(nx, 0);
            hh->SetBinError(nx, 0);
            nb_raz += 1;
         }
      }
   }

   return nb_raz;
}

//###############################################################################################################"
//-------------------------------------------------
Int_t KVHistoManipulator::Apply_TCutG(TH2* hh, TCutG* cut, TString mode)
{
//-------------------------------------------------
   // IMPORTANT l'histo est modifie
   //
   // Applique une selection sur un histo 2D a partir d'un TCutG;
   // Si mode=In seules les cellules comprises dans le TCutG sont gardes
   // mode=Out --> Inverse
   // la fonction renvoie le nbre de cellules mis a zero
   // Attention le test ne se fait que sur les valeurs centrales des cellules (GetBinCenter())

   if (!hh) {
      cout << "pointeur histogramme nul" << endl;
      return -1;
   }
   Int_t nb_raz = 0;
   Bool_t raz = kFALSE;

   for (Int_t nx = 1; nx <= hh->GetNbinsX(); nx += 1) {
      for (Int_t ny = 1; ny <= hh->GetNbinsY(); ny += 1) {
         raz = kFALSE;
         Double_t valx = hh->GetXaxis()->GetBinCenter(nx);
         Double_t valy = hh->GetYaxis()->GetBinCenter(ny);
         if (mode == "in" && !cut->IsInside(valx, valy)) raz = kTRUE;
         if (mode == "out" && cut->IsInside(valx, valy)) raz = kTRUE;
         if (raz) {
            hh->SetBinContent(nx, ny, 0);
            hh->SetBinError(nx, ny, 0);
            nb_raz += 1;
         }
      }
   }

   return nb_raz;
}

//###############################################################################################################
//-------------------------------------------------
TH1* KVHistoManipulator::ScaleHisto(TH1* hh, TF1* fx, TF1* fy, Int_t nx, Int_t ny, Double_t xmin, Double_t xmax, Double_t ymin, Double_t ymax, Option_t* norm)
{
//-------------------------------------------------
   // Applique les transformations correspondantes aux fonctions (TF1 *) donnees en argument
   // et donne en sortie l histogramme transforme celui ci a pour nom "nom_de_histo_input"_scaled
   //
   // IMPORTANT l'histo d'entree n'est pas modifie
   //
   // pour TH1 et TProfile n'est pris en compte que TF1 *fx;
   // les parametres de la fonction doivent etre initialises avant.
   // Si la fonction est un pointeur NULL, aucune transformation n est appliquee et l axe reste tel quel.
   // L'intervalle de validite des fonctions TF1::SetRange est determine a partir des limites de l histo apres transformations
   // nx xmin et xmax sont les parametres de binnage des histo
   // si nx=-1, ceux ci sont recalcules automatiquement nx = nx_initial, xmin = fx(xmin), xmax = fx(xmax)
   // si nx!=-1, xmin et xmax doivent etre aussi specifies
   // il en est de meme pour l'axe y
   //
   // OPTION: norm
   //  norm = "" (default) : no adjustment is made for the change in bin width due to the transformation
   //  norm = "width" : bin contents are adjusted for width change, so that the integral of the histogram
   //                   contents taking into account the bin width (i.e. TH1::Integral("width")) is the same.
   //
   // NOTE for 1D histos:
   // If the binning of the scaled histogram is imposed by the user (nx!=-1), then in order
   // to achieve a continuous scaled distribution we have to randomize X within each bin.
   // This will only work if the bin contents of 'hh' are integer numbers, i.e. unnormalised raw data histogram.

   Bool_t width = !strcmp(norm, "width");
   TH1* gg = nullptr;
   Double_t abs;
   Bool_t fixed_bins = (nx != -1);
   if (fx) {
      if (nx == -1) {
         nx = hh->GetNbinsX();
         abs = hh->GetXaxis()->GetBinLowEdge(1);
         xmin = fx->Eval(abs);
         abs = hh->GetXaxis()->GetBinUpEdge(hh->GetNbinsX());
         xmax = fx->Eval(abs);
      }
      fx->SetRange(hh->GetXaxis()->GetBinLowEdge(1), hh->GetXaxis()->GetBinUpEdge(nx));
      fx->SetNpx(nx + 1);
   }
   else {
      nx = hh->GetNbinsX();
      xmin = hh->GetXaxis()->GetBinLowEdge(1);
      xmax = hh->GetXaxis()->GetBinUpEdge(hh->GetNbinsX());
   }

   if (hh->InheritsFrom("TH2")) {
      if (fy) {
         if (ny == -1) {
            ny = hh->GetNbinsY();
            abs = hh->GetYaxis()->GetBinLowEdge(1);
            ymin = fy->Eval(abs);
            abs = hh->GetYaxis()->GetBinUpEdge(hh->GetNbinsY());
            ymax = fy->Eval(abs);
         }
         fy->SetRange(hh->GetYaxis()->GetBinLowEdge(1), hh->GetYaxis()->GetBinUpEdge(ny));
         fy->SetNpx(ny + 1);
      }
      else {
         ny = hh->GetNbinsY();
         ymin = hh->GetYaxis()->GetBinLowEdge(1);
         ymax = hh->GetYaxis()->GetBinUpEdge(hh->GetNbinsY());
      }
   }

   TClass* clas = TClass::GetClass(hh->ClassName());
   gg = (TH1*) clas->New();
   if (!gg) return nullptr;
   TString hname;
   hname.Form("%s_scaled", hh->GetName());
   gg->SetNameTitle(hname.Data(), hh->GetTitle());

   if (hh->InheritsFrom("TH2"))  gg->SetBins(nx, xmin, xmax, ny, ymin, ymax);
   else                          gg->SetBins(nx, xmin, xmax);

   // if option norm="width', take account of change of X bin width between original and scaled histograms
   Double_t Xbin_width_corr = 1.0, Ybin_width_corr = 1.0;
   if (width) {
      Double_t orig_Xbin_width = (hh->GetXaxis()->GetXmax() - hh->GetXaxis()->GetXmin()) / hh->GetNbinsX();
      Double_t new_Xbin_width = (gg->GetXaxis()->GetXmax() - gg->GetXaxis()->GetXmin()) / gg->GetNbinsX();
      Xbin_width_corr = orig_Xbin_width / new_Xbin_width;
      if (hh->InheritsFrom("TH2")) {
         // Take account of change of X bin width between original and scaled histograms
         Double_t orig_Ybin_width = (hh->GetYaxis()->GetXmax() - hh->GetYaxis()->GetXmin()) / hh->GetNbinsY();
         Double_t new_Ybin_width = (gg->GetYaxis()->GetXmax() - gg->GetYaxis()->GetXmin()) / gg->GetNbinsY();
         Ybin_width_corr = orig_Ybin_width / new_Ybin_width;
      }
   }

   for (Int_t xx = 1; xx <= hh->GetNbinsX(); xx += 1) {
      Double_t bmin = hh->GetXaxis()->GetBinLowEdge(xx);
      Double_t bmax = hh->GetXaxis()->GetBinUpEdge(xx);
      abs  = gRandom->Uniform(bmin, bmax);
      if (abs == bmax) abs = bmin;
      Double_t resx = abs;
      if (fx) resx = fx->Eval(abs);
      if (hh->InheritsFrom("TH2")) {
         for (Int_t yy = 1; yy <= hh->GetNbinsY(); yy += 1) {
            if (hh->GetBinContent(xx, yy) > 0) {
               bmin = hh->GetYaxis()->GetBinLowEdge(yy);
               bmax = hh->GetYaxis()->GetBinUpEdge(yy);
               abs  = gRandom->Uniform(bmin, bmax);
               if (abs == bmax) abs = bmin;
               Double_t resy = abs;
               if (fy) resy = fy->Eval(abs);
               gg->SetBinContent(gg->GetXaxis()->FindBin(resx),
                                 gg->GetYaxis()->FindBin(resy),
                                 hh->GetBinContent(xx, yy)*Xbin_width_corr * Ybin_width_corr);
               //gg->SetBinError(gg->GetXaxis()->FindBin(resx),gg->GetYaxis()->FindBin(resy),hh->GetBinError(xx,yy));
            }
         }
      }
      else {
         // 1-D histogram
         if (fixed_bins) {
            // histogram binning imposed by user. we need to randomise inside bins of original histogram
            // otherwise scaled histogram will be discontinuously filled.
            Int_t nmax = (Int_t)hh->GetBinContent(xx);
            for (int i = 0; i < nmax; i++) {
               abs  = gRandom->Uniform(bmin, bmax);
               Double_t resx = fx->Eval(abs);
               gg->Fill(resx, Xbin_width_corr);
               //cout << resx << "  " << Xbin_width_corr << endl;
            }
         }
         else {
            // range and binning calculated from function.
            Double_t resy = hh->GetBinContent(xx);
            if (fy) resy = fy->Eval(resy);
            gg->SetBinContent(gg->GetXaxis()->FindBin(resx), resy * Xbin_width_corr);
         }
      }
   }
   return gg;
}

//###############################################################################################################
//-------------------------------------------------
TGraph* KVHistoManipulator::ScaleGraph(TGraph* hh, TF1* fx, TF1* fy)
{
//-------------------------------------------------
   // Applique les transformations correspondantes aux fonctions (TF1 *) donnees en argument
   // et donne en sortie le graph transforme celui ci a pour nom "nom_de_histo_input"_scaled
   //
   // IMPORTANT le graph d'entree n'est pas modifie
   //
   // les parametres de la fonction doivent etre initialises avant.
   // Si la fonction est un pointeur NULL, aucune transformation n est appliquee et l axe reste tel quel.

   TGraph* gg = nullptr;
   TClass* clas = TClass::GetClass(hh->ClassName());
   gg = (TGraph*) clas->New();
   if (!gg) return nullptr;
   TString hname;
   hname.Form("%s_scaled", hh->GetName());
   gg->SetNameTitle(hname.Data(), hh->GetTitle());

   Int_t np = hh->GetN();
   for (Int_t nn = 0; nn < np; nn += 1) {
      Double_t xx1, yy1;
      hh->GetPoint(nn, xx1, yy1);
      Double_t xx2 = xx1;
      if (fx) xx2 = fx->Eval(xx1);
      Double_t yy2 = yy1;
      if (fy) yy2 = fy->Eval(yy1);
      gg->SetPoint(nn, xx2, yy2);
      if (gg->InheritsFrom("TGraphErrors")) {
         // transformation of errors
         // if f = f(x) the error on f, e_f, for a given error on x, e_x, is
         //    e_f  =  abs0(df/dx) * e_x
         Double_t e_x = ((TGraphErrors*)hh)->GetErrorX(nn);
         Double_t e_y = ((TGraphErrors*)hh)->GetErrorY(nn);
         if (fx) e_x = TMath::Abs(fx->Derivative(xx1)) * e_x;
         if (fy) e_y = TMath::Abs(fy->Derivative(yy1)) * e_y;
         ((TGraphErrors*)gg)->SetPointError(nn, e_x, e_y);
      }
   }

   return gg;

}
//###############################################################################################################
//-------------------------------------------------
TH1* KVHistoManipulator::CentreeReduite(TH1* hh, Int_t nx, Int_t ny, Double_t xmin, Double_t xmax, Double_t ymin, Double_t ymax)
{
//-------------------------------------------------
   //Exemple d utilisation de la methode KVHistoManipulator::ScaleHisto avec ici
   //L'obtention des distributions centrees reduites

   if (!hh) {
      cout << "pointeur histogramme nul" << endl;
      return hh;
   }

   TString expression;
   expression.Form("(x-%lf)/%lf", hh->GetMean(1), hh->GetRMS(1));
   TF1 fx("fx", expression.Data());
   unique_ptr<TF1> fy;
   if (hh->InheritsFrom("TH2")) {
      expression.Form("(x-%lf)/%lf", hh->GetMean(2), hh->GetRMS(2));
      fy.reset(new TF1("fy", expression.Data()));
   }

   TH1* gg = ScaleHisto(hh, &fx, fy.get(), nx, ny, xmin, xmax, ymin, ymax);
   TString hname;
   hname.Form("%s_centred", hh->GetName());
   gg->SetName(hname.Data());

   return gg;
}
//###############################################################################################################
//-------------------------------------------------
TH2* KVHistoManipulator::CentreeReduiteX(TH2* hh, Int_t nx, Double_t xmin, Double_t xmax)
{
//-------------------------------------------------
   //Exemple d utilisation de la methode KVHistoManipulator::ScaleHisto avec ici
   //L'obtention des distributions centrees reduites

   if (!hh) {
      cout << "pointeur histogramme nul" << endl;
      return hh;
   }

   TString expression;
   expression.Form("(x-%lf)/%lf", hh->GetMean(1), hh->GetRMS(1));
   TF1 fx("fx", expression.Data());
   TH2* gg = (TH2*)ScaleHisto(hh, &fx, nullptr, nx, -1, xmin, xmax, -1., -1.);
   TString hname;
   hname.Form("%s_centred_X", hh->GetName());
   gg->SetName(hname.Data());

   return gg;
}
//###############################################################################################################
//-------------------------------------------------
TH2* KVHistoManipulator::CentreeReduiteY(TH2* hh, Int_t ny, Double_t ymin, Double_t ymax)
{
//-------------------------------------------------
   //Exemple d utilisation de la methode KVHistoManipulator::ScaleHisto avec ici
   //L'obtention des distributions centrees reduites

   if (!hh) {
      cout << "pointeur histogramme nul" << endl;
      return hh;
   }

   TString expression;
   expression.Form("(x-%lf)/%lf", hh->GetMean(1), hh->GetRMS(1));
   TF1 fy("fy", expression.Data());
   TH2* gg = (TH2*)ScaleHisto(hh, nullptr, &fy, -1, ny, -1., -1., ymin, ymax);
   TString hname;
   hname.Form("%s_centred_Y", hh->GetName());
   gg->SetName(hname.Data());

   return gg;
}

//###############################################################################################################
//-------------------------------------------------
TH2* KVHistoManipulator::RenormaliseHisto(TH2* hh, Int_t bmin, Int_t bmax, TString axis, Double_t valref)
{
//-------------------------------------------------
   // Renormalisation de l histogramme 2D sous la contrainte d'une ditribution plate suivant X ou Y
   // (signal de bimodalite par exemple)
   // et donne en sortie l histogramme transforme celui ci a pour nom "nom_de_histo_input"_normalised
   //
   // IMPORTANT l'histo d'entree n'est pas modifie
   //
   // bmin, bmax : intervalle de bins ou l'on effectue la renormalisation (par defaut -1,-1 --> toute la plage)
   // les contenus hors de l intervalle [bmin,bmax] sont remis a zero
   // Si on veut une distribution plate en X ils indiquent l'intervalle sur cet axe x
   // valref : valeur en stat de la distribution plate (par defaut 1)
   //
   // ATTENTION la propagation des erreurs n est pas (pour l instant) implementee

   if (!hh) {
      cout << "pointeur histogramme nul" << endl;
      return hh;
   }

   if (bmin == -1) bmin = 1;
   TString hname;
   hname.Form("%s_normalised", hh->GetName());
   TH2* clone = 0;
   if ((clone = (TH2*)gDirectory->Get(hname.Data()))) delete clone;
   clone = (TH2*)hh->Clone(hname.Data());
   clone->Reset();

   if (axis == "X") {
      if (bmax == -1) {
         bmax = hh->GetNbinsX();
      }
      for (Int_t nx = bmin; nx <= bmax; nx += 1) {
         Double_t integ = 0;
         for (Int_t ny = 1; ny <= hh->GetNbinsY(); ny += 1) integ += hh->GetBinContent(nx, ny);
         if (integ > 0) {
            for (Int_t ny = 1; ny <= hh->GetNbinsY(); ny += 1) {
               clone->SetBinContent(nx, ny, hh->GetBinContent(nx, ny)*valref / integ);
               if (hh->GetBinContent(nx, ny) > 0) {
                  Double_t erreur = clone->GetBinContent(nx, ny) * TMath::Sqrt(1. / hh->GetBinContent(nx, ny) + 1. / integ);
                  clone->SetBinError(nx, ny, erreur);
               }
            }
         }
      }
   }
   else if (axis == "Y") {
      if (bmax == -1) {
         bmax = hh->GetNbinsY();
      }
      for (Int_t ny = bmin; ny <= bmax; ny += 1) {
         Double_t integ = 0;
         for (Int_t nx = 1; nx <= hh->GetNbinsX(); nx += 1) integ += hh->GetBinContent(nx, ny);
         if (integ > 0) {
            for (Int_t nx = 1; nx <= hh->GetNbinsX(); nx += 1) {
               clone->SetBinContent(nx, ny, hh->GetBinContent(nx, ny)*valref / integ);
               Double_t erreur = clone->GetBinContent(nx, ny) * TMath::Sqrt(1. / hh->GetBinContent(nx, ny) + 1. / integ);
               clone->SetBinError(nx, ny, erreur);
            }
         }
      }
   }
   else {
      cout << "l option TString axis doit etre X ou Y" << endl;
   }
   return clone;
}
//###############################################################################################################
//-------------------------------------------------
TH2* KVHistoManipulator::RenormaliseHisto(TH2* hh, Double_t valmin, Double_t valmax, TString axis, Double_t valref)
{
//-------------------------------------------------
   // Les bornes valmin, valmax definissent l'intervalle ou l on effectue la renormalisation
   // On en derive les valeurs de bins
   // pour la methode KVHistoManipulator::RenormaliseHisto(TH1 *hh,TString axis,Double_t valref,Int_t bmin,Int_t bmax)

   if (!hh) {
      cout << "pointeur histogramme nul" << endl;
      return hh;
   }
   if (axis == "X")       return RenormaliseHisto(hh, hh->GetXaxis()->FindBin(valmin), hh->GetXaxis()->FindBin(valmax), axis, valref);
   else if (axis == "Y")  return RenormaliseHisto(hh, hh->GetYaxis()->FindBin(valmin), hh->GetYaxis()->FindBin(valmax), axis, valref);
   else {
      cout << "l option TString axis doit etre X ou Y" << endl;
      return nullptr;
   }

}

//###############################################################################################################
//-------------------------------------------------
TH1*  KVHistoManipulator::CumulatedHisto(TH1* hh, TString direction, Int_t bmin, Int_t bmax, Option_t* norm)
{
//-------------------------------------------------
   // Cumule le contenu de l histo hh entre bin bmin et bmax et retourne l histo correspondant
   // Si direction="C" (default):
   // SetBinContent(n) = GetBinContent(bmin)+GetBinContent(bmin+1)+ ... + GetBinContent(n)
   // Si direction="D":
   // SetBinContent(n) = GetBinContent(bmax)+GetBinContent(bmax-1)+ ... + GetBinContent(n)
   //
   // Donne en sortie l histogramme transforme celui ci a pour nom "nom_de_histo_input"_cumulated
   //
   // IMPORTANT l'histo d'entree n'est pas modifie
   //
   // si bmin=-1, bmin=1
   // si bmax=-1, bmax=GetNbinsX()
   //
   // Avec norm = "surf" (default) l'integral de l histo cumule est egale a 1
   // Avec norm = "max" le contenu de l'histogram est renormalise de facon a ce que le maximum soit 1

   if (!hh) {
      cout << "pointeur histogramme nul" << endl;
      return hh;
   }
   direction.ToUpper();
   if (direction != "C" && direction != "D") {
      cout << "l option TString direction doit etre C ou D" << endl;
      return nullptr;
   }
   if (hh->GetDimension() == 1) {
      if (bmin < 1) bmin = 1;
      if (bmax < 1) bmax = hh->GetNbinsX();
      TString hname;
      hname.Form("%s_cumulated", hh->GetName());
      TH1* clone = (TH1*)hh->Clone(hname.Data());
      clone->Reset();
      // get array of cumulated bins
      Double_t* integral = hh->GetIntegral();
      if (direction == "C") {
         /* "standard" cumulative histogram, from bin 'bmin' to bin 'bmax' */
         Double_t offset = integral[bmin - 1], rescale = 1.0;
         if (bmax < hh->GetNbinsX() && integral[bmax] > offset) rescale = 1. / (integral[bmax] - offset);
         for (Int_t nx = 1; nx <= hh->GetNbinsX(); nx += 1) {
            if (nx >= bmin && nx <= bmax) clone->SetBinContent(nx, rescale * (integral[nx] - offset));
            else if (nx > bmax) clone->SetBinContent(nx, 1.0);
            else if (nx < bmin) clone->SetBinContent(nx, 0.0);
         }
      }
      else {
         /* "reverse" cumulative histogram, from bin 'bmax' to bin 'bmin' */
         Double_t offset = integral[bmax], rescale = 1.0;
         if (integral[bmin - 1] < offset) rescale = 1. / (offset - integral[bmin - 1]);
         for (Int_t nx = 1; nx <= hh->GetNbinsX(); nx += 1) {
            if (nx >= bmin && nx <= bmax) clone->SetBinContent(nx, rescale * (offset - integral[nx - 1]));
            else if (nx > bmax) clone->SetBinContent(nx, 0.0);
            else if (nx < bmin) clone->SetBinContent(nx, 1.0);
         }
      }
      // normalisation
      if (!strcmp(norm, "surf")) {
         Double_t sw = clone->GetSumOfWeights();
         if (sw > 0) clone->Scale(1. / sw);
      }
      else if (!strcmp(norm, "max")) {
         Double_t max = clone->GetMaximum();
         if (max > 0) clone->Scale(1. / max);
      }
      return clone;
   }
   else {
      cout << "cette methode n est pas prevue pour les TH2, TH3" << endl;
      return nullptr;
   }

}

//###############################################################################################################
//-------------------------------------------------
TH1*  KVHistoManipulator::GetDerivative(TH1* hh, Int_t order)
{
//-------------------------------------------------
   // retourne la derivee d ordre 0, 1 ou 2 d'un histogramme
   // 0 -> correspond a un lissage (smooth) de la distribution
   // 1 et 2 correspondent aux derivees premieres et deuxiemes
   //
   // 0 -> derivee zero Yi = 1/35*(-3yi-2 + 12yi-1 +17yi +12yi1 -3yi2)
   // 1 -> derivee premiere Y'i = 1/12h*(yi-2 - 8yi-1 +8yi1 -1yi2)
   // 2 -> derivee seconde Y''i = 1/7h/h*(2yi-2 -1 yi-1 -2yi -1yi1 +2yi2)
   // les derivees pour les bins 1,2 et n-1,n ne sont pas calculees
   //
   // IMPORTANT l'histo d'entree n'est pas modifie

   if (!hh) {
      cout << "pointeur histogramme nul" << endl;
      return hh;
   }
   if (!(0 <= order && order <= 2)) {
      cout << "ordre " << order << "n est pas implemente" << endl;
      return nullptr;
   }
   if (hh->GetDimension() == 1) {

      TString hname;
      hname.Form("%s_derivated_%d", hh->GetName(), order);
      TH1* clone = 0;

      if (hh->InheritsFrom("TProfile"))   clone = new TH1F(hname.Data(), hh->GetTitle(), hh->GetNbinsX(), hh->GetBinLowEdge(1), hh->GetBinLowEdge(hh->GetNbinsX() + 1));
      else                                clone = (TH1*)hh->Clone(hname.Data());

      clone->Reset();

      for (Int_t nx = 3; nx <= hh->GetNbinsX() - 2; nx += 1) {
         Double_t dev = 0;
         if (order == 0) {
            dev = 1. / 35.*(
                     -3 * hh->GetBinContent(nx - 2)
                     + 12 * hh->GetBinContent(nx - 1)
                     + 17 * hh->GetBinContent(nx)
                     + 12 * hh->GetBinContent(nx + 1)
                     - 3 * hh->GetBinContent(nx + 2)
                  );
         }
         else if (order == 1) {
            Double_t h = hh->GetBinWidth(1);
            dev = 1 / 12. / h * (
                     1 * hh->GetBinContent(nx - 2)
                     - 8 * hh->GetBinContent(nx - 1)
                     + 0 * hh->GetBinContent(nx)
                     + 8 * hh->GetBinContent(nx + 1)
                     - 1 * hh->GetBinContent(nx + 2)
                  );
         }
         else {
            Double_t h2 = pow(hh->GetBinWidth(1), 2.);
            dev = 1 / 7. / h2 * (
                     2 * hh->GetBinContent(nx - 2)
                     - 1 * hh->GetBinContent(nx - 1)
                     - 2 * hh->GetBinContent(nx)
                     - 1 * hh->GetBinContent(nx + 1)
                     + 2 * hh->GetBinContent(nx + 2)
                  );
         }
         clone->SetBinContent(nx, dev);
      }
      return clone;
   }
   else {
      cout << "cette methode n est pas prevue pour les TH2, TH3" << endl;
      return nullptr;
   }

}

//###############################################################################################################
//-------------------------------------------------
TGraphErrors*  KVHistoManipulator::GetMomentEvolution(TH2* hh, TString momentx, TString momenty, TString axis, Double_t stat_min)
{
//-------------------------------------------------
   // Renvoie un TGraph
   // A partir d'un 2D histo, permet de tracer l'evolution d'un moment en fonction d'un autre moment
   // ou d'un moment d'un axe en fonction de l'aure axe.
   // Les TString momentx et momenty doivent etre des "Get like" methodes connues de TH1
   // comme GetMean, GetRMS ou GetKurtosis :
   // - Si momenty="" -> on obtient l'evolution du moment momentx en fonction de
   // la variable associée a l'autre axe.
   // - Si momenty!="" -> on obtient l'evolution du moment momenty en fonction du momentx
   //
   // Si TString axis = X la variable en X est le parametre d echantillonage et vice-versa
   //
   // Exemple :
   // GetMomentEvolution(histo,"GetMean","GetSkewness","X") -> Evolution de la skewness en fonction de la moyenne de l'observable
   // en Y par tranche de l'observable X

   if (axis != "X" && axis != "Y") {
      cout << "GetMomentEvolution(TH2*,TString ,TString ,TString) Mauvaise syntaxe pour TString axis (X ou Y) " << endl;
      return nullptr;
   }
   TMethodCall cmx;
   cmx.InitWithPrototype(TClass::GetClass("TH1D"), Form("%s", momentx.Data()), "int");
   if (!cmx.IsValid()) {
      cout << "GetMomentEvolution(TH2*,TString ,TString ,TString) TString momentx n'est pas une methode valide " << momentx.Data() << endl;
      return nullptr;
   }
   unique_ptr<TMethodCall> Ecmx(new TMethodCall());
   Ecmx->InitWithPrototype(TClass::GetClass("TH1D"), Form("%sError", momentx.Data()), "int");

   unique_ptr<TMethodCall> cmy, Ecmy;
   if (momenty != "")  {
      cmy.reset(new TMethodCall());
      cmy->InitWithPrototype(TClass::GetClass("TH1D"), Form("%s", momenty.Data()), "int");
      if (!cmy->IsValid()) {
         cout << "GetMomentEvolution(TH2*,TString ,TString ,TString) TString momenty n'est pas une methode valide " << momenty.Data() << endl;
         return nullptr;
      }
      Ecmy.reset(new TMethodCall());
      Ecmy->InitWithPrototype(TClass::GetClass("TH1D"), Form("%sError", momenty.Data()), "int");
   }

   TString fmt_histo;
   fmt_histo.Form("GetMomentEvolution_%s", hh->GetName());
   KVNumberList lbins = "";
   Int_t nmax = 0;
   if (axis == "Y") nmax = hh->GetNbinsX();
   else nmax = hh->GetNbinsY();

   Int_t npts = 0;
   for (Int_t nn = 1; nn <= nmax; nn += 1) {
      Double_t stat = 0;
      if (axis == "Y") stat = ((TH1D*)hh->ProjectionY(fmt_histo.Data(), nn, nn))->Integral();
      else           stat = ((TH1D*)hh->ProjectionX(fmt_histo.Data(), nn, nn))->Integral();
      if (stat > stat_min) {
         npts += 1;
         lbins.Add(nn);
      }
      gDirectory->Delete(fmt_histo.Data());
   }
   if (npts > 0) {
      TGraphErrors* gr = new TGraphErrors(npts);
      TH1D* hp;
      npts = 0;
      Double_t valx, valy, Evaly = 0, Evalx = 0;
      lbins.Begin();
      while (!lbins.End()) {
         Int_t bin = lbins.Next();
         if (axis == "Y") hp = (TH1D*)hh->ProjectionY(fmt_histo.Data(), bin, bin);
         else           hp = (TH1D*)hh->ProjectionX(fmt_histo.Data(), bin, bin);
         cmx.Execute(hp, "1", valx);
         if (Ecmx->IsValid()) Ecmx->Execute(hp, "1", Evalx);
         if (momenty != "") {
            cmy->Execute(hp, "1", valy);
            if (Ecmy->IsValid()) Ecmy->Execute(hp, "1", Evaly);
            gr->SetPoint(npts, valx, valy);
            if (Evalx != 0 && Evaly != 0) gr->SetPointError(npts, Evalx, Evaly);
            npts += 1;
         }
         else {
            if (axis == "Y") {
               valy = hh->GetXaxis()->GetBinCenter(bin);
               Evaly = hh->GetXaxis()->GetBinWidth(bin) / 2.;
            }
            else           {
               valy = hh->GetYaxis()->GetBinCenter(bin);
               Evaly = hh->GetYaxis()->GetBinWidth(bin) / 2.;
            }
            gr->SetPoint(npts, valy, valx);
            if (Evalx != 0) gr->SetPointError(npts, Evaly, Evalx);
            npts += 1;
         }
         gDirectory->Delete(fmt_histo.Data());
      }
      return gr;
   }
   else {
      cout << "GetMomentEvolution(TH2*,TString ,TString ,TString) Aucun point dans le TGraph" << endl;
      return nullptr;
   }

}


//-------------------------------------------------
TGraph* KVHistoManipulator::LinkGraphs(TGraph* grx, TGraph* gry)
{
//-------------------------------------------------
   // A partir de deux graphs ayant le meme nombre de points et le meme axe X,
   // cette methode produit un graph correspondant a la correlation entre les deux axes Y
   // Les inputs peuvent etre aussi bien des TGraph que des TGraphErrors dans ce dernier cas
   // les barres d erreurs sont prises en compte

   Double_t* yy = gry->GetY();
   Double_t* xx = grx->GetY();
   Double_t* ey = 0;
   Double_t* ex = 0;

   if (grx->GetN() != gry->GetN()) {
      printf("ERREUR : KVHistoManipulator::LinkGraphs : les deux graphs n ont pas le meme nbre de points\n");
      return 0;
   }
   Int_t npoints = grx->GetN();

   TGraph* corre = 0;
   if (grx->InheritsFrom("TGraphErrors") || gry->InheritsFrom("TGraphErrors")) {
      if (grx->InheritsFrom("TGraphErrors")) ex = grx->GetEX();
      if (gry->InheritsFrom("TGraphErrors")) ey = gry->GetEY();
      corre = new TGraphErrors(npoints, xx, yy, ex, ey);
   }
   else corre = new TGraph(npoints, xx, yy);

   TString name;
   name.Form("corre_%s_VS_%s", gry->GetName(), grx->GetName());
   corre->SetNameTitle(name.Data(), grx->GetTitle());

   return corre;

}


//###############################################################################################################
//-------------------------------------------------
Double_t KVHistoManipulator::GetCorrelationFactor(TH2* hh)
{
//-------------------------------------------------
   // Calcul le coefficient de correlation entre les variables X et Y
   // d'un bidim... Equivalent a TH2F::GetCorrelationFactor() de ROOT
   Double_t sumxy = 0;
   Double_t sumx = 0, sumx2 = 0;
   Double_t sumy = 0, sumy2 = 0;

   Double_t compt = 0;
   for (Int_t nx = 1; nx <= hh->GetNbinsX(); nx += 1) {
      for (Int_t ny = 1; ny <= hh->GetNbinsY(); ny += 1) {
         compt += hh->GetBinContent(nx, ny);
         sumxy += hh->GetBinContent(nx, ny) * hh->GetXaxis()->GetBinCenter(nx) * hh->GetYaxis()->GetBinCenter(ny);
         sumx  += hh->GetBinContent(nx, ny) * hh->GetXaxis()->GetBinCenter(nx);
         sumy  += hh->GetBinContent(nx, ny) * hh->GetYaxis()->GetBinCenter(ny);
         sumx2 += hh->GetBinContent(nx, ny) * pow(hh->GetXaxis()->GetBinCenter(nx), 2.);
         sumy2 += hh->GetBinContent(nx, ny) * pow(hh->GetYaxis()->GetBinCenter(ny), 2.);
      }
   }

   Double_t meanxy = sumxy / compt;
   Double_t meanx = sumx / compt;
   Double_t meany = sumy / compt;
   Double_t meanx2 = sumx2 / compt;
   Double_t sigmax = sqrt(meanx2 - pow(meanx, 2.));
   Double_t meany2 = sumy2 / compt;
   Double_t sigmay = sqrt(meany2 - pow(meany, 2.));

   Double_t rho = (meanxy - meanx * meany) / (sigmax * sigmay);
   return rho;

}
//###############################################################################################################"
//-------------------------------------------------
KVList* KVHistoManipulator::Give_ProjectionList(TH2* hh, Double_t MinIntegral, TString axis)
{
//-------------------------------------------------
   // Retourne une liste contenant les projections par tranche de l'axe (TString axis="X" ou "Y")
   // remplissant une condition leur integral qui doit etre superieur à MinIntegral (=-1 par defaut)
   // si axis="X", les projections sur l'axe Y de l'histogramme est fait pour chaque bin de l'axe X

   if (!hh) {
      cout << "pointeur histogramme nul" << endl;
      return NULL;
   }
   TH1D* h1d = 0;
   TString proj_name;
   if (hh->InheritsFrom("TH2")) {
      KVList* list = new KVList();
      cout << "TH2" << endl;
      if (axis == "X") {
         for (Int_t nx = 1; nx <= hh->GetNbinsX(); nx += 1) {
            Double_t integ = 0;
            for (Int_t ny = 1; ny <= hh->GetNbinsY(); ny += 1)
               integ += hh->GetBinContent(nx, ny);

            if (integ > MinIntegral) {
               proj_name.Form("%s_bX_%d", hh->GetName(), nx);
               h1d = hh->ProjectionY(proj_name.Data(), nx, nx);
               h1d->SetTitle(Form("%lf", hh->GetXaxis()->GetBinCenter(nx)));
               list->Add(h1d);
            }

         }
      }
      else if (axis == "Y") {
         for (Int_t ny = 1; ny <= hh->GetNbinsY(); ny += 1) {
            Double_t integ = 0;
            for (Int_t nx = 1; nx <= hh->GetNbinsX(); nx += 1)
               integ += hh->GetBinContent(nx, ny);

            if (integ > MinIntegral) {
               proj_name.Form("%s_bY_%d", hh->GetName(), ny);
               h1d = hh->ProjectionX(proj_name.Data(), ny, ny);
               h1d->SetTitle(Form("%lf", hh->GetYaxis()->GetBinCenter(ny)));
               list->Add(h1d);
            }
         }
      }
      else {
         cout << "l option TString axis doit etre X ou Y" << endl;
      }

      return list;
   }
   else {
      cout << "cette methode n est prevue que pour les TH2 and sons" << endl;
      return NULL;
   }

}

//-------------------------------------------------
KVNumberList* KVHistoManipulator::Saucisson(TH1* hh, Int_t ntranches)
{
//-------------------------------------------------
   // Donne les valeurs permettant de decouper l'integral
   // d'un histograme en tranche de 10% de la stat total
   // retourne une KVNumberList indiquant les delimitation des tranches
   //
   // L'utilisateur doit effacer cette liste apres utilisation
   //

   if (!hh) {
      cout << "pointeur histogramme nul" << endl;
      return NULL;
   }

   TString proj_name;
   Double_t integral = 0;
   for (Int_t nx = 1; nx <= hh->GetXaxis()->GetNbins(); nx += 1) {
      integral += hh->GetBinContent(nx);
   }

   KVNumberList* nl = new KVNumberList();

   Double_t tranche = 0;
   printf("integral du spectre %lf -> tranche de %lf\n", integral, integral / ntranches);

   Int_t nt = 0;
   for (Int_t nx = hh->GetXaxis()->GetNbins(); nx >= 1; nx -= 1) {
      tranche += hh->GetBinContent(nx);
      //printf("nx=%d, nt=%d, tranche[nt]=%lf -> %lf\n",nx,nt,tranche,integral/ntranches);
      if (tranche >= integral / ntranches) {

         nt += 1;
         //printf("\tnouvelle tranche %d %d\n",nx,nt);
         nl->Add(nx);
         tranche = 0;

      }
   }

   //Verif
   /*
   Int_t sup=1;
   Int_t inf = 1;
   nl->Begin();
   while (!nl->End()){
      sup = nl->Next();
      printf("%d %d - %lf -> %lf\n",inf,sup,hh->Integral(inf,sup),hh->Integral(inf,sup)/integral*100);
      inf = sup+1;
   }
   sup = hh->GetXaxis()->GetNbins();
   printf("%d %d - %lf -> %lf\n",inf,sup,hh->Integral(inf,sup),hh->Integral(inf,sup)/integral*100);
   */
   return nl;
}

//-------------------------------------------------
TH2* KVHistoManipulator::PermuteAxis(TH2* hh)
{
//-------------------------------------------------
   // Cree un histo 2D en intervertissant les axes
   //
   // L'utilisateur doit effacer cet histo apres utilisation
   //

   if (!hh) {
      cout << "pointeur histogramme nul" << endl;
      return NULL;
   }
   if (!hh->InheritsFrom("TH2")) {
      Error("PermuteAxis", "methode definie uniquement pour les classes TH2 et filles");
   }
   Int_t nx = hh->GetNbinsX();
   Int_t ny = hh->GetNbinsY();

   TH2F* gg = new TH2F(
      Form("%s_perm", hh->GetName()),
      hh->GetTitle(),
      ny,
      hh->GetYaxis()->GetBinLowEdge(1),
      hh->GetYaxis()->GetBinLowEdge(ny + 1),
      nx,
      hh->GetXaxis()->GetBinLowEdge(1),
      hh->GetXaxis()->GetBinLowEdge(nx + 1)
   );

   for (Int_t xx = 1; xx <= nx; xx += 1) {
      for (Int_t yy = 1; yy <= ny; yy += 1) {

         gg->SetBinContent(yy, xx, hh->GetBinContent(xx, yy));

      }
   }
   return gg;
}

//-------------------------------------------------
TGraph* KVHistoManipulator::PermuteAxis(TGraph* gr)
{
//-------------------------------------------------
   // Cree un TGraph en intervertissant les axes
   //
   // L'utilisateur doit effacer ce graph apres utilisation
   //

   if (!gr) {
      cout << "pointeur graph nul" << endl;
      return NULL;
   }
   if (!gr->InheritsFrom("TGraph")) {
      Error("PermuteAxis", "methode definie uniquement pour les classes TGraph et filles");
   }

   TGraphErrors* gr2 = new TGraphErrors();
   for (Int_t nn = 0; nn < gr->GetN(); nn += 1) {
      Double_t px, py;
      gr->GetPoint(nn, px, py);
      gr2->SetPoint(nn, py, px);
      if (gr->InheritsFrom("TGraphErrors")) {
         gr2->SetPointError(nn, ((TGraphErrors*)gr)->GetErrorY(nn), ((TGraphErrors*)gr)->GetErrorX(nn));
      }
   }

   return gr2;
}

//-------------------------------------------------
TGraphErrors* KVHistoManipulator::MakeGraphFrom(TProfile* pf, Bool_t Error)
{
//-------------------------------------------------
   // Cree un graph à partir d un histo
   //
   // L'utilisateur doit effacer ce TGraph apres utilisation
   //

   if (!pf) {
      cout << "pointeur histogramme nul" << endl;
      return NULL;
   }
   if (!pf->InheritsFrom("TProfile")) {
      //Error("MakeGraphFrom","methode definie uniquement pour les classes TProfile et filles");
      return NULL;
   }
   Int_t nx = pf->GetNbinsX();

   TGraphErrors* gr = new TGraphErrors();
   for (Int_t xx = 1; xx <= nx; xx += 1) {
      if (pf->GetBinEntries(xx) > 0) {
         gr->SetPoint(gr->GetN(), pf->GetBinCenter(xx), pf->GetBinContent(xx));
         if (Error)
            gr->SetPointError(gr->GetN() - 1, pf->GetBinWidth(xx) / 2, pf->GetBinError(xx));
      }
   }

   return gr;
}

// //-------------------------------------------------
// TGraphErrors* KVHistoManipulator::MakeGraphFrom(TH1* pf, Bool_t Error)
// {
// //-------------------------------------------------
//    // Cree un graph à partir d un histo
//    //
//    // L'utilisateur doit effacer ce TGraph apres utilisation
//    //
//
//    if (!pf) {
//       cout << "pointeur histogramme nul" << endl;
//       return NULL;
//    }
//    if (!pf->InheritsFrom("TH1")) {
//       //Error("MakeGraphFrom","methode definie uniquement pour les classes TProfile et filles");
//    }
//    else if (pf->InheritsFrom("TProfile"))
//    {
//       return
//    }
//    Int_t nx = pf->GetNbinsX();
//
//    TGraphErrors* gr = new TGraphErrors();
//    for (Int_t xx = 1; xx <= nx; xx += 1) {
//       if (pf->GetBinEntries(xx) > 0) {
//          gr->SetPoint(gr->GetN(), pf->GetBinCenter(xx), pf->GetBinContent(xx));
//          if (Error)
//             gr->SetPointError(gr->GetN() - 1, pf->GetBinWidth(xx) / 2, pf->GetBinError(xx));
//       }
//    }
//
//    return gr;
// }

//###############################################################################################################"
//-------------------------------------------------
void KVHistoManipulator::DefinePattern(TH1* ob, TString titleX, TString titleY, TString labelX, TString labelY)
{
//-------------------------------------------------
   DefinePattern(ob->GetXaxis(), titleX, labelX);
   DefinePattern(ob->GetYaxis(), titleY, labelY);

}
//###############################################################################################################"
//-------------------------------------------------
void KVHistoManipulator::DefinePattern(TGraph* ob, TString titleX, TString titleY, TString labelX, TString labelY)
{
//-------------------------------------------------
   DefinePattern(ob->GetXaxis(), titleX, labelX);
   DefinePattern(ob->GetYaxis(), titleY, labelY);

}
//###############################################################################################################"
//-------------------------------------------------
void KVHistoManipulator::DefinePattern(TF1* ob, TString titleX, TString titleY, TString labelX, TString labelY)
{
//-------------------------------------------------
   DefinePattern(ob->GetXaxis(), titleX, labelX);
   DefinePattern(ob->GetYaxis(), titleY, labelY);

}
//###############################################################################################################"
//-------------------------------------------------
void KVHistoManipulator::DefinePattern(TAxis* ax, TString title, TString label)
{
//-------------------------------------------------

   TObjArray* tok = NULL;

   if (!title.IsNull()) {
      tok = title.Tokenize(" ");
      if (tok->GetEntries() == 3) {
         ax->SetTitleFont(((TObjString*)tok->At(0))->GetString().Atoi());
         ax->SetTitleSize(((TObjString*)tok->At(1))->GetString().Atof());
         ax->SetTitleOffset(((TObjString*)tok->At(2))->GetString().Atof());
      }
   }

   if (!label.IsNull()) {
      tok = label.Tokenize(" ");
      if (tok->GetEntries() == 3) {
         ax->SetLabelFont(((TObjString*)tok->At(0))->GetString().Atoi());
         ax->SetLabelSize(((TObjString*)tok->At(1))->GetString().Atof());
         ax->SetLabelOffset(((TObjString*)tok->At(2))->GetString().Atof());
      }
   }

   if (tok) delete tok;

}

//###############################################################################################################"
//-------------------------------------------------
void KVHistoManipulator::DefineLineStyle(TAttLine* ob, TString line)
{
//-------------------------------------------------

   TObjArray* tok = NULL;
   if (ob->IsA()->InheritsFrom("TAttLine")) {
      if (!line.IsNull()) {
         tok = line.Tokenize(" ");
         if (tok->GetEntries() == 3) {
            ob->SetLineColor(((TObjString*)tok->At(0))->GetString().Atoi());
            ob->SetLineStyle(((TObjString*)tok->At(1))->GetString().Atoi());
            ob->SetLineWidth(((TObjString*)tok->At(2))->GetString().Atoi());
         }
      }
   }
   if (tok) delete tok;

}

//###############################################################################################################"
//-------------------------------------------------
void KVHistoManipulator::DefineMarkerStyle(TAttMarker* ob, TString marker)
{
//-------------------------------------------------

   TObjArray* tok = NULL;
   if (ob->IsA()->InheritsFrom("TAttMarker")) {
      if (!marker.IsNull()) {
         tok = marker.Tokenize(" ");
         if (tok->GetEntries() == 3) {
            ob->SetMarkerColor(((TObjString*)tok->At(0))->GetString().Atoi());
            ob->SetMarkerStyle(((TObjString*)tok->At(1))->GetString().Atoi());
            ob->SetMarkerSize(((TObjString*)tok->At(2))->GetString().Atof());
         }
      }
   }
   if (tok) delete tok;

}

//###############################################################################################################"
//-------------------------------------------------
void KVHistoManipulator::DefineStyle(TObject* ob, TString line, TString marker)
{
//-------------------------------------------------

   DefineLineStyle((TAttLine*)ob, line);
   DefineMarkerStyle((TAttMarker*)ob, marker);

}

//###############################################################################################################"
//-------------------------------------------------
void KVHistoManipulator::DefineTitle(TH1* ob, TString xtit, TString ytit)
{
//-------------------------------------------------

   ob->GetXaxis()->SetTitle(xtit);
   ob->GetYaxis()->SetTitle(ytit);

}
//###############################################################################################################"
//-------------------------------------------------
void KVHistoManipulator::DefineTitle(TGraph* ob, TString xtit, TString ytit)
{
//-------------------------------------------------

   ob->GetXaxis()->SetTitle(xtit);
   ob->GetYaxis()->SetTitle(ytit);

}
//###############################################################################################################"
//-------------------------------------------------
void KVHistoManipulator::DefineTitle(TF1* ob, TString xtit, TString ytit)
{
//-------------------------------------------------

   ob->GetXaxis()->SetTitle(xtit);
   ob->GetYaxis()->SetTitle(ytit);

}
//###############################################################################################################"
//-------------------------------------------------
Double_t KVHistoManipulator::GetX(TH1* ob, Double_t val, Double_t eps, Int_t nmax, Double_t xmin, Double_t xmax)
{
   // Return value of abscissa X for which the interpolated value
   // of the histogram contents is equal to the given value, val.
   // We use the false position method (which should always converge...)
   // eps is required precision, i.e. convergence condition is that no further change
   // in result greater than eps is found.
   // nmax = maximum number of iterations
   // A solution is searched for X between the limits Xmin and Xmax of the X axis of the histo
   // unless arguments (xmin,xmax) are given to bracket the search

   TSpline5* spline = new TSpline5(ob);
   Double_t Xmax;
   Double_t Xmin;
   if (xmin == xmax) {
      Xmax = ob->GetXaxis()->GetXmax();
      Xmin = ob->GetXaxis()->GetXmin();
   }
   else {
      Xmax = xmax;
      Xmin = xmin;
   }

   Int_t n, side = 0;
   Double_t r = 0, fr, fs, s, ft, t;
   s = Xmin;
   t = Xmax;
   fs = spline->Eval(s) - val;
   ft = spline->Eval(t) - val;

   for (n = 1; n <= nmax; n++) {
      r = (fs * t - ft * s) / (fs - ft);
      if (fabs(t - s) < eps * fabs(t + s)) break;
      fr = spline->Eval(r) - val;

      if (fr * ft > 0) {
         t = r;
         ft = fr;
         if (side == -1) fs /= 2;
         side = -1;
      }
      else if (fs * fr > 0) {
         s = r;
         fs = fr;
         if (side == +1) ft /= 2;
         side = +1;
      }
      else break;
   }
   delete spline;
   return r;
}
//###############################################################################################################"
//-------------------------------------------------
TF1* KVHistoManipulator::RescaleX(TH1* hist1, TH1* hist2, Int_t degree, Double_t* params,
                                  Int_t npoints, const Char_t* direction, Double_t xmin, Double_t xmax, Double_t qmin, Double_t qmax, Double_t eps)
{
   // Find the polynomial transformation of the X-axis of 1-D histogram hist1
   // so that the distribution ressembles that of histogram hist2.
   // Returns a TF1 which can be used to rescale hist1 (hist1 and hist2 are not modified).
   // User's responsibility to delete the TF1.
   // After fitting, the array params is filled with:
   // params[0] = a0
   // params[1] = a1
   // ...
   // params[degree] = an
   // params[degree+1] = Chisquare/NDF
   // Therefore params MUST BE at least of dimension (degree+2)
   //
   // npoints : by default (npoints=-1), we use npoints=degree+2 values of comparison
   //           of the two cumulative distributions (see below).
   //           more comparison points can be used by setting npoints>=degree+2.
   // direction : by default ("C") we use the cumulative histogram summed from low x to high x.
   //            if direction="D", we use the cumulative histogram summed from high x to low x.
   // xmin, xmax : range of values of abscissa used to build cumulative histograms. default
   //             (xmin=xmax=-1) include all values.
   // qmin, qmax : minimum & maximum values of cumulative histograms used for the
   //              comparison (see below). by default qmin=0.05, qmax=0.95.
   //
   // METHOD
   // ======
   // 'hist1' contains the distribution P1 of variable X1
   // 'hist2' contains the distribution P2 of variable X2
   // Supposing that we can write X2=f_n(X1), with f_n(x)=a0 + a1*x + a2*x^2 + ... + an*x^n,
   // what are the parameters of the polynomial for which P1(f_n(X1))=P2(X2) ?
   //
   // Consider the cumulative distributions, C1(X1) and C2(X2).
   // For npoints=2 we compare the 2 X1 & X2 values for which C1=C2=qmin, qmax
   // For npoints=3 we compare the 3 X1 & X2 values for which C1=C2=qmin, qmin+(qmax-qmin)/2, qmax
   // For npoints=4 we compare the 4 X1 & X2 values for which C1=C2=qmin, qmin+(qmax-qmin)/3, qmin+2*(qmax-qmin)/3, qmax
   // etc. etc.
   // In each case we fit the npoints couples (X1,X2) with f_n
   //

   int i;
   // calculate comparison points
   npoints = TMath::Max(npoints, degree + 2);
   TString func_name = Form("pol%d", degree);
   TF1* fonc = new TF1("f", func_name.Data());
   fonc->SetName(Form("RescaleX-%s", func_name.Data()));
   RescaleX(hist1, hist2, fonc, npoints, direction, xmin, xmax, qmin, qmax, eps);
   for (i = 0; i < degree + 1; i++) {
      params[i] = fonc->GetParameter(i);
   }
   Double_t chisquare = fonc->GetChisquare();
   if (fonc->GetNDF() > 0.0) chisquare /= fonc->GetNDF();
   params[degree + 1] = chisquare;

   return fonc;
}
//###############################################################################################################"
//-------------------------------------------------
TH1* KVHistoManipulator::MakeHistoRescaleX(TH1* hist1, TH1* hist2, Int_t degree, Double_t* params,
      Option_t* opt, Int_t npoints, const Char_t* direction, Double_t xmin, Double_t xmax, Double_t qmin, Double_t qmax,
      Double_t eps)
{
   // Uses RescaleX(TH1* hist1, TH1* hist2, Int_t degree, Double_t* params, Double_t eps)
   // to find a polynomial transformation of 'hist1' abscissa so that its
   // distribution resembles that of 'hist2', then generates a new rescaled version of 'hist1'.
   //
   // degree = degree of polynomial to use
   // params = array of dimension [degree+2], after rescaling it will contain
   //          the values of fitted parameters of polynomial, plus the Chi2/NDF of the fit:
   //    params[0] = a0
   //    params[1] = a1
   //    ...
   //    params[degree] = an
   //    params[degree+1] = Chisquare/NDF = Chisquare (NDF is always equal to 1)
   //
   // npoints : by default (npoints=-1), we use npoints=degree+2 values of comparison
   //           of the two cumulative distributions (see method RescaleX).
   //           more comparison points can be used by setting npoints>=degree+2.
   //
   // direction : by default ("C") we use the cumulative histogram summed from low x to high x.
   //            if direction="D", we use the cumulative histogram summed from high x to low x.
   // xmin, xmax : range of values of abscissa used to build cumulative histograms. default
   //             (xmin=xmax=-1) include all values.
   // qmin, qmax : minimum & maximum values of cumulative histograms used for the
   //              comparison (see method RescaleX). by default qmin=0.05, qmax=0.95.
   //
   // eps = relative precision used to find comparison points of histos (default = 1.e-07)
   //
   // OPTIONS
   // =======
   //  opt = "norm" : rescaled histogram normalised to have same integral as hist2
   //  opt = "bins" : rescaled histogram will have same number of bins & same limits as hist2
   //  opt = "normbins" |
   //  opt = "binsnorm" |--> rescaled histogram can be superposed and added to hist2
   //
   // EXAMPLE OF USE
   // =======================
   // In the following example, we fill two histograms with different numbers of random
   // values drawn from two Gaussian distributions with different centroids and widths.
   // We also add to each histogram a 'pedestal' peak which is unrelated to the 'physical'
   // distributions, and significant enough so that it does not permit a correct scaling of
   // the physical part of the distribution.
   //
   // We can overcome the problem of the 'pedestal' by
   // using the 'inverse cumulated distribution' and excluding the channels
   // where the pedestals are present. The correct scaling of the physical
   // distributions is recovered, as shown below:
   /*
   BEGIN_MACRO(source)
   {
   TH1F* h10 = new TH1F("h10","gaussian",4096,-.5,4095.5);
   TF1 g10("g10","gaus(0)",0,4100);
   g10.SetParameters(1,1095,233);
   h10->FillRandom("g10",56130);
   h10->SetBinContent(85,13425);
   TH1F* h20 = new TH1F("h20","gaussian",4096,-.5,4095.5);
   g10.SetParameters(1,1673,487);
   h20->FillRandom("g10",21370);
   h20->SetBinContent(78,17900);
   KVHistoManipulator HM2;
   HM2.SetVisDebug();   // turn on visual debugging -> create canvas showing rescaling procedure
   Double_t params0[10];
   // make rescaling using inverse cumulative distribution, limit to x=[100,4095]
   TH1* sc30 =HM2.MakeHistoRescaleX(h10,h20,1,params0,"binsnorm",5,"D",100,4095);
   return gROOT->GetListOfCanvases()->FindObject("VDCanvas");
   }
   END_MACRO
   */

   TF1* scalefunc = RescaleX(hist1, hist2, degree, params, npoints, direction, xmin, xmax, qmin, qmax, eps);
   TString options(opt);
   options.ToUpper();
   Bool_t norm = options.Contains("NORM");
   Bool_t bins = options.Contains("BINS");
   Int_t nx = (bins ? hist2->GetNbinsX() : -1);
   Double_t nxmin = (bins ? hist2->GetXaxis()->GetXmin() : -1);
   Double_t nxmax = (bins ? hist2->GetXaxis()->GetXmax() : -1);
   TH1* scalehisto = ScaleHisto(hist1, scalefunc, 0, nx, -1, nxmin, nxmax, -1.0, -1.0, "width");
   if (norm) scalehisto->Scale(hist2->Integral("width") / scalehisto->Integral("width"));
   if (kVisDebug) {
      fVDCanvas->cd(4);
      scalehisto->DrawCopy()->SetLineColor(kRed);
      hist2->DrawCopy("same")->SetLineColor(kBlack);
      gPad->SetLogy(kTRUE);
      gPad->Modified();
      gPad->Update();
   }
   delete scalefunc;
   return scalehisto;
}
//###############################################################################################################"
//-------------------------------------------------
void KVHistoManipulator::RescaleX(TH1* hist1, TH1* hist2, TF1* scale_func, Int_t npoints,
                                  const Char_t* direction, Double_t xmin, Double_t xmax, Double_t qmin, Double_t qmax, Double_t eps)
{
   // Find the transformation of the X-axis of 1-D histogram hist1
   // so that the distribution ressembles that of histogram hist2.
   // The user provides a function f(x) (TF1* scale_func) which is supposed to
   // transform the abscissa X1 of 'hist1' in such a way that P1(f(X1)) = P2(X2).
   // We fit 'npoints' comparison points (see below), npoints>=2.
   //
   // direction : by default ("C") we use the cumulative histogram summed from low x to high x.
   //            if direction="D", we use the cumulative histogram summed from high x to low x.
   // xmin, xmax : range of values of abscissa used to build cumulative histograms. default
   //             (xmin=xmax=-1) include all values.
   // qmin, qmax : minimum & maximum values of cumulative histograms used for the
   //              comparison (see below). by default qmin=0.05, qmax=0.95.
   // METHOD
   // ======
   // 'hist1' contains the distribution P1 of variable X1
   // 'hist2' contains the distribution P2 of variable X2
   // Supposing that we can write X2=f(X1), we compare the abscissa of different
   // points of the two cumulative distributions, C1(X1) and C2(X2).
   // For npoints=2 we compare the 2 X1 & X2 values for which C1=C2=qmin, qmax
   // For npoints=3 we compare the 3 X1 & X2 values for which C1=C2=qmin, qmin+(qmax-qmin)/2, qmax
   // For npoints=4 we compare the 4 X1 & X2 values for which C1=C2=qmin, qmin+(qmax-qmin)/3, qmin+2*(qmax-qmin)/3, qmax
   // etc. etc.
   // In each case we fit the 'npoints' couples (X1,X2) with the TF1 pointed to by 'scale_func'

   if (kVisDebug) {
      if (!fVDCanvas) fVDCanvas = new TCanvas("VDCanvas", "KVHistoManipulator::RescaleX");
      gStyle->SetOptStat("");
      fVDCanvas->Clear();
      fVDCanvas->Divide(2, 2);
      fVDCanvas->cd(1);
      hist1->DrawCopy()->SetLineColor(kBlue);
      hist2->DrawCopy("same")->SetLineColor(kBlack);
      gPad->SetLogy(kTRUE);
      gPad->Modified();
      gPad->Update();
   }
   int i;
   npoints = TMath::Max(2, npoints);
   Info("RescaleX", "Calculating transformation of histo %s using reference histo %s, %d points of comparison",
        hist1->GetName(), hist2->GetName(), npoints);
   TH1* cum1 = 0;
   TH1* cum2 = 0;
   if (xmin > -1 && xmax > -1) {
      cum1 = CumulatedHisto(hist1, xmin, xmax, direction, "max");
      cum2 = CumulatedHisto(hist2, xmin, xmax, direction, "max");
   }
   else {
      cum1 = CumulatedHisto(hist1, direction, -1, -1, "max");
      cum2 = CumulatedHisto(hist2, direction, -1, -1, "max");
   }
   if (kVisDebug) {
      fVDCanvas->cd(2);
      cum1->DrawCopy()->SetLineColor(kBlue);
      cum2->DrawCopy("same")->SetLineColor(kBlack);
      gPad->Modified();
      gPad->Update();
   }
   // calculate comparison points
   Double_t* quantiles = new Double_t[npoints];
   Double_t delta_q = (qmax - qmin) / (1.0 * (npoints - 1));
   for (i = 0; i < npoints; i++) quantiles[i] = qmin + i * delta_q;
   // get X1 and X2 values corresponding to quantiles
   Double_t* X1 = new Double_t[npoints];
   Double_t* X2 = new Double_t[npoints];
   for (i = 0; i < npoints; i++) {
      X1[i] = GetX(cum1, quantiles[i], eps);
      X2[i] = GetX(cum2, quantiles[i], eps);
   }
   for (i = 0; i < npoints; i++) {
      printf("COMPARISON: i=%d  quantile=%f  X1=%f  X2=%f\n",
             i, quantiles[i], X1[i], X2[i]);
   }
   // fill TGraph with points to fit
   TGraph* fitgraph = new TGraph(npoints, X1, X2);
   TString fitoptions = "0N";
   if (kVisDebug) fitoptions = "";
   if (fitgraph->Fit(scale_func, fitoptions.Data()) != 0) {
      Error("RescaleX", "Fitting with function %s failed to converge",
            scale_func->GetName());
   }
   if (kVisDebug) {
      fVDCanvas->cd(3);
      fitgraph->SetMarkerStyle(20);
      gStyle->SetOptStat(1011);
      fitgraph->DrawClone("ap");
      gPad->Modified();
      gPad->Update();
   }
   delete cum1;
   delete cum2;
   delete fitgraph;
   delete [] quantiles;
   delete [] X1;
   delete [] X2;
}
//###############################################################################################################"
//-------------------------------------------------
TH1* KVHistoManipulator::MakeHistoRescaleX(TH1* hist1, TH1* hist2, TF1* scale_func, Int_t npoints,
      Option_t* opt, const Char_t* direction, Double_t xmin, Double_t xmax, Double_t qmin, Double_t qmax, Double_t eps)
{
   // Uses RescaleX(TH1* hist1, TH1* hist2, TF1* scale_func, Int_t npoints, Double_t eps)
   // to transform 'hist1' abscissa using TF1 'scale_func' so that its
   // distribution resembles that of 'hist2', then generates a new rescaled version of 'hist1'.
   //
   // npoints = number of points of comparison between the two histograms.
   //           Make sure this is sufficient for the TF1 used in the transformation.
   //           i.e. for a polynomial of degree 1 (a+bx), 2 points are enough,
   //           3 will give a meaningful Chi^2 value.
   // direction : by default ("C") we use the cumulative histogram summed from low x to high x.
   //            if direction="D", we use the cumulative histogram summed from high x to low x.
   // xmin, xmax : range of values of abscissa used to build cumulative histograms. default
   //             (xmin=xmax=-1) include all values.
   // qmin, qmax : minimum & maximum values of cumulative histograms used for the
   //              comparison (see method RescaleX). by default qmin=0.05, qmax=0.95.
   // eps = relative precision used to find comparison points of histos (default = 1.e-07)
   //
   // OPTIONS
   // =======
   //  opt = "norm" : rescaled histogram normalised to have same integral as hist2
   //  opt = "bins" : rescaled histogram will have same number of bins & same limits as hist2
   //  opt = "normbins" |
   //  opt = "binsnorm" |--> rescaled histogram can be superposed and added to hist2

   RescaleX(hist1, hist2, scale_func, npoints, direction, xmin, xmax, qmin, qmax, eps);
   TString options(opt);
   options.ToUpper();
   Bool_t norm = options.Contains("NORM");
   Bool_t bins = options.Contains("BINS");
   Int_t nx = (bins ? hist2->GetNbinsX() : -1);
   Double_t nxmin = (bins ? hist2->GetXaxis()->GetXmin() : -1);
   Double_t nxmax = (bins ? hist2->GetXaxis()->GetXmax() : -1);
   TH1* scalehisto = ScaleHisto(hist1, scale_func, 0, nx, -1, nxmin, nxmax, -1.0, -1.0, "width");
   if (norm) scalehisto->Scale(hist2->Integral("width") / scalehisto->Integral("width"));
   return scalehisto;
}
//-------------------------------------------------
TH1* KVHistoManipulator::CumulatedHisto(TH1* hh, Double_t xmin, Double_t xmax, const TString& direction, Option_t* norm)
{
   // Cumule le contenu de l histo hh entre xmin et xmax et retourne l histo correspondant
   // Voir CumulatedHisto(TH1* ,TString ,Int_t ,Int_t , Option_t*).
   Int_t bmin = hh->FindBin(xmin);
   Int_t bmax = hh->FindBin(xmax);
   if (bmax > hh->GetNbinsX()) bmax = hh->GetNbinsX();
   return CumulatedHisto(hh, direction, bmin, bmax, norm);
}

//-------------------------------------------------
Double_t KVHistoManipulator::GetChisquare(TH1* h1, TF1* f1, Bool_t norm, Bool_t err, Double_t* para)
{
   //Camcul du chi2 entre un histogramme et une fonction donnée
   //Warning : ne prend en compte que les bins avec une stat>0
   //de l histogramme
   // norm = kTRUE (default), normalise la valeur du Chi2 au nombre de bin pris en compte
   // err = kTRUE (default), prend en compte les erreurs du contenu des bins dans le calcul
   // (si celle ci est >0)
   Double_t chi2 = 0;
   Int_t nbre = 0;
   if (h1->InheritsFrom("TH2")) {
      Double_t xx[2];
      for (Int_t nx = 1; nx < h1->GetNbinsX(); nx += 1)
         for (Int_t ny = 1; ny <= h1->GetNbinsY(); ny += 1) {
            Double_t hval = h1->GetBinContent(nx, ny);
            if (hval > 0) {
               if (err) {
                  Double_t herr = h1->GetBinError(nx, ny);
                  if (herr > 0) {
                     nbre += 1;
                     xx[0] = h1->GetXaxis()->GetBinCenter(nx);
                     xx[1] = h1->GetYaxis()->GetBinCenter(ny);
                     Double_t fval = f1->EvalPar(xx, para);
                     chi2 += TMath::Power((hval - fval) / herr, 2.);
                  }
               }
               else {
                  nbre += 1;
                  xx[0] = h1->GetXaxis()->GetBinCenter(nx);
                  xx[1] = h1->GetYaxis()->GetBinCenter(ny);
                  Double_t fval = f1->EvalPar(xx, para);
                  chi2 += TMath::Power((hval - fval), 2.);
               }
            }
         }
   }
   else {
      Double_t xx[1];
      for (Int_t nx = 1; nx < h1->GetNbinsX(); nx += 1) {
         Double_t hval = h1->GetBinContent(nx);
         if (hval > 0) {
            if (err) {
               Double_t herr = h1->GetBinError(nx);
               if (herr > 0) {
                  nbre += 1;
                  xx[0] = h1->GetXaxis()->GetBinCenter(nx);
                  Double_t fval = f1->EvalPar(xx, para);
                  chi2 += TMath::Power((hval - fval) / herr, 2.);
               }
            }
            else {
               nbre += 1;
               xx[0] = h1->GetXaxis()->GetBinCenter(nx);
               Double_t fval = f1->EvalPar(xx, para);
               chi2 += TMath::Power((hval - fval), 2.);
            }
         }
      }
   }

   if (nbre == 0) {
      printf("Warning, KVHistoManipulator::GetChisquare :\n\taucune cellule price en compte dans le calcul du Chi2 ...\n");
      return -1;
   }
   return (norm ? chi2 / nbre : chi2);


}
//-------------------------------------------------
Double_t KVHistoManipulator::GetLikelihood(TH1* h1, TF1* f1, Bool_t norm, Double_t* para)
{
   //Calcul du chi2 entre un histogramme et une fonction donnée
   //Warning : ne prend en compte que les bins avec une stat>0
   //de l histogramme
   // norm = kTRUE (default), normalise la valeur du Chi2 au nombre de bin pris en compte
   // err = kTRUE (default), prend en compte les erreurs du contenu des bins dans le calcul
   // (si celle ci est >0)
   Double_t chi2 = 0;
   Int_t nbre = 0;
   if (h1->InheritsFrom("TH2")) {
      Double_t xx[2];
      for (Int_t nx = 1; nx < h1->GetNbinsX(); nx += 1)
         for (Int_t ny = 1; ny <= h1->GetNbinsY(); ny += 1) {
            Double_t hval = h1->GetBinContent(nx, ny);
            if (hval > 0) {
               nbre += 1;
               xx[0] = h1->GetXaxis()->GetBinCenter(nx);
               xx[1] = h1->GetYaxis()->GetBinCenter(ny);
               Double_t fval = f1->EvalPar(xx, para);
               Double_t logfval = TMath::Log(fval);
               chi2 += fval - 1 * hval * logfval;
            }
         }
   }
   else {
      Double_t xx[1];
      for (Int_t nx = 1; nx < h1->GetNbinsX(); nx += 1) {
         Double_t hval = h1->GetBinContent(nx);
         if (hval > 0) {
            nbre += 1;
            xx[0] = h1->GetXaxis()->GetBinCenter(nx);
            Double_t fval = f1->EvalPar(xx, para);
            Double_t logfval = TMath::Log(fval);
            chi2 += fval - 1 * hval * logfval;
         }
      }
   }

   if (nbre == 0) {
      printf("Warning, KVHistoManipulator::GetChisquare :\n\taucune cellule price en compte dans le calcul du Chi2 ...\n");
      return -1;
   }
   return (norm ? chi2 / nbre : chi2);

}

//______________________________________________________________________________________________
TGraph* KVHistoManipulator::DivideGraphs(TGraph* G1, TGraph* G2)
{
   // Create and fill a TGraph containing, for each point in G1 and G2,
   // the value of (y1/y2).
   // G1 & G2 should have the same number of points, with the same x-coordinates
   // i.e. x1 = x2 for all points
   // if any y2 value = 0, we set the corresponding point's y=0

   Int_t npoints = G1->GetN();
   if (G2->GetN() != npoints) {
      Error("DivideGraphs", "Graphs must have same number of points");
      return nullptr;
   }
   // make copy of G1
   AUTO_NEW_CTOR(TGraph, Gdiv)(*G1);
   Gdiv->SetName(Form("%s_divided_by_%s", G1->GetName(), G2->GetName()));
   Gdiv->SetTitle(Form("%s divided by %s", G1->GetTitle(), G2->GetTitle()));
   Double_t* X = G1->GetX();
   Double_t* Y1 = G1->GetY();
   Double_t* Y2 = G2->GetY();
   for (int i = 0; i < npoints; i++) {
      if (Y2[i] != 0) Gdiv->SetPoint(i, X[i], Y1[i] / Y2[i]);
      else  Gdiv->SetPoint(i, X[i], 0.);
   }
   return Gdiv;
}

//______________________________________________________________________________________________
TGraph* KVHistoManipulator::ComputeNewGraphFrom(TGraph* g0, TGraph* g1, const TString& formula)
{
   Int_t npoints = g0->GetN();
   if (g1->GetN() != npoints) {
      Error("ComputeNewGraphFrom", "Graphs must have same number of points %d != %d", npoints, g1->GetN());
      return nullptr;
   }

   TF1 f1("func_ComputeNewGraphFrom", formula, 0, 1);
   if (f1.IsZombie() || f1.GetNpar() != 2) {
      Error("ComputeNewGraphFrom", "formula %s for the operation is not valid or has not 2 parameters", formula.Data());
      return nullptr;
   }

   AUTO_NEW_CTOR(TGraph, gfinal)();
   gfinal->SetName(Form("from_%s_%s", g0->GetName(), g1->GetName()));
   Double_t* x0 = g0->GetX();
   Double_t* y0 = g0->GetY();

   Double_t* x1 = g1->GetX();
   Double_t* y1 = g1->GetY();

   for (Int_t ii = 0; ii < npoints; ii++) {
      f1.SetParameters(y0[ii], y1[ii]);
      if (x1[ii] != x0[ii])
         Warning("ComputeNewGraphFrom", "X values are different for the same point %d : %lf %lf", ii, x0[ii], x1[ii]);
      Double_t result = f1.Eval(x0[ii]);
      gfinal->SetPoint(ii, x0[ii], result);
   }
   return gfinal;
}

//______________________________________________________________________________________________
std::vector<Double_t> KVHistoManipulator::GetLimits(TGraph* G1)
{
   /*
   xmin -> limits[0];
   ymin -> limits[1];
   xmax -> limits[2];
   ymax -> limits[3];
   */
   std::vector<Double_t> limits(4);
   Double_t xx, yy;
   for (Int_t ii = 0; ii < G1->GetN(); ii += 1) {
      G1->GetPoint(ii, xx, yy);
      if (ii == 0) {
         limits[0] = limits[2] = xx;
         limits[1] = limits[3] = yy;
      }
      else {
         if (xx < limits[0]) limits[0] = xx;
         if (yy < limits[1]) limits[1] = yy;
         if (xx > limits[2]) limits[2] = xx;
         if (yy > limits[3]) limits[3] = yy;
      }
   }

   return limits;
}

//______________________________________________________________________________________________
std::vector<Double_t> KVHistoManipulator::GetLimits(TMultiGraph* mgr)
{

   /*
   xmin -> limits[0];
   ymin -> limits[1];
   xmax -> limits[2];
   ymax -> limits[3];
   */
   std::vector<Double_t> limits, temp;

   TList* lg = mgr->GetListOfGraphs();
   TGraph* gr = nullptr;
   for (Int_t ii = 0; ii < lg->GetEntries(); ii += 1) {
      gr = dynamic_cast<TGraph*>(lg->At(ii));
      if (ii == 0) {
         limits = GetLimits(gr);
      }
      else {
         temp = GetLimits(gr);
         if (temp[0] < limits[0]) limits[0] = temp[0];
         if (temp[1] < limits[1]) limits[1] = temp[1];
         if (temp[2] > limits[2]) limits[2] = temp[2];
         if (temp[3] > limits[3]) limits[3] = temp[3];
      }
   }

   return limits;

}

//______________________________________________________________________________________________
std::vector<Double_t> KVHistoManipulator::GetLimits(TProfile* G1)
{
   /*
   xmin -> limits[0];
   ymin -> limits[1];
   xmax -> limits[2];
   ymax -> limits[3];
   */
   std::vector<Double_t> limits(4);
   Double_t xx, yy;
   Bool_t first = kTRUE;
   for (Int_t ii = 1; ii <= G1->GetNbinsX(); ii += 1) {
      Double_t stat = G1->GetBinEntries(ii);
      if (stat > 0) {
         xx = G1->GetBinCenter(ii);
         yy = G1->GetBinContent(ii);
         if (first) {
            first = kFALSE;
            limits[0] = limits[2] = xx;
            limits[1] = limits[3] = yy;
         }
         else {
            if (xx < limits[0]) limits[0] = xx;
            if (yy < limits[1]) limits[1] = yy;
            if (xx > limits[2]) limits[2] = xx;
            if (yy > limits[3]) limits[3] = yy;
         }
      }
   }

   return limits;

}

//______________________________________________________________________________________________
std::vector<Double_t> KVHistoManipulator::GetLimits(TSeqCollection* mgr)
{

   /*
   xmin -> limits[0];
   ymin -> limits[1];
   xmax -> limits[2];
   ymax -> limits[3];
   */
   std::vector<Double_t> temp, limits;

   TProfile* gr = nullptr;
   for (Int_t ii = 0; ii < mgr->GetEntries(); ii += 1) {
      gr = dynamic_cast<TProfile*>(mgr->At(ii));
      if (ii == 0) {
         limits = GetLimits(gr);
      }
      else {
         temp = GetLimits(gr);
         if (temp[0] < limits[0]) limits[0] = temp[0];
         if (temp[1] < limits[1]) limits[1] = temp[1];
         if (temp[2] > limits[2]) limits[2] = temp[2];
         if (temp[3] > limits[3]) limits[3] = temp[3];
      }
   }

   return limits;

}

//______________________________________________________________________________________________
void KVHistoManipulator::ApplyCurrentLimitsToAllCanvas(Bool_t AlsoLog)
{

   //Getthe limits of the histogram in the current pad
   //and apply them to the others histogram drawn on the others pads

   if (!gPad) return;
   TObject* obj = nullptr;
   TVirtualPad* tmp = gPad;
   TIter nextp(gPad->GetListOfPrimitives());
   TH1* h1 = nullptr;
   while ((obj = nextp())) {
      if (obj->InheritsFrom("TH1")) {
         h1 = dynamic_cast<TH1*>(obj);
      }
   }
   if (h1) {
      Int_t x1 = h1->GetXaxis()->GetFirst();
      Int_t x2 = h1->GetXaxis()->GetLast();
      Double_t y1, y2;
      Double_t z1, z2;

      if (h1->GetDimension() == 1) { //TH1 ou TProfile
         y1 = h1->GetMinimum();
         y2 = h1->GetMaximum();
      }
      else {
         y1 = h1->GetYaxis()->GetFirst();
         y2 = h1->GetYaxis()->GetLast();
      }

      if (h1->GetDimension() == 2) { //TH2 ou TProfile2D
         z1 = h1->GetMinimum();
         z2 = h1->GetMaximum();
      }
      else {
         z1 = h1->GetZaxis()->GetFirst();
         z2 = h1->GetZaxis()->GetLast();
      }

      std::cout << x1 << " " << x2 << " - " << y1 << " " << y2 << " - " << z1 << " " << z2 << std::endl;

      Int_t nc = 1;
      TCanvas* cc = gPad->GetCanvas();
      TVirtualPad* pad = nullptr;
      while ((pad = cc->GetPad(nc))) {
         if (tmp != pad) {
            pad->cd();
            if (AlsoLog) {
               gPad->SetLogx(tmp->GetLogx());
               gPad->SetLogy(tmp->GetLogy());
               gPad->SetLogz(tmp->GetLogz());
            }
            TIter nextq(gPad->GetListOfPrimitives());
            TH1* h1 = nullptr;
            while ((obj = nextq())) {
               if (obj->InheritsFrom("TH1")) {
                  h1 = dynamic_cast<TH1*>(obj);

                  h1->GetXaxis()->SetRange(x1, x2);
                  if (h1->GetDimension() == 1) {
                     h1->SetMinimum(y1);
                     h1->SetMaximum(y2);
                  }
                  else {
                     h1->GetYaxis()->SetRange(y1, y2);
                  }
                  if (h1->GetDimension() == 2) {
                     h1->SetMinimum(z1);
                     h1->SetMaximum(z2);
                  }
                  else {
                     h1->GetZaxis()->SetRange(y1, y2);
                  }
               }
            }
            gPad->Update();
         }
         nc += 1;
      }
      cc->Paint();
      cc->Update();
   }
   gPad = tmp;
}
