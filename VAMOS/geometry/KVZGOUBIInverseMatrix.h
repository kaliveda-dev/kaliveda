//Created by KVClassFactory on Tue Feb 14 10:53:57 2017
//Author: Patrick St-Onge,,,

#ifndef __KVZGOUBIINVERSEMATRIX_H
#define __KVZGOUBIINVERSEMATRIX_H

#include "KVBase.h"
#include "KVZGOUBITrajectory.h"
#include "KVZGOUBIInversePosition.h"
#include "KVZGOUBIComb.h"
#include <vector>
#include <math.h>
#include "TFile.h"
#include <TTree.h>
#include "KVDataSet.h"
#include "KVVAMOSReconTrajectory.h"
#include "KVVAMOS.h"
#include "TChain.h"

class KVZGOUBIInverseMatrix : public KVBase {
protected:
   std::vector<KVZGOUBITrajectory> ZGOUBIDatabase;
   std::vector<KVZGOUBIInversePosition> ZGOUBIInversePositionDatabase;
   Float_t xfmin;
   Float_t xfmax;
   Float_t yfmin;
   Float_t yfmax;
   Float_t thetafmin;
   Float_t thetafmax;
   Float_t phifmin;
   Float_t phifmax;
   Float_t delta_xf;
   Float_t delta_yf;
   Float_t delta_thetaf;
   Float_t delta_phif;
   Int_t nbstep_xf;
   Int_t nbstep_yf;
   Int_t nbstep_thetaf;
   Int_t nbstep_phif;
   Float_t characteristicdistance_xf;
   Float_t characteristicdistance_yf;
   Float_t characteristicdistance_thetaf;
   Float_t characteristicdistance_phif;
   KVZGOUBIComb comb;

public:
   KVZGOUBIInverseMatrix();
   KVZGOUBIInverseMatrix(Int_t nbstepxft, Int_t nbstepthetaft, Int_t nbstepyft, Int_t nbstepphift, TChain* ch);
   KVZGOUBIInverseMatrix(const KVZGOUBIInverseMatrix&);
   KVZGOUBIInverseMatrix(const Char_t* name, const Char_t* title = "");

   virtual ~KVZGOUBIInverseMatrix();
   virtual void Copy(TObject&) const;
   void SetZGOUBIDatabase(TChain* ch);
   void AddZGOUBITrajectory(Float_t ThetaVt, Float_t PhiVt, Float_t Deltat, Float_t XFt, Float_t ThetaFt, Float_t YFt, Float_t PhiFt, Float_t Patht);
   void FindExtremum();
   void PrintExtremum();
   void FindDeltaParameters();
   void SetNbSteps(Int_t nbstepxft, Int_t nbstepthetaft, Int_t nbstepyft, Int_t nbstepphift);
   void AddZGOUBIInversePosition();
   void initZGOUBIInversePositionDatabase();
   void SetZGOUBIInversePositionDatabase();
   Int_t GetZGOUBIDatabase_position(Float_t XFt, Float_t ThetaFt, Float_t YFt, Float_t PhiFt);
   std::vector<Int_t> GetClosest4DVoxels(Float_t XFt, Float_t ThetaFt, Float_t YFt, Float_t PhiFt, Int_t nbneighbors);
   Float_t GetDistance(Float_t xf, Float_t thetaf, Float_t yf, Float_t phif, Float_t xf_line, Float_t thetaf_line, Float_t yf_line, Float_t phif_line);
   Int_t GetNearestLinenb(Float_t XFt, Float_t ThetaFt, Float_t YFt, Float_t PhiFt);
   KVZGOUBITrajectory GetZGOUBITrajectory(Int_t Trajectorynb);
   std::vector<Int_t> GetNearestLinenbs(Float_t XFt, Float_t ThetaFt, Float_t YFt, Float_t PhiFt, Int_t nblines);
   Float_t GetDistance(Float_t xf, Float_t thetaf, Float_t yf, Float_t phif, Int_t linenb);
   Float_t GetDistance(Int_t linenb1, Int_t linenb2);
   std::vector<Int_t> GetLinesinRadius(Float_t XFt, Float_t ThetaFt, Float_t YFt, Float_t PhiFt, Float_t radius);
   // std::vector<Float_t> testGetLinesinRadius_bari(Float_t XFt, Float_t ThetaFt, Float_t YFt, Float_t PhiFt, Float_t radiusmultiplier);
   //std::vector<Float_t> testGetLinesinRadius_weight(Float_t XFt, Float_t ThetaFt, Float_t YFt, Float_t PhiFt, Float_t radiusmultiplier);
   std::vector<Float_t> testGetResults_weight(Float_t XFt, Float_t ThetaFt, Float_t YFt, Float_t PhiFt, Int_t nblines);
   std::vector<Float_t> testGetResults_weight_comb(Float_t XFt, Float_t ThetaFt, Float_t YFt, Float_t PhiFt, Int_t nblines);
   void Setcharacteristicdistance(Float_t d_XF, Float_t d_ThetaF, Float_t d_YF, Float_t d_PhiF) {
      characteristicdistance_xf = d_XF;
      characteristicdistance_thetaf = d_ThetaF;
      characteristicdistance_yf = d_YF;
      characteristicdistance_phif = d_PhiF;
   }
   ClassDef(KVZGOUBIInverseMatrix, 1) //Class used to reconstruct trajectories in VAMOS
};

#endif
