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
   Bool_t ReconstructFPtoLab(Double_t x_f, Double_t y_f, Double_t theta_f, Double_t phi_f, Double_t& brho, Double_t& path, Double_t& theta_v, Double_t& phi_v);
   ClassDef(KVZGOUBIReconstruction, 1) //Class used to access one ZGOUBI Trajectory
};

#endif
