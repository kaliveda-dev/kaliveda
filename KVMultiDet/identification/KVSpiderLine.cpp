#include "KVSpiderLine.h"
#include <TMath.h>

using namespace std;

ClassImp(KVSpiderLine)

KVSpiderLine::KVSpiderLine(int z_, Double_t pdy)
{
   SetName(Form("Z=%d", z_));
   _z      = z_;
   _a      = -1;
   _filled = false;

   _line = new TGraph();
   _line->SetName(GetName());
   _iline = new TGraph();
   _iline->SetName(Form("I%s", GetName()));

   _ff  = 0;
   _pow = 0;
   _fitStatus = 1;
   _pdy = pdy;
   ResetCounter();
   _nAcceptedPoints = 100000;
}

KVSpiderLine::KVSpiderLine(int z_, int a_)
{
   SetName(Form("Z=%d,A=%d", z_, a_));
   _z      = z_;
   _a      = a_;
   _filled = false;

   _line = new TGraph();
   _line->SetName(GetName());
   _iline = new TGraph();
   _iline->SetName(Form("I%s", GetName()));

   _ff  = 0;
   _pow = 0;
   _fitStatus = 1;
   _pdy = 0;
   ResetCounter();
   _nAcceptedPoints = 100000;
}

KVSpiderLine::KVSpiderLine()
{
   _line = 0;
   _iline = 0;
   ResetCounter();
   _nAcceptedPoints = 100000;
}


void KVSpiderLine::SetZ(int z_)
{
   SetName(Form("Z=%d", z_));
   if (_a > 0) SetName(Form("%s,A=%d", GetName(), _a));
   _z      = z_;
   _line->SetName(GetName());
   _iline->SetName(Form("I%s", GetName()));
}

void KVSpiderLine::SetA(int a_)
{
   SetName(Form("Z=%d,A=%d", _z, a_));
   _a      = a_;
   _line->SetName(GetName());
   _iline->SetName(Form("I%s", GetName()));
}


bool KVSpiderLine::AddPoint(double x_, double y_, bool test_, int n_)
{
   if (!CheckStatus()) return false;
   int n;
   bool valid = true;

   if (n_ == -1) n = _line->GetN();
   else n = n_;

   if (test_) valid = TestPoint(x_, y_);
   if (valid) {
      _line->SetPoint(n, x_, y_);
      _iline->SetPoint(_iline->GetN(), x_, y_);
   }

   _pointsCounter++;
   return valid;
}


bool KVSpiderLine::AddInterpolatePoint(double x_, double y_, bool test_, int n_)
{
   if (!CheckStatus()) return false;
   int n;
   bool valid = true;

   if (n_ == -1) n = _iline->GetN();
   else n = n_;

   if (test_) valid = TestPoint(x_, y_);
   if (valid) _iline->SetPoint(n, x_, y_);

   return valid;
}


void KVSpiderLine::Apply(TF1* f)
{
   if (!f) return;
   if (!CheckStatus()) return;

   _line->Apply(f);
   _iline->Apply(f);

   _ff  = 0;

   return;
}


bool KVSpiderLine::ReplaceLastPoint(double x_, double y_)
{
   if (!CheckStatus()) return false;

   _line->SetPoint(GetN() - 1, x_, y_);
   _iline->SetPoint(GetInterpolateN() - 1, x_, y_);

   return true;
}


void KVSpiderLine::Sort(bool ascending_)
{
   if (!CheckStatus()) return;

   _line->Sort(&TGraph::CompareX, ascending_);
   _iline->Sort(&TGraph::CompareX, ascending_);

   return;
}


bool KVSpiderLine::GetStatus()
{
   return _filled;
}


void KVSpiderLine::SetStatus(bool filled_)
{
   _filled = filled_;
}


TF1* KVSpiderLine::GetFunction(double min_, double max_)
{
   if (!CheckStatus()) return 0;

   double min;
   double max;

   double xtest = GetX();
   if (GetX(0) > GetX()) xtest = GetX(0);
   double ytest = GetY();
   if (GetY(0) > GetY()) ytest = GetY(0);

   double p0 = TMath::Power(xtest, 0.4) * ytest;

   if (min_ == -1.) {
      if (GetX(0) < GetX()) min = GetX(0) - 10;
      else min = GetX() - 10;
      if (min <= 0.) min += 10.;
   } else min = min_;
   if (max_ == -1.) {
      if (GetX(0) < GetX()) max = GetX() + 10.;
      else max = GetX(0) + 10.;
   } else max = max_;

   if (!_ff) {
      _ff = new TF1(GetName(), Form("[0]*TMath::Power(x,%lf)/(TMath::Power((x+[1]),[2]))", _pow), min, max);
      _ff->SetParameters(p0, 100., 0.4);
   } else {
      double fmin, fmax;
      _ff->GetRange(fmin, fmax);
      if ((min <= fmin) || (max >= fmax)) {
         _ff->SetRange(min, max);
      }
   }

   _fitStatus = _line->Fit(_ff, "QN");
   return _ff;
}

bool KVSpiderLine::TestPoint(double x_, double y_, double dy_, bool fit)
{
   if (!CheckStatus()) return false;
   if (_pointsCounter >= _nAcceptedPoints) return false;

   TF1* locf = 0;
   if ((GetN() >= 10) && (fit)) {
      locf = GetFunction();
      if (_fitStatus != 0) fit = false;
   }

   bool valid = true;

   double dy;
   if (dy_ > 0.) dy = dy_;
   else {
      dy = ((GetInterpolateY() - _pdy) / _z) * 1.;
   }

   if ((GetN() >= 10) && (fit)) {
      double yext = locf->Eval(x_);
      if ((TMath::Abs(yext - y_) >= dy)) {
         valid = false;
      }
   } else if ((TMath::Abs(GetInterpolateY() - y_) >= dy)) valid = false;
   return valid;
}

double KVSpiderLine::GetDistance(double x_, double y_)
{
   double xx = x_;
   double yy = y_;
   double ox = GetInterpolateX();
   double oy = GetInterpolateY();

   double dist = TMath::Sqrt((xx - ox) * (xx - ox) + (yy - oy) * (yy - oy));
   return dist;
}

int KVSpiderLine::GetN()const
{
   if (!CheckStatus()) return false;
   return _line->GetN();
}


int KVSpiderLine::GetInterpolateN()const
{
   if (!CheckStatus()) return false;
   return _iline->GetN();
}



double KVSpiderLine::GetX()const
{
   if (!CheckStatus()) return false;
   return GetX(_line->GetN() - 1);
}

double KVSpiderLine::GetX(int n_)const
{
   if (!CheckStatus()) return false;
   int n;
   if ((n_ <= _line->GetN() - 1) && (n_ >= 0)) n = n_;
   else {
      cout << "WARNING: KVSpiderLine::GetX(): Invalid index n = '" << n_ << "'." << endl;
      return -1.;
   }
   return _line->GetX()[n];
}


double KVSpiderLine::GetY()const
{
   if (!CheckStatus()) return false;
   return GetY(_line->GetN() - 1);
}

double KVSpiderLine::GetY(int n_)const
{
   if (!CheckStatus()) return false;
   int n;
   if ((n_ <= _line->GetN() - 1) && (n_ >= 0)) n = n_;
   else {
      cout << "WARNING: KVSpiderLine::GetY(): Invalid index n = '" << n_ << "'." << endl;
      return -1.;
   }
   return _line->GetY()[n];
}



double KVSpiderLine::GetInterpolateX()const
{
   if (!CheckStatus()) return false;
   return GetInterpolateX(_iline->GetN() - 1);
}

double KVSpiderLine::GetInterpolateX(int n_)const
{
   if (!CheckStatus()) return false;
   int n;
   if ((n_ <= _iline->GetN() - 1) && (n_ >= 0)) n = n_;
   else {
      cout << "WARNING: KVSpiderLine::GetInterpolateX(): Invalid index n = '" << n_ << "'." << endl;
      return -1.;
   }
   return _iline->GetX()[n];
}


double KVSpiderLine::GetInterpolateY()const
{
   if (!CheckStatus()) return false;
   return GetInterpolateY(_iline->GetN() - 1);
}

double KVSpiderLine::GetInterpolateY(int n_)const
{
   if (!CheckStatus()) return false;
   int n;
   if ((n_ <= _iline->GetN() - 1) && (n_ >= 0)) n = n_;
   else {
      cout << "WARNING: KVSpiderLine::GetX(): Invalid index n = '" << n_ << "'." << endl;
      return -1.;
   }
   return _iline->GetY()[n];
}


void KVSpiderLine::Draw(Option_t* opt_)
{
   TString opt(opt_);
   TString com("");
   if (!CheckStatus()) return;

   if (opt.Contains("A"))    com += "A";
   if (opt.Contains("same")) com += "same";

   if (opt.Contains("I")) {
      _iline->SetMarkerColor(kGreen);
      _iline->SetMarkerSize(0.9);
      _iline->SetMarkerStyle(21);
      _iline->SetLineWidth(1);
      _iline->SetLineColor(kGreen);
      _iline->Draw(Form("PL%s", com.Data()));
      if (com.Contains("A")) com.ReplaceAll("A", "");
   }

   if (opt.Contains("L")) {
      _line->SetMarkerColor(kRed);
      _line->SetMarkerSize(1.3);
      _line->SetMarkerStyle(20);
      _line->SetLineWidth(2);
      _line->SetLineColor(kRed);
      if (_line->GetN() <= 1) _line->Draw(Form("P%s", com.Data()));
      else _line->Draw(Form("PL%s", com.Data()));
   }

   if (opt.Contains("N")) {
      TLatex* zname = 0;
      if (GetX(0) > GetX()) zname = new TLatex(GetX(0), GetY(0), GetName());
      else zname = new TLatex(GetX(), GetY(), GetName());
      zname->SetTextSize(0.02);
      zname->Draw("same");
   }
}


bool KVSpiderLine::CheckStatus()const
{
   if ((!_line) || (!_iline)) return false;
   return true;
}





















