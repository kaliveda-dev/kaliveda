//Created by KVClassFactory on Tue Jan 27 11:37:39 2015
//Author: ,,,

#ifndef __KVFAZIA_H
#define __KVFAZIA_H

#include "KVMultiDetArray.h"

#include <KVGeoImport.h>
#include <KVEnv.h>
#include <KVSignal.h>

#if ROOT_VERSION_CODE <= ROOT_VERSION(5,32,0)
#include "TGeoMatrix.h"
#endif

class KVDetectorEvent;
#ifdef WITH_PROTOBUF
#ifndef __CINT__
namespace DAQ {
   class FzEvent;
}
#endif
#endif

/**
  \class KVFAZIA
  \ingroup FAZIAGeo
  \brief Description of a FAZIA detector geometry
 */
class KVFAZIA : public KVMultiDetArray {
protected:
   TString fFGeoType;  //type of FAZIA geometry (="compact",...)
   Double_t fFDist;    //distance of FAZIA detectors from target (in cm)
   Double_t fFThetaMin;//minimum polar angle for compact geometry (in degrees)
   Int_t fNblocks;   //number of blocks
   Int_t fStartingBlockNumber;   //starting number of block incrementation
   //Bool_t fBuildTarget; //kTRUE to include target frame in the geometry
   TString fCorrespondanceFile; //name of the file where are listed links between geometry and detector names
   KVString fDetectorLabels;
   KVString fSignalTypes;
   Double_t fImport_dTheta;//! for geometry import
   Double_t fImport_dPhi;//! for geometry import
   Double_t fImport_ThetaMin;//! for geometry import
   Double_t fImport_ThetaMax;//! for geometry import
   Double_t fImport_PhiMin;//! for geometry import
   Double_t fImport_PhiMax;//! for geometry import
   Double_t fImport_Xorg;//! for geometry import
   Double_t fImport_Yorg;//! for geometry import
   Double_t fImport_Zorg;//! for geometry import
   int fQuartet[8][2];//! quartet number from #FEE and #FPGA
   int fTelescope[8][2];//! telescope number from #FEE and #FPGA

   // values of trapezoidal filter rise time set in the fpgas defined in .kvrootrc
   Double_t fQH1risetime;
   Double_t fQ2risetime;
   Double_t fQ3slowrisetime;
   Double_t fQ3fastrisetime;

   //methods to be implemented in child classes
   virtual void BuildFAZIA();
   virtual void GetGeometryParameters();
   //

   virtual void BuildTarget();
   void GenerateCorrespondanceFile();
   virtual void SetNameOfDetectors(KVEnv& env);

   virtual void DefineStructureFormats(KVGeoImport&) {}

#ifdef WITH_MFM
   Bool_t handle_raw_data_event_mfmframe(const MFMCommonFrame&);
#endif
   void copy_fired_parameters_to_recon_param_list()
   {
      // override KVMultiDetArray method.
      // raw data signals are not copied to the general (event) parameter list,
      // they are stored in the lists of the individual particles and reset in the
      // detectors when the particles are read back.
      //
      // the formatting of the signal names is not compatible with the code in KVReconstructedEvent::Streamer,
      // hence the FAZIA-specific treatment which is handled by KVFAZIA::FillDetectorList.
   }

   void PerformClosedROOTGeometryOperations();

   void CreateCorrespondence();
#ifdef WITH_PROTOBUF
   Bool_t handle_raw_data_event_protobuf(KVProtobufDataReader&);
#ifndef __CINT__
   Bool_t treat_event(const DAQ::FzEvent&);
#endif
   Double_t TreatEnergy(Int_t sigid, Int_t eid, UInt_t val);
#endif
   TString GetSignalName(Int_t bb, Int_t qq, Int_t tt, Int_t idsig);
public:

   KVFAZIA(const Char_t* title = "");
   virtual ~KVFAZIA();
   void AddDetectorLabel(const Char_t* label);

   virtual void Build(Int_t = -1);

   void GetDetectorEvent(KVDetectorEvent* detev, const TSeqCollection* dets);
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

   void SetGeometryImportParameters(Double_t dt = 0.25, Double_t dp = 1.0, Double_t tmin = 2., Double_t pmin = 0, Double_t tmax = 20., Double_t pmax = 360.,
                                    Double_t xorg = 0, Double_t yorg = 0, Double_t zorg = 0)
   {
      // Set angular arguments for call to KVGeoImport::ImportGeometry in KVFAZIA::Build
      // Also set origin for geometry import to (xorg,yorg,zorg) [default: (0,0,0)]
      fImport_dPhi = dp;
      fImport_dTheta = dt;
      fImport_PhiMax = pmax;
      fImport_PhiMin = pmin;
      fImport_ThetaMax = tmax;
      fImport_ThetaMin = tmin;
      fImport_Xorg = xorg;
      fImport_Yorg = yorg;
      fImport_Zorg = zorg;
   }
   void FillDetectorList(KVReconstructedNucleus* rnuc, KVHashList* DetList, const KVString& DetNames);

   KVGroupReconstructor* GetReconstructorForGroup(const KVGroup*) const;
   Double_t GetSetupParameter(const Char_t* parname);

   ClassDef(KVFAZIA, 1) //Base class for description of the FAZIA set up
};

//................  global variable
R__EXTERN KVFAZIA* gFazia;

#endif
