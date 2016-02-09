//Created by KVClassFactory on Tue Jan 27 11:37:39 2015
//Author: ,,,

#ifndef __KVFAZIA_H
#define __KVFAZIA_H

#include "KVMultiDetArray.h"

#include <KVGeoImport.h>
#include <KVEnv.h>

#if ROOT_VERSION_CODE <= ROOT_VERSION(5,32,0)
#include "TGeoMatrix.h"
#endif

class KVDetectorEvent;

class KVFAZIA : public KVMultiDetArray {
protected:
   TString fFGeoType;  //type of FAZIA geometry (="compact",...)
   Double_t fFDist;    //distance of FAZIA detectors from target (in cm)
   Double_t fFThetaMin;//minimum polar angle for compact geometry (in degrees)
   Int_t fNblocks;   //number of blocks
   Int_t fStartingBlockNumber;   //starting number of block incrementation
   Bool_t fBuildTarget; //kTRUE to include target frame in the geometry
   TString fCorrespondanceFile; //name of the file where are listed links between geometry and detector names
   KVString fDetectorLabels;
   KVString fSignalTypes;
   Double_t fImport_dTheta;//! for geometry import
   Double_t fImport_dPhi;//! for geometry import
   Double_t fImport_ThetaMin;//! for geometry import
   Double_t fImport_ThetaMax;//! for geometry import
   Double_t fImport_PhiMin;//! for geometry import
   Double_t fImport_PhiMax;//! for geometry import

   //methods to be implemented in child classes
   virtual void BuildFAZIA();
   virtual void GetGeometryParameters();
   //

   virtual void BuildTarget();
   void GenerateCorrespondanceFile();
   virtual void SetNameOfDetectors(KVEnv& env);

   virtual void DefineStructureFormats(KVGeoImport&) {}

public:

   KVFAZIA();
   virtual ~KVFAZIA();
   void AddDetectorLabel(const Char_t* label);

   virtual void Build(Int_t run = -1);
   void SortIDTelescopes();

   void GetDetectorEvent(KVDetectorEvent* detev, TSeqCollection* fired_params);
   Int_t GetNumberOfBlocks() const
   {
      return fNblocks;
   }
   void IncludeTargetInGeometry(Bool_t include = kTRUE)
   {
      fBuildTarget = include;
   }

   KVString GetDetectorLabels() const
   {
      return fDetectorLabels;
   }
   const Char_t* GetSignalTypes() const
   {
      return fSignalTypes.Data();
   }

   void SetGeometryImportParameters(Double_t dt = 0.25, Double_t dp = 1.0, Double_t tmin = 2., Double_t pmin = 0, Double_t tmax = 20., Double_t pmax = 360.)
   {
      // Set angular arguments for call to KVGeoImport::ImportGeometry in KVFAZIA::Build
      fImport_dPhi = dp;
      fImport_dTheta = dt;
      fImport_PhiMax = pmax;
      fImport_PhiMin = pmin;
      fImport_ThetaMax = tmax;
      fImport_ThetaMin = tmin;
   }

   ClassDef(KVFAZIA, 1) //Base class for description of the FAZIA set up
};

//................  global variable
R__EXTERN KVFAZIA* gFazia;

#endif
