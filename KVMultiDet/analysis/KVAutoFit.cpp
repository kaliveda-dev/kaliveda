//Created by KVClassFactory on Tue Feb  7 15:17:39 2012
//Author: bonnet

#include "KVAutoFit.h"
#include "KVConfig.h"
#include "Riostream.h"
#include "TStyle.h"
#include "TSystem.h"
#include "TH2.h"
#include "TObjString.h"

using namespace std;

ClassImp(KVAutoFit)


KVAutoFit::KVAutoFit(Bool_t batch)
{
   // Default constructor
   //
   // le booleen is2D indique si les histo et donc les fonctions
   // doivent etre a 2D ou 1D
   // par defaut is2D= kFALSE dans la methode init
   fBatch = batch;
   SetName("KVAutoFit");
   init();
   Clear();
   /*
   is2D = kTRUE;
   */

}

KVAutoFit::KVAutoFit()
{
   // Default constructor
   //

}

//________________________________________________________________
void KVAutoFit::init()
{
   lfunc = new KVHashList();
   lfunc->SetOwner(kTRUE);
   userdefined = kFALSE;
   c1 = 0;
   lplabel = 0;
   if (!fBatch) {
      c1 = new TCanvas(Form("%s_canvas", GetName()), Form("%s_canvas", GetName()), 0, 0, 1200, 600);

      c1->AddExec("interval", "ap->GetInterval()");
      c1->AddExec("recommence", "ap->Recommence()");
      c1->AddExec("gather", "ap->Gather()");

      lplabel = new KVHashList();
      lplabel->SetOwner(kTRUE);
      lplabel->Add(new TPaveLabel(0.10, 0.92, 0.25, 0.99, "Annule Tout", "NDC"));
      lplabel->Add(new TPaveLabel(0.27, 0.92, 0.44, 0.99, "Annule Dernier", "NDC"));
      lplabel->Add(new TPaveLabel(0.46, 0.92, 0.55, 0.99, "Gather", "NDC"));
      lplabel->Add(new TPaveLabel(0.57, 0.92, 0.65, 0.99, "Suivant", "NDC"));

      lplabel->Execute("SetTextSize", "0.625");
      lplabel->Execute("SetFillColor", "10");
   }

   hfit = 0;
   hclone = 0;

   f1Dfit = 0;
   f2Dfit = 0;

   gStyle->SetOptTitle(0);

   is2D = kFALSE;

   lhisto = 0;
   nhisto = 0;

   koption = "";
}

//________________________________________________________________

KVAutoFit::KVAutoFit(const KVAutoFit& obj)  : KVBase()
{
   // Copy constructor
   // This ctor is used to make a copy of an existing object (for example
   // when a method returns an object), and it is always a good idea to
   // implement it.
   // If your class allocates memory in its constructor(s) then it is ESSENTIAL :-)

   obj.Copy(*this);
}

//________________________________________________________________

void KVAutoFit::Copy(TObject& obj) const
{
   // This method copies the current state of 'this' object into 'obj'
   // You should add here any member variables, for example:
   //    (supposing a member variable KVAutoFit::fToto)
   //    CastedObj.fToto = fToto;
   // or
   //    CastedObj.SetToto( GetToto() );

   KVBase::Copy(obj);
   //KVAutoFit& CastedObj = (KVAutoFit&)obj;
}

//________________________________________________________________
void KVAutoFit::Clear(Option_t*)
{
   //Efface les fonctions de fit
   //et le contenu du canvas
   lfunc->Clear();
   if (!fBatch) c1->Clear();
   ClearRange();

}
//________________________________________________________________
void KVAutoFit::ClearRange(void)
{
   //Efface les bornes min et max pour le prochain fit
   XminSet = kFALSE;
   XmaxSet = kFALSE;

   Xmin = Xmax = -1;
   Ymin = Ymax = -1;

}

//________________________________________________________________
KVAutoFit::~KVAutoFit()
{
   // Destructor
   Clear();

   delete lfunc;
   if (!fBatch) {
      delete c1;
      delete lplabel;
   }
   hfit = 0;
   if (hclone)
      delete hclone;
   hclone = 0;
   f1Dfit = 0;
   f2Dfit = 0;

}

//________________________________________________________________
void KVAutoFit::SetHistos(KVHashList* lh, TString option)
{
   if (fBatch)
      return;

   if (option == "all") {
      Info("SetHistos", "On s'occupe de tous les histos de la liste");
   }
   else if (option == "relecture") {
      Info("SetHistos", "On s'occupe de tous les histos de la liste en relisant les infos des histos pour lesquels on en a (des info)");
   }
   else if (option == "inconnu") {
      Info("SetHistos", "On ne s'occupe que des histos pour lesquels on n'a pas d info");
   }
   else {
      Info("SetHistos", "la variable option doit prendre la valeur \"all\", \"relecture\" ou \"inconnu\"");
      return;
   }

   koption = option;

   lhisto = lh;
   nhisto = -1;
   NextHisto();

}

//________________________________________________________________
void KVAutoFit::NextHisto()
{

   if (fBatch)
      return;

   if (!lhisto) return;

   nhisto += 1;

   if (nhisto >= lhisto->GetEntries()) {
      Info("NextHisto", "Fin de la liste d histos");
      return;
   }

   if (koption == "all" || koption == "relecture") {
      SetHisto((TH1F*)lhisto->At(nhisto));
   }

   if (koption == "inconnu") {
      TString snom(lhisto->At(nhisto)->GetName());
      while (IsKnown(snom.Data())) {
         nhisto += 1;
         if (nhisto >= lhisto->GetEntries()) {
            Info("NextHisto", "Fin de la liste d histos");
            return;
         }
         snom.Form("%s", lhisto->At(nhisto)->GetName());
      }
      SetHisto((TH1F*)lhisto->At(nhisto));
   }

}

//________________________________________________________________
void KVAutoFit::Print(Option_t*) const
{
   if (hfit)
      Info("Print", "Liste des fonctions pour l'histogram %s  :", hfit->GetName());
   else
      Info("Print", "Liste des fonctions :");
   lfunc->Print();

}

//________________________________________________________________
KVHashList* KVAutoFit::GetFunctions() const
{

   return lfunc;

}

//________________________________________________________________
Bool_t KVAutoFit::NewFunction()
{
   //Generation de la fonction 1D ou 2D
   //definie dans la methode NewFunction_1D ou NewFunction_2D

   if (is2D)
      return NewFunction_2D();
   else
      return NewFunction_1D();

}
//________________________________________________________________
Bool_t KVAutoFit::NewFunction_1D()
{

   Info("NewFunction_1D", "To be defined in child class");
   return kFALSE;
}
//________________________________________________________________
Bool_t KVAutoFit::NewFunction_2D()
{
   Info("NewFunction_2D", "To be defined in child class");
   return kFALSE;

}

//________________________________________________________________
TF1* KVAutoFit::ReloadFunction(const Char_t* name, Int_t np)
{
   //Generation de la fonction 1D ou 2D
   //definie dans la methode NewFunction_1D ou NewFunction_2D

   if (is2D)
      return ReloadFunction_2D(name, np);
   else
      return ReloadFunction_1D(name, np);

}
//________________________________________________________________
TF1* KVAutoFit::ReloadFunction_1D(const Char_t*, Int_t)
{

   Info("ReloadFunction_1D", "To be defined in child class");
   return 0;
}
//________________________________________________________________
TF1* KVAutoFit::ReloadFunction_2D(const Char_t*, Int_t)
{
   Info("ReloadFunction_2D", "To be defined in child class");
   return 0;

}

//________________________________________________________________
Double_t KVAutoFit::f2D(Double_t*, Double_t*)
{
   //userdefined = kTRUE;
   return 0;

}

//________________________________________________________________
Double_t KVAutoFit::f1D(Double_t*, Double_t*)
{

   //userdefined = kTRUE;
   return 0;

}

//________________________________________________________________
void KVAutoFit::SetHisto(TH1* hh)
{

   if (fBatch)
      return;

   Clear();

   Bool_t ok = kFALSE;
   if (hh->InheritsFrom("TH2")) {
      if (is2D)
         ok = kTRUE;
   }
   else if (hh->InheritsFrom("TH1")) {
      if (!is2D)
         ok = kTRUE;
   }

   if (!ok) {
      Warning("SetHisto", "La dimension de lhisto n'est pas compatible avec celle definie ds cette classe");
      return;
   }

   if (hclone)
      delete hclone;
   hclone = (TH1*)hh->Clone("ap_clone");

   c1->cd();

   hfit = hh;
   if (is2D)   hfit->Draw("colz");
   else        hfit->Draw();

   lplabel->Execute("Draw", "");

   if (koption == "relecture")
      if (IsKnown(hfit->GetName()))
         Relecture(hfit->GetName());

   c1->Update();

}

//________________________________________________________________
void KVAutoFit::GetInterval()
{

   if (fBatch)
      return;

   Int_t event = gPad->GetEvent();
   TObject* select = gPad->GetSelected();

//   cout << event << endl;
   if (event == 24) HandleKey(gPad->GetEventY());

   if (!select) {}
   else {
      if (select->InheritsFrom("TPaveLabel"))
         return;
   }

   if (event == kButton1Down) {

      Int_t xx = gPad->GetEventX();
      Int_t yy = gPad->GetEventY();

      Xmin = gPad->AbsPixeltoX(xx);
      Ymin = gPad->AbsPixeltoY(yy);

      XminSet = kTRUE;
   }
   else if (event == kButton2Up) {

      Int_t xx = gPad->GetEventX();
      Int_t yy = gPad->GetEventY();

      Xmax = gPad->AbsPixeltoX(xx);
      Ymax = gPad->AbsPixeltoY(yy);

      if (Xmax > Xmin) {
         XmaxSet = kTRUE;
         if (Ymin > Ymax) {
            Double_t temp = Ymax;
            Ymax = Ymin;
            Ymin = temp;
         }
      }
   }
   else if (event == kButton1Double) {
      Save();
      NextHisto();
   }

   if (XminSet && XmaxSet) {

      printf("\tX : %lf %lf\n", Xmin, Xmax);
      printf("\tY : %lf %lf\n", Ymin, Ymax);


      //-------------
      //fit a 1D ou 2D
      if (NewFunction()) {
         if (!is2D) {
            hfit->Fit(f1Dfit, "0N", "", Xmin, Xmax);
            f1Dfit->Draw("same");
            ExtraDrawing();
         }
         else {
            hfit->Fit(f2Dfit, "0N", "", Xmin, Xmax);
            f2Dfit->Draw("same,cont2");
            ExtraDrawing();
         }
      }
      //-------------

      ClearRange();
      c1->Update();
   }

}

//________________________________________________________________
void KVAutoFit::Recommence()
{

   if (fBatch)
      return;

   Int_t event = gPad->GetEvent();
   TObject* select = gPad->GetSelected();
   if (!select) return;
//else printf("Cliquage sur %s %s\n",select->GetName(),select->ClassName());

   if (event == kButton1Down) {
      if (!strcmp("Annule Tout", select->GetTitle())) {
         Info("", "On efface les fonctions...");
         lfunc->Clear();
         ClearRange();
      }

      if (!strcmp("Annule Dernier", select->GetTitle())) {
         Info("", "On retire la derniere fonction ...");
         Int_t nf = lfunc->GetEntries();
         if (nf > 0)
            delete lfunc->RemoveAt(nf - 1);
         ClearRange();
      }
      if (!strcmp("Suivant", select->GetTitle())) {
         NextHisto();
      }

   }

}
//________________________________________________________________
void KVAutoFit::Gather()
{

   if (fBatch)
      return;

   Int_t event = gPad->GetEvent();
   TObject* select = gPad->GetSelected();
   if (!select) return;
   if (event == kButton1Down) {
      if (!strcmp("Gather", select->GetTitle())) {
         Info("Gather", "Do nothing");
      }
   }

}

//________________________________________________________________
void KVAutoFit::Save()
{

   if (lfunc->GetEntries() == 0) return;

   TF1* f1;
   ofstream fout(Form("%s", hfit->GetName()));
   TIter it(lfunc);
   fout << "// Functions generated by class=" << GetName() << endl;
#ifdef WITH_BZR_INFOS
   fout << "// From bzr revision=" << KVBase::bzrRevisionNumber() << endl;
#endif
#ifdef WITH_GIT_INFOS
   fout << "// From " << KVBase::gitBranch() << "@" << KVBase::gitCommit() << endl;
#endif
   fout << lfunc->GetEntries() << endl;
   while ((f1 = (TF1*)it.Next())) {
      fout << f1->GetName() << endl;
      fout << userdefined << endl;
      if (!userdefined)
         fout << f1->GetExpFormula() << endl;
      else
         fout << f1->GetNpar() << endl;
      Double_t x1, x2, y1, y2;
      if (!is2D) {
         f1->GetRange(x1, x2);
         fout << x1 << " " << x2 << endl;
      }
      else {
         f1->GetRange(x1, y1, x2, y2);
         fout << x1 << " " << x2 << " " << y1 << " " << y2 << endl;
      }
      for (Int_t ii = 0; ii < f1->GetNpar(); ii += 1) {
         fout << ii << " " << f1->GetParameter(ii) << " " << f1->GetParError(ii) << endl;
      }

   }
   fout.close();

}

//________________________________________________________________
void KVAutoFit::Relecture(const Char_t* name)
{

   Info("Relecture", "%s, Reading existing data", name);
   TObjArray* toks;
   ifstream fin(name);
   TString line;

   if (!is2D) {
      TF1* freload;
      TString f1d;
      f1d.Form("%s::f1D", GetName());
      while (fin.good()) {
         line.ReadLine(fin);
         while (line.BeginsWith("//")) line.ReadLine(fin);

         Int_t nf = line.Atoi();    //Lecture du nombre de fonctions enregistrees
         for (Int_t ii = 0; ii < nf; ii += 1) {

            line.ReadLine(fin);     //lecture du nom de la fonction
            TString snom = line;

            line.ReadLine(fin);     //lecture de l'expression ou de la methode definissant la fonction
            TString sfor = line;

            if (sfor.Atoi() == 0) {
               //Creation de la fonction a partir d une formule
               line.ReadLine(fin);
               freload = new TF1(snom.Data(), line.Data());
            }
            else {
               //lecture de l'expression ou de la methode definissant la fonction
               //lecture du nombre de parametres
               line.ReadLine(fin);
               Int_t np = line.Atoi();
               freload = ReloadFunction(snom.Data(), np);
            }

            line.ReadLine(fin);  //Lecture de l intervalle de definition
            toks = line.Tokenize(" ");
            freload->SetRange(
               ((TObjString*)toks->At(0))->GetString().Atof(),
               ((TObjString*)toks->At(1))->GetString().Atof()
            );
            delete toks;

            for (Int_t jj = 0; jj < freload->GetNpar(); jj += 1) { //Lecture des parametres
               line.ReadLine(fin);
               toks = line.Tokenize(" ");
               freload->SetParameter(jj, ((TObjString*)toks->At(1))->GetString().Atof());
               freload->SetParError(jj, ((TObjString*)toks->At(2))->GetString().Atof());
               delete toks;
            }

            lfunc->Add(freload);
            if (!fBatch)
               freload->Draw("same");

         }
      }
   }
   else {
      TF2* freload;
      TString f2d;
      f2d.Form("%s::f2D", GetName());
      while (fin.good()) {
         line.ReadLine(fin);
         while (line.BeginsWith("//")) line.ReadLine(fin);

         Int_t nf = line.Atoi();    //Lecture du nombre de fonctions enregistrees
         for (Int_t ii = 0; ii < nf; ii += 1) {

            line.ReadLine(fin);     //lecture du nom de la fonction
            TString snom = line;

            line.ReadLine(fin);     //lecture de l'expression ou de la methode definissant la fonction
            TString sfor = line;

            if (sfor.Atoi() == 0) {
               //Creation de la fonction a partir d une formule
               line.ReadLine(fin);
               freload = new TF2(snom.Data(), line.Data());
            }
            else {
               //lecture de l'expression ou de la methode definissant la fonction
               //lecture du nombre de parametres
               line.ReadLine(fin);
               Int_t np = line.Atoi();
               freload = (TF2*)ReloadFunction(snom.Data(), np);
            }

            line.ReadLine(fin);  //Lecture de l intervalle de definition
            toks = line.Tokenize(" ");
            freload->SetRange(
               ((TObjString*)toks->At(0))->GetString().Atof(),
               ((TObjString*)toks->At(2))->GetString().Atof(),
               ((TObjString*)toks->At(1))->GetString().Atof(),
               ((TObjString*)toks->At(3))->GetString().Atof()
            );
            delete toks;

            for (Int_t jj = 0; jj < freload->GetNpar(); jj += 1) { //Lecture des parametres
               line.ReadLine(fin);
               toks = line.Tokenize(" ");
               freload->SetParameter(jj, ((TObjString*)toks->At(1))->GetString().Atof());
               freload->SetParError(jj, ((TObjString*)toks->At(2))->GetString().Atof());
               delete toks;
            }

            lfunc->Add(freload);
            if (!fBatch)
               freload->Draw("cont2,same");

         }
      }
   }


   fin.close();
   Info("Relecture", "done");


}


//________________________________________________________________
Bool_t KVAutoFit::IsKnown(const Char_t* name)
{

   return (gSystem->IsFileInIncludePath(name));

}
