//Created by KVClassFactory on Tue Jun 29 14:44:22 2010
//Author: bonnet

#ifndef __KVNAMEVALUELIST_H
#define __KVNAMEVALUELIST_H

#include "KVHashList.h"
#include "TNamed.h"
#include "TRegexp.h"
#include "KVNamedParameter.h"
class KVEnv;

/**
 \class KVNameValueList
 \ingroup Base
 \brief Handles lists of named parameters with different types, a list of KVNamedParameter objects

 ###Create a list from a string###
 You can create and initialise a list of parameters with the constructor:

 ~~~~~~~~~~~{.cpp}
 KVNameValueList k("A=10,B=stripes,C=6.751");
 k.Print();

 KVNameValueList:: :  (0x5602712ae5b0)
 <A=10>
 <B=stripes>
 <C=6.751>
 ~~~~~~~~~~~

 ###Adding and modifying parameters###
 Parameters are dynamically added or modified by calling SetValue():

 ~~~~~~~~~~~{.cpp}
 KVNameValueList l("MyList", "A list of things");
 l.SetValue("a", 6);
 l.SetValue("b", 3.14);
 l.SetValue("c", "sqrt");

 l.SetValue("a", false);  <= change value & type of existing parameter

 l.Print();

 KVNameValueList::MyList : A list of things (0x7f405e51e010)
 <a=false>
 <b=3.14>
 <c=sqrt>
 ~~~~~~~~~~~

 ###Retrieving parameters###
 Before trying to retrieve a parameter from the list, it is advisable to test if a parameter with the correct
 name **and type** exists in the list:

 ~~~~~~~~~~~{.cpp}
 l.HasDoubleParameter("a"); => false
 l.HasParameter<double>("b"); => true
 l.HasBoolParameter("a"); => true
 ~~~~~~~~~~~

 Values can then be retrieved with the following methods:

 ~~~~~~~~~~~{.cpp}
 l.GetValue<bool>("a"); => false
 l.GetDoubleValue("b"); => 3.1400
 l.GetValue<std::string>("c"); => (std::string) "sqrt"
 ~~~~~~~~~~~

 Some conversions between different types will work implicitly, however
 be aware that any attempt to convert a non-numeric string parameter into
 a numerical value will result in zero:

 ~~~~~~~~~~~{.cpp}
 l.GetTStringValue("a"); => (TString) "false"
 l.GetValue<int>("b"); => 3
 l.GetValue<double>("c"); => 0.0000
 ~~~~~~~~~~~

 Another method allows to test whether a parameter with a given name exists and has
 a specific value (including of the right type):

 ~~~~~~~~~~~{.cpp}
 l.IsValue("a",false); => (bool) true
 l.IsValue("b",3); => (bool) false
 l.IsValue("b",3.14); => (bool) true
 ~~~~~~~~~~~

 ###64-bit parameters###
 In addition to the four types handled by KVNamedParameter, KVNameValueList adds the possibility to
 store (unsigned) 64-bit integers using SetValue64bit():

 ~~~~~~~~~~~{.cpp}
 l.SetValue64bit("A64", 1234567890987654321);

 l.Print();

 KVNameValueList::MyList : A list of things (0x7f405e51e010)
 <a=false>
 <b=3.14>
 <c=sqrt>
 <A64_hi=287445236>
 <A64_lo=-1318314831>
 ~~~~~~~~~~~

 The 64-bit value is stored as 2 32-bit integers with suffixes "hi" and "lo". Dedicated methods must be used
 to test for the presence or retrieve the value of a 64-bit parameter:

 ~~~~~~~~~~~{.cpp}
 l.HasValue64bit("A64"); => (bool) true

 l.GetValue64bit("A64"); => (unsigned long long) 1234567890987654321
 ~~~~~~~~~~~

 */

class KVNameValueList : public TNamed {
protected:
   KVHashList fList;//list of KVNamedParameter objects
   Bool_t fIgnoreBool;//do not convert "yes", "false", "on", etc. in TEnv file to boolean

public:

   KVNameValueList();
   KVNameValueList(const Char_t* name, const Char_t* title = "");
   KVNameValueList(const KVNameValueList&);
   virtual ~KVNameValueList();
   KVNameValueList& operator=(const KVNameValueList&);

   bool Set(const KVString&);

   KVHashList* GetList() const;

   virtual void Clear(Option_t* opt = "");
   virtual void ClearSelection(TRegexp&);
   virtual void Print(Option_t* opt = "") const;
   virtual void ls(Option_t* opt = "") const;
   void SetOwner(Bool_t enable = kTRUE);
   Bool_t IsOwner() const;

   void Copy(TObject& nvl) const;
   Int_t Compare(const TObject* nvl) const;

   template<typename value_type>
   void SetValue(const Char_t* name, value_type value)
   {
      //associate a parameter (define by its name) and a value
      //if the parameter is not in the list, it is added
      //if it's in the list replace its value
      KVNamedParameter* par = FindParameter(name);
      par ? par->Set(name, value) : fList.Add(new KVNamedParameter(name, value));
   }
   void SetValue(const KVNamedParameter&);
   void SetValue64bit(const Char_t* name, ULong64_t);
   ULong64_t GetValue64bit(const Char_t* name) const;
   Bool_t HasValue64bit(const Char_t* name) const;

   template <typename value_type>
   void SetValueAt(const Char_t* name, value_type value, Int_t idx)
   {
      //associate a parameter (define by its name) and a value
      //if the parameter is not in the list, it is inserted at
      //location idx in the list.
      //if it's in the list replace its value and move it at
      //location idx in the list.
      KVNamedParameter* par = FindParameter(name);
      if (par) {
         par->Set(value);
         if (fList.IndexOf(par) != idx) {
            fList.GetCollection()->Remove(par);
            fList.AddAt(par, idx);
         }
      }
      else fList.AddAt(new KVNamedParameter(name, value), idx);
   }

   template <typename value_type>
   void SetFirstValue(const Char_t* name, value_type value)
   {
      //associate a parameter (define by its name) and a value
      //if the parameter is not in the list, it is inserted at
      //the begining of the list.
      //if it's in the list replace its value and move it at
      //the begining of the list.
      KVNamedParameter* par = FindParameter(name);
      if (par) {
         par->Set(value);
         if (fList.First() != par) {
            fList.GetCollection()->Remove(par);
            fList.AddFirst(par);
         }
      }
      else fList.AddFirst(new KVNamedParameter(name, value));
   }

   template <typename value_type>
   void SetLastValue(const Char_t* name, value_type value)
   {
      //associate a parameter (define by its name) and a value
      //if the parameter is not in the list, it is inserted at
      //the end of the list.
      //if it's in the list replace its value and move it at
      //the end of the list.
      KVNamedParameter* par = FindParameter(name);
      if (par) {
         par->Set(value);
         if (fList.Last() != par) {
            fList.GetCollection()->Remove(par);
            fList.AddLast(par);
         }
      }
      else fList.AddLast(new KVNamedParameter(name, value));
   }

   template <typename value_type>
   void IncrementValue(const Char_t* name, value_type value)
   {
      //increment a parameter (define by its name) by a value
      //if the parameter is not in the list, it is added
      //if it's in the list increment its value
      KVNamedParameter* par = FindParameter(name);
      par ? par->Add(KVNamedParameter(name, value)) : fList.Add(new KVNamedParameter(name, value));
   }

   template <typename value_type>
   Bool_t IsValue(const Char_t* name, value_type value) const
   {
      // Returns kTRUE if parameter with given name exists and is equal to given value
      KVNamedParameter* par = FindParameter(name);
      if (par) {
         KVNamedParameter tmp(name, value);
         return (*par) == tmp;
      }
      return kFALSE;
   }

   KVNamedParameter* FindParameter(const Char_t* name) const;
   KVNamedParameter* GetParameter(Int_t idx) const;
   void RemoveParameter(const Char_t* name);
   Bool_t HasParameter(const Char_t* name) const;
   template <typename value_type>
   Bool_t HasParameter(const Char_t* name) const
   {
      // Return kTRUE if list has parameter called 'name' and it is of given type

      KVNamedParameter* p = FindParameter(name);
      return (p && p->Is<value_type>());
   }
   Bool_t HasIntParameter(const Char_t* name) const
   {
      return HasParameter<int>(name);
   }
   Bool_t HasBoolParameter(const Char_t* name) const
   {
      return HasParameter<bool>(name);
   }
   Bool_t HasDoubleParameter(const Char_t* name) const
   {
      return HasParameter<double>(name);
   }
   Bool_t HasStringParameter(const Char_t* name) const
   {
      return HasParameter<TString>(name);
   }
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

   template <typename value_type>
   value_type GetValue(const Char_t* name) const
   {
      // return the value of named parameter
      // returns a default value (-1, false or "-1")
      // if no parameter with such a name is present

      KVNamedParameter* par = FindParameter(name);
      return (par ? par->Get<value_type>() : KVNamedParameter::DefaultValue<value_type>());
   }

   Int_t GetIntValue(const Char_t* name) const
   {
      return GetValue<int>(name);
   }
   Bool_t GetBoolValue(const Char_t* name) const
   {
      return GetValue<bool>(name);
   }
   Double_t GetDoubleValue(const Char_t* name) const
   {
      return GetValue<double>(name);
   }
   const Char_t* GetStringValue(const Char_t* name) const
   {
      return GetValue<cstring>(name);
   }
   TString GetTStringValue(const Char_t* name) const;

   template <typename value_type>
   value_type GetValue(Int_t idx) const
   {
      // return the value of parameter at position idx
      // returns a default value (-1, false or "-1") if idx is
      // greater than the number of stored parameters
      KVNamedParameter* par = GetParameter(idx);
      return (par ? par->Get<value_type>() : KVNamedParameter::DefaultValue<value_type>());
   }

   Int_t GetIntValue(Int_t idx) const
   {
      return GetValue<int>(idx);
   }
   Bool_t GetBoolValue(Int_t idx) const
   {
      return GetValue<bool>(idx);
   }
   Double_t GetDoubleValue(Int_t idx) const
   {
      return GetValue<double>(idx);
   }
   const Char_t* GetStringValue(Int_t idx) const
   {
      return GetValue<cstring>(idx);
   }
   TString GetTStringValue(Int_t idx) const;

   virtual void ReadEnvFile(const Char_t* filename);
   virtual KVEnv* ProduceEnvFile();
   virtual void WriteEnvFile(const Char_t* filename);

   void Sort(Bool_t order = kSortAscending)
   {
      // Sort a list of numerical values into ascending order (by default).
      // Use order = kSortDescending to reverse order.
      fList.Sort(order);
   }

   KVNameValueList operator += (const KVNameValueList& nvl);

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
   void Merge(const KVNameValueList&);
   void AddValue(const KVNamedParameter& p);

   ClassDef(KVNameValueList, 4) //A general-purpose list of parameters
};
#endif
