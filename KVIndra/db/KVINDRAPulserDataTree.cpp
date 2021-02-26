/*
$Id: KVINDRAPulserDataTree.cpp,v 1.7 2009/03/27 16:42:58 franklan Exp $
$Revision: 1.7 $
$Date: 2009/03/27 16:42:58 $
*/

//Created by KVClassFactory on Wed Jan 21 11:56:26 2009
//Author: franklan

#include "KVINDRAPulserDataTree.h"
#include "KVDataSet.h"
#include "KVINDRA.h"
#include "KVINDRADBRun.h"

using namespace std;

ClassImp(KVINDRAPulserDataTree)



KVINDRAPulserDataTree::KVINDRAPulserDataTree()
{
   // Default constructor
   fArb = 0;
   fRun = 0;
   fVal = 0;
   fIndex = 0;
   fRunlist = 0;
}

KVINDRAPulserDataTree::~KVINDRAPulserDataTree()
{
   // Destructor
   if (fVal) delete [] fVal;
   if (fIndex) {
      fIndex->Delete();
      delete fIndex;
   }
}

void KVINDRAPulserDataTree::Build()
{
   // Create and fill tree with pulser data.
   // We look for the following two directories:
   //
   // $KVROOT/KVFiles/name_of_dataset/gene_detecteurs
   // $KVROOT/KVFiles/name_of_dataset/gene_pins
   //
   // If not found, we look for the following compressed archives:
   //
   // $KVROOT/KVFiles/name_of_dataset/gene_detecteurs.tgz
   // $KVROOT/KVFiles/name_of_dataset/gene_pins.tgz
   //
   // and if found, uncompress them ('tar -zxf').
   //    [[ N.B. in this case the extracted directories will be deleted after reading,
   //    [[ ensuring that if new archives are supplied,
   //    [[ we always use the latest versions of files.
   //
   // The default names of these directories are defined in .kvrootrc by:
   //
   // KVINDRAPulserDataTree.GeneDetDir:   gene_detecteurs
   // KVINDRAPulserDataTree.GenePinDir:   gene_pins
   //
   // Dataset-dependent alternatives can be defined using:
   //
   // dataset_name.KVINDRAPulserDataTree.GeneDetDir:   dataset_specific_value
   //
   // The first directory (gene_detecteurs) must contain 1 file per run with names like:
   //
   //    run8820.gene
   //
   // These files are generated using example analysis class GetGeneMean
   // (see Examples). They contain the mean value of every acquisition parameter
   // associated with a detector in the run:
   //
   //    CI_0201_GG     3095.28
   //    CI_0201_PG     275.626
   //    CI_0203_GG     2863.66
   //    CI_0203_PG     263.308
   //    CI_0205_GG     3042.83
   //    etc.
   //
   // i.e. 'name of acquisition parameter'    'mean value for run'
   //
   // The second directory (gene_pins) must contain 1 file per run with names like:
   //
   //    run8820.genepin
   // OR run8820.laserpin
   // OR run8820.genelaserpin
   //
   // These files are generated using example analysis class GetGeneMeanPin
   // (see Examples). They contain the mean values of the acquisition parameters
   // associated with pin diodes in the run:
   //
   //    PILA_01_PG_gene     792.616
   //    PILA_01_PG_laser     1747.18
   //    PILA_01_GG_gene     0
   //    etc.
   //
   // We create a TTree with 1 branch for each detector acquisition parameter.
   // For PILA and SI_PIN parameters, we create a 'gene' and a 'laser' branch for each.

   fGeneDir = new KVTarArchive(GetDirectoryName("GeneDetDir"), gDataSet->GetDataSetDir());
   fPinDir = new KVTarArchive(GetDirectoryName("GenePinDir"), gDataSet->GetDataSetDir());
   if (!fGeneDir->IsOK() && !fPinDir->IsOK()) {
      Info("Build", "No data available to build pulser data tree");
      return;
   }
   CreateTree();
   ReadData();
   delete fGeneDir;
   delete fPinDir;
}

TString KVINDRAPulserDataTree::GetDirectoryName(const Char_t* dirvar)
{
   // Returns the name of the directory defined by the .kvrootrc environment variable
   //
   // KVINDRAPulserDataTree.[dirvar]
   // OR
   // dataset_name.KVINDRAPulserDataTree.[dirvar]:

   TString search, datasetenv;
   datasetenv.Form("KVINDRAPulserDataTree.%s", dirvar);
   search = gDataSet->GetDataSetEnv(datasetenv.Data(), "");
   if (search == "") {
      Error("GetDirectoryName", "%s is not defined for dataset %s. Check .kvrootrc files.",
            datasetenv.Data(), gDataSet->GetName());
   }
   return search;
}

void KVINDRAPulserDataTree::CreateTree()
{
   // Create new TTree with
   //   1 branch 'Run' with run number
   //   1 branch for each acquisition parameter of every detector (except time markers)
   //   2 branches for each 'PILA_...' or 'SI_PIN...' parameter, suffixed with '_laser' and '_gene'
   //
   // NB if multidetector has not been built, it will be built by this method

   fArb = new TTree("PulserData", "Created by KVINDRAPulserDataTree");
   fArb->SetDirectory(0);

   fArb->Branch("Run", &fRun, "Run/I");

   if (!gIndra) KVMultiDetArray::MakeMultiDetector(gDataSet->GetName());

   const KVSeqCollection* acq_pars = nullptr;//gIndra->GetACQParams();

   fTab_siz = acq_pars->GetEntries() + 20;
   fVal = new Float_t[fTab_siz];
   fIndex = new THashTable(20, 5);
   fIndex->SetOwner(kTRUE);

   TIter nxtACQ(acq_pars);
   KVACQParam* ap = 0;
   Int_t ap_num = 0;
   KVBase* iob;
   while ((ap = (KVACQParam*)nxtACQ())) {

      TString ap_name(ap->GetName());
      TString ap_type(ap->GetType());
      if (ap_name.BeginsWith("PILA") || ap_name.BeginsWith("SI_PIN")) {
         ap_name += "_laser";
         iob = new KVBase(ap_name.Data());
         iob->SetNumber(ap_num);
         fIndex->Add(iob);
         fArb->Branch(ap_name.Data(), &fVal[ap_num++], Form("%s/F", ap_name.Data()));
         ap_name.Form("%s%s", ap->GetName(), "_gene");
         iob = new KVBase(ap_name.Data());
         iob->SetNumber(ap_num);
         fIndex->Add(iob);
         fArb->Branch(ap_name.Data(), &fVal[ap_num++], Form("%s/F", ap_name.Data()));
      }
      else if (ap_type != "T") {
         iob = new KVBase(ap_name.Data());
         iob->SetNumber(ap_num);
         fIndex->Add(iob);
         fArb->Branch(ap_name.Data(), &fVal[ap_num++], Form("%s/F", ap_name.Data()));
      }
      if (ap_num > fTab_siz - 2) {
         Error("CreateTree",
               "Number of branches to create is greater than estimated (%d). Not all parameters can be treated.",
               fTab_siz);
         return;
      }

   }
   //keep number of used 'slots' in array
   fTab_siz = ap_num;
}

void KVINDRAPulserDataTree::ReadFile(ifstream& fin)
{
   // Read data in one file

   KVString line;
   line.ReadLine(fin);
   while (fin.good()) {
      if (!line.BeginsWith("#")) {
         line.Begin(" ");
         KVString br_name = line.Next(kTRUE);
         Int_t index = GetIndex(br_name.Data());
         fVal[index] = line.Next(kTRUE).Atof();
      }
      line.ReadLine(fin);
   }
   fin.close();
}

UChar_t KVINDRAPulserDataTree::ReadData(Int_t run)
{
   // Read data for one run, fill tree

   UChar_t msg = 0;
   fRun = run;
   if (fGeneDir->IsOK()) {
      ifstream f;
      if (OpenGeneData(run, f)) ReadFile(f);
      else msg = msg | 1;
   }
   if (fPinDir->IsOK()) {
      ifstream f;
      if (OpenPinData(run, f)) ReadFile(f);
      else msg = msg | 2;
   }
   fArb->Fill();
   return msg;
}

Bool_t KVINDRAPulserDataTree::OpenGeneData(Int_t run, ifstream& f)
{
   // Open gene data for one run
   // We look for file 'runXXXX.gene' in the directory given by
   // environment variable KVINDRAPulserDataTree.GeneDetDir.

   TString fname;
   fname.Form("/run%d.gene", run);
   fname.Prepend(gDataSet->GetDataSetEnv("KVINDRAPulserDataTree.GeneDetDir", ""));
   return gDataSet->OpenDataSetFile(fname.Data(), f);
}

Bool_t KVINDRAPulserDataTree::OpenPinData(Int_t run, ifstream& f)
{
   // Open pin data for one run
   // We look for one of the following files in the directory given by
   // environment variable KVINDRAPulserDataTree.GenePinDir:
   //
   //    runXXXX.genepin
   // OR runXXXX.laserpin
   // OR runXXXX.genelaserpin

   TString fname;
   fname.Form("/run%d.genepin", run);
   TString pindir(gDataSet->GetDataSetEnv("KVINDRAPulserDataTree.GenePinDir", ""));
   fname.Prepend(pindir);
   if (gDataSet->OpenDataSetFile(fname.Data(), f)) return kTRUE;
   fname.Form("/run%d.laserpin", run);
   fname.Prepend(pindir);
   if (gDataSet->OpenDataSetFile(fname.Data(), f)) return kTRUE;
   fname.Form("/run%d.genelaserpin", run);
   fname.Prepend(pindir);
   return gDataSet->OpenDataSetFile(fname.Data(), f);
}

void KVINDRAPulserDataTree::ReadData()
{
   // Read data for every run in dataset

   if (!fRunlist) {
      Error("ReadData", "Must set list of runs first using SetRunList(TList*)");
      return;
   }
   Info("ReadData", "Reading pulser and laser data for all runs");
   TIter Nxt_r(fRunlist);
   KVINDRADBRun* run = 0;
   KVNumberList missing1, missing2;
   while ((run = (KVINDRADBRun*)Nxt_r())) {
      Int_t run_num = run->GetNumber();
      //reset all array members to -1
      for (int i = 0; i < fTab_siz; i++) fVal[i] = -1.0;
      std::cout << "\rInfo in <KVINDRAPulserDataTree::ReadData>: Reading data for run " << run_num << std::flush;
      UChar_t msg = ReadData(run_num);
      if (msg & 1) missing1.Add(run_num);
      if (msg & 2) missing2.Add(run_num);
   }
   std::cout << std::endl;
   if (missing1.GetEntries())
      Warning("ReadData", "Missing file 'run[run_num].gene' for runs: %s", missing1.GetList());
   if (missing2.GetEntries())
      Warning("ReadData", "Missing file 'run[run_num].[gene][laser]pin' for runs: %s", missing2.GetList());
}

void KVINDRAPulserDataTree::ReadTree(TFile* file)
{
   // Read pulser data tree from file

   fArb = (TTree*)file->Get("PulserData");
   if (fArb) {
      //disable all branches except Run number
      fArb->SetBranchStatus("*", 0);
      fArb->SetBranchStatus("Run", 1);
   }
}

void KVINDRAPulserDataTree::WriteTree(TFile* file)
{
   // Write pulser data tree in file
   // We build and index based on the Run number and store it in the tree.

   if (fArb) {
      fArb->SetDirectory(file);
      fArb->BuildIndex("Run");
      fArb->Write();
   }
}

Float_t KVINDRAPulserDataTree::GetMean(const Char_t* param, Int_t run)
{
   // Return mean value of pulser/laser for given parameter and run.
   // For detectors, param should be name of an acquisition parameter
   // e.g. CI_0201_PG, CSI_1301_L, etc.
   // For pin laser diodes, param should be name of associated acquisition parameter
   // with either '_laser' or '_gene' appended
   // e.g. PILA_05_PG_laser, SI_PIN1_PG_gene
   //
   // Returns -1.0 if no data available for this parameter/run.

   if (!fArb) return -1.0;

   //find corresponding branch
   TBranch* br = fArb->GetBranch(param);
   if (!br) {
      //no branch found - wrong name given ?
      Error("GetMean", "No branch found with name %s", param);
      return -1.0;
   }
   //enable branch
   fArb->SetBranchStatus(param, 1);
   //connect variable to branch
   Float_t value = -1.0;
   br->SetAddress(&value);
   //read entry corresponding to run
   Int_t bytes = fArb->GetEntryWithIndex(run);
   if (bytes < 0) {
      //unknown run number
      Error("GetMean", "Unknown run %d", run);
      return -1.0;
   }
   //disable branch
   fArb->SetBranchStatus(param, 0);

   return value;
}
