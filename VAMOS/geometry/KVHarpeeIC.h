//Created by KVClassFactory on Wed Jul 25 10:14:20 2012
//Author: Guilain ADEMARD

#ifndef __KVHARPEEIC_H
#define __KVHARPEEIC_H
#define ARPEEIC_NSEG  7

#include "KVVAMOSDetector.h"
#include "KVUnits.h"

class KVHarpeeIC : public KVVAMOSDetector {
private:

   static KVString fACQParamTypes; //!types of available Acquision parameters
   static KVString fPositionTypes; //!types of available positions

protected:

   void init();

public:

   KVHarpeeIC();
   KVHarpeeIC(UInt_t number, Float_t pressure, Float_t temp = 19., Float_t thick = 10.457 * KVUnits::cm);
   KVHarpeeIC(const KVHarpeeIC&) ;
   virtual ~KVHarpeeIC();

   void Copy(TObject&) const;

   virtual const Char_t*   GetArrayName();
   virtual const Char_t*   GetEBaseName() const;

   virtual Double_t  GetCalibE();
   virtual Double_t  GetCalibE(Int_t seg_num);
   virtual Double_t  GetEnergy();
   virtual UInt_t    GetFiredSegNumber(Option_t* opt = "P");
   virtual Double_t  GetPressure() const;
   virtual Bool_t    IsECalibrated() const;
   virtual Bool_t    PositionIsOK();
   virtual void      SetACQParams();
   virtual void      SetPressure(Double_t);
   virtual void      SetTemperature(Double_t);
   // ------ inline functions ----------------------//

   virtual KVString& GetACQParamTypes();
   virtual KVString& GetPositionTypes();

   ClassDef(KVHarpeeIC, 1) //Ionisiation chamber of Harpee, used at the focal plan of VAMOS
};
//_________________________________________________________________________

inline  KVString& KVHarpeeIC::GetACQParamTypes()
{
   return fACQParamTypes;
}
inline  KVString& KVHarpeeIC::GetPositionTypes()
{
   return fPositionTypes;
}
inline  Double_t KVHarpeeIC::GetPressure() const /* mbar */
{
   // Give pressure of gas in mbar
   return KVDetector::GetPressure() / KVUnits::mbar;
}
#endif
