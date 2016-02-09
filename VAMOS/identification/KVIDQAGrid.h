//Created by KVClassFactory on Thu Jun 19 10:20:29 2014
//Author: Guilain ADEMARD

#ifndef __KVIDQAGRID_H
#define __KVIDQAGRID_H

#include "KVIDGrid.h"
#include "KVIDQALine.h"
#include "KVIdentificationResult.h"
#include "KVReconstructedNucleus.h"

class KVIDQAGrid : public KVIDGrid {
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

   Int_t fICode;   //! identification code
   Int_t fOnlyQId; //! dummy variable used by context menu dialog boxes
   Char_t* fToF; //! dummy variable used by context menu dialog boxes
   Double_t fRealQ;//! current identified real Q


   void init();
   Bool_t AcceptIDForTest()
   {
      // Used by TestIdentification.
      // The result of the identification may be excluded from the histograms of PID
      // and PID vs. Eres, depending on the quality code of the identification algorithm.
      // (given by GetQualityCode()).
      // For a general (Q,A) grid we only include particles with GetQualityCode() < 4 as being "well-identified"
      return (fICode < kICODE4);
   };

   virtual void MakeYvsAoQGrid(const Char_t* Y, Int_t Qmin, Int_t Qmax, Int_t Amin, Int_t Amax);

public:

   KVIDQAGrid();
   virtual ~KVIDQAGrid();
   void Copy(TObject& obj) const;

   virtual Int_t  GetNumberOfMasses() const;

   // Returns kTRUE if the graph is only to be used for Q identification
   // (no mass information). Default is to identify both Q & A.
   Bool_t OnlyQId() const
   {
      return fOnlyZId;
   };

   // Returns kTRUE if the graph is only to be used for Q identification
   // (no mass information). Default is to identify both Q & A
   Bool_t IsOnlyQId() const
   {
      return fOnlyZId;
   };

   void RemoveLine(Int_t Q);  // *MENU*
   void RemoveQLines(const Char_t* QList);     // *MENU*
   void SetOnlyQId(Bool_t yes = kTRUE)
   {
      SetOnlyZId(yes);
   };//  *TOGGLE={Hierarchy="Q identification only"}*
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
   virtual KVIDQALine* GetQLine(Int_t q, Int_t&) const;

   virtual TClass* DefaultIDLineClass()
   {
      return TClass::GetClass("KVIDQALine");
   };
   Int_t GetQualityCode() const
   {
      // Return quality code for previously-attempted identification
      // Meanings of code values are given in class description
      return fICode;
   };

   virtual void Identify(Double_t x, Double_t y, KVIdentificationResult*) const;

   virtual void MakeQvsAoQGrid(Int_t Qmin, Int_t Qmax, Int_t Amin, Int_t Amax);//*MENU*
   virtual void MakeAvsAoQGrid(Int_t Qmin, Int_t Qmax, Int_t Amin, Int_t Amax);//*MENU*

   KVIDGraph* MakeSubsetGraph(Int_t Qmin, Int_t Qmax, const Char_t* /*graph_class*/ = ""); //*MENU*
   KVIDGraph* MakeSubsetGraph(TList*, TClass* /*graph_class*/ = 0);



   TFile* FindAMarkers(const Char_t* name_of_data_histo, const Char_t* q_list = "");
   void UnDraw();

   const Char_t* GetToF() const
   {
      return fPar->GetStringValue("ToF");
   }
   void  SetToF(const Char_t* tof)
   {
      fPar->SetValue("ToF", tof);
      Modified();
   }// *MENU={Hierarchy="Set.../Time of Flight"}*

   void TestIdentification(TH2F* data, TH1F* h1_q,
                           TH2F* h2_q_qxaoq, TH2F* h2_q_a = 0);

   void GetLimitsOf_A_Q_AoQ(Int_t& Amin, Int_t& Amax, Int_t& Qmin, Int_t& Qmax, Double_t& AoQmin, Double_t& AoQmax);

   void QAConvertYaxis();//*MENU*

   Bool_t IsAvsAoQ() const
   {
      return (GetVarY()[0] == 'A');
   }
   Bool_t IsQvsAoQ() const
   {
      return (GetVarY()[0] == 'Q');
   }

   Double_t GetRealQ() const
   {
      return fRealQ;
   }

   ClassDef(KVIDQAGrid, 1) //Base class for 2D Q & A identification grids
};

#endif
