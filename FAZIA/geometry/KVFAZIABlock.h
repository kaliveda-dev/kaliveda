//Created by KVClassFactory on Thu May 30 10:13:17 2013
//Author: Eric Bonnet

#ifndef __KVFAZIABLOCK_H
#define __KVFAZIABLOCK_H

#include "TGeoVolume.h"
#include <TVector3.h>
#include "KVConfig.h"

/**
  \class KVFAZIABlock
  \brief Standard geometry of 16-telescope FAZIA block
  \ingroup FAZIAGeo
 */
class KVFAZIABlock : public TGeoVolumeAssembly {

   double fTotSidWBld;
   double frame_side;
   double centre_cross_thick;
   double side_thick;
   double si_holder_side;
   double si_centre;
   double front_frame_depth;
   double frame_depth_300;
   double frame_depth_500;
   double frame_edge_depth;
   double si300_position;
   double si500_position;
   double full_width;
   double dist_si2_csi;
   double csi_depth;
   double full_length_quartet;
   double thick_si1;
   double thick_si2;
   double dist_csi_sarco;
   double sarco_length;
   double sarco_width;
   double sarco_height;
   double sarco_epaisseur;
   double sarco_uplift;

   TGeoVolume* MakeRightCrossPieceCSI(double front_side, double separation, double distPlane, double depth);
   TGeoVolume* MakeLeftCrossPieceCSI(double front_side, double separation, double distPlane, double depth);
   TGeoVolume* MakeCrossPieceCSI(double front_side, double separation, double distPlane, double depth);
   std::vector<double> GetCoordsCSI(int tel_num, double front_side, double separation, double distPlane, double depth);
   TGeoVolume* MakeErgalFrame(double frame_depth, double ergal_edge_depth);
   TGeoVolume* MakeQuartet();
   TGeoVolume* MakeSarco();
   struct Projector {
      double rap, depth;
      Projector(double D, double d) : rap((D + d) / D), depth(d) {}

      TVector3 operator()(const TVector3& v)
      {
#ifdef WITH_CPP11
         return {rap * v.X(), rap * v.Y(), v.Z() + depth};
#else
         return TVector3(rap * v.X(), rap * v.Y(), v.Z() + depth);
#endif
      }
   };

public:
   KVFAZIABlock();
   virtual ~KVFAZIABlock() {}

   Double_t GetTotalSideWithBlindage() const
   {
      // Total length of one side of block including blindage
      return fTotSidWBld;
   }
   Double_t GetNominalDistanceTargetBlockCentre(double dist_SI1 = 100.) const
   {
      // Distance from origin at which to place the volume corresponding to this block
      // so that SI1 is dist_SI1 cm from target
      //
      // i.e. the following code:
      //
      //~~~~~~~~~
      //   KVFAZIABlock* b = new KVFAZIABlock;
      //   gGeoManager->GetTopVolume()->AddNode(b, 1, new TGeoTranslation(0,0,b->GetNominalDistanceTargetBlockCentre()));
      //~~~~~~~~~
      //
      // will place a block at 0 deg. with its SI1 100cm from the target

      return dist_SI1 + 0.5 * full_length_quartet - (front_frame_depth + frame_edge_depth);
   }
   Double_t GetBrassCollimatorDepth() const
   {
      // Depth of brass frame placed in front of SI1 detectors
      return front_frame_depth;
   }

   ClassDef(KVFAZIABlock, 1) //Block of 16 telescopes FAZIA
};

#endif
