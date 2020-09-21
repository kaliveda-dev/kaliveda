/*
$Id: KVIDZoneContour.h,v 1.2 2016/10/09 13:36:00 pastore g3dott0 $
$Revision: 1.2 $
$Date: 2016/10/09 13:36:00 $
*/

//Author: PastoreGiu

#ifndef __KVIDZoneContour_H
#define __KVIDZoneContour_H

#include "KVIDContour.h"

/**
\class KVIDZoneContour
\brief Graphical contour for including regions with mass identification in PSA(Imax) maps
\ingroup Identification
*/

class KVIDZoneContour : public KVIDContour {
protected:

   Bool_t   fExclusive;//set to kTRUE if want to accept points OUTSIDE the contour

   virtual void SetZ(Int_t /*atnum*/)
   {
      ;
   };
   virtual void SetA(Int_t /*atnum*/)
   {
      ;
   };
   virtual void SetAandZ(Int_t /*atnum*/, Int_t /*ztnum*/)
   {
      ;
   };
   virtual void SetMassFormula(Int_t /*mf*/)
   {
      ;
   };
   virtual void WriteAsciiFile_extras(std::ofstream&, const Char_t* name_prefix = "");
   virtual void ReadAsciiFile_extras(std::ifstream&);

public:

   KVIDZoneContour();
   KVIDZoneContour(const KVIDZoneContour&);
   KVIDZoneContour(const TCutG&);
   virtual ~KVIDZoneContour();

   virtual Int_t GetA() const
   {
      return 0;
   };
   virtual Int_t GetZ() const
   {
      return 0;
   };
   virtual Int_t GetMassFormula()const
   {
      return 0;
   };

   virtual Bool_t TestPoint(Double_t x, Double_t y)
   {
      // Test whether point (x,y) is in the acceptable region defined
      // by this contour. This means
      //   for an inclusive contour:   (x,y) OK if inside contour
      //   for an exclusive contour:   (x,y) OK if outside contour

      return fExclusive ? !IsInside(x, y) : IsInside(x, y);
   };

   // Make contour exclusive i.e. only accept points outside contour
   void SetExclusive(Bool_t e = kTRUE); // *TOGGLE*

   // kTRUE for exclusive contour, kFALSE for inclusive contour
   Bool_t IsExclusive() const
   {
      return fExclusive;
   };
   // This is redeclared to make it appear in context menus for KVIDContourLines
   virtual void     SetName(const char* name)
   {
      KVIDContour::SetName(name);
   };  // *MENU*

   ClassDef(KVIDZoneContour, 1) //Graphical contour for excluding/including regions in particle identification maps
};

#endif
