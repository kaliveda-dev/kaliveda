//Created by KVClassFactory on Fri Jun 20 12:07:39 2014
//Author: Guilain ADEMARD

#ifndef __KVIDQALINE_H
#define __KVIDQALINE_H

#include "KVIDZALine.h"

#include "KVIDQAMarker.h"
//class KVIDQAMarker;
class KVIDQALine : public KVIDZALine {

   friend class KVIDQAGrid;

protected:

   Int_t fQ;//! dummy variable used by context menu dialog boxes
   Int_t fNextA;//! next A value for the next call of InsertMarker
   Bool_t fNextAinc;//! true if the next A value should increase

   KVList* fMarkers; //-> list of Q-A identification markers

   void init();
   virtual void SetNameFromNucleus();
   virtual void UpdateLineStyle();
   virtual void WriteAsciiFile_extras(std::ofstream&, const Char_t* name_prefix = "");
   virtual void ReadAsciiFile_extras(std::ifstream&);
   virtual KVIDQAMarker* New(const Char_t*);
public:
   KVIDQALine();
   virtual ~KVIDQALine();
   void Copy(TObject& obj) const;
   virtual void Draw(Option_t* chopt = "");
   virtual void UnDraw();
   virtual void ExecuteEvent(Int_t event, Int_t px, Int_t py);
   void IdentA(Double_t x, Double_t y, Int_t& A, Double_t& realA, Int_t& code) const;

   virtual Int_t InsertMarker(Int_t A = 0); // *MENU* *ARGS={A=>fNextA}
   virtual Int_t RemoveMarker(Int_t a = 0); //*MENU*
   virtual Int_t InsertPoint();//*MENU*
   virtual Int_t InsertPoint(Int_t i, Double_t x, Double_t y, Double_t x_prev);
   using KVIDZALine::RemovePoint;//*MENU*
   virtual Int_t RemovePoint(Int_t i);
   void IncrementPtIdxOfMarkers(Int_t idx, Int_t ival = 1);

   void FindAMarkers(TH1* h);

   Int_t GetNextA() const
   {
      return fNextA;
   }
   void  SetNextA(Int_t A)
   {
      fNextA = A;
   }

   KVIDQAMarker* FindNearestIDMarker(Double_t x, Double_t y, Int_t& idx, Int_t& idx_low, Int_t& idx_up, Double_t& dist, Double_t& dist_low, Double_t& dist_up) const;

   virtual void Initialize();
   virtual void CalculateMarkerWidths();
   Bool_t ProjIsBetween(Double_t x, Double_t y, KVIDQAMarker* m1, KVIDQAMarker* m2) const;

   void IncrementAMarkers(Int_t val = 1, Option_t* dir = "all"); //*MENU*

   //---------- inline methods ---------------------//

   virtual Int_t  GetA() const;
   virtual Int_t GetQ() const;
   virtual Int_t  GetNumberOfMasses() const;
   virtual void SetQ(Int_t qtnum)
   {
      SetZ(qtnum);
   }; // *MENU={Hierarchy="SetNucleus.../Q"}*
   virtual void AddMarker(KVIDQAMarker* marker);
   KVList* GetMarkers() const;
   KVIDQAMarker* GetMarkerAt(Int_t index) const;
   Bool_t OnlyQId() const;
   void SortMarkers();
   Bool_t IsAvsAoQ() const;
   Bool_t IsQvsAoQ() const;

   ClassDef(KVIDQALine, 1) //Base class for identification ridge lines and spots corresponding to different charge states and masses respectively
};

//_______________________________________________________________//

inline void KVIDQALine::SetNameFromNucleus()
{
   SetName(Form("Q=%d", GetQ()));
   fMarkers->R__FOR_EACH(KVIDQAMarker, SetNameFromNucleus)();
   UpdateLineStyle();
};
//_______________________________________________________________//

inline void KVIDQALine::UpdateLineStyle()
{
   // Set line style and color as a function of the fQ value
   static Int_t lc[] = { kGray + 1, kGray + 2, kGray + 3, kBlack};
   Int_t c = lc[GetQ() % 4];
   SetLineColor(c);

}
//_______________________________________________________________//

inline void KVIDQALine::AddMarker(KVIDQAMarker* marker)
{
   // Add QA-marker to the line. It will be deleted by the line.
   fMarkers->Add(marker);
   marker->SetParent(this);
}
//_______________________________________________________________//

inline KVList* KVIDQALine::GetMarkers() const
{
   return fMarkers;
}
//_______________________________________________________________//

inline Int_t KVIDQALine::GetA() const
{
   // returns number of A markers
   return fMarkers->GetEntries();
};
//_______________________________________________________________//

inline Int_t KVIDQALine::GetQ() const
{
   // returns charge states
   return GetZ();
};
//_______________________________________________________________//

inline Int_t KVIDQALine::GetNumberOfMasses() const
{
   // returns number of masses identifiable by this line
   // i.e. number of A markers
   return fMarkers->GetEntries();
};
//_______________________________________________________________//

inline KVIDQAMarker* KVIDQALine::GetMarkerAt(Int_t index) const
{
   // Return marker at position 'index' (=0,1,...) in list of A-makers (fMarkers)
   return (KVIDQAMarker*)fMarkers->At(index);
};
//_______________________________________________________________//

inline Bool_t KVIDQALine::OnlyQId() const
{
   return fGridOnlyZId;
};
//_______________________________________________________________//

inline void KVIDQALine::SortMarkers()
{
   if (!fMarkers->IsSorted()) fMarkers->Sort();
}
//_______________________________________________________________//

inline Bool_t KVIDQALine::IsAvsAoQ() const
{
   return (GetVarY()[0] == 'A');
}
//_______________________________________________________________//

inline Bool_t KVIDQALine::IsQvsAoQ() const
{
   return (GetVarY()[0] == 'Q');
}

#endif
