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

/**
\class KVIDZAGrid
\brief Identification grid with lines corresponding to different nuclear isotopes (KVIDZALine)
\ingroup Identification

Such a grid can be used either to identify simultaneously both the mass and charge of detected
particles (if lines for several isotopes of each atomic number are drawn), or solely the charge
(if only one isotope per Z is drawn, and if SetOnlyZId(kTRUE) is called).

<h3>Identification quality codes</h3>
After each identification attempt, the value returned by GetQualityCode() indicates whether the
identification was successful or not. The meaning of the different codes depends on the type
of identification.

<h4>Z & A (mass & charge) isotopic identification grid</h4>
<ul>
     <li> KVIDZAGrid::kICODE0,                   ok</li>
  <li>     KVIDZAGrid::kICODE1,                   Z ok, slight ambiguity of A, which could be larger</li>
  <li>     KVIDZAGrid::kICODE2,                   Z ok, slight ambiguity of A, which could be smaller</li>
    <li>   KVIDZAGrid::kICODE3,                   Z ok, slight ambiguity of A, which could be larger or smaller</li>
   <li>    KVIDZAGrid::kICODE4,                   point is in between two isotopes of different Z, too far from either to be considered well-identified</li>
  <li>     KVIDZAGrid::kICODE5,                   point is in between two isotopes of different Z, too far from either to be considered well-identified</li>
   <li>    KVIDZAGrid::kICODE6,                   (x,y) is below first line in grid</li>
   <li>    KVIDZAGrid::kICODE7,                   (x,y) is above last line in grid</li>
  <li>     KVIDZAGrid::kICODE8,                   no identification: (x,y) out of range covered by grid</li>
</ul>

<h4>Z-only charge identification grid</h4>
<ul>
     <li> KVIDZAGrid::kICODE0,                   ok</li>
  <li>     KVIDZAGrid::kICODE1,                   slight ambiguity of Z, which could be larger</li>
  <li>     KVIDZAGrid::kICODE2,                   slight ambiguity of Z, which could be smaller</li>
    <li>   KVIDZAGrid::kICODE3,                   slight ambiguity of Z, which could be larger or smaller</li>
   <li>    KVIDZAGrid::kICODE4,                   point is in between two lines of different Z, too far from either to be considered well-identified</li>
  <li>     KVIDZAGrid::kICODE5,                   point is in between two lines of different Z, too far from either to be considered well-identified</li>
   <li>    KVIDZAGrid::kICODE6,                   (x,y) is below first line in grid</li>
   <li>    KVIDZAGrid::kICODE7,                   (x,y) is above last line in grid</li>
  <li>     KVIDZAGrid::kICODE8,                   no identification: (x,y) out of range covered by grid</li>
</ul>

In both cases, an acceptable identification is achieved if the quality code is kICODE0, kICODE1, kICODE2, or kICODE3.<br>
Points with codes kICODE4 or kICODE5 are normally considered as "noise" (pile-up ?) and should be rejected.<br>
Points which are (vertically) out of range for this grid have code kICODE6 (point too far below) or kICODE7 (point too far above).<br>
Points with code kICODE8 are totally out of range.
 */

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
