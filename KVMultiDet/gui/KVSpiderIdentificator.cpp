
#include "KVSpiderIdentificator.h"
#include <TCanvas.h>
#include <TSystem.h>
#include "KVSpiderLineSiCsI.h"

#include <KVCanvas.h>

using namespace std;

ClassImp(KVSpiderIdentificator)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVSpiderIdentificator</h2>
<h4>Générateur de lignes d'identifications</h4>

Présentation de l'éditeur : <a href="http://indra.in2p3.fr/KaliVedaDoc/images/JourneeAnalyse0112_1.pdf"> JourneeAnalyse0112_1.pdf </a> (FR)

<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////


KVSpiderIdentificator::KVSpiderIdentificator()
{
   _is_initialized = false;
   SetDefault();
}

KVSpiderIdentificator::KVSpiderIdentificator(TH2F* h_, Double_t Xm, Double_t Ym, Double_t pdx, Double_t pdy, Int_t type)
{
   _is_initialized = false;
   SetDefault();
   fType = type;
   Init(h_, Xm, Ym, pdx, pdy);
}


KVSpiderIdentificator::~KVSpiderIdentificator()
{
   Close();
}


void KVSpiderIdentificator::SetDefault()
{
   _debug  = false;
   _auto   = true;
   _useFit = true;
   _hlist.SetOwner();
   _dlist.SetOwner();
   _bfactor = 1.;
   _nAngleUp = 20;
   _nAngleDown = 40;
   _alpha = 1.7;
}


void KVSpiderIdentificator::Close()
{
   _hlist.Clear();
   _dlist.Clear();

}


void KVSpiderIdentificator::Clear(Option_t* option)
{
   if (!strcmp(option, "ON EN A MARRE DES WARNING !!!!")) return;

   _hlist.Clear();
   _dlist.Clear();
   _llist.Clear("all");
   _htot = 0;
   _is_initialized = false;
}


void KVSpiderIdentificator::SetParameters(double bining_)
{
   _bfactor = bining_;
}


void KVSpiderIdentificator::Init(TH2F* h_, Double_t Xm, Double_t Ym, Double_t pdx, Double_t pdy)
{
   if (!TestHistogram(h_)) return;
   else _htot = h_;

   double xm    = h_->GetXaxis()->GetXmax();
   double ym    = h_->GetYaxis()->GetXmax();

   _xmax = xm;
   _ymax = ym;

   bool   foundx = false;
   bool   foundy = false;

   TH1D* hx = _htot->ProjectionX();
   TH1D* hy = _htot->ProjectionY();

   int x0 = pdx;
   int y0 = pdy;
   int z0;

   if ((pdx < 0.) || (pdy < 0.)) _htot->GetMaximumBin(x0, y0, z0);
   _x0 = x0;
   _y0 = y0;

   int j = hx->GetNbinsX() - 1;

   while ((!foundx) && (j >= 0)) {
      if (hx->GetBinContent(j) >= 3) {
         xm = (double)j;
         foundx = true;
      }
      j--;
   }

   j = hy->GetNbinsX() - 1;
   while (!foundy && (j >= 0)) {
      if (hy->GetBinContent(j) >= 3) {
         ym = (double)j;
         foundy = true;
      }
      j--;
   }

   delete hx;
   delete hy;

   if ((!foundx) || (!foundy)) {
      cout << "ERROR: KVSpiderIdentificator::Init(): Invalid Histogram !" << endl;
      return;
   }
   _xm = xm;
   _ym = ym;

   if (Xm > 0.) _xm = Xm;
   if (Ym > 0.) _ym = Ym;

   _invalid = new TGraph();

   _is_initialized = true;
   return;
}

void KVSpiderIdentificator::CalculateTheta()
{
   if ((_ym > 0) && (_xm > 0)) _ftheta = TMath::ATan((_ym - _y0) / (_xm - _x0)) * TMath::RadToDeg();
}



bool KVSpiderIdentificator::CheckPath(const Char_t* path_)
{
   if (!gSystem->OpenDirectory(path_)) gSystem->MakeDirectory(path_);
   return true;
}



TH1F* KVSpiderIdentificator::GetProjection(TH2F* h_, KVDroite* d_, int rebin_)
{
   if (!TestHistogram(h_)) return 0;

   double a0    = d_->GetA0();
   double th    = d_->GetTheta();
   double costh = TMath::Cos(TMath::DegToRad() * th);
   double sinth = TMath::Sin(TMath::DegToRad() * th);
   double mma   = -1;

   double thetam = TMath::ATan(_ymax / _xmax) * TMath::RadToDeg();

   if (th <= thetam) mma = _xmax / costh;
   else mma = _ymax / sinth;

   int  mmb = (int)mma / (rebin_ * _bfactor);

   TH1F* h1 = new TH1F(Form("%s_proj", h_->GetName()), h_->GetTitle(), mmb, 0, mma);

   int xbins = h_->GetNbinsX();
   int ybins = h_->GetNbinsY();
   int nbins = xbins * ybins;
   if (_debug)cout << "DEBUG: GetProjection(): total number of bins : '" << nbins << "'." << endl;

   for (int x = 0; x <= xbins; x++) {
      Double_t bminx = h_->GetXaxis()->GetBinLowEdge(x);
      Double_t bmaxx = h_->GetXaxis()->GetBinUpEdge(x);
      Double_t xx  = _alea.Uniform(bminx, bmaxx);
      if (xx == bmaxx) xx = bminx;

      for (int y = 0; y <= ybins; y++) {
         Double_t bminy = h_->GetYaxis()->GetBinLowEdge(y);
         Double_t bmaxy = h_->GetYaxis()->GetBinUpEdge(y);
         Double_t yy  = _alea.Uniform(bminy, bmaxy);
         if (yy == bmaxy) yy = bminy;

         Double_t content = (Double_t) h_->GetBinContent(x, y);

         yy -= a0;
         Double_t x1 = xx * costh + yy * sinth;

         if (content > 0.) h1->Fill(x1, content);
      }
   }
   return h1;
}


void KVSpiderIdentificator::SetHistogram(TH2F* h_, Double_t Xm, Double_t Ym)
{
   Init(h_, Xm, Ym);
   return;
}


bool KVSpiderIdentificator::TestHistogram(TH1* h_)
{
   if (!h_) {
      cout << "ERROR: KVSpiderIdentificator::TestHistogram(): Invalid pointer on 'TH2F' histogram !" << endl;
      return false;
   } else if (h_->Integral() == 0) {
      cout << "ERROR: KVSpiderIdentificator::SetHistogram(): Invalid histogram for SpIDer identification !" << endl;
      return false;
   }
   return true;
}


TH2F* KVSpiderIdentificator::CreateHistogram(double th_, double alpha_)
{
   TList* ll = CreateHistograms(th_, th_, 1, true, alpha_);
   TH2F* h = (TH2F*)ll->At(0);
   return h;
}


TList* KVSpiderIdentificator::CreateHistograms(double thmin_, double thmax_, int nth_, bool cos_, double alpha_)
{
   if (!TestHistogram(_htot)) return 0;
   else if (!_is_initialized) {
      cout << "ERROR: KVSpiderIdentificator::CreateHistogram(): Invalid range of theta [" << thmin_ << ";" << thmax_ << "] !" << endl;
      return 0;
   } else if ((thmin_ > thmax_) || (thmin_ < 0.) || (thmin_ >= 90.) || (thmax_ <= 0.) || (thmax_ > 90.)) {
      cout << "ERROR: KVSpiderIdentificator::CreateHistogram(): Invalid range of theta [" << thmin_ << ";" << thmax_ << "] !" << endl;
      return 0;
   } else if (nth_ <= 0) {
      cout << "ERROR: KVSpiderIdentificator::CreateHistogram(): Invalid number of step '" << nth_ << "' !" << endl;
      return 0;
   }

   double aa = 0.;
   if (alpha_ <= 0.) {
      if (_auto) aa = _alpha;
      else aa = 1.;
   } else aa = alpha_;

   double min;
   double max;
   double step;

   if (cos_) {
      min  = TMath::Cos(thmax_ * TMath::DegToRad());
      max  = TMath::Cos(thmin_ * TMath::DegToRad());
   } else {
      min = thmin_;
      max = thmax_;
   }
   if ((thmin_ == thmax_) || (nth_ == 1)) step = 10;
   else step = (max - min) / ((nth_ - 1) * 1.);

   int xbins = _htot->GetNbinsX();
   int ybins = _htot->GetNbinsY();

   for (int x = 0; x <= xbins; x++) {
      double bminx = _htot->GetXaxis()->GetBinLowEdge(x);
      double bmaxx = _htot->GetXaxis()->GetBinUpEdge(x);
      for (int y = 0; y <= ybins; y++) {
         int content = (int) _htot->GetBinContent(x, y);

         double bminy = _htot->GetXaxis()->GetBinLowEdge(y);
         double bmaxy = _htot->GetXaxis()->GetBinUpEdge(y);
         double xx  = 0.;
         double yy  = 0.;
         for (int i = 0; i < content; i++) {
            xx  = _alea.Uniform(bminx, bmaxx);
            yy  = _alea.Uniform(bminy, bmaxy);
            if (xx == bmaxx) xx = bminx;
            if (yy == bmaxy) yy = bminy;

            for (double cth = min; cth < max + 0.0001 * step; cth += step) {
               double theta;
               if (cos_) {
                  theta = TMath::ACos(cth) * TMath::RadToDeg();
               } else {
                  theta = cth;
               }
               double tmax  = TMath::Tan(TMath::DegToRad() * (theta + aa));
               double tmin  = TMath::Tan(TMath::DegToRad() * (theta - aa));


               if ((yy <= _y0 + (xx - _x0)*tmax) && (yy >= _y0 + (xx - _x0)*tmin)) {
                  if (!(_htemp = (TH2F*)_hlist.FindObject(Form("CUT_%06.3lf", theta)))) {
                     _htemp = new TH2F(Form("CUT_%06.3lf", theta), Form("CUT_%06.3lf", theta), 1024, 0, _xmax, 1024, 0, _ymax);
                     _hlist.AddLast(_htemp);

                     KVDroite* dd = new KVDroite(_x0, _y0, theta);
                     dd->SetName(_htemp->GetName());
                     _dlist.AddLast(dd);
                  }
                  _htemp->Fill(xx, yy);
               }
            }
         }
      }
   }

   //  TCanvas* cc = new TCanvas("cc1","cc1",800,800);
   //  cc->cd();
   //  ((TH2F*)_hlist.At(0))->DrawClone("col");

   return &_hlist;
}


bool KVSpiderIdentificator::SearchPeack(TH1F* h1_, double theta_, int create_, double sigma_, double peakmin_, int, int smooth_, TString opt_)
{
   if (!TestHistogram(h1_)) {
      Warning("SearchPeack", "Bad histo...");
      return false;
   }

   _dtemp = (KVDroite*)_dlist.FindObject(Form("CUT_%06.3lf", theta_));
   TF1* ff = _dtemp->GetFunction();

   //  h1_->Rebin(rebin_*_bfactor);
   h1_->Smooth(smooth_);

   if (_nAngleUp == 0 && _nAngleDown == 0) {
      opt_ = "";
   }

   int nfound = _ss.Search(h1_, sigma_, opt_.Data(), (peakmin_ / h1_->GetMaximum()));

   if (_debug && _nAngleUp == 0 && _nAngleDown == 0) {
      h1_->SaveAs("/home/dgruyer/e613/thesis/doc/proj.root");
   }

//    return 0;
//    if(_nAngleUp==0&&_nAngleDown==0)
//    {
//      TCanvas* cc = new TCanvas("can","can",800,800);
//      cc->cd();
//      h1_->DrawClone();
//      TGraph* gg = new TGraph(nfound,_ss.GetPositionX(),_ss.GetPositionY());
//      gg->DrawClone("P");
//      return 0;
//    }
   //  return 0;

   double theta = _dtemp->GetTheta();
   double costh = TMath::Cos(TMath::DegToRad() * theta);

   list<double> lx;

#if ROOT_VERSION_CODE > ROOT_VERSION(5,99,01)
   Double_t* xpeaks = _ss.GetPositionX();
#else
   Float_t* xpeaks = _ss.GetPositionX();
#endif

   for (int p = 0; p < nfound; p++) {
      double xp = xpeaks[p];
      lx.push_back(xp);
   }
   lx.sort();

   double ox   = 0.;
   double oy   = 0.;
   double dist = 0.;
   int TrueZ   = 0;
   int OTZ     = 0;

   int p  = 1;
   int ok = 0;
   int npp = 1;

   list<double>::iterator it;
   for (it = lx.begin(); it != lx.end(); ++it) {
      if (p != 1) {
         Float_t xp = *it;

         double xx = xp * costh;
         double yy = ff->Eval(xx);
         //      double yyp = _y0 + TMath::Sin(TMath::DegToRad()*(_otheta))*TMath::Cos(TMath::DegToRad()*(theta-_otheta))*TMath::Sqrt((xx-_x0)*(xx-_x0)+(yy-_y0)*(yy-_y0));

         bool valid = true;
         //      bool assoc = false;

         bool TrueAss = false;


         _invalid->SetPoint(_invalid->GetN(), xx, yy);

         double d = TMath::Sqrt((xx - ox) * (xx - ox) + (yy - oy) * (yy - oy));
         if ((!(_spline = (KVSpiderLine*)_llist.FindObject(Form("Z=%d", p))))) {
            if (create_ == 0) {
               if (valid) {
                  if (fType == kSiCsI) _spline = new KVSpiderLineSiCsI(p, GetY0());
                  else              _spline = new KVSpiderLine(p, GetY0());
                  _llist.AddLast(_spline);
               }
            } else valid = false;
         }

         double caca = 0.4;
         if (create_ == 0) caca = 0.6;

         if ((p >= 4) && (d <= caca * dist) && (valid)) {
            valid = false;

            _spline = (KVSpiderLine*)_llist.FindObject(Form("Z=%d", TrueZ));
            _spline->SetStatus(false);

            double xmoy = (_spline->GetX() * npp + xx) / (npp + 1);
            double ymoy = (_spline->GetY() * npp + yy) / (npp + 1);

            if (_spline->TestPoint(xmoy, ymoy, dist * 0.3, _useFit)) {
               _spline->ReplaceLastPoint(xmoy, ymoy);
               _spline->SetStatus(true);
               TrueZ = _spline->GetZ();
               TrueAss = true;
               ok++;
            }
            p--;
         }

         if ((p >= 4) && (valid)) {
            if ((yy >= 4000.) || (xx >= 4000.)) valid = false;
         }

         if ((create_ == 0) && (valid)) {
            if ((p >= 10) && (d >= 1.7 * dist)) {
               valid = false;
               return true;
            } else {
               _spline->AddPoint(xx, yy);
               _spline->SetStatus(true);
               TrueZ = _spline->GetZ();
               TrueAss = true;
               ok++;
            }
         } else {
            _invalid->SetPoint(_invalid->GetN(), xx, yy);
         }

         if ((create_ != 0) && (valid)) {
            bool assoc = false;
            //   bool empty = false;
            bool test  = false;
            int ii = -5;

            double d1 = 20000.;
            double d2 = 0.;

            KVSpiderLine* izp = 0;
            while ((!assoc) && (valid)) {
               if ((izp = (KVSpiderLine*)_llist.FindObject(Form("Z=%d", p + ii)))) {
                  if ((izp->GetInterpolateN() != 0)) {
                     d2 = izp->GetDistance(xx, yy);

                     if ((d2 > d1) || test) {
                        _spline = (KVSpiderLine*)_llist.FindObject(Form("Z=%d", p + ii - 1));
                        if (_spline->TestPoint(xx, yy, -1., _useFit)) {
                           _spline->AddPoint(xx, yy);
                           _spline->SetStatus(true);
                           TrueZ = _spline->GetZ();
                           assoc = true;
                           valid = false;
                           TrueAss = true;
                        } else {
                           test = true;
                        }
                     }
                     d1 = d2;
                  }
                  //     else empty = true;
               }
               if (ii >= 10) {
                  valid = true;
                  assoc = true;
               }
               ii++;
            }
            if (!valid) p += ii - 2;
         }

         if (TrueZ == OTZ) npp++;
         else npp = 1;

         if ((_spline = (KVSpiderLine*)_llist.FindObject(Form("Z=%d", TrueZ))) && (TrueZ != OTZ)) {
            xx = _spline->GetInterpolateX();
            yy = _spline->GetInterpolateY();

            if ((_spline = (KVSpiderLine*)_llist.FindObject(Form("Z=%d", OTZ)))) {
               ox = _spline->GetInterpolateX();
               oy = _spline->GetInterpolateY();
            } else {
               ox = 0.;
               oy = 0.;
            }
            dist = TMath::Sqrt((xx - ox) * (xx - ox) + (yy - oy) * (yy - oy)) / (TrueZ - OTZ);
            ox = xx;
            oy = yy;
         }
         if (TrueAss) OTZ = TrueZ;
      }
      p++;
      _otheta = theta;
   }

   return true;
}


bool KVSpiderIdentificator::ProcessIdentification()
{

   CalculateTheta();
   //  double detail_angle = _ftheta*0.3;
   Int_t angle_proc = 0;

   int cre = 0;
   CreateHistograms(_ftheta, _ftheta, 1, true, _alpha);
   TIter nexti(&_hlist);
   while ((_htemp = (TH2F*)nexti())) {
      _dtemp = (KVDroite*)_dlist.FindObject(_htemp->GetName());
      TH1F* hh = GetProjection(_htemp, _dtemp, 12);
      SearchPeack(hh, _dtemp->GetTheta(), cre, 2., 1., 12, 5);
      angle_proc += 1;
      Increment((Float_t) angle_proc);      //sends signal to GUI progress bar
      gSystem->ProcessEvents();
      delete hh;
   }
   _hlist.Clear();

   TIter nextl1(&_llist);
   while ((_spline = (KVSpiderLine*)nextl1())) {
      if (_spline->GetN() == 0) _llist.Remove(_spline);
      else {
         _spline->SetAcceptedPoints(2);
         _spline->ResetCounter();
      }
      //    else _spline->SetStatus(false);
   }

   CreateHistograms(_ftheta + 1., 89.2, _nAngleUp, true, _alpha * 0.6 / 1.7);
   _hlist.Sort();
   TIter next(&_hlist);
   while ((_htemp = (TH2F*)next())) {
      _dtemp = (KVDroite*)_dlist.FindObject(_htemp->GetName());
      TH1F* hh = GetProjection(_htemp, _dtemp, 17);
      SearchPeack(hh, _dtemp->GetTheta(), -1, 2., 1., 17, 5);
      delete hh;
      TIter nextl2(&_llist);
      while ((_spline = (KVSpiderLine*)nextl2())) {
         _spline->SetStatus(false);
         _spline->ResetCounter();
      }
      angle_proc += 1;
      Increment((Float_t) angle_proc);      //sends signal to GUI progress bar
      gSystem->ProcessEvents();
   }
   _hlist.Clear();


   TIter nextl(&_llist);
   while ((_spline = (KVSpiderLine*)nextl())) {
      _spline->Sort(true);
      _spline->ResetCounter();
   }


   cre = 1;
   //  CreateHistograms(detail_angle,_ftheta-1.,25,true,1.);
   CreateHistograms(1.5, _ftheta - 1., _nAngleDown, true, _alpha * 1. / 1.7);

   _hlist.Sort(false);
   TIter nextt(&_hlist);
   while ((_htemp = (TH2F*)nextt())) {
      _dtemp = (KVDroite*)_dlist.FindObject(_htemp->GetName());
      TH1F* hh = GetProjection(_htemp, _dtemp, 15);
      SearchPeack(hh, _dtemp->GetTheta(), cre, 2., 1., 15, 5);
      cre = 1;
      delete hh;
      TIter nextl3(&_llist);
      while ((_spline = (KVSpiderLine*)nextl3())) {
         _spline->SetStatus(false);
         _spline->ResetCounter();
      }
      angle_proc += 1;
      Increment((Float_t) angle_proc);      //sends signal to GUI progress bar
      gSystem->ProcessEvents();
   }
   _hlist.Clear();


   //   cre = 1;
   //   CreateHistograms(0.7,detail_angle-2,17,false,.5);
   //
   //   _hlist.Sort(false);
   //   TIter nexttt(&_hlist);
   //   while((_htemp=(TH2F*)nexttt()))
   //     {
   //     _dtemp = (KVDroite*)_dlist.FindObject(_htemp->GetName());
   //     TH1F* hh = GetProjection(_htemp,_dtemp,17);
   //     SearchPeack(hh,_dtemp->GetTheta(),cre,2.,1.,17,5);
   //     delete hh;
   //     }
   //   _hlist.Clear();


   TIter nextli(&_llist);
   while ((_spline = (KVSpiderLine*)nextli())) {
      _spline->Sort(true);
   }

   return true;
}


bool KVSpiderIdentificator::GetLines(int npoints_, double alpha_)
{

   CreateHistograms(_ftheta, _ftheta, 1, true, alpha_);
   TIter nexti(&_hlist);
   while ((_htemp = (TH2F*)nexti())) {
      _dtemp = (KVDroite*)_dlist.FindObject(_htemp->GetName());
      TH1F* hh = GetProjection(_htemp, _dtemp);
      SearchPeack(hh, _dtemp->GetTheta(), 0, 2., 3., 1, 5);
      delete hh;
   }
   _hlist.Clear();

   if (npoints_ == 1) return false;
   //  else npoints_;

   CreateHistograms(_ftheta + 1., _ftheta + 1. + npoints_, npoints_, true, alpha_);
   _hlist.Sort();
   TIter next(&_hlist);
   while ((_htemp = (TH2F*)next())) {
      _dtemp = (KVDroite*)_dlist.FindObject(_htemp->GetName());
      TH1F* hh = GetProjection(_htemp, _dtemp);
      SearchPeack(hh, _dtemp->GetTheta(), -1, 2., 3., 1, 5);
      delete hh;
   }
   _hlist.Clear();


   return true;
}


void KVSpiderIdentificator::Draw(Option_t* opt_)
{
   KVCanvas* cc = new KVCanvas(Form("%s_C", _htot->GetName()), Form("%s_C", _htot->GetTitle()), 800, 800);
   cc->cd()->SetLogz();
   TString option(opt_);

   _htot->SetStats(kFALSE);
   _htot->Draw("col");

   if (option.Contains("R")) {
      _invalid->SetMarkerColor(kBlack);
      _invalid->SetMarkerSize(0.7);
      _invalid->SetMarkerStyle(21);
      _invalid->Draw("Psame");
   }

   KVDroite* dd = 0;
   TF1*      ff = 0;

   if (option.Contains("D")) {
      _dlist.Sort();
      TIter nextd(&_dlist);
      Int_t id = 0;
      while ((dd = (KVDroite*)nextd())) {
         ff = dd->GetFunction();
         if (dd->GetTheta() == _ftheta) {
            ff->SetLineWidth(2);
            ff->SetLineColor(kRed);
         } else {
            ff->SetLineWidth(2);
            ff->SetLineColor(kRed);
         }
         if (id % 3 == 0) ff->Draw("same");
         id++;
      }
   }

   if (option.Contains("N") || option.Contains("L") || option.Contains("I") || option.Contains("F")) {
      TIter sp(&_llist);
      while ((_spline = (KVSpiderLine*)sp())) {
         _spline->Draw(option.Data());
         if (option.Contains("F")) {
            TF1* ff = _spline->GetFunction();
            ff->SetLineColor(kBlack);
            ff->SetLineWidth(2);
            ff->Draw("same");
         }
      }
   }

   return;
}


void KVSpiderIdentificator::SaveAsPdf(Option_t* opt_, const Char_t* path_)
{
   Draw(opt_);
   TCanvas* cc = (TCanvas*) gROOT->FindObject(_htot->GetName());
   _htot->RebinX(4);
   _htot->RebinY(4);

   CheckPath(path_);

   cc->Print(Form("%s/%s.pdf", path_, cc->GetName()), "pdf");
   cc->Close();
   delete cc;
}


TList* KVSpiderIdentificator::GetListOfLines()
{
   return &_llist;
}


KVSpiderLine* KVSpiderIdentificator::GetLine(int z_)
{
   KVSpiderLine* tmpline = 0;
   if ((tmpline = (KVSpiderLine*)_llist.FindObject(Form("Z=%d", z_)))) return tmpline;
   else {
      cout << "WARNING: KVSpiderIdentificator::GetLine(): unknown line 'Z=" << z_ << "' !" << endl;
      return 0;
   }
}













