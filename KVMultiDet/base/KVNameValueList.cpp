//Created by KVClassFactory on Tue Jun 29 14:44:22 2010
//Author: bonnet

#include "KVClassFactory.h"
#include "KVNameValueList.h"
#include "Riostream.h"
#include <KVEnv.h>
#include <TROOT.h>

using namespace std;

ClassImp(KVNameValueList)

//______________________________________________
KVNameValueList::KVNameValueList()
   : fList(), fIgnoreBool(kFALSE)
{
   // Default constructor
   fList.SetOwner(kTRUE);
}

//______________________________________________
KVNameValueList::KVNameValueList(const Char_t* name, const Char_t* title)
   : TNamed(name, title), fList(), fIgnoreBool(kFALSE)
{
   // Ctor with name & title
   //
   // if name contains a comma-separated list of parameter/value pairs,
   // it will be used to initialise the list (no name set)
   fList.SetOwner(kTRUE);

   if (Set(name)) SetName("");
}

//______________________________________________
KVNameValueList::KVNameValueList(const KVNameValueList& NVL) : TNamed()
{
   // Copy constructor
   NVL.Copy(*this);
   fList.SetOwner(kTRUE);
}

//______________________________________________
KVNameValueList::~KVNameValueList()
{
   // Destructor
   fList.Clear();// will delete objects in list if owner
}

KVNameValueList& KVNameValueList::operator=(const KVNameValueList& o)
{
   if (&o != this) o.Copy(*this);
   return (*this);
}

bool KVNameValueList::Set(const KVString& list)
{
   // If list contains a comma-separated list of parameter/value pairs
   //
   //~~~~~~~~~~~~~~~~~~~
   //list = "param1=val1,param2=val2,..."
   //~~~~~~~~~~~~~~~~~~~
   //
   // then use it to initialise the parameters of the list, and return true (any existing parameters will be removed).
   //
   // If list does not contain at least one '=' character, do nothing and return false.

   if (!list.Contains("=")) return false;

   Clear();
   list.Begin(",");
   while (!list.End()) {
      KVString pair = list.Next(kTRUE);
      pair.Begin("=");
      KVString parname = pair.Next(kTRUE);
      KVString parval = pair.Next(kTRUE);
      if (parval.IsDigit()) {
         // integer number
         SetValue(parname, parval.Atoi());
      }
      else if (parval.IsFloat()) {
         // real number
         SetValue(parname, parval.Atof());
      }
      else {
         // string
         SetValue(parname, parval);
      }
   }
   return true;
}

//______________________________________________
KVHashList* KVNameValueList::GetList() const
{
   //return the pointeur of the KVHashList where
   //parameters are stored with their values
   return (KVHashList*)&fList;
}

//______________________________________________
void KVNameValueList::Copy(TObject& nvl) const
{
   // Copy this to the nvl object.
   // Any existing parameters will be destroyed

   TNamed::Copy(nvl);
   KVNameValueList& _obj = (KVNameValueList&)nvl;
   fList.Copy(_obj.fList);
   _obj.fIgnoreBool = fIgnoreBool;
}

//______________________________________________
void KVNameValueList::Clear(Option_t* opt)
{
   //Clear all the stored parameters
   //Deletes the parameter objects if owner & opt!="nodelete"
   fList.Clear(opt);
}

void KVNameValueList::ClearSelection(TRegexp& sel)
{
   // Remove from list all parameters whose name matches the regular expression
   // Examples:
   //  remove all parameters starting with "toto": TRegexp sel("^toto")
   //  remove all parameters with "toto" in name:  TRegexp sel("toto")

   TList toBeRemoved;
   Int_t np1 = GetNpar();
   for (Int_t ii = 0; ii < np1; ii += 1) {
      TString name = GetParameter(ii)->GetName();
      if (name.Contains(sel)) toBeRemoved.Add(new TNamed(name.Data(), ""));
   }
   if (toBeRemoved.GetEntries()) {
      TIter next(&toBeRemoved);
      TNamed* tbr;
      while ((tbr = (TNamed*)next())) RemoveParameter(tbr->GetName());
      toBeRemoved.Delete();
   }
}

//______________________________________________
void KVNameValueList::Print(Option_t* option) const
{
   // Print stored parameters (name, and value)
   // Option can be used to select type of parameters to print:
   //   option = "int", "double", or "string"

   if (!GetNpar()) return;

   TROOT::IndentLevel();
   cout << "KVNameValueList::" << GetName() << " : " << GetTitle() << " (" << this << ")" << endl;
   TROOT::IncreaseDirLevel();
   for (Int_t ii = 0; ii < GetNpar(); ii += 1) {
      GetParameter(ii)->ls(option);
   }
   TROOT::DecreaseDirLevel();
}

void KVNameValueList::ls(Option_t*) const
{
   if (TString(GetName()) != "") cout << GetName();
   else cout << "KVNameValueList";
   cout << " : ";
   for (int i = 0; i < GetNpar(); ++i) {
      cout << GetParameter(i)->GetName() << "=";
      switch (GetParameter(i)->GetType()) {
         case KVNamedParameter::kIsDouble:
            cout << GetParameter(i)->GetDouble();
            break;
         case KVNamedParameter::kIsInt:
            cout << GetParameter(i)->GetInt();
            break;
         case KVNamedParameter::kIsString:
            cout << GetParameter(i)->GetString();
            break;
         case KVNamedParameter::kIsBool:
            cout << boolalpha << GetParameter(i)->GetBool();
            break;
      }
      if (i < GetNpar() - 1) cout << ",";
   }
   cout << endl;
}

//______________________________________________
void KVNameValueList::SetOwner(Bool_t enable)
{
   //set if the KVNameValueList owns its objects or not
   //by default it is owner
   fList.SetOwner(enable);
}

//______________________________________________
Bool_t KVNameValueList::IsOwner() const
{
   //return kTRUE if the list owns its objects
   //kFALSE if not
   return fList.IsOwner();
}

//______________________________________________
Int_t KVNameValueList::Compare(const TObject* obj) const
{
   // Compare the contents of two KVNameValueList
   // Returns the number of same parameters (name and value)

   KVNameValueList* nvl = (KVNameValueList*)obj;
   Int_t neq = 0;
   Int_t np1 = GetNpar();
   Int_t np2 = nvl->GetNpar();
   for (Int_t ii = 0; ii < np1; ii += 1) {
      for (Int_t jj = 0; jj < np2; jj += 1) {

         if (*(GetParameter(ii)) == *(GetParameter(jj)))  neq += 1;
      }
   }
   return neq;

}

void KVNameValueList::SetValue(const KVNamedParameter& p)
{
   // add (or replace) a parameter with the same name, type & value as 'p'

   KVNamedParameter* par = FindParameter(p.GetName());
   par ? par->Set(p.GetName(), p) : fList.Add(new KVNamedParameter(p));

}

void KVNameValueList::SetValue64bit(const Char_t* name, ULong64_t x)
{
   // Store a 64-bit integer in the list as two 32-bit parameters with
   // names 'name_up' and 'name_lo'

   TString parname = name;
   parname += "_hi";
   SetValue(parname, (Int_t)(x >> 32));
   parname = name;
   parname += "_lo";
   SetValue(parname, (Int_t)((x << 32) >> 32));
}

ULong64_t KVNameValueList::GetValue64bit(const Char_t* name) const
{
   // Return a 64-bit integer stored as two 32-bit parameters with
   // names 'name_up' and 'name_lo'

   ULong64_t lo, hi;
   TString parname = name;
   parname += "_lo";
   lo = (UInt_t)GetIntValue(parname);
   parname = name;
   parname += "_hi";
   hi = (UInt_t)GetIntValue(parname);
   ULong64_t x = (ULong64_t)((hi << 32) + lo);
   return x;
}

Bool_t KVNameValueList::HasValue64bit(const Char_t* name) const
{
   // Returns kTRUE if 'name' is stored as a 64-bit value i.e. if
   // integer parameters 'name_lo' and 'name_hi' are defined

   TString parname_hi = name;
   parname_hi += "_hi";
   TString parname_lo = name;
   parname_lo += "_lo";
   return (HasIntParameter(parname_hi) && HasIntParameter(parname_lo));
}

void KVNameValueList::AddValue(const KVNamedParameter& p)
{
   // if a parameter with the same name & type as 'p' exists,
   // add numerical value of p to value of parameter in list,
   // or for strings we add to a comma-separated list of strings.
   // otherwise, add a copy of p to list

   KVNamedParameter* par = FindParameter(p.GetName());
   par ? par->Add(p) : fList.Add(new KVNamedParameter(p));
}

//______________________________________________
KVNamedParameter* KVNameValueList::FindParameter(const Char_t* name) const
{
   //return the parameter object with the asking name
   return (KVNamedParameter*)fList.FindObject(name);
}

KVNamedParameter* KVNameValueList::GetParameter(Int_t idx) const
{
   //return the parameter object with index idx
   return (KVNamedParameter*)fList.At(idx);
}

//______________________________________________
void KVNameValueList::RemoveParameter(const Char_t* name)
{
   //remove parameter from the list,
   //Warning the TNamed object associated is deleted

   KVNamedParameter* par = FindParameter(name);
   if (par) {
      fList.Remove(par);
      delete par;
   }
}

//______________________________________________
Bool_t KVNameValueList::HasParameter(const Char_t* name) const
{
   //Check if there is a parameter with the asked name
   //in the list
   //kTRUE, parameter already present
   //kFALSE, if not
   return (FindParameter(name) != nullptr);
}

//______________________________________________
Int_t KVNameValueList::GetNameIndex(const Char_t* name) const
{
   //return the position in the list of a given parameter
   //using its name
   //return -1 if no parameter with such name are present

   TObject* par = 0;
   Int_t idx = 0;
   TIter next(&fList);
   while ((par = next())) {
      if (!strcmp(par->GetName(), name)) return idx;
      idx++;
   }
   Error("GetNameIndex", "Parameter \"%s\" not found, -1 returned", name);
   return -1;
}

//______________________________________________
const Char_t* KVNameValueList::GetNameAt(Int_t idx) const
{
   //return the name of the parameter store at the idx position
   //in the list
   //if the idx is greater than the number of stored parameters
   //return empty string

   if (idx >= GetNpar()) {
      Error("GetNameAt", "index has to be less than %d, empty string is returned", GetNpar());
      return "";
   }
   return fList.At(idx)->GetName();
}

//______________________________________________
TString KVNameValueList::GetTStringValue(const Char_t* name) const
{
   //return the value in TString format
   //for a parameter using its name
   //return string "-1" if no parameter with such name are present

   KVNamedParameter* par = FindParameter(name);
   if (!par) {
      Error("GetStringValue(const Char_t*)", "\"%s\" does not correspond to an existing parameter, default value \"-1\" is returned", name);
      return "-1";
   }
   return par->GetTString();
}

//______________________________________________
Int_t KVNameValueList::GetNpar() const
{
   //return the number of stored parameters
   return fList.GetEntries();
}

//______________________________________________
TString KVNameValueList::GetTStringValue(Int_t idx) const
{
   //return the value in string format
   //for a parameter using its position
   //return -1 idx is greater than the number of stored parameters
   static TString tmp("-1");
   if (idx >= GetNpar()) {
      Error("GetStringValue(Int_t)", "index has to be less than %d, \"-1\" is returned\n", GetNpar());
      return tmp;
   }
   return GetParameter(idx)->GetTString();
}

void KVNameValueList::ReadEnvFile(const Char_t* filename)
{
   // Read all name-value pairs in the TEnv format file and store in list.
   // Clears any previously stored values.
   //
   // values are read as strings from the TEnv and we use
   // TString::IsDigit, TString::IsFloat to decide whether to store
   // them as integers, floats, or strings.
   // booleans are recognized as: TRUE, FALSE, ON, OFF, YES, NO, OK, NOT
   // (to disable this feature and read such values as strings, call
   // SetIgnoreBool(kTRUE))
   //
   // Special case:
   //   if the parameter name contains the string NumberList
   //   then we store the value string as is, as in this case
   //   it is assumed to be the string representation of a
   //   KVNumberList (easily confused with floating point numbers)

   Clear();
   KVEnv env_file;
   Int_t status = env_file.ReadFile(filename, kEnvAll);
   if (status == -1) {
      Error("ReadEnvFile", "The file %s does not exist", filename);
      return;
   }
   THashList* name_value_list = env_file.GetTable();
   TIter next_nv(name_value_list);
   TEnvRec* nv_pair;
   while ((nv_pair = (TEnvRec*)next_nv())) {
      TString parname(nv_pair->GetName());
      if (parname == "KVNameValueList.Name") SetName(nv_pair->GetValue());
      else if (parname == "KVNameValueList.Title") SetTitle(nv_pair->GetValue());
      else if (parname.Contains("NumberList")) SetValue(parname, nv_pair->GetValue());
      else {
         TString parval(nv_pair->GetValue());
         if (parval.IsDigit()) SetValue(parname, parval.Atoi());
         else if (parval.IsFloat()) SetValue(parname, parval.Atof());
         else {
            TString PARVAL(parval);
            PARVAL.ToUpper();
            if (!fIgnoreBool && (PARVAL == "TRUE" || PARVAL == "FALSE" || PARVAL == "ON" || PARVAL == "OFF"
                                 || PARVAL == "YES" || PARVAL == "NO" || PARVAL == "OK" || PARVAL == "NOT"))
               SetValue(parname, (Bool_t)env_file.GetValue(parname, 0));
            else SetValue(parname, parval);
         }
      }
   }
}

KVEnv* KVNameValueList::ProduceEnvFile()
{
   // Put all name-value pairs in this list as a TEnv format.
   // delete after use
   KVEnv* envfile = new KVEnv();
   envfile->SetValue("KVNameValueList.Name", GetName());
   envfile->SetValue("KVNameValueList.Title", GetTitle());
   WriteToEnv(envfile);
   return envfile;
}

void KVNameValueList::WriteEnvFile(const Char_t* filename)
{
   // Write all name-value pairs in this list as a TEnv format file.
   KVEnv* envfile = ProduceEnvFile();
   envfile->SetRcName(filename);
   envfile->Save();
   delete envfile;
}


KVNameValueList KVNameValueList::operator += (const KVNameValueList& nvl)
{
   TIter it(nvl.GetList());
   KVNamedParameter* par = 0;
   while ((par = (KVNamedParameter*)it())) SetValue(*par);
   return *this;
}

void KVNameValueList::WriteClass(const Char_t* classname, const Char_t* classdesc, const Char_t* base_class)
{
   // Generate a class with member variables and Get/Set methods corresponding
   // to the names and types of the parameters in the list
   // For booleans we use Isxxxx/SetIsxxx

   KVClassFactory cf(classname, classdesc, base_class);
   cf.AddGetSetMethods(*this);
   cf.GenerateCode();
}

void KVNameValueList::SetFromEnv(TEnv* tenv, const TString& prefix)
{
   // Update the values of any parameters in the KVNameValueList which are found
   // in the TEnv, optionally using the given prefix.
   // Example: if KVNameValueList contains a parameter "Legs" and if prefix="Large",
   // then if the TEnv contains a value "Large.Legs", it will be used to update "Legs"

   for (int i = 0; i < GetNpar(); ++i) GetParameter(i)->Set(tenv, prefix);
}


void KVNameValueList::WriteToEnv(TEnv* tenv, const TString& prefix)
{
   // Write the values of all parameters in the KVNameValueList in the TEnv,
   // optionally using the given prefix.

   for (int i = 0; i < GetNpar(); ++i) GetParameter(i)->WriteToEnv(tenv, prefix);
}

void KVNameValueList::Merge(const KVNameValueList& other)
{
   // Merge other list into this one.
   // Any parameters in 'other' which do not exist in this one are added.
   // Any parameters which exist in both have their values summed.

   for (int i = 0; i < other.GetNpar(); ++i) {
      KVNamedParameter* np_other = other.GetParameter(i);
      AddValue(*np_other);
   }
}










