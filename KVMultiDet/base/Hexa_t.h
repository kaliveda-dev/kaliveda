#ifndef __HEXA_T__
#define __HEXA_T__

#include "Rtypes.h"
#include "Riostream.h"
#include "TString.h"

/**
  \class Hexa_t
  \ingroup Core
  \brief Hexadecimal numbers
 */

class Hexa_t {
private:
   Long64_t fVal;
   static TString fPrefix;      //the prefix used to represent hexadecimal numbers as strings

public:
   Hexa_t();
   Hexa_t(const Long64_t);
   Hexa_t(const Char_t*);
   Hexa_t(const Hexa_t& other) : fVal(other.fVal) {}
   virtual ~ Hexa_t()
   {
   };
   //_________________________________________//

   static void SetPrefix(const Char_t* pref)
   {
      fPrefix = pref;
   };
   static const Char_t* GetPrefix()
   {
      return fPrefix.Data();
   };

   void Set(const Long64_t);
   Long64_t Value() const;
   void Set(const Char_t*);
   const Char_t* String(UChar_t nbits = 0) const;

   void SetBit(UChar_t, Long64_t = -1);
   void ResetBit(UChar_t);
   Bool_t TestBit(UChar_t);

   Hexa_t& operator=(const Long64_t);
   Hexa_t& operator=(const Char_t*);
//      Hexa_t& operator=(const Binary_t&);
   Hexa_t& operator=(const Hexa_t&);
   //convertors
   operator       Int_t() const
   {
      return (Int_t) Value();
   };
   operator       Long64_t() const
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
   operator       const char* () const
   {
      return String();
   };

   Hexa_t operator+(const Hexa_t& b1);
//      Hexa_t operator+(const Binary_t& b1);
   Hexa_t operator+(const Long64_t l1);
   Hexa_t operator+(const Char_t* c1);

//      Binary_t Binary() const;
//      void TestBinary();

   ClassDef(Hexa_t, 1)          //a Hexadecimal number
};


// Hexa_t operator+(const Hexa_t& b1, const Hexa_t& b2);
// Hexa_t operator+(const Hexa_t& b1, const Long64_t l2);
// Hexa_t operator+(const Long64_t l1, const Hexa_t& b2);
// Hexa_t operator+(const Hexa_t& b1, const Char_t* c2);
// Hexa_t operator+(const Char_t* c1, const Hexa_t& b2);
//      //bitwise OR
// Hexa_t operator|(const Hexa_t& b1, const Hexa_t& b2);
// Hexa_t operator|(const Hexa_t& b1, const Long64_t l2);
// Hexa_t operator|(const Long64_t l1, const Hexa_t& b2);
// Hexa_t operator|(const Hexa_t& b1, const Char_t* c2);
// Hexa_t operator|(const Char_t* c1, const Hexa_t& b2);
//      //bitwise AND
// Hexa_t operator&(const Hexa_t& b1, const Hexa_t& b2);
// Hexa_t operator&(const Hexa_t& b1, const Long64_t l2);
// Hexa_t operator&(const Long64_t l1, const Hexa_t& b2);
// Hexa_t operator&(const Hexa_t& b1, const Char_t* c2);
// Hexa_t operator&(const Char_t* c1, const Hexa_t& b2);

#endif
