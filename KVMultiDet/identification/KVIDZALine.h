/***************************************************************************
                          KVIDZALine.h  -  description
                             -------------------
    begin                : Nov 10 2004
    copyright            : (C) 2004 by J.D. Frankland
    email                : frankland@ganil.fr

$Id: KVIDZALine.h,v 1.6 2009/03/13 13:04:11 franklan Exp $
***************************************************************************/

#ifndef KVIDZALine_H
#define KVIDZALine_H

#include "KVIDLine.h"
#include "KVNucleus.h"
#include "TGraphErrors.h"

class KVIDZALine: public KVIDLine {

   friend class KVIDZAGrid;

protected:

   Double_t fWidth;                                // the "width" of the line
   TGraphErrors* fLineWithWidth;      //! used to display width of line

   virtual void SetAsymWidth(Double_t d_l, Double_t d_r);

   // redeclared as 'protected' method, to avoid users setting name
   // directly (name is generated from A & Z of line)
   virtual void SetName(const char* name)
   {
      TGraph::SetName(name);
   };
   virtual void WriteAsciiFile_extras(std::ofstream&, const Char_t* name_prefix = "");
   virtual void ReadAsciiFile_extras(std::ifstream&);

public:

   KVIDZALine();
   KVIDZALine(const KVIDZALine&);

   virtual ~ KVIDZALine();
   void Copy(TObject& obj) const;

   Double_t GetWidth() const
   {
      return fWidth;
   };
   void SetWidth(Double_t w)
   {
      fWidth = w;
   };

   TGraphErrors* GetLineWithWidth();

   inline virtual Int_t Compare(const TObject*) const;

   virtual void Print(Option_t* opt = "") const;

   void ReadAsciiFile_KVIDZLine(std::ifstream&);

   //virtual Int_t GetID() const { return GetA();}

   ClassDef(KVIDZALine, 2)//Base class for identification ridge lines corresponding to different nuclear species
};

//_____________________________________________________________________________________________

inline Int_t KVIDZALine::Compare(const TObject* obj) const
{
   //Used to sort lists of KVIDZALines.
   //Sorts the lines as a function of increasing Z, then increasing A
   //(i.e. p,d,t,3He,4He,6He, etc. etc.)

   //First compare Z : only if Z1=Z2 do we need to compare A

   int zline1 = GetZ();
   int zline2 = ((KVIDZALine*)obj)->GetZ();
   if (zline1 > zline2)
      return 1;
   if (zline1 < zline2)
      return -1;
   // same Z: compare A
   int aline1 = GetA();
   int aline2 = ((KVIDZALine*)obj)->GetA();
   if (aline1 > aline2)
      return 1;
   if (aline1 < aline2)
      return -1;
   return 0;
}

class KVIDZLine : public KVIDZALine {

public:
   KVIDZLine();
   virtual ~KVIDZLine() {};

   ClassDef(KVIDZLine, 3) //FOR BACKWARDS COMPATIBILITY ONLY. OBSOLETE.
};

#endif
