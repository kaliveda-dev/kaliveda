//Created by KVClassFactory on Mon Jan 23 10:03:13 2017
//Author: Diego Gruyer

#ifndef __KVITVFINDERDIALOG_H
#define __KVITVFINDERDIALOG_H

#include "RQ_OBJECT.h"

#include "TLine.h"
#include "TMarker.h"

#include "TGFrame.h"
#include "TGMenu.h"
#include "TGListBox.h"
#include "TGButtonGroup.h"
#include "TGButton.h"
#include "KVIDGraph.h"
#include "RQ_OBJECT.h"
#include <TGComboBox.h>
#include <TGLabel.h>
#include <TGNumberEntry.h>
#include <TGTextEntry.h>
#include <TGProgressBar.h>
#include "KVCanvas.h"
#include "KVList.h"
#include "KVListView.h"
#include "KVIDZAFromZGrid.h"
#include "TGToolBar.h"
#include "TGButton.h"

#include <TSpectrum.h>
#include <TROOT.h>
#include <TF1.h>


/**
\class KVItvFinderDialog
\brief GUI for finding/fixing mass identification intervals
\ingroup GUI
 */

class KVItvFinderDialog {
   RQ_OBJECT("KVZAFinderDialog")

public:
   enum {
      M_SAVE,
      M_NEW,
      M_DEL,
      M_MASS,
      M_LOG,
      M_UNZOOM
   };


public:
   TGTransientFrame* fMain;
   KVCanvas* fCanvas;
   TVirtualPad* fPad;

   TGToolBar* fToolBar;
   TGToolBar* fToolBar2;
   TGButton* fTBbuttons[50];
   Int_t fNbButtons;


   KVList*     fCustoms;
   KVListView* fCustomView;
   KVListView* fCurrentView;

   KVIDZAFromZGrid* fGrid;
   TH2*        fHisto;
   TH1*        fLinearHisto;

   KVList fItvPaint;
//    KVPIDIntervalFinder* fPIDFinder;

   TSpectrum fSpectrum;
   TGraph*   fPoints;
   Int_t     fNPoints;
   TList     fFunc;
   Int_t     fNpeaks[30];

   Int_t fNextIntervalZ;

   Double_t fSig, fRat;


public:
   KVItvFinderDialog(KVIDZAFromZGrid* gg, TH2* hh);
   virtual ~KVItvFinderDialog();

   int  GetNextIntevalZ()
   {
      return fNextIntervalZ;
   }
   void SetNextIntevalZ(int zz)
   {
      fNextIntervalZ = zz;
   }

   void DisplayPIDint();
   void SelectionITVChanged();
   void UpdatePIDList();

   void ZoomOnCanvas();
   void DrawIntervals();
   void DrawInterval(interval_set* itvs, bool label = 0);

   void ClearInterval(interval_set* itvs);

   void DoIdentification() {}
   void LinearizeHisto(int nbins);

   void DoClose() {}
   void CloseWindow() {}

   void Identify();//{ProcessIdentification(1,25);}//{Info("SaveGrid","Not yet implemented");}
   void Identify(double sigma, double ratio);//{ProcessIdentification(1,25);}//{Info("SaveGrid","Not yet implemented");}
   void SaveGrid();//{fGrid->GetIntervalSets()->ls(); fGrid->GetParameters()->ls();}
   void ExportToGrid();
   void NewInterval();//{Info("NewInterval","Not yet implemented");}
   void NewIntervalSet();//{Info("NewIntervalSet","Not yet implemented");}
   void RemoveInterval();//{Info("RemoveInterval","Not yet implemented");}
   void MassesUp();//{Info("ChangeMasses","Not yet implemented");}
   void MassesDown();//{Info("ChangeMasses","Not yet implemented");}
   void UpdateLists();//{cout << "toto" << endl;}
   void TestIdent();//;{cout << "TestIdent()" << endl;}
   void SetLogy();
   void UnzoomHisto();

   void ProcessIdentification(Int_t zmin = -1, Int_t zmax = -1);
   void FindPIDIntervals(Int_t zz);
   Double_t fpeaks(Double_t* x, Double_t* par);


   ClassDef(KVItvFinderDialog, 1) //gui to KVPIDIntevalFinder
};


//class interval_painter: public TNamed{
//    RQ_OBJECT("interval_painter")

//    public:
//        TMarker* fMarker;
//    TLine* fLine1;
//    TLine* fLine2;

//    TH1* fHisto;
//    interval* fInterval;

//public:
//    interval_painter(){}
//    interval_painter(interval* itv, TH1* hh);
//    virtual ~interval_painter();

//    void Draw(Option_t *    option = "");

//    ClassDef(interval_painter,1)//gui to interval_painter
//};

#endif
