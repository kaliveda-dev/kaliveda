//Created by KVClassFactory on Thu Mar 19 12:53:22 2020
//Author: John Frankland,,,

#include "KVFAZIAMidRapExplorer.h"
#include "KVFAZIABeltConfig.h"

ClassImp(KVFAZIAMidRapExplorer)

void KVFAZIAMidRapExplorer::BuildFAZIA()
{
   KVFAZIABeltConfig* belt_left =
      new KVFAZIABeltConfig(fBlockParams[0].fNblocsX, fBlockParams[0].fNblocsY,
                            fBlockParams[0].fBeltDistanceTarget, fBlockParams[0].fInterBlockSpacing,
                            -fBlockParams[0].fDistanceOffset);
   TGeoVolume* top = gGeoManager->GetTopVolume();
   top->AddNode(belt_left, 0,
                GetVolumePositioningMatrix(fBlockParams[0].fBeltDistanceTarget, fBlockParams[0].fThetaBelt, fBlockParams[0].fPhiBelt));
   KVFAZIABeltConfig* belt_right =
      new KVFAZIABeltConfig(fBlockParams[1].fNblocsX, fBlockParams[1].fNblocsY,
                            fBlockParams[1].fBeltDistanceTarget, fBlockParams[1].fInterBlockSpacing,
                            fBlockParams[1].fDistanceOffset);
   top->AddNode(belt_right, 1,
                GetVolumePositioningMatrix(fBlockParams[1].fBeltDistanceTarget, fBlockParams[1].fThetaBelt, fBlockParams[1].fPhiBelt));
}

void KVFAZIAMidRapExplorer::GetGeometryParameters()
{
   for (int ibelt = 0; ibelt < 2; ++ibelt) {
      fBlockParams[ibelt].fNblocsX = (int)GetDataSetEnv(fDataSet, Form("KVFAZIAMidRapExplorer.BELT%d.BlocksX", ibelt), 0.0);
      fBlockParams[ibelt].fNblocsY = (int)GetDataSetEnv(fDataSet, Form("KVFAZIAMidRapExplorer.BELT%d.BlocksY", ibelt), 0.0);
      fBlockParams[ibelt].fBeltDistanceTarget = GetDataSetEnv(fDataSet, Form("KVFAZIAMidRapExplorer.BELT%d.BeltDistanceTarget", ibelt), -1.0);
      fBlockParams[ibelt].fThetaBelt = GetDataSetEnv(fDataSet, Form("KVFAZIAMidRapExplorer.BELT%d.ThetaBelt", ibelt), -1.0);
      fBlockParams[ibelt].fPhiBelt = GetDataSetEnv(fDataSet, Form("KVFAZIAMidRapExplorer.BELT%d.PhiBelt", ibelt), -1.0);
      fBlockParams[ibelt].fInterBlockSpacing = GetDataSetEnv(fDataSet, Form("KVFAZIAMidRapExplorer.BELT%d.InterBlockSpacing", ibelt), -1.0);
      fBlockParams[ibelt].fDistanceOffset = GetDataSetEnv(fDataSet, Form("KVFAZIAMidRapExplorer.BELT%d.DistanceOffset", ibelt), -1.0);
   }
   SetGeometryImportParameters(0.1, 1, TMath::Min(fBlockParams[0].fThetaBelt, fBlockParams[1].fThetaBelt) - 12, 0,
                               TMath::Max(fBlockParams[0].fThetaBelt, fBlockParams[1].fThetaBelt) + 12);

}

void KVFAZIAMidRapExplorer::SetNameOfDetectors(KVEnv& env)
{
   //define the format of detectors name
   // label-index
   // where index = block*100+quartet*10+telescope
   // example :
   // SI1-123 is the Silicon 1 of the block 1, the quartet 2 and the telescope 3
   // block number increases continuously from 1st block of belt 0 to last block of belt 1

   int block_offset = 0;
   for (int belt = 0; belt <= 1; ++belt) {
      for (Int_t bb = 0; bb < fBlockParams[belt].fNblocsX * fBlockParams[belt].fNblocsY; bb += 1) {
         for (Int_t qq = 1; qq <= 4; qq += 1) {
            for (Int_t tt = 1; tt <= 4; tt += 1) {
               fDetectorLabels.Begin(",");
               while (!fDetectorLabels.End()) {
                  KVString sdet = fDetectorLabels.Next();
                  env.SetValue(
                     Form("BELT_%d_BLOCK_%d_QUARTET_%d_%s-T%d", belt, bb, qq, sdet.Data(), tt),
                     Form("%s-%d", sdet.Data(), (bb + block_offset) * 100 + 10 * qq + tt)
                  );
               }
            }
         }
      }
      block_offset += fBlockParams[belt].fNblocsX * fBlockParams[belt].fNblocsY;
   }
}

