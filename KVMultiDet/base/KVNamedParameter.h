//Created by KVClassFactory on Wed Nov 30 13:54:07 2011
//Author: John Frankland,,,

#ifndef __KVNAMEDPARAMETER_H
#define __KVNAMEDPARAMETER_H

#include "TNamed.h"

class KVNamedParameter : public TNamed {

   enum {
      kIsString = BIT(14),
      kIsDouble = BIT(15),
      kIsInt = BIT(16)
   };
   enum {
      kBitMask = 0x0001c000
   };
   void ResetBits()
   {
      ResetBit(kBitMask);
   };

   Int_t GetType() const
   {
      // compare this value with the values kIsString, kIsDouble, etc.
      return TestBits(kBitMask);
   };

   void SetType(UInt_t f)
   {
      ResetBits();
      SetBit(f);
   };

protected:
   Double_t fNumber;//used to store numerical (integer or floating-point) values

public:
   KVNamedParameter();
   KVNamedParameter(const char*);
   KVNamedParameter(const char*, const char*);
   KVNamedParameter(const char*, Double_t);
   KVNamedParameter(const char*, Int_t);
   virtual ~KVNamedParameter();

   void Set(const char*, const char*);
   void Set(const char*, Double_t);
   void Set(const char*, Int_t);
   void Set(const char*);
   void Set(Double_t);
   void Set(Int_t);
   void Set(const KVNamedParameter&);

   const Char_t* GetString() const;
   const TString& GetTString() const;
   Double_t GetDouble() const;
   Int_t GetInt() const;

   virtual void Clear(Option_t* = "");

   Bool_t IsString() const
   {
      return GetType() == kIsString;
   };
   Bool_t IsDouble() const
   {
      return GetType() == kIsDouble;
   };
   Bool_t IsInt() const
   {
      return GetType() == kIsInt;
   };
   Bool_t IsNumber() const
   {
      return (IsDouble() || IsInt());
   }

   Bool_t   IsEqual(const TObject* obj) const;
   Bool_t operator== (const KVNamedParameter&) const;
   virtual void Print(Option_t* opt = "") const;
   virtual void ls(Option_t* opt = "") const;

   Int_t Compare(const TObject* obj) const;

   ClassDef(KVNamedParameter, 1) //A generic parameter with a name and a value
};

#endif
