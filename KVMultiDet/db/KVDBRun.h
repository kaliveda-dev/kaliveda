/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef KV_DB_RUN_H
#define KV_DB_RUN_H

#include "KVDBRecord.h"
#include "KVDBSystem.h"
#include "KVTarget.h"
#include "KVString.h"
#include "TDatime.h"
#include "KVNameValueList.h"
#include <RQ_OBJECT.h>

#define KV__GET_INT(__param) return (fParameters.HasIntParameter(__param) ? fParameters.GetIntValue(__param) : 0);
#define KV__GET_ULONG(__param) return (fParameters.HasValue64bit(__param) ? fParameters.GetValue64bit(__param) : 0);
#define KV__GET_DBL(__param) return (fParameters.HasDoubleParameter(__param) ? fParameters.GetDoubleValue(__param) : 0.);
#define KV__GET_STR(__param) return (fParameters.HasStringParameter(__param) ? fParameters.GetStringValue(__param) : "");
#define KV__SET_INT(__param,__val) fParameters.SetValue(__param,__val);
#define KV__SET_ULONG(__param,__val) fParameters.SetValue64bit(__param,__val);
#define KV__SET_DBL(__param,__val) fParameters.SetValue(__param,__val);
#define KV__SET_STR(__param,__val) fParameters.SetValue(__param,__val);

/**
  \class KVDBRun
  \brief Description of an experimental run in database
  \ingroup Calibration,DM
 */

class KVDBRun: public KVDBRecord {

   RQ_OBJECT("KVDBRun")
   Bool_t fBlockSignals;//!

protected:

   KVNameValueList fParameters;//list of named parameters for run
   TDatime fDatime;             //!set dynamically with date&time of ROOT file corresponding to run
   TString fKVVersion;             //!set dynamically with KaliVeda version used to generate ROOT file corresponding to run
   TString fUserName;             //!set dynamically with name of user who generated ROOT file corresponding to run

public:

   void SetKVVersion(const Char_t* V)
   {
      fKVVersion = V;
   };
   const Char_t* GetKVVersion() const
   {
      return fKVVersion.Data();
   };
   void SetUserName(const Char_t* U)
   {
      fUserName = U;
   };
   const Char_t* GetUserName() const
   {
      return fUserName.Data();
   };

   void Modified() // *SIGNAL*
   {
      // Signal sent out when run properties change (used by GUI)
      if (!fBlockSignals) Emit("Modified()");
   };
   void BlockSignals(Bool_t yes = kTRUE)
   {
      // if yes=kTRUE, block 'Modified()' signal
      // if yes=kFALSE, allow 'Modified()' signal
      fBlockSignals = yes;
   };

   //Returns kTRUE if a parameter with the given name (either integer, floating point or string) has been set.
   //For string parameters, if the parameter exists we also check if the string is empty (unless check_whitespace=kFALSE)
   Bool_t Has(const Char_t* param, Bool_t check_whitespace = kTRUE) const
   {
      if (check_whitespace && fParameters.HasStringParameter(param)) {
         return !(fParameters.GetTStringValue(param).IsWhitespace());
      }
      return (fParameters.HasIntParameter(param) || fParameters.HasDoubleParameter(param) || fParameters.HasStringParameter(param));
   };

   KVDBRun();
   KVDBRun(Int_t number, const Char_t* title);
   virtual ~ KVDBRun();

   void SetNumber(Int_t n)
   {
      KVDBRecord::SetNumber(n);
      Modified();
   }

   void SetTrigger(Int_t trig)
   {
      SetScaler("Trigger multiplicity", trig);
   };

   Int_t GetTrigger() const
   {
      return GetScaler("Trigger multiplicity");
   };
   const Char_t* GetTriggerString() const
   {
      if (GetTrigger() > 0) return Form("M>=%d", GetTrigger());
      else return Form("xxx");
   };

   const TDatime& GetDatime() const
   {
      return fDatime;
   };
   const Char_t* GetDatimeString()
   {
      return fDatime.AsString();
   };
   void SetDatime(TDatime& dat)
   {
      dat.Copy(fDatime);
      Modified();
   };

   KVDBSystem* GetSystem() const;
   const Char_t* GetSystemName() const
   {
      return (GetSystem() ? GetSystem()->GetName() : "");
   };

   ULong64_t GetEvents() const
   {
      return GetScaler64("Events");
   };
   Double_t GetTime() const
   {
      return Get("Length (min.)");
   };
   Double_t GetSize() const
   {
      return Get("Size (MB)");
   };

   const Char_t* GetComments() const
   {
      return GetString("Comments");
   };
   const Char_t* GetStartDate() const
   {
      return GetString("Start Date");
   };
   const Char_t* GetDate() const
   {
      return GetStartDate();
   };
   const Char_t* GetEndDate() const
   {
      return GetString("End Date");
   };

   //Return target used for this run (actually target of KVDBSystem associated to run)
   KVTarget* GetTarget() const
   {
      return (GetSystem() ? GetSystem()->GetTarget() : 0);
   };

   void SetEvents(ULong64_t evt_number)
   {
      SetScaler64("Events", evt_number);
   };
   void SetTime(Double_t time)
   {
      Set("Length (min.)", time);
   };
   void SetSize(Double_t s)
   {
      Set("Size (MB)", s);
   };
   void SetComments(const KVString& comments)
   {
      Set("Comments", comments);
   };
   void SetStartDate(const KVString& date)
   {
      Set("Start Date", date);
   };
   void SetDate(const KVString& d)
   {
      SetStartDate(d);
   };
   void SetEndDate(const KVString& d)
   {
      Set("End Date", d);
   };

   virtual void SetSystem(KVDBSystem* system);
   virtual void UnsetSystem();

   virtual void Print(Option_t* option = "") const;

   virtual void WriteRunListLine(std::ostream&, Char_t delim = '|') const;
   virtual void ReadRunListLine(const KVString&);
   virtual void WriteRunListHeader(std::ostream&, Char_t delim = '|') const;

   //Set value for the scaler with the given name for this run
   virtual void SetScaler(const Char_t* name, Int_t val)
   {
      KV__SET_INT(name, val)
      Modified();
   };
   //Get value of scaler with the given name for this run
   virtual Int_t GetScaler(const Char_t* name) const
   {
      KV__GET_INT(name)
   };
   //Set value for 64-bit scaler with the given name for this run
   virtual void SetScaler64(const Char_t* name, ULong64_t val)
   {
      KV__SET_ULONG(name, val)
      Modified();
   };
   //Get value of 64-bit scaler with the given name for this run
   virtual ULong64_t GetScaler64(const Char_t* name) const
   {
      KV__GET_ULONG(name)
   };
   //Set numerical (non-scaler) characteristic of run
   void Set(const Char_t* param, Double_t val)
   {
      KV__SET_DBL(param, val)
      Modified();
   };
   //Get numerical (non-scaler) characteristic of run
   Double_t Get(const Char_t* param) const
   {
      KV__GET_DBL(param)
   };
   //Set characteristic of run
   void Set(const Char_t* param, const KVString& val)
   {
      KV__SET_STR(param, val)
      Modified();
   };
   //Get characteristic of run
   const Char_t* GetString(const Char_t* param) const
   {
      KV__GET_STR(param)
   };

   ClassDef(KVDBRun, 10)         //Base class for an experiment run
};
#endif
