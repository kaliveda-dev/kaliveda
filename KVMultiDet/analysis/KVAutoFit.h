//Created by KVClassFactory on Tue Feb  7 15:17:39 2012
//Author: bonnet

#ifndef __KVAUTOFIT_H
#define __KVAUTOFIT_H

#include "KVBase.h"

#include "TObject.h"
#include "TH1F.h"
#include "TF1.h"
#include "TF2.h"
#include "KVHashList.h"
#include "TCanvas.h"
#include "TString.h"
#include "KVList.h"
#include "TPaveLabel.h"


class KVAutoFit : public KVBase {

protected:
   KVHashList* lfunc;   //liste des fonctions de fit

   TF1* f1Dfit;         //fonction de fit une dimension
   TF2* f2Dfit;         //fonction de fit deux dimensions

   TH1* hfit;           //histogramme analyse
   TH1* hclone;         //clone de histogramme analyse
   TCanvas* c1;         //Canvas ou est trace l histo a analyse


   Bool_t XminSet, XmaxSet; //Indique que l intervalle du fit est defini
   Double_t Xmin, Xmax; //Borne en X du fit
   Double_t Ymin, Ymax; //Borne en Y du fit

   KVHashList* lplabel; //contient la liste des TPaveLabel constituant le menu contextuel

   KVHashList* lhisto;  //liste des histos initialise via SetHistos method
   Int_t nhisto;        //nombre d'histo traites
   /*
   TString spath,sdir,sfile;
   TString output_file;
   */
   Bool_t userdefined;
   TString koption;

   virtual void init();
   void ClearRange(void);

   Bool_t NewFunction();
   virtual Bool_t NewFunction_1D();
   virtual Bool_t NewFunction_2D();

   TF1* ReloadFunction(const Char_t*, Int_t);
   virtual TF1* ReloadFunction_1D(const Char_t*, Int_t);
   virtual TF1* ReloadFunction_2D(const Char_t*, Int_t);


   virtual Double_t f2D(Double_t* xx, Double_t* para);
   virtual Double_t f1D(Double_t* xx, Double_t* para);

   void NextHisto();
   Bool_t IsKnown(const Char_t*);


public:
   Bool_t is2D;         //histogramme analyse herite de TH1 ou TH2

   KVAutoFit();
   KVAutoFit(const KVAutoFit&) ;
   virtual ~KVAutoFit();
   void Copy(TObject&) const;

   void Clear(Option_t* opt = "");
   void Print(Option_t* opt = "") const;
   KVHashList* GetFunctions() const;

   virtual void SetHistos(KVHashList* lh, TString option = "");
   virtual void SetHisto(TH1* hh);
   void GetInterval();
   void Recommence();
   virtual void Gather();

   virtual void Save();
   virtual void Relecture(const Char_t* name);

   ClassDef(KVAutoFit, 1) //Manage SemiAutomatic Fits
};

#endif
