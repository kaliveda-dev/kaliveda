//Created by KVClassFactory on Wed Nov 30 13:54:07 2011
//Author: John Frankland,,,

#include "KVNamedParameter.h"
#include "KVBase.h"
#include "Riostream.h"
#include "TROOT.h"

using namespace std;

ClassImp(KVNamedParameter)

KVNamedParameter::KVNamedParameter() : TNamed(), fNumber(0.0)
{
   // Default constructor
   ResetBits();
}

KVNamedParameter::~KVNamedParameter()
{
   // Destructor
}

KVNamedParameter::KVNamedParameter(const char* nom)
   : TNamed(nom, ""), fNumber(0.0)
{
   ResetBits();
}

KVNamedParameter::KVNamedParameter(const char* nom, const char* val)
   : TNamed(nom, val), fNumber(0.0)
{
   SetType(kIsString);
}

void KVNamedParameter::Set(const char* name, const char* val)
{
   SetNameTitle(name, val);
   SetType(kIsString);
   fNumber = 0.0;
}

void KVNamedParameter::Set(const char* val)
{
   SetTitle(val);
   SetType(kIsString);
   fNumber = 0.0;
}

KVNamedParameter::KVNamedParameter(const char* nom, Double_t val)
   : TNamed(nom, "Double_t"), fNumber(val)
{
   SetType(kIsDouble);
}

void KVNamedParameter::Set(const char* name, Double_t val)
{
   SetNameTitle(name, "Double_t");
   SetType(kIsDouble);
   fNumber = val;
}

void KVNamedParameter::Set(Double_t val)
{
   SetTitle("Double_t");
   SetType(kIsDouble);
   fNumber = val;
}

KVNamedParameter::KVNamedParameter(const char* nom, Int_t val)
   : TNamed(nom, "Int_t"), fNumber(val)
{
   SetType(kIsInt);
}

KVNamedParameter::KVNamedParameter(const char* nom, Bool_t val)
   : TNamed(nom, "Bool_t"), fNumber(val)
{
   SetType(kIsBool);
}

KVNamedParameter::KVNamedParameter(const char* nom, const KVNamedParameter& p)
   : TNamed(nom, p.GetTitle()), fNumber(p.fNumber)
{
   // Create parameter with given name "nom", and the type & value of "p"
   SetType(p.GetType());
}

void KVNamedParameter::Set(const char* name, Int_t val)
{
   SetNameTitle(name, "Int_t");
   SetType(kIsInt);
   fNumber = val;
}

void KVNamedParameter::Set(const char* name, Bool_t val)
{
   SetNameTitle(name, "Bool_t");
   SetType(kIsBool);
   fNumber = val;
}

void KVNamedParameter::Set(const char* nom, const KVNamedParameter& p)
{
   // Set parameter name "nom" with the type & value of "p"
   SetNameTitle(nom, p.GetTitle());
   fNumber = p.fNumber;
   SetType(p.GetType());
}

void KVNamedParameter::Add(const KVNamedParameter& p)
{
   // Numerical values: Add the numerical value of "p" to this parameter
   // Strings: add string to comma-separated list of values
   // If parameters are not same type, print warning and do nothing

   if (GetType() != p.GetType()) {
      Warning("Add", "Parameters are not same type: this->name=%s (type=%d) other->name=%s (type=%d)",
              GetName(), GetType(), p.GetName(), p.GetType());
      return;
   }
   if (IsString()) {
      TString w = Get<TString>() + ",";
      w += p.Get<TString>();
      Set(w);
   }
   else
      fNumber += p.fNumber;
}

void KVNamedParameter::Set(Int_t val)
{
   SetTitle("Int_t");
   SetType(kIsInt);
   fNumber = val;
}

void KVNamedParameter::Set(Bool_t val)
{
   SetTitle("Bool_t");
   SetType(kIsBool);
   fNumber = val;
}

//void KVNamedParameter::Set(const KVNamedParameter& p)
//{
//   // Set name, type & value of parameter according to type & value of 'p'

//   if (p.IsString()) Set(p.GetName(), p.GetString());
//   else if (p.IsInt()) Set(p.GetName(), p.GetInt());
//   else if (p.IsDouble()) Set(p.GetName(), p.GetDouble());
//   else if (p.IsBool()) Set(p.GetName(), p.GetBool());
//   else Warning("Set(const KVNamedParameter&)", "Unknown type of parameter argument");
//}

void KVNamedParameter::Set(TEnv* e, const TString& p)
{
   // Look for value in TEnv with same name as this parameter, or prefixed with "p."
   // If found, set value according to TEnv

   TString name = p;
   if (name != "") name.Append(".");
   name += GetName();
   if (IsString()) Set(e->GetValue(name, GetString()));
   else if (IsInt()) Set(e->GetValue(name, GetInt()));
   else if (IsDouble()) Set(e->GetValue(name, GetDouble()));
   else if (IsBool()) Set((Bool_t)e->GetValue(name, GetBool()));
}

void KVNamedParameter::Clear(Option_t*)
{
   // Removes the name and any assigned value
   SetNameTitle("", "");
   fNumber = 0.0;
   ResetBits();
}

const Char_t* KVNamedParameter::GetString() const
{
   // Returns value of parameter as a string, whatever the type
   // (integer or floating values are converted to a string,
   // booleans are "true" or "false")

   if (IsString()) return GetTitle();
   static TString convert = "";
   if (IsDouble())
      convert.Form("%lf", fNumber);
   else if (IsBool())
      convert = (GetBool() ? "true" : "false");
   else
      convert.Form("%d", (Int_t)fNumber);
   return convert.Data();
}

TString KVNamedParameter::GetTString() const
{
   // Returns value of parameter as a TString, whatever the type
   // (integer or floating values are converted to a string)

   if (IsString()) return fTitle;
   TString convert;
   if (IsDouble())
      convert.Form("%lf", fNumber);
   else if (IsBool())
      convert = (GetBool() ? "true" : "false");
   else
      convert.Form("%d", (Int_t)fNumber);
   return convert;
}

Double_t KVNamedParameter::GetDouble() const
{
   // returns double if parameter value is of numerical type
   // if string, conversion to floating point is attempted

   if (IsString()) {
      return fTitle.Atof();
   }
   return fNumber;
}

Int_t KVNamedParameter::GetInt() const
{
   // returns integer if parameter value is of numerical type
   // if string, conversion to integer is attempted

   if (IsString()) {
      return fTitle.Atoi();
   }
   return (Int_t)fNumber;
}

Bool_t KVNamedParameter::GetBool() const
{
   // returns boolean if parameter value is of boolean type
   // if string, conversion to integer is attempted

   if (IsString()) {
      return fTitle.Atoi();
   }
   return (Bool_t)fNumber;
}

Bool_t KVNamedParameter::IsEqual(const TObject* obj) const
{
   // Test for equality between two parameters
   // Returns kTRUE if both the name, the type, and the value of the parameters are identical

   if (!obj->InheritsFrom("KVNamedParameter")) return kFALSE;
   KVNamedParameter* _obj = (KVNamedParameter*)obj;
   return ((*this) == (*_obj));
}

Bool_t KVNamedParameter::operator== (const KVNamedParameter& other) const
{
   // Test for equality between two parameters
   // Returns kTRUE if both the name, the type, and the value of the parameters are identical

   if ((other.fName != fName)) return kFALSE;
   return HasSameValueAs(other);
}

Bool_t KVNamedParameter::HasSameValueAs(const KVNamedParameter& other) const
{
   // Returns kTRUF if the two parameters have the same type and the
   // same value (don't care about parameter names)
   if (other.GetType() != GetType()) return kFALSE;
   switch (GetType()) {
      case kIsString:
         if (fTitle == other.fTitle) return kTRUE;
         break;

      case kIsInt:
         if (other.GetInt() == GetInt()) return kTRUE;
         break;

      case kIsDouble:
         return KVBase::AreEqual(other.GetDouble(), GetDouble());
         break;

      case kIsBool:
         if (other.GetBool() == GetBool()) return kTRUE;
         break;

      default:
         return kFALSE;
   }
   return kFALSE;
}

void KVNamedParameter::Print(Option_t*) const
{
   if (IsString()) {
      Info("Print", "Name = %s type = string value = %s", GetName(), GetTitle());
   }
   else
      Info("Print", "Name = %s type = %s value = %s", GetName(), GetTitle(), GetString());
}

void KVNamedParameter::ls(Option_t* option) const
{
   // compact listing of parameter name & value, used by KVNameValueList::Print
   // option controls what is printed:
   //   "" (default) : all parameters
   //   "int" : only integer parameters
   //   "bool" : only boolean parameters
   //   "double" : only double parameters
   //   "string" : only string parameters

   Bool_t can_print = kTRUE;
   if (strcmp(option, "")) {
      TString opt(option);
      opt.ToLower();
      if (opt == "int" && !IsInt()) can_print = kFALSE;
      else if (opt == "bool" && !IsBool()) can_print = kFALSE;
      else if (opt == "double" && !IsDouble()) can_print = kFALSE;
      else if (opt == "string" && !IsString()) can_print = kFALSE;
   }
   TROOT::IndentLevel();
   if (IsString()) {
      if (can_print) cout << "<" << GetName() << "=" << GetTitle() << ">" << endl;
   }
   else {
      switch (GetType()) {
         case kIsInt:
            if (can_print) cout << "<" << GetName() << "=" << GetInt() << ">" << endl;
            break;

         case kIsBool:
            if (can_print) cout << "<" << GetName() << "=" << GetString() << ">" << endl;
            break;

         case kIsDouble:
            if (can_print) cout << "<" << GetName() << "=" << GetDouble() << ">" << endl;
            break;

         default:
            break;
      }
   }
}

Int_t KVNamedParameter::Compare(const TObject* obj) const
{
   // Compares numerical parameters for sorting lists (such as KVNameValueList)
   if (!IsNumber()) return 0;
   const KVNamedParameter* other = dynamic_cast<const KVNamedParameter*>(obj);
   if (!other || !other->IsNumber()) return 0;
   // check for equality
   if ((*other) == (*this)) return 0;
   return ((other->fNumber) > fNumber ? -1 : 1);
}

void KVNamedParameter::WriteToEnv(TEnv* e, const TString& p)
{
   // Write parameter in TEnv, using optional prefix p as "p.[name]"

   TString name = p;
   if (name != "") name.Append(".");
   name += GetName();
   if (IsInt()) e->SetValue(name, GetInt());
   else if (IsDouble()) e->SetValue(name, GetDouble());
   else e->SetValue(name, GetString());
}

const Char_t* KVNamedParameter::GetSQLType() const
{
   // Returns type of parameter for use in SQLite database
   // "INTEGER", "REAL", or "TEXT"

   static TString sql_type;
   switch (GetType()) {
      case kIsString:
         sql_type = "TEXT";
         break;
      case kIsDouble:
         sql_type = "REAL";
         break;
      case kIsInt:
      case kIsBool:
         sql_type = "INTEGER";
   }
   return sql_type.Data();
}
