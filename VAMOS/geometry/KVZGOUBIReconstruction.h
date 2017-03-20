//Created by KVClassFactory on Tue Feb 14 10:53:57 2017
//Author: Patrick St-Onge,,,

#ifndef __KVZGOUBIRECONSTRUCTION_H
#define __KVZGOUBIRECONSTRUCTION_H

#include "KVBase.h"
#include "KVZGOUBIInverseMatrix.h"
#include "KVZGOUBITrajectory.h"

class KVZGOUBIInverseMatrix;

class KVZGOUBIReconstruction : public KVBase {
protected:
   KVZGOUBIInverseMatrix* Matrix_2D;
   KVZGOUBIInverseMatrix* Matrix_4D;
public:
   KVZGOUBIReconstruction();
   KVZGOUBIReconstruction(Bool_t init);
   KVZGOUBIReconstruction(const KVZGOUBIReconstruction&);
   KVZGOUBIReconstruction(const Char_t* name, const Char_t* title = "");
   virtual ~KVZGOUBIReconstruction();
   virtual void Copy(TObject&) const;
   void Init();
   Bool_t ReconstructFPtoLab(KVVAMOSReconTrajectory* traj);
   ClassDef(KVZGOUBIReconstruction, 1) //Class used to access one ZGOUBI Trajectory
};

#endif
