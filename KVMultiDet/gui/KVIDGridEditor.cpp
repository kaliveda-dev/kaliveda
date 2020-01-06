//Created by KVClassFactory on Fri Feb 17 11:05:30 2012
//Author: dgruyer

#include "KVIDGridEditor.h"

#include "TSpectrum.h"
#include "TStyle.h"
#include "TROOT.h"
#include <iostream>
#include <sstream>

#include <KeySymbols.h>
#include <KVSpIdGUI.h>
#include <KVZAFinderDialog.h>
#include "KVTreeAnalyzer.h"
#include <KVHistogram.h>
#include "KVTestIDGridDialog.h"
#include "KVItvFinderDialog.h"
#include "TFrame.h"

using namespace std;

ClassImp(KVIDGridEditor)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVIDGridEditor</h2>
<h4>Editeur de grille d'identification</h4>
<img alt="KVIDGridEditor" src="http://indra.in2p3.fr/KaliVedaDoc/images/IDGridEditor.png"><br><br>
Doc :  <a href="http://indra.in2p3.fr/KaliVedaDoc/images/DocGridEditor.pdf"> DocGridEditor.pdf </a> (FR)
Presentation de l'editeur : <a href="http://indra.in2p3.fr/KaliVedaDoc/images/JourneeAnalyse0312_1.pdf"> JourneeAnalyse0312_1.pdf </a> (FR)

<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVIDGridEditor* gIDGridEditor = nullptr;

KVIDGridEditor::KVIDGridEditor()
{
   // Default constructor
   fSpiderOption = "DRLF";
   gStyle->SetPalette(55);

   // Style
   gStyle->SetPadLeftMargin(0.1);
   gStyle->SetPadBottomMargin(0.1);
   gStyle->SetPadTopMargin(0.1);
   gStyle->SetPadRightMargin(0.1);
   gStyle->SetPadTickX(1);
   gStyle->SetPadTickY(1);
   gStyle->SetNdivisions(310, "xyz");
   gStyle->SetLabelSize(0.05, "xyz");
   gStyle->SetTitleSize(0.05, "xyz");
   gStyle->SetTitleOffset(1., "xzy");
   gStyle->SetTitleFont(42, "xyz");
   gStyle->SetLabelFont(42, "xyz");
   gStyle->SetOptStat(0);
   gStyle->SetOptTitle(0);
   gROOT->ForceStyle();

   fPointStyle = 4;
   fPointSize = 0.8;

   gIDGridEditor = this;

   SetName("gIDGridEditor");
   SetDefault();

   fListOfMethods = "";
   fDefaultMethod = "";

   AddMethod("SaveCurrentGrid");
   AddMethod("ChangeMasses");
   AddMethod("ChangeCharges");
   AddMethod("SelectLinesByZ");
   AddMethod("MakeScaleX");
   AddMethod("MakeScaleY");
   AddMethod("SetSelectedColor");
   AddMethod("SetPointStyle");
   AddMethod("SetPointSize");
   AddMethod("SetVarXVarY");
   AddMethod("SetRunList");
   AddMethod("AddParameter");
   AddMethod("SetXScaleFactor");
   AddMethod("SetYScaleFactor");
   AddMethod("SetSVGMode");

   ft  = new TF1("tranlation", "(x+[0])", 0, 50000);
   fs  = new TF1("scale", "(x-[0])*[1]+[0]", 0, 50000);
   fsy = new TF1("scale_y", "(x-[0])*[1]+[0]", 0, 50000);

   frx = new TF2("rotation_x", "(x-[0])*TMath::Cos([2])-(y-[1])*TMath::Sin([2])+[0]", 0, 50000);
   fry = new TF2("rotation_y", "(x-[0])*TMath::Sin([2])+(y-[1])*TMath::Cos([2])+[1]", 0, 50000);

   ListOfLines = new KVList;
   ListOfLines->SetOwner(kFALSE);

   lplabel = new KVHashList();
   lplabel->SetOwner(kTRUE);
   lplabel2 = new KVHashList();
   lplabel2->SetOwner(kTRUE);
   lplabel3 = new KVHashList();
   lplabel3->SetOwner(kTRUE);
   lplabel4 = new KVHashList();
   lplabel4->SetOwner(kTRUE);
   lplabel5 = new KVHashList();
   lplabel5->SetOwner(kTRUE);

   ResetScalingRecap();

//   MakeCustomMenuForLines();

}

//________________________________________________________________
void KVIDGridEditor::StartViewer()
{
   //  Close();

   if (IsClosed()) {
      fCanvas = new KVIDGridEditorCanvas(Form("%sCanvas", GetName()), Form("%sCanvas", GetName()), 800, 600);
      fCanvas->AddExec("transform", "gIDGridEditor->MakeTransformation()");
      fCanvas->AddExec("recommence", "gIDGridEditor->SelectLabel()");
      // connect canvas' Closed() signal to method CanvasWasClosed().
      // this way we always know if the canvas is closed by user closing the window
      fCanvas->Connect("Closed()", "KVIDGridEditor", this, "CanvasWasClosed()");
      fPad = fCanvas->cd();

      if (!ready) init();
      if (TheHisto) TheHisto->Draw("col");
      if (TheGrid)  TheGrid->Draw();
      DrawAtt(false);
   }
}

void KVIDGridEditor::CanvasWasClosed()
{
   // Slot connected to the 'Closed()' signal of the canvas.
   // If the user closes the canvas window this method gets called.
   fCanvas = 0;
   fPad = 0;
}

//________________________________________________________________
void KVIDGridEditor::SetDefault()
{
   TheHisto = 0;
   TheGrid  = 0;
   fPivot  = 0;
   fPiedestal  = 0;
   fPad  = 0;
   x0 = y0 = 0.;
   fCanvas = 0;
   fKeyShow = 0;

   fSpiderFactor = -1.;
   fSpiderZp = -1;

   itrans = iact = iopt = 0;
   imod = 20;

   ownhisto   = false;
   dlmode     = false;
   drawmode   = false;
   selectmode = false;
   aoemode    = false;
   moved      = false;
   venermode  = false;
   fDebug     = false;
   ready      = false;
   is_col     = false;

   fSVGMode   = false;
   fBlackMode = false;
   fJoelMode  = false;
   fSVGIndex  = 0;

   SelectedColor = kOrange + 1;
}

//________________________________________________________________
Bool_t KVIDGridEditor::IsClosed()
{
   return (!fCanvas);
}

//________________________________________________________________
void KVIDGridEditor::Close()
{
   if (!IsClosed()) {
      fCanvas->Disconnect("Closed()", this, "CanvasWasClosed()");
      fCanvas->Close();
      delete fCanvas;
      fCanvas = 0;
      fPad = 0;
   }
   return;
}

//________________________________________________________________
void KVIDGridEditor::Clear(const Option_t* opt)
{
   TString option(opt);
   option.ToUpper();

   dlmode     = false;
   drawmode   = false;
   selectmode = false;
   moved      = false;

   SelectedColor = kOrange + 1;

   ResetColor(ListOfLines);
   ListOfLines->Clear();

   if (option.Contains("AL")) {
      if ((TheHisto)) {
         if (ownhisto) {
            TheHisto->Delete();
            delete TheHisto;
            ownhisto = false;
         }
         TheHisto->Delete();
         TheHisto = 0;
      }
      if (TheGrid) {
         if (!IsClosed()) TheGrid->UnDraw();
         TheGrid = 0;
      }
      fPad->Clear();
   }

   SetPivot(0., 0.);

   lplabel->Execute("SetTextSize", "0.625");
   lplabel->Execute("SetFillColor", "kWhite");
   lplabel2->Execute("SetTextSize", "0.625");
   lplabel2->Execute("SetFillColor", "kWhite");
   lplabel3->Execute("SetTextSize", "0.6");
   lplabel3->Execute("SetFillColor", "kWhite");
   lplabel4->Execute("SetTextSize", "0.6");
   lplabel4->Execute("SetFillColor", "kWhite");

   lplabel->Execute("SetBorderSize", "1");
   lplabel2->Execute("SetBorderSize", "1");
   lplabel3->Execute("SetBorderSize", "1");
   lplabel4->Execute("SetBorderSize", "1");
   lplabel5->Execute("SetBorderSize", "1");

   DrawAtt(true);
   UpdateViewer();
}

//________________________________________________________________
void KVIDGridEditor::UpdateViewer()
{
   if (!IsClosed()) {
      fPad->Modified();
      fPad->Update();
      if (fSVGMode) {
         TString dir("$SVGDOC");
         if (gSystem->ExpandPathName(dir)) dir = ".";
         fCanvas->SaveAs(Form("%s/gIDGridEditorPrint-%d.png", dir.Data(), fSVGIndex));
         //      gROOT->ProcessLine(Form(".! import -window root %s/gIDGridEditorPrint-%d.png",dir.Data(),fSVGIndex));
         if (!gSystem->AccessPathName(Form("%s/gIDGridEditorPrint-%d.png", dir.Data(), fSVGIndex))) fSVGIndex++;
      }
   }
}

//________________________________________________________________
KVIDGridEditor::KVIDGridEditor(const KVIDGridEditor& obj)  : KVBase()
{
   // Copy constructor
   // This ctor is used to make a copy of an existing object (for example
   // when a method returns an object), and it is always a good idea to
   // implement it.
   // If your class allocates memory in its constructor(s) then it is ESSENTIAL :-)

   obj.Copy(*this);
}

//________________________________________________________________
KVIDGridEditor::~KVIDGridEditor()
{
   // Destructor

   if (ownhisto) delete TheHisto;
   if (fCanvas) delete fCanvas;
   if (lplabel) {
      lplabel->Delete("all");
      delete lplabel;
   }
   if (lplabel2) {
      lplabel2->Delete("all");
      delete lplabel2;
   }
   if (lplabel3) {
      lplabel3->Delete("all");
      delete lplabel3;
   }
   if (lplabel4) {
      lplabel4->Delete("all");
      delete lplabel4;
   }
   if (lplabel5) {
      lplabel5->Delete("all");
      delete lplabel5;
   }
   if (gIDGridEditor == this) gIDGridEditor = 0x0;

}

//________________________________________________________________
void KVIDGridEditor::Copy(TObject& obj) const
{
   // This method copies the current state of 'this' object Int_to 'obj'
   // You should add here any member variables, for example:
   //   (supposing a member variable KVIDGridEditor::fToto)
   //   CastedObj.fToto = fToto;
   // or
   //   CastedObj.SetToto( GetToto() );

   KVBase::Copy(obj);
   //KVIDGridEditor& CastedObj = (KVIDGridEditor&)obj;
}

//________________________________________________________________
void KVIDGridEditor::init()
{
   AddTransformation("T_{X}");
   AddTransformation("T_{Y}");
   AddTransformation("R_{Z}");
   AddTransformation("S_{X}");
   AddTransformation("S_{Y}");
   AddTransformation("S_{XY}");
   AddTransformation("S_{C}");

   AddAction("#odot");
   //  AddAction("0");
   AddAction("#Leftarrow");
   AddAction("Lz");
   AddAction("Ly");
   AddAction("Lx");

   //  AddGridOption("All",lplabel3);
   AddGridOption("Select", lplabel3);

   AddGridOption("Edit", lplabel4);
   AddGridOption("Delete", lplabel4);
   AddGridOption("Line", lplabel4);
   AddGridOption("Cut", lplabel4);
   AddGridOption("Fit", lplabel4);
   AddGridOption("Test", lplabel4);
   AddGridOption("Mass", lplabel4);
   AddGridOption("Spider", lplabel4);
   AddGridOption("More", lplabel4);

   ConstructModulator();

   gStyle->SetOptTitle(0);
   gStyle->SetOptStat(0);
   fCanvas->Clear();

   SetPivot(0., 0.);
   SetPiedestal(0., 0.);
   Clear("ALL");
   ready = true;

   return;
}

//________________________________________________________________
void KVIDGridEditor::ConstructModulator()
{
   Int_t i = 0;

   Double_t x1 = 0.92;
   Double_t x2 = 0.99;

   Double_t y1 = 0.90 - i * 0.06;
   Double_t y2 = y1 - 0.05;

   TPaveLabel* templabel = 0;

   templabel = new TPaveLabel(x1, y1, x2, y2, "+", "NDC");
   templabel->SetName(templabel->GetTitle());
   templabel->SetFillColor(kWhite);
   lplabel5->Add(templabel);

   i++;
   y1 = 0.90 - i * 0.06;
   y2 = y1 - 0.05;

   modulator = new TPaveLabel(x1, y1, x2, y2, Form("%d", imod), "NDC");
   modulator->SetName(templabel->GetTitle());
   modulator->SetFillColor(kGreen);
   lplabel5->Add(modulator);

   i++;
   y1 = 0.90 - i * 0.06;
   y2 = y1 - 0.05;

   templabel = new TPaveLabel(x1, y1, x2, y2, "-", "NDC");
   templabel->SetName(templabel->GetTitle());
   templabel->SetFillColor(kWhite);
   lplabel5->Add(templabel);

   lplabel5->Execute("SetTextSize", "0.625");
}

//________________________________________________________________
void KVIDGridEditor::AddTransformation(TString label)
{
   Double_t y1 = 0.92;
   Double_t y2 = 0.99;

   Double_t x1 = 0.10 + itrans * 0.06;
   Double_t x2 = x1 + 0.05;

   TPaveLabel* templabel = 0;

   templabel = new TPaveLabel(x1, y1, x2, y2, label.Data(), "NDC");
   templabel->SetName(templabel->GetTitle());
   lplabel->Add(templabel);

   itrans++;
   return;
}

//________________________________________________________________
void KVIDGridEditor::AddAction(TString label)
{
   Double_t y1 = 0.92;
   Double_t y2 = 0.99;

   Double_t x2 = 0.90 - iact * 0.06;
   Double_t x1 = x2 - 0.05;

   TPaveLabel* templabel = 0;

   templabel = new TPaveLabel(x1, y1, x2, y2, label.Data(), "NDC");
   templabel->SetName(templabel->GetTitle());
   lplabel2->Add(templabel);

   iact++;
   return;
}

//________________________________________________________________
void KVIDGridEditor::AddGridOption(TString label, KVHashList* thelist)
{
   Double_t x1 = 0.92;
   Double_t x2 = 0.99;

   Double_t y1 = 0.10 + iopt * 0.06;
   Double_t y2 = y1 + 0.05;

   TPaveLabel* templabel = 0;

   templabel = new TPaveLabel(x1, y1, x2, y2, label.Data(), "NDC");
   templabel->SetName(templabel->GetTitle());
   thelist->Add(templabel);

   iopt++;
   return;
}

void KVIDGridEditor::RecurseFileStructureFindHistos(TString& hist_names, TDirectory* the_directory)
{
   // Recursively scan folders in a file looking for all TH2-derived objects.
   // Their names are added to the TString.

   if (!the_directory) return;
   TIter next_key(the_directory->GetListOfKeys());
   TKey* key;
   while ((key = (TKey*)next_key())) {
      if (key->IsFolder()) {
         RecurseFileStructureFindHistos(hist_names, the_directory->GetDirectory(key->GetName()));
      }
      else {
         TString key_class = key->GetClassName();
         if (key_class.Contains("TH2")) hist_names += Form(" %s", key->GetName());
      }
   }
}

//________________________________________________________________
TString KVIDGridEditor::ListOfHistogramInMemory()
{
   // Fill a TString with the names of all histograms in
   //   - ROOT memory (gROOT)
   //   - all open files
   //   - all canvases
   //   - any instance of KVTreeAnalyzer
   // The list is sorted lexographically

   KVString HistosNames = "";
   KVList histos;

   // histos in memory
   TIter nextmem(gROOT->GetList());
   TObject* obj;
   while ((obj = nextmem())) {
      //if (obj->InheritsFrom("TH2")) HistosNames += Form(" %s", obj->GetName());
      if (obj->InheritsFrom("TH2")) histos.Add(new TObjString(obj->GetName()));
   }

   // histos in files
   TFile* f;
   TIter next(gROOT->GetListOfFiles());
   while ((f = (TFile*)next())) {
      // beware KV database file!!!
      if (!strcmp(gSystem->BaseName(f->GetName()), "DataBase.root")) continue;
      RecurseFileStructureFindHistos(HistosNames, f);
   }
   HistosNames.Begin(" ");
   while (!HistosNames.End()) {
      KVString g = HistosNames.Next();
      if (g != "gIDGridEditorDefaultHistogram") histos.Add(new TObjString(g.Data()));
   }
   HistosNames = "";

   // histos in canvases
   TIter nextc(gROOT->GetListOfCanvases());
   TCanvas* canv = 0;
   while ((canv = (TCanvas*)nextc())) {
      //printf("%s\n",canv->GetName());
      if (strcmp(canv->GetName(), "gIDGridEditorCanvas")) {
         TIter next_step1(canv->GetListOfPrimitives());
         TObject* obj1 = 0;
         while ((obj1 = next_step1())) {
            //printf("%s\n",obj1->GetName());
            if (obj1->InheritsFrom("TPad")) {
               TObject* obj2 = 0;
               TIter next_step2(((TPad*)obj1)->GetListOfPrimitives());
               while ((obj2 = next_step2())) {
                  printf("%s\n", obj2->GetName());
                  if (obj2->InheritsFrom("TH2")) {
                     //HistosNames += Form(" %s", obj2->GetName());
                     histos.Add(new TObjString(obj2->GetName()));
                  }
               }
            }
            else if (obj1->InheritsFrom("TH2")) {
               //HistosNames += Form(" %s", ((TH2*)obj1)->GetName());
               histos.Add(new TObjString(obj1->GetName()));
            }
         }
      }
   }

   // KVTreeAnalyzer
   if (gTreeAnalyzer) {
      // Get all 2-D histograms from current KVTreeAnalyzer instance
      TIter nexthist(gTreeAnalyzer->GetHistoList());
      KVHistogram* obj = 0;
      while ((obj = (KVHistogram*)nexthist())) {
         if (obj->IsType("Histo") && obj->GetHisto()->InheritsFrom("TH2")) {
            //HistosNames += Form(" %s", obj->GetName());
            histos.Add(new TObjString(obj->GetName()));
         }
      }
   }
   //if (HistosNames.Contains("gIDGridEditorDefaultHistogram")) HistosNames.ReplaceAll("gIDGridEditorDefaultHistogram", "");
   TObjString* s = (TObjString*)histos.FindObject("gIDGridEditorDefaultHistogram");
   if (s) {
      histos.Remove(s);
      delete s;
   }
   // sort list of histograms by name
   histos.Sort();
   TIter ith(&histos);
   while ((s = (TObjString*)ith())) {
      HistosNames += Form(" %s", s->GetString().Data());
   }

   return HistosNames;
}

//________________________________________________________________
TString KVIDGridEditor::PreselectHistogram(TString ListOfName, Int_t)
{
   // Look in list of histogram names for one containing the name of
   // the current grid. If found, return it.

   if (!TheGrid) return "";
   TString result = "";
   TString Iter;

   KVString str(ListOfName.Data());
   str.Begin(" ");
   while (!str.End()) {
      Iter = str.Next(kTRUE);
      if (Iter.EqualTo(TheGrid->GetName())) {
         result = Iter.Data();
         return result;
      }
   }

   str.Begin(" ");
   while (!str.End()) {
      Iter = str.Next(kTRUE);
      if (Iter.Contains(TheGrid->GetName())) {
         result = Iter.Data();
         return result;
      }
   }

   return result;
}

//________________________________________________________________
void KVIDGridEditor::SetHisto(TH2* hh)
{

   if (!hh) {
      KVString Listo   = ListOfHistogramInMemory();
      TString Select  = PreselectHistogram(Listo);

      TString Choices;
      TString Default;
      Choices = "Dummy ";
      if (TheHisto) {
         Default = "Current";
         Choices += "Current ";
      }
      else Default = "Dummy";

      if (Listo == "") {
      }
      else if (Select == "") {
         Choices += Listo;
      }
      else {
         Default = Select;
         Choices += Select.Data();
         Choices += " ";
         //Choices += Listo.ReplaceAll(Select.Data(), "");
         Listo.Begin(" ");//in case list contains a histo with a similar name to Select
         // if Select="SI_CSI_0801" and the list contains "SI_CSI_0801_GG"
         while (!Listo.End()) {
            KVString s = Listo.Next();
            if (s != Select) Choices += Form("%s ", s.Data());
         }
      }

      TString Answer;
      Bool_t okpressed;

      if (Choices.Contains(" ")) {
         new KVDropDownDialog(gClient->GetDefaultRoot(), "Choose a histogram :", Choices.Data(), Default.Data(), &Answer, &okpressed);
         if (!okpressed) {
            Answer = "Current";
            return;
         }
      }
      else Answer = Default;

      if (!Answer.Contains("Current") && ownhisto) {
         delete TheHisto;
         TheHisto = 0;
         ownhisto = false;
      }

      if ((!Answer.Contains("Current")) && (!Answer.Contains("Dummy"))) {
         TheHistoChoice = 0;
         if ((TheHistoChoice = (TH2*)gROOT->FindObject(Answer.Data()))) TheHisto = TheHistoChoice;
         else if ((TheHistoChoice = (TH2*)gFile->Get(Answer.Data()))) TheHisto = TheHistoChoice;
         else if ((TheHistoChoice = (TH2*)gFile->FindObjectAnyFile(Answer.Data()))) TheHisto = TheHistoChoice;
         else if ((TheHistoChoice = (TH2*)gFile->FindObjectAny(Answer.Data()))) TheHisto = TheHistoChoice;
         else if (gTreeAnalyzer && (TheHistoChoice = (TH2*)gTreeAnalyzer->GetHistogram(Answer.Data()))) TheHisto = TheHistoChoice;
         else if ((TheHistoChoice = FindInCanvases(Answer.Data()))) TheHisto = TheHistoChoice;
         else Answer = "Dummy";
      }

      if (Answer.Contains("Dummy")) {
         TString hname = Form("%sDefaultHistogram", GetName());
         Double_t Xmax = 4096.;
         Double_t Ymax = 4096;
         if (TheGrid) {
            TheGrid->Initialize();
            // pour pouvoir utiliser un pointeur KVIDGraph* au lieu de KVIDZAGrid*
            //           Xmax = TheGrid->GetZmaxLine()->GetXaxis()->GetXmax();
            //             Ymax = TheGrid->GetZmaxLine()->GetYaxis()->GetXmax();
            TheGrid->FindAxisLimits();
            Xmax = TheGrid->GetXmax();
            Ymax = TheGrid->GetYmax();
         }
         TH2* TmpH = 0;
         if ((TmpH = (TH2*)gROOT->FindObject(hname.Data()))) delete TmpH;
         TheHisto = new TH2F(hname.Data(), hname.Data(), 2048, 0, Xmax, 2048, 0, Ymax);
         ownhisto = true;
      }
   }
   else if (!hh->InheritsFrom("TH2")) {
      cout << "ERROR: KVIDGridEditor::SetHisto(): '" << hh->GetName() << "' must be a 2D histogram !" << endl;
      return;
   }
   else {
      if ((ownhisto) && (TheHisto)) {
         delete TheHisto;
         TheHisto = 0;
      }
      TheHisto = hh;
      ownhisto = false;
   }

   if (!IsClosed() && (TheHisto)) {
      fPad = fCanvas->cd();//au cas ou il y a plusieurs canvas ouverts
      TheHisto->Draw("col");
      fPad->SetLogz(true);
      TheHisto->SetMinimum(1);
   }
   DrawAtt(true);
   return;

}

//________________________________________________________________
TH2* KVIDGridEditor::FindInCanvases(const Char_t* histoname)
{

   TIter nextc(gROOT->GetListOfCanvases());
   TCanvas* cc = 0;
   TObject* obj = 0;
   while ((cc = (TCanvas*)nextc())) {
      if (strcmp(cc->GetName(), "gIDGridEditorCanvas")) {
         if ((obj = cc->FindObject(histoname))) {
            return (TH2*)obj;
         }
      }
   }
   return 0;

}

//________________________________________________________________
void KVIDGridEditor::DrawAtt(Bool_t piv)
{
   if ((!ready) || IsClosed()) return;

   lplabel->Execute("Draw", "");
   lplabel2->Execute("Draw", "");
   lplabel3->Execute("Draw", "");
   lplabel4->Execute("Draw", "");
   lplabel5->Execute("Draw", "");

   if (!piv) SetPivot(0., 0.);
   else fPivot->Draw("P");

   if (!piv) SetPiedestal(0., 0.);
   else fPivot->Draw("P");


   UpdateViewer();
   return;
}

//________________________________________________________________
void KVIDGridEditor::SetGrid(KVIDGraph* gg, Bool_t histo)
{
   if (!gg) {
      cout << "ERROR: KVIDGridEditor::SetHisto(): invalid pointer on the grid !" << endl;
      return;
   }
   if ((TheGrid && TheGrid->TestBit(TObject::kNotDeleted)) && (!IsClosed())) TheGrid->UnDraw();

   Clear();

   TheGrid = gg;
   if (histo) SetHisto(0);
   if (!IsClosed()) TheGrid->Draw();

   fSpiderFactor = -1.;
   fSpiderZp     = -1;

   //  DrawAtt(true);

   UpdateViewer();

   ResetScalingRecap();

   return;
}

//________________________________________________________________
void KVIDGridEditor::SetGrid(TString GridName)
{
   Bool_t sethisto = true;

   if (!strcmp(GridName.Data(), "")) {
      TString Answer;
      Bool_t proposename = false;
      if (TheGrid) Answer = TheGrid->GetName();
      else if (TheHisto) {
         Answer = TheHisto->GetName();
         proposename = true;
      }
      Bool_t okpressed;
      new KVInputDialog(gClient->GetDefaultRoot(), "Enter the name of your grid :", &Answer, &okpressed);
      if (!okpressed) return;
      GridName = Answer.Data();
      if (proposename && (!strcmp(TheHisto->GetName(), Answer.Data()))) sethisto = false;
   }

   KVIDGraph* tempgrid = 0;
   if (!gIDGridManager) return;
   if (!(tempgrid = (KVIDGraph*)gIDGridManager->GetGrids()->FindObject(GridName.Data()))) {
      cout << "WARNING: KVIDGridEditor::SetGrid(): Unknown grid named '" << GridName.Data() << "' !" << endl;
      return;
   }
   else SetGrid(tempgrid, sethisto);
   return;
}

//________________________________________________________________
void KVIDGridEditor::SetPivot(Double_t xx0, Double_t yy0)
{

   if (!fPivot) {
      fPivot = new TGraph;
      fPivot->SetMarkerStyle(2);
      fPivot->SetMarkerSize(2);
      fPivot->SetMarkerColor(kRed);
      fPivot->SetName("ThePivot");
   }
   else fPivot->SetPoint(0, xx0, yy0);
}

//________________________________________________________________
void KVIDGridEditor::SetPiedestal(Double_t ppdx, Double_t ppdy)
{
   // piedestal used during SpiderIdentification
   if (!fPivot) {
      //    fPiedestal = new TGraph;
      fPivot->SetMarkerStyle(2);
      fPivot->SetMarkerSize(2);
      fPivot->SetMarkerColor(kBlack);
      fPivot->SetName("ThePiedestal");
   }
   else fPivot->SetPoint(1, ppdx, ppdy);
}

//________________________________________________________________
void KVIDGridEditor::SelectLabel()
{
   Int_t event = fPad->GetEvent();
   if (event == kMouseMotion) return;
   TObject* select = fPad->GetSelected();

   if (!select) return;
   if (!select->InheritsFrom("TPaveLabel")) return;

   if (fSVGMode && ((event == kButton1Down) || (event == kButton1Double) || (event == kButton1Shift))) {
      TString tmpStr;
      if (event == kButton1Down)        tmpStr = "click";
      else if (event == kButton1Double) tmpStr = "double click";
      else if (event == kButton1Shift)  tmpStr = "shift click";

      Int_t px = fPad->AbsPixeltoX(fPad->GetEventX());
      Int_t py = fPad->AbsPixeltoY(fPad->GetEventY());

      TLatex* mouse = new TLatex(px, py, tmpStr.Data());

      if (mouse->GetX() >= 0.5) mouse->SetTextAlign(32);
      else       mouse->SetTextAlign(12);

      mouse->Draw();
      UpdateViewer();
      delete mouse;
      mouse = 0;
   }

   TPaveLabel* label = (TPaveLabel*)select;
   if (event == kButton1Down) {
      Int_t color = label->GetFillColor();
      if (lplabel->Contains(label)) {
         lplabel->Execute("SetFillColor", fBlackMode ? "kBlack" : "kWhite");
         if (color == kWhite || color == kBlack) label->SetFillColor(kRed);
         else if (color == kRed) label->SetFillColor(fBlackMode ? kBlack : kWhite);
         UpdateViewer();
      }
      else if (lplabel2->Contains(label)) {
         label->SetFillColor(kRed);
         UpdateViewer();
         DispatchOrder(label);
      }
      else if (lplabel4->Contains(label)) {
         DispatchOrder(label);
      }
      else if (lplabel3->Contains(label)) {
         lplabel3->Execute("SetFillColor", fBlackMode ? "kBlack" : "kWhite");
         if (color == kWhite || color == kBlack) label->SetFillColor(kGreen);
         if (color == kGreen)  label->SetFillColor(fBlackMode ? kBlack : kWhite);
         //      SelectLines(label);
         SelectLines("Select");
         UpdateViewer();
      }
      else if (lplabel5->Contains(label) && (label != modulator)) {
         label->SetFillColor(kGreen);
         ChangeStep(label->GetTitle());
         UpdateViewer();
      }
   }
   else if (event == kButton1Up) {
      if (lplabel2->Contains(label)) {
         label->SetFillColor(fBlackMode ? kBlack : kWhite);
         UpdateViewer();
      }
      else if (lplabel5->Contains(label) && (label != modulator)) {
         label->SetFillColor(fBlackMode ? kBlack : kWhite);
         UpdateViewer();
      }
   }
   else if (event == kButton1Double) {
      if (lplabel5->Contains(label) && (label != modulator)) {
         label->SetFillColor(kGreen);
         ChangeStep(label->GetTitle(), 9);
         UpdateViewer();
      }
   }
   else if (event == kButton1Shift) {
      if (lplabel5->Contains(label) && (label != modulator)) {
         label->SetFillColor(kGreen);
         ChangeStep(label->GetTitle(), 100);
         UpdateViewer();
      }
      else if (lplabel3->Contains(label)) {
         lplabel3->Execute("SetFillColor", "kGreen");
         //      if(color==kWhite) label->SetFillColor(kGreen);
         //      if(color==kGreen)  label->SetFillColor(kWhite);
         //      SelectLines(label);
         SelectLines("All");
         UpdateViewer();
      }
   }
}

//________________________________________________________________
void KVIDGridEditor::MakeTransformation()
{
   Int_t event = fPad->GetEvent();
   TObject* select = fPad->GetSelected();

   if (fSVGMode && ((event == kButton1Down) || (event == kButton1Double) || (event == kButton1Shift) || (event == kWheelUp) || (event == kWheelDown))) {
      TString tmpStr;
      if (event == kButton1Down)        tmpStr = "click";
      else if (event == kButton1Double) tmpStr = "double click";
      else if (event == kButton1Shift)  tmpStr = "shift click";
      else if (event == kWheelUp)       tmpStr = "wheel up";
      else if (event == kWheelDown)     tmpStr = "wheel down";

      Int_t px = fPad->AbsPixeltoX(fPad->GetEventX());
      Int_t py = fPad->AbsPixeltoY(fPad->GetEventY());

      TLatex* mouse = new TLatex(px, py, tmpStr.Data());

      if (mouse->GetX() >= 0.5) mouse->SetTextAlign(32);
      else       mouse->SetTextAlign(12);

      mouse->Draw();
      UpdateViewer();
      delete mouse;
      mouse = 0;
      UpdateViewer();
   }

   if (((event == kMouseMotion) || (event == kButton1Motion)) && (TheHisto)) {
      if (!(aoemode)) return;

      Double_t size = 0.4 - 0.35 * venermode;

      Int_t dX = 0;
      Int_t dY = 0;

      Int_t px = fPad->GetEventX();
      Int_t py = fPad->GetEventY();

      Double_t ppx = fPad->AbsPixeltoX(px);
      Double_t ppy = fPad->AbsPixeltoY(py);

      TAxis* ax = TheHisto->GetXaxis();
      Int_t X0 = ax->GetFirst();
      Int_t X1 = ax->GetLast();
      Int_t NbinsX = ax->GetNbins();
      px = ax->FindBin(ppx);

      Double_t ddX   = (X1 + X0) * 0.5 - px;
      Double_t distX = TMath::Abs(ddX) / (X1 - X0);
      if (distX >= 0.5) return;

      TAxis* ay = TheHisto->GetYaxis();
      Int_t Y0 = ay->GetFirst();
      Int_t Y1 = ay->GetLast();
      Int_t NbinsY = ay->GetNbins();
      py = ay->FindBin(ppy);

      Double_t ddY   = (Y1 + Y0) * 0.5 - py;
      Double_t distY = TMath::Abs(ddY) / (Y1 - Y0);
      if (distY >= 0.5) return;

      if ((distX <= size) && (distY <= size)) return;

      dX = TMath::Nint(ddX * (0.05 + 0.05 * venermode));
      dY = TMath::Nint(ddY * (0.05 + 0.05 * venermode));

      if (TMath::Abs(dX) < 1) dX = TMath::Sign(1., ddX);
      if (TMath::Abs(dY) < 1) dY = TMath::Sign(1., ddY);

      Bool_t up = false;

      if ((X0 - dX > 0) && (X1 - dX < NbinsX)) {
         ax->SetRange(X0 - dX, X1 - dX);
         up = true;
      }
      if ((Y0 - dY > 0) && (Y1 - dY < NbinsY)) {
         ay->SetRange(Y0 - dY, Y1 - dY);
         up = true;
      }

      if (up) UpdateViewer();
   }

   if (!select) {}
   else {
      if (select->InheritsFrom("TPaveLabel")) return;
   }

   if ((event == kButton1Up) && (dlmode) && (select)) {
//      Info("MakeTransformation","coucou 1 %s %s",select->GetName(),select->ClassName());
      if (select->InheritsFrom("KVIDCutLine") || select->InheritsFrom("KVIDCutContour")) {
         DeleteCut((KVIDentifier*)select);
//         Info("MakeTransformation","coucou %s",select->GetName());
      }
      else if (select->InheritsFrom("KVIDQAMarker")) select->Delete();
      else if (select->InheritsFrom("KVIDentifier")) {
         DeleteLine((KVIDentifier*)select);
      }
   }
   if ((event == kButton1Up) && (select) && (!dlmode)) {
      if (select->InheritsFrom("KVIDentifier")) {
         KVIDentifier* line = (KVIDentifier*)select;
         if (selectmode) {
            if (!ListOfLines->Contains(line)) {
               line->SetLineColor(SelectedColor);
               ListOfLines->AddLast(line);
               UpdateViewer();
            }
            else {
               ListOfLines->Remove(line);
               ResetColor(line);
               UpdateViewer();
            }
         }
         else if (ListOfLines->Contains(line)) {
            ListOfLines->Remove(line);
            ResetColor(line);
            //   TPaveLabel* tmplabel = (TPaveLabel*)lplabel3->FindObject("All");
            //   tmplabel->SetFillColor(kWhite);
            //   TPaveLabel* tmplabel = (TPaveLabel*)lplabel3->At(0);
            //   tmplabel->SetFillColor(kGreen);
            //   SelectLines(tmplabel);
            SelectLines("Select");
            UpdateViewer();
         }
      }
   }

   if ((event == kButton1Shift) && (select) && (!dlmode)) {
      if (!select->InheritsFrom("KVIDZALine") || ListOfLines->IsEmpty());
      else {
         KVIDZALine* line = (KVIDZALine*)select;
         if (ListOfLines->Contains(select)) return;
         Int_t LastZ = ((KVIDZALine*)ListOfLines->At(ListOfLines->GetSize() - 1))->GetZ();
         Int_t SeleZ = line->GetZ();
         Int_t LastA = ((KVIDZALine*)ListOfLines->At(ListOfLines->GetSize() - 1))->GetA();
         Int_t SeleA = line->GetA();
         Int_t zmin, zmax, amin, amax;
         if (LastZ == SeleZ) {
            zmin = LastZ;
            zmax = LastZ;
            amin = TMath::Min(LastA, SeleA);
            amax = TMath::Max(LastA, SeleA);
         }
         else if (LastZ < SeleZ) {
            zmin = LastZ;
            amin = LastA;
            zmax = SeleZ;
            amax = SeleA;
         }
         else {
            zmax = LastZ;
            amax = LastA;
            zmin = SeleZ;
            amin = SeleA;
         }


         for (int Z = zmin; Z <= zmax; Z++) {
            KVList* tmpl = (KVList*)TheGrid->GetIdentifiers()->GetSubListWithMethod(Form("%d", Z), "GetZ");
            TIter it(tmpl);
            line = 0;
            while ((line = (KVIDZALine*)it())) {
               if (ListOfLines->Contains(line)) continue;
               if (zmax == zmin) {
                  if ((line->GetA() > amin) && (line->GetA() < amax)) {
                     line->SetLineColor(SelectedColor);
                     ListOfLines->AddLast(line);
                  }
                  continue;
               }
               if ((line->GetZ() == zmin) && (line->GetA() > amin)) {
                  line->SetLineColor(SelectedColor);
                  ListOfLines->AddLast(line);
                  continue;
               }
               if ((line->GetZ() == zmax) && (line->GetA() < amax)) {
                  line->SetLineColor(SelectedColor);
                  ListOfLines->AddLast(line);
                  continue;
               }
               if ((line->GetZ() != zmax) && (line->GetZ() != zmin)) {
                  line->SetLineColor(SelectedColor);
                  ListOfLines->AddLast(line);
                  continue;
               }
            }
            delete tmpl;
         }
      }
   }
   if ((event == kButton1Double)) {
      if (drawmode) drawmode = false;
      else if (!select->InheritsFrom("KVIDentifier")) {
         Int_t xx = fPad->GetEventX();
         Int_t yy = fPad->GetEventY();

         x0 = fPad->AbsPixeltoX(xx);
         y0 = fPad->AbsPixeltoY(yy);

         SetPivot(x0, y0);
         fPivot->Draw("P");
         UpdateViewer();
      }
   }
   if ((event == kButton1Shift) && (!drawmode)) {
      if (!select->InheritsFrom("KVIDentifier")) {
         Int_t xx = fPad->GetEventX();
         Int_t yy = fPad->GetEventY();

         x0 = fPad->AbsPixeltoX(xx);
         y0 = fPad->AbsPixeltoY(yy);

         SetPiedestal(x0, y0);
         fPivot->Draw("P");
         UpdateViewer();
      }
   }
   if ((event == kWheelUp) || (event == kWheelDown)) {
      Int_t sign = (event == kWheelUp ? 1 : -1);
      const char* who = WhoIsSelected();

      if (ListOfLines->IsEmpty())     DynamicZoom(sign, fPad->GetEventX(), fPad->GetEventY());
      else if (!strcmp(who, ""))       DynamicZoom(sign, fPad->GetEventX(), fPad->GetEventY());
      else if (!strcmp(who, "T_{X}"))  TranslateX(sign);
      else if (!strcmp(who, "T_{Y}"))  TranslateY(sign);
      else if (!strcmp(who, "R_{Z}"))  RotateZ(sign);
      else if (!strcmp(who, "S_{X}"))  ScaleX(sign);
      else if (!strcmp(who, "S_{Y}"))  ScaleY(sign);
      else if (!strcmp(who, "S_{XY}")) ScaleXY(sign);
      else if (!strcmp(who, "S_{C}"))  ScaleCurvature(sign);
   }

   //if(event==kButton2Up) ForceUpdate();
   if ((event == kESC) && (TheHisto)) Unzoom();

   return;
}

//________________________________________________________________
void KVIDGridEditor::ZoomSelected()
{
   if (!TheHisto) return;
   TAxis* ax = TheHisto->GetXaxis();

   Double_t ratio1 = (xmin - fPad->GetUxmin()) / (fPad->GetUxmax() - fPad->GetUxmin());
   Double_t ratio2 = (xmax - fPad->GetUxmin()) / (fPad->GetUxmax() - fPad->GetUxmin());

   if ((ratio2 - ratio1 > 0.05)) {
      ax->SetRangeUser(xmin, xmax);
   }

   ax = TheHisto->GetYaxis();

   ratio1 = (ymin - fPad->GetUymin()) / (fPad->GetUymax() - fPad->GetUymin());
   ratio2 = (ymax - fPad->GetUymin()) / (fPad->GetUymax() - fPad->GetUymin());

   if ((ratio2 - ratio1 > 0.05)) {
      ax->SetRangeUser(ymin, ymax);
   }

   xmax = xmin = ymax = ymin = 0.;
   return;
}

//________________________________________________________________
void KVIDGridEditor::DispatchOrder(TPaveLabel* label)
{
   TString commande(label->GetName());

   if (commande.Contains("#Leftarrow"))  Undo();
   else if (commande.Contains("0"))      SetPivot(0, 0);
   else if (commande.Contains("#odot"))  Unzoom();
   else if (commande.Contains("Lz"))     SetLogz();
   else if (commande.Contains("Ly"))     SetLogy();
   else if (commande.Contains("Lx"))     SetLogx();
   else if (commande.Contains("Cut"))    NewCut();
   else if (commande.Contains("Line"))   NewLine();
   else if (commande.Contains("Edit"))   SetEditable(label);
   else if (commande.Contains("Fit")) {
      label->SetFillColor(kRed);
      UpdateViewer();
      FitGrid();
      label->SetFillColor(fBlackMode ? kBlack : kWhite);
      UpdateViewer();
   }
   else if (commande.Contains("Test")) {
      label->SetFillColor(kRed);
      UpdateViewer();
      TestGrid();
      label->SetFillColor(fBlackMode ? kBlack : kWhite);
      UpdateViewer();
   }
   else if (commande.Contains("Mass")) {
      label->SetFillColor(kRed);
      UpdateViewer();
      FindZALines();
      label->SetFillColor(fBlackMode ? kBlack : kWhite);
      UpdateViewer();
   }
   else if (commande.Contains("Spider")) {
      label->SetFillColor(kRed);
      UpdateViewer();
      SpiderIdentification();
      label->SetFillColor(fBlackMode ? kBlack : kWhite);
      UpdateViewer();
   }
   else if (commande.Contains("More")) {
      label->SetFillColor(kRed);
      UpdateViewer();
      SuggestMoreAction();
      label->SetFillColor(fBlackMode ? kBlack : kWhite);
      UpdateViewer();
   }
   else if (commande.Contains("Delete")) {
      if (!TheGrid) return;
      Int_t color = label->GetFillColor();
      if (color == kRed) {
         label->SetFillColor(fBlackMode ? kBlack : kWhite);
         dlmode = false;
         UpdateViewer();
      }
      else if (color == kWhite || color == kBlack) {
         label->SetFillColor(kRed);
         dlmode = true;
         UpdateViewer();
      }
   }
   else cout << "WARNING: KVIDGridEditor::DispatchOrder(): unknown order '" << commande << "' !" << endl;
}

//________________________________________________________________
void KVIDGridEditor::SetEditable(TPaveLabel* label)
{
   if (TheGrid) {
      Bool_t iseditable = TheGrid->IsEditable();
      TheGrid->SetEditable(!iseditable);
      if (iseditable) label->SetFillColor(fBlackMode ? kBlack : kWhite);
      else label->SetFillColor(kRed);
   }
   else label->SetFillColor(fBlackMode ? kBlack : kWhite);
   UpdateViewer();
   return;
}

//________________________________________________________________
//void KVIDGridEditor::SelectLines(TPaveLabel* label)
void KVIDGridEditor::SelectLines(const Char_t* label)
{
   if (!TheGrid) return;
   //  TString title(label->GetTitle());
   TString title(label);
   Int_t color = ((TPaveLabel*)lplabel3->At(0))->GetFillColor();

   if (title.Contains("All")) {
      //     if(color==kWhite)
      //       {
      //       ResetColor(ListOfLines);
      //       ListOfLines->Clear();
      //       }
      //     else if(color==kGreen)
      //       {
      ListOfLines->AddAll(TheGrid->GetIdentifiers());
      ListOfLines->AddAll(TheGrid->GetCuts());
      ListOfLines->R__FOR_EACH(KVIDentifier, SetLineColor)(SelectedColor);
      ListOfLines->R__FOR_EACH(KVIDentifier, SetMarkerColor)(SelectedColor);
      //      }
      selectmode = false;
   }
   if (title.Contains("Select")) {
      if (color == kWhite || color == kBlack) {
         selectmode = false;
         ResetColor(ListOfLines);
         ListOfLines->Clear();
      }
      if (color == kGreen) {
         ListOfLines->R__FOR_EACH(KVIDentifier, SetLineColor)(SelectedColor);
         selectmode = true;
      }
   }
   UpdateViewer();
   return;
}

//________________________________________________________________
void KVIDGridEditor::NewLine()
{
   if (!TheGrid) return;
   TPaveLabel* label = (TPaveLabel*)lplabel4->FindObject("Line");
   label->SetFillColor(kRed);
   UpdateViewer();

   drawmode = true;

   TString resname;
   resname.Form("%s.IDClass", TheGrid->ClassName());
   TString cut_choices = gEnv->GetValue(resname.Data(), "");
   resname.Form("%s.DefaultIDClass", TheGrid->ClassName());
   TString cut_default = gEnv->GetValue(resname.Data(), "");
   TString cut_class;
   Bool_t okpressed;

   if (cut_choices.Contains(" ")) {
      new KVDropDownDialog(gClient->GetDefaultRoot(),
                           "Choose class of new identifier :",
                           cut_choices.Data(),
                           cut_default.Data(),
                           &cut_class,
                           &okpressed);
      if (!okpressed) return;
   }
   else cut_class = cut_choices;

   TheGrid->DrawAndAdd("ID", cut_class.Data());

   label->SetFillColor(fBlackMode ? kBlack : kWhite);
   if (fBlackMode) ResetColor(TheGrid->GetIdentifiers());

   UpdateViewer();
   if (fDebug) cout << "INFO: KVIDGridEditor::NewLine(): New Line has been added to the current grid..." << endl;
   return;
}

//________________________________________________________________
void KVIDGridEditor::NewCut()

{
   if (!TheGrid) return;
   TPaveLabel* label = (TPaveLabel*)lplabel4->FindObject("Cut");
   label->SetFillColor(kRed);
   UpdateViewer();

   drawmode = true;

   TString resname;
   resname.Form("%s.CutClass", TheGrid->ClassName());
   TString cut_choices = gEnv->GetValue(resname.Data(), "");
   resname.Form("%s.DefaultCutClass", TheGrid->ClassName());
   TString cut_default = gEnv->GetValue(resname.Data(), "");
   cut_default.ReplaceAll(" ", "");
   cut_default.ReplaceAll("KVIDCut", "");
   TString cut_class;
   KVString cut_types = cut_choices;
   cut_types.ReplaceAll("KVIDCut", "");
   Bool_t okpressed;

   if (cut_choices.Contains(" ")) {
      if (!strcmp(cut_default, "")) {
         cut_types.Begin(" ");
         cut_default = cut_types.Next();
      }
      new KVDropDownDialog(gClient->GetDefaultRoot(),
                           "Choose class of new cut :",
                           cut_types.Data(),
                           cut_default.Data(),
                           &cut_class,
                           &okpressed);
      if (!okpressed) {
         label->SetFillColor(kWhite);
         UpdateViewer();
         drawmode = false;
         return;
      }
   }
   else cut_class = cut_types;

   cut_class.Prepend("KVIDCut");
   TheGrid->DrawAndAdd("CUT", cut_class.Data());

   label->SetFillColor(fBlackMode ? kBlack : kWhite);
   UpdateViewer();

   if (fDebug) cout << "INFO: KVIDGridEditor::NewCut(): New Cut has been added to the current grid..." << endl;
   return;
}

//________________________________________________________________
Double_t KVIDGridEditor::GetX0()
{
   return fPivot->GetX()[0];
}

//________________________________________________________________
Double_t KVIDGridEditor::GetY0()
{
   return fPivot->GetY()[0];

}

//________________________________________________________________
Double_t KVIDGridEditor::GetPdx()
{
   return fPivot->GetX()[1];

}

//________________________________________________________________
Double_t KVIDGridEditor::GetPdy()
{
   return fPivot->GetY()[1];

}

//________________________________________________________________
void KVIDGridEditor::SpiderIdentification()
{
   if (!TheGrid) return;
   if (!TheHisto) return;

   x0 = GetX0();
   y0 = GetY0();

   Double_t pdx = GetPdx();
   Double_t pdy = GetPdy();

   //  TheGrid->UnDraw();
   //  TheGrid->Clear();

   new KVSpIdGUI(TheGrid, TheHisto, x0, y0, pdx, pdy, fSpiderOption.Data());

   //   fSpiderFactor = Factor;
   //   fSpiderZp = Zp;
   //
   //   if(TheGrid->GetIdentifiers()->GetSize()!=0)
   //     {
   //
   //     Int_t ret_val;
   //     new TGMsgBox(gClient->GetDefaultRoot(), gClient->GetDefaultRoot(), "ID Grid Editor",
   //                   "This will delete all existing lines. Are you sure ?",
   //                   kMBIconExclamation, kMBOk | kMBCancel, &ret_val);
   //
   //     if (ret_val & kMBOk)
   //       {
   //       TheGrid->Clear();
   //       }
   //     else return;
   //     }
   //
   //   double ScaleFactorX = TheHisto->GetNbinsX()*1./(TheHisto->GetXaxis()->GetXmax());
   //   double ScaleFactorY = TheHisto->GetNbinsY()*1/(TheHisto->GetYaxis()->GetXmax());
   //
   //   Double_t factor = fSpiderFactor;
   //   if(fSpiderZp>0)
   //     {
   //     factor = TMath::Sqrt(x0*x0*(ScaleFactorX*ScaleFactorX)+y0*y0*(ScaleFactorY*ScaleFactorY))/(20.*fSpiderZp*12.);
   //     fSpiderFactor = factor;
   //     }
   //
   //   if(fDebug) cout << "DEBUG: KVIDGridEditor::SpiderIdentification(): " << fSpiderZp << " " << fSpiderFactor << endl;
   //   fSpiderZp = -1;
   //
   // //  SetPivot(0.,0.);
   //   Unzoom();
   //   UpdateViewer();
   //
   //   KVHistoManipulator hm;
   //   TF1 RtLt("RtLt",Form("x*%lf",ScaleFactorX),0,TheHisto->GetXaxis()->GetXmax());
   //   TF1 RtLty("RtLty",Form("x*%lf",ScaleFactorY),0,TheHisto->GetXaxis()->GetXmax());
   //   TH2F* hh = (TH2F*)hm.ScaleHisto(TheHisto,&RtLt,&RtLty);
   //
   //   KVSpiderIdentificator* tata = 0;
   //   tata = new KVSpiderIdentificator(hh, x0*ScaleFactorX, y0*ScaleFactorY);
   //
   //   if((tata->GetX0()>100)||(tata->GetY0()>100))
   //     {
   //     tata->SetX0(0.);
   //     tata->SetY0(0.);
   //     }
   //
   //   if(pdx>=0.) tata->SetX0(pdx*ScaleFactorX);
   //   if(pdy>=0.) tata->SetY0(pdy*ScaleFactorY);
   //
   //   tata->SetParameters(factor);
   // //  tata->UseFit(useFit);
   //   tata->ProcessIdentification();
   //
   //   if(debug) tata->Draw("DRLF");
   //
   //   TList* ll = (TList*)tata->GetListOfLines();
   //
   //   KVIDZALine* TheLine = 0;
   //   int zmax = 0;
   //
   //   KVSpiderLine* spline = 0;
   //   TIter next_line(ll);
   //   while((spline = (KVSpiderLine*)next_line()))
   //     {
   //     if((spline->GetN()>10))//&&(spline->GetX(0)<=tata->GetX0()+200.))
   //       {
   //       TF1* ff1 = 0;
   //       if(type==kSiCsI) ff1 =
   //       spline->GetFunction(tata->GetX0()*0.01,TMath::Max(hh->GetXaxis()->GetXmax()*0.95,spline->GetX(spline->GetN()-1)));
   //       if(type==kSiSi)  ff1 = spline->GetFunction(tata->GetX0()*0.01,TMath::Min(hh->GetXaxis()->GetXmax()*0.99,spline->GetX(spline->GetN()-1)*1.5));
   //       else ff1 = spline->GetFunction();
   //       if((type==kSiCsI)&&(ff1->GetParameter(1)>=3000.||(ff1->GetParameter(2)<=0.35)||(ff1->GetParameter(2)>=1.)))
   //         {
   //         Info("SpiderIdentification","Z = %d has been rejected (fit parameters)",spline->GetZ());
   //         continue;
   //         }
   //       TheLine = (KVIDZALine*)((KVIDZAGrid*)TheGrid)->NewLine("ID");
   //       TheLine->SetZ(spline->GetZ());
   //       double min,max;
   //       ff1->GetRange(min,max);
   //       double step = 20.;
   //       double stepmax = 800.;
   //       double x = 0.;
   //       for(x=min+1; x<max+0.0001*step; x+=step)
   //   {
   //         if(step<=stepmax) step*=1.3;
   //   if(ff1->Eval(x)<4000) TheLine->SetPoint(TheLine->GetN(),x,ff1->Eval(x));
   //   }
   //       if(max>x)TheLine->SetPoint(TheLine->GetN(),max,ff1->Eval(max));
   //
   //       TheGrid->Add("ID",TheLine);
   //       if(spline->GetZ()>=zmax) zmax = spline->GetZ();
   //       }
   //     else
   //       {
   //       Info("SpiderIdentification","Z = %d has been rejected (too few points)",spline->GetZ());
   //       }
   //     }
   //
   //   if(fDebug)Info("SpiderIdentification","last line generated : Z = %d.",zmax);
   //
   //   TF1 fx("fx12",Form("x/%lf",ScaleFactorX),0.,hh->GetNbinsX()*1.);
   //   TF1 fy("fy12",Form("x/%lf",ScaleFactorY),0.,hh->GetNbinsY()*1.);
   //   TheGrid->Scale(&fx,&fy);
   //
   // //  SetPivot(tata->GetX0(),tata->GetY0());
   //
   //   if(!debug) delete tata;
   //   if(!debug) delete hh;

   //  fPad->cd();
   //  TheGrid->UnDraw();
   //  TheGrid->Draw();

   //  UpdateViewer();
   //  cout << "toto " << endl;

   return;
}

//________________________________________________________________
void KVIDGridEditor::TestGrid()
{
   if (!TheGrid) return;
   if (!TheHisto) return;

   new KVTestIDGridDialog(gClient->GetDefaultRoot(), gClient->GetDefaultRoot(), 10, 10, TheGrid, TheHisto);

   if (fDebug) cout << "INFO: KVIDGridEditor::TestGrid(): testing the current grid..." << endl;
}

//________________________________________________________________
void KVIDGridEditor::FitGrid()
{
   if (!TheGrid) return;

   KVVirtualIDFitter* fitter = KVVirtualIDFitter::GetDefaultFitter();
   fitter->SetGrid(TheGrid);
   fitter->SetPad(TheGrid->GetPad());
   TMethod* m = fitter->IsA()->GetMethodAny("FitPanel");
   TContextMenu* cm = new TContextMenu("FitPanel", "Context menu for KVVirtualIDFitter::FitPanel");
   cm->Action(fitter, m);
   delete cm;

   if (fDebug) cout << "INFO: KVIDGridEditor::FitGrid(): fitting grid '" << TheGrid->GetName() << "'..." << endl;
}

//________________________________________________________________
void KVIDGridEditor::SuggestMoreAction()
{
   if (fListOfMethods.EndsWith(" ")) fListOfMethods.Remove(fListOfMethods.Sizeof() - 2);

   TString Default = fDefaultMethod.Data();
   TString Choices = fListOfMethods.Data();

   TString Answer;
   Bool_t okpressed;
   new KVDropDownDialog(gClient->GetDefaultRoot(), "Choose an action :", Choices.Data(), Default.Data(), &Answer, &okpressed);
   if (!okpressed) return;

   TMethod* m = 0;
   if (!TheGrid) return;
   else if ((m = TheGrid->IsA()->GetMethodAllAny(Answer.Data()))) {
      TContextMenu* cm = new TContextMenu(Answer.Data(), Form("Context menu for KVIDGridEditor::%s", Answer.Data()));
      cm->Action(TheGrid, m);
      delete cm;
   }
   else if ((m = IsA()->GetMethodAllAny(Answer.Data()))) {
      TContextMenu* cm = new TContextMenu(Answer.Data(), Form("Context menu for KVIDGridEditor::%s", Answer.Data()));
      cm->Action(this, m);
      delete cm;
   }
   else cout << "INFO: KVIDGridEditor::SuggestMoreAction(): '" << Answer << "' not implemented..." << endl;

}


//________________________________________________________________
void KVIDGridEditor::SelectLinesByZ(const Char_t* ListOfZ)
{
   if (!TheGrid) return;
   if (!ListOfLines) return;

   ResetColor(ListOfLines);
   ListOfLines->Clear();

   Int_t found;
   KVNumberList ZL(ListOfZ);
   ZL.Begin();
   while (!ZL.End()) {
      Int_t Z = ZL.Next();
      KVIDZALine* line = ((KVIDZAGrid*)TheGrid)->GetZLine(Z, found);
      if ((found == -1) || (!line)) continue;
      if (!ListOfLines->Contains(line)) {
         line->SetLineColor(SelectedColor);
         ListOfLines->AddLast(line);
      }
   }
   return;
}


//________________________________________________________________
void KVIDGridEditor::SaveCurrentGrid()
{
   if (!TheGrid) return;
   TString currentdir(gSystem->ExpandPathName("."));

   static TString dir(gSystem->ExpandPathName("."));
   const char* filetypes[] = {
      "ID Grid files", "*.dat",
      "All files", "*",
      0, 0
   };
   TGFileInfo fi;
   fi.fFileTypes = filetypes;
   fi.fIniDir = StrDup(dir);
   //  fi.fFilename = Form("%s.dat",TheGrid->GetName());
   new TGFileDialog(gClient->GetDefaultRoot(), gClient->GetDefaultRoot(), kFDSave, &fi);
   if (fi.fFilename) {
      //if no ".xxx" ending given, we add ".dat"
      TString filenam(fi.fFilename);
      if (filenam.Contains("toto")) filenam.ReplaceAll("toto", TheHisto->GetName());
      if (!filenam.Contains('.')) filenam += ".dat";
      TheGrid->WriteAsciiFile(filenam.Data());
   }
   dir = fi.fIniDir;
   gSystem->cd(currentdir.Data());
}

//________________________________________________________________
void KVIDGridEditor::OpenRootFile()
{
   static TString dir("$HISTOROOT");
   TString currentdir(gSystem->ExpandPathName("."));

   if (gSystem->ExpandPathName(dir)) dir = ".";

   const char* filetypes[] = {"Root files", "*.root", "All files", "*", 0, 0};
   TGFileInfo fi;
   fi.fFileTypes = filetypes;
   fi.fIniDir = StrDup(dir);
   new TGFileDialog(gClient->GetDefaultRoot(), gClient->GetDefaultRoot(), kFDOpen, &fi);
   if (fi.fFilename) {
      if (!(TFile::Open(fi.fFilename))) {
         new TGMsgBox(gClient->GetDefaultRoot(), gClient->GetDefaultRoot(), "ID Grid Editor", Form("Could not open file %s", fi.fFilename), 0, kMBOk);
      }
   }
   dir = fi.fIniDir;
   gSystem->cd(currentdir.Data());
}

//________________________________________________________________
void KVIDGridEditor::ChooseSelectedColor()
{
   TString Default = "kOrange";
   TString Choices = Default;
   Choices += " ";
   Choices += "kBlack kGreen kBlue kRed kYellow kCyan kMagenta";

   TString Answer;
   Bool_t okpressed;
   new KVDropDownDialog(gClient->GetDefaultRoot(), "Choose an action :", Choices.Data(), Default.Data(), &Answer, &okpressed);
   if (!okpressed) return;

   if (!strcmp(Answer.Data(), "kOrange")) SetSelectedColor(kOrange + 1);
   else if (!strcmp(Answer.Data(), "kGreen")) SetSelectedColor(kGreen);
   else if (!strcmp(Answer.Data(), "kBlack")) SetSelectedColor(kBlack);
   else if (!strcmp(Answer.Data(), "kBlue")) SetSelectedColor(kBlue);
   else if (!strcmp(Answer.Data(), "kRed")) SetSelectedColor(kRed);
   else if (!strcmp(Answer.Data(), "kYellow")) SetSelectedColor(kYellow);
   else if (!strcmp(Answer.Data(), "kCyan")) SetSelectedColor(kCyan);
   else if (!strcmp(Answer.Data(), "kMagenta")) SetSelectedColor(kMagenta);

   return;
}

//________________________________________________________________
void KVIDGridEditor::DeleteLine(KVIDentifier* line)
{
   if (!TheGrid) return;
   if (!line) return;
   if (!TheGrid->GetIdentifiers()->Contains(line)) return;

   if (ListOfLines->Contains(line)) ListOfLines->Remove(line);
   TheGrid->RemoveIdentifier(line);
   return;
}

//________________________________________________________________
void KVIDGridEditor::DeleteCut(KVIDentifier* cut)
{
   if (!TheGrid) return;
   if (!TheGrid->GetCuts()->Contains(cut)) return;

   if (ListOfLines->Contains(cut)) ListOfLines->Remove(cut);
   TheGrid->RemoveCut(cut);
   return;
}

//________________________________________________________________
void KVIDGridEditor::ChangeStep(const char* title, Int_t dstep)
{
   TString commande(title);
   if (commande.Contains("+")) {
      imod += dstep;
   }
   else if (commande.Contains("-")) {
      imod -= dstep;
      if (imod <= 0)imod = 1;
   }
   modulator->SetLabel(Form("%d", imod));
   UpdateViewer();
   return;
}

//________________________________________________________________
void KVIDGridEditor::SetLogz()
{
   if (fCanvas->IsLogz()) fPad->SetLogz(0);
   else fPad->SetLogz(1);
}

//________________________________________________________________
void KVIDGridEditor::SetLogy()
{
   if (fCanvas->IsLogy()) fPad->SetLogy(0);
   else fPad->SetLogy(1);
}

//________________________________________________________________
void KVIDGridEditor::SetLogx()
{
   if (fCanvas->IsLogx()) fPad->SetLogx(0);
   else fPad->SetLogx(1);
}

//________________________________________________________________
void KVIDGridEditor::Unzoom()
{
   if (TheHisto) {
      TheHisto->GetXaxis()->UnZoom();
      TheHisto->GetYaxis()->UnZoom();
   }
}

//________________________________________________________________
void KVIDGridEditor::Undo()
{
   // Revert current grid to its last saved version
   // Normally this will be the state of the grid before starting
   // the current editing session

   if (!TheGrid) return;

   TString username = gSystem->GetUserInfo()->fRealName;
   username.ReplaceAll(",", "");
   Int_t ret_val;
   new TGMsgBox(gClient->GetDefaultRoot(), gClient->GetDefaultRoot(), "ID Grid Editor",
                Form("This will undo all changes to the grid. Are you sure, %s?", username.Data()),
                kMBIconExclamation, kMBOk | kMBCancel, &ret_val);

   if (ret_val & kMBOk) {
      if (ListOfLines && ListOfLines->GetEntries()) {
         Clear();
      }
      TheGrid->RevertToLastSavedVersion();
   }
   TPaveLabel* tmplabel = (TPaveLabel*)lplabel2->FindObject("#Leftarrow");
   tmplabel->SetFillColor(kWhite);
   UpdateViewer();
}

//________________________________________________________________
const char* KVIDGridEditor::WhoIsSelected()
{
   TPaveLabel* label = 0;
   TIter nextlabel(lplabel);
   while ((label = (TPaveLabel*)nextlabel())) {
      if (label->GetFillColor() == kRed) return label->GetName();
   }
   return "";
}

//________________________________________________________________
void KVIDGridEditor::TranslateX(Int_t Sign)
{
   if (!TheGrid) return;
   if (!ListOfLines) return;
   if (ListOfLines->IsEmpty()) return;

   Double_t step   = TheHisto->GetXaxis()->GetBinWidth(1) * (imod) * 0.02;
   Double_t factor = Sign * step;

   ft->SetParameter(0, factor);
   ListOfLines->R__FOR_EACH(KVIDentifier, Scale)(ft, 0);

   UpdateViewer();
   if (fDebug) cout << "INFO: KVIDGridEditor::TranslateX(): translation on the X axis (" << (Sign > 0 ? "+" : "-") << step << ") !" << endl;
   return;
}

//________________________________________________________________
void KVIDGridEditor::TranslateY(Int_t Sign)
{
   if (!TheGrid) return;
   if (!ListOfLines) return;
   if (ListOfLines->IsEmpty()) return;

   Double_t step   = TheHisto->GetXaxis()->GetBinWidth(1) * (imod) * 0.02;
   Double_t factor = Sign * step;

   ft->SetParameter(0, factor);
   ListOfLines->R__FOR_EACH(KVIDentifier, Scale)(0, ft);

   UpdateViewer();
   if (fDebug) cout << "INFO: KVIDGridEditor::TranslateY(): translation on the Y axis (" << (Sign > 0 ? "+" : "-") << step << ") !" << endl;
   return;
}

//________________________________________________________________
void KVIDGridEditor::DynamicZoom(Int_t Sign, Int_t px, Int_t py, Double_t speed)
{
   // Zoom in or out of histogram with mouse wheel

   if (!TheHisto) return;

   Double_t percent = TMath::Abs(0.15 * speed - Sign * 0.05);

   Int_t dX = 0;
   Int_t dY = 0;

   if (px == 0) px = (TheHisto->GetXaxis()->GetFirst() + TheHisto->GetXaxis()->GetLast()) / 2;
   else      px = fPad->AbsPixeltoX(px);

   if (py == 0) py = (TheHisto->GetYaxis()->GetFirst() + TheHisto->GetYaxis()->GetLast()) / 2;
   else py = fPad->AbsPixeltoY(py);

   TAxis* ax = TheHisto->GetXaxis();
   Int_t NbinsX = ax->GetNbins();
   Int_t X0 = ax->GetFirst();
   Int_t X1 = ax->GetLast();
   Int_t step = TMath::Min(TMath::Max(1, (Int_t)(percent * (X1 - X0))), NbinsX / 2);
   step *= Sign;
   X0 = TMath::Min(TMath::Max(X0 + step, 1), X1 - step);
   X1 = TMath::Max(TMath::Min(X1 - step, NbinsX), X0);
   if (X0 >= X1) X0 = X1 - 1;
   if (Sign > 0) dX = (Int_t)(X0 + (X1 - X0) * 0.5 - ax->FindBin(px));
   if ((X0 - dX) < 0) ax->SetRange(0, X1 - X0);
   else if ((X1 - dX) > ax->GetNbins()) ax->SetRange(ax->GetNbins() - (X1 - X0), ax->GetNbins());
   else ax->SetRange(X0 - dX, X1 - dX);

   ax = TheHisto->GetYaxis();
   Int_t NbinsY = ax->GetNbins();
   Int_t Y0 = ax->GetFirst();
   Int_t Y1 = ax->GetLast();
   step = TMath::Min(TMath::Max(1, (Int_t)(percent * (Y1 - Y0))), NbinsY / 2);
   step *= Sign;
   Y0 = TMath::Min(TMath::Max(Y0 + step, 1), Y1 - step);
   Y1 = TMath::Max(TMath::Min(Y1 - step, NbinsY), Y0);
   if (Y0 >= Y1) Y0 = Y1 - 1;
   if (Sign > 0) dY = (Int_t)(Y0 + (Y1 - Y0) * 0.5 - ax->FindBin(py));
   if ((Y0 - dY) < 0) ax->SetRange(0, Y1 - Y0);
   else if ((Y1 - dY) > ax->GetNbins()) ax->SetRange(ax->GetNbins() - (Y1 - Y0), ax->GetNbins());
   else ax->SetRange(Y0 - dY, Y1 - dY);

   UpdateViewer();
   return;
}

//________________________________________________________________
void KVIDGridEditor::RotateZ(Int_t Sign)
{
   if (!TheGrid) return;
   if (!ListOfLines) return;
   if (ListOfLines->IsEmpty()) return;

   Double_t step  = 1.*(imod / 100.);
   if (step >= 45.) step = 45.;
   Double_t theta = Sign * step * TMath::DegToRad();

   x0 = fPivot->GetX()[0];
   y0 = fPivot->GetY()[0];

   frx->SetParameters(x0, y0, theta);
   fry->SetParameters(x0, y0, theta);

   ListOfLines->R__FOR_EACH(KVIDentifier, Scale)(frx, fry);

   UpdateViewer();
   if (fDebug) cout << "INFO: KVIDGridEditor::RotateZ(): rotation around the Z axis (" << (Sign > 0 ? "+" : "-") << step << ") !" << endl;
   return;
}


//________________________________________________________________
void KVIDGridEditor::MakeScaleX(Double_t scaleFactor)
{
   if (!TheGrid) return;
   if (!ListOfLines) return;
   if (ListOfLines->IsEmpty()) return;

   x0 = fPivot->GetX()[0];

   fs->SetParameters(x0, scaleFactor);
   ListOfLines->R__FOR_EACH(KVIDentifier, Scale)(fs, 0);

   UpdateViewer();

   fAx *= scaleFactor;
   fBx  = fs->Eval(fBx);

   return;
}


//________________________________________________________________
void KVIDGridEditor::MakeScaleY(Double_t scaleFactor)
{
   if (!TheGrid) return;
   if (!ListOfLines) return;
   if (ListOfLines->IsEmpty()) return;

   y0 = fPivot->GetY()[0];

   fs->SetParameters(y0, scaleFactor);
   ListOfLines->R__FOR_EACH(KVIDentifier, Scale)(0, fs);

   UpdateViewer();

   fAy *= scaleFactor;
   fBy  = fs->Eval(fBy);

   return;
}


//________________________________________________________________
void KVIDGridEditor::ScaleX(Int_t Sign)
{
   if (!TheGrid) return;
   if (!ListOfLines) return;
   if (ListOfLines->IsEmpty()) return;

   Double_t step   = 0.05 * (imod / 100.);
   Double_t factor = 1. + Sign * step;
   MakeScaleX(factor);

   x0 = fPivot->GetX()[0];

   fs->SetParameters(x0, factor);
   ListOfLines->R__FOR_EACH(KVIDentifier, Scale)(fs, 0);

   UpdateViewer();
   if (fDebug) Info("ScaleX", "scaling on the X axis (*%f) !", factor);
   return;
}

//________________________________________________________________
void KVIDGridEditor::ScaleY(Int_t Sign)
{
   if (!TheGrid) return;
   if (!ListOfLines) return;
   if (ListOfLines->IsEmpty()) return;

   Double_t step   = 0.05 * (imod / 100.);
   Double_t factor = 1. + Sign * step;
   MakeScaleY(factor);

   y0 = fPivot->GetY()[0];

   fs->SetParameters(y0, factor);
   ListOfLines->R__FOR_EACH(KVIDentifier, Scale)(0, fs);

   UpdateViewer();
   if (fDebug) Info("ScaleY", "scaling on the Y axis (*%f) !", factor);
   return;
}

//________________________________________________________________
void KVIDGridEditor::ScaleXY(Int_t Sign)
{
   if (!TheGrid) return;
   if (!ListOfLines) return;
   if (ListOfLines->IsEmpty()) return;

   Double_t step   = 0.05 * (imod / 100.);
   Double_t factor = 1. + Sign * step;

   x0 = fPivot->GetX()[0];
   y0 = fPivot->GetY()[0];

   fs->SetParameters(x0, factor);
   fsy->SetParameters(y0, factor);

   ListOfLines->R__FOR_EACH(KVIDentifier, Scale)(fs, fsy);

   UpdateViewer();

   fAx *= factor;
   fBx  = fs->Eval(fBx);
   fAy *= factor;
   fBy  = fsy->Eval(fBy);

   if (fDebug) Info("ScaleXY", "scaling (*%f) !", factor);
   return;
}

//________________________________________________________________
void KVIDGridEditor::ScaleCurvature(Int_t Sign)
{
   if (!TheGrid) return;
   if (!ListOfLines) return;
   if (ListOfLines->IsEmpty()) return;

   Double_t step   = 0.05 * (imod / 100.);
   Double_t factor = 1. + Sign * step;

   KVIDentifier* idd = 0;
   TIter nextidd(ListOfLines);

   while ((idd = (KVIDentifier*)nextidd())) {
      Double_t x1 = idd->GetX()[0];
      Double_t x2 = idd->GetX()[idd->GetN() - 1];
      Double_t y1 = idd->GetY()[0];
      Double_t y2 = idd->GetY()[idd->GetN() - 1];

      Double_t a = (y2 - y1) / (x2 - x1);
      //    Double_t b = y1 - a*x1;
      Double_t theta = TMath::ATan(a);

      frx->SetParameters(x1, y1, -theta);
      fry->SetParameters(x1, y1, -theta);
      idd->Scale(frx, fry);

      fs->SetParameters(y1, factor);
      idd->Scale(0, fs);

      frx->SetParameters(x1, y1, theta);
      fry->SetParameters(x1, y1, theta);
      idd->Scale(frx, fry);
   }

   UpdateViewer();
   return;

}

//________________________________________________________________
void KVIDGridEditor::ResetColor(KVIDentifier* Ident)
{
   if (!TheGrid) return;
   if (!(TheGrid->GetCuts()->Contains(Ident))) {
      Ident->SetLineColor(fBlackMode ? kBlue : kBlack);
      Ident->SetMarkerColor(fBlackMode ? kRed : kBlack);
   }
   else {
      Ident->SetLineColor(kRed);
      Ident->SetMarkerColor(kRed);
   }
   return;
}

//________________________________________________________________
void KVIDGridEditor::ResetColor(KVList* IdentList)
{
   KVIDentifier* idd = 0;
   TIter nextidd(IdentList);
   while ((idd = (KVIDentifier*)nextidd())) {
      ResetColor(idd);
   }
}

//________________________________________________________________
void KVIDGridEditor::ForceUpdate()
{
   if (IsClosed()) return;
   fCanvas->cd();
   fCanvas->Clear();
   if (TheHisto) {
      TheHisto->Draw("col");
   }
   if (TheGrid) {
      TheGrid->Draw();
   }
   if (fPivot) fPivot->Draw("P");
   DrawAtt(false);

   fPad->Modified();
   fPad->Update();
   if (fDebug) cout << "INFO: KVIDGridEditor::ForceUpdate(): Canvas and Co has been updated !" << endl;
}

//________________________________________________________________
Bool_t KVIDGridEditor::HandleKey(Int_t, Int_t py)
{
   // Handle keys

   char tmp[3];
   //    UInt_t keysym;
   //    gVirtualX->LookupString(event, tmp, sizeof(tmp), keysym);
   TPaveLabel* label = 0;
   Int_t color;

   if (fSVGMode) {
      TString tmpStr(tmp);
      //        if((keysym>=4144)&&(keysym<=4155)) tmpStr.Form("f%d",keysym-4143);
      tmpStr.ToUpper();
      fKeyShow = new TPaveText(0.8, 0.8, 0.9, 0.9, "brNDC");
      fKeyShow->SetBorderSize(0);
      fKeyShow->SetFillColor(kGray + 3);
      fKeyShow->SetTextColor(0);
      fKeyShow->AddText(tmpStr.Data());
      fKeyShow->Draw();
      UpdateViewer();
   }
   //   if (event->fType == kGKeyPress) {
   //      switch ((EKeySym)keysym) {
   switch ((EKeySym)py) {
      case kKey_F1:
         label = (TPaveLabel*)lplabel->At(0);
         SelectTrans(label);
         UpdateViewer();
         break;

      case kKey_F2:
         label = (TPaveLabel*)lplabel->At(1);
         SelectTrans(label);
         UpdateViewer();
         break;

      case kKey_F3:
         label = (TPaveLabel*)lplabel->At(2);
         SelectTrans(label);
         UpdateViewer();
         break;

      case kKey_F4:
         label = (TPaveLabel*)lplabel->At(3);
         SelectTrans(label);
         UpdateViewer();
         break;

      case kKey_F5:
         label = (TPaveLabel*)lplabel->At(4);
         SelectTrans(label);
         UpdateViewer();
         break;

      case kKey_F6:
         label = (TPaveLabel*)lplabel->At(5);
         SelectTrans(label);
         UpdateViewer();
         break;

      case kKey_F7:
         label = (TPaveLabel*)lplabel->At(6);
         SelectTrans(label);
         UpdateViewer();
         break;

      case kKey_F8:
         SetLogy();
         UpdateViewer();
         break;

      case kKey_F9:
         SetLogz();
         UpdateViewer();
         break;

      case kKey_F12:
         Unzoom();
         UpdateViewer();
         break;

      case kKey_s:
         SaveCurrentGrid();
         UpdateViewer();
         break;

      case kKey_l:
         label = (TPaveLabel*)lplabel4->FindObject("Line");
         DispatchOrder(label);
         UpdateViewer();
         break;

      case kKey_u:
         UpdateViewer();
         break;

      case kKey_e:
         label = (TPaveLabel*)lplabel4->FindObject("Edit");
         SetEditable(label);
         UpdateViewer();
         break;

      case kKey_o:
         DynamicZoom(-1, 0, 0, 0.1);
//         x0 = 0.;
//         y0 = 0.;
//         SetPivot(x0, y0);
//         SetPiedestal(0.0, 0.0);
//         UpdateViewer();
         break;

      case kKey_i:
         DynamicZoom(1, 0, 0, 0.1);
         break;

      case kKey_b:
         if (fBlackMode) {
            fBlackMode = false;
            if (fJoelMode) {
               fPad->SetFillStyle(1001);
               fPad->GetFrame()->SetFillStyle(1001);
            }
            fPad->SetFillColor(kWhite);
            fPad->GetFrame()->SetFillColor(kWhite);
            gStyle->SetFrameFillColor(kWhite);
            gROOT->ForceStyle();
            gStyle->SetPalette(55);
            TheHisto->GetXaxis()->SetAxisColor(kBlack);
            TheHisto->GetYaxis()->SetAxisColor(kBlack);
            TheHisto->GetXaxis()->SetLabelColor(kBlack);
            TheHisto->GetYaxis()->SetLabelColor(kBlack);

            SelectedColor = kOrange + 1;

            lplabel->Execute("SetFillColor", "kWhite");
            lplabel->Execute("SetTextColor", "kBlack");
            lplabel->Execute("SetLineColor", "kBlack");

            lplabel2->Execute("SetFillColor", "kWhite");
            lplabel2->Execute("SetTextColor", "kBlack");
            lplabel2->Execute("SetLineColor", "kBlack");

            lplabel3->Execute("SetFillColor", "kWhite");
            lplabel3->Execute("SetTextColor", "kBlack");
            lplabel3->Execute("SetLineColor", "kBlack");

            lplabel4->Execute("SetFillColor", "kWhite");
            lplabel4->Execute("SetTextColor", "kBlack");
            lplabel4->Execute("SetLineColor", "kBlack");

            TIter it(lplabel5);
            TPaveLabel* dummy = 0;
            while ((dummy = (TPaveLabel*)it()))
               if (dummy != modulator) {
                  dummy->SetFillColor(kWhite);
                  dummy->SetTextColor(kBlack);
                  dummy->SetLineColor(kBlack);
               }

         }
         else {
            fBlackMode = true;
            if (fJoelMode) {
               fPad->SetFillStyle(3905);
               fPad->SetFillColor(kBlack);
               fPad->GetFrame()->SetFillStyle(3905);
               fPad->GetFrame()->SetFillColor(kBlack);
               gStyle->SetHatchesSpacing(1.5);
               gStyle->SetHatchesLineWidth(17);
            }
            else {
               fPad->SetFillColor(kBlack);
               fPad->GetFrame()->SetFillColor(kBlack);
               gStyle->SetFrameFillColor(kBlack);
               gROOT->ForceStyle();
               gStyle->SetPalette(56);
               TheHisto->GetXaxis()->SetAxisColor(kWhite);
               TheHisto->GetYaxis()->SetAxisColor(kWhite);
               TheHisto->GetXaxis()->SetLabelColor(kWhite);
               TheHisto->GetYaxis()->SetLabelColor(kWhite);

               SelectedColor = kMagenta;

               lplabel->Execute("SetFillColor", "kBlack");
               lplabel->Execute("SetTextColor", "kWhite");
               lplabel->Execute("SetLineColor", "kWhite");

               lplabel2->Execute("SetFillColor", "kBlack");
               lplabel2->Execute("SetTextColor", "kWhite");
               lplabel2->Execute("SetLineColor", "kWhite");

               lplabel3->Execute("SetFillColor", "kBlack");
               lplabel3->Execute("SetTextColor", "kWhite");
               lplabel3->Execute("SetLineColor", "kWhite");

               lplabel4->Execute("SetFillColor", "kBlack");
               lplabel4->Execute("SetTextColor", "kWhite");
               lplabel4->Execute("SetLineColor", "kWhite");

               TIter it(lplabel5);
               TPaveLabel* dummy = 0;
               while ((dummy = (TPaveLabel*)it()))
                  if (dummy != modulator) {
                     dummy->SetFillColor(kBlack);
                     dummy->SetTextColor(kWhite);
                     dummy->SetLineColor(kWhite);
                  }

            }
         }
         ResetColor(TheGrid->GetIdentifiers());
         UpdateViewer();
         break;

      case kKey_j:
         fJoelMode = !fJoelMode;
         break;

      case kKey_d:
         label = (TPaveLabel*)lplabel4->FindObject("Delete");
         DispatchOrder(label);
         UpdateViewer();
         break;

      case kKey_f:
//      ZoomOnMouser();
         fAutoZoomMode = !fAutoZoomMode;
         break;

      case kKey_a:
         lplabel3->Execute("SetFillColor", "kGreen");
         SelectLines("All");
         UpdateViewer();
         break;

      case kKey_x:
         ChangeZoomRatio(-1);
         UpdateViewer();
         break;

      case kKey_y:
         ChangeZoomRatio(1);
         UpdateViewer();
         break;

      case kKey_m:
         if (fPointStyle == 4) {
            fPointStyle = 20;
            fPointSize = 1.;
         }
         else {
            fPointStyle = 4;
            fPointSize = .8;
         }
         TheGrid->GetIdentifiers()->Execute("SetMarkerStyle", Form("%d", fPointStyle));
         TheGrid->GetCuts()->Execute("SetMarkerStyle", Form("%d", fPointStyle));
         TheGrid->GetIdentifiers()->Execute("SetMarkerSize", Form("%lf", fPointSize));
         TheGrid->GetCuts()->Execute("SetMarkerSize", Form("%lf", fPointSize));
         UpdateViewer();
         break;

      case kKey_z:
         label = (TPaveLabel*)lplabel3->FindObject("Select");
         color = label->GetFillColor();
         lplabel3->Execute("SetFillColor", fBlackMode ? "kBlack" : "kWhite");
         if (color == kWhite || color == kBlack) label->SetFillColor(kGreen);
         if (color == kGreen)  label->SetFillColor(fBlackMode ? kBlack : kWhite);
         SelectLines("Select");
         UpdateViewer();
         break;

      case kKey_w:
         aoemode = !aoemode;
         break;

      case kKey_v:
         venermode = !venermode;
         break;

      case kKey_Plus:
         if (TheHisto) {
            TheHisto->SetMinimum(TheHisto->GetMinimum() + 1);
            UpdateViewer();
         }
         break;
      case kKey_Minus:
         if (TheHisto) {
            TheHisto->SetMinimum(TMath::Max(0, (int)TheHisto->GetMinimum() - 1));
            UpdateViewer();
         }
         break;

      case kKey_c:
         SetLogz();
         UpdateViewer();
         break;

      case kKey_t:
         MoveHor(1, .2, false);
         break;

      case kKey_Left:
      case kKey_4:
         MoveHor(1, 0.1);
         break;

      case kKey_r:
         MoveHor(-1, .2, false);
         break;


      case kKey_Right:
      case kKey_6:
         MoveHor(-1, 0.1);
         break;

      case kKey_n:
         MoveVert(1, .25, false);
         break;

      case kKey_Down:
      case kKey_2:
         MoveVert(1, 0.1);
         break;

      case kKey_p:
         MoveVert(-1, .25, false);
         break;

      case kKey_Up:
      case kKey_8:
         MoveVert(-1, 0.1);
         break;

      case kKey_Space:
         break;
      default:
         return kTRUE;
   }
   //}
   if (fSVGMode && fKeyShow) {
      delete fKeyShow;
      fKeyShow = 0;
      UpdateViewer();
   }
   return kTRUE;
}

//________________________________________________________________
void KVIDGridEditor::MoveHor(Int_t sign, Double_t speed, Bool_t update)
{
   if (!TheHisto) return;

   TAxis* xAxis = TheHisto->GetXaxis();

   Int_t XX1 = xAxis->GetFirst();
   Int_t XX2 = xAxis->GetLast();

   if ((XX1 == 1) && (sign == 1)) return;
   if ((XX2 == xAxis->GetNbins() - 1) && (sign == -1)) return;

   Int_t dX = (Int_t)sign * (XX1 - XX2) * 0.25 * speed;

   xAxis->SetRange(XX1 + dX, XX2 + dX);
   if (update) UpdateViewer();
}

//________________________________________________________________
void KVIDGridEditor::ChangeZoomRatio(Int_t sign, Double_t speed)
{
   if (!TheHisto) return;

   TAxis* axis = 0;
   if (sign < 0) axis = TheHisto->GetXaxis();
   else       axis = TheHisto->GetYaxis();

   Int_t XX1 = axis->GetFirst();
   Int_t XX2 = axis->GetLast();

   Int_t dX = (Int_t) - 1 * (XX1 - XX2) * 0.1 * speed;

   axis->SetRange(XX1 + dX, XX2 - dX);
   UpdateViewer();
}


//________________________________________________________________
void KVIDGridEditor::MoveVert(Int_t sign, Double_t speed, Bool_t update)
{
   if (!TheHisto) return;

   TAxis* yAxis = TheHisto->GetYaxis();

   Int_t XX1 = yAxis->GetFirst();
   Int_t XX2 = yAxis->GetLast();

   if ((XX1 == 1) && (sign == 1)) return;
   if ((XX2 == yAxis->GetNbins() - 1) && (sign == -1)) return;

   Int_t dX = (Int_t)sign * (XX1 - XX2) * 0.25 * speed;

   yAxis->SetRange(XX1 + dX, XX2 + dX);
   if (update) UpdateViewer();
}

void KVIDGridEditor::ZoomOnMouse()
{

}


//________________________________________________________________
void KVIDGridEditor::SelectTrans(TPaveLabel* label)
{
   if (!label) return;

   Int_t color = label->GetFillColor();
   lplabel->Execute("SetFillColor", fBlackMode ? "kBlack" : "kWhite");
   if (color == kWhite || color == kBlack) label->SetFillColor(kRed);
   else if (color == kRed) label->SetFillColor(fBlackMode ? kBlack : kWhite);

   return;
}

void KVIDGridEditor::SetPointStyle(int pstyle)
{
   fPointStyle = pstyle;
   TheGrid->GetIdentifiers()->Execute("SetMarkerStyle", Form("%d", fPointStyle));
   TheGrid->GetCuts()->Execute("SetMarkerStyle", Form("%d", fPointStyle));
   fPad->Modified();
   fPad->Update();
}

void KVIDGridEditor::SetPointSize(double psize)
{
   fPointSize = psize;
   TheGrid->GetIdentifiers()->Execute("SetMarkerSize", Form("%lf", fPointSize));
   TheGrid->GetCuts()->Execute("SetMarkerSize", Form("%lf", fPointSize));
   fPad->Modified();
   fPad->Update();
}

void KVIDGridEditor::FindZALines()
{
   if ((!TheHisto) || (!TheGrid)) return;

   if (TheGrid->InheritsFrom("KVIDZAFromZGrid")) new KVItvFinderDialog((KVIDZAFromZGrid*)TheGrid, TheHisto);
   else                                         new KVZAFinderDialog(TheGrid, TheHisto);
   //   KVZALineFinder toto((KVIDZAGrid*)TheGrid, TheHisto);
   //   toto.SetAList(A);
   //   toto.SetNbinsByZ(binByZ);
   //   toto.ProcessIdentification(zmin,zmax);
   //
   //   SetHisto(toto.GetHisto());
   //   SetGrid(toto.GetGrid(),kFALSE);
   //   UpdateViewer();
}

void KVIDGridEditor::ChangeMasses(const Char_t* Zl, Int_t dA)
{
   KVNumberList ZL(Zl);
   ZL.Begin();
   while (!ZL.End()) {
      Int_t Z = ZL.Next();
      KVList* ll = (KVList*) TheGrid->GetIdentifiers()->GetSubListWithMethod(Form("%d", Z), "GetZ");
      Info("ChangeMasses", "%d lines found for Z=%d", ll->GetSize(), Z);

      KVIDentifier* id = 0;
      TIter next(ll);
      while ((id = (KVIDentifier*)next())) {
         Info("ChangeMasses", "A=%d -> A=%d", id->GetA(), id->GetA() + dA);
         id->SetA(id->GetA() + dA);
      }
      delete ll;
   }
}

void KVIDGridEditor::ChangeCharges(const Char_t* Zl, Int_t dZ)
{
   KVNumberList ZL(Zl);

   IntArray ztab = ZL.GetArray();
   Int_t  n = ztab.size();
   if (!n) return;

   for (int i = n - 1; i > 0; i--) {
      Int_t Z = ztab[i];
      KVList* ll = (KVList*) TheGrid->GetIdentifiers()->GetSubListWithMethod(Form("%d", Z), "GetZ");
      Info("ChangeMasses", "%d lines found for Z=%d", ll->GetSize(), Z);

      KVIDentifier* id = 0;
      TIter next(ll);
      while ((id = (KVIDentifier*)next())) {
         Info("ChangeMasses", "Z=%d -> Z=%d", id->GetZ(), id->GetZ() + dZ);
         id->SetZ(id->GetZ() + dZ);
      }
      delete ll;

   }
}

void KVIDGridEditor::AddMethod(const char* theMethod)
{
   if (fListOfMethods.IsNull()) fDefaultMethod += theMethod;
   fListOfMethods += theMethod;
   fListOfMethods += " ";
   return;
}

void KVIDGridEditor::PrintScalingRecap()
{
   // Print a summary of X and Y scaling transformations (Sx, Sy, Sxy)
   // made since the last call of ResetScalingRecap();
   Info("PrintScalingRecap",
        "Scaling recap:\n  X --> %f X + %f\n  Y --> %f Y + %f"
        , fAx, fBx, fAy, fBy);
}
