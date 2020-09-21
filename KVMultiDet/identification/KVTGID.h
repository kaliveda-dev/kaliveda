/***************************************************************************
                          KVTGID.h  -  description
                             -------------------
    begin                : 5 July 2005
    copyright            : (C) 2005 by J.D. Frankland
    email                : frankland@ganil.fr

$Id: KVTGID.h,v 1.12 2009/03/03 14:27:15 franklan Exp $
***************************************************************************/

#ifndef KVTGID__H
#define KVTGID__H

#include "TF1.h"
#include "KVIDGrid.h"
#include "TString.h"
#include "KVNumberList.h"

/**
\class KVTGID
\brief Abstract base class for particle identfication using functionals developed by L. Tassan-Got (IPN Orsay)
\ingroup Identification

These functionals are defined in the KVTGIDFunctions
namespace, and the functional used by each KVTGID object is defined by
giving its name to the constructor. These objects are persistent, i.e. can be
retrieved from a ROOT file and used for identification.

GetIdentification() method performs an identification. Status code can be retrieved
afterwards using GetStatus();

A KVIDGrid identification grid can be generated from the functional in order
to visualise the corresponding identification lines. (MakeIDGrid)

The following methods must be defined by child-classes:
~~~~{.cpp}
      SetIdent() -- determine how to set the identification for each line in the grid
      AddLine()  -- define the type of IDLine added to the grid
      NewGrid()  -- define the type of IDGrid to generate
~~~~
*/

class KVTGID: public TF1 {

protected:

   Double_t fID_min;            //minimum ID fitted with functional
   Double_t fID_max;            //maximum ID fitted with functional
   Int_t fStatus;               //! transient member, holds status code of last call to GetIdentification
   void init();
   TString fTGIDFunctionName;   //name of KVTGIDFunctions:: namespace function used for identification

   Int_t fLambda;                //indices of parameters
   Int_t fMu;
   Int_t fG;
   Int_t fPdx;
   Int_t fPdy;
   Int_t fAlpha;
   Int_t fBeta;
   Int_t fNu;
   Int_t fXi;
   Int_t fEta;
   Int_t fType;            // type of functional (=0 standard, =1 extended)
   Int_t fLight;           // with (1) or without (0) CsI light-energy dependence
   Int_t fZorA;            // used for Z (1) or A (0) identification
   Int_t fMassFormula;     // mass formula used to calculate A from Z (if Z identification used)

   KVNumberList fRuns;        //list of runs for which fit is valid
   TString fVarX;             //quantity used for X coordinates
   TString fVarY;             //quantity used for Y coordinates
   KVString fTelescopes;        //list of telescopes for which fit is valid

   virtual void SetIdent(KVIDLine*, Double_t ID) = 0;
   virtual KVIDLine* AddLine(KVIDGrid*) = 0;

   void SetStringTelescopes(const Char_t* s)
   {
      // Set directly the contents of fTelescopes
      fTelescopes = s;
   };

public:

   Int_t GetFunctionalType() const
   {
      return fType;
   };
   Int_t GetZorA() const
   {
      return fZorA;
   };
   Int_t GetLightEnergyDependence() const
   {
      return fLight;
   };

   //status codes for GetIdentification
   enum {
      kStatus_OK,               //normal identification
      kStatus_NotBetween_IDMin_IDMax    //no solution between fID_min and fID_max
   };

   const Char_t* GetFunctionName() const
   {
      return fTGIDFunctionName.Data();
   };

   KVTGID();
   KVTGID(const KVTGID&);
   KVTGID(const Char_t* name, const Char_t* function,
          Double_t xmin, Double_t xmax, Int_t npar, Int_t par_x,
          Int_t par_y);
   virtual ~ KVTGID() {};

   virtual void Copy(TObject& tgid) const;
   void SetIDmax(Double_t x)
   {
      fID_max = x;
   };
   Double_t GetIDmax() const
   {
      return fID_max;
   };
   void SetIDmin(Double_t x)
   {
      fID_min = x;
   };
   Double_t GetIDmin() const
   {
      return fID_min;
   };

   virtual Double_t GetIdentification(Double_t ID_min, Double_t ID_max,
                                      Double_t& ID_quality,
                                      Double_t* par = 0);

   virtual void AddLineToGrid(KVIDGrid* g, Int_t ID, Int_t npoints,
                              Double_t xmin, Double_t xmax, Bool_t log_scale = kFALSE);

   virtual void Print(Option_t* option = "") const;

   Int_t Compare(const TObject* obj) const;

   inline virtual Int_t GetStatus() const
   {
      return fStatus;
   };
   virtual const Char_t* GetStatusString() const;

   Double_t GetDistanceToLine(Double_t x, Double_t y, Int_t id,
                              Double_t* params = 0);

   static KVTGID* MakeTGID(const Char_t* name, Int_t type, Int_t light, Int_t ZorA, Int_t mass);

   void SetLambda(Double_t val)
   {
      if (fLambda > -1) SetParameter(fLambda, val);
   };
   Double_t GetLambda() const
   {
      return fLambda > -1 ? GetParameter(fLambda) : 0;
   };
   void SetMu(Double_t val)
   {
      if (fMu > -1) SetParameter(fMu, val);
   };
   Double_t GetMu() const
   {
      return fMu > -1 ? GetParameter(fMu) : 0;
   };
   void SetG(Double_t val)
   {
      if (fG > -1) SetParameter(fG, val);
   };
   Double_t GetG() const
   {
      return fG > -1 ? GetParameter(fG) : 0;
   };
   void SetPdx(Double_t val)
   {
      if (fPdx > -1) SetParameter(fPdx, val);
   };
   Double_t GetPdx() const
   {
      return fPdx > -1 ? GetParameter(fPdx) : 0;
   };
   void SetPdy(Double_t val)
   {
      if (fPdy > -1) SetParameter(fPdy, val);
   };
   Double_t GetPdy() const
   {
      return fPdy > -1 ? GetParameter(fPdy) : 0;
   };
   void SetAlpha(Double_t val)
   {
      if (fAlpha > -1) SetParameter(fAlpha, val);
   };
   Double_t GetAlpha() const
   {
      return fAlpha > -1 ? GetParameter(fAlpha) : 0;
   };
   void SetBeta(Double_t val)
   {
      if (fBeta > -1) SetParameter(fBeta, val);
   };
   Double_t GetBeta() const
   {
      return fBeta > -1 ? GetParameter(fBeta) : 0;
   };
   void SetNu(Double_t val)
   {
      if (fNu > -1) SetParameter(fNu, val);
   };
   Double_t GetNu() const
   {
      return fNu > -1 ? GetParameter(fNu) : 0;
   };
   void SetXi(Double_t val)
   {
      if (fXi > -1) SetParameter(fXi, val);
   };
   Double_t GetXi() const
   {
      return fXi > -1 ? GetParameter(fXi) : 0;
   };
   void SetEta(Double_t val)
   {
      if (fEta > -1) SetParameter(fEta, val);
   };
   Double_t GetEta() const
   {
      return fEta > -1 ? GetParameter(fEta) : 0;
   };
   void SetMassformula(Int_t val)
   {
      fMassFormula = val;
   };
   Int_t GetMassFormula() const
   {
      return fMassFormula;
   };
   void SetLTGParameters(Double_t* par);
   void SetLTGParameters(Float_t* par);
   static Int_t GetNumberOfLTGParameters(Int_t type, Int_t light);
   void SetLTGParameterNames();

   void SetValidRuns(const KVNumberList& r)
   {
      fRuns = r;
   };
   const KVNumberList& GetValidRuns() const
   {
      return fRuns;
   };
   Bool_t IsValidForRun(Int_t run) const
   {
      // Returns kTRUE if 'run' is contained in list of runs for which fit is valid, fRuns.
      // If fRuns is empty, returns kTRUE for ALL runs.
      return (fRuns.IsEmpty() || fRuns.Contains(run));
   };
   void SetVarX(const Char_t* x)
   {
      fVarX = x;
   };
   const Char_t* GetVarX() const
   {
      return fVarX.Data();
   };
   void SetVarY(const Char_t* x)
   {
      fVarY = x;
   };
   const Char_t* GetVarY() const
   {
      return fVarY.Data();
   };
   void SetIDTelescopes(const TCollection*);
   void ClearIDTelescopes()
   {
      fTelescopes = "/";
   };
   void AddIDTelescope(KVBase* tel)
   {
      // Adds tel to list of ID telescopes for which this fit is valid
      fTelescopes += tel->GetName();
      fTelescopes += "/";
   };
   Bool_t IsValidForTelescope(KVBase* tel) const
   {
      // return kTRUE if fit is good for this telescope
      TString id = Form("/%s/", tel->GetName());
      return fTelescopes.Contains(id);
   };
   const KVString& GetIDTelescopes() const
   {
      return fTelescopes;
   }
   void WriteToAsciiFile(std::ofstream&) const;
   static KVTGID* ReadFromAsciiFile(const Char_t* name, std::ifstream&);

   ClassDef(KVTGID, 5)          //Abstract base class for particle identfication using functionals developed by L. Tassan-Got (IPN Orsay)
};

#endif
