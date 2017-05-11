//Created by KVClassFactory on Thu Feb  9 00:40:28 2017
//Author: Quentin Fable,,,

#include "KVVAMOSWeightFinder.h"
#include "KVDataSet.h"
#include "TObjArray.h"
#include "TObjString.h"
#include "Riostream.h"
#include "TLeaf.h"

ClassImp(KVVAMOSWeightFinder)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVVAMOSWeightFinder</h2>
<h4>Class used to estimate normalisation weights for VAMOS events</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////
KVVAMOSWeightFinder* gVAMOSWeightFinder = 0;

KVVAMOSWeightFinder::KVVAMOSWeightFinder(Int_t run_number = -1) : KVBase("VAMOSWeightFinder", "Normalisation weight estimator of VAMOS events")
{
   // Default constructor
   fRunNumber = run_number;
   fkIsInit   = kFALSE;
   fkverbose  = kFALSE;

   fchain = new TChain("tree");
}
//____________________________________________________________________________//

KVVAMOSWeightFinder::KVVAMOSWeightFinder(const KVVAMOSWeightFinder& obj) : KVBase("VAMOSWeightFinder", "Normalisation weight estimator of VAMOS events")
{
   // Copy constructor
   // Use this constructor to copy an existing object
   obj.Copy(*this);
}

//____________________________________________________________________________//

KVVAMOSWeightFinder::~KVVAMOSWeightFinder()
{
   // Destructor
   fRunList.Clear();
   fCompRunList.Clear();
   fvec_infos.clear();
   fvec_TCfiles.clear();
   fvec_TCsteps.clear();

   delete fchain;
   fchain = NULL;

   if (gVAMOSWeightFinder == this) gVAMOSWeightFinder = 0;
}
//____________________________________________________________________________//

void KVVAMOSWeightFinder::Copy(TObject& obj) const
{
   // This method copies the current state of 'this' KVVAMOSWeightFinder
   // object into 'obj'.

   KVBase::Copy(obj);
   KVVAMOSWeightFinder& CastedObj = (KVVAMOSWeightFinder&)obj;
   CastedObj.fRunNumber = fRunNumber;
   CastedObj.fRunList = fRunList;
   CastedObj.fCompRunList = fCompRunList;
   CastedObj.fvec_infos = fvec_infos;
   CastedObj.fvec_TCfiles = fvec_TCfiles;
   CastedObj.fvec_TCsteps = fvec_TCsteps;
   CastedObj.fchain = fchain;
}
//____________________________________________________________________________//
void KVVAMOSWeightFinder::cd()
{
   //Make this the default batch system
   gVAMOSWeightFinder = this;
}

void KVVAMOSWeightFinder::SetRunList(KVNumberList& nl)
{
   fRunList.Clear();
   fRunList = nl;
   fCompRunList = nl;
}

//____________________________________________________________________________//

void KVVAMOSWeightFinder::Init()
{
   //Initialise the weight finder.
   //Need to have set the run list correctly first...

   if ((!fRunList.IsEmpty()) && (fRunNumber > 0) && (fRunList.Contains(fRunNumber))) {
      ReadRunListInDataSet();
      SortInfoVector();
      ReadTransCoefFileListInDataSet();
      if (CheckTransCoefSteps()) fkIsInit = kTRUE;
   } else fkIsInit = kFALSE;

   if (!fkIsInit) Warning("Init", "... problem in initialisation: runlist is empty OR/AND current run number<0 OR/AND run list doesn't contain the current run ...");
   return;
}
//____________________________________________________________________________//

void KVVAMOSWeightFinder::ReadRunListInDataSet()
{
   // Read the experimental informations listed for the given run list
   // in file set in the dataset directory:
   // $KVROOT/KVFiles/<name_of_dataset>
   //
   // This file, specific to a given dataset, is found and
   // opened from its name given by the following environment
   // variable defined in $KVROOT/KVFiles/.kvrootrc:
   //
   // [dataset_name].KVVAMOSWeightFinder.RunListFile:  [file_name]
   //
   // This file contains a list of: Run, Brho, ThetaVamos, DT, Scalers

   //Find the file to use
   TString filename = gDataSet->GetDataSetEnv("KVVAMOSWeightFinder.RunListFile");
   if (filename == "") {
      Warning("ReadRunListInDataSet", "... No filename defined. Should be given by %s.KVVAMOSWeightFinder.RunListFile ...", gDataSet->GetName());
      return;
   }
   std::ifstream ifile;
   if (gDataSet->OpenDataSetFile(filename.Data(), ifile)) {
      if (fkverbose) Info("ReadRunListInDataSet", "... run list informations in: '%s' ...", filename.Data());
      ReadInformations(ifile);
      ifile.close();
   }
}
//____________________________________________________________________________//

void KVVAMOSWeightFinder::ReadInformations(std::ifstream& file)
{
   // Read informations in run list file and save them only for
   // the chosen runs in 'nl'.

   Float_t run;
   Float_t brho;
   Float_t vamos_angle;
   Float_t dt;
   Float_t scalers;

   std::string newline;
   while (std::getline(file, newline)) {
      //Ignore comments in the file
      if (newline.compare(0, 1, "#") != 0) {
         sscanf(newline.c_str(), "%f %f %f %f %f", &run, &brho, &vamos_angle, &dt, &scalers);

         if (fRunList.Contains((Int_t) run)) {
            std::vector<Float_t> line;
            line.push_back(run);
            line.push_back(brho);
            line.push_back(vamos_angle);
            line.push_back(dt);
            line.push_back(scalers);
            fvec_infos.push_back(line);

            fCompRunList.Remove((Int_t) run);
         }
      }
   }

   //look at complementary list:
   //any run in this list doesn't exist in the
   //experimental run list file
   if (!fCompRunList.IsEmpty()) {
      if (fkverbose) Info("ReadInformations", "... the following runs don't exist in data set run list file and will be ignored:\n%s", fCompRunList.GetList());
   }
}
//____________________________________________________________________________//

void KVVAMOSWeightFinder::SortInfoVector()
{
   //Sort all info vectors contained in fvec_infos
   //according to their run number in ascending order.
   //It ensures that the position of the run in fRunList
   //is the same than in fvec_infos.

   std::sort(fvec_infos.begin(), fvec_infos.end(), KVVAMOSWeightFinder::SortVectors);

   if (fkverbose) Info("SortLineVector", "... informations saved in ascending order of run ...");
}
//____________________________________________________________________________//

Bool_t KVVAMOSWeightFinder::SortVectors(const std::vector<Float_t>& vector1, const std::vector<Float_t>& vector2)
{
   //Loop over and sort vectors
   //in ascending order of their first value (so run number).

   //different run
   if (vector1.at(0) != vector2.at(0)) {
      if (vector1.at(0) < vector2.at(0)) return kTRUE;
      else return kFALSE;
   }

   return kTRUE;
}
//____________________________________________________________________________//

Int_t KVVAMOSWeightFinder::GetRunPosition(Int_t run_number)
{
   //Returns the position of the run in fvec_infos/fRunList
   Int_t pos = -1;

   Bool_t found = kFALSE;
   Int_t next_pos = 0;
   fRunList.Begin();
   while (!fRunList.End() && !found) {
      Int_t next_val = fRunList.Next();
      if ((next_val == run_number) && (!fCompRunList.Contains(next_val))) {
         pos = next_pos;
         found = kTRUE;
      }

      next_pos++;
   }

   return pos;
}

//____________________________________________________________________________//
Float_t KVVAMOSWeightFinder::GetInfoValue(Int_t run_number, Int_t num_value)
{
   //Returns the value of the given information for the given run.
   Float_t val = -666.;

   Int_t pos = GetRunPosition(run_number);

   if (pos > 0) val = GetInfoLine(pos).at(num_value);

   return val;
}

//____________________________________________________________________________//
void KVVAMOSWeightFinder::ReadTransCoefFileListInDataSet()
{
   // Read the trans. coef. files from
   // a file set in the dataset directory:
   // $KVROOT/KVFiles/<name_of_dataset>
   //
   // This file, specific to a given dataset, is found and
   // opened from its name given by the following environment
   // variable defined in $KVROOT/KVFiles/.kvrootrc:
   //
   // [dataset_name].KVVAMOSWeightFinder.TransCoefFileList:  [file_name]
   //
   // This file contains a list of trans. coef. *.root files, each
   // of them associated to a ThetaVamos. Each of these *.root files
   // contains more specificly a TTree with trans.coef values and a TEnv
   // of (delta, thetaI) ranges and steps, mandatory to use the TTree.

   //Find the list file to use
   TString filename = gDataSet->GetDataSetEnv("KVVAMOSWeightFinder.TransCoefFileList");
   if (filename == "") {
      Warning("ReadTransCoefFilesInDataSet", "No filename defined. Should be given by %s.KVVAMOSWeightFinder.TransCoefFileList", gDataSet->GetName());
      return;
   }
   std::ifstream ifile;
   if (gDataSet->OpenDataSetFile(filename.Data(), ifile)) {
      if (fkverbose) Info("ReadTransCoefFilesInDataSet", "trans. coef. list of files in:  '%s' ", filename.Data());
      ReadTransCoefListFile(ifile);
      ifile.close();
   }
}

//____________________________________________________________________________//
void KVVAMOSWeightFinder::ReadTransCoefListFile(std::ifstream& file)
{
   //Read the trans. coef. list file. for the data set
   //For each file listed in this list file we:
   //- save in 'fvec_TCfiles' a pair (VamosAngle, file_name) containing the VamosAngle and its associated file
   //- read the file TEnv and save steps informations in 'fvec_TCsteps'
   //- Add the tree in fchain

   std::string newline;
   while (std::getline(file, newline)) {
      //Ignore comments in the file
      if (newline.compare(0, 1, "#") != 0) {

         TString ss(newline.c_str());
         TObjArray* obj_arr = ss.Tokenize(" ");
         obj_arr->ls();

         TString str_va = ((TObjString*) obj_arr->At(0))->GetString();
         TString str_na = ((TObjString*) obj_arr->At(1))->GetString();

         fvec_TCfiles.push_back(std::make_pair(str_va.Atof(), str_na));

         TString fullpath;
         TFile* file = NULL;
         if (gDataSet->SearchKVFile(str_na.Data(), fullpath, gDataSet->GetName())) {
            file = TFile::Open(fullpath.Data());
            if (file) {

               if (fkverbose) {
                  Info("ReadTransCoefListFile", "... file %s opened, infos follow ...", file->GetName());
                  file->ls();
               }

               //find the TEnv and fill the vector with step values
               TEnv* env = (TEnv*) file->Get("TEnv");
               assert(env);
               std::vector<Float_t> line;
               line.push_back(env->GetValue("ThetaVamos_deg", (Float_t) - 1));
               line.push_back(env->GetValue("delta_min", (Float_t) - 1));
               line.push_back(env->GetValue("delta_max", (Float_t) - 1));
               line.push_back(env->GetValue("delta_step", (Float_t) - 1));
               line.push_back(env->GetValue("delta_Nstep", (Float_t) - 1));
               line.push_back(env->GetValue("thetaI_min_rad", (Float_t) - 1));
               line.push_back(env->GetValue("thetaI_max_rad", (Float_t) - 1));
               line.push_back(env->GetValue("thetaI_step_rad", (Float_t) - 1));
               line.push_back(env->GetValue("thetaI_Nstep_rad", (Float_t) - 1));
               line.push_back(env->GetValue("MonteCarlo_stat", (Float_t) - 1));
               fvec_TCsteps.push_back(line);

               //add the TTree in fchain
               fchain->Add(file->GetName());

               //close file
               //file->Close();
            }
         }
      }
   }//all lines read
}

//____________________________________________________________________________//
Bool_t KVVAMOSWeightFinder::CheckTransCoefSteps()
{
   //Internal check of all trans. coef. files steps and infos.
   //Use the steps informations extracted from all trans. coef. files
   //and saved in fvec_TCsteps vector.
   //Check if the steps in delta, the number of values and even the Monte-Carlo statistic
   //used to generate the trans. coef. *.root files are the same for all used
   //files saved in the dataset directory.
   Bool_t ok = kFALSE;

   //just one line
   if (fvec_TCsteps.size() == 1) ok = kTRUE;

   //more than one line
   else {
      for (unsigned i = 0; i < fvec_TCsteps.size() - 1; i++) {
         std::vector<Float_t> line0 = fvec_TCsteps.at(i);
         std::vector<Float_t> line1 = fvec_TCsteps.at(i + 1);

         Int_t nmc0 = (Int_t) line0.at(9);
         Int_t nmc1 = (Int_t) line1.at(9);
         Int_t nd0  = (Int_t) line0.at(4);
         Int_t nd1  = (Int_t) line1.at(4);
         Int_t nt0  = (Int_t) line0.at(8);
         Int_t nt1  = (Int_t) line1.at(8);

         //Ndelta, Ntheta and NMonteCarlo is OK
         if ((nmc0 == nmc1) && (nd0 == nd1) && (nt0 == nt1)) {
            Float_t dmin0 = line0.at(1);
            Float_t dmax0 = line0.at(2);
            Float_t dst0  = line0.at(3);
            Float_t dmin1 = line1.at(1);
            Float_t dmax1 = line1.at(2);
            Float_t dst1  = line1.at(3);
            Float_t epsd  = TMath::Min(dst0, dst1);

            //Delta steps and ranges should be the same
            if ((TMath::Abs(dmin0 - dmin1) <= epsd / 2.) && (TMath::Abs(dmax0 - dmax1) <= epsd / 2.) && (TMath::Abs(dst0 - dst1) <= epsd / 2.)) {
               ok = kTRUE;
            }

            else if (fkverbose) {
               Error("ChecTransCoefSteps", "... error in delta ranges ...\n (dmin0=%f, dmin1=%f) | (dmax0=%f, dmax1=%f) | (dstep0=%f, dstep1=%f)",
                     dmin0, dmin1, dmax0, dmax1, dst0, dst1);
            }
         }

         //Ndelta, Ntheta and NMonteCarlo is not OK
         else if (fkverbose) {
            Error("ChecTransCoefSteps", "... error in Nsteps ...\n (nMC0=%d, nMC1=%d) | (nDelta0=%d, nDelta1=%d) | (nTheta0=%d, nTheta1=%d)",
                  nmc0, nmc1, nd0, nd1, nt0, nt1);
         }
      }
   }

   return ok;
}

//____________________________________________________________________________//
Float_t KVVAMOSWeightFinder::GetTransCoef(Float_t VamosAngle_deg, Float_t delta, Float_t thetaI_rad)
{
   //Find the trans. coef. for a given (ThetaVamos, delta, thetaI) triplet.
   //First find the path to the file coresponding to the given ThetaVamos
   //(precision of 0.1 deg) and containing a TTree of its associated trans. coefs.
   //Then use the associated steps to extract the correct entry number of the
   //couple (delta, thetaI) in the TTree.

   //Find VamosAngle position in fvec_TCfiles
   Int_t num_angle = 0;
   Int_t nn = 0;
   for (std::vector< std::pair<Float_t, TString> >::iterator it = fvec_TCfiles.begin(); it != fvec_TCfiles.end(); ++it) {
      std::pair<Float_t, TString> par = *it;
      Float_t angle = par.first;
      TString path  = par.second;

      if (TMath::Abs(VamosAngle_deg - angle) <= 0.1) {
         num_angle = nn;
         if (fkverbose) {
            Info("GetTransCoef", "... found VamosAngle (=%f) position in vector -> num_angle=%d ...\n",
                 VamosAngle_deg, num_angle);
         }
      }

      nn++;
   }

   //Find, using the associated VamosAngle TEnv saved in fvec_TCsteps:
   //- delta position in its range
   //- thetaI position in its range
   //- total number of delta
   //- total number of thetaI
   //Also check if the VamosAngle is OK.
   std::vector<Float_t> line = fvec_TCsteps.at(num_angle);
   if (TMath::Abs(line.at(0) - VamosAngle_deg) > 0.1) {
      if (fkverbose) Error("GetTransCoef", "... found VamosAngle in steps vector (=%f) is different from given angle (=%f) ...",
                              line.at(0), VamosAngle_deg);
      return -666.;
   }

   Int_t ndelta = GetNValue(delta, line.at(1), line.at(2), line.at(3));
   Int_t ntheta = GetNValue(thetaI_rad, line.at(5), line.at(6), line.at(7));
   //Int_t ntot_delta = line.at(4)+1;
   Int_t ntot_delta = line.at(4);
   Int_t ntot_theta = line.at(8) + 1;

   if (fkverbose) {
      Info("GetTransCoef", "... found delta/theta positions in their associated ranges -> ndelta=%d, ntheta=%d, ntot_delta=%d, ntot_theta=%d ...",
           ndelta, ntheta, ntot_delta, ntot_theta);
   }

   //Use the found values to exctract the entry in fchain corresponding
   //to the wanted trans. coef.
   if (ndelta >= 0 && ntheta >= 0 && ntot_delta > 0 && ntot_theta > 0) {
      Long64_t pos_tree = (ndelta * ntot_theta) + ntheta; //entry in the tree
      Long64_t pos_chain = num_angle * (ntot_delta * ntot_theta) + pos_tree; //entry in the chain

      if (fkverbose) Info("GetTransCoef", "... found entry in tree (=%lli) and entry in chain (=%lli / %lli) ...", pos_tree, pos_chain, fchain->GetEntries());

      Float_t tc, dd, tt;
      fchain->SetBranchAddress("TransCoef", &tc);
      fchain->SetBranchAddress("Zdelta", &dd);
      fchain->SetBranchAddress("ZthetaI", &tt);
      fchain->GetEntry(pos_chain, 1);

      //debug
      if (fkverbose) {
         Info("GetTransCoef", "... input: (VamosAngle=%f, delta=%f, thetaI=%f) | closest: (delta=%f, thetaI=%f) | chain_entry: (delta=%f, thetaI%f) ...",
              VamosAngle_deg, delta, thetaI_rad, GetClosestValue(delta, line.at(3)), GetClosestValue(thetaI_rad, line.at(7)), dd, tt);
      }

      return tc;
   }

   else return -666.;
}
//____________________________________________________________________________//

Int_t KVVAMOSWeightFinder::GetNValue(Float_t val_exp, Float_t val_min, Float_t val_max, Float_t val_step)
{
   //Return position of the experimental observable 'val_exp'
   //in its associated range '[val_min, val_max]'
   //and with the step 'val_step'.

   //Get the closest value considering the step
   Float_t cl_val = GetClosestValue(val_exp, val_step);

   //If the closest delta is out of the given range
   if ((cl_val > val_max + val_step / 2.) || (cl_val < val_min - val_step / 2.)) {
      if (fkverbose) {
         if (cl_val > val_max + val_step / 2.) Error("GetNValue", "... closest value (=%f) above val_max+step/2. (=%f) ...", cl_val, val_max + val_step / 2.);
         if (cl_val < val_min - val_step / 2.) Error("GetNValue", "... closest value (=%f) below val_min-step/2. (=%f) ...", cl_val, val_min - val_step / 2.);
      }
      return -1;
   }

   //Closest value is in given range
   else {
      Int_t val_num = TMath::Nint((cl_val - val_min) / val_step);
      return val_num;
   }
}
//____________________________________________________________________________//

void KVVAMOSWeightFinder::PrintRunInfoVector()
{
   //Print the current state of fvec_infos, vector containing
   //'Run   BrhoRef   ThetaVamos   DeadTime   Scalers_INDRA' for each run,
   //and used for weight computation.

   Info("PrintRunInfoVector", "... printing list of available run infos ...\n(Run, BrhoRef ThetaVamos DeadTime Scalers_INDRA)\n\n");

   Int_t num = 0;
   for (std::vector< std::vector<Float_t> >::iterator it = fvec_infos.begin(); it != fvec_infos.end(); ++it) {
      std::vector<Float_t> line = *it;
      printf("[%d] %d %f %f %f %d\n", num, (int) line.at(0), line.at(1), line.at(2), line.at(3), (int) line.at(4));
      num++;
   }
}

//____________________________________________________________________________//
void KVVAMOSWeightFinder::PrintTransCoefInfoVector()
{
   //Print the current state of fvec_TCfiles, vector containing
   //'VamosAngle   TransCoef_file' for each VamosAngle,
   //and used for weight computation.

   Info("PrintTransCoefInfoVector", "... printing list of available transmission coefficients files infos ...\n\n");

   Int_t num = 0;
   for (std::vector< std::pair<Float_t, TString> >::iterator it = fvec_TCfiles.begin(); it != fvec_TCfiles.end(); ++it) {
      std::pair<Float_t, TString> line = *it;
      printf("[%d] %f %s\n", num, line.first, (line.second).Data());
      num++;
   }
}

//____________________________________________________________________________//
void KVVAMOSWeightFinder::PrintTransCoefStepVector()
{
   //Print the current state of fvec_TCsteps, vector containing
   //'VamosAngle_deg   delta_min   delta_max   delta_step   Ndelta   theta_min   theta_max   theta_step  Ntheta',
   //for each VamosAngle and used for weight computation.

   Info("PrintTransCoefStepVector", "... printing list of available trans. coef. infos  ...\n(VamosAngle_deg delta_min delta_max delta_step Ndelta theta_min theta_max theta_step Ntheta)\n\n");

   Int_t num = 0;
   for (std::vector< std::vector<Float_t> >::iterator it = fvec_TCsteps.begin(); it != fvec_TCsteps.end(); ++it) {
      std::vector<Float_t> line = *it;
      printf("[%d] %f %f %f %f %d %f %f %f %d\n",
             num, line.at(0), line.at(1), line.at(2), line.at(3), (int) line.at(4), line.at(5), line.at(6), line.at(7), (int) line.at(8));
      num++;
   }
}

//____________________________________________________________________________//
Float_t KVVAMOSWeightFinder::GetInverseWeight(Float_t brho, Float_t thetaI)
{
   //Compute the weight for a given experimental VAMOS event,
   //using the identified VAMOS nucleus informations, which means:
   //(bro_exp, thetaI_rad) where 'brho' is in T.m of the nucleus,
   //'thetaI' is the experimental theta angle in rad and in lab. ref. frame in
   //INDRA convention of the nucleus ( and also the current run number this events
   //belongs to, set as an argument of the class constructor).
   //
   //The inverse weight will be computed from:
   //- the chosen run list (see SetRunList() method) and the associated experimental
   //  (BrhoRef, AngleVamos, Scalers_INDRA) values from this run list.
   //- the transmission coefficient obtained from ZGOUBI simulations (normalised over
   //  2*TMath::Pi() in phi angle in lab. ref. frame and INDRA convention, depending on
   //  the (brho, thetaI) of the nucleus).
   //- the experimental DeadTime associated to fRunNumber, the run number the VAMOS
   //  events belongs to.
   //
   //It was chosen to apply a weighted average on VAMOS events only defined by their
   //(Brho, thetaI) values. In this way we are able to use ALL the statistics without
   //applying cuts on overlaping experimental Brho.
   //
   //The returned total weight 'W_tot' will be:
   // Begin_Latex
   // W_{tot}(B #rho , theta_{I}, run_number)= DT_{corr}(run_number) * W(B #rho , #theta _{I})
   // End_Latex
   //
   // where:
   // Begin_Latex
   // #frac{1}{W(B #rho , #theta _{I})} = #frac{#sum_{i}^{RunList}Scaler^{i} #cdot T(#delta ^{i}, #theta_{I} , #theta ^{i}_{V})}{#sum_{i}^{RunList}Scaler^{i}}
   // End_Latex
   // and:
   // Begin_Latex
   // DT_{corr}(run_number) = #frac{1}{1-DT(run_number)}
   // End_Latex
   //
   // and:
   //    run_number = current run number which the identified VAMOS nucleus belongs to
   //    delta      = brho/brho_ref_i
   //    brho       = experimental brho in T.m
   //    brho_ref_i = reference brho of VAMOS for the run i
   //    thetaI     = experimental theta angle in rad and in lab. ref. frame in INDRA convention
   //    Scaler_i   = value of INDRA scalers for the run i
   //    theta_V    = rotation angle of VAMOS in theta around beam trajectory

   if (fkverbose) Info("GetInverseWeight", "... starting weight computation for VAMOS event with (delta=%f, thetaI=%f, run=%d) ...", brho, thetaI, fRunNumber);

   Float_t num     = 0.;
   Float_t denum   = 0.;
   Float_t dt_corr = -666.;
   Float_t dt_run  = GetDeadTime(fRunNumber); //default '-666.' value will be returned if not found

   //DT found from the event run_number
   if (dt_run >= 0.) {
      //compute DT_corr for the current run
      dt_corr = 1. / (1. - dt_run);

      //extract W(Brho, ThetaI) from runlist
      fRunList.Begin();
      while (!fRunList.End()) {
         Int_t next_run = fRunList.Next();

         Float_t brho_ref = GetBrhoRef(next_run);
         Float_t scaler   = GetScalerINDRA(next_run);
         Float_t thetav   = GetThetaVamos(next_run);
         Float_t tc       = GetTransCoef(thetav, brho / brho_ref, thetaI);

         //(brho_ref, scaler) is OK for the given run
         //if the transmission coef. doesn't exist for the given (ThetaVamos, delta, thetaI)
         //(i.e tc=-666.), we still consider the scaler in the denumerator, but not in the
         //numerator (i.e we consider tc=0.)
         if ((brho_ref > 0) && (scaler > 0)) {
            denum += scaler;
            if (tc >= 0.) num += tc * scaler;
         }

         if (fkverbose) {
            Info("GetInverseWeight", "... (next_run=%d, brho=%f, thetaI=%f) -> (brho_ref=%f, thetav=%f, scaler=%f, tc=%f) ...",
                 next_run, brho, thetaI, brho_ref, thetav, scaler, tc);
            Info("GetInverseWeight", "... numerator=%f, denumerator=%f", num, denum);
         }
      }

      //return W_tot
      return num / denum / dt_corr;
   }

   //DT not found for the current run, weight can't be computed
   //default value -666. will be returned...
   else {
      if (fkverbose) {
         Info("GetInverseWeight", "... !!! DT(current_run=%d)=%f <0. !!! can't compute the weight of the event ...", fRunNumber, dt_run);
      }

      return -666.;
   }
}
