//Created by KVClassFactory on Tue Jun 29 14:44:22 2010
//Author: bonnet

#include "KVClassFactory.h"
#include "KVNameValueList.h"
#include "Riostream.h"
#include <KVEnv.h>
#include <TROOT.h>

using namespace std;

ClassImp(KVNameValueList)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVNameValueList</h2>
<h4>A general-purpose list of parameters (name and associated value)</h4>
<!-- */
// --> END_HTML
//Cette classe permet de gerer des listes de parametres avec pour chaque une valeur
//associee
//L'objet contient une liste (KVHashList) rempli de KVNamedParameter
//permettant de faire le lien entre un nom et une valeur
//Les objets de la liste lui appartiennent par defaut
//
//Fonctionnement:
//--------------
//Chaque parametre defini par son nom ne peut etre present qu une fois
//les valeurs peuvent etre passer et recuperer sous trois format Double_t, Int_t et Char_t*
//
//Les parametres sont ajoutes automatiquement dans la liste et leur valeur changee
//par les memes methodes SetValue
//
//Il est possible de connaitre :
// - l'index d'un parametre dans la liste -> GetNameIndex()
// - sa presence ou non -> HasParameter()
// - le nom d'un parametre pour un index donne -> GetNameAt()
//Il est possible de retirer un parametre : RemoveParameter()
//
////////////////////////////////////////////////////////////////////////////////

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
   fList.SetOwner(kTRUE);
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

//______________________________________________
void KVNameValueList::SetValue_str(const Char_t* name, const Char_t* value)
{
   //associate a parameter (defined by its name) and a value
   //if the parameter is not in the list, it is added
   //if it's in the list replace its value

   KVNamedParameter* par = FindParameter(name);
   par ? par->Set(value) : fList.Add(new KVNamedParameter(name, value));
}

//______________________________________________
void KVNameValueList::SetValue_int(const Char_t* name, Int_t value)
{
   //associate a parameter (define by its name) and a value
   //if the parameter is not in the list, it is added
   //if it's in the list replace its value
   KVNamedParameter* par = FindParameter(name);
   par ? par->Set(value) : fList.Add(new KVNamedParameter(name, value));
}

void KVNameValueList::SetValue_bool(const Char_t* name, Bool_t value)
{
   //associate a parameter (define by its name) and a value
   //if the parameter is not in the list, it is added
   //if it's in the list replace its value
   KVNamedParameter* par = FindParameter(name);
   par ? par->Set(value) : fList.Add(new KVNamedParameter(name, value));
}

//______________________________________________
void KVNameValueList::SetValue_flt(const Char_t* name, Double_t value)
{
   //associate a parameter (define by its name) and a value
   //if the parameter is not in the list, it is added
   //if it's in the list replace its value
   KVNamedParameter* par = FindParameter(name);
   par ? par->Set(value) : fList.Add(new KVNamedParameter(name, value));
}

//______________________________________________
Double_t KVNameValueList::IncValue_flt(const Char_t* name, Double_t value)
{
   //increment a parameter (define by its name) by a value
   //if the parameter is not in the list, it is added
   //if it's in the list increment its value
   //the new value of the parameter is returned
   KVNamedParameter* par = FindParameter(name);
   par ? par->Set(value += par->GetDouble()) : fList.Add(new KVNamedParameter(name, value));
   return value;
}

//______________________________________________
Int_t KVNameValueList::IncValue_int(const Char_t* name, Int_t value)
{
   //increment a parameter (define by its name) by a value
   //if the parameter is not in the list, it is added
   //if it's in the list increment its value
   //the new value of the parameter is returned
   KVNamedParameter* par = FindParameter(name);
   par ? par->Set(value += par->GetInt()) : fList.Add(new KVNamedParameter(name, value));
   return value;
}

//______________________________________________
void KVNameValueList::SetValue(const Char_t* name, const Char_t* value)
{
   //associate a parameter (defined by its name) and a value
   //if the parameter is not in the list, it is added
   //if it's in the list replace its value

   SetValue_str(name, value);
}

//______________________________________________
void KVNameValueList::SetValue(const Char_t* name, Int_t value)
{
   //associate a parameter (define by its name) and a value
   //if the parameter is not in the list, it is added
   //if it's in the list replace its value
   SetValue_int(name, value);
}

void KVNameValueList::SetValue(const Char_t* name, Bool_t value)
{
   //associate a parameter (define by its name) and a value
   //if the parameter is not in the list, it is added
   //if it's in the list replace its value
   SetValue_bool(name, value);
}

//______________________________________________
void KVNameValueList::SetValue(const Char_t* name, Double_t value)
{
   //associate a parameter (define by its name) and a value
   //if the parameter is not in the list, it is added
   //if it's in the list replace its value
   SetValue_flt(name, value);
}

void KVNameValueList::SetValue(const KVNamedParameter& p)
{
   // add (or replace) a parameter with the same name, type & value as 'p'

   KVNamedParameter* par = FindParameter(p.GetName());
   par ? par->Set(p) : fList.Add(new KVNamedParameter(p));

}

//______________________________________________
void KVNameValueList::SetValueAt(const Char_t* name, Double_t value, Int_t idx)
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
   } else fList.AddAt(new KVNamedParameter(name, value), idx);
}

//______________________________________________
void KVNameValueList::SetFirstValue(const Char_t* name, Double_t value)
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
   } else fList.AddFirst(new KVNamedParameter(name, value));
}

//______________________________________________
void KVNameValueList::SetLastValue(const Char_t* name, Double_t value)
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
   } else fList.AddLast(new KVNamedParameter(name, value));
}

//______________________________________________
Double_t KVNameValueList::IncrementValue(const Char_t* name, Double_t value)
{
   return IncValue_flt(name, value);
}

//______________________________________________
Int_t KVNameValueList::IncrementValue(const Char_t* name, Int_t value)
{
   return IncValue_int(name, value);
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
   return (FindParameter(name) != NULL);
}

Bool_t KVNameValueList::HasIntParameter(const Char_t* name) const
{
   // Return kTRUE if list has parameter called 'name' and it is an integer value

   KVNamedParameter* p = FindParameter(name);
   return (p && p->IsInt());
}

Bool_t KVNameValueList::HasBoolParameter(const Char_t* name) const
{
   // Return kTRUE if list has parameter called 'name' and it is a boolean value

   KVNamedParameter* p = FindParameter(name);
   return (p && p->IsBool());
}

Bool_t KVNameValueList::HasDoubleParameter(const Char_t* name) const
{
   // Return kTRUE if list has parameter called 'name' and it is a double/floating-point value

   KVNamedParameter* p = FindParameter(name);
   return (p && p->IsDouble());
}

Bool_t KVNameValueList::HasStringParameter(const Char_t* name) const
{
   // Return kTRUE if list has parameter called 'name' and it is a string value

   KVNamedParameter* p = FindParameter(name);
   return (p && p->IsString());
}

//______________________________________________
Int_t KVNameValueList::GetNameIndex(const Char_t* name)
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
Int_t KVNameValueList::GetIntValue(const Char_t* name) const
{
   //return the value in integer format
   //for a parameter using its name
   //return -1 if no parameter with such name are present

   KVNamedParameter* par = FindParameter(name);
   return (par ? par->GetInt() : -1);
}

Bool_t KVNameValueList::GetBoolValue(const Char_t* name) const
{
   //return the value in boolean format
   //for a parameter using its name
   //return false if no parameter with such name is present

   KVNamedParameter* par = FindParameter(name);
   return (par ? par->GetBool() : kFALSE);
}

//______________________________________________
Double_t KVNameValueList::GetDoubleValue(const Char_t* name) const
{
   //return the value in double format
   //for a parameter using its name
   //return -1 if no parameter with such name are present

   KVNamedParameter* par = FindParameter(name);
   return (par ? par->GetDouble() : -1.0);
}

//______________________________________________
const Char_t* KVNameValueList::GetStringValue(const Char_t* name) const
{
   //return the value in string format
   //for a parameter using its name
   //return string "-1" if no parameter with such name are present

   KVNamedParameter* par = FindParameter(name);
   if (!par) {
      Error("GetStringValue(const Char_t*)", "\"%s\" does not correspond to an existing parameter, default value \"-1\" is returned", name);
      return Form("%d", -1);
   }
   return par->GetString();
}

//______________________________________________
const TString& KVNameValueList::GetTStringValue(const Char_t* name) const
{
   //return the value in TString format
   //for a parameter using its name
   //return string "-1" if no parameter with such name are present

   KVNamedParameter* par = FindParameter(name);
   static TString tmp("-1");
   if (!par) {
      Error("GetStringValue(const Char_t*)", "\"%s\" does not correspond to an existing parameter, default value \"-1\" is returned", name);
      return tmp;
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
Int_t KVNameValueList::GetIntValue(Int_t idx) const
{
   //return the value in integer format
   //for a parameter using its position
   //return -1 idx is greater than the number of stored parameters
   KVNamedParameter* par = GetParameter(idx);
   return (par ? par->GetInt() : -1);
}

Bool_t KVNameValueList::GetBoolValue(Int_t idx) const
{
   //return the value in bool format
   //for a parameter using its position
   //return false if idx is greater than the number of stored parameters
   KVNamedParameter* par = GetParameter(idx);
   return (par ? par->GetBool() : kFALSE);
}

//______________________________________________
Double_t KVNameValueList::GetDoubleValue(Int_t idx) const
{
   //return the value in double format
   //for a parameter using its position
   //return -1 idx is greater than the number of stored parameters
   KVNamedParameter* par = GetParameter(idx);
   return (par ? par->GetDouble() : -1.0);
}

//______________________________________________
const Char_t* KVNameValueList::GetStringValue(Int_t idx) const
{
   //return the value in string format
   //for a parameter using its position
   //return -1 idx is greater than the number of stored parameters
   if (idx >= GetNpar()) {
      Error("GetStringValue(Int_t)", "index has to be less than %d, \"-1\" is returned\n", GetNpar());
      return Form("%d", -1);
   }
   return GetParameter(idx)->GetString();
}

//______________________________________________
const TString& KVNameValueList::GetTStringValue(Int_t idx) const
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
Bool_t KVNameValueList::IsValue(const Char_t* name, const Char_t* value)
{
   // Returns kTRUE if parameter with given name exists and is equal to given value
   KVNamedParameter* par = FindParameter(name);
   if (par) {
      KVNamedParameter tmp(name, value);
      return (*par) == tmp;
   }
   return kFALSE;
}

Bool_t KVNameValueList::IsValue(const Char_t* name, Int_t value)
{
   // Returns kTRUE if parameter with given name exists and is equal to given value
   KVNamedParameter* par = FindParameter(name);
   if (par) {
      KVNamedParameter tmp(name, value);
      return (*par) == tmp;
   }
   return kFALSE;
}

Bool_t KVNameValueList::IsValue(const Char_t* name, Bool_t value)
{
   // Returns kTRUE if parameter with given name exists and is equal to given value
   KVNamedParameter* par = FindParameter(name);
   if (par) {
      KVNamedParameter tmp(name, value);
      return (*par) == tmp;
   }
   return kFALSE;
}

Bool_t KVNameValueList::IsValue(const Char_t* name, Double_t value)
{
   // Returns kTRUE if parameter with given name exists and is equal to given value
   KVNamedParameter* par = FindParameter(name);
   if (par) {
      KVNamedParameter tmp(name, value);
      return (*par) == tmp;
   }
   return kFALSE;
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


KVNameValueList KVNameValueList::operator += (KVNameValueList& nvl)
{
   TIter it(nvl.GetList());
   KVNamedParameter* par = 0;
   while ((par = (KVNamedParameter*)it())) {

      if (par->IsInt())         SetValue(par->GetName(), par->GetInt());
      else if (par->IsDouble()) SetValue(par->GetName(), par->GetDouble());
      else if (par->IsString()) SetValue(par->GetName(), par->GetString());
      else if (par->IsBool()) SetValue(par->GetName(), par->GetBool());
   }
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










