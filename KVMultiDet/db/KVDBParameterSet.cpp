/***************************************************************************
$Id: KVDBParameterSet.cpp,v 1.11 2007/04/18 14:30:20 ebonnet Exp $
                          KVDBParameterSet.cpp  -  description
                             -------------------
    begin                : jeu fév 13 2003
    copyright            : (C) 2003 by Alexis Mignon
    email                : mignon@ganil.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "KVDBParameterSet.h"
#include <cstdarg>
#include <iostream>
#include "TString.h"
#include "TBuffer.h"
using std::cout;
using std::endl;

ClassImp(KVDBParameterSet);

//___________________________________________________________________________
//      Jeu de parametres pour la calibration
//      Cette classe est juste un "wraper" pour un tableau de paramètre
//
//

//___________________________________________________________________________
KVDBParameterSet::KVDBParameterSet()
{
   fParamNumber = 0;
   fParameters = 0;
   fParamNames = 0;
}

//___________________________________________________________________________
KVDBParameterSet::KVDBParameterSet(const Char_t* name,
                                   const Char_t* title,
                                   UShort_t pnum): KVDBRecord(name, title)
{
//Initialise a KVDBRecord for a set of "pnum" parameters.
//The names of the parameters are "par_1", "par_2", etc.
//The KVDBKey "Runs" is initialised to cross-reference the runs for which
//this set of parameters is valid.
   fParamNumber = pnum;
   fParameters = new Double_t[pnum];
   fParamNames = new Char_t* [pnum];
   for (UInt_t i = 0; i < pnum; i++) {
      fParameters[i] = 0;
      fParamNames[i] = new Char_t[32];
      sprintf(fParamNames[i], "par_%u", i + 1);
   }
   AddKey("Runs", "List of Runs");
   //Unique not used because it makes the setting up of the list horribly slow
   // if(key) key->SetUniqueStatus(kTRUE);
}

//___________________________________________________________________________
KVDBParameterSet::~KVDBParameterSet()
{
   if (fParameters) {
      delete[]fParameters;
      fParameters = 0;
   }
   if (fParamNames) {
      for (int i = 0; i < fParamNumber; i++) {
         delete[]fParamNames[i];
      }
      fParamNames = 0;
   }
}

void KVDBParameterSet::Streamer(TBuffer& R__b)
{
   // Stream an object of class KVDBParameterSet.

   UInt_t R__s, R__c;
   if (R__b.IsReading()) {
      Version_t R__v = R__b.ReadVersion(&R__s, &R__c);
      if (R__v) {
      }
      KVDBRecord::Streamer(R__b);
      R__b >> fParamNumber;
      if (fParameters) {
         delete[]fParameters;
         fParameters = 0;
      }
      if (fParamNumber) {
         fParameters = new Double_t[fParamNumber];
         R__b.ReadFastArray(fParameters, fParamNumber);
      }
      if (fParamNames) {
         for (int i = 0; i < fParamNumber; i++) {
            delete[]fParamNames[i];
         }
         fParamNames = 0;
      }
      if (fParamNumber) {
         fParamNames = new Char_t* [fParamNumber];
         for (Int_t i = 0; i < fParamNumber; i++) {
            fParamNames[i] = new Char_t[32];
            R__b.ReadFastArray(fParamNames[i], 32);
         }
      }
      R__b.CheckByteCount(R__s, R__c, KVDBParameterSet::IsA());
   } else {
      R__c = R__b.WriteVersion(KVDBParameterSet::IsA(), kTRUE);
      KVDBRecord::Streamer(R__b);
      R__b << fParamNumber;
      if (fParamNumber) {
         R__b.WriteFastArray(fParameters, fParamNumber);
         for (Int_t i = 0; i < fParamNumber; i++) {
            R__b.WriteFastArray(fParamNames[i], 32);
         }
      }
      R__b.SetByteCount(R__c, kTRUE);
   }
}

//____________________________________________________________________________
void KVDBParameterSet::SetParameters(Double_t val, ...)
{
   // Be cautious when using this method, if the number of arguments is lower than
   // the expected number of parameters a segmantation fault will occur !!!!!
   // exceeding argument won't be considered at all

   va_list ap;
   va_start(ap, val);

   int arg_n = 0;
   fParameters[arg_n++] = val;
   while (arg_n < fParamNumber) {
      fParameters[arg_n] = va_arg(ap, Double_t);
      arg_n++;
   }

   va_end(ap);
}

//____________________________________________________________________________
void KVDBParameterSet::SetParameters(const std::vector<Double_t>* const pars)
{
   assert(pars->size() <= static_cast<UInt_t>(fParamNumber));

   UInt_t arg_n(0);
   std::vector<Double_t>::const_iterator it;
   for (it = pars->begin(); it != pars->end(); ++it) {
      fParameters[arg_n] = *it;
      ++arg_n;
   }
}

//____________________________________________________________________________
void KVDBParameterSet::SetParamName(UShort_t i, const Char_t* name)
{
   strcpy(fParamNames[i], name);
}

//____________________________________________________________________________
Double_t KVDBParameterSet::GetParameter(TString name)  const
{
   return GetParameter(GetParamIndex(name));
}

//____________________________________________________________________________
void KVDBParameterSet::SetParameter(TString name, Double_t val)
{
   SetParameter(GetParamIndex(name), val);
}

//____________________________________________________________________________
Int_t KVDBParameterSet::GetParamIndex(TString name)  const
{
   Int_t index = 0, ii = 0;
   while (TString(fParamNames[ii++]) != name && ii < GetParamNumber()) index = ii;
   if (TString(fParamNames[index]) != name) cout << "parameter " << name << " does not exist last defined parameter position is set" << endl;
   return index;
}

//____________________________________________________________________________
void KVDBParameterSet::SetParamNames(const Char_t* name, ...)
{
   // Be cautious when using this method, if the number of arguments is lower than
   // the expected number of parameters a segmantation fault will occur !!!!!
   // exceeding argument won't be considered at all

#ifdef KV_DEBUG
   Info("SetParamNames(const Char_t* name,...)", "Start");
#endif
   va_list ap;
   va_start(ap, name);

   int arg_n = 0;
   strcpy(fParamNames[arg_n++], name);
   while (arg_n < fParamNumber) {
      strcpy(fParamNames[arg_n], va_arg(ap, Char_t*));
      arg_n++;
   }

#ifdef KV_DEBUG
   Info("SetParamNames(const Char_t* name,...)", "OK");
#endif
}

//_____________________________________________________________________________
void KVDBParameterSet::Print(Option_t*) const
{
   KVDBRecord::Print();
   const int max_disp_param = 10;
   cout << ">>>> KVParameterSet :" << endl
        << GetName() << "  " << GetTitle() << endl
        << " Parameters :\t" << GetParamNumber() << endl;
   for (int i = 0; (i < GetParamNumber()) && (i < max_disp_param); i++) {
      cout << GetParamName(i) << " : " << GetParameter(i) << endl;
   }
   cout << endl << "<<<<<<<<<<" << endl;
}
