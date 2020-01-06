/***************************************************************************
$Id: KVDBParameterSet.h,v 1.13 2007/04/18 14:28:49 ebonnet Exp $
                          KVDBParameterSet.h  -  description
                             -------------------
    begin                : wed feb 12 2003
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
#ifndef KVDB_PARAMETER_SET_H
#define KVDB_PARAMETER_SET_H

#include <cassert>
#include "KVDBRecord.h"
#include "TString.h"
#include "KVDBKey.h"
#include "KVNameValueList.h"

class KVDBParameterSet: public KVDBRecord {
protected:
   Int_t fParamNumber;          //number of parameters
   KVNameValueList fParameters;//parameters

public:
   KVDBParameterSet();
   KVDBParameterSet(const Char_t* name, const Char_t* type,
                    UShort_t pnum);
   virtual ~ KVDBParameterSet();

   Int_t GetParamNumber() const
   {
      return fParamNumber;
   }
   Double_t GetParameter(UShort_t i = 0) const;
   Double_t GetParameter(TString name) const;
   TString GetStringParameter(const TString& name) const;
   const Char_t* GetParamName(UShort_t i = 0) const;
   Bool_t HasParameter(const Char_t* name) const
   {
      return fParameters.HasParameter(name);
   }
   const KVNameValueList& GetParameters() const
   {
      return fParameters;
   }
   KVRList* GetRuns() const;
   void SetParameter(UShort_t i, Double_t val);
   void SetParameter(Double_t val);
   void SetParameter(TString name, Double_t val);
   void SetParameter(const TString& name, const char* value);
   void SetParameters(Double_t val, ...);
   void SetParameters(const std::vector<Double_t>* const pars);
   void SetParamName(UShort_t i, const Char_t* name);
   void SetParamNames(const Char_t* name, ...);
   void Print(Option_t* option = "") const;

   ClassDef(KVDBParameterSet, 1)       // Set of parameters for calibration
};

inline Double_t KVDBParameterSet::GetParameter(UShort_t i) const
{
   return fParameters.GetDoubleValue(i);
}

inline const Char_t* KVDBParameterSet::GetParamName(UShort_t i) const
{
   return fParameters.GetNameAt(i);
}

inline KVRList* KVDBParameterSet::GetRuns() const
{
   return GetKey("Runs")->GetLinks();
}
inline void KVDBParameterSet::SetParameter(UShort_t i, Double_t val)
{
   fParameters.GetParameter(i)->Set(val);
}

inline void KVDBParameterSet::SetParameter(Double_t val)
{
   SetParameter(0, val);
}


#endif
