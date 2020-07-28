#include "KVNumberList.h"
#include "KVString.h"
#include <TObjArray.h>
#include <TObjString.h>
#include <Riostream.h>
#include "KVError.h"
#include "TMath.h"
#include "TRandom.h"

ClassImp(KVNumberList)

//____________________________________________________________________________________________//

void KVNumberList::init_numberlist()
{
   //Default initialisation used by ctors
   fMaxNLimits = 0;
   fNLimits = 0;
   fLowerBounds = new TArrayI;
   fUpperBounds = new TArrayI;
   fFirstValue = 99999999;
   fLastValue = -99999999;
   fNValues = 0;
   fName = ClassName();
   fIsParsed = kTRUE;
}

//____________________________________________________________________________________________//

KVNumberList::KVNumberList(): fString()
{
   //Default constructor
   init_numberlist();
}

//____________________________________________________________________________________________//

KVNumberList::KVNumberList(const KVNumberList& orig) : TObject()
{
   //Copy constructor
   init_numberlist();
   orig.Copy(*this);
}

//____________________________________________________________________________________________//

KVNumberList::KVNumberList(const Char_t* list): fString(list)
{
   //Initialise number list using string and parse it to fill limits arrays
   //Any number will only appear once.
   init_numberlist();
   fIsParsed = kFALSE;
}

KVNumberList::KVNumberList(Int_t x)
{
   //Initialise number list using single number

   init_numberlist();
   Add(x);
}

//____________________________________________________________________________________________//
KVNumberList::KVNumberList(Int_t deb, Int_t fin, Int_t pas): fString()
{
   //Initialise number list using numbers from deb to fin with a step=pas
   //i.e. deb,deb+pas,deb+2*pas,...,fin
   init_numberlist();
   SetMinMax(deb, fin, pas);
}

#ifdef WITH_CPP11
KVNumberList::KVNumberList(std::initializer_list<int> L)
{
   // Use an initializer list of integers to set up number list

   init_numberlist();
   for (auto i : L) Add(i);
}
#endif

//____________________________________________________________________________________________//

void KVNumberList::ParseList() const
{
   //PRIVATE METHOD
   //Breaks string containing list down and fills limits arrays accordingly

   if (fIsParsed) return;
   clear();
   ParseAndFindLimits(fString, ' ');
   fIsParsed = kTRUE;
}

//____________________________________________________________________________________________//

void KVNumberList::Clear(Option_t*)
{
   //Empty number list, reset it to initial state.
   clear();
   fString = "";
}

void KVNumberList::clear() const
{
   // private method called by ParseList()
   fNLimits = 0;
   fLowerBounds->Reset();
   fUpperBounds->Reset();
   fFirstValue = 99999999;
   fLastValue = -99999999;
   fNValues = 0;
   fIsParsed = kTRUE;
}

//____________________________________________________________________________________________//

void KVNumberList::SetName(const char* name)
{
   // Change (i.e. set) the name of the KVNumberList.
   // WARNING: if the object is a member of a THashTable or THashList container
   // the container must be Rehash()'ed after SetName(). For example the list
   // of objects in the current directory is a THashList.
   fName = name;
}

//____________________________________________________________________________________________//

void KVNumberList::ParseAndFindLimits(const TString& string, const Char_t delim) const
{
   //Takes a string and breaks it up into its constituent parts,
   //which were initially separated by white space or a comma.
   //Any part which contains "-" will be sent to AddLimits().
   unique_ptr<TObjArray> toks1(string.Tokenize(delim));
   Int_t n_toks = toks1->GetEntries();
   for (int i = 0; i < n_toks; i++) {
      TString tok = ((TObjString*)(*toks1)[i])->GetString();
      KVString kvtok(tok);
      if (tok.Contains(','))
         ParseAndFindLimits(tok, ',');
      else if (tok.Contains(' '))
         ParseAndFindLimits(tok, ' ');
      else if (tok.Contains('-')) {
         AddLimits(tok);
      }
      else if (kvtok.IsDigit()) {
         Int_t val = kvtok.Atoi();
         AddLimits(val, val);
      }
   }
}

//____________________________________________________________________________________________//

void KVNumberList::AddLimits(TString& string) const
{
   //'string' should contain something like "34-59" i.e. two integers separated by a '-'.
   //these two numbers are taken for new lower and upper limits, fNLimits is increased by one,
   //if necessary the arrays' size is increased.
   unique_ptr<TObjArray> toks1(string.Tokenize('-'));
   Int_t n_toks = toks1->GetEntries();
   if (n_toks != 2) return;
   KVString lower(((TObjString*)(*toks1)[0])->GetString());
   KVString upper(((TObjString*)(*toks1)[1])->GetString());
   Int_t ilow = lower.Atoi();
   Int_t iupp = upper.Atoi();
   AddLimits(ilow, iupp);
}

//____________________________________________________________________________________________//

void KVNumberList::AddLimits(Int_t min, Int_t max) const
{
   //The numbers contained in the range [min,max]
   //are added to the list.

   if (++fNLimits > fMaxNLimits) {
      fMaxNLimits += 10;
      fLowerBounds->Set(fMaxNLimits);
      fUpperBounds->Set(fMaxNLimits);
   }
   (*fLowerBounds)[fNLimits - 1] = min;
   (*fUpperBounds)[fNLimits - 1] = max;
   if (min < fFirstValue)
      fFirstValue = min;
   if (max > fLastValue)
      fLastValue = max;
   fNValues += (max - min + 1);
}

//____________________________________________________________________________________________//

void KVNumberList::PrintLimits() const
{
   //Print detailed break-down of list

   //remove duplicate entries, correct fList, then re-parse
   GetList();
   fIsParsed = kFALSE; // force re-parse
   ParseList();

   std::cout << "KVNumberList::" << GetName() << std::endl;
   std::cout << "There are " << fNLimits << " limits in the string : " <<
             fString.Data() << std::endl;
   std::cout << "MIN = ";
   for (int i = 0; i < fNLimits; i++) {
      std::cout << Form("%5d", (*fLowerBounds)[i]);
      if (i < fNLimits - 1)
         std::cout << ",";
   }
   std::cout << std::endl;
   std::cout << "MAX = ";
   for (int i = 0; i < fNLimits; i++) {
      std::cout << Form("%5d", (*fUpperBounds)[i]);
      if (i < fNLimits - 1)
         std::cout << ",";
   }
   std::cout << std::endl;
   std::cout << "First value = " << fFirstValue << "  Last value = " <<
             fLastValue << std::endl;
}

bool KVNumberList::operator==(const KVNumberList& other) const
{
   // Equality test for number lists

   return (TString(GetList()) == TString(other.GetList()));
}

bool KVNumberList::operator!=(const KVNumberList& other) const
{
   // Inequality test for number lists

   return !((*this) == other);
}

//____________________________________________________________________________________________//

void KVNumberList::SetList(const TString& list)
{
   // Replace internal string representation of number list
   // List will be parsed before any subsequent operations
   fString = list;
   fIsParsed = kFALSE;
}

//____________________________________________________________________________________________//

Bool_t KVNumberList::Contains(Int_t val) const
{
   //returns kTRUE if the value 'val' is contained in the ranges defined by the number list
   if (!fIsParsed) ParseList();
   for (int i = 0; i < fNLimits; i++) {
      if (val >= (*fLowerBounds)[i] && val <= (*fUpperBounds)[i])
         return kTRUE;
   }
   return kFALSE;
}

//____________________________________________________________________________________________//

Int_t KVNumberList::First() const
{
   //Returns smallest number included in list
   if (!fIsParsed) ParseList();
   return fFirstValue;
}

//____________________________________________________________________________________________//

Int_t KVNumberList::Last() const
{
   //Returns largest number included in list
   if (!fIsParsed) ParseList();
   return fLastValue;
}

//____________________________________________________________________________________________//

IntArray KVNumberList::GetArray() const
{
   // Creates and fills a sorted array with all the unique
   // values compatible with the ranges defined in the list.
   // (Sorting is in increasing order).

   if (IsEmpty())
      return IntArray();

   if (!fIsParsed) ParseList();

   IntArray temp(fNValues);
   Int_t index = 0;
   for (int i = 0; i < fNLimits; i++) {
      Int_t min = (*fLowerBounds)[i];
      Int_t max = (*fUpperBounds)[i];
      for (int j = min; j <= max; j++) {
         temp[index++] = j;
      }
   }
   //now check for duplicate entries
   //we sort the array in increasing order
   //any duplicate entries will then be adjacent
   IntArrayIter beg = temp.begin();
   std::sort(beg, temp.end());
   IntArrayIter end = std::unique(beg, temp.end());
   Int_t n_uniq = std::distance(beg, end); //number of unique values

   if (n_uniq < fNValues) {
      // duplicates were removed
      // reduce the size of the vector
      temp.resize(n_uniq);
      // we reconstruct a string containing all unique values & reparse it
      fString.Form("%d", (*(beg++)));
      while (beg != end) {
         fString += Form(" %d", (*(beg++)));
      }
      fNValues = n_uniq;
      fIsParsed = kFALSE; // force re-parsing
      ParseList();
   }
   return temp;
}

//____________________________________________________________________________________________//

void KVNumberList::Add(Int_t n)
{
   //Add value 'n' to the list
   TString tmp = (fString != "" ? fString + " " : fString);
   tmp += n;
   SetList(tmp);
}

//____________________________________________________________________________________________//

void KVNumberList::Remove(Int_t n)
{
   //Remove value 'n' from the list
   TString tmp = " " + TString(GetExpandedList()) + " ";
   tmp.ReplaceAll(Form(" %d ", n), " ");
   SetList(tmp);
}

//____________________________________________________________________________________________//

void KVNumberList::Add(const KVNumberList& list)
{
   //Add values in 'list' to this list
   TString tmp = (fString != "" ? fString + " " : fString);
   tmp += list.fString;
   SetList(tmp);
}

//____________________________________________________________________________________________//

void KVNumberList::Remove(const KVNumberList& list)
{
   //Remove values in 'list' from this list
   TString tmp = " " + TString(GetExpandedList()) + " ";
   KVNumberList tampon(list);
   if (!tampon.IsEmpty()) {
      tampon.Begin();
      while (!tampon.End()) tmp.ReplaceAll(Form(" %d ", tampon.Next()), " ");
   }
   SetList(tmp);
}

//____________________________________________________________________________________________//

void KVNumberList::Remove(const Char_t* list)
{
   //Remove values in 'list' to this list
   KVNumberList tmp(list);
   Remove(tmp);
}

//____________________________________________________________________________________________//

void KVNumberList::Add(Int_t n, Int_t* arr)
{
   //Add n values from array arr to the list

   TString tmp = (fString != "" ? fString + " " : fString);
   for (int i = 0; i < n; i++) {
      tmp += arr[i];
      tmp += " ";
   }
   SetList(tmp);
}

void KVNumberList::Add(const IntArray& v)
{
   // Add all values in IntArray (=std::vector<int>) to the list

   TString tmp = (fString != "" ? fString + " " : fString);
   for (IntArrayCIter it = v.begin(); it != v.end(); ++it) {
      tmp += *it;
      tmp += " ";
   }
   SetList(tmp);
}

KVNumberList KVNumberList::operator+(const KVNumberList& other)
{
   // Return sum of this list and the other one

   KVNumberList tmp(*this);
   tmp.Add(other);
   return tmp;
}

//____________________________________________________________________________________________//

void KVNumberList::Remove(Int_t n, Int_t* arr)
{
   //Remove n values from array arr to the list
   TString tmp = " ";
   for (int i = 0; i < n; i++) {
      tmp += arr[i];
      tmp += " ";
   }
   Remove(tmp);
}

//____________________________________________________________________________________________//

void KVNumberList::SetMinMax(Int_t min, Int_t max, Int_t pas)
{
   //Set list with all values from 'min' to 'max'
   TString tmp;
   for (int i = min; i <= max; i += pas) {
      tmp += i;
      tmp += " ";
   }
   SetList(tmp);
}

//____________________________________________________________________________________________//

void KVNumberList::Inter(const KVNumberList& list)
{
   //keep the AND logic operation result between 'list' and this list
   //i.e. keep only numbers which appear in both lists

   KVNumberList tampon(list);
   if (tampon.IsEmpty() || IsEmpty()) {
      SetList("");
   }
   else {
      TString tmp = "";
      tampon.Begin();
      while (!tampon.End()) {
         Int_t n = tampon.Next();
         if (!Contains(n)) tmp += n;
         tmp += " ";
      }
      Begin();
      while (!End()) {
         Int_t n = Next();
         if (!tampon.Contains(n)) tmp += n;
         tmp += " ";
      }
      Remove(tmp);
   }
}

//____________________________________________________________________________________________//

const Char_t* KVNumberList::GetList() const
{
   //Get string containing list. This is most compact representation possible,
   //i.e. all continuous ranges are represented as "minval-maxval"
   //This string will become the new internal representation of the list.
   //Returns empty string if list is empty.

   if (!fIsParsed) ParseList();

   //no numbers in list  ?
   if (!fNValues) {
      fString = "";
      return fString.Data();
   }
   //get array of all values
   IntArray arr = GetArray();
   IntArrayIter it = arr.begin();
   Int_t min, max;
   min = max = *it;   //put min & max = smallest (first) value to start with
   fString = "";
   for (++it; it != arr.end(); ++it) {

      Int_t val = *it;        // loop over values in increasing order

      if (val - * (it - 1) > 1) {
         //cout << "end of continuous range ?" << endl;
         if (min != max) {
            fString += Form("%d-%d ", min, max);
         }
         else {
            fString += Form("%d ", min);
         }
         min = max = val;
      }
      else {
         //cout << "continuous range" << endl;
         max = val;
      }

   }
   if (min != max) {
      fString += Form("%d-%d", min, max);
   }
   else {
      fString += Form("%d", min);
   }
   return fString.Data();
}

//____________________________________________________________________________________________//

const Char_t* KVNumberList::GetExpandedList() const
{
   // Get string containing list. Every unique value contained
   // in the list will be represented.
   // Returns empty string if list is empty.

   if (!fIsParsed) ParseList();

   static TString tmp = "";

   //no numbers in list  ?
   if (!fNValues) {
      fString = "";
      return fString.Data();
   }
   //get array of all values
   IntArray arr = GetArray();
   tmp = "";
   IntArrayIter it = arr.begin();
   for (; it != arr.end() - 1; ++it) {

      Int_t val = *it;        // loop over values in increasing order
      tmp += Form("%d ", val);
   }
   Int_t val = *it;       //last value
   tmp += Form("%d", val);
   return tmp.Data();
}

//____________________________________________________________________________________________//

TString KVNumberList::GetLogical(const Char_t* observable) const
{
   // Get logical expression of 'this' list in the TTree:Draw condition format
   // observable is one of the leaf of the TTree
   // 12-15 20 --> ( 12<=observable&&observable<=15 || observable==20 )
   // return "" if 'this' list  is empty

   if (IsEmpty()) return "";
   GetList();
   TString tmp = fString;
   tmp.ReplaceAll(" ", "||");
   unique_ptr<TObjArray> toks(tmp.Tokenize("||"));
   static TString cond;
   cond = "( ";
   Int_t nt = toks->GetEntries();
   for (Int_t ii = 0; ii < nt; ii += 1) {
      TString line = ((TObjString*)(*toks)[ii])->GetString();
      if (line.Contains("-")) {
         line.ReplaceAll("-", Form("<=%s&&%s<=", observable, observable));
         cond += line;
      }
      else {
         cond += Form("%s==", observable) + line;
      }
      if (ii != nt - 1) cond += "||";
   }
   cond += " )";
   return cond;
}

TString KVNumberList::GetSQL(const Char_t* column) const
{
   // Get equivalent for SQL 'WHERE' clause
   // e.g. 12-15 20 --> column BETWEEN 12 AND 15 OR column=20
   // (column name will be correctly quoted in case it contains spaces)
   // return "" if 'this' list  is empty

   if (IsEmpty()) return "";
   GetList();
   TString qcol = Form("\"%s\"", column);
   KVString tmp = fString;
   static TString cond;
   cond = "";
   tmp.Begin(" ");
   while (!tmp.End()) {
      if (cond != "") cond += " OR ";
      KVString tmp2 = tmp.Next();
      if (tmp2.Contains("-")) {
         cond += (qcol + " BETWEEN ");
         tmp2.Begin("-");
         cond += tmp2.Next();
         cond += " AND ";
         cond += tmp2.Next();
      }
      else {
         cond += (qcol + "=");
         cond += tmp2;
      }
   }
   return cond;
}

//____________________________________________________________________________________________//

KVNumberList& KVNumberList::operator=(const KVNumberList& val)
{
   //Set this number list equal to val

   val.Copy(*this);
   return (*this);
}

void KVNumberList::Copy(TObject& o) const
{
   // Copy content of this number list into 'o'

   ((KVNumberList&)o).Set(fString);
}

//____________________________________________________________________________________________//

Int_t KVNumberList::GetNValues() const
{
   // Returns total number of unique entries in list
   // Note that this calls GetArray() just in order to remove
   // any duplicate entries in the list and make sure fNValues
   // is the number of unique entries.

   GetArray();//will remove any duplicates and correct fNValues
   return fNValues;
}

//____________________________________________________________________________________________//

Int_t KVNumberList::Next() const
{
   //Use this method to iterate over all numbers in the list
   //Initialise first by calling Begin(), then loop until End() returns kTRUE:
   //
   // KVNumberList r("1-10");
   // r.Begin();
   // while( !r.End() ){
   //    Int_t next_val = r.Next();
   //    ...
   // }
   //If list is empty, End() always returns kTRUE and Next() returns -1.

   if (fValues.empty()) {
      Warning("Next", "List is empty. -1 returned.");
      return -1;
   }
   if (fIterIndex >= fEndList) {
      Warning("Next", "Attempt to iterate beyond end of list. -1 returned.");
      return -1;
   }
   Int_t val = *(fIterIndex++);
   return val;
}

//____________________________________________________________________________________________//

void KVNumberList::Begin() const
{
   // Call before using Next(). Resets iterator to beginning of list.
   // If list is empty, End() always returns kTRUE and Next() returns -1.

   fValues = GetArray();
   fIterIndex = fValues.begin();
   fEndList = fValues.end();
}

IntArrayIter KVNumberList::begin()
{
   // Returns a std::iterator over all unique values in the list, ordered from smallest to largest.
   // Allows use in range-based for loops:
   //
   //~~~~~~~~~{.cpp}
   //  KVNumberList pl("1-3,6");
   //  for(auto i : pl) cout << i << " ";
   //
   // //output:
   // //1 2 3 6
   //~~~~~~~~~

   fValues = GetArray();
   fIterIndex = fValues.begin();
   fEndList = fValues.end();
   return fIterIndex;
}

IntArrayIter KVNumberList::end()
{
   // Returns a std::iterator to element after end of list.
   // Allows use in range-based for loops.
   //
   // \sa begin()
   return fEndList;
}

//____________________________________________________________________________________________//

Int_t KVNumberList::At(Int_t index) const
{
   // Returns value of number in list at position 'index' (index = 0, 1, .., GetNValues()-1)
   // We check that 'index' is neither too big nor too small (otherwise we return -1).
   // WARNING: the list is first sorted into ascending order (and any duplicate entries are
   // removed), so the index does not necessarily correspond to the order in which numbers
   // are added to the list.

   IntArray arr = GetArray();
   Int_t n = -1;
   try {
      n = arr.at(index);
   }
   catch (std::exception& e) {
      Warning(KV__ERROR(At), "Index out of bounds. -1 returned.");
   }
   return n;
}

//____________________________________________________________________________________________//

Int_t KVNumberList::operator[](Int_t index) const
{
   // Returns value of number in list at position 'index' (index = 0, 1, .., GetNValues()-1)
   // We check that 'index' is neither too big nor too small (otherwise we return -1).
   // WARNING: the list is first sorted into ascending order (and any duplicate entries are
   // removed), so the index does not necessarily correspond to the order in which numbers
   // are added to the list.

   return At(index);
}

//____________________________________________________________________________________________//

const Char_t* KVNumberList::AsString(Int_t maxlen) const
{
   //Get string containing list. This is most compact representation possible,
   //i.e. all continuous ranges are represented as "minval-maxval".
   //If maxlen>0, and if length of resulting string is longer than maxlen,
   //we truncate the list to show only the beginning and the end of the list,
   //with "..." in between, i.e. "6000-...-8910". Note that the minimum size of
   //the resulting string is 5 i.e. "6...0".
   //Returns empty string if list is empty.

   fTMPSTR = GetList();
   if (maxlen) {
      maxlen = TMath::Max(maxlen, 5);
      if (fTMPSTR.Length() > maxlen) {
         Int_t len_left = maxlen - 3; // 3 for "..."
         Int_t len_start = len_left / 2;
         Int_t len_end = len_left - len_start;
         TString tmp2 = fTMPSTR(0, len_start);
         tmp2 += "...";
         tmp2 += fTMPSTR(fTMPSTR.Length() - len_end, fTMPSTR.Length() - 1);
         fTMPSTR = tmp2;
      }
   }
   return fTMPSTR.Data();
}

const Char_t* KVNumberList::AsHumanReadableString() const
{
   KVString tmp = AsString();
   int nws = tmp.GetNValues(" ");
   fTMPSTR = "";

   if (nws == 1) fTMPSTR = tmp;
   else {
      tmp.Begin(" ");
      for (int ii = 0; ii < nws - 1; ii++) fTMPSTR += Form("%s%s", tmp.Next().Data(), (ii == nws - 2) ? "" : ", ");
      fTMPSTR += Form(" and %s", tmp.Next().Data());
   }
   fTMPSTR.ReplaceAll("-", " to ");
   return fTMPSTR.Data();
}


Int_t KVNumberList::GetRandom() const
{
   // Draw number at random from list
   return At(gRandom->Integer(GetEntries()));
}

//____________________________________________________________________________________________//

Bool_t KVNumberList::IsFull(Int_t vinf, Int_t vsup) const
{
   //Return kTRUE if the list is in fact a continuous list of number
   // ie "123-126" or "1,2,3,4" etc ...
   //Use vinf et vsup if you want to test only a sub part
   //of the list by default vinf=vsup=-1 and the whole list is considered
   //in the test
   // ie :
   // for "123-127 129" it will be returned kFALSE
   // for "123-127 129",-1,-1 it will be returned kFALSE
   // for "123-127 129",123,127 it will be returned kTRUE
   // for "123-127 129",-1,127 it will be returned kTRUE
   // for "123-127 129",127,-1 it will be returned kFALSE

   if ((vinf == -1) && (vsup == -1)) {
      Int_t total = Last() - First() + 1;
      return (total == GetNValues());
   }
   else {
      return GetSubList(vinf, vsup).IsFull();
   }
}

//____________________________________________________________________________________________//

KVNumberList KVNumberList::GetComplementaryList() const
{
   //Return the complementary of the list
   // ie for "123-127 129" it will be returned "128"

   KVNumberList nl("");
   if (IsFull()) return nl;
   nl.SetMinMax(this->First(), this->Last());
   nl.Remove(*this);
   return nl;

}
//____________________________________________________________________________________________//

KVNumberList KVNumberList::GetSubList(Int_t vinf, Int_t vsup) const
{
   //Return the sublist of numbers between vinf and vsup
   // i.e. put in the sublist val if vinf <= val <= vsup
   // if vinf=-1, put no lower limit
   // if vsup=-1, put no upper limit
   // if vinf = vsup = -1, just clone the list
   // i.e. "123-135 145-456",130,400 it will be returned "130-135 145-400"

   KVNumberList nl("");
   if (vinf > vsup) return nl;
   if (vinf == -1) vinf = First();
   if (vsup == -1) vsup = Last();
   Begin();
   while (!End()) {
      Int_t val = Next();
      if (val >= vinf && val <= vsup) nl.Add(val);
      else if (val > vsup) return nl;
   }
   return nl;

}

//____________________________________________________________________________________________//

KVNumberList KVNumberList::operator-(const KVNumberList& other)
{
   // Returns difference between 'this' and 'other'
   // i.e. 'this' list with any values in 'other' removed
   KVNumberList tmp(*this);
   tmp.Remove(other);
   return tmp;
}

//____________________________________________________________________________________________//

void KVNumberList::Print(Option_t*) const
{
   PrintLimits();
}

//____________________________________________________________________________________________//

TList* KVNumberList::CutInSubList(Int_t number)
{
   //Create sublist of KVNumberList with "number" values
   //TList object has to be deleted after use by the user
   TList* list = new TList();
   list->SetOwner(kTRUE);

   KVNumberList* nl = 0;
   Begin();
   while (!End()) {
      Int_t vv = Next();
      if (!nl) {
         nl = new KVNumberList();
         list->Add(nl);
      }
      else if (nl->GetNValues() >= number) {
         nl = new KVNumberList();
         list->Add(nl);
      }
      nl->Add(vv);
   }

   return list;

}

/** \example base_kvnumberlist.C
# Examples of use of the KVNumberList class

KVNumberList provides many tools for handling discontinuous ranges of integers.

To execute this function, do:

    $ kaliveda
    kaliveda[0] .L base_kvnumberlist.C+
    kaliveda[1] test_kvnumberlist()

*/
