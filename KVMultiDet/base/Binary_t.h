//$Id: Binary_t.h,v 1.5 2007/11/21 11:23:34 franklan Exp $

#ifndef __BINARY_T__
#define __BINARY_T__

#include "Rtypes.h"
#include "Riostream.h"
#include "TString.h"

class Hexa_t;

template < class T > class Binary_t {
private:
   UChar_t fNBitsRep;           //number of bits used to represent value
   UChar_t fNBits;              //number of bits used to store value = kBitsPerByte*sizeof(T)
   T fVal;                      //decimal used to store value

   TString fDumString;  //!dummy, used by String method
   TString fDumString2;  //!dummy, used by WriteSubValue method

   void init();

public:
   Binary_t();
   Binary_t(const T);
   Binary_t(const Char_t*);
   Binary_t(const Binary_t&);
   virtual ~ Binary_t()
   {
   };
   void Set(const T);
   void Set(const Char_t*);
   T Value() const;
   const Char_t* String(UChar_t nbits = 0);

   Long64_t Max() const;

   void SetBit(UChar_t);
   void SetBit(UChar_t, T);
   void ResetBit(UChar_t);
   Bool_t TestBit(UChar_t);

   void WriteSubvalue(const T, UChar_t msb, UChar_t nbits);
   T Subvalue(UChar_t msb, UChar_t nbits);
   //equals operators
   Binary_t < T >& operator=(const Binary_t < T >&);
   Binary_t < T >& operator=(const T);
   Binary_t < T >& operator=(const Char_t*);
//      Binary_t<T>& operator=(const Hexa_t&);
   //convertors
   operator       Int_t() const
   {
      return (Int_t) Value();
   };
   operator       T() const
   {
      return Value();
   };
   operator       Float_t() const
   {
      return (Float_t) Value();
   };
   operator       Double_t() const
   {
      return (Double_t) Value();
   };
   operator       const char* ()
   {
      return String();
   };

   Hexa_t Hexa() const;

   void SetNBits(UChar_t nbits)
   {
      fNBitsRep = nbits;
   };
   UChar_t GetNBits() const
   {
      return (fNBitsRep);
   };

   //addtion
//      Binary_t<T> operator+(const Binary_t<T>& b1);
//      Binary_t<T> operator+(const T l1);
//      Binary_t<T> operator+(const Char_t* c1);
//      Binary_t<T> operator+(const Hexa_t& h1);
   //or
   Binary_t<T> operator|(const Binary_t<T>& b1);
//      Binary_t operator|(const Long64_t l1);
//      Binary_t operator|(const Char_t* c1);
//      Binary_t operator|(const Hexa_t& h1);
//      //and
   Binary_t<T> operator&(const Binary_t<T>& b1);
//      Binary_t operator&(const Long64_t l1);
//      Binary_t operator&(const Char_t* c1);
//      Binary_t operator&(const Hexa_t& h1);

   Bool_t operator==(const Binary_t < T >&);
   Bool_t operator!=(const Binary_t < T >&);
   Bool_t operator!=(const Char_t*);

   void Print(Option_t* opt = "") const;

   ClassDef(Binary_t, 1)        //a binary number
};

//addtion
// Binary_t operator+(const Long64_t l1, const Binary_t& b2);
// Binary_t operator+(const Char_t* c1, const Binary_t& b2);
//bitwise OR
// Binary_t operator|(const Long64_t l1, const Binary_t& b2);
// Binary_t operator|(const Char_t* c1, const Binary_t& b2);
//      //bitwise AND
// Binary_t operator&(const Long64_t l1, const Binary_t& b2);
// Binary_t operator&(const Char_t* c1, const Binary_t& b2);

#include "Riostream.h"
#include "TMath.h"
#include "Hexa_t.h"
#include "TString.h"

template < class T > void Binary_t < T >::init()
{
   //Default initialisation; set fNBits = kBitsPerByte*sizeof(T)
   fNBits = (UChar_t)(kBitsPerByte * sizeof(T));
}

template < class T > Binary_t < T >::Binary_t()
{
   //Default constructor
   fNBitsRep = 0;
   fVal = 0;
   init();
}

template < class T > Binary_t < T >::Binary_t(const Binary_t < T >& b)
{
   //copy constructor
   init();
   fNBitsRep = b.GetNBits();
   fVal = b.Value();
}

template < class T > Binary_t < T >::Binary_t(const T val)
{
   //Create binary number corresponding to decimal value "val"
   fNBitsRep = 0;
   fVal = val;
   init();
}

template < class T > Binary_t < T >::Binary_t(const Char_t* val)
{
   //Create binary number from string containing binary value
   Set(val);
   fNBitsRep = 0;
   init();
}

template < class T > void Binary_t < T >::Set(const T val)
{
   //Set binary number corresponding to decimal value "val"
   fVal = val;
}

template < class T > T Binary_t < T >::Value() const
{
   //Get unsigned decimal value of binary number
   return (fVal);
}

template < class T > void Binary_t < T >::Set(const Char_t* val)
{
   //Set value from string containing binary number
   //If their are more bits in string than number can hold, we only use
   //the fNBits bits from bit 0 (LSB) to bit fNBits-1

   fVal = 0;
   Int_t nbits = (Int_t) strlen(val);
   Int_t ibit = 0;
   Int_t idx_min = nbits - fNBits;
   idx_min = idx_min * (idx_min > 0);
   for (Int_t i = nbits - 1; i >= idx_min; i--) {
      if (val[i] - '0') {       //if a '1' is present here...
         fVal += (T)(1 << ibit);
      }
      ibit++;
   }
}

template < class T > const Char_t* Binary_t < T >::String(UChar_t nbits)
{
   //Return string containing binary representation of number
   //By default, shortest possible representation is returned.
   //If a default number of bits has been set with SetNBits,
   //or if the number of bits is given as argument, the number will be represented using
   //that many bits. If this is more than are necessary, we pad with leading zeroes.
   //If there are not enough bits to represent the whole of the number, only the bits from
   //0 to (nbits-1) are shown.
   //
   //e.g. if we have Binary_t b="111011" :
   //b.String(8) will give "00111011" (padded with 2 leading zeroes)
   //b.String(3) will give "011" (bits 0 to 2 only, i.e. first 3 bits starting from bit 0, which is LSB)
   UChar_t setbits = 0;
   if (!Value()) {
      fDumString = "0";
      setbits = 1;
   } else {
      fDumString = "";
      T cont = Value();
      while (cont) {
         if (cont % 2)
            fDumString.Prepend('1');
         else
            fDumString.Prepend('0');
         setbits++;
         cont /= 2;
      }
   }
   //work out number of leading zeroes to add (if any)
   //if number is too short for required number of bits, lzos>0 and equals number of zeroes to pad with
   //if number is too long for required number of bits, lzos<0
   Int_t lzos =
      (nbits ? (Int_t) nbits -
       (Int_t) setbits : (GetNBits() ? (Int_t) GetNBits() -
                          (Int_t) setbits : 0));
   if (lzos > 0) {
      //Add 'lzos' leading zeroes
      fDumString.Prepend('0', lzos);
   } else if (lzos < 0) {
      //cut off (-lzos) most significant bits
      fDumString.Remove(0, -lzos);
   }
   return fDumString.Data();
}

template < class T > Binary_t < T >& Binary_t < T >::operator=(const T val)
{
   //Assign a decimal value to the binary number
   Set(val);
   return (*this);
}

template < class T > Binary_t < T >& Binary_t <
T >::operator=(const Char_t* val)
{
   //Assign a binary value (as a string) to the binary number
   Set(val);
   return (*this);
}

// Binary_t& Binary_t<T>::operator=(const Hexa_t& val)
// {
//      //Assign a hexadecimal value to the binary number
//      Set(val.Value());
//      return (*this);
// }

template < class T > Binary_t < T >& Binary_t <
T >::operator=(const Binary_t& val)
{
   //Assign a  value to the binary number
   Set(val.Value());
   return (*this);
}

template <class T> Bool_t Binary_t<T>::operator==(const Binary_t < T >& b2)
{
   // Test equality
   return (fVal == b2.fVal);
}

template <class T> Bool_t Binary_t<T>::operator!=(const Binary_t < T >& b2)
{
   // Test inequality
   return (fVal != b2.fVal);
}

template <class T> Bool_t Binary_t<T>::operator!=(const Char_t* b2)
{
   // Test inequality
   Binary_t<T> Bb2(b2);
   return ((*this) != Bb2);
}

//--------------------------------------------------addition operators
//
// template <class T> Binary_t<T> Binary_t<T>::operator+(const Binary_t<T>& b1)
// {
//      //Addition of two binary numbers
//      return Binary_t(fVal + b1.Value());
// }
//
// // Binary_t Binary_t<T>::operator+(const Hexa_t& h1)
// // {
// //   //Addition of binary and hexa numbers
// //   return Binary_t(fVal + h1.Value());
// // }
//
// template <class T> Binary_t<T> Binary_t<T>::operator+(const T I1)
// {
//      //Addition of two binary numbers
//      return Binary_t(fVal+I1);
// }
//
// // Binary_t operator+(const T l1, const Binary_t& b2)
// // {
// //   //Addition of two binary numbers
// //   return (b2+l1);
// // }
//
// template <class T> Binary_t<T> Binary_t<T>::operator+(const Char_t* c1)
// {
//      //Addition of two binary numbers
//      return ((*this)+Binary_t<T>(c1));
// }
//
// Binary_t operator+(const Char_t* c1, const Binary_t& b2)
// {
//      //Addition of two binary numbers
//      return (b2+c1);
// }

//--------------------------------------------------bitwise OR operators

template <class T> Binary_t<T> Binary_t<T>::operator|(const Binary_t<T>& b1)
{
   //bitwise OR of two binary numbers
   Binary_t<T> tmp(fVal | b1.Value());
   return tmp;
}

// Binary_t Binary_t<T>::operator|(const Hexa_t& b1)
// {
//      //bitwise OR of two binary numbers
//      return Binary_t(b1.Value() | fVal);
// }
//
// Binary_t Binary_t<T>::operator|(const T l1)
// {
//      //bitwise OR of two binary numbers
//      return Binary_t(fVal | l1);
// }
//
// Binary_t operator|(const T l1, const Binary_t& b2)
// {
//      //bitwise OR of two binary numbers
//      return (b2|I1);
// }
//
// Binary_t operator|(const Char_t* c1)
// {
//      //bitwise OR of two binary numbers
//      return (Binary_t(c1)|(*this));
// }
//
// Binary_t operator|(const Char_t* c1, const Binary_t& b2)
// {
//      //bitwise OR of two binary numbers
//      Binary_t b1(c1);
//      Binary_t tmp(b1.Value() | b2.Value());
//      return tmp;
// }
//
// //--------------------------------------------------bitwise AND operators
//
template <class T> Binary_t<T> Binary_t<T>::operator&(const Binary_t<T>& b1)
{
   //bitwise AND of two binary numbers
   Binary_t<T> tmp(fVal & b1.Value());
   return tmp;
}

// Binary_t operator&(const Binary_t& b1, const T l2)
// {
//      //bitwise AND of two binary numbers
//      Binary_t tmp(b1.Value() & l2);
//      return tmp;
// }
//
// Binary_t operator&(const T l1, const Binary_t& b2)
// {
//      //bitwise AND of two binary numbers
//      Binary_t tmp(l1 & b2.Value());
//      return tmp;
// }
//
// Binary_t operator&(const Binary_t& b1, const Char_t* c2)
// {
//      //bitwise AND of two binary numbers
//      Binary_t b2(c2);
//      Binary_t tmp(b1.Value() & b2.Value());
//      return tmp;
// }
//
// Binary_t operator&(const Char_t* c1, const Binary_t& b2)
// {
//      //bitwise AND of two binary numbers
//      Binary_t b1(c1);
//      Binary_t tmp(b1.Value() & b2.Value());
//      return tmp;
// }

template < class T > void Binary_t < T >::SetBit(UChar_t nbit)
{
   //Set bit 'nbit' to 1
   //The least significant bit is numbered 0.

   SETBIT(fVal, nbit);
}

template < class T > void Binary_t < T >::SetBit(UChar_t nbit, T val)
{
   //Set bit 'nbit' to 1 to the same value as the equivalent bit in 'val'

   if (TESTBIT(val, nbit))
      SetBit(nbit);
   else
      ResetBit(nbit);
}

template < class T > void Binary_t < T >::ResetBit(UChar_t nbit)
{
   //Set bit 'nbit' to 0
   //The least significant bit is numbered 0.

   CLRBIT(fVal, nbit);
}

template < class T > Bool_t Binary_t < T >::TestBit(UChar_t nbit)
{
   //Returns kTRUE if bit is equal to '1'
   //The least significant bit is numbered 0.

   return TESTBIT(fVal, nbit);
}

template < class T > Hexa_t Binary_t < T >::Hexa() const
{
   //Returns number in its hexadecimal form
   return Hexa_t((Long64_t) Value());
}

template < class T > void Binary_t < T >::WriteSubvalue(const T val,
      UChar_t msb,
      UChar_t nbits)
{
   //Write the value 'val' in the 'nbits' bits from bit 'msb' (Most Significant Bit) to
   //bit (msb-nbits+1).
   //Warning if the following conditions are not respected:
   //1 <= nbits <= fNBits   0 <= msb <= (fNBits-1)  (msb-nbits+1)>=0

   //transform everything to 'int' for arithmetic to work normally!
   Int_t _fNBits, _msb, _nbits;
   _fNBits = (Int_t) fNBits;
   _msb = (Int_t) msb;
   _nbits = (Int_t) nbits;
   if (_nbits < 1 || _nbits > _fNBits || _msb > (_fNBits - 1)
         || (_msb - _nbits + 1) < 0) {
      std::cout << "Error in <Binary_t<T>::WriteSubvalue> : ";
      if (_nbits < 1)
         std::cout << "nbits<1";
      if (_nbits > _fNBits)
         std::cout << "nbits(" << _nbits << ")>fNBits(" << _fNBits << ")";
      if (_msb > (_fNBits - 1))
         std::cout << "msb(" << _msb << ") > fNBits-1(" << _fNBits - 1 << ")";
      if ((_msb - _nbits + 1) < 0)
         std::cout << "(msb-nbits+1) < 0 : msb=" << _msb << " nbits=" << _nbits;
      std::cout << std::endl;
      return;
   }
   //get binary representation of 'val' with nbits
   Binary_t < T > tmp(val);
   //get full fNBits bit representation
   String(fNBits); //NB result is in fDumString !!
   //replace nbits bits from bit msb to msb-nbits+1 with binary representation of 'val'
   fDumString.Replace((_fNBits - _msb - 1), nbits, tmp.String(nbits));
   //set new value
   Set(fDumString.Data());
}

template < class T > void Binary_t < T >::Print(Option_t*) const
{
   std::cout << "Binary number : " << const_cast<Binary_t<T>*>(this)->String() << " : fNBits=" << (int) fNBits
             << " fNBitsRep=" << (int) fNBitsRep << " fVal=" << (Long64_t)
             Value() << std::endl;
}

template < class T > T Binary_t < T >::Subvalue(UChar_t firstbit,
      UChar_t nbits)
{
   //Returns the decimal value of a part of this binary number.
   //The bits (firstbit, firstbit-1, firstbit-2, ... , firstbit-nbits+1) [see BIT NUMBERING CONVENTION]
   //are used.
   //If firstbit-nbits+1 < 0 (i.e. there are not enough bits in the number)
   //we use all the bits from firstbit down to bit 0.

   //transform all to 'int' for arithmetic to work!!
   Int_t _firstbit, _nbits;
   _firstbit = (Int_t) firstbit;
   _nbits = (Int_t) nbits;
   //check number of bits
   _nbits = ((_firstbit - _nbits + 1) < 0 ? _firstbit + 1 : _nbits);

   //Get string representation of value with enough bits i.e. (firstbit+1) bits
   String(_firstbit + 1); //result is in fDumString!!
   //Now get value of the required substring of this string
   fDumString2 = fDumString(0, _nbits);

   Binary_t < T > tmp;
   tmp.Set(fDumString2.Data());
   return (tmp.Value());
}

template < class T > Long64_t Binary_t < T >::Max() const
{
   //Returns maximum value that can be stored in number, i.e. when all bits = 1
   return (Long64_t)pow(2.0, (int)fNBits) - 1;
}

//_____________________________________________________________________________//
//typedef aliases
typedef Binary_t < UChar_t > Binary8_t;
typedef Binary_t < UShort_t > Binary16_t;
typedef Binary_t < UInt_t > Binary32_t;
typedef Binary_t < Long64_t > Binary64_t;
#endif
