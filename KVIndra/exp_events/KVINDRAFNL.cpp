/***************************************************************************
$Id: KVINDRAFNL.cpp,v 1.11 2008/12/17 13:01:26 franklan Exp $
                          kvindrafnl.cpp  -  description
                             -------------------
    begin                : Wed Oct 29 2003
    copyright            : (C) 2003 by J.D. Frankland
    email                : frankland@ganil.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "KVINDRAFNL.h"
#include "KVSiB.h"
#include "KVBIC.h"
#include "KVTelBlocking.h"
#include "KVIDTelescope.h"
#include "KVIDBlocking.h"
#include "KVDataSet.h"

#define CHIO_SI_PIEDS "pied_chiosi_e416a.dat"
#define CSI_PIEDS "pied_csi_e416a.dat"

ClassImp(KVINDRAFNL)
KVINDRAFNL::KVINDRAFNL()
{
   //Default ctor.
}

//_________________________________________________________________________________

KVINDRAFNL::~KVINDRAFNL()
{
   //Default dtor.
}

//_________________________________________________________________________________

void KVINDRAFNL::MakeListOfDetectorTypes()
{
//   //Add SIB and BIC detectors to KVINDRA4 list
//   KVINDRA4::MakeListOfDetectorTypes();
//   KVSiB *sib = new KVSiB(500.0);
//   fDetectorTypes->Add(sib);
//   KVBIC *bic = new KVBIC(30.); //BIC 30 Torr
//   fDetectorTypes->Add(bic);
}

//_________________________________________________________________________________

void KVINDRAFNL::PrototypeTelescopes()
{
   //Add KVTelBlocking telescopes to list of prototypes for KVINDRA4
   //Each of these replaces a ChIo cell and block of 4 Si/CsI, so we use the azimuthal
   //widths corresponding to the replaced ChIo in each case.

//   KVINDRA4::PrototypeTelescopes();

//   //telescope blocking #1 : ring 4 12h
//   KVTelBlocking *tel = new KVTelBlocking(1);
//   fTelescopes->Add(tel);
//   tel->SetType("Blocking T1");
//   tel->SetAzimuthalWidth(29.10);

//   //telescope blocking #2 & #3 : ring 6 1h & 9h
//   tel = new KVTelBlocking(2);
//   fTelescopes->Add(tel);
//   tel->SetType("Blocking T2/3");
//   tel->SetAzimuthalWidth(29.28);

}

//_________________________________________________________________________________

void KVINDRAFNL::BuildGeometry()
{
   //build KVINDRA4, then remove detectors not present in E416
   //These are:
   //      ChIo    Ring 2, Rings 13/14/16
   //      Si300   Rings 1-5
   //      ChIo-Si-CsI groups: 0423-0524, 0617-0718, 0601-0702
   //Then add the 3 blocking telescopes T1, T2 and T3 (BIC + SIB)

   KVINDRA4::BuildGeometry();

   //remove chio's
   RemoveRing("CHIO", 2);
   RemoveRing("CHIO", 13);
   RemoveRing("CHIO", 14);
   RemoveRing("CHIO", 16);

   //remove Si300
   for (int ring = 1; ring < 6; ring++) {
      GetRing("SI-CSI", ring)->RemoveDetectors("SI");
   }

}

//_________________________________________________________________________________

void KVINDRAFNL::Build(Int_t run)
{
   //Construct KVINDRAFNL
   KVINDRA4::Build(run);

   GetRing("CHIO", 4)->RemoveTelescope("CI_0423");
   GetRing("SI-CSI", 4)->RemoveTelescope("CSI_0423");
   GetRing("SI-CSI", 4)->RemoveTelescope("CSI_0424");
   GetRing("SI-CSI", 5)->RemoveTelescope("CSI_0523");
   GetRing("SI-CSI", 5)->RemoveTelescope("CSI_0524");
   GetRing("CHIO", 6)->RemoveTelescope("CI_0601");
   GetRing("SI-CSI", 6)->RemoveTelescope("SI_CSI_0601");
   GetRing("SI-CSI", 6)->RemoveTelescope("SI_CSI_0602");
   GetRing("SI-CSI", 7)->RemoveTelescope("SI_CSI_0701");
   GetRing("SI-CSI", 7)->RemoveTelescope("SI_CSI_0702");
   GetRing("CHIO", 6)->RemoveTelescope("CI_0617");
   GetRing("SI-CSI", 6)->RemoveTelescope("SI_CSI_0617");
   GetRing("SI-CSI", 6)->RemoveTelescope("SI_CSI_0618");
   GetRing("SI-CSI", 7)->RemoveTelescope("SI_CSI_0717");
   GetRing("SI-CSI", 7)->RemoveTelescope("SI_CSI_0718");

   //add blocking telescopes
//   KVLayer *B__lay = new KVLayer();     //blocking layer

//   KVTelescope *B__tel =
//       (KVTelescope *) fTelescopes->FindObjectByType("Blocking T1");
//   //distance target->entrance of telescope.
//   //1224mm is distance to SIB, so have to subtract depth of SIB
//   Double_t dist = 1224. - B__tel->GetDepth(2);
//   B__lay->AddRing(new KVRing(4, 7.21, 13.83, 0., 1, dist, B__tel)

//       );

//   B__tel = (KVTelescope *) fTelescopes->FindObjectByType("Blocking T2/3");
//   //distance target->entrance of telescope.
//   //1003mm is distance to SIB, so have to subtract depth of SIB
//   dist = 1003. - B__tel->GetDepth(2);
//   B__lay->
//       AddRing(new
//               KVRing(6, 14.21, 26.74, 30., 2, dist, B__tel, 1, 2, 240.)

//       );

//   B__lay->SetName("BLOCKING");
//   AddLayer(B__lay);

//   UpdateArray();

//   SetName("INDRA");
//   SetTitle("INDRA for FNL Experiments E416 & E416a");

//   //set pedestals
//   Info("Build", "Setting pedestals for ChIo/Si detectors");
//   TString fullpath;
//   SearchKVFile(CHIO_SI_PIEDS, fullpath, fDataSet.Data());
//   SetPedestals(fullpath.Data());
//   Info("Build", "Setting pedestals for CsI detectors");
//   SearchKVFile(CSI_PIEDS, fullpath, fDataSet.Data());
//   SetPedestals(fullpath.Data());
}

//_________________________________________________________________________________

KVINDRADetector* KVINDRAFNL::GetDetectorByType(UInt_t cou, UInt_t mod,
      UInt_t type) const
{
   //Modified KVINDRA::GetDetectorByType
   //If the type is "ChIo_*" or "Si_*" and ring number 'cou' and
   //module number 'mod' correspond to one of the INDRA detectors
   //removed to make way for the blocking telescopes, then we return
   //a pointer to the corresponding BIC or SIB
   //
   //i.e.
   //GetDetectorByType(4,23,ChIo_GG/PG/T) -> BIC_1
   //GetDetectorByType(4,23,Si_GG/PG/T) -> SIB_1
   //GetDetectorByType(4,24,Si_GG/PG/T) -> SIB_1
   //GetDetectorByType(5,23,Si_GG/PG/T) -> SIB_1
   //GetDetectorByType(5,24,Si_GG/PG/T) -> SIB_1
   //
   //GetDetectorByType(6,1,ChIo_GG/PG/T) -> BIC_2
   //GetDetectorByType(6,1,Si_GG/PG/T) -> SIB_2
   //GetDetectorByType(6,2,Si_GG/PG/T) -> SIB_2
   //GetDetectorByType(7,1,Si_GG/PG/T) -> SIB_2
   //GetDetectorByType(7,2,Si_GG/PG/T) -> SIB_2
   //
   //GetDetectorByType(6,17,ChIo_GG/PG/T) -> BIC_3
   //GetDetectorByType(6,17,Si_GG/PG/T) -> SIB_3
   //GetDetectorByType(6,18,Si_GG/PG/T) -> SIB_3
   //GetDetectorByType(7,17,Si_GG/PG/T) -> SIB_3
   //GetDetectorByType(7,18,Si_GG/PG/T) -> SIB_3
   //

   KVINDRADetector* det = KVINDRA::GetDetectorByType(cou, mod, type);

   if (!det) {
      if (cou == 4 && mod == 23 && (type >= ChIo_GG && type <= ChIo_T))
         det = (KVINDRADetector*)GetDetector("BIC_1");
      else if (cou == 6 && mod == 1 && (type >= ChIo_GG && type <= ChIo_T))
         det = (KVINDRADetector*)GetDetector("BIC_2");
      else if (cou == 6 && mod == 17
               && (type >= ChIo_GG && type <= ChIo_T))
         det = (KVINDRADetector*)GetDetector("BIC_3");
      else if ((cou >= 4 && cou <= 5) && (mod >= 23 && mod <= 24)
               && (type >= Si_GG && type <= Si_T))
         det = (KVINDRADetector*)GetDetector("SIB_1");
      else if ((cou >= 6 && cou <= 7) && (mod >= 1 && mod <= 2)
               && (type >= Si_GG && type <= Si_T))
         det = (KVINDRADetector*)GetDetector("SIB_2");
      else if ((cou >= 6 && cou <= 7) && (mod >= 17 && mod <= 18)
               && (type >= Si_GG && type <= Si_T))
         det = (KVINDRADetector*)GetDetector("SIB_3");
   }
   return det;
}

