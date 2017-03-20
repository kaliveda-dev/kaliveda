//Created by KVClassFactory on Tue Feb 14 10:53:57 2017
//Author: Patrick St-Onge,,,

#include "KVZGOUBIReconstruction.h"

ClassImp(KVZGOUBIReconstruction)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVZGOUBITrajectory</h2>
<h4>Class used to access one ZGOUBI Trajectory</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVZGOUBIReconstruction::KVZGOUBIReconstruction()
   : KVBase()
{
   // Default constructor
   Matrix_2D = NULL;
   Matrix_4D = NULL;
}

//____________________________________________________________________________//

KVZGOUBIReconstruction::KVZGOUBIReconstruction(Bool_t init)
   : KVBase()
{
   Matrix_2D = NULL;
   Matrix_4D = NULL;
   if (init == true) Init();
}

//____________________________________________________________________________//


KVZGOUBIReconstruction::KVZGOUBIReconstruction(const KVZGOUBIReconstruction& obj) : KVBase()
{
   // Copy constructor
   // Use this constructor to copy an existing object
   obj.Copy(*this);
}

//____________________________________________________________________________//

KVZGOUBIReconstruction::KVZGOUBIReconstruction(const Char_t* name, const Char_t* title)
   : KVBase(name, title)
{
   // Constructor inherited from KVBase
}

//____________________________________________________________________________//

KVZGOUBIReconstruction::~KVZGOUBIReconstruction()
{
   // Destructor
}

//____________________________________________________________________________//

void KVZGOUBIReconstruction::Copy(TObject& obj) const
{
   // This method copies the current state of 'this' KVZGOUBITrajectory
   // object into 'obj'.

   KVBase::Copy(obj);
   KVZGOUBIReconstruction& CastedObj = (KVZGOUBIReconstruction&)obj;
   CastedObj.Matrix_2D = Matrix_2D ;
   CastedObj.Matrix_4D = Matrix_4D ;

}
//____________________________________________________________________________//

void KVZGOUBIReconstruction::Init()
{
   TString hardcoded_datasetsubdir = "INDRA_e494s";


   TString filename4D = gDataSet->GetDataSetEnv("KVZGOUBIReconstruction.ZGOUBIDatabase4D");
   TString filename_localdatabase4D = gDataSet->GetDataSetEnv("KVZGOUBIReconstruction.ZGOUBIDatabase_local4D");
   if (filename4D == "" && filename_localdatabase4D == "") {
      Warning("SetZGOUBITFile_fromdataset", "No filename defined. Should be given by %s.KVZGOUBIInverseMatrix.ZGOUBIDatabase or %s.KVZGOUBIInverseMatrix.ZGOUBIDatabase_local", gDataSet->GetName(), gDataSet->GetName());
      return;
   } else {
      Bool_t rootfiles_status = true;
      Int_t rootfile_nb = 1;
      TString rootfile_name;
      TString fullpath;
      TChain* t = new TChain("tree");
      if (filename_localdatabase4D != "") {
         std::cout << "Local DataBase for 4D:" << filename_localdatabase4D << std::endl;
         t->Add(filename_localdatabase4D.Data());
         std::cout << "Trajectories:" << t->GetEntries() << std::endl;
      } else {
         while (rootfiles_status == true) {
            rootfile_name.Form("%s_%i.root", filename4D.Data(), rootfile_nb);
            if (gDataSet->SearchKVFile(rootfile_name.Data(), fullpath, hardcoded_datasetsubdir.Data())) {
               std::cout << "DataSet DataBase for 4D:" << fullpath << std::endl;
               t->Add(fullpath.Data());
               rootfile_nb++;
            } else {
               rootfiles_status = false;
            }
         }
      }
      Matrix_4D = new KVZGOUBIInverseMatrix(250, 250, 1, 1, t);
      delete t;
   }


   TString filename2D = gDataSet->GetDataSetEnv("KVZGOUBIReconstruction.ZGOUBIDatabase2D");
   TString filename_localdatabase2D = gDataSet->GetDataSetEnv("KVZGOUBIReconstruction.ZGOUBIDatabase_local2D");
   if (filename2D == "" && filename_localdatabase2D == "") {
      Warning("SetZGOUBITFile_fromdataset", "No filename defined. Should be given by %s.KVZGOUBIInverseMatrix.ZGOUBIDatabase or %s.KVZGOUBIInverseMatrix.ZGOUBIDatabase_local", gDataSet->GetName(), gDataSet->GetName());
      return;
   } else {
      Bool_t rootfiles_status = true;
      Int_t rootfile_nb = 1;
      TString rootfile_name;
      TString fullpath;
      TChain* t = new TChain("tree");
      if (filename_localdatabase2D != "") {
         std::cout << "Local DataBase for 2D:" << filename_localdatabase2D << std::endl;
         t->Add(filename_localdatabase2D.Data());
         std::cout << "Trajectories:" << t->GetEntries() << std::endl;
      } else {
         while (rootfiles_status == true) {
            rootfile_name.Form("%s_%i.root", filename2D.Data(), rootfile_nb);
            if (gDataSet->SearchKVFile(rootfile_name.Data(), fullpath, hardcoded_datasetsubdir.Data())) {
               std::cout << "DataSet DataBase for 2D:" << fullpath << std::endl;
               t->Add(fullpath.Data());
               rootfile_nb++;
            } else {
               rootfiles_status = false;
            }
         }
      }
      Matrix_2D = new KVZGOUBIInverseMatrix(250, 250, 1, 1, t);
      delete t;
   }
}
//____________________________________________________________________________//

Bool_t KVZGOUBIReconstruction::ReconstructFPtoLab(KVVAMOSReconTrajectory* traj)
{
   // Reconstruction of the trajectory at the target point, in the reference
   // frame of the laboratory, from the trajectory at the focal plane.
   // Then in the object 'traj' the focal plane parameters have to be given
   // i.e. traj->FPparamsAreReady() has to return true.
   //
   // The result is stored in the 'traj' object and the method returns true
   // if the attempt is a success.

   Bool_t ok = kFALSE;

   if (!traj->FPparamsAreReady()) {
      Error("ReconstructFPtoLab", "Focal plane position parameters are not ready to reconstruct the trajectory");
      return ok;
   }


   Float_t XFt = (Float_t) - 1.*traj->pointFP[0];
   Float_t YFt = (Float_t) - 1.*traj->pointFP[1];
   Float_t ThetaFt = (Float_t) - 1.*traj->GetThetaF() * TMath::DegToRad() * 1000.;
   Float_t PhiFt = (Float_t) - 1.*traj->GetPhiF() * TMath::DegToRad() * 1000.;

   /*
   std::vector<Float_t> results_identification=testGetLinesinRadius_weight(XFt, ThetaFt, YFt, PhiFt,1.1);
   //std::cout<<"out testGetLinesinRadius_weight"<<std::endl;
   if(results_identification[9]>0)
     {
       std::cout<<results_identification[7]<<std::endl;
       traj->path=(Double_t) results_identification[7];
       traj->Brho=(Double_t) results_identification[6] * gVamos->GetBrhoRef();

       Double_t theta = (Double_t) -1.*results_identification[4] / 1000.; // in rad
       Double_t phi   = (Double_t) -1.*results_identification[5] / 1000.; // in rad
       Double_t X     = TMath::Sin(theta) * TMath::Cos(phi);
       Double_t Y     = TMath::Sin(phi);
       Double_t Z     = TMath::Cos(theta) * TMath::Cos(phi);

       traj->dirLab.SetXYZ(X, Y, Z);
       traj->dirLab.RotateY(gVamos->GetAngle()*TMath::DegToRad());

       ok = kTRUE;
     }
   */
   /*
   Matrix_2D->Setcharacteristicdistance(1, 1, 1000000, 1000000);
   Int_t nearestlinenb = Matrix_2D->GetNearestLinenb(XFt, ThetaFt, YFt, PhiFt);
   if (nearestlinenb >= 0) {
     KVZGOUBITrajectory NearestLine2D = Matrix_2D->GetZGOUBITrajectory(nearestlinenb);

     Matrix_4D->Setcharacteristicdistance(1, 1, 1, 1);
     nearestlinenb = Matrix_4D->GetNearestLinenb(XFt, ThetaFt, YFt, PhiFt);
     if (nearestlinenb >= 0) {
       traj->path = (Double_t) NearestLine2D.GetPath();
       traj->Brho = (Double_t) NearestLine2D.GetDelta() * gVamos->GetBrhoRef();
       Double_t theta = (Double_t) - 1.*NearestLine2D.GetThetaV() / 1000.; // in rad
       KVZGOUBITrajectory NearestLine4D = Matrix_4D->GetZGOUBITrajectory(nearestlinenb);
       Double_t phi   = (Double_t) - 1.*NearestLine4D.GetPhiV() / 1000.; // in rad
       Double_t X     = TMath::Sin(theta) * TMath::Cos(phi);
       Double_t Y     = TMath::Sin(phi);
       Double_t Z     = TMath::Cos(theta) * TMath::Cos(phi);
       traj->dirLab.SetXYZ(X, Y, Z);
       traj->dirLab.RotateY(gVamos->GetAngle()*TMath::DegToRad());
       ok = kTRUE;
     }
   }
   */


   Matrix_2D->Setcharacteristicdistance(1, 1, 1000000, 1000000);
   std::vector<Float_t> results_identification2D = Matrix_2D->testGetResults_weight_comb(XFt, ThetaFt, YFt, PhiFt, 10);
   if (results_identification2D[9] > 0) {
      Matrix_4D->Setcharacteristicdistance(1, 1, 1, 1);
      std::vector<Float_t> results_identification4D = Matrix_4D->testGetResults_weight_comb(XFt, ThetaFt, YFt, PhiFt, 10);
      if (results_identification4D[9] > 0) {
         traj->path = (Double_t) results_identification2D[7];
         traj->Brho = (Double_t) results_identification2D[6] * gVamos->GetBrhoRef();
         Double_t theta = (Double_t) - 1.*results_identification2D[4] / 1000.; // in rad
         Double_t phi   = (Double_t) - 1.*results_identification4D[5] / 1000.; // in rad
         Double_t X     = TMath::Sin(theta) * TMath::Cos(phi);
         Double_t Y     = TMath::Sin(phi);
         Double_t Z     = TMath::Cos(theta) * TMath::Cos(phi);

         traj->dirLab.SetXYZ(X, Y, Z);
         traj->dirLab.RotateY(gVamos->GetAngle()*TMath::DegToRad());
         ok = kTRUE;
      }
   }
   traj->SetFPtoLabAttempted();
   return ok;
}
