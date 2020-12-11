//Created by KVClassFactory on Wed Feb  1 11:09:41 2012
//Author: John Frankland,,,

#ifndef __INDRAGEOMETRYBUILDER_H
#define __INDRAGEOMETRYBUILDER_H

#include "KVBase.h"
#include "TVector3.h"
#include "KVDetector.h"
#include "TGeoVolume.h"
#include "TGeoMatrix.h"
#include "KVNumberList.h"
#include "KVNameValueList.h"

class TGeoManager;
/**
\class INDRAGeometryBuilder
\brief Build INDRA geometry from Huguet CAO infos
\ingroup INDRAGeometry

 N.B. - the Ring 1 Si-CsI detectors (>=INDRA_camp4)
        are described using the same geometry as for the
        1st campaign phoswich detectors (no CAO data available)
*/

class INDRAGeometryBuilder : public KVBase {
   TString fDetName;//!name of detector
   TVector3 fInnerFront[4];//!coords of inner front face
   TVector3 fOuterFront[4];//!coords of outer front face
   TVector3 fFrameFront[4];//!coords of outer front face
   KVList* fLayers;//!list of materials making up layers of current detector
   KVDetector* fProtoDetector;//!detector used for last fLayers look up
   Int_t fActiveLayer;//!index of active layer of current detector
   KVMaterial fFrameMat;//!material of outer frame
   Double_t fTotalThickness;//!sum of thicknesses of layers of current detector
   TVector3 fInnerCentre;//!centre of inner face
   TVector3 fOuterCentre;//!centre of outer face
   TVector3 fFrameCentre;//!centre of frame
   Int_t Ndets;//!number of detectors in ring
   Double_t thetaMin, thetaMax, deltaPhi, phi0; //!theoretical geometry
   TGeoVolume* fFrameVolume;//!geo volume representing frame
   TGeoVolume* fDetVolume;//!geo volume representing frame
   Int_t fInnerPads, fOuterPads;
   Int_t fInnerRing, fOuterRing;
   Int_t fInnerDmod, fOuterDmod;
   Int_t fInnerModmin, fOuterModmin;
   Int_t fModmax;
   Double_t fRingCentreDistance;
   TGeoTranslation* fDetectorPosition;
   TGeoVolumeAssembly* fEtalonVol;
   Double_t fEtalonTheta[10];
   Double_t fEtalonPhi[10];
   void CorrectCoordinates(Double_t*, Double_t&, Double_t&);

public:
   INDRAGeometryBuilder();
   INDRAGeometryBuilder(const INDRAGeometryBuilder&) ;
   ROOT_COPY_ASSIGN_OP(INDRAGeometryBuilder)
   virtual ~INDRAGeometryBuilder();
   void Copy(TObject&) const;

   void ReadDetCAO(const Char_t* detname, Int_t ring);
   void CalculateBackPlaneCoordinates(TVector3* frontcoords, TVector3 centre, Double_t depth, TVector3* backcoords);
   void CalculateCentre(TVector3* corners, TVector3& centre);
   void CalculateCornersInPlane(TVector3* plane, Double_t thetamin,
                                Double_t thetamax, Double_t phimin, Double_t phimax, TVector3* corners);

   void Print(Option_t* = "") const;

   void MakeFrame(TString det_type, Int_t ring_num);
   void MakeDetector(const Char_t* det, TVector3* som, TVector3 cen);
   void PlaceFrame(Double_t phi, Int_t copy_no);
   void PlaceDetector();
   void MakeRing(const Char_t* det, int ring);
   void MakeEtalon(int RING);
   void ReflectPad(TVector3* orig, Double_t phicentre, TVector3* newpad);
   void CloseAndDraw();

   void Build(Bool_t withTarget = kTRUE, Bool_t closeGeometry = kTRUE);
   void BuildTarget();
   void Build(const KVNumberList& rings, const KVNameValueList& detectors);
   void TransformToOwnFrame(TVector3* orig, TVector3& centre, TVector3* ownframe);

   ClassDef(INDRAGeometryBuilder, 0) //Build INDRA geometry from Huguet CAO infos
   void BuildEtalonVolumes();
   Bool_t CheckDetectorPresent(TString detname);
};

#endif
