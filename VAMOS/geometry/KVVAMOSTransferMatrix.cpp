//Created by KVClassFactory on Fri Mar 29 08:56:52 2013
//Author: Guilain ADEMARD

#include "KVVAMOSTransferMatrix.h"
#include "KVUnits.h"
#include "KVDataSet.h"
#include "KVVAMOS.h"

#include "TEnv.h"

ClassImp(KVVAMOSTransferMatrix)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVVAMOSTransferMatrix</h2>
<h4>VAMOS's transformation matrix to map the measured coordinates at the focal plane back to the target</h4>
<!-- */
// --> END_HTML
//
// For each transmitted trajectory the parameters that are to be reconstructed
// are expressed as a set of four independent functions:
//
//     Brho  = F1( Xf, Yf, Thetaf, Phif)
//     Theta = F2( Xf, Yf, Thetaf, Phif)
//     Phi   = F3( Xf, Yf, Thetaf, Phif)
//     path  = F4( Xf, Yf, Thetaf, Phif)
//
// The four functions "F" are a polynomial of four variables (Xf, Yf, Thetaf, Phif)
// which have to be stored in a KVVAMOSReconTrajectory object.
// As a example, the expression for Brho is:
//Begin_Latex( fontsize=22 )
//    B_{#rho} = #sum_{i,j,k,l=0}^{i+j+k+l=n} C_{ijkl} X_{f}^{i} Y_{f}^{j} #theta_{f}^{k} #phi_{f}^{l}
//
//End_Latex
// where n is the order of the polynomial and Begin_Latex( fontsize=22 ) C_{ijkl} End_Latex are
// the coefficients related to the properties of the experimental system.
////////////////////////////////////////////////////////////////////////////////

void KVVAMOSTransferMatrix::init()
{
   //Initializing of data members
   fOrder = 0;
   for (UChar_t p = 0; p < 4; p++) {
      fNCoef[p] = 0;
      fCoef [p] = NULL;
      f_ijkl[p] = NULL;
   }
}
//________________________________________________________________

KVVAMOSTransferMatrix::KVVAMOSTransferMatrix(Bool_t scan_ds) : KVBase("VAMOSTransferMatrix", "Transfer Matrix of VAMOS")
{
   // Default constructor
   init();
   if (scan_ds) ReadCoefInDataSet();
}
//________________________________________________________________

KVVAMOSTransferMatrix::KVVAMOSTransferMatrix(const KVVAMOSTransferMatrix& obj)  : KVBase("VAMOSTransferMatrix", "Transfer Matrix of VAMOS")
{
   // Copy constructor
   // This ctor is used to make a copy of an existing object
   init();
   obj.Copy(*this);
}
//________________________________________________________________

KVVAMOSTransferMatrix::~KVVAMOSTransferMatrix()
{
   // Destructor
   DeleteMatrix();
}
//________________________________________________________________

void KVVAMOSTransferMatrix::Streamer(TBuffer& R__b)
{
   //Stream an object of class KVVAMOSTransferMatrix.
   //The coefficient arrays are streamed to be able to save correctly
   //a KVVAMOSTransferMatrix object in a ROOT file.

   if (R__b.IsReading()) {
      R__b.ReadClassBuffer(KVVAMOSTransferMatrix::Class(), this);
      for (UChar_t p = 0; p < 4; p++) {
         if (fNCoef[p]) {
            fCoef [p] = new Double_t[ fNCoef[p] ];
            f_ijkl[p] = new UShort_t[ fNCoef[p] ];
            R__b.ReadFastArray(fCoef [p], fNCoef[p]);
            R__b.ReadFastArray(f_ijkl[p], fNCoef[p]);
         }
      }
   } else {
      R__b.WriteClassBuffer(KVVAMOSTransferMatrix::Class(), this);
      for (UChar_t p = 0; p < 4; p++) {
         if (fNCoef[p]) {
            R__b.WriteFastArray(fCoef [p], fNCoef[p]);
            R__b.WriteFastArray(f_ijkl[p], fNCoef[p]);
         }
      }
   }
}
//________________________________________________________________

void KVVAMOSTransferMatrix::Copy(TObject& obj) const
{
   // This method copies the current state of 'this' KVVAMOSTransferMatrix
   // object into 'obj'.

   KVBase::Copy(obj);
   KVVAMOSTransferMatrix& CastedObj = (KVVAMOSTransferMatrix&)obj;
   CastedObj.fOrder = fOrder;
   CastedObj.DeleteMatrix();

   for (UChar_t p = 0; p < 4; p++) { //loop over parameters (Brho, theta, phi, path)

      CastedObj.fNCoef[p] = fNCoef[p];
      CastedObj.fCoef [p] = new Double_t[ fNCoef[p] ];
      CastedObj.f_ijkl[p] = new UShort_t[ fNCoef[p] ];
      for (UShort_t i = 0; i < fNCoef[p]; i++) {
         CastedObj.fCoef [p][i] = fCoef [p][i];
         CastedObj.f_ijkl[p][i] = f_ijkl[p][i];
      }
   }
}
//________________________________________________________________

void KVVAMOSTransferMatrix::NewMatrix()
{
   //Assignes dynamic memory to the array used to store the coefficients
   // of this matrix. The size of this array depends on the ordrer of the
   // matrix.
   //

   DeleteMatrix();

   UShort_t ncoef = GetMaxNcoef();
   for (UChar_t p = 0; p < 4; p++) { //loop over parameters (Brho, theta, phi, path)
      fNCoef[p] = ncoef;
      fCoef [p] = new Double_t[ ncoef ];
      f_ijkl[p] = new UShort_t[ ncoef ];
      for (UShort_t i = 0; i < ncoef; i++) { // loop over each coef.
         // initialize the coef.
         fCoef [p][i] = 0.;
         f_ijkl[p][i] = 0;
      }
   }
}
//________________________________________________________________

void KVVAMOSTransferMatrix::DeleteMatrix()
{
   //Deletes memory allocated for the array used to store the coefficients
   //of this matrix.

   for (UChar_t p = 0; p < 4; p++) { //loop over parameters (Brho, theta, phi, path)
      if (fCoef[p]) {
         delete[] fCoef[p];
         fCoef[p] = NULL;
      }
      if (f_ijkl[p]) {
         delete[] f_ijkl[p];
         f_ijkl[p] = NULL;
      }
      fNCoef[p] = 0;
   }
   ResetBit(kIsCompressed);
   ResetBit(kIsReady);
}
//________________________________________________________________

void KVVAMOSTransferMatrix::ReadCoefInDataSet()
{
   //Read the coefficients in files listed in another file set in the
   //dataset directory:
   //$KVROOT/KVFiles/<name_of_dataset>
   //
   // This last file is found and open from its name given by the following
   // environment variable defined in $KVROOT/KVFiles/.kvrootrc:
   //
   // KVVAMOSTransferMatrix.CoefficientFile:     VAMOStransMatrixCoef.list
   //
   // A file name specific to a given dataset may also be defined:
   //
   // INDRA_e494s.KVVAMOSTransferMatrix.CoefficientFile:   E494StransMatrixCoef.list
   //
   // where 'INDRA_e494s' is the name of the dataset in question.
   //
   // In this file, the order has to be given by:
   //
   // ORDER: 7
   //
   // To write the name of the file associated to a given parameter PARAM =  DELTA (Brho), THETA, PHI, PATH
   // use the line:
   //
   // PARAM: filename.dat
   //
   // The method ReadCoefficients(...) is called for each listed file.

   TEnv infos;
   TString path(gDataSet->GetDataSetDir());
   path += "/";
   TString envname = Form("%s.CoefficientFile", ClassName());
   path += gDataSet->GetDataSetEnv(envname.Data(), "VAMOStransMatrixCoef.list");
// Info("ReadCoefInDataSet"," reading file %s", path.Data() );

   infos.ReadFile(path.Data(), kEnvAll);

   // by default the order is seven
   if (!SetOrder(infos.GetValue("ORDER", 7))) {
      Error("ReadCoefInDataSet", "reading aborted");
      return;
   }

   ifstream ifile;
   TString fname;

   fname = infos.GetValue("DELTA", "BrhoReconCoef.cal");
   if (gDataSet->OpenDataSetFile(fname.Data(), ifile)) {
      Info("ReadCoefInDataSet", "reading DELTA Coefficients in %s", fname.Data());
      ReadCoefficients(kBrho, ifile);
      ifile.close();
   }

   fname = infos.GetValue("THETA", "ThetaReconCoef.cal");
   if (gDataSet->OpenDataSetFile(fname.Data(), ifile)) {
      Info("ReadCoefInDataSet", "reading THETA Coefficients in %s", fname.Data());
      ReadCoefficients(kTheta, ifile);
      ifile.close();
   }

   fname = infos.GetValue("PHI", "PhiReconCoef.cal");
   if (gDataSet->OpenDataSetFile(fname.Data(), ifile)) {
      Info("ReadCoefInDataSet", "reading PHI Coefficients in %s", fname.Data());
      ReadCoefficients(kPhi, ifile);
      ifile.close();
   }

   fname = infos.GetValue("PATH", "PhiReconCoef.cal");
   if (gDataSet->OpenDataSetFile(fname.Data(), ifile)) {
      Info("ReadCoefInDataSet", "reading PATH Coefficients in %s", fname.Data());
      ReadCoefficients(kPath, ifile);
      ifile.close();
   }

   // Reduce the size of the coefficient arrays
   CompressArrays();

   SetIsReady();
}
//________________________________________________________________

void KVVAMOSTransferMatrix::ReadCoefficients(EReconParam par, ifstream& file)
{
   // Reads the value of coefficients Begin_Latex( fontsize=22 ) C_{ijkl} End_Latex used to reconstruct the parameter 'par' in the file 'file'.
   // This method read the files given by Maurycy Rejmund obtained by using
   // the ZGOUBI code (F. Meot, Nucl. Instr. and Meth. A 427 (1999) 353.

   UShort_t nn[] = {0, 0, 0, 0, 0};
   UShort_t NtoRead = GetMaxNcoef();

   //skip the first line of the file which does not contain any coef.
   file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

   RecursiveReadOfCoef(par, file, fOrder, nn, NtoRead);
}
//________________________________________________________________

void KVVAMOSTransferMatrix::RecursiveReadOfCoef(EReconParam par, ifstream& file, UChar_t order, UShort_t* nijkl, UShort_t& NtoRead, UChar_t prev_i)
{
   // Method used by ReadCoefficients(...) to read coefficients in the file
   // having a specific format which is different of the one used to store the
   // coefficients in the array fCoef.
   //
   // Inputs:  par   - parameter associated to the coefficents to be read
   //                (kBrho, kTheta, kPhi, kPath)
   //          file  - file to be read
   //          order - function order in the recursive process
   //          nijkl - array of counters [5] to count the power of each
   //                  parameter (Xf, Yf, ThetaF, PhiF) i.e. to find the
   //                  index i,j,k,l for the coefficient Begin_Latex( fontsize=22 ) C_{ijkl} End_Latex
   //                  For the first call of this method, each element of 'n'
   //                  has to be equal zero.
   //        NtoRead - number of coefficients still to be read.
   //         prev_i - indice of the parameter at the previous order in the
   //                  recursive process. To be equal to 0 for the first call.

   order--;

   // loop over : 1, Xf, Yf, ThetaF, PhiF
   for (UChar_t i = prev_i; i < 5; i++) {
      nijkl[i]++;
      if (order) RecursiveReadOfCoef(par, file, order, nijkl, NtoRead, i);
      else {
         if (!NtoRead) {
            Error("RecursiveReadOfCoef", "Recursive processing not ended but there is not coefficient to be read anymore!");
            return;
         }
         NtoRead--;
         file >> fCoef[par][NtoRead];

         UShort_t* ijkl = nijkl + 1;
         f_ijkl[par][NtoRead] = Code_ijkl(ijkl[0],  ijkl[1], ijkl[2], ijkl[3]);
      }
      nijkl[i]--;
   }
}
//________________________________________________________________

void KVVAMOSTransferMatrix::CompressArrays()
{
   // Reduces the size of the coefficient arrays by removing the null
   // coefficients not used in calculations.

   if (TestBit(kIsCompressed)) {
      Warning("CompressArrays", "Compression already done");
      return;
   }

   // new compressed arrays
   UShort_t Ncoef[4];
   Double_t* coef[4];
   UShort_t* ijkl[4];

   //loop over each parameter
   for (UChar_t p = 0; p < 4; p++) {
      Ncoef[p] = 0;

      //Count the number of coefficients not null
      for (UShort_t i = 0; i < fNCoef[p]; i++) {
         if (fCoef[p][i] != 0) Ncoef[p]++;
      }

      // new arrays to store only not null coefficients
      coef[p] = new Double_t[ Ncoef[p] ];
      ijkl[p] = new UShort_t[ Ncoef[p] ];
      Int_t j = 0;
      for (UShort_t i = 0; i < fNCoef[p]; i++) {
         //only fill the new arrays with not null coefficients
         if (fCoef[p][i] != 0) {
            coef[p][j] = fCoef [p][i];
            ijkl[p][j] = f_ijkl[p][i];
            j++;
         }
      }

      //delete and replace the old arrays
      delete[] fCoef [p];
      delete[] f_ijkl[p];

      fNCoef[p] = Ncoef[p];
      fCoef [p] = coef [p];
      f_ijkl[p] = ijkl [p];
   }

   SetBit(kIsCompressed);
}
//________________________________________________________________

Bool_t KVVAMOSTransferMatrix::ReconstructFPtoLab(KVVAMOSReconTrajectory* traj)
{
   // Reconstruction of the trajectory at the target point, in the reference
   // frame of the laboratory, from the trajectory at the focal plane.
   // Then in the object 'traj' the focal plane parameters have to be given
   // i.e. traj->FPparamsAreReady() has to return true.
   //
   // the result is store in the 'traj' object and the method returns true
   // if the attempt is a success.

   Bool_t ok = kFALSE;

   if (!traj->FPparamsAreReady()) {
      Error("ReconstructFPtoLab", "Focal plane position parameters are not ready to reconstruct the trajectory");
      return ok;
   }
   if (!IsReady()) {
      Error("ReconstructFPtoLab", "Not ready, the coefficients are missing");
      return ok;
   }

   Double_t  XYTPf[4];
   Double_t  XYTPf_phi[4];
   Double_t* xytpf[4];

   // Xf, Yf, ThetaF and PhiF for the reconstruction of Brho, Theta and Path
   XYTPf[0] = -1.*traj->pointFP[0] / KVUnits::m;        //Xf in m
   XYTPf[1] =  0.;                                      //Yf in m
   XYTPf[2] = -1.*traj->GetThetaF() * TMath::DegToRad(); //ThetaF in rad
   XYTPf[3] =  0.;                                      //PhiF in rad
   xytpf[kBrho] = xytpf[kTheta] = xytpf[kPath] = XYTPf;


   // Xf, Yf, ThetaF and PhiF for the reconstruction of Phi
   XYTPf_phi[0] = XYTPf[0];                              //Xf in m
   XYTPf_phi[1] = -1.*traj->pointFP[1] / KVUnits::m;     //Yf in m
   XYTPf_phi[2] = XYTPf[2];                              //ThetaF in rad
   XYTPf_phi[3] = -1.*traj->GetPhiF() * TMath::DegToRad(); //PhiF in rad
   xytpf[kPhi]  = XYTPf_phi;


   Double_t res[4] = {0, 0, 0, 0};
   UShort_t i, j, k, l;
   for (UChar_t p = 0; p < 4; p++) { //loop over parameters (Brho, theta, phi, path)

      //loop over the coefficients for each parameter
      for (UShort_t c = 0; c < fNCoef[p]; c++) {

         Decode_ijkl(f_ijkl[p][c], i, j, k, l);

         res[p] += fCoef[p][c]                //Cijkl
                   *TMath::Power(xytpf[p][0] , i)   //Xf^i
                   *TMath::Power(xytpf[p][1] , j)   //Yf^j
                   *TMath::Power(xytpf[p][2] , k)   //ThetaF^k
                   *TMath::Power(xytpf[p][3] , l);  //PhiF^l
      }
   }

   // the reconstruction is OK if it satisfies the condition:
   //  Brho > 0.001        T.m
   //  -300 < Theta < 300  mrad
   //  -300 < Phi   < 300  mrad
   //     0 < path  < 2000 cm
   if (res[kBrho] > 0.001
         &&
         -300. < res[kTheta] && res[kTheta] < 300.
         &&
         -300. < res[kPhi] && res[kPhi] < 300.
         &&
         0 < res[kPath] && res[kPath] < 2000.
      ) {
      traj->path = res[kPath];                        //in cm
      traj->Brho = res[kBrho] * gVamos->GetBrhoRef(); //in Tm

      Double_t theta = - res[kTheta] / 1000; // in rad
      Double_t phi   = - res[kPhi  ] / 1000; // in rad

      Double_t X     = TMath::Sin(theta) * TMath::Cos(phi);
      Double_t Y     = TMath::Sin(phi);
      Double_t Z     = TMath::Cos(theta) * TMath::Cos(phi);

      traj->dirLab.SetXYZ(X, Y, Z);
      traj->dirLab.RotateY(gVamos->GetAngle()*TMath::DegToRad());

      ok = kTRUE;
   }

   traj->SetFPtoLabAttempted();

   return ok;
}
//________________________________________________________________

Bool_t KVVAMOSTransferMatrix::ReconstructFPtoLab(Double_t x_f, Double_t y_f, Double_t theta_f, Double_t phi_f, Double_t& brho, Double_t& path, Double_t& theta_v, Double_t& phi_v)
{
   // x_f and y_f in cm.
   // phi_f, theta_f, phi_l and theta_l in degree.

   static KVVAMOSReconTrajectory traj;

   traj.pointFP[0] = x_f;
   traj.pointFP[1] = y_f;
   Double_t x = TMath::Sin(theta_f * TMath::DegToRad()) * TMath::Cos(phi_f * TMath::DegToRad());
   Double_t y = TMath::Sin(phi_f * TMath::DegToRad());
   Double_t z = TMath::Cos(theta_f * TMath::DegToRad()) * TMath::Cos(phi_f * TMath::DegToRad());
   traj.dirFP.SetXYZ(x, y, z);
   traj.SetFPparamsReady();

   Bool_t status = ReconstructFPtoLab(&traj);

   path    = traj.GetPath();
   brho    = traj.GetBrho();
   theta_v = traj.GetThetaV();
   phi_v   = traj.GetPhiV();

   return status;
}
//________________________________________________________________

Bool_t KVVAMOSTransferMatrix::ReconstructLabToFP(KVVAMOSReconTrajectory* traj)
{
   // Reconstruction of the trajectory at the focal plan from the trajectory
   // at the target point in the reference frame of the laboratory.
   // Then in the object 'traj' the laboratory parameters have to be given
   // i.e. traj->LabParamsAreReady() has to return true.
   //
   // the result is store in the 'traj' object and the method returns true
   // if the attempt is a success.
   //
   // METHOD TO BE IMPLEMENTED

   UNUSED(traj);
   Warning("ReconstructLabToFP", "To be implemented");
   return kFALSE;
}
//________________________________________________________________

void KVVAMOSTransferMatrix::SetCoef(EReconParam par, UShort_t i, UShort_t j, UShort_t k, UShort_t l, Double_t coef)
{
   // Sets the value of the coefficient Begin_Latex( fontsize=22 ) C_{ijkl} End_Latex used to reconstruct the parameter 'par'.
   //
   // par = kBrho, kTheta, kPhi, kPath

   if (TestBit(kIsCompressed)) {
      Error("SetCoef", "too late to set coefficient, compression already done");
      return;
   }

   if (i <= fOrder && j <= fOrder && k <= fOrder && l <= fOrder) {

      for (UShort_t c = 0; c < fNCoef[ par ]; c++) {

         //replace a null coefficient by the new one
         if (fCoef[par][c] == 0) {
            fCoef[par][c] = coef;
            f_ijkl[par][c] = Code_ijkl(i, j, k, l);
            return;
         }
      }
      Warning("SetCoef", "you can not set coefficient anymore. The arrays are full");
   } else Error("SetCoef", " one of the indexes (i,j,k,l) is greater than the order");
}
//________________________________________________________________

Bool_t KVVAMOSTransferMatrix::SetOrder(UChar_t order)
{
   //Set the order of the polynomials.
   //An order greater than 15 can not be processed by this class.

   if (order > 15) {
      Error("SetOrder", "order %d greater than 15 is not allowed", order);
      return kFALSE;
   }
   fOrder = order;
   NewMatrix();
   return kTRUE;
}
//________________________________________________________________

void KVVAMOSTransferMatrix::Print(Option_t* option) const
{
   //Prints informations about this object.
   //Use the option="coef" if you want to print the coefficients for each
   //parameters

   cout << ClassName() << " object: Name=" << GetName() << " Title=" << GetTitle();
   if (strcmp(GetLabel(), ""))
      cout << " Label=" << GetLabel();
   if (GetNumber() != 0)
      cout << " Number=" << GetNumber();
   cout << " Order= " << (Int_t)GetOrder();
   cout << endl;

   if (strcmp(option, "coef") || !fOrder) return;


   TString par;
   UShort_t i, j, k, l;

   for (UChar_t p = 0; p < 4; p++) {
      switch (p) {
         case kBrho:
            par = "Brho";
            break;
         case kTheta:
            par = "Theta";
            break;
         case kPhi:
            par = "Phi";
            break;
         case kPath:
            par = "Path";
            break;

      }
      cout << endl << setw(8) << par.Data() << ": " << endl;

      for (UShort_t c = 0; c < fNCoef[p]; c++) {

         Decode_ijkl(f_ijkl[p][c], i, j, k, l);
         cout << setw(9) << " " << Form("Coef(%d,%d,%d,%d) = %f", i, j, k, l, fCoef[p][c]) << endl;
      }

      cout << setw(9) << " " << "with " << fNCoef[p] << " coefficients stored over " << GetMaxNcoef() << endl;
      cout << endl;
   }
}
