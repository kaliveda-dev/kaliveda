//Created by KVClassFactory on Tue Sep  4 11:26:12 2012
//Author: Guilain ADEMARD

#ifndef __KVVAMOS_H
#define __KVVAMOS_H

#include "KVMultiDetArray.h"
#include "KVBase.h"
#include "KVList.h"
#include "KVHashList.h"
#include "TGeoMatrix.h"
#include "KVReconstructedNucleus.h"
#include "KVMacros.h" // 'UNUSED' macro
#include "KVZGOUBIReconstruction.h"

using namespace std;

class KVEBYEDAT_ACQParam;
class KVVAMOSDetector;
class TGeoVolume;
class TEnv;
class KVCalibrator;
class KVDetector;
class KVVAMOSTransferMatrix;
class KVVAMOSReconGeoNavigator;
class KVVAMOSWeightFinder;
class KVReconstructedNucleus;
class KVBasicVAMOSFilter;
class KVZGOUBIReconstruction;

class KVVAMOS : public KVMultiDetArray {

private:

   enum {
      kGeoModified = BIT(21)  //flag set when the geometry is modified
   };

   static KVString fACQParamTypes; //!types of available Acquision parameters
   static KVString fPositionTypes; //!types of available positions

protected:

   Double_t    fAngle;        //!Angular rotation around the target (deg)
   Double_t    fBrhoRef;      //!Reference magnetic rigidity (T.m)
   Double_t    fBeamHF;       //!High frequency of the beam (MHz)
   KVHashList* fCalibrators;  //References to calibrator associated to detectors
   KVList*     fFiredDets;    //!List of fired detectors of VAMOS
   Double_t    fFocalPos;     //!Position of the focal plane from target center (in cm)
   TGeoHMatrix fFocalToTarget;//!focal-plane to target position transformation matrix
   TGeoVolume* fFPvolume;     //!TGeoVolume centered on the focal plane
   UInt_t      fGr;           //!Used to number groups
   TGeoRotation*          fRotation;   //!rotation matrix around the target
   KVMaterial*            fStripFoil; //stripping foil used in experiment
   Double_t               fStripFoilPos; // distance between the target point and the stripping foil
   KVVAMOSReconGeoNavigator* fReconNavigator;//! navigator used to reconstruct nuclei in VAMOS
   KVVAMOSTransferMatrix* fTransMatrix;//!Transfer matrix for the reconstruction LAB<-->FP
   KVVAMOSWeightFinder* fWeightFinder;//!Weight computation for VAMOS events
   KVZGOUBIReconstruction* fZGOUBIReconstruction;//!Transfer matrix for the reconstruction LAB<-->FP using ZGOUBI
   KVList*      fVACQParams;  //->References to data acquisition parameter belonging to VAMOS
   TGeoVolume*  fVAMOSvol;    //!TGeoVolume of VAMOS
   KVList*      fVCalibrators;//->References to calibrator belonging to VAMOS
   Double_t     fECalibPar[4];//! nucleus energy calibration parameters
   Bool_t       fECalibStatus;//! energy calibration status

   KVBasicVAMOSFilter* fFilter; //! geometric basic VAMOS filter

   KVNameValueList ToFlist_deltapath; //list of distance between FP and detectors in z

   virtual void   BuildFocalPlaneGeometry(TEnv* infos);
   virtual Bool_t BuildGeoVolume(TEnv* infos);
   virtual void   BuildVAMOSGeometry();
   virtual void   InitGeometry();
   virtual Int_t  LoadGeoInfosIn(TEnv* infos);
   virtual void   MakeListOfDetectors();
   virtual Bool_t ReadDetectorGroupFile(ifstream& ifile);
   virtual void   SetArrayACQParams();
   virtual void   SetCalibrators();
   virtual void   SetGroupsAndIDTelescopes();
   virtual void set_up_telescope(KVDetector* de, KVDetector* e, KVIDTelescope* idt, TCollection* l);
   virtual void set_up_single_stage_telescope(KVDetector* det, KVIDTelescope* idt, TCollection* l);


public:

   KVVAMOS();
   KVVAMOS(const KVVAMOS&) ;
   virtual ~KVVAMOS();

   void init();

   void     AddACQParam(KVEBYEDAT_ACQParam* par, Bool_t owner);
   Bool_t   AddCalibrator(KVCalibrator* cal, Bool_t owner = kFALSE);
   virtual void     Build(Int_t run = -1);
   virtual Bool_t   Calibrate(KVReconstructedNucleus* nuc);
   virtual void     Clear(Option_t* opt = "");
   void     Copy(TObject&) const;
   KVNameValueList* DetectParticle(KVNucleus* part);
   void     FocalToTarget(const Double_t* focal, Double_t* target);
   void     FocalToTargetVect(const Double_t* focal, Double_t* target);
   virtual KVList*  GetFiredDetectors(Option_t* opt = "Pany");
   virtual Int_t    GetIDTelescopes(KVDetector*, KVDetector*, TCollection*);
   virtual Double_t GetStripFoilEnergyLossCorrection(KVReconstructedNucleus*);
   KVVAMOSReconGeoNavigator* GetReconNavigator();
   KVVAMOSTransferMatrix* GetTransferMatrix();
   KVZGOUBIReconstruction* GetZGOUBIReconstruction();
   KVVAMOSWeightFinder* GetWeightFinder();
   virtual void     Initialize();
   static  KVVAMOS* MakeVAMOS(const Char_t* name);
   virtual void     ResetParameters();
   void     SetECalibParameters(Double_t c_0, Double_t c_chi, Double_t c_si, Double_t c_csi = 0);
   virtual void     SetPedestal(const Char_t* name, Float_t ped);
   void     SetStripFoil(KVMaterial* foil, Double_t pos = 0);
   void     SetStripFoil(const Char_t* material, const Float_t area_density, Double_t pos);
   void     SetTransferMatrix(KVVAMOSTransferMatrix* mat);
   void     SetZGOUBIReconstruction(KVZGOUBIReconstruction* mat);
   void     TargetToFocal(const Double_t* target, Double_t* focal);
   void     TargetToFocalVect(const Double_t* target, Double_t* focal);
   virtual void     UpdateGeometry();
   void          SetToFlist_deltapath();
   KVNameValueList GetToFlist_deltapath();

   // ----- inline methods

   void          GeoModified();
   Double_t      GetAngle()                            const;
   Double_t      GetBeamHF()                           const;
   Double_t      GetBeamPeriod()                       const;
   Double_t      GetBrhoRef()                          const;
   KVCalibrator* GetCalibrator(const Char_t* type)  const;
   Bool_t        GetECalibStatus()                     const;
   Double_t      GetFocalPlanePosition()               const;
   TGeoVolume*   GetFocalPlaneVolume()                 const;
   TGeoHMatrix   GetFocalToTargetMatrix();
   TGeoVolume*   GetGeoVolume()                        const;
   KVHashList*   GetListOfCalibrators()                const;
   KVList*       GetListOfVCalibrators()               const;
   KVMaterial*   GetStripFoil()                        const;
   Double_t      GetStripFoilPosition()                const;
   Int_t         GetTrigger()                          const;
   KVEBYEDAT_ACQParam*   GetVACQParam(const Char_t* name);
   KVList*       GetVACQParams()                       const;
   KVCalibrator* GetVCalibrator(const Char_t* type) const;
   Bool_t        IsGeoModified()                       const;
   void          SetAngle(Double_t angle);
   void          SetBeamHF(Double_t hf);
   void          SetBrhoRef(Double_t Brho);
   void          SetCurrentRunNumber(UInt_t run);
   void          SetECalibStatus(Bool_t status);
   void          SetFocalPlanePosition(Double_t pos);

   //----- static methods

   static UInt_t    CalculateUniqueID(KVBase* param, KVVAMOSDetector* det = NULL);
   static UChar_t   GetACQParamTypeIdx(const Char_t* type, KVVAMOSDetector* det = NULL);
   static UChar_t   GetACQParamTypeIdxFromID(UInt_t id)
   {
      return (id / 1000) % 10;
   }
   static KVString& GetACQParamTypes()
   {
      return fACQParamTypes;
   }
   static UChar_t   GetPositionTypeIdx(const Char_t* type, KVVAMOSDetector* det = NULL);
   static UChar_t   GetPositionTypeIdxFromID(UInt_t id)
   {
      return (id / 10000) % 10;
   }
   static KVString& GetPositionTypes()
   {
      return fPositionTypes;
   }
   static Bool_t    IsUsedToMeasure(const Char_t* type, KVVAMOSDetector* det = NULL);

   void ShowECalibPar()
   {
      cout << "par0= " << fECalibPar[0] << endl;
      cout << "par1= " << fECalibPar[1] << endl;
      cout << "par2= " << fECalibPar[2] << endl;
      cout << "par3= " << fECalibPar[3] << endl;
   }
   ClassDef(KVVAMOS, 1) //VAMOS: variable mode spectrometer at GANIL
};

//_________________________________________________________________________

inline void KVVAMOS::GeoModified()
{
   SetBit(kGeoModified);
}


inline Double_t    KVVAMOS::GetAngle()      const
{
   return fAngle;
}
inline Double_t    KVVAMOS::GetBeamHF()     const
{
   return fBeamHF;
}


inline Double_t KVVAMOS::GetBeamPeriod() const
{
   //Return the beam period in ns
   return 1.e3 / fBeamHF;
}


inline Double_t KVVAMOS::GetBrhoRef() const
{
   return fBrhoRef;
}

inline Bool_t KVVAMOS::GetECalibStatus() const
{
   return fECalibStatus;
}

inline KVCalibrator* KVVAMOS::GetCalibrator(const Char_t* type) const
{
   if (fCalibrators)
      return (KVCalibrator*)fCalibrators->FindObjectByType(type);
   return 0;
}

inline Double_t KVVAMOS::GetFocalPlanePosition()  const
{
   return fFocalPos;
}
inline TGeoVolume* KVVAMOS::GetFocalPlaneVolume() const
{
   return fFPvolume;
}


inline TGeoHMatrix KVVAMOS::GetFocalToTargetMatrix()
{
   if (IsGeoModified()) UpdateGeometry();
   return fFocalToTarget;
}


inline TGeoVolume* KVVAMOS::GetGeoVolume() const
{
   // Returns the global TGeoVolume of VAMOS built when the method
   // BuildVAMOSGeometry() is called.
   return fVAMOSvol;
}


inline KVHashList* KVVAMOS::GetListOfCalibrators()   const
{
   return fCalibrators;
}
inline KVList*     KVVAMOS::GetListOfVCalibrators()  const
{
   return fVCalibrators;
}

inline KVMaterial* KVVAMOS::GetStripFoil()           const
{
   return fStripFoil;
}
inline Double_t KVVAMOS::GetStripFoilPosition()      const
{
   return fStripFoilPos;
}
inline Int_t       KVVAMOS::GetTrigger()             const
{
   return 1;
}

inline KVEBYEDAT_ACQParam* KVVAMOS::GetVACQParam(const Char_t* name)
{
   return (KVEBYEDAT_ACQParam*) fVACQParams->FindObject(name);
}

inline KVList* KVVAMOS::GetVACQParams() const
{
   return fVACQParams;
}


inline KVCalibrator* KVVAMOS::GetVCalibrator(const Char_t* type) const
{
   if (fVCalibrators)
      return (KVCalibrator*)fVCalibrators->FindObjectByType(type);
   return 0;
}


inline Bool_t KVVAMOS::IsGeoModified() const
{
   return TestBit(kGeoModified);
}


inline void KVVAMOS::SetAngle(Double_t angle)
{
   if (fAngle == angle) return;
   fAngle = angle;
   if (fRotation) {
      fRotation->Clear();
      fRotation->RotateY(angle);
   }
   GeoModified();
   UpdateGeometry();
}


inline void KVVAMOS::SetBeamHF(Double_t hf)
{
   fBeamHF   = hf;
}
inline void KVVAMOS::SetBrhoRef(Double_t Brho)
{
   fBrhoRef  = Brho;
}
inline void KVVAMOS::SetCurrentRunNumber(UInt_t run)
{
   fCurrentRun = run;
}
inline void KVVAMOS::SetECalibStatus(Bool_t status)
{
   fECalibStatus = status;
}
inline void KVVAMOS::SetFocalPlanePosition(Double_t pos)
{
   fFocalPos = pos;
}


//................  global variable
R__EXTERN KVVAMOS* gVamos;

#endif
