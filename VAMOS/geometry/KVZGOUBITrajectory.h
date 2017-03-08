//Created by KVClassFactory on Tue Feb 14 10:53:57 2017
//Author: Patrick St-Onge,,,

#ifndef __KVZGOUBITRAJECTORY_H
#define __KVZGOUBITRAJECTORY_H

#include "KVBase.h"

class KVZGOUBITrajectory : public KVBase {
protected:
   Float_t ThetaV;
   Float_t PhiV;
   Float_t Delta;
   Float_t XF;
   Float_t YF;
   Float_t ThetaF;
   Float_t PhiF;
   Float_t Path;
public:
   KVZGOUBITrajectory();
   KVZGOUBITrajectory(Float_t ThetaV_t, Float_t PhiV_t, Float_t Delta_t, Float_t XF_t, Float_t ThetaF_t, Float_t YF_t, Float_t PhiF_t, Float_t Path_t);
   KVZGOUBITrajectory(const KVZGOUBITrajectory&);
   KVZGOUBITrajectory(const Char_t* name, const Char_t* title = "");

   virtual ~KVZGOUBITrajectory();
   virtual void Copy(TObject&) const;

   void SetTrajectoryParameters(Float_t ThetaVt, Float_t PhiVt, Float_t Deltat, Float_t XFt, Float_t ThetaFt, Float_t YFt, Float_t PhiFt, Float_t Patht);

   Float_t GetThetaV();
   Float_t GetPhiV();
   Float_t GetDelta();
   Float_t GetXF();
   Float_t GetYF();
   Float_t GetThetaF();
   Float_t GetPhiF();
   Float_t GetPath();

   ClassDef(KVZGOUBITrajectory, 1) //Class used to access one ZGOUBI Trajectory
};

#endif
