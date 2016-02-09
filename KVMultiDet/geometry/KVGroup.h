#ifndef KVGROUP_H
#define KVGROUP_H

#define KVGROUP_ADD_UNKNOWN_TELESCOPE "Attempt to add undefined telescope to group"

#include "KVGeoStrucElement.h"
#include "KVList.h"

class KVDetector;
class KVNucleus;
class KVNameValueList;

class KVGroup : public KVGeoStrucElement {

protected:
   enum {
      kIsRemoving = BIT(14)     //flag set during call to RemoveTelescope
   };
   KVList* fReconstructedNuclei;        //!Particles reconstructed in this group

public:
   enum {
      kForwards,
      kBackwards
   };
   KVGroup();
   void init();
   virtual ~ KVGroup();
   virtual void SetNumber(UInt_t num)
   {
      // Setting number for group also sets name to "Group_n"
      SetName(Form("Group_%u", num));
      KVGeoStrucElement::SetNumber(num);
   };

   virtual UInt_t GetNumberOfDetectorLayers();

   void Reset();

   virtual TList* GetDetectorsInLayer(UInt_t lay);
   virtual TList* GetAlignedDetectors(KVDetector*, UChar_t dir = kBackwards);

   inline UInt_t GetHits()
   {
      if (fReconstructedNuclei)
         return fReconstructedNuclei->GetSize();
      else
         return 0;
   };
   void ClearHitDetectors();
   //inline UInt_t GetNIdentified();
   //inline UInt_t GetNUnidentified();
   KVList* GetParticles()
   {
      return fReconstructedNuclei;
   }
   void AddHit(KVNucleus* kvd);
   void RemoveHit(KVNucleus* kvd);

   Bool_t IsRemoving()
   {
      return TestBit(kIsRemoving);
   }
   virtual void Sort() {};
   virtual void CountLayers() {};

   ClassDef(KVGroup, 1)//Group of detectors having similar angular positions.
};

/*#ifndef KVRECONSTRUCTEDNUCLEUS_H
#include "KVReconstructedNucleus.h"
#endif
inline UInt_t KVGroup::GetNIdentified()
{
   //number of identified particles reconstructed in group
   UInt_t n = 0;
   if (GetHits()) {
      TIter next(fReconstructedNuclei);
      KVReconstructedNucleus* nuc = 0;
      while ((nuc = (KVReconstructedNucleus*) next()))
         n += (UInt_t) nuc->IsIdentified();
   }
   return n;
};
inline UInt_t KVGroup::GetNUnidentified()
{
   //number of unidentified particles reconstructed in group
   return (GetHits() - GetNIdentified());
};
*/
#endif
