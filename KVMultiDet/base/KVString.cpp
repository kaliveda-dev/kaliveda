//$Id: KVString.cpp,v 1.15 2008/04/04 13:08:00 franklan Exp $

#include "KVString.h"
#include "Riostream.h"
#include "TObjString.h"
#include "TObjArray.h"
#include <list>
#include <stdlib.h>
#include <ctype.h>
#include <algorithm>

#include "snprintf.h"
#include "TBuffer.h"
#include "TError.h"
#include "Bytes.h"
#include "TClass.h"
#include "TMath.h"
#include <TObject.h>
#include <list>
#include "TRandom.h"

#ifdef R__GLOBALSTL
namespace std {
   using::list;
}
#endif
ClassImp(KVString)


#ifdef __WITHOUT_TSTRING_TOKENIZE
TObjArray* KVString::Tokenize(const TString& delim) const
{
   // This function is used to isolate sequential tokens in a TString.
   // These tokens are separated in the string by at least one of the
   // characters in delim. The returned array contains the tokens
   // as TObjString's. The returned array is the owner of the objects,
   // and must be deleted by the user.

   std::list < Int_t > splitIndex;

   Int_t i, start, nrDiff = 0;
   for (i = 0; i < delim.Length(); i++) {
      start = 0;
      while (start < Length()) {
         Int_t pos = Index(delim(i), start);
         if (pos == kNPOS)
            break;
         splitIndex.push_back(pos);
         start = pos + 1;
      }
      if (start > 0)
         nrDiff++;
   }
   splitIndex.push_back(Length());

   if (nrDiff > 1)
      splitIndex.sort();

   TObjArray* arr = new TObjArray();
   arr->SetOwner();

   start = -1;
   std::list < Int_t >::const_iterator it;
#ifndef R__HPUX
   for (it = splitIndex.begin(); it != splitIndex.end(); it++) {
#else
   for (it = splitIndex.begin();
         it != (std::list < Int_t >::const_iterator) splitIndex.end();
         it++) {
#endif
      Int_t stop = *it;
      if (stop - 1 >= start + 1) {
         TString tok = (*this)(start + 1, stop - start - 1);
         TObjString* objstr = new TObjString(tok);
         arr->Add(objstr);
      }
      start = stop;
   }

   return arr;
}
#endif

#ifdef __WITH_KVSTRING_ISDIGIT
Bool_t KVString::IsDigit() const
{
   // Returns true if all characters in string are digits (0-9) or whitespaces,
   // i.e. "123456" and "123 456" are both valid integer strings.
   // Returns false in case string length is 0 or string contains other
   // characters or only whitespace.

   const char* cp = Data();
   Ssiz_t len = Length();
   if (len == 0) return kFALSE;
   Int_t b = 0, d = 0;
   for (Ssiz_t i = 0; i < len; ++i) {
      if (cp[i] != ' ' && !isdigit(cp[i])) return kFALSE;
      if (cp[i] == ' ') b++;
      if (isdigit(cp[i])) d++;
   }
   if (b && !d)
      return kFALSE;
   return kTRUE;
}
#endif

#ifdef __WITH_KVSTRING_REMOVE
KVString& KVString::Remove(TString::EStripType st, char c)
{
   // Remove char c at begin and/or end of string (like Strip() but
   // modifies directly the string.
   *this = Strip(st, c);
   return *this;
}
#endif

#ifdef __WITH_KVSTRING_ATOI
Int_t KVString::Atoi() const
{
   //Enhanced version of TString::Atoi().
   //Atoi(): with string="123 456", TString::Atoi() gives value=123
   //              KVString::Atoi() gives value=123456

   Int_t end = Index(" ");
   //if no whitespaces in string, just use atoi()
   if (end == -1)
      return atoi(Data());

   Int_t start = 0;
   TString tmp;
   while (end > -1) {
      tmp += (*this)(start, end - start);
      start = end + 1;
      end = Index(" ", start);
   }
   end = Length();
   tmp += (*this)(start, end - start);
   return atoi(tmp.Data());
}
#endif

#ifdef __WITH_KVSTRING_ATOF
Double_t KVString::Atof() const
{
   //Enhanced version of TString::Atof().
   //Atof(): with string="12,34", TString::Atof() gives value=12
   //              KVString::Atof() gives value=12.34

   //look for a comma and some whitespace
   Int_t comma = Index(",");
   Int_t end = Index(" ");
   //if no commas & no whitespace in string, just use atof()
   if (comma == -1 && end == -1)
      return atof(Data());
   TString tmp = *this;
   if (comma > -1) {
      //replace comma with full stop
      tmp.Replace(comma, 1, ".");
   }
   //no whitespace ?
   if (end == -1)
      return atof(tmp.Data());
   //remove whitespace
   Int_t start = 0;
   TString tmp2;
   while (end > -1) {
      tmp2 += tmp(start, end - start);
      start = end + 1;
      end = tmp.Index(" ", start);
   }
   end = tmp.Length();
   tmp2 += tmp(start, end - start);
   return atof(tmp2.Data());
}
#endif

#ifdef __WITH_KVSTRING_ISFLOAT
Bool_t KVString::IsFloat() const
{
   //Returns kTRUE if string contains a floating point or integer number
   // Examples of valid formats are:
   //    64320
   //    64 320
   //    6 4 3 2 0
   //    6.4320     6,4320
   //    6.43e20   6.43E20    6,43e20
   //    6.43e-20  6.43E-20   6,43e-20

   //we first check if we have an integer, in this case, IsDigit() will be true straight away
   if (IsDigit()) return kTRUE;

   TString tmp = *this;
   //now we look for occurrences of '.', ',', e', 'E', '+', '-' and replace each
   //with ' '. if it is a floating point, IsDigit() will then return kTRUE
   Int_t i_dot, i_e, i_plus, i_minus, i_comma;
   i_dot = i_e = i_plus = i_minus = i_comma = -1;

   i_dot = tmp.First('.');
   if (i_dot > -1) tmp.Replace(i_dot, 1, " ", 1);
   i_comma = tmp.First(',');
   if (i_comma > -1) tmp.Replace(i_comma, 1, " ", 1);
   i_e = tmp.First('e');
   if (i_e > -1)
      tmp.Replace(i_e, 1, " ", 1);
   else {
      //try for a capital "E"
      i_e = tmp.First('E');
      if (i_e > -1) tmp.Replace(i_e, 1, " ", 1);
   }
   i_plus = tmp.First('+');
   if (i_plus > -1) tmp.Replace(i_plus, 1, " ", 1);
   i_minus = tmp.First('-');
   if (i_minus > -1) tmp.Replace(i_minus, 1, " ", 1);

   //test if it is now uniquely composed of numbers
   return tmp.IsDigit();
}
#endif

KVString& KVString::Substitute(const Char_t c1, const Char_t c2)
{
   //Replace every occurence of 'c1' with 'c2'
   ReplaceAll(&c1, 1, &c2, 1);
   return (*this);
}

#ifdef __WITH_KVSTRING_ISWHITESPACE
Bool_t KVString::IsWhitespace() const
{
   //Returns kTRUE if string is empty (IsNull()) or if every character = ' '
   return (Length() == CountChar(' '));
}
#endif

Int_t KVString::Sscanf(const Char_t* fmt, ...)
{
   //A more strict implementation than the standard 'sscanf'
   //We require that any characters in the format string which are not format descriptors
   //("%d" etc.) be identical to the characters in this KVString
   //
   //i.e. for a string "file6.root", sscanf("file6.root", "file%d.root", &some_integer) returns 1
   //but so does sscanf("file6.root", "file%danything_you_want_here_i_dont_care", &some_integer).
   //
   //Use this method when you ONLY want "file%d.root" to match "file6.root"
   //
   //Returns number of fields read from string
   //
   //HANDLED FORMAT DESCRIPTORS
   //
   //      %d   -  simple integer descriptor
   //      %3d   -  simple integer descriptor with length - only integers of correct length accepted
   //               (leading characters may be white space, i.e. '  4' for 4 written with '%3d')
   //      %03d   -  simple integer descriptor with length + zero padding - only integers of correct length accepted
   //               (leading characters may be zeroes, i.e. '004' for 4 written with '%03d')
   //      %*   -  just garbage, no argument required, it is not read. we ignore the rest of the string
   //                      and the rest of the format. this is not counted as a field to be read. i.e. Sscanf("%*")
   //                      gives 0 for any string, not because it doesn't match, but because there is nothing to read.

   va_list args;
   va_start(args, fmt);

   Int_t str_index = 0;
   Int_t read_items = 0;
   Int_t fmt_index = 0;
   const char* cp = Data();
   Int_t int_format_length_descriptor = 0;
   Bool_t zero_padding = kFALSE;

   while (fmt[fmt_index] != '\0') {

      if (fmt[fmt_index] == '%') {
         //handle format descriptor
         fmt_index++;
         if (fmt[fmt_index] >= '0' && fmt[fmt_index] <= '9') {
            // case of %nd or %0nd with n=some integer
            zero_padding = (fmt[fmt_index] == '0');
            if (zero_padding) fmt_index++;
            // stick together all figures until 'd' (or some other non-number) is found
            KVString length_of_number = "";
            while (fmt[fmt_index] >= '0' && fmt[fmt_index] <= '9') {
               length_of_number += fmt[fmt_index++];
            }
            int_format_length_descriptor = length_of_number.Atoi();
         }
         if (fmt[fmt_index] == 'd') {
            //read an integer
            KVString dummy;
            if (int_format_length_descriptor) {
               if (zero_padding) {
                  // fixed length integer with leading zeroes
                  // i.e. for %03d, '3' will be represented by '003'
                  // we must read int_format_length_descriptor consecutive integers
                  Int_t figures_read = 0;
                  while (cp[str_index] >= '0' && cp[str_index] <= '9') {
                     dummy += cp[str_index++];
                     figures_read++;
                  }
                  if (figures_read != int_format_length_descriptor) {
                     // number is not correct length, string is not good
                     va_end(args);
                     return 0;
                  }
                  else {
                     // good
                     *(va_arg(args, int*)) = dummy.Atoi();
                     fmt_index++;
                     read_items++;
                  }
               }
               else {
                  // fixed length integer with white-space padding
                  // i.e. for %3d, '3' will be represented by '  3'
                  // we must read int_format_length_descriptor consecutive characters
                  // which are either white-space or numbers, at least the last one must
                  // be a number, and once we start reading numbers we cannot have any more
                  // white space
                  Bool_t no_more_whitespace = kFALSE;
                  while (int_format_length_descriptor) {
                     if (cp[str_index] == '\0') {
                        // tried to read past end of string - no good
                        va_end(args);
                        return 0;
                     }
                     if ((cp[str_index] != ' ') && (cp[str_index] < '0' || cp[str_index] > '9')) {
                        // read a non-whitespace non-number - no good
                        va_end(args);
                        return 0;
                     }
                     if ((cp[str_index] == ' ') && no_more_whitespace) {
                        // read a whitespace after starting to read numbers - no good
                        va_end(args);
                        return 0;
                     }
                     if (cp[str_index] != ' ') {
                        no_more_whitespace = kTRUE;
                        dummy += cp[str_index];
                     }
                     str_index++;
                     int_format_length_descriptor--;
                  }
                  // check we read at least one number
                  if (!no_more_whitespace) {
                     va_end(args);
                     return 0;
                  }
                  // check that next character in string is not a number
                  if (cp[str_index + 1] != '\0' && (cp[str_index + 1] < '0' || cp[str_index + 1] > '9')) {
                     va_end(args);
                     return 0;
                  }
                  // good
                  *(va_arg(args, int*)) = dummy.Atoi();
                  fmt_index++;
                  read_items++;

               }
            }
            else {
               // any length of integer i.e. '%d'
               while (cp[str_index] >= '0' && cp[str_index] <= '9')
                  dummy += cp[str_index++];
               *(va_arg(args, int*)) = dummy.Atoi();
               fmt_index++;
               read_items++;
            }
         }
         else if (fmt[fmt_index] == '*') {
            //rest of string is garbage
            va_end(args);
            return read_items;
         }
      }
      else {
         //check character in format against string
         if (fmt[fmt_index] != cp[str_index]) {
            va_end(args);
            return 0;           //not the same string, return 0
         }
         fmt_index++;
         str_index++;
      }


   }

   va_end(args);

   //if we haven't got to the end of the string, it must not match the format
   if (cp[str_index] != '\0')
      return 0;

   return read_items;
}


Bool_t KVString::Match(TString pattern)
{
   // Check if pattern fit the considered string
   // As in ls shell command the * symbol represents the non discriminant part
   // of the pattern
   // if no * is present in the pattern, the result correspond to TString::Contains method
   // Example KVString st(file_R45.dat);
   // st.Match("*") -> kTRUE
   // st.Match("file") ->kTRUE
   // st.Match("*file*R*") ->kTRUE
   // etc ....
   if (!pattern.Contains("*")) return this->Contains(pattern);
   else if (pattern == "*") return kTRUE;
   else {
      unique_ptr<TObjArray> tok(pattern.Tokenize("*"));
      Int_t n_tok = tok->GetEntries();
      if (!pattern.BeginsWith("*"))
         if (!BeginsWith(((TObjString*)tok->First())->GetString())) {
            return kFALSE;
         }
      if (!pattern.EndsWith("*"))
         if (!EndsWith(((TObjString*)tok->Last())->GetString())) {
            return kFALSE;
         }

      Int_t idx = 0, num = 0;
      for (Int_t ii = 0; ii < n_tok; ii += 1) {
         idx = Index(((TObjString*)tok->At(ii))->GetString(), idx);
         if (idx != -1) {
            num += 1;
            idx++;
         }
         else break;
      }
      if (num == n_tok) return kTRUE;
      else return kFALSE;
   }
}

void KVString::Begin(TString delim) const
{
   // Begin(), Next() and End() can be used to loop over items in
   // a string separated by the delimiter character given as argument
   // to Begin().
   //
   // Example:
   //   KVString str("First | Second | Third");
   //   str.Begin("|");
   //   while( !str.End() ){
   //     cout << str.Next().Data() << endl;
   //   }
   //
   // This will give the following output:
   //
   // First
   //  Second
   //  Third
   //
   // WARNING: If the delimiter character is not contained in the string,
   // calling Next() will return the entire contents of the string, after
   // which End() will return kTRUE. This allows to parse strings containing
   // variable numbers of parameters separated by a delimiter which is only
   // used with 2 or more parameters, i.e.:
   //
   //      "par1|par2|par3" -> "par1" "par2" "par3"
   //      "par1"           -> "par1"

   fEndList = kFALSE;
   fIterIndex = 0;
   if (IsNull()) {
      fEndList = kTRUE;
      kObjArr.reset(nullptr);
   }
   else {
      kObjArr.reset(Tokenize(delim));
      if (!kObjArr->GetEntries()) {
         fEndList = kTRUE;
         kObjArr.reset(nullptr);
      }
   }
}

Bool_t KVString::End() const
{
   // Begin(), Next() and End() can be used to loop over items in
   // a string separated by the delimiter character given as argument
   // to Begin().
   //
   // Example:
   //   KVString str("First | Second | Third");
   //   str.Begin("|");
   //   while( !str.End() ){
   //     cout << str.Next().Data() << endl;
   //   }
   //
   // This will give the following output:
   //
   // First
   //  Second
   //  Third
   return fEndList;
}

KVString KVString::Next(Bool_t strip_whitespace) const
{
   // Begin(), Next() and End() can be used to loop over items in
   // a string separated by the delimiter character given as argument
   // to Begin().
   // If strip_whitespace=kTRUE (default is kFALSE), any leading or
   // trailing whitespace is removed from each item.
   //
   // Example:
   //   KVString str("First | Second | Third");
   //   str.Begin("|");
   //   while( !str.End() ){
   //     cout << str.Next(kTRUE).Data() << endl;
   //   }
   //
   // This will give the following output:
   //
   // First
   // Second
   // Third
   //
   // whereas if Next() is used (i.e. strip_whitespace=kFALSE),
   // this gives:
   //
   // First
   //  Second
   //  Third

   KVString st;
   if (!kObjArr.get()) return st;
   st = ((TObjString*)kObjArr->At(fIterIndex++))->GetString();
   fEndList = (fIterIndex == kObjArr->GetEntries());
   if (fEndList) kObjArr.reset(nullptr);
   if (strip_whitespace) st.Remove(kBoth, ' ');
   return st;
}

void KVString::RBegin(TString delim) const
{
   // RBegin(), RNext() and End() can be used to loop BACKWARDS over items in
   // a string separated by the delimiter character given as argument
   // to RBegin().
   //
   // Example:
   //   KVString str("First | Second | Third");
   //   str.RBegin("|");
   //   while( !str.End() ){
   //     cout << str.RNext().Data() << endl;
   //   }
   //
   // This will give the following output:
   //
   //  Third
   //  Second
   // First
   //
   // WARNING: If the delimiter character is not contained in the string,
   // calling RNext() will return the entire contents of the string, after
   // which End() will return kTRUE. This allows to parse strings containing
   // variable numbers of parameters separated by a delimiter which is only
   // used with 2 or more parameters, i.e.:
   //
   //      "par1|par2|par3" -> "par3" "par2" "par1"
   //      "par1"           -> "par1"

   fEndList = kFALSE;
   fIterIndex = 0;
   if (IsNull()) {
      fEndList = kTRUE;
      kObjArr.reset(nullptr);
   }
   else {
      kObjArr.reset(Tokenize(delim));
      if (!kObjArr->GetEntries()) {
         fEndList = kTRUE;
         kObjArr.reset(nullptr);
      }
      fIterIndex = kObjArr->GetEntries() - 1;
   }
}

KVString KVString::RNext(Bool_t strip_whitespace) const
{
   // RBegin(), RNext() and End() can be used to loop BACKWARDS over items in
   // a string separated by the delimiter character given as argument
   // to RBegin().
   // If strip_whitespace=kTRUE (default is kFALSE), any leading or
   // trailing whitespace is removed from each item.
   //
   // Example:
   //   KVString str("First | Second | Third");
   //   str.RBegin("|");
   //   while( !str.End() ){
   //     cout << str.RNext(kTRUE).Data() << endl;
   //   }
   //
   // This will give the following output:
   //
   // Third
   // Second
   // First
   //
   // whereas if RNext() is used (i.e. strip_whitespace=kFALSE),
   // this gives:
   //
   //  Third
   //  Second
   // First

   KVString st;
   if (!kObjArr.get()) return st;
   st = ((TObjString*)kObjArr->At(fIterIndex--))->GetString();
   fEndList = (fIterIndex == -1);
   if (fEndList) kObjArr.reset(nullptr);
   if (strip_whitespace) st.Remove(kBoth, ' ');
   return st;
}

Int_t KVString::GetNValues(TString delim) const
{
   // Count the number of substrings in this string separated by the given character(s)
   // e.g. given a string "one | two | three", GetNValues("|") returns 3
   // Note that if the 'delim' character is not contained in the string,
   // GetNValues() will return 1 (not 0) - see Begin().

   KVString copy(*this);
   Int_t nn = 0;
   copy.Begin(delim);
   while (!copy.End()) {
      copy.Next();
      nn += 1;
   }
   return nn;
}

std::vector<KVString> KVString::Vectorize(TString delim, Bool_t strip_whitespace)
{
   // Split string into components according to delimiter 'delim'
   // See Begin()/Next()/End()

   std::vector<KVString> v;
   Begin(delim);
   while (!End()) {
      v.push_back(Next(strip_whitespace));
   }
   return v;
}
#ifdef __WITH_KVSTRING_ITOA
//______________________________________________________________________________
Bool_t KVString::IsBin() const
{
   // Returns true if all characters in string are binary digits (0,1).
   // Returns false in case string length is 0 or string contains other
   // characters.

   const char* cp = Data();
   Ssiz_t len = Length();
   if (len == 0) return kFALSE;
   for (Ssiz_t i = 0; i < len; ++i)
      if (cp[i] != '0' && cp[i] != '1')
         return kFALSE;
   return kTRUE;
}

//______________________________________________________________________________
Bool_t KVString::IsOct() const
{
   // Returns true if all characters in string are octal digits (0-7).
   // Returns false in case string length is 0 or string contains other
   // characters.

   const char* cp = Data();
   Ssiz_t len = Length();
   if (len == 0) return kFALSE;
   for (Ssiz_t i = 0; i < len; ++i)
      if (!isdigit(cp[i]) || cp[i] == '8' || cp[i] == '9')
         return kFALSE;
   return kTRUE;
}

//______________________________________________________________________________
Bool_t KVString::IsDec() const
{
   // Returns true if all characters in string are decimal digits (0-9).
   // Returns false in case string length is 0 or string contains other
   // characters.

   const char* cp = Data();
   Ssiz_t len = Length();
   if (len == 0) return kFALSE;
   for (Ssiz_t i = 0; i < len; ++i)
      if (!isdigit(cp[i]))
         return kFALSE;
   return kTRUE;
}

//______________________________________________________________________________
Bool_t KVString::IsInBaseN(Int_t base) const
{
   // Returns true if all characters in string are expressed in the base
   // specified (range=2-36), i.e. {0,1} for base 2, {0-9,a-f,A-F} for base 16,
   // {0-9,a-z,A-Z} for base 36. Returns false in case string length is 0 or
   // string contains other characters.

   if (base < 2 || base > 36) {
      Error("KVString::IsInBaseN", "base %d is not supported. Suppported bases are {2,3,...,36}.", base);
      return kFALSE;
   }
   if (Length() == 0) {
      Error("KVString::IsInBaseN", "input string is empty.") ;
      return kFALSE;
   }
   KVString str = KVString(Data()) ;
   str.ToUpper() ;
   KVString str_ref0 = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ" ;
   KVString str_ref = str_ref0 ;
   str_ref.Remove(base) ;
   Bool_t isInBase = kTRUE ;
   for (Int_t k = 0; k < str.Length(); k++) {
      if (! str_ref.Contains(str[k])) {
         isInBase = kFALSE ;
         break ;
      }
   }
   return (isInBase);
}
#endif
//______________________________________________________________________________
KVString KVString::Itoa(Int_t value, Int_t base)
{
   // Converts an Int_t to a KVString with respect to the base specified (2-36).
   // Thus it is an enhanced version of sprintf (adapted from versions 0.4 of
   // http://www.jb.man.ac.uk/~slowe/cpp/itoa.html).
   // Usage: the following statement produce the same output, namely "1111"
   //   std::cout << KVString::Itoa(15,2) ;
   //   std::cout << KVString::Itoa(0xF,2) ; /// 0x prefix to handle hex
   //   std::cout << KVString::Itoa(017,2) ; /// 0  prefix to handle oct
   // In case of error returns the "!" string.

#ifdef __WITH_KVSTRING_ITOA
   std::string buf;
   // check that the base if valid
   if (base < 2 || base > 36) {
      Error("KVString::Itoa", "base %d is not supported. Suppported bases are {2,3,...,36}.", base) ;
      return (KVString("!"));
   }
   buf.reserve(35); // Pre-allocate enough space (35=kMaxDigits)
   Int_t quotient = value;
   // Translating number to string with base:
   do {
      buf += "0123456789abcdefghijklmnopqrstuvwxyz"[ TMath::Abs(quotient % base) ];
      quotient /= base;
   }
   while (quotient);
   // Append the negative sign
   if (value < 0) buf += '-';
   std::reverse(buf.begin(), buf.end());
   return (KVString(buf.data()));
#else
   return TString::Itoa(value, base);
#endif
}

//______________________________________________________________________________
KVString KVString::UItoa(UInt_t value, Int_t base)
{
   // Converts a UInt_t (twice the range of an Int_t) to a KVString with respect
   // to the base specified (2-36). Thus it is an enhanced version of sprintf
   // (adapted from versions 0.4 of http://www.jb.man.ac.uk/~slowe/cpp/itoa.html).
   // In case of error returns the "!" string.

#ifdef __WITH_KVSTRING_ITOA
   std::string buf;
   // check that the base if valid
   if (base < 2 || base > 36) {
      Error("KVString::UItoa", "base %d is not supported. Suppported bases are {2,3,...,36}.", base);
      return (KVString("!"));
   }
   buf.reserve(35); // Pre-allocate enough space (35=kMaxDigits)
   UInt_t quotient = value;
   // Translating number to string with base:
   do {
      buf += "0123456789abcdefghijklmnopqrstuvwxyz"[ quotient % base ];
      quotient /= base;
   }
   while (quotient);
   std::reverse(buf.begin(), buf.end());
   return (KVString(buf.data()));
#else
   return TString::UItoa(value, base);
#endif
}

//______________________________________________________________________________
KVString KVString::LLtoa(Long64_t value, Int_t base)
{
   // Converts a Long64_t to a KVString with respect to the base specified (2-36).
   // Thus it is an enhanced version of sprintf (adapted from versions 0.4 of
   // http://www.jb.man.ac.uk/~slowe/cpp/itoa.html).
   // In case of error returns the "!" string.

#ifdef __WITH_KVSTRING_ITOA
   std::string buf;
   // check that the base if valid
   if (base < 2 || base > 36) {
      Error("KVString::LLtoa", "base %d is not supported. Suppported bases are {2,3,...,36}.", base);
      return (KVString("!"));
   }
   buf.reserve(35); // Pre-allocate enough space (35=kMaxDigits)
   Long64_t quotient = value;
   // Translating number to string with base:
   do {
      buf += "0123456789abcdefghijklmnopqrstuvwxyz"[ TMath::Abs(quotient % base) ];
      quotient /= base;
   }
   while (quotient);
   // Append the negative sign
   if (value < 0) buf += '-';
   std::reverse(buf.begin(), buf.end());
   return (KVString(buf.data()));
#else
   return TString::LLtoa(value, base);
#endif
}

//______________________________________________________________________________
KVString KVString::ULLtoa(ULong64_t value, Int_t base)
{
   // Converts a ULong64_t (twice the range of an Long64_t) to a KVString with
   // respect to the base specified (2-36). Thus it is an enhanced version of
   // sprintf (adapted from versions 0.4 of http://www.jb.man.ac.uk/~slowe/cpp/itoa.html).
   // In case of error returns the "!" string.

#ifdef __WITH_KVSTRING_ITOA
   std::string buf;
   // check that the base if valid
   if (base < 2 || base > 36) {
      Error("KVString::ULLtoa", "base %d is not supported. Suppported bases are {2,3,...,36}.", base);
      return (KVString("!"));
   }
   buf.reserve(35); // Pre-allocate enough space (35=kMaxDigits)
   ULong64_t quotient = value;
   // Translating number to string with base:
   do {
      buf += "0123456789abcdefghijklmnopqrstuvwxyz"[ quotient % base ];
      quotient /= base;
   }
   while (quotient);
   std::reverse(buf.begin(), buf.end());
   return (KVString(buf.data()));
#else
   return TString::ULLtoa(value, base);
#endif
}

//______________________________________________________________________________
KVString KVString::BaseConvert(const KVString& s_in, Int_t base_in, Int_t base_out)
{
   // Converts string from base base_in to base base_out. Supported bases
   // are 2-36. At most 64 bit data can be converted.

#ifdef __WITH_KVSTRING_ITOA
   KVString s_out = "!" ;  // return value in case of issue
   // checking base range
   if (base_in < 2 || base_in > 36 || base_out < 2 || base_out > 36) {
      Error("KVString::BaseConvert", "only bases 2-36 are supported (base_in=%d, base_out=%d).", base_in, base_out);
      return (s_out);
   }
   // cleaning s_in
   KVString s_in_ = s_in;
   Bool_t isSigned = kFALSE;
   if (s_in_[0] == '-') {
      isSigned = kTRUE;
      s_in_.Remove(0, 1);
   }
   if (!isSigned && s_in_[0] == '+') s_in_.Remove(0, 1);  // !isSigned to avoid strings beginning with "-+"
   if (base_in == 16 && s_in_.BeginsWith("0x")) s_in_.Remove(0, 2);  // removing hex prefix if any
   s_in_ = KVString(s_in_.Strip(KVString::kLeading, '0'));  // removing leading zeros (necessary for length comparison below)
   // checking s_in_ is expressed in the mentionned base
   if (!s_in_.IsInBaseN(base_in)) {
      Error("KVString::BaseConvert", "s_in=\"%s\" is not in base %d", s_in.Data(), base_in);
      return (s_out);
   }
   // checking s_in <= 64 bits
   KVString s_max = KVString::ULLtoa(18446744073709551615ULL, base_in);
   if (s_in_.Length() > s_max.Length()) {
      // string comparison (s_in_>s_max) does not take care of length
      Error("KVString::BaseConvert", "s_in=\"%s\" > %s = 2^64-1 in base %d.", s_in.Data(), s_max.Data(), base_in);
      return (s_out);
   }
   else if (s_in_.Length() == s_max.Length()) {
      // if ( s_in_.Length() < s_max.Length() ) everything's fine
      s_in_.ToLower();  // s_max is lower case
      if (s_in_ > s_max) {
         // string comparison
         Error("KVString::BaseConvert", "s_in=\"%s\" > %s = 2^64-1 in base %d.", s_in.Data(), s_max.Data(), base_in);
         return (s_out);
      }
   }

   // computing s_out
   ULong64_t i = ULong64_t(strtoull(s_in.Data(), 0, base_in));
   s_out = KVString::ULLtoa(i, base_out);
   if (isSigned) s_out.Prepend("-");
   return (s_out);
#else
   return TString::BaseConvert(s_in, base_in, base_out);
#endif
}

void KVString::RemoveAllExtraWhiteSpace()
{
   // Remove any superfluous whitespace (or tabs or newlines) from this string (modifies string)
   // i.e. transform "   Mary               Had\tA   Little \n          Laaaaaaaaaaaaaaaaaamb"
   // into "Mary Had A Little Lamb"

   Begin(" \n\t");
   KVString tmp;
   while (!End()) {
      if (tmp.Length()) tmp += " ";
      tmp += Next();
   }
   *this = tmp;
}

KVString KVString::StripAllExtraWhiteSpace() const
{
   // Remove any superfluous whitespace (or tabs or newlines) from string (does not modify string)
   // i.e. transform "   Mary               Had\tA   Little \n          Laaaaaaaaaaaaaaaaaamb"
   // into "Mary Had A Little Lamb"

   KVString tmp = *this;
   KVString tmp2;
   tmp.Begin(" \n\t");
   while (!tmp.End()) {
      if (tmp2.Length()) tmp2 += " ";
      tmp2 += tmp.Next();
   }
   return tmp2;
}

KVString& KVString::FindCommonCharacters(const TCollection* list, const char bug)
{
   // list is a collection of objects with names
   // this method generates a string containing all characters which appear
   // in every name in the list, the others are replaced by the 'bug' character.
   //
   // example:
   //   list contains a set of TNamed with names:
   //        run_0001.root
   //        run_0002.root
   //             ...
   //        run_0099.root
   //
   // then toto.FindCommonCharacters(list) will produce toto="run_00*.root"

   KVString tmp;
   TIter next(list);
   TObject* o;
   // find differences
   while ((o = next())) {
      if (tmp == "") {
         tmp = o->GetName();
         continue;
      }
      int tmplen = tmp.Length();
      KVString tmp2 = o->GetName();
      int tmp2len = tmp2.Length();
      int len = TMath::Min(tmplen, tmp2len);
      for (int i = 0; i < len; i++) {
         if (tmp[i] != tmp2[i]) tmp[i] = bug;
      }
      if (tmp2len > tmplen) {
         tmp.Append(bug, tmp2len - tmplen);
      }
   }
   // replace multiple occurences of bug
   int tmplen = tmp.Length();
   *this = "";
   bool do_bug = false;
   for (int i = 0; i < tmplen; i++) {
      if (do_bug) {
         if (tmp[i] == bug) continue;
         else do_bug = false;
      }
      else if (tmp[i] == bug) {
         do_bug = true;
      }
      Append(tmp[i]);
   }

   return *this;
}


KVString& KVString::FindCommonTitleCharacters(const TCollection* list, const char bug)
{
   // list is a collection of objects with titles
   // this method generates a string containing all characters which appear
   // in every title in the list, the others are replaced by the 'bug' character.
   //
   // example:
   //   list contains a set of TNamed with titles:
   //        run_0001.root
   //        run_0002.root
   //             ...
   //        run_0099.root
   //
   // then toto.FindCommonCharacters(list) will produce toto="run_00*.root"

   KVString tmp;
   TIter next(list);
   TObject* o;
   // find differences
   while ((o = next())) {
      if (tmp == "") {
         tmp = o->GetTitle();
         continue;
      }
      int tmplen = tmp.Length();
      KVString tmp2 = o->GetTitle();
      int tmp2len = tmp2.Length();
      int len = TMath::Min(tmplen, tmp2len);
      for (int i = 0; i < len; i++) {
         if (tmp[i] != tmp2[i]) tmp[i] = bug;
      }
      if (tmp2len > tmplen) {
         tmp.Append(bug, tmp2len - tmplen);
      }
   }
   // replace multiple occurences of bug
   int tmplen = tmp.Length();
   *this = "";
   bool do_bug = false;
   for (int i = 0; i < tmplen; i++) {
      if (do_bug) {
         if (tmp[i] == bug) continue;
         else do_bug = false;
      }
      else if (tmp[i] == bug) {
         do_bug = true;
      }
      Append(tmp[i]);
   }

   return *this;
}

void KVString::RandomLetterSequence(Int_t length)
{
   // Generate a random sequence of upper- and lower-case letters
   // of given length

   Clear();
   for (Int_t i = 0; i < length; ++i) {
      UInt_t p = gRandom->Integer(52);
      if (p < 26) Append((char)p + 'A');
      else Append((char)(p - 26) + 'a');
   }
}

void KVString::Capitalize()
{
   // Change first character of string from lower to upper case

   if ((*this)[0] >= 'a' && (*this)[0] <= 'z')(*this)[0] -= ('a' - 'A');
}


KVString::KVString(Double_t value, Double_t error): TString(""), kObjArr(nullptr), fIterIndex(-1), fEndList(kTRUE)
{
   Double_t y  = value;
   Double_t ey = error;

   TString sy = Format("%1.2e", y);
   TString sey = Format("%1.1e", ey);

   TString sy_dec, sy_exp, sey_dec, sey_exp;
   Double_t y_dec, ey_dec;
   Int_t y_exp, ey_exp;

   //Recup de la valeur y
   unique_ptr<TObjArray> loa_y(sy.Tokenize("e"));

   TIter next_y(loa_y.get());
   TObjString* os_y = 0;
   os_y = (TObjString*)next_y();
   sy_dec = os_y->GetString();
   os_y = (TObjString*)next_y();
   sy_exp = os_y->GetString();

   y_dec = sy_dec.Atof();
   y_exp = sy_exp.Atoi();

   //Recup de la valeur ey
   unique_ptr<TObjArray> loa_ey(sey.Tokenize("e"));

   TIter next_ey(loa_ey.get());
   TObjString* os_ey = 0;

   os_ey = (TObjString*)next_ey();
   sey_dec = os_ey->GetString();

   os_ey = (TObjString*)next_ey();
   sey_exp = os_ey->GetString();

   ey_dec = sey_dec.Atof();
   ey_exp = sey_exp.Atoi();

   Double_t err = ey_dec * TMath::Power(10., ey_exp - y_exp);
   TString s;

   if (!((TString)Format("%1.2g", y_dec)).Contains(".") && err >= 1) {

      if (!((TString)Format("%1.2g", err)).Contains(".")) {
         if (y_exp == ey_exp) s = Format("%1.2g.0(%g.0).10$^{%d}$", y_dec, ey_dec, y_exp);
         else s = Format("%1.3g.0(%g.0).10$^{%d}$", y_dec, err, y_exp);
      }
      else if (((TString)Format("%1.2g", err)) == ((TString)Format("%1.1g", err)) && ((TString)Format("%1.2g", err)).Contains(".")) {
         if (y_exp == ey_exp) s = Format("%1.2g.0(%g0).10$^{%d}$", y_dec, ey_dec, y_exp);
         else s = Format("%1.3g.0(%g0).10$^{%d}$", y_dec, err, y_exp);
      }
      else {
         if (y_exp == ey_exp) s = Format("%1.2g.0(%g).10$^{%d}$", y_dec, ey_dec, y_exp);
         else s = Format("%1.3g.0(%g).10$^{%d}$", y_dec, err, y_exp);
      }
   }
   else if (((TString)Format("%1.3g", y_dec)) == ((TString)Format("%1.2g", y_dec)) && ((TString)Format("%1.2g", y_dec)).Contains(".") && err < 1) {

      if (!((TString)Format("%1.2g", err)).Contains(".")) {
         if (y_exp == ey_exp) s = Format("%1.2g0(%g.0).10$^{%d}$", y_dec, ey_dec, y_exp);
         else s = Format("%1.3g0(%g.0).10$^{%d}$", y_dec, err, y_exp);
      }
      else if (((TString)Format("%1.2g", err)) == ((TString)Format("%1.1g", err)) && ((TString)Format("%1.2g", err)).Contains(".")) {
         if (y_exp == ey_exp) s = Format("%1.2g0(%g0).10$^{%d}$", y_dec, ey_dec, y_exp);
         else s = Format("%1.3g0(%g0).10$^{%d}$", y_dec, err, y_exp);
      }
      else {
         if (y_exp == ey_exp) s = Format("%1.2g0(%g).10$^{%d}$", y_dec, ey_dec, y_exp);
         else s = Format("%1.3g0(%g).10$^{%d}$", y_dec, err, y_exp);
      }
   }
   else if (!((TString)Format("%1.2g", err)).Contains(".")) {
      if (y_exp == ey_exp) s = Format("%1.2g(%g.0).10$^{%d}$", y_dec, ey_dec, y_exp);
      else s = Format("%1.3g(%g.0).10$^{%d}$", y_dec, err, y_exp);
   }
   else if (((TString)Format("%1.2g", err)) == ((TString)Format("%1.1g", err)) && ((TString)Format("%1.2g", err)).Contains(".")) {
      if (y_exp == ey_exp) s = Format("%1.2g(%g0).10$^{%d}$", y_dec, ey_dec, y_exp);
      else s = Format("%1.3g(%g0).10$^{%d}$", y_dec, err, y_exp);
   }
   else {
      if (y_exp == ey_exp) s = Format("%1.2g(%g).10$^{%d}$", y_dec, ey_dec, y_exp);
      else s = Format("%1.3g(%g).10$^{%d}$", y_dec, err, y_exp);;
   }

   s.ReplaceAll(".10$^{0}$", "");
   s.ReplaceAll("0)", ")");

   Form("%s", s.Data());
}
