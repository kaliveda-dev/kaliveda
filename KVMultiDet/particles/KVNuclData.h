//Created by KVClassFactory on Wed Feb 23 10:21:20 2011
//Author: bonnet

#ifndef __KVNUCLDATA_H
#define __KVNUCLDATA_H

#include "TObject.h"
#include "TString.h"
#include "KVConfig.h"

/**
\class KVNuclData
\brief Simple abstract class to store value related to nuclear data
\ingroup NucProp
*/

class KVNuclData : public TObject {
protected:
   Double_t kValue;  //numerical value
   TString  fName;   //name of the object
   TString  fUnits; //units (if any)

   void init();
   /*
   TObject's bits can be used as flags, bits 0 - 13 and 24-31 are
   reserved as  global bits while bits 14 - 23 can be used in different
   */
   enum EStatusBits {
      kMeasured = BIT(14)
   };


public:
   KVNuclData();
   KVNuclData(const Char_t* name, const Char_t* units = "NONE");
   KVNuclData(const KVNuclData&);
   ROOT_COPY_ASSIGN_OP(KVNuclData)
   virtual ~KVNuclData();
   void Copy(TObject& object) const;

   void SetName(TString name)
   {
      fName = name;
   }
   const Char_t* GetName() const;

   void SetValue(Double_t val);
   Double_t GetValue() const;

   const Char_t* GetUnit() const
   {
      return fUnits;
   }

   void SetMeasured(Bool_t measured)
   {
      SetBit(kMeasured, measured);
   }
   Bool_t IsMeasured() const
   {
      return TestBit(kMeasured);
   }

   virtual void Print(Option_t* option = "") const;


   ClassDef(KVNuclData, 2) //Simple abstract class to store value related to nuclear data
};

#endif
