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
KVDBParameterSet::KVDBParameterSet()
{
   fParamNumber = 0;
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
   for (UInt_t i = 0; i < pnum; i++) {
      fParameters.SetValue(Form("par_%u", i + 1), 0.0);
   }
   AddKey("Runs", "List of Runs");
   //Unique not used because it makes the setting up of the list horribly slow
   // if(key) key->SetUniqueStatus(kTRUE);
}

//___________________________________________________________________________
KVDBParameterSet::~KVDBParameterSet()
{
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
   SetParameter(arg_n++, val);
   while (arg_n < fParamNumber) {
      SetParameter(arg_n, va_arg(ap, Double_t));
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
      SetParameter(arg_n, *it);
      ++arg_n;
   }
}

//____________________________________________________________________________
void KVDBParameterSet::SetParamName(UShort_t i, const Char_t* name)
{
   fParameters.GetParameter(i)->SetName(name);
}

//____________________________________________________________________________
Double_t KVDBParameterSet::GetParameter(TString name)  const
{
   return fParameters.GetDoubleValue(name);
}

TString KVDBParameterSet::GetStringParameter(const TString& name) const
{
   return fParameters.GetTStringValue(name);
}

//____________________________________________________________________________
void KVDBParameterSet::SetParameter(TString name, Double_t val)
{
   fParameters.SetValue(name, val);
}

void KVDBParameterSet::SetParameter(const TString& name, const char* value)
{
   fParameters.SetValue(name, value);
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
   SetParamName(arg_n, name);
   while (arg_n < fParamNumber) {
      SetParamName(arg_n, va_arg(ap, Char_t*));
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
   cout << ">>>> KVParameterSet :" << endl
        << GetName() << "  " << GetTitle() << endl
        << " Parameters :\t" << GetParamNumber() << endl;
   fParameters.Print();
   cout << endl << "<<<<<<<<<<" << endl;
}
