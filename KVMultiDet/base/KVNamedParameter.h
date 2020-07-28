//Created by KVClassFactory on Wed Nov 30 13:54:07 2011
//Author: John Frankland,,,

#ifndef __KVNAMEDPARAMETER_H
#define __KVNAMEDPARAMETER_H

#include "TNamed.h"
#include "TEnv.h"
#include "KVString.h"

typedef const char* cstring;

/**
  \class KVNamedParameter
  \ingroup Core
  \brief A generic named parameter storing values of different types

  The four different types which are handled are: `int`, `double`, `bool`, `TString`:

  ###Setting and retrieving parameter values###

  ~~~~~~~~~~~~~~~~{.cpp}
  KVNamedParameter a("a", 12.34);
  a.Is<double>(); // => true
  a.IsInt(); // => false
  a.IsNumber(); // => true

  KVNamedParameter b("b", true);
  b.Is<bool>(); // => true
  b.Is<TString>(); // => false
  ~~~~~~~~~~~~~~~~

  The same parameter can be reused to store a value with a different type:

  ~~~~~~~~~~~~~~~~{.cpp}
  b.Set("hello");
  b.IsBool(); // => false
  b.IsString(); // => true
  b.Is<std::string>(); // => true
  ~~~~~~~~~~~~~~~~

  To retrieve the value of the parameter, its type must be given. This need not be
  the original type of the parameter, some common sense transformations are
  allowed:

  ~~~~~~~~~~~~~~~~{.cpp}
  b.GetString(); => (const char*) "hello"
  b.Get<std::string>(); => (std::string) "hello"
  b.Get<int>() => (int) 0

  a.Get<double>(); => (double) 12.34
  a.GetString(); => (const char*) "12.34"
  a.GetInt(); => (int) 12

  b.Set("12.34");
  b.GetDouble(); => (double) 12.34
  b.Get<int>() => (int) 12
  ~~~~~~~~~~~~~~~~

  ###Comparing parameters###

  Two types of comparison are possible. The `==` operator or member function IsEqual() test for strict
  equality between two parameters, i.e. they must have the same type, value, **and name**:

  ~~~~~~~~~~~~~~~~{.cpp}
  KVNamedParameter a{"A",12}, b{"A",12};

  cout << (a==b) << endl; => (bool) true

  b.SetName("B");
  cout << b.IsEqual(&a) << endl; => (bool) false
  ~~~~~~~~~~~~~~~~

  On the other hand, HasTheSameValueAs() returns `true` as long as both parameters have the same type and
  value:

  ~~~~~~~~~~~~~~~~{.cpp}
  cout << b.HasTheSameValueAs(a) << endl; => (bool) true
  ~~~~~~~~~~~~~~~~

  ###Miscellaneous###
  The methods WriteToEnv(TEnv*, const TString &) and Set(TEnv*, const TString &) can be used to read/write parameter lists
  from/to ROOT resource files (`.rootrc`-style files).

  Add(const KVNamedParameter &) will sum the values of two numerical parameters or concatenate two string parameters with
  a separating comma.
 */

class KVNamedParameter : public TNamed {

   friend class KVNameValueList;

   enum {
      kIsString = BIT(14),
      kIsDouble = BIT(15),
      kIsInt = BIT(16),
      kIsBool = BIT(17)
   };
   template <typename T>
   struct typecode {};

   enum {
      kBitMask = 0x0003c000
   };
   void ResetBits()
   {
      ResetBit(kBitMask);
   }

   Int_t GetType() const
   {
      // compare this value with the values kIsString, kIsDouble, etc.
      return TestBits(kBitMask);
   }

   void SetType(UInt_t f)
   {
      ResetBits();
      SetBit(f);
   }

protected:
   Double_t fNumber;//used to store numerical (integer or floating-point) values

public:
   KVNamedParameter();
   KVNamedParameter(const char*);
   KVNamedParameter(const char*, const char*);
   KVNamedParameter(const char*, Double_t);
   KVNamedParameter(const char*, Int_t);
   KVNamedParameter(const char*, Bool_t);
   KVNamedParameter(const char*, const KVNamedParameter&);
   virtual ~KVNamedParameter();

   void Set(const char*, const char*);
   void Set(const char*, Double_t);
   void Set(const char*, Int_t);
   void Set(const char*, Bool_t);
   void Set(const char*, const KVNamedParameter&);
   void Set(const char*);
   void Set(Double_t);
   void Set(Int_t);
   void Set(Bool_t);
   //void Set(const KVNamedParameter&);
   void Set(TEnv*, const TString& p = "");

   const Char_t* GetString() const;
   TString GetTString() const;
   Double_t GetDouble() const;
   Int_t GetInt() const;
   Bool_t GetBool() const;

   template <typename T>
   T Get() const
   {
      // compile-time error if one of allowed specializations not used
      T a;
      a.very_unlikely_method_name();
   }
   template <typename T>
   static T DefaultValue();
   virtual void Clear(Option_t* = "");

   template <typename T>
   Bool_t Is() const;
   Bool_t IsString() const
   {
      return GetType() == kIsString;
   }
   Bool_t IsDouble() const
   {
      return GetType() == kIsDouble;
   }
   Bool_t IsInt() const
   {
      return GetType() == kIsInt;
   }
   Bool_t IsBool() const
   {
      return GetType() == kIsBool;
   }
   Bool_t IsNumber() const
   {
      return (IsDouble() || IsInt());
   }

   Bool_t   IsEqual(const TObject* obj) const;
   Bool_t operator== (const KVNamedParameter&) const;
   Bool_t HasSameValueAs(const KVNamedParameter&) const;
   virtual void Print(Option_t* opt = "") const;
   virtual void ls(Option_t* opt = "") const;

   Int_t Compare(const TObject* obj) const;
   void WriteToEnv(TEnv*, const TString& p = "");

   const Char_t* GetSQLType() const;
   void Add(const KVNamedParameter& p);

   ClassDef(KVNamedParameter, 1) //A generic parameter with a name and a value
};

template<> inline int KVNamedParameter::Get<int>() const
{
   return GetInt();
}
template<> inline double KVNamedParameter::Get<double>() const
{
   return GetDouble();
}
template<> inline bool KVNamedParameter::Get<bool>() const
{
   return GetBool();
}
template<> inline KVString KVNamedParameter::Get<KVString>() const
{
   return GetTString();
}
template<> inline TString KVNamedParameter::Get<TString>() const
{
   return GetTString();
}
template<> inline cstring KVNamedParameter::Get<cstring>() const
{
   return GetString();
}
template<> inline std::string KVNamedParameter::Get<std::string>() const
{
   return std::string(GetString());
}
template<> inline int KVNamedParameter::DefaultValue<int>()
{
   return -1;
}
template<> inline double KVNamedParameter::DefaultValue<double>()
{
   return -1.0;
}
template<> inline bool KVNamedParameter::DefaultValue<bool>()
{
   return false;
}
template<> inline TString KVNamedParameter::DefaultValue<TString>()
{
   return "-1";
}
template<> inline KVString KVNamedParameter::DefaultValue<KVString>()
{
   return "-1";
}
template<> inline cstring KVNamedParameter::DefaultValue<cstring>()
{
   return "-1";
}
template<> inline std::string KVNamedParameter::DefaultValue<std::string>()
{
   return "-1";
}
#ifndef __CINT__
template<> struct KVNamedParameter::typecode<int> {
   static const int VALUE = KVNamedParameter::kIsInt;
};
template<> struct KVNamedParameter::typecode<double> {
   static const int VALUE = KVNamedParameter::kIsDouble;
};
template<> struct KVNamedParameter::typecode<bool> {
   static const int VALUE = KVNamedParameter::kIsBool;
};
template<> struct KVNamedParameter::typecode<std::string> {
   static const int VALUE = KVNamedParameter::kIsString;
};
template<> struct KVNamedParameter::typecode<cstring> {
   static const int VALUE = KVNamedParameter::kIsString;
};
template<> struct KVNamedParameter::typecode<KVString> {
   static const int VALUE = KVNamedParameter::kIsString;
};
template<> struct KVNamedParameter::typecode<TString> {
   static const int VALUE = KVNamedParameter::kIsString;
};
#endif
template <typename T>
inline Bool_t KVNamedParameter::Is() const
{
   return GetType() == typecode<T>::VALUE;
}
#endif

