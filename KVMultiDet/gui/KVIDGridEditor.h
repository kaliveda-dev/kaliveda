//Created by KVClassFactory on Fri Feb 17 11:05:30 2012
//Author: dgruyer

#ifndef __KVIDGridEditor_H
#define __KVIDGridEditor_H

#include "TCanvas.h"
#include "TPaveLabel.h"
#include "TPaveText.h"
#include "TStyle.h"
#include "TH2F.h"
#include "TF1.h"
#include "TF2.h"
#include "TROOT.h"
#include "TView.h"
#include "TKey.h"
#include "TSpectrum.h"

#include "TMethod.h"
#include "TContextMenu.h"
#include "TGMsgBox.h"
#include "TGFileDialog.h"

#include "KVIDGridEditorCanvas.h"
#include "KVBase.h"
#include "KVHashList.h"
#include "KVIDGraph.h"
#include "KVIDGridManager.h"
#include "KVIDZALine.h"
#include "KVIDZAGrid.h"
#include "KVIDentifier.h"
#include "KVDropDownDialog.h"
#include "KVVirtualIDFitter.h"
#include "TMethod.h"
#include "TContextMenu.h"
#include "TGMsgBox.h"
#include "KVInputDialog.h"
#include "KVHistoManipulator.h"

#include "KVSpiderIdentificator.h"
#include "KVZALineFinder.h"

#define FACTOR -1.

class KVIDGridEditor : public KVBase {
//friend class KeyHandler;
   friend class KVIDGridEditorCanvas;

public:
   enum MatrixType {
      kSiCsI,
      kSiSi,
      kChIoSi,
      kPSA
   };

protected:

   Bool_t fDebug;
   Bool_t fSVGMode;
   Int_t  fSVGIndex;
   TPaveText* fKeyShow;

   KVIDGridEditorCanvas* fCanvas;
   TVirtualPad*          fPad;
   TGraph*               fPivot;
   TGraph*               fPiedestal;
   KVString              fListOfMethods;
   KVString              fDefaultMethod;

   KVHashList* lplabel;       //contient la liste des TPaveLabel pour les transformations (rouge)
   KVHashList* lplabel2;      //contient la liste des TPaveLabel pour les actions (rouge)
   KVHashList* lplabel3;      //contient la liste des TPaveLabel pour les selections de lignes (vert)
   KVHashList* lplabel4;      //contient la liste des TPaveLabel pour editer la grille (rouge)
   KVHashList* lplabel5;      //contient la liste des TPaveLabel pour moduler les pas (vert)

   TPaveLabel* modulator;

   TH2*        TheHisto;      //pointeur sur l'histogramme
   TH2*        TheHistoChoice;
   KVIDGraph*  TheGrid;       //pointeur sur la grille courante
   KVList*     ListOfLines;      //liste des lignes selectionnees

   Int_t SelectedColor;       //couleur des lignes selectionnees

   Double_t x0;            //coordonne x du pivot
   Double_t y0;            //coordonne y du pivot
   Int_t itrans, iact, iopt, imod; //utilise pour le placement des boutons

   TF1* ft;          //translation
   TF1* fs;          //scaling
   TF1* fsy;            //scaling
   TF2* frx;            //rotation
   TF2* fry;            //rotation

   Double_t fAx, fBx, fAy, fBy; // scaling recap for current grid

   Bool_t ownhisto;        //true si histo par defaut
   Bool_t dlmode;       //true si mode delete
   Bool_t drawmode;        //true si mode draw (line, cut)
   Bool_t aoemode;         //true si mode age of empire
   Bool_t selectmode;         //true si mode vener
   Bool_t venermode;       //true si mode select
   Bool_t ready;

   Double_t xmin, xmax, ymin, ymax; //utilises pour les differents zooms
   Double_t oldx, oldy;       //utilises pour les differents zooms
   Bool_t   moved;         //utilise pour les differents zooms

   Double_t fSpiderFactor;
   Int_t    fSpiderZp;
   TString  fSpiderOption;

protected:

   Double_t GetX0();
   Double_t GetY0();
   Double_t GetPdx();
   Double_t GetPdy();

   Bool_t HandleKey(Int_t px, Int_t py);
   void MoveHor(Int_t sign);
   void MoveVert(Int_t sign);

   void AddMethod(const char* theMethod);
   void SelectTrans(TPaveLabel* label);

   void SetDefault();                  //
   void init();                     //
   void AddTransformation(TString label);          //construction de l'interface
   void AddAction(TString label);                  //graphique et initialisation
   void AddGridOption(TString label, KVHashList* thelist);        //
   void ConstructModulator();             //

   void TranslateX(Int_t Sign);
   void TranslateY(Int_t Sign);
   void RotateZ(Int_t Sign);
   void ScaleX(Int_t Sign);
   void ScaleY(Int_t Sign);
   void ScaleXY(Int_t Sign);
   void ScaleCurvature(Int_t Sign);

   void DynamicZoom(Int_t Sign, Int_t px = 0, Int_t py = 0); //zoom molette
   void Unzoom();                      //unzoom histo (accessible via l'interface : I)
   void ZoomSelected();                //zoom 'gnuplot'

   void SetLogz();                     //echelle log sur Oz
   void SetLogy();                     //echelle log sur Oy
   void SetLogx();                     //echelle log sur Ox
   void Undo();                        //annule toute les operation sur la grille courante (I)

   void SetPivot(Double_t xx0, Double_t yy0);       // set pivot position (double click on the histo)
   void SetPiedestal(Double_t ppdx, Double_t ppdy); // set piedestal position (shift click on the histo)

   void SetEditable(TPaveLabel* label);
//   void SelectLines(TPaveLabel* label);
   void SelectLines(const Char_t* label);

   void NewLine();                  //'Line'
   void NewCut();                   //'Cut'
   void FitGrid();                  //'Fit' avec fonctionnelle de T.G
   void TestGrid();                 //'Test' linearisation de l'histo avec la grille courante

   void DeleteLine(KVIDentifier* line);            //'Delete'
   void DeleteCut(KVIDentifier* cut);           //'Delete'

   void UpdateViewer();                //rafraichit l'interface
   void DrawAtt(Bool_t piv);              //affiche les bouton sur le canvas
   void ResetColor(KVIDentifier* Ident);        //redonne la couleur d'origine a une ligne
   void ResetColor(KVList* IdentList);          //redonne la couleur d'origine a une liste de ligne

   void SuggestMoreAction();              //'More', modifier cette methode pour ajouter des fonctionnalites !!!
   void ChooseSelectedColor();               //'More' -> SetSelectedColor change la couleur des lignes selectionnes
   void OpenRootFile();                //pas implemente

   void DispatchOrder(TPaveLabel* label);       //methode qui distribue les ordres quand on clic sur un bouton
   void ChangeStep(const char* title, Int_t dstep = 1);
   const char* WhoIsSelected();              //methode qui regarde quelle transformation est selectionnee

   TString ListOfHistogramInMemory();           //revoie la liste des histo dans gFile
   TString PreselectHistogram(TString ListOfName, Int_t ipert = 0); //revoie le nom de l' histo qui as le mm nom que la grille (si existe)
   TH2* FindInCanvases(const Char_t* name);

public:

   KVIDGridEditor();
   KVIDGridEditor(const KVIDGridEditor&) ;
   virtual ~KVIDGridEditor();
   void Copy(TObject&) const;

   void StartViewer();                 //ouvre l'interface graphique, utilise quand double clic sur une grille dans le gIDGridManagerGUI
   void Clear(const Option_t* opt = "");        //nettoi l'interface ('all' enleve la grille et l'histo)
   void Close();                 //ferme l'interface graphique
   Bool_t IsClosed();                  //true si l'interface est fermee
   void CanvasWasClosed();     // called when GUI is used to close the canvas
   void ForceUpdate();                 //rafraichit toute l'interface de force
   void SaveCurrentGrid();             //'More' -> SaveCurrentGrid ouvre une boite de dialogue pour sauver la grille

   void SetHisto(TH2* hh);             //donne l'histo a l'editeur
   void SetGrid(KVIDGraph* gg, Bool_t histo = true);  //donne la grille a l'editeur
   void SetGrid(TString GridName);           //donne la grille a l'editeur par son nom

   KVIDGraph* GetGrid()
   {
      return TheGrid;
   }
   TH2*       GetHisto()
   {
      return TheHisto;
   }

   TCanvas* GetEditorCanvas()
   {
      return fCanvas;
   }

   void SelectLabel();                 //methode ajouter a la liste des auto-exec du canvas : gere les bouton
   void MakeTransformation();             //methode ajouter a la liste des auto-exec du canvas : gere le reste

   void SetSelectedColor(Int_t color)
   {
      SelectedColor = color;
   }
   void SelectLinesByZ(const Char_t* ListOfZ);        //'More' -> 'SelectLinesByZ'
   void SetDebug(Bool_t debug)
   {
      fDebug = debug;
   }
   void SetSVGMode()
   {
      fSVGMode = !fSVGMode;
   }
   void MakeScaleX(Double_t scaleFactor);
   void MakeScaleY(Double_t scaleFactor);

   Int_t GetSpiderZp()
   {
      return fSpiderZp;
   }
   Double_t GetSpiderFactor()
   {
      return fSpiderFactor;
   }

   void SetSpiderOption(const char* opt = "DRLF")
   {
      fSpiderOption = opt;
   }
   void SpiderIdentification();
   void FindZALines();
   void ChangeMasses(const Char_t* Zl, Int_t dA);
   void ChangeCharges(const Char_t* Zl, Int_t dZ);

   void ResetScalingRecap()
   {
      fAx = fAy = 1;
      fBx = fBy = 0;
   }
   void PrintScalingRecap();

   ClassDef(KVIDGridEditor, 1) // outil de modification de grille.
};

//................  global variable
R__EXTERN KVIDGridEditor* gIDGridEditor;

#endif
