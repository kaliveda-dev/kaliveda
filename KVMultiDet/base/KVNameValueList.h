//Created by KVClassFactory on Tue Jun 29 14:44:22 2010
//Author: bonnet

#ifndef __KVNAMEVALUELIST_H
#define __KVNAMEVALUELIST_H

#include "KVHashList.h"
#include "TNamed.h"
#include "TRegexp.h"
#include "KVNamedParameter.h"
class KVEnv;

class KVNameValueList : public TNamed {
protected:
   KVHashList fList;//list of KVNamedParameter objects
   Bool_t fIgnoreBool;//do not convert "yes", "false", "on", etc. in TEnv file to boolean

   virtual void SetValue_str(const Char_t* name, const Char_t* value);
   virtual void SetValue_int(const Char_t* name, Int_t value);
   virtual void SetValue_bool(const Char_t* name, Bool_t value);
   virtual void SetValue_flt(const Char_t* name, Double_t value);

   virtual Double_t IncValue_flt(const Char_t* name, Double_t value);
   virtual Int_t    IncValue_int(const Char_t* name, Int_t value);

public:

   KVNameValueList();
   KVNameValueList(const Char_t* name, const Char_t* title = "");
   KVNameValueList(const KVNameValueList&);
   virtual ~KVNameValueList();
   KVNameValueList& operator=(const KVNameValueList&);

   KVHashList* GetList() const;

   virtual void Clear(Option_t* opt = "");
   virtual void ClearSelection(TRegexp&);
   virtual void Print(Option_t* opt = "") const;
   virtual void ls(Option_t* opt = "") const
   {
      Print(opt);
   }

   void SetOwner(Bool_t enable = kTRUE);
   Bool_t IsOwner() const;

   void Copy(TObject& nvl) const;
   Int_t Compare(const TObject* nvl) const;

   void SetValue(const Char_t* name, const Char_t* value);
   void SetValue(const Char_t* name, Int_t value);
   void SetValue(const Char_t* name, Bool_t value);
   void SetValue(const Char_t* name, Double_t value);
   void SetValue(const KVNamedParameter&);

   void SetValueAt(const Char_t* name, Double_t value, Int_t idx);
   void SetFirstValue(const Char_t* name, Double_t value);
   void SetLastValue(const Char_t* name, Double_t value);

   Double_t IncrementValue(const Char_t* name, Double_t value);
   Int_t    IncrementValue(const Char_t* name, Int_t value);

   Bool_t IsValue(const Char_t* name, const Char_t* value);
   Bool_t IsValue(const Char_t* name, Int_t value);
   Bool_t IsValue(const Char_t* name, Bool_t value);
   Bool_t IsValue(const Char_t* name, Double_t value);

   KVNamedParameter* FindParameter(const Char_t* name) const;
   KVNamedParameter* GetParameter(Int_t idx) const;
   void RemoveParameter(const Char_t* name);
   Bool_t HasParameter(const Char_t* name) const;
   Bool_t HasIntParameter(const Char_t* name) const;
   Bool_t HasBoolParameter(const Char_t* name) const;
   Bool_t HasDoubleParameter(const Char_t* name) const;
   Bool_t HasStringParameter(const Char_t* name) const;
   Int_t GetNameIndex(const Char_t* name);
   const Char_t* GetNameAt(Int_t idx) const;
   Int_t GetNpar() const;
   Int_t GetEntries() const
   {
      return GetNpar();
   }
   Bool_t IsEmpty() const
   {
      return GetNpar() == 0;
   }

   Int_t GetIntValue(const Char_t* name) const;
   Bool_t GetBoolValue(const Char_t* name) const;
   Double_t GetDoubleValue(const Char_t* name) const;
   const Char_t* GetStringValue(const Char_t* name) const;
   const TString& GetTStringValue(const Char_t* name) const;

   Int_t GetIntValue(Int_t idx) const;
   Bool_t GetBoolValue(Int_t idx) const;
   Double_t GetDoubleValue(Int_t idx) const;
   const Char_t* GetStringValue(Int_t idx) const;
   const TString& GetTStringValue(Int_t idx) const;

   virtual void ReadEnvFile(const Char_t* filename);
   virtual KVEnv* ProduceEnvFile();
   virtual void WriteEnvFile(const Char_t* filename);

   void Sort(Bool_t order = kSortAscending)
   {
      fList.Sort(order);
   }

   KVNameValueList operator += (KVNameValueList& nvl);

   void WriteClass(const Char_t* classname, const Char_t* classdesc, const Char_t* base_class = "");

   void SetIgnoreBool(Bool_t ignore = kTRUE)
   {
      // When reading a list from a file using ReadEnvFile(), any string values matching
      // TRUE, FALSE, ON, OFF, YES, NO, OK, NOT are automatically converted to boolean
      // parameters. If you want to disable this and keep such strings as strings,
      // call this method first.
      fIgnoreBool = ignore;
   }
   void SetFromEnv(TEnv* tenv, const TString& prefix = "");
   void WriteToEnv(TEnv* tenv, const TString& prefix = "");

   ClassDef(KVNameValueList, 4) //A general-purpose list of parameters
};

#endif
