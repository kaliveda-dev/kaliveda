//Created by KVClassFactory on Wed Jul  9 15:38:39 2014
//Author: Guilain ADEMARD

#ifndef __KVIDQAMARKER_H
#define __KVIDQAMARKER_H

#include "TMarker.h"
#include "TMath.h"
#include "KVMacros.h" // 'UNUSED' macro

class KVIDQALine;
class KVIDQAMarker : public TMarker {
   friend class KVIDQALine;

protected:

   Int_t fA;        //Mass associated to this marker
   Int_t fIdx;      //index of the low closest point from this marker
   Double_t fDelta; // normalized position from low closest point and up closest point
   KVIDQALine* fParent; //! parent identification line
   TString fName;
   Double_t fWidth; //the "width" of the marker

   void init();
   virtual void SetNameFromNucleus();
   virtual void UpdateMarkerStyle();
   virtual void ReadAsciiFile(std::ifstream& file);
   virtual void WriteAsciiFile(std::ofstream& file);

public:
   KVIDQAMarker();
   KVIDQAMarker(KVIDQALine* parent, Int_t a);
   virtual ~KVIDQAMarker();
   void Copy(TObject& obj) const;
   virtual Int_t  Compare(const TObject* obj) const;
   virtual void ExecuteEvent(Int_t event, Int_t px, Int_t py);
   void SetParent(KVIDQALine* parent);
   void SetPointIndex(Int_t idx, Double_t delta = 0);
   Int_t GetQ() const;
   virtual void ls(Option_t* option = "") const;
   virtual void UpdateXandY();
   void SetPointIndexAndX(Int_t idx, Double_t x);
   Double_t DistanceToMarker(Double_t px, Double_t py) const;

   //-------------------- inline methods ------------------//

   virtual const char*  GetName() const;
   void SetName(const Char_t* name);
   Int_t GetA() const;
   void SetA(Int_t a);   // *MENU*
   Double_t GetDelta() const;
   Int_t GetPointIndex() const;
   void GetPointIndexAndDelta(Int_t& idx, Double_t& delta) const;
   Bool_t IsSortable() const;

   Double_t GetWidth() const;
   void     SetWidth(Double_t w);

   ClassDef(KVIDQAMarker, 1) //Base class for identification markers corresponding to differents couples of mass and charge state
};
//_____________________________________________________//

inline const char* KVIDQAMarker::GetName() const
{
   return fName.Data();
}
//_____________________________________________________//

inline void KVIDQAMarker::SetName(const Char_t* name)
{
   fName = name;
}
//_____________________________________________________//

inline void KVIDQAMarker::SetNameFromNucleus()
{
   SetName(Form("Q=%d A=%d", GetQ(), GetA()));
   UpdateMarkerStyle();
};
//_____________________________________________________//

inline void KVIDQAMarker::UpdateMarkerStyle()
{
   // Set marker style and color as a function of the fA value
   SetMarkerStyle(20);
   static Int_t mc[] = { kGray + 1, kGray + 2, kGray + 3, kBlack};
   Int_t c = mc[fA % 4];
   SetMarkerColor(c);
}
//_____________________________________________________//

inline Int_t KVIDQAMarker::GetA() const
{
   return fA;
}
//_____________________________________________________//

inline void KVIDQAMarker::SetA(Int_t a)
{
   fA = a;
   SetNameFromNucleus();
}
//_____________________________________________________//

inline Double_t KVIDQAMarker::GetDelta() const
{
   return fDelta;
}
//_____________________________________________________//

inline Int_t KVIDQAMarker::GetPointIndex() const
{
   return fIdx;
}
//_____________________________________________________//

inline void KVIDQAMarker::GetPointIndexAndDelta(Int_t& idx, Double_t& delta) const
{
   idx   = fIdx;
   delta = fDelta;
}
//_____________________________________________________//

inline Bool_t KVIDQAMarker::IsSortable() const
{
   return kTRUE;
}
//_____________________________________________________//

inline Double_t KVIDQAMarker::GetWidth() const
{
   return fWidth;
}
//_____________________________________________________//

inline void KVIDQAMarker::SetWidth(Double_t w)
{
   fWidth = w;
}
#endif
