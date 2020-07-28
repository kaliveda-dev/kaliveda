#include "Hexa_t.h"
#include "Riostream.h"
#include "TMath.h"
#include "TString.h"
#include "Binary_t.h"

ClassImp(Hexa_t)
TString Hexa_t::fPrefix = "0x"; //default prefix for hexadecimal numbers

Hexa_t::Hexa_t()
{
   //Create hexa number
   fVal = 0;
}

Hexa_t::Hexa_t(const Long64_t val)
{
   //Create hexa number corresponding to decimal value "val"
   fVal = val;
}

Hexa_t::Hexa_t(const Char_t* val)
{
   //Create hexa number from string containing hexa value
   Set(val);
}

void Hexa_t::Set(const Long64_t val)
{
   //Set number corresponding to decimal value "val"
   fVal = val;
}

Long64_t Hexa_t::Value() const
{
   //Get decimal value of number
   return fVal;
}

void Hexa_t::Set(const Char_t* val)
{
   //Set value from string containing hexa number
   //Can be written in lower or upper case.
   //Acceptable formats are "a0D4", "0xff21" and "$f5"

   TString tmp(val);
   tmp.ToUpper();
   if (tmp.BeginsWith("0X"))
      tmp.Remove(0, 2);
   if (tmp.BeginsWith("$"))
      tmp.Remove(0, 1);
   UInt_t xval;
   sscanf(tmp.Data(), "%X", &xval);
   Set((Long64_t) xval);
}

const Char_t* Hexa_t::String(UChar_t) const
{
   //Return pointer to string containing hexadecimal representation of number
   //This string should be copied immediately before further use.
   //The prefix used to represent hexadecimal numbers is "0x" by default.
   //This can be changed using SetPrefix.

   //use Form's circular Char_t buffer to return pointer
   return (Form("%s%llx", fPrefix.Data(), (ULong64_t)Value()));
}

Hexa_t& Hexa_t::operator=(const Long64_t val)
{
   //Assign a decimal value to the Hexa number
   Set(val);
   return (*this);
}

Hexa_t& Hexa_t::operator=(const Char_t* val)
{
   //Assign a Hexa value (as a string) to the Hexa number
   Set(val);
   return (*this);
}

// Hexa_t& Hexa_t::operator=(const Binary_t& val)
// {
//      //Assign a binary value to the Hexa number
//      Set(val.Value());
//      return (*this);
// }

Hexa_t& Hexa_t::operator=(const Hexa_t& val)
{
   //Assign a hexadecimal value to the Hexa number
   Set(val.Value());
   return (*this);
}

//
Hexa_t Hexa_t::operator+(const Hexa_t& b1)
{
   //Addition of two Hexa numbers
   return Hexa_t(b1.Value() + fVal);
}

// Hexa_t Hexa_t::operator+(const Binary_t& b1)
// {
//      //Addition of two Hexa numbers
//      return Hexa_t(b1.Value() + fVal);
// }

Hexa_t Hexa_t::operator+(const Long64_t l1)
{
   //Addition of two Hexa numbers
   return Hexa_t(l1 + fVal);
}

Hexa_t Hexa_t::operator+(const Char_t* c1)
{
   //Addition of two Hexa numbers
   return Hexa_t(c1) + (*this);
}

//--------------------------------------------------bitwise OR operators

// Hexa_t operator|(const Hexa_t& b1, const Hexa_t& b2)
// {
//      //bitwise OR of two Hexa numbers
//      Hexa_t tmp(b1.Value() | b2.Value());
//      return tmp;
// }
//
// Hexa_t operator|(const Hexa_t& b1, const Long64_t l2)
// {
//      //bitwise OR of two Hexa numbers
//      Hexa_t tmp(b1.Value() | l2);
//      return tmp;
// }
//
// Hexa_t operator|(const Long64_t l1, const Hexa_t& b2)
// {
//      //bitwise OR of two Hexa numbers
//      Hexa_t tmp(l1 | b2.Value());
//      return tmp;
// }
//
// Hexa_t operator|(const Hexa_t& b1, const Char_t* c2)
// {
//      //bitwise OR of two Hexa numbers
//      Hexa_t b2(c2);
//      Hexa_t tmp(b1.Value() | b2.Value());
//      return tmp;
// }
//
// Hexa_t operator|(const Char_t* c1, const Hexa_t& b2)
// {
//      //bitwise OR of two Hexa numbers
//      Hexa_t b1(c1);
//      Hexa_t tmp(b1.Value() | b2.Value());
//      return tmp;
// }
//
// //--------------------------------------------------bitwise AND operators
//
// Hexa_t operator&(const Hexa_t& b1, const Hexa_t& b2)
// {
//      //bitwise AND of two Hexa numbers
//      Hexa_t tmp(b1.Value() & b2.Value());
//      return tmp;
// }
//
// Hexa_t operator&(const Hexa_t& b1, const Long64_t l2)
// {
//      //bitwise AND of two Hexa numbers
//      Hexa_t tmp(b1.Value() & l2);
//      return tmp;
// }
//
// Hexa_t operator&(const Long64_t l1, const Hexa_t& b2)
// {
//      //bitwise AND of two Hexa numbers
//      Hexa_t tmp(l1 & b2.Value());
//      return tmp;
// }
//
// Hexa_t operator&(const Hexa_t& b1, const Char_t* c2)
// {
//      //bitwise AND of two Hexa numbers
//      Hexa_t b2(c2);
//      Hexa_t tmp(b1.Value() & b2.Value());
//      return tmp;
// }
//
// Hexa_t operator&(const Char_t* c1, const Hexa_t& b2)
// {
//      //bitwise AND of two Hexa numbers
//      Hexa_t b1(c1);
//      Hexa_t tmp(b1.Value() & b2.Value());
//      return tmp;
// }

void Hexa_t::SetBit(UChar_t nbit, Long64_t val)
{
   //Set bit 'nbit' to 1
   //The least significant bit is numbered 0.
   //If "val" is given, bit 'nbit' is set to the same value as the equivalent bit in 'val'

   if (val + 1) {               //default value of val=-1, so any value >= 0 will make this true
      if (TESTBIT(val, nbit))
         SetBit(nbit);
      else
         ResetBit(nbit);
   }
   else
      SETBIT(fVal, nbit);
}

void Hexa_t::ResetBit(UChar_t nbit)
{
   //Set bit 'nbit' to 0
   //The least significant bit is numbered 0.

   CLRBIT(fVal, nbit);
}

Bool_t Hexa_t::TestBit(UChar_t nbit)
{
   //Returns kTRUE if bit is equal to '1'
   //The least significant bit is numbered 0.

   return TESTBIT(fVal, nbit);
}

// Binary_t Hexa_t::Binary() const
// {
//      //Return binary equivalent of number
//      return Binary_t(Value());
// }

// void Hexa_t::TestBinary()
// {
//      //Test binary template clas
//      Binary16_t a(3), b("1101"), c(a);
//      a.Print(); b.Print(); c.Print();
//      cout << "//--------------------------------------------//" << endl;
//      Binary16_t d=16, e="11001", f=a;
//      d.Print(); e.Print(); f.Print();
//      cout << "//--------------------------------------------//" << endl;
//      a.Set("111111"); b.Set(53);
//      a.Print(); b.Print();
//      cout << "//--------------------------------------------//" << endl;
//      c=a;
//      b="101010";
//      a=16;
//      a.Print(); b.Print(); c.Print();
//      cout << "//--------------------------------------------//" << endl;
//      a.WriteSubvalue(42, 7, 6);
//      a.Print();
//      cout << (int)c.Subvalue(3,4) <<endl;
//      cout << "//--------------------------------------------//" << endl;
//
// }
