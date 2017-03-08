//Created by KVClassFactory on Tue Feb 14 10:53:57 2017
//Author: Patrick St-Onge,,,

#ifndef __KVZGOUBIINVERSEPOSITION_H
#define __KVZGOUBIINVERSEPOSITION_H

#include "KVBase.h"
#include <vector>

class KVZGOUBIInversePosition : public KVBase {
protected:
   std::vector<Int_t> InversePositionVector;
public:
   KVZGOUBIInversePosition();
   KVZGOUBIInversePosition(const KVZGOUBIInversePosition&);
   KVZGOUBIInversePosition(const Char_t* name, const Char_t* title = "");

   virtual ~KVZGOUBIInversePosition();
   virtual void Copy(TObject&) const;
   void AddtoInversePositionVector(Int_t TrajectoryPosition);
   Int_t GetInversePositionVectorValue(Int_t vectorposition);
   Bool_t GetInversePositionVectorStatus();
   Int_t GetInversePositionVectorSize();

   ClassDef(KVZGOUBIInversePosition, 1) //Class used to access ZGOUBI Trajectories in KVZGOUBIInverseMatrix
};

#endif
