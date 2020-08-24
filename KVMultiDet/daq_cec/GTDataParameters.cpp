// $Id: GTDataParameters.cpp,v 1.6 2007/06/08 15:49:10 franklan Exp $
// Author: $Author: franklan $

//                        GTDataParameters.cpp  -  Parameters name handling
//                             -------------------
//    begin                : Thu Jun 14 2001
//    copyright            : (C) 2001 by Garp
//    email                : patois@ganil.fr
// ---------------------------------------------------------------------------
/*
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 **************************************************************************
*/

#include "Riostream.h"

#include "TNamed.h"

// ganil_tape lib headers
#include "GanTape/GEN_TYPE.H"
#include "GanTape/gan_acq_buf.h"
#include "GTDataParameters.h"
#include "TList.h"

using namespace std;

ClassImp(GTDataParameters);

//______________________________________________________________________________
GTDataParameters::GTDataParameters(void)
{
   //Default constructor. Don't create anything yet.
   fList = new TList;
   fList->SetOwner();  // As owners of the data in tree, they will be deleted
   // when we'll delete the tree itself
}

//______________________________________________________________________________
GTDataParameters::~GTDataParameters(void)
{
   delete fList;
}


//______________________________________________________________________________
char* GTDataParameters::CopyParam(char* Dest, char* Source) const
{
   // Small utility routine to copy a char string.
   char c;
   do {
      c = *Source++;
      if ((c == ',') || (c == 0x0d))
         c = '\0';
      *Dest++ = c;
   }
   while (c != '\0');
   return (Source);
}


//______________________________________________________________________________
int GTDataParameters::Fill(const char* buffParam)
{
   // Data from the parameter buffet (buffParam) have been read from disk and is
   // parsed by this routine who create the list of labels and corresponding
   // index.
   //Returns largest parameter number (not necessarily same as total number of parameters)

   char* CurrPointer;
   char NomParam[20], chaine[20];
   int  NumPar, NbBits;

   CurrPointer = (char*)buffParam;

   fMaxIndex = 0;

   while (strncmp(CurrPointer, "    ", 4) != 0 &&
          strncmp(CurrPointer, "!!!!", 4) != 0) {
      CurrPointer = CopyParam(NomParam, CurrPointer);
      CurrPointer = CopyParam(chaine, CurrPointer);
      sscanf(chaine, "%d", &NumPar);
      CurrPointer = CopyParam(chaine, CurrPointer);
      sscanf(chaine, "%d", &NbBits);
      GTDataPar* par = new GTDataPar(NomParam, NumPar, NbBits);
      fList->Add(par);
      fMaxIndex = TMath::Max(par->Index(), fMaxIndex);
   }
   return (fMaxIndex);
}

//______________________________________________________________________________
int GTDataParameters::GetIndex(const TString parName) const
{
   // Return the index number corresponding to a text label.
   GTDataPar* par = (GTDataPar*)fList->FindObject(parName);
   if (!par) {
      cout << "Parameter " << parName << " not found!" << endl;
      return (-1);
   }
   return (par->Index());
}

//______________________________________________________________________________
const char* GTDataParameters::GetParName(const int index) const
{
   // Return the text label name corresponding to a parameter number
   TIter next(fList);
   GTDataPar* par;
   while ((par = (GTDataPar*)next())) {
      if (par->Index() == index) return par->GetName();
   }
   return ("");
}

//______________________________________________________________________________
GTDataPar* GTDataParameters::GetPar(const int index) const
{
   // Return the parameter corresponding to the parameter number
   TIter next(fList);
   GTDataPar* par;
   while ((par = (GTDataPar*)next())) {
      if (par->Index() == index) return par;
   }
   return 0;
}


//______________________________________________________________________________
GTDataPar* GTDataParameters::GetPar(const TString parName) const
{
   // Return the parameter corresponding to the parameter name
   return (GTDataPar*)fList->FindObject(parName);
}

