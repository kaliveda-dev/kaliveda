//Created by KVClassFactory on Fri Nov 20 12:23:35 2015
//Author: gruyer,,,

#include "KVLevelScheme.h"
#include "KVFileReader.h"
#include "TH2F.h"
#include "TLine.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TLatex.h"
using namespace std;

ClassImp(KVLevelScheme)



KVLevelScheme::KVLevelScheme(const char* symb)
{
   // Default constructor
   ncol = 1;
   dy = 300;
   dx = 1.3;
   txs = 22;
   ddx = .6;
   hh = 0;
   cc = 0;

   fCompNuc = new KVNucleus(symb);
   fCompNuc->SetExcitEnergy(0);

   fFunc = new TF1("ExciEnergy", this, &KVLevelScheme::Evaluate, 0, 20000, 1, "KVLevelScheme", "Evaluate");
   fFunc->SetNpx(5000);

   fFuncErel = new TF1("RelEnergy", this, &KVLevelScheme::EvaluateErel, 0, 20000, 1, "KVLevelScheme", "EvaluateErel");
   fFuncErel->SetNpx(5000);

   InitStructure();
}

KVLevelScheme::~KVLevelScheme()
{
   // Destructor
   if (fFunc) delete fFunc;
}

void KVLevelScheme::InitStructure()
{
   TString dir = "";
   KVBase::SearchKVFile("levels", dir, "data");
   ReadLevels(Form("%s/%02d.dat", dir.Data(), fCompNuc->GetZ()));
}

void KVLevelScheme::ReadLevels(const char* file)
{
   KVFileReader fr;
   fr.OpenFileToRead(file);

   while (fr.IsOK()) {
      fr.ReadLine("|");
      KVString tmp  = "";

      Int_t npars  = fr.GetNparRead();
      if (npars == 0) break;

      Int_t aa, zz;
      KVString jpi, gamma;
      Double_t ee, de; //, gam, dgam;
      ee = de = 0.;

      aa = fr.GetIntReadPar(0);
      zz = fr.GetIntReadPar(1);

      if (aa != fCompNuc->GetA() || zz != fCompNuc->GetZ()) continue;

      ee = fr.GetDoubleReadPar(2);

      tmp = fr.GetReadPar(3);
      tmp.RemoveAllExtraWhiteSpace();
      if (!tmp.EqualTo("")) de = tmp.Atof();
      else                 de = 0.;

      tmp = fr.GetReadPar(4);
      tmp.RemoveAllExtraWhiteSpace();
      if (!tmp.EqualTo("")) {
         jpi = tmp.Data();
         jpi.ReplaceAll("GE", "");
         jpi.ReplaceAll(" &amp; ", ",");
         jpi.ReplaceAll("LE", "");
         jpi.ReplaceAll("LT", "");
         jpi.ReplaceAll("AP", "");
         jpi.ReplaceAll(" OR ", "or");
         jpi.ReplaceAll(" TO ", "to");
      }
      else jpi = "";

      gamma = "";
      if (npars == 6) {
         gamma = fr.GetReadPar(5);
      }

      AddResonance(ee, jpi.Data(), gamma.Data());
   }
   fr.CloseFile();
}

void KVLevelScheme::AddResonance(Double_t ex, const char* jpi, const char* gam)
{
   KVExcitedState* ll = new KVExcitedState;
   ll->set(ex, jpi, gam);
   fLevels.AddLast(ll);
}

void KVLevelScheme::Print()
{
   cout << endl;
   cout << "  n." << "            Jpi" << "            E" << "           T1/2" << endl;
   cout << endl;
   for (Int_t ii = (int)fLevels.GetSize() - 1; ii >= 0; ii--) {
      KVExcitedState* ll = (KVExcitedState*)fLevels.At(ii);
      cout << Form("%3d", ii) << ".";
      ll->print();
   }
   cout << endl;
   cout << "                    " << Form("%5s", fCompNuc->GetSymbol()) << endl;
   cout << endl;
}

double KVLevelScheme::func(double xx, double tt)
{
   double yy = 0.;
   for (Int_t ii = 0; ii < fLevels.GetSize(); ii++) {
      KVExcitedState* ll = (KVExcitedState*)fLevels.At(ii);
      yy += ll->eval(xx);
   }
   yy *= TMath::Exp(-xx / tt);
   return yy;
}

double KVLevelScheme::getWidth(double gam, TString unit)
{
   unit.ToUpper();
   if (unit.EqualTo("EV"))       gam *= 1e-3;
   else if (unit.EqualTo("KEV")) gam *= 1;
   else if (unit.EqualTo("MEV")) gam *= 1e3;
   else if (unit.Contains("S"))  gam *= 1e-6;
   return gam;
}

int KVLevelScheme::getJ(TString jpi)
{
   jpi.ReplaceAll("(", "");
   jpi.ReplaceAll(")", "");

   int j = 0;

   if (jpi.EqualTo(""))   return 0;
   if (jpi.EqualTo("GE")) return 0;

   if (jpi.Contains("-"))  jpi = jpi(0, jpi.Index("-"));
   if (jpi.Contains("+"))  jpi = jpi(0, jpi.Index("+"));
   if (jpi.Contains("/2")) jpi = jpi(0, jpi.Index("/"));
   if (jpi.Contains(","))  jpi = jpi(0, jpi.Index(","));

   j = jpi.Atoi();
   return j;
}

Double_t KVLevelScheme::GetLevelEnergy(int il)
{
   if (il > (int)fLevels.GetSize()) return 0;
   else return ((KVExcitedState*) fLevels.At(il))->fEnergy;
}

Double_t KVLevelScheme::GetLevelWidth(int il)
{
   if (il > (int)fLevels.GetSize()) return 0;
   else return ((KVExcitedState*) fLevels.At(il))->fWidth;
}

Int_t KVLevelScheme::GetLevelSpin(int il)
{
   if (il > (int)fLevels.GetSize()) return 0;
   else return ((KVExcitedState*) fLevels.At(il))->fSpin;
}

Int_t KVLevelScheme::GetLevelParity(int il)
{
   if (il > (int)fLevels.GetSize()) return 0;
   else return ((KVExcitedState*) fLevels.At(il))->fParity;
}

const char* KVLevelScheme::GetJPiStr(int il)
{
   if (il > (int)fLevels.GetSize()) return "";
   else return ((KVExcitedState*) fLevels.At(il))->fJPi.Data();
}
const char* KVLevelScheme::GetEGammaStr(int il)
{
   if (il > (int)fLevels.GetSize()) return 0;
   KVString gam = ((KVExcitedState*) fLevels.At(il))->fGamma.Data();
   gam.RemoveAllExtraWhiteSpace();
   gam.ReplaceAll("-1.0", "-");
   return Form("%d (%s)", TMath::Nint(((KVExcitedState*) fLevels.At(il))->fEnergy), gam.Data());
}

int KVLevelScheme::getPI(TString jpi)
{
   jpi.ReplaceAll("(", "");
   jpi.ReplaceAll(")", "");

//    int j = 0;
   Int_t pi = 0;

   if (jpi.EqualTo(""))   return 0;
   if (jpi.EqualTo("GE")) return 0;

   // determination of pi
   if ((jpi.Contains("-")) && (jpi.Contains("+"))) {
      if (jpi.Index("+") < jpi.Index("-")) pi = 1;
      else  pi = -1;
   }
   else if (jpi.Contains("-")) pi = -1;
   else                       pi = 1;

   return pi;
}


double KVLevelScheme::Evaluate(double* x, double* p)
{
   double xx = x[0];
   double tt = p[0];
   return func(xx, tt);
}

double KVLevelScheme::EvaluateErel(double* x, double* p)
{
   double xx = x[0];
   double tt = p[0];
   return func(xx - fQvalue * 1000, tt);
}

void KVLevelScheme::GetParticlesFromErel(KVNucleus* n1, KVNucleus* n2, double erel, bool randAngle, TVector3* vsrc)
{
   KVNucleus nuc1(n1->GetZ(), n1->GetA());
   KVNucleus nuc2(n2->GetZ(), n2->GetA());

   double m1 = nuc1.GetMass();
   double m2 = nuc2.GetMass();
   double e1 = (m2 / (m1 + m2)) * erel;
   double e2 = (m1 / (m1 + m2)) * erel;

   nuc1.SetKE(e1);
   nuc1.SetTheta(0);
   nuc2.SetKE(e2);
   nuc2.SetTheta(180);

   if (randAngle) {
      TVector3 v1r = nuc1.GetVelocity();
      TVector3 v2r = nuc2.GetVelocity();
      TRotation rr;
      rr.SetXEulerAngles(gRandom->Rndm() * 2.*TMath::Pi(), TMath::ACos(gRandom->Rndm() * 2. - 1.), gRandom->Rndm() * 2.*TMath::Pi());
      v1r *= rr;
      nuc1.SetVelocity(v1r);
      v2r *= rr;
      nuc2.SetVelocity(v2r);
   }

   if (vsrc) {
      *vsrc *= -1;
      nuc1.SetFrame("src", *vsrc);
      nuc2.SetFrame("src", *vsrc);
      n1->SetVelocity(nuc1.GetFrame("src")->GetVelocity());
      n2->SetVelocity(nuc2.GetFrame("src")->GetVelocity());
   }

}

void KVLevelScheme::GetParticlesFromExci(KVNucleus* nuc1, KVNucleus* nuc2, double erel, bool randAngle, TVector3* vsrc)
{
   KVNucleus comp = *nuc1 + *nuc2;
   double qq = -1 * (comp.GetExcitEnergy());
   GetParticlesFromErel(nuc1, nuc2, erel + qq, randAngle, vsrc);
}

void KVLevelScheme::GetRandomParticles(KVNucleus* n1, KVNucleus* n2, double T/*keV*/, bool randAngle, TVector3* vsrc)
{
   if (!fLevels.GetSize()) cout << "KVLevelScheme::GetRandomParticles: please initialize the level scheme firts..." << endl;
   fFunc->SetParameter(0, T);
   double excit = fFunc->GetRandom();

   GetParticlesFromExci(n1, n2, excit, randAngle, vsrc);
}

void KVLevelScheme::SetDrawStyle(double deMin, double fullWidth, double lineWidth, int textSize)
{
   dy  = deMin;
   dx  = fullWidth;
   ddx = lineWidth;
   txs = textSize;
}

void KVLevelScheme::Draw(Option_t* /*option*/)
{
   ncol = 1;
   int icol = 0;

   int cols[20];
   for (int ic = 0; ic < 20; ic++) cols[ic] = 0;
   cols[0] = GetLevelEnergy(0);

   for (int ii = 0; ii < GetNLevels(); ii++) {
      for (int ic = 0; ic < TMath::Max(ii, 20); ic++) {
         if ((GetLevelEnergy(ii) - cols[ic] > dy)) {
            cols[ic] = GetLevelEnergy(ii);
            icol = ic;
            if (icol + 1 > ncol) ncol = icol + 1;
            break;
         }
      }
   }

   double max = GetLevelEnergy(GetNLevels() - 1);
   if (hh) delete hh;
   hh = new TH2F(Form("dumhist%s", fCompNuc->GetSymbol()), "", 1, 0, ncol * dx + 0.5 * dx, 1000, -200, max + 200);

//   TString opt = option;
   gStyle->SetOptStat(0);
//   if (!opt.Contains("same")) {
   if (cc) delete cc;
   cc = new TCanvas(Form("levels%s", fCompNuc->GetSymbol()), Form("levels%s", fCompNuc->GetSymbol()), (ncol) * (dx) * 0.4 * 400, 800);
   cc->SetTickx(1);
   cc->SetTicky(1);
   cc->SetTopMargin(0.02);
   cc->SetBottomMargin(0.02);
   cc->SetRightMargin(0.02);
   cc->SetLeftMargin(0.02);

   hh->GetXaxis()->SetAxisColor(0);
   hh->GetYaxis()->SetAxisColor(0);

   hh->GetXaxis()->SetLabelSize(0);
   hh->GetYaxis()->SetNdivisions(0);

   hh->Draw();
//   }

   for (int ic = 0; ic < 20; ic++) cols[ic] = 0;
   cols[0] = GetLevelEnergy(0);

   icol = 0;
   for (int ii = 0; ii < GetNLevels(); ii++) {
      for (int ic = 0; ic < TMath::Max(ii, 20); ic++) {
         if ((GetLevelEnergy(ii) - cols[ic] > dy)) {
            cols[ic] = GetLevelEnergy(ii);
            icol = ic;
            break;
         }
      }

      TLine* ll = new TLine(0.5 + dx * icol, GetLevelEnergy(ii), ddx + .5 + dx * icol, GetLevelEnergy(ii));
      ll->Draw("same");
      TString jpi = GetJPiStr(ii);
      jpi.ReplaceAll("+", "^{+}");
      jpi.ReplaceAll("-", "^{-}");
      TLatex* tex = new TLatex(.4 + dx * icol, GetLevelEnergy(ii), jpi.Data());
      tex->SetTextAlign(32);
      tex->SetTextFont(133);
      tex->SetTextSize(txs);
      //        tex->Draw();

      tex = new TLatex(ddx + .6 + dx * icol, GetLevelEnergy(ii), Form("%d", TMath::Nint(GetLevelEnergy(ii)))); //GetEGammaStr(ii));
      tex->SetTextAlign(12);
      tex->SetTextFont(133);
      tex->SetTextSize(txs);
      tex->Draw();
   }

   TLatex* tte = new TLatex(0.5, 0.95, Form("^{%d}%s", fCompNuc->GetA(), fCompNuc->GetSymbol("EL")));
   tte->SetNDC(1);
   tte->SetTextAlign(23);
   tte->SetTextFont(133);
   tte->SetTextSize(txs);
   tte->Draw();

   hh->GetXaxis()->SetLimits(0, dx * (ncol + 0.5));
}

void KVLevelScheme::DrawThreshold(const char* symb, Option_t* option, double ex)
{
   TString opt = option;
   ncol++;
   KVNucleus a(symb);

   KVNucleus tmp = *fCompNuc - a;
   double qa = tmp.GetExcitEnergy() * -1.;

   TLatex* tte = 0;
   TLine* lq = 0;

   TString decay = Form("^{%d}%s + ^{%d}%s", a.GetA(), a.GetSymbol("EL"), tmp.GetA(), tmp.GetSymbol("EL"));
   tte = new TLatex(0.5 + (0.5 * ddx) + dx * (ncol - 1), qa * 1000 - 100, decay.Data());
   tte->SetTextAlign(23);
   tte->SetTextFont(133);
   tte->SetTextSize(txs);
   tte->Draw();


   if (opt.Contains("l")) {
      lq = new TLine(0.5, qa * 1000, 0.5 + dx * (ncol - 1), qa * 1000);
      lq->SetLineColor(kGray);
      lq->SetLineStyle(7);
      lq->Draw();
   }

   lq = new TLine(0.5 + dx * (ncol - 1), qa * 1000, 0.5 + ddx + dx * (ncol - 1), qa * 1000);
   lq->Draw();

   tte = new TLatex(ddx + .6 + dx * (ncol - 1), qa * 1000, Form("%d", TMath::Nint(1000 * qa)));
   tte->SetTextAlign(12);
   tte->SetTextFont(133);
   tte->SetTextSize(txs);
   tte->Draw();

   if (ex > 0) {
      a.SetExcitEnergy(ex / 1000.);
      tmp = *fCompNuc - a;
      qa = tmp.GetExcitEnergy() * -1.;

      if (opt.Contains("l")) {
         lq = new TLine(0.5, qa * 1000, 0.5 + dx * (ncol - 1), qa * 1000);
         lq->SetLineColor(kGray);
         lq->SetLineStyle(7);
         lq->Draw();
      }

      lq = new TLine(0.5 + dx * (ncol - 1), qa * 1000, 0.5 + ddx + dx * (ncol - 1), qa * 1000);
      lq->Draw();
      tte = new TLatex(ddx + .6 + dx * (ncol - 1), qa * 1000, Form("%d", TMath::Nint(1000 * qa)));
      tte->SetTextAlign(12);
      tte->SetTextFont(133);
      tte->SetTextSize(txs);
      tte->Draw();
   }
   cc->SetWindowSize((ncol) * (dx) * 0.35 * 400, 800);
   hh->GetXaxis()->SetLimits(0, dx * (ncol + 0.5));
}

double KVLevelScheme::GetThreshold(const char* outnuc)
{
   if (!strcmp(outnuc, "")) outnuc = "1H";
   KVNucleus a(outnuc);
   KVNucleus tmp = *fCompNuc - a;
   return tmp.GetExcitEnergy() * -1000.; //returns the threshold in keV
}

void KVLevelScheme::SetDecayProduct(KVNucleus* nuc, double excit_energy)
{
   fDecayProd = nuc;
   fDecayProd->SetExcitEnergy(excit_energy * 0.001);
   KVNucleus tmp = *fCompNuc - *fDecayProd;
   fQvalue = tmp.GetExcitEnergy() * -1.;
}
