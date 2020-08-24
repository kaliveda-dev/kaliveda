/*
$Id: KVRunListLine.cpp,v 1.6 2006/10/19 14:32:43 franklan Exp $
*/

#include "KVRunListLine.h"
#include "RVersion.h"
#include "KVString.h"
#include "TError.h"
#include <TClass.h>

using namespace std;

ClassImp(KVRunListLine)

KVRunListLine::KVRunListLine()
{
   //Default ctor
   //Initial size of TObjArray (fMaxNFields) is 5. This will be increased if necessary in BreakLineIntoFields().

   fMaxNFields = 5;
   fFields = new TObjArray(fMaxNFields);
   fFields->SetOwner();
   fFieldsSet = kFALSE;
}

KVRunListLine::~KVRunListLine()
{
   fFields->Delete();
   delete fFields;
   fFields = 0;
   fFieldKeys.Clear();
   fIndexList.Clear();
}

//_____________________________________________________________________________________

Int_t KVRunListLine::BreakLineIntoFields(const char _delim)
{
   //Break down "line" into fields separated by the delimiter "_delim". Fields are stored in TObjArray of TObjString s (including empty fields
   //i.e. 2 consecutive delimiters in "line" with nothing between are translated as a field whose content is "". TString::Tokenize() simply skips over such
   //cases, so we can't use it with our field idxing system). Remove leading/trailing white space from each field.
   //Return the total number of fields found (including empty fields).

   Int_t allfields = 0;
   Int_t start = 0;
   TString delim(_delim);
   Int_t end = fLine.Index(delim);
   TObjArray& tca = *fFields;
   while (end > -1) {

      tca[allfields] = new TObjString();

      if (start < end) {
         TString _dummy = fLine(start, end - start);
         KVString dummy(_dummy);

         //remove trailing/leading tabs
         dummy.Remove(TString::kBoth, '\t');
         //remove trailing/leading whitespace
         dummy.Remove(TString::kBoth, ' ');

#if ROOT_VERSION_CODE >= ROOT_VERSION(3,5,0)
         //since ROOT v3.05 SetString(const char *)
         ((TObjString*) tca[allfields])->SetString(dummy.Data());
#else
         //before ROOT v3.05 SetString(char *)
         ((TObjString*) tca[allfields])->SetString((char*) dummy.Data());
#endif

      }
      allfields++;
      //check size of TObjArray fFields - if too many fields are found, double size of array
      if (allfields == fMaxNFields) {
         fMaxNFields *= 2;
         tca.Expand(fMaxNFields);
      }
      start = end + 1;
      end = fLine.Index(delim, end + 1);
      //if last column does not have a delimiter on the right, we take rest of line up to end
      if (end == -1 && start < fLine.Length())
         end = fLine.Length();
   }
   //does this line define the field names ?
   if (IsFieldHeader())
      SetFields();
   return allfields;
}

//_____________________________________________________________________________________

void KVRunListLine::SetFields()
{
   //set field idxes based on field header line in file identified by IsFieldHeader().
   //this method is called automatically by BreakLineIntoFields().
   //we check that at least one index has been defined at the end
   //previous definitions are forgotten

   //clear index list
   fIndexList.Clear();
   //reset flag
   fFieldsSet = kFALSE;

   TIter next_field(fFields);
   TObjString* field = 0;
   Int_t idx = 0;
   while ((field = (TObjString*) next_field())) {

      if (field->GetString().IsAscii() && field->GetString().Length()) {
         //non-null ASCII string ? use it as column header
         fIndexList.SetValue(field->GetString().Data(), idx);
      }
      idx++;

   }

   if (!fIndexList.GetNpar()) {
      //no indices set - something's wrong
      Error(Form("%s::SetFields", IsA()->GetName()), "No indices set");
      return;
   }
   //set flag
   fFieldsSet = kTRUE;
}

//_____________________________________________________________________________________

TObjString* KVRunListLine::GetFieldString(const Char_t* fname) const
{
   //returns a TObjString containing the field in the line corresponding to column title "fname".
   //Check that field indices are set and field is known otherwise return 0

   Int_t idx = GetFieldIndex(fname);
   if (idx < 0)
      return 0;
   TObjArray& tca = *fFields;
   return (TObjString*) tca[idx];
}

//_____________________________________________________________________________________

const Char_t* KVRunListLine::GetField(const Char_t* fname) const
{
   //returns a string containing the field in the line corresponding to column title "fname".
   //returns 0 if field not found

   TObjString* tmp = GetFieldString(fname);
   if (tmp)
      return tmp->String().Data();
   return 0;
}

//_____________________________________________________________________________________

Int_t KVRunListLine::GetIntField(const Char_t* fname)
{
   //Returns the integer translation of the field in the line corresponding to column title "fname".
   //We check the field is present in current line and that all characters in the field are numbers (KVString::IsDigit).
   //If not, 0 is returned.

   TObjString* tmp = GetFieldString(fname);
   if (!tmp)
      return 0;
   KVString kvs(tmp->String());
   if (HasFieldValue(fname) && kvs.IsDigit())
      return kvs.Atoi();
   return 0;
}

//_____________________________________________________________________________________

Float_t KVRunListLine::GetFloatField(const Char_t* fname)
{
   //returns the floating-point translation of the field in the line corresponding to column title "fname".
   //We check that the field is a floating point number (IsFloat=kTRUE)
   //If not, 0 is returned.

   TObjString* tmp = GetFieldString(fname);
   if (!tmp)
      return 0;
   KVString kvs(tmp->String());
   if (IsFloat(fname))
      return kvs.Atof();
   return 0.0;
}

//_____________________________________________________________________________________

void KVRunListLine::Print() const
{
   //Print information on this reader object.
   //If fields have been set, we show which fields have been found.
   //Break down of current line is shown.

   cout << IsA()->GetName() << " object" << endl;
   cout << "Current line : " << endl << fLine << endl;

   if (!FieldsSet())
      return;

   //show all defined fields with current values
   cout << "Field name : current value" << endl;
   cout << "==========================" << endl;

   for (Int_t j = 0; j < fIndexList.GetNpar(); j++) {
      PrintFieldValue(fIndexList.GetParameter(j)->GetName());
   }
}

//_____________________________________________________________________________________

Bool_t KVRunListLine::IsFloat(const Char_t* name)
{
   //Returns kTRUE if field "name" contains a floating point number
   //Examples are:
   //      64320
   //      6.4320
   //      6.43e20   6.43E20
   //      6.43e-20  6.43E-20

   if (HasFieldValue(name)) {   //check field is present

      TObjString* tmp = GetFieldString(name);
      if (!tmp)
         return 0;
      KVString kvs(tmp->String());
      return kvs.IsFloat();
   }
   return kFALSE;
}

//______________________________________________________________________//

Bool_t KVRunListLine::FieldContains(const Char_t* field,
                                    const Char_t* string) const
{
   //Returns true if "field" exists, has been filled, and contains "string" as part or all of its value
   if (!HasFieldValue(field))
      return kFALSE;
   TObjString* tmp = GetFieldString(field);
   if (!tmp)
      return kFALSE;
   KVString kvs(tmp->String());
   return kvs.Contains(string);
}

//_____________________________________________________________________________________

Int_t KVRunListLine::GetTrigger(const Char_t* field_name,
                                const Char_t* fmt)
{
   //Returns value of "Trigger" field, assuming it is written with format "fmt" i.e. if fmt = "M>=%d" (default) we expect "M>=1", "M>=4" etc.
   //The actual field name for the trigger is given as first argument (default value = "Trigger")
   //If trigger field is not present, we return -1 (but no error)
   //If format is not respected, an error message is printed and -1 is returned.

   if (!HasFieldValue(field_name))
      return -1;
   Int_t trigger = -1;
   if (sscanf(GetField(field_name), fmt, &trigger) != 1) {
      Warning(Form("%s::GetTrigger", IsA()->GetName()),
              "trigger field - %s - unreadable", GetField(field_name));
   }
   return trigger;
}

//_____________________________________________________________________________________

void KVRunListLine::SetFieldKeys(Int_t nkeys, const Char_t* key1, ...)
{
   //Set keywords used to identify lines in the file which define the column headings, i.e. the field names.
   //Give the number of keywords (nkeys>=1) and then the list of keywords (at least one).
   //
   //Each and every keyword given as argument here (at least one word must be given) must appear in a line for it to be considered
   //a column heading. Lines identified as such (by IsFieldHeader()) are used to set indices for each column (SetFields()).

   //clear out list of keywords
   fFieldKeys.Clear();

   fFieldKeys.Add(new TObjString(key1));

   Int_t narg = 1;
   va_list ap;
   va_start(ap, key1);
   while (narg < nkeys) {

      fFieldKeys.Add(new TObjString(va_arg(ap, const Char_t*)));
      narg++;
   }
   va_end(ap);
}

//_____________________________________________________________________________________

void KVRunListLine::SetRunKeys(Int_t nkeys, const Char_t* key1, ...)
{
   //Set list of fields which must have values for a line to be considered a "good" run line
   //Give the number of fields (nkeys>=1) and then the list of field names (at least one).

   //clear out list of keywords
   fRunKeys.Clear();

   fRunKeys.Add(new TObjString(key1));

   Int_t narg = 1;
   va_list ap;
   va_start(ap, key1);
   while (narg < nkeys) {

      fRunKeys.Add(new TObjString(va_arg(ap, const Char_t*)));
      narg++;
   }
   va_end(ap);
}

//_____________________________________________________________________________________

Bool_t KVRunListLine::IsFieldHeader()
{
   //Returns kTRUE if and only if current line contains ALL of the keywords defined by SetFieldKeys()

   if (!fFieldKeys.GetSize()) {
      //no keywords set - can't recognize header
      return kFALSE;
   }
   //test all keywords in list - first one not contained in "line" causes us to return kFALSE
   TIter next_kw(&fFieldKeys);
   TObjString* os;
   while ((os = (TObjString*) next_kw())) {
      if (!fLine.Contains(os->GetString())) {
         return kFALSE;
      }
   }
   return kTRUE;
}

//_____________________________________________________________________________________

Bool_t KVRunListLine::GoodRunLine()
{
   //Returns kTRUE if and only if current line contains integer values for ALL the fields defined by SetRunKeys

   if (!fRunKeys.GetSize()) {
      //no fields set
      return kFALSE;
   }
   //test all fields in list - first one without a value in line causes us to return kFALSE
   TIter next_kw(&fRunKeys);
   TObjString* os;
   while ((os = (TObjString*) next_kw())) {
      if (!HasFieldValue(os->GetString())) {
         return kFALSE;
      }
      KVString tmp(GetField(os->GetString()));
      if (!tmp.IsDigit()) {
         return kFALSE;
      }
   }
   return kTRUE;
}
