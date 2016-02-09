// $Id: GTDataParameters.H,v 1.7 2007/09/04 14:36:47 ebonnet Exp $
// Author: $Author: ebonnet $
/***************************************************************************
//                        GTDataParameters.H  -  Parameters name handling
//                             -------------------
//    begin                : Thu Jun 14 2001
//    copyright            : (C) 2001 by Garp
//    email                : patois@ganil.fr
//////////////////////////////////////////////////////////////////////////
//
// Part of ROOTGanilTape
//
// GTDataParameters
//
// Handle parameters data names and related offsets in the event buffer
//
//
//////////////////////////////////////////////////////////////////////////
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef GTDataParameters_H
#define GTDataParameters_H

#include "TNamed.h"
#include "TString.h"
//______________________________________________________________________________
class GTDataPar : public TNamed {
public:
   GTDataPar(const TString name, const int index, const int bits) :
      TNamed(name, (TString)"")
   {
      fIndex = index;
      fBits = bits;
   }
   int Index(void)const
   {
      return (fIndex);
   };
   int Bits(void) const
   {
      return fBits;
   };
private:
   int fIndex; // Parameter index in data array
   int fBits; // number of bits used to represent parameter
};


//______________________________________________________________________________
class GTDataParameters : public TObject {
public:
   GTDataParameters(void);
   ~GTDataParameters(void);
   int         Fill(const char* buffParam);
   int         GetIndex(const TString parName) const;
   const char* GetParName(const int index) const;
   GTDataPar* GetPar(const int index) const;
   GTDataPar* GetPar(const TString parName) const;
   Int_t GetMaxIndex() const
   {
      return fMaxIndex;
   };
   const TList* GetList() const
   {
      return fList;
   };
private:
   char* CopyParam(char* Dest, char* Source) const;
   TList* fList; //! Parameters List, maybe a better structure could be made
   // but OK for now.
   Int_t fMaxIndex;//largest parameter number in parameter list
public:
   ClassDef(GTDataParameters, 1) // Data Parameter names and index class
};

#endif
