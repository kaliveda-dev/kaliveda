//Created by KVClassFactory on Wed Nov 30 13:54:07 2011
//Author: John Frankland,,,

#include "KVNamedParameter.h"
#include "KVBase.h"
#include "Riostream.h"
#include "TROOT.h"

using namespace std;

ClassImp(KVNamedParameter)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVNamedParameter</h2>
<h4>A generic parameter with a name and a value</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

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

void KVNamedParameter::Set(const char* name, Int_t val)
{
   SetNameTitle(name, "Int_t");
   SetType(kIsInt);
   fNumber = val;
}

void KVNamedParameter::Set(Int_t val)
{
   SetTitle("Int_t");
   SetType(kIsInt);
   fNumber = val;
}

void KVNamedParameter::Set(const KVNamedParameter& p)
{
   // Set name, type & value of parameter according to type & value of 'p'

   if (p.IsString()) Set(p.GetName(), p.GetString());
   else if (p.IsInt()) Set(p.GetName(), p.GetInt());
   else if (p.IsDouble()) Set(p.GetName(), p.GetDouble());
   Warning("Set(const KVNamedParameter&)", "Unknown type of parameter argument");
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
   // (integer or floating values are converted to a string)

   if (IsString()) return GetTitle();
   static TString convert = "";
   if (IsDouble())
      convert.Form("%lf", fNumber);
   else
      convert.Form("%d", (Int_t)fNumber);
   return convert.Data();
}

const TString& KVNamedParameter::GetTString() const
{
   // Returns value of parameter as a TString, whatever the type
   // (integer or floating values are converted to a string)

   if (IsString()) return fTitle;
   static TString convert = "";
   if (IsDouble())
      convert.Form("%lf", fNumber);
   else
      convert.Form("%d", (Int_t)fNumber);
   return convert;
}

Double_t KVNamedParameter::GetDouble() const
{
   // returns double if parameter value is of numerical type
   // if string, print warning and return zero
   if (IsString()) {
      Warning("GetDouble", "Parameter %s is a string : %s", GetName(), GetTitle());
      return 0.0;
   }
   return fNumber;
}

Int_t KVNamedParameter::GetInt() const
{
   // returns integer if parameter value is of numerical type
   // if string, print warning and return zero
   if (IsString()) {
      Warning("GetInt", "Parameter %s is a string : %s", GetName(), GetTitle());
      return 0;
   }
   return (Int_t)fNumber;
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

   if ((other.fName != fName) || other.GetType() != GetType()) return kFALSE;
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

      default:
         return kFALSE;
   }
   return kFALSE;
}

void KVNamedParameter::Print(Option_t*) const
{
   if (IsString()) {
      Info("Print", "Name = %s type = string value = %s", GetName(), GetTitle());
   } else
      Info("Print", "Name = %s type = %s value = %f", GetName(), GetTitle(), fNumber);
}

void KVNamedParameter::ls(Option_t* option) const
{
   // compact listing of parameter name & value, used by KVNameValueList::Print
   // option controls what is printed:
   //   "" (default) : all parameters
   //   "int" : only integer parameters
   //   "double" : only double parameters
   //   "string" : only string parameters

   Bool_t can_print = kTRUE;
   if (strcmp(option, "")) {
      TString opt(option);
      opt.ToLower();
      if (opt == "int" && !IsInt()) can_print = kFALSE;
      else if (opt == "double" && !IsDouble()) can_print = kFALSE;
      else if (opt == "string" && !IsString()) can_print = kFALSE;
   }
   TROOT::IndentLevel();
   if (IsString()) {
      if (can_print) cout << "<" << GetName() << "=" << GetTitle() << ">" << endl;
   } else {
      switch (GetType()) {
         case kIsInt:
            if (can_print) cout << "<" << GetName() << "=" << GetInt() << ">" << endl;
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
