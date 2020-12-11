//Created by KVClassFactory on Sat Oct  3 14:18:09 2009
//Author: John Frankland,,,

#include "KVINDRADetector.h"
#include "KVGroup.h"
#include "KVDataSet.h"

ClassImp(KVINDRADetector)

const Char_t* KVINDRADetector::GetArrayName()
{
   // Name of detector given in the form
   //     CI_0213, SI_0911, CSI_1705
   // to be compatible with GANIL acquisition parameters
   //
   // The root of the name is the detector type

   fFName =
      Form("%s_%02d%02d", GetType(), GetRingNumber(),
           GetModuleNumber());
   return fFName.Data();
}

//_______________________________________________________________________________
void KVINDRADetector::AddACQParamType(const Char_t* type)
{
   // Add an acquisition parameter of given type to this detector
   // The parameter will be named according to INDRA/GANIL acquisition
   // convention, i.e. name_of_detector_type e.g.
   //    CI_0201_PG (type="PG")
   //    CSI_0913_R (type="R")
   //
   // If you want to use a different extension for a given type for certain
   // detectors, you can define them in the .kvrootrc file, e.g.:
   //
   //   e613.KVACQParam.SI_0101.T:    TOF
   //
   // Then for the dataset 'e613', calling GetACQParam("T") for the detector
   // SI_0101 will return the parameter with name "SI_0101_TOF"

   TString extension = gDataSet->GetDataSetEnv(Form("KVACQParam.%s.%s", GetName(), type), type);
   TString name;
   name = this->GetName();
   name.Append("_");
   name.Append(extension.Data());

   AddACQParam(new KVACQParam(name, type));
}

//________________________________________________________________________________
KVACQParam* KVINDRADetector::GetACQParam(const Char_t* type) const
{
   // Look for acquisition parameter with given type in list
   // of parameters associated with this detector.
   //
   //    *** WARNING ***
   // This method overrides the KVDetector::GetACQParam(const Char_t* name)
   // method, which interprets the argument as the FULL name of the parameter.
   // In this version, only the 'type' needs to be given,
   //   e.g.  GetACQParam("PG") to retrieve e.g. the "CI_0201_PG" parameter
   //    associated to detector "CI_0201".
   // If using this method in a loop over a list of INDRA- and non-INDRA
   // detectors, the behaviour of the method will be different for the two
   // types of detectors, and results may not be what you wanted.
   //
   // If you want to use a different extension for a given type for certain
   // detectors, you can define them in the .kvrootrc file, e.g.:
   //
   //   e613.KVACQParam.SI_0101.T:    TOF
   //
   // Then for the dataset 'e613', calling GetACQParam("T") for the detector
   // SI_0101 will return the parameter with name "SI_0101_TOF"

   TString extension = gDataSet->GetDataSetEnv(Form("KVACQParam.%s.%s", GetName(), type), type);
   extension.Prepend("_");
   // search just using the extension
   // (in case e.g. of a mismatch between detector & parameter names)
   TIter next(GetACQParamList());
   KVACQParam* p;
   while ((p = (KVACQParam*)next())) {
      TString name = p->GetName();
      if (name.EndsWith(extension)) return p;
   }
   return 0;
}

//__________________________________________________________________________________________________________________________

Double_t KVINDRADetector::GetPGfromGG(Double_t GG)
{
   //Linear conversion from GG to PG coder values (silicon and ionisation chamber detectors)
   //If GG is not given as argument, the current value of the detector's GG ACQParam is read
   //The PG value returned includes the current pedestal:
   //      PG = pied_PG + fGGtoPG_0 + fGGtoPG_1 * (GG - pied_GG)
   //alpha, beta coefficients are obtained by fitting (PG-pied) vs. (GG-pied) for data.
   if (GG < 0)
      GG = (Double_t) GetGG();
   Double_t PG =
      GetPedestal("PG") + fGGtoPG_0 + fGGtoPG_1 * (GG - GetPedestal("GG"));
   return PG;
}

//__________________________________________________________________________________________________________________________

Double_t KVINDRADetector::GetGGfromPG(Double_t PG)
{
   //Linear conversion from PG to GG coder values (silicon and ionisation chamber detectors)
   //If PG is not given as argument, the current value of the detector's PG ACQParam is read
   //The GG value returned includes the current pedestal:
   //      GG = pied_GG - (fGGtoPG_0/fGGtoPG_1) + (PG - pied_PG)/fGGtoPG_1
   //alpha, beta coefficients are obtained by fitting (PG-pied) vs. (GG-pied) for data.
   if (PG < 0)
      PG = (Double_t) GetPG();
   Double_t GG =
      GetPedestal("GG") - (fGGtoPG_0 / fGGtoPG_1)  + (PG - GetPedestal("PG")) / fGGtoPG_1;
   return GG;
}

KVINDRADetector* KVINDRADetector::FindChIo()
{
   //PRIVATE METHOD
   //Used when GetChIo is called the first time to retrieve the
   //pointer to the ChIo of the group associated to this detector
   if (GetTelescope()) {
      KVGroup* kvgr = GetTelescope()->GetGroup();
      if (kvgr) {
         const KVSeqCollection* dets = kvgr->GetDetectors();
         TIter next_det(dets);
         KVINDRADetector* dd;
         while ((dd = (KVINDRADetector*) next_det())) {
            if (dd->InheritsFrom("KVChIo"))
               fChIo = dd;
         }
      }
   }
   else
      fChIo = 0;
   return fChIo;
}

//__________________________________________________________________________________________________________________________
void KVINDRADetector::SetNumeroCodeur(Int_t numero)
{
   //Set the number of the electronic module of type QDC
   NumeroCodeur = numero;
}

//__________________________________________________________________________________________________________________________
Int_t KVINDRADetector::GetNumeroCodeur()
{
   //Get the number of the electronic module of type QDC
   //this field are initialized in KVINDRA::Build()
   return NumeroCodeur;
}

void KVINDRADetector::SetThickness(Double_t thick)
{
   // Overrides KVDetector::SetThickness
   // If using ROOT geometry, print warning that any change in detector thickness will not
   // be taken into account in the geometry.
   // All thicknesses have to be set before the ROOT geometry is generated.

   if (ROOTGeo()) {
      Warning("SetThickness", "Using ROOT geometry. Changes to detector thickness will not be taken into account in geometry.");
   }
   KVMaterial::SetThickness(thick);
}
