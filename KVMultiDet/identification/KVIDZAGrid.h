/***************************************************************************
                          KVIDZAGrid.h  -  description
                             -------------------
    begin                : Nov 24 2004
    copyright            : (C) 2004 by J.D. Frankland
    email                : frankland@ganil.fr

$Id: KVIDZAGrid.h,v 1.14 2009/04/01 09:38:10 franklan Exp $
***************************************************************************/

#ifndef KVIDZAGrid_H
#define KVIDZAGrid_H

#include "KVIDGrid.h"
#include "TObjArray.h"

class KVIDZALine;

class KVIDZAGrid: public KVIDGrid {

protected:

   UShort_t fZMax;              //largest Z of lines in grid
   KVIDZALine*  fZMaxLine;//! line with biggest Z and A

   void SetZmax(Int_t z)
   {
      fZMax = z;
   };

   KVIDLine* fClosest;          //!closest line to last-identified point
   KVIDLine* fLsups;//!
   KVIDLine* fLsup;//!
   KVIDLine* fLinf;//!
   KVIDLine* fLinfi;//!
   Double_t fDistanceClosest;   //!distance from point to closest line
   Int_t fIdxClosest;         //!index of closest line in main list fIdentifiers
   Int_t fICode;                //!code de retour

   Int_t kinfi, kinf, ksup, ksups;//! used by IdentZA and IdentZ
   Double_t dinf, dsup, dinfi, dsups;//!
   Double_t winf, wsup, winfi, wsups;//!
   Int_t Zinfi, Zinf, Zsup, Zsups;//!
   Int_t Ainfi, Ainf, Asup, Asups;//!

   Int_t Aint;//!mass of line used to identify particle
   Int_t Zint;//!Z of line used to identify particle

   virtual Bool_t FindFourEmbracingLines(Double_t x, Double_t y, const Char_t* position);
   void init();

public:

   enum {
      kICODE0,
      kICODE1,
      kICODE2,
      kICODE3,
      kICODE4,
      kICODE5,
      kICODE6,
      kICODE7,
      kICODE8,
      kICODE9,
      kICODE10
   };

protected:
   Bool_t AcceptIDForTest()
   {
      // Used by TestIdentification.
      // The result of the identification may be excluded from the histograms of PID
      // and PID vs. Eres, depending on the quality code of the identification algorithm.
      // (given by GetQualityCode()).
      // For a general (Z,A) grid we only include particles with GetQualityCode() < 4 as being "well-identified"
      return (fICode < kICODE4);
   };

public:
   KVIDZAGrid();
   KVIDZAGrid(const KVIDZAGrid&);
   virtual ~ KVIDZAGrid();
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   virtual void Copy(TObject&) const;
#else
   virtual void Copy(TObject&);
#endif

   void RemoveLine(Int_t Z, Int_t A = -1); // *MENU*
   void RemoveZLines(const Char_t* ZList);     // *MENU*

   void SetVarXVarY(char* VarX, char* VarY)
   {
      SetVarX(VarX);
      SetVarY(VarY);
   };
   void AddParameter(char* Name, char* Value)
   {
      fPar->SetValue(Name, Value);
   };

   virtual void Initialize();// *MENU*
   void DrawLinesWithWidth();// *MENU={Hierarchy="View.../DrawLinesWithWidth"}*

   virtual void CalculateLineWidths();
   Int_t GetZmax() const
   {
      return (Int_t)fZMax;
   };
   virtual KVIDZALine* GetZLine(Int_t z, Int_t&) const;
   KVIDZALine* GetZmaxLine() const
   {
      return fZMaxLine;
   };
   virtual KVIDZALine* GetZALine(Int_t z, Int_t a, Int_t&) const;

   virtual void IdentZA(Double_t x, Double_t y, Int_t& Z, Double_t& A);
   virtual TClass* DefaultIDLineClass()
   {
      return TClass::GetClass("KVIDZALine");
   };
   virtual void IdentZ(Double_t x, Double_t y, Double_t& Z);
   Int_t GetQualityCode() const
   {
      // Return quality code for previously-attempted identification
      // Meanings of code values are given in class description
      return fICode;
   };

   virtual void Identify(Double_t x, Double_t y, KVIdentificationResult*) const;

   inline KVIDLine* GetClosestLine() const
   {
      return fClosest;
   };
   inline Double_t GetDistanceClosestLine() const
   {
      return fDistanceClosest;
   };
   inline UChar_t GetIndexClosest() const
   {
      return fIdxClosest;
   };

   //virtual void MakeEDeltaEZGrid(Int_t Zmin, Int_t Zmax, Int_t npoints=20, Double_t gamma = 2);//*MENU*

   KVIDGraph* MakeSubsetGraph(Int_t Zmin, Int_t Zmax, const Char_t* /*graph_class*/ = ""); //*MENU*
   KVIDGraph* MakeSubsetGraph(TList*, TClass* /*graph_class*/ = 0);

   ClassDef(KVIDZAGrid, 2)     //Base class for 2D Z & A identification grids
};

class KVIDZGrid : public KVIDZAGrid {
public:
   KVIDZGrid() {};
   virtual ~KVIDZGrid() {};

   ClassDef(KVIDZGrid, 3) //FOR BACKWARDS COMPATIBILITY ONLY. OBSOLETE.
};
#endif
