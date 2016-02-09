/*
$Id: KVIDCutLine.h,v 1.2 2009/03/03 13:36:00 franklan Exp $
$Revision: 1.2 $
$Date: 2009/03/03 13:36:00 $
*/

//Created by KVClassFactory on Fri Feb 15 15:00:17 2008
//Author: franklan

#ifndef __KVIDCUTLINE_H
#define __KVIDCUTLINE_H

#include "KVIDLine.h"
#include "KVString.h"

class KVIDCutLine : public KVIDLine {
protected:
   KVString fAcceptedDirection;         //direction relative to cut line corresponding to acceptable region
   virtual void WriteAsciiFile_extras(std::ofstream&, const Char_t* name_prefix = "");
   virtual void ReadAsciiFile_extras(std::ifstream&);
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

public:
   KVIDCutLine();
   virtual ~KVIDCutLine();

   // Set the direction of the acceptable region relative to the cut line
   // E.g. if points to identify must be above this cut, use
   //    cut->SetAcceptedDirection("above")
   // Possible values are: "above", "below", "left" or "right"
   // (see KVIDLine::WhereAmI).
   virtual void SetAcceptedDirection(const Char_t* dir);  // *MENU*

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
   }
   virtual Bool_t TestPoint(Double_t x, Double_t y)
   {
      // Test whether point (x,y) is in the acceptable region defined
      // by this cut line and the direction given to SetAcceptedDirection
      // E.g. if the accepted direction has been set to "left",
      // the point (x,y) must be to the left of this line in
      // order for this method to return kTRUE.
      return WhereAmI(x, y, fAcceptedDirection.Data());
   };

   virtual const Char_t* GetAcceptedDirection() const
   {
      return fAcceptedDirection.Data();
   }

   // This is redeclared to make it appear in context menus for KVIDCutLines
   virtual void     SetName(const char* name)
   {
      KVIDLine::SetName(name);
   };  // *MENU*

   ClassDef(KVIDCutLine, 1) //Line in ID grid used to delimit regions where identification is possible
};

#endif
