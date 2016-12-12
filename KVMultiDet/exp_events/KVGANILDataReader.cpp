//Created by KVClassFactory on Wed Sep 23 16:07:38 2009
//Author: Chbihi

#include "KVGANILDataReader.h"
#include "GTGanilData.h"
#include "GTOneScaler.h"
#include "KVDataSet.h"
#include "KVMultiDetArray.h"
#include "TSystem.h"
#include "TUrl.h"
#include "TPluginManager.h"
#include "RVersion.h"
#include <TInterpreter.h>

ClassImp(KVGANILDataReader)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVGANILDataReader</h2>
<h4>Reads GANIL acquisition files</h4>
   Open and initialise a GANIL data file for reading.
   If file cannot be opened, this object will be made a zombie. Do not use.
   To test if file is open, use IsOpen().
   The basename of the file (excluding any path) can be obtained using GetName()
   The full pathname of the file can be obtained using GetTitle()
   <br><br>
   If the dataset corresponding to the data to be read is known i.e. if gDataSet has been defined and points
   to the correct dataset, this will allow to build the necessary multidetector object if it has not already
   been done, and to set the calibration parameters etc. as a function of the run number.
   <br><br>
   If not (i.e. if no information is available on detectors, calibrations, geometry, etc.),
   then a list of KVACQParam objects will be generated and connected ready for reading the data.
   <br><br>
   To fill a TTree with all data in the file, do the following:<br>
<pre>
KVGANILDataReader* runfile = new KVGANILDataReader("run1.dat");
TFile* file = new TFile("run1.root","recreate");
TTree* T = new TTree("Run1", "Raw data for Run1");
runfile->SetUserTree(T);
while( runfile->GetNextEvent() ) ;
file->Write();
file->Close();
</pre><br>
See method <a href="#KVGANILDataReader:SetUserTree">SetUserTree()</a> for more details.
See below if you want to include a TTree containing scaler data in the file.
<h4>Scaler buffers management</h4>
By default, scaler buffers are ignored (<a href="GTGanilData.html#GTGanilData:SetScalerBuffersManagement">GTGanilData::SetScalerBuffersManagement</a>(GTGanilData::kSkipScaler)).
This can be changed by changing the value of<br>
<pre>
KVGANILDataReader.ScalerBuffersManagement:       kSkipScaler
</pre>
<br>
For possible values, see <a href="GTGanilData.html#GTGanilData:SetScalerBuffersManagement">GTGanilData::SetScalerBuffersManagement</a>.<br><br>
You can also add a second TTree to the user tree generated above containing the values
of all scaler buffers written in the data file. The TTree will be called 'Scalers'.
You need to add "SCALERS" to the option given to method <a href="#KVGANILDataReader:SetUserTree">SetUserTree()</a> (see below).
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVGANILDataReader::KVGANILDataReader(const Char_t* file)
{
   //Open and initialise a GANIL data file for reading.
   //By default, Scaler buffers are ignored.
   //If file cannot be opened, this object will be made a zombie. Do not use.
   //To test if file is open, use IsOpen().
   //The basename of the file (excluding any path) can be obtained using GetName()
   //The full pathname of the file can be obtained using GetTitle()
   //
   //If the dataset corresponding to the data to be read is known i.e. if gDataSet has been defined and points
   //to the correct dataset, this will allow to build the necessary multidetector object if it has not already
   //been done, and to set the calibration parameters etc. as a function of the run number.
   //
   //If not (i.e. if no information is available on detectors, calibrations, geometry, etc.),
   //then a list of KVACQParam objects will be generated and connected ready for reading the data.

   init();
   OpenFile(file);
}

KVGANILDataReader::~KVGANILDataReader()
{
   // Destructor
   if (fGanilData) {
      delete fGanilData;
      fGanilData = 0;
   }
   fParameters->Clear();
   delete fParameters;
   if (fExtParams) {
      fExtParams->Delete("slow");
      delete fExtParams;
   }
   delete fFired;
   if (ParVal) delete [] ParVal;
   if (ParNum) delete [] ParNum;
}

void KVGANILDataReader::init()
{
   //default initialisations

   fExtParams = 0;
   fParameters = new KVHashList;
   fParameters->SetCleanup(kTRUE);
   fGanilData = 0;
   fUserTree = 0;
   fFired = new KVHashList;
   ParVal = 0;
   ParNum = 0;
   make_arrays = make_leaves = kFALSE;
}

void KVGANILDataReader::SetUserTree(TTree* T, Option_t* opt)
{
   // To fill a TTree with the data in the current file, create a TTree:
   //    TFile* file = new TFile("run1.root","recreate");
   //    TTree* T = new TTree("Run1", "Raw data for Run1");
   // and then call this method: SetUserTree(T)
   // If you read all events of the file, the TTree will be automatically filled
   // with data :
   //    while( runfile->GetNextEvent() ) ;
   //
   // Two different TTree structures are available, depending on the option string:
   //
   //    opt = "arrays": [default]
   //
   // The TTree will have the following structure:
   //
   //    *Br    0 :NbParFired : NbParFired/I    = number of fired parameters in event
   //    *............................................................................*
   //    *Br    1 :ParNum    : ParNum[NbParFired]/i   = array of indices of fired parameters
   //    *............................................................................*
   //    *Br    2 :ParVal    : ParVal[NbParFired]/s   = array of values of fired parameters
   //
   // This structure is the fastest to fill and produces the smallest file sizes.
   // In order to be able to directly access the parameters as if option "leaves" were used
   // (i.e. one branch/leaf for each parameter), we add two aliases for each parameter to
   // the tree:
   //        PARNAME           = value of parameter if present in event
   //        PARNAME_M      = number of times parameter appears in event
   // Assuming that each parameter only appears at most once in each event, i.e. PARNAME_M=0 or 1,
   // then
   //    root[0] T->Draw("PARNAME", "PARNAME_M")
   // will histogram the value of PARNAME for each event in which it is present.
   // (if the selection condition "PARNAME_M" is not used, the histogram will also be filled with a 0
   // for each event in which PARNAME does not appear).
   //          N.B. the PARNAME alias is in fact the sum of the values of PARNAME in each event.
   //          If PARNAME_M>1 in some events, it is not the individual values but their sum which will
   //          be histogrammed in this case.
   //
   // Thus, if the data file has parameters called "PAR_1" and "PAR_2",
   // the following command will work
   //
   //    root[0]  T->Draw("PAR_1:PAR_2", "PAR_1_M&&PAR_2_M", "col")
   //
   // even though no branches "PAR_1" or "PAR_2" exist.
   //
   //
   //
   //    opt = "leaves":
   //
   // The TTree will have a branch/leaf for each parameter. This option is slower and produces
   // larger files.
   //
   // If the option string contains both "arrays" and "leaves", then both structures will be used
   // (in this case there is a high redundancy, as each parameter is stored twice).
   //
   // The full list of parameters is stored in a TObjArray in the list returned by TTree::GetUserInfo().
   // Each parameter is represented by a TNamed object.
   // In order to retrieve the name of the parameter with index 674 (e.g. taken from branch ParNum),
   // do:
   //     TObjArray* parlist = (TObjArray*) T->GetUserInfo()->FindObject("ParameterList");
   //     cout << "Par 674 name = " << (*parlist)[674]->GetName() << endl;
   //
   //
   //  Automatic creation & filling of Scalers TTree
   //
   // give an option string containing "scalers", i.e. "leaves,scalers", or "ARRAYS+SCALERS", etc.
   // a TTree with name 'Scalers' will be created, all scaler buffers will be written in it.


   TString option = opt;
   option.ToUpper();
   make_arrays = option.Contains("ARRAYS");
   make_leaves = option.Contains("LEAVES");
   Bool_t make_scalers = option.Contains("SCALERS");
   if (make_scalers) {
      fGanilData->SetScalerBuffersManagement(GTGanilData::kAutoWriteScaler);
   }

   fUserTree = T;
   if (make_arrays) {
      Int_t maxParFired = GetRawDataParameters()->GetEntries();
      ParVal = new UShort_t[maxParFired];
      ParNum = new UInt_t[maxParFired];
      fUserTree->Branch("NbParFired", &NbParFired, "NbParFired/I");
      fUserTree->Branch("ParNum", ParNum, "ParNum[NbParFired]/i");
      fUserTree->Branch("ParVal", ParVal, "ParVal[NbParFired]/s");
   }
   if (make_leaves) {
      TIter next_rawpar(GetRawDataParameters());
      KVACQParam* acqpar;
      while ((acqpar = (KVACQParam*)next_rawpar())) {
         TString leaf;
         leaf.Form("%s/S", acqpar->GetName());
         // for parameters with <=8 bits only use 1 byte for storage
         if (acqpar->GetNbBits() <= 8) leaf += "1";
         fUserTree->Branch(acqpar->GetName(), *(acqpar->ConnectData()), leaf.Data());
      }
   }

#if ROOT_VERSION_CODE > ROOT_VERSION(5,25,4)
#if ROOT_VERSION_CODE < ROOT_VERSION(5,26,1)
   // The TTree::OptimizeBaskets mechanism is disabled, as for ROOT versions < 5.26/00b
   // this lead to a memory leak
   fUserTree->SetAutoFlush(0);
#endif
#endif

   // add list of parameter names in fUserTree->GetUserInfos()
   // and if option="arrays" add aliases for each parameter & its multiplicity

   // TObjArray has to be as big as the largest parameter number in the list
   // of raw data parameters. So first loop over parameters to find max param number.
   UInt_t maxpar = 0;
   TIter next(GetRawDataParameters());
   KVACQParam* par;
   while ((par = (KVACQParam*)next())) if (par->GetNumber() > maxpar) maxpar = par->GetNumber();

   TObjArray* parlist = new TObjArray(maxpar, 1);
   parlist->SetName("ParameterList");

   next.Reset();
   while ((par = (KVACQParam*)next())) {
      parlist->AddAt(new TNamed(par->GetName(), Form("index=%d", par->GetNumber())), par->GetNumber());
      if (make_arrays) {
         fUserTree->SetAlias(par->GetName(), Form("Sum$((ParNum==%d)*ParVal)", par->GetNumber()));
         fUserTree->SetAlias(Form("%s_M", par->GetName()), Form("Sum$(ParNum==%d)", par->GetNumber()));
      }
   }
   fUserTree->GetUserInfo()->Add(parlist);
}

void KVGANILDataReader::OpenFile(const Char_t* file)
{
   //Open and initialise a GANIL data file for reading.
   //By default, scaler buffers are ignored.
   //This can be changed by changing the value of
   //
   // KVGANILDataReader.ScalerBuffersManagement:       kSkipScaler
   //
   //If file cannot be opened, this object will be made a zombie. Do not use.
   //To test if file is open, use IsOpen().
   //The basename of the file (excluding any path) can be obtained using GetName()
   //The full pathname of the file can be obtained using GetTitle()
   //
   //If the dataset corresponding to the data to be read is known i.e. if gDataSet has been defined and points
   //to the correct dataset, the associated multidetector object with the calibration parameters etc.
   //for the run will be built (any previously existing object pointed to by gMultiDetArray will
   //be deleted beforehand)
   //
   //If not (i.e. if no information is available on detectors, calibrations, geometry, etc.),
   //then a list of KVACQParam objects will be generated and connected ready for reading the data.
   //This list can be obtained with method GetRawDataParameters().

   if (fGanilData) {
      delete fGanilData;
      fGanilData = 0;
   }

   fGanilData = NewGanTapeInterface();

   fGanilData->SetFileName(file);
   SetName(gSystem->BaseName(file));
   SetTitle(file);

   // handling scaler buffers
   TString what_scale = gEnv->GetValue("KVGANILDataReader.ScalerBuffersManagement", "kSkipScaler");
   what_scale.Prepend("GTGanilData::");
   Long_t ws = gInterpreter->ProcessLine(what_scale);
   fGanilData->SetScalerBuffersManagement((GTGanilData::ScalerWhat_t)ws);

   fGanilData->Open();

   //test whether file has been opened
   if (!fGanilData->IsOpen()) {
      //if initial attempt fails, we try to open the file as a 'raw' TFile
      //This may work when we are attempting to open a remote file i.e.
      //via rfio, and a subsequent attempt to open the file using the GanTape
      //routines may then succeed.
      TUrl rawtfile(file, kTRUE);
      rawtfile.SetOptions("filetype=raw");
      TFile* rawfile = TFile::Open(rawtfile.GetUrl());
      if (!rawfile) {
         Error("OpenFile", "File cannot be opened: %s",
               file);
         MakeZombie();
         return;
      }
      //TFile::Open managed to open file! Try again...
      delete rawfile;
      fGanilData->Open();
      if (!fGanilData->IsOpen()) {
         //failed again ??!
         Error("OpenFile", "File cannot be opened: %s",
               file);
         MakeZombie();
         return;
      }
   }
   // if this data belongs to a known (and currently active dataset), we
   // build the corresponding multidetector array corresponding to the run
   if (gDataSet)
      KVMultiDetArray::MakeMultiDetector(gDataSet->GetName(), fGanilData->GetRunNumber());

   ConnectRawDataParameters();
}

//__________________________________________________________________________

void KVGANILDataReader::ConnectRawDataParameters()
{
   //Private utility method called by KVGANILDataReader ctor.
   //fParameters is filled with a KVACQParam for every acquisition parameter in the file.
   //If there exists a gMultiDetArray corresponding to this data, we use the KVACQParams
   //already defined for the detectors of the array whenever possible.
   //For any parameters for which no KVACQParam already exists (a fortiori if no
   //gMultiDetArray exists) we create new KVACQParam objects which will be deleted
   //with this KVGANILDataReader (these objects can be accessed from the list
   //returned by GetUnknownParameters()).
   //To access the full list of data parameters in the file after this method has been
   //called (i.e. after the file is opened), use GetRawDataParameters().

   TIter next(fGanilData->GetListOfDataParameters());
   KVACQParam* par;
   GTDataPar* daq_par;
   while ((daq_par = (GTDataPar*) next())) {//loop over all parameters
      par = CheckACQParam(daq_par->GetName());
      fGanilData->Connect(par->GetName(), par->ConnectData());
      par->SetNumber(daq_par->Index());
      par->SetNbBits(daq_par->Bits());
      fParameters->Add(par);
   }
}

//___________________________________________________________________________

KVACQParam* KVGANILDataReader::CheckACQParam(const Char_t* par_name)
{
   //Check the named acquisition parameter.
   //We look for a corresponding parameter in the list of acq params belonging to
   //the current KVMultiDetArray (if one exists).
   //If none is found, we create a new acq param which is added to the list of "unknown parameters"

   KVACQParam* par;
   if (!gMultiDetArray || !(par = gMultiDetArray->GetACQParam(par_name))) {
      //create new unknown parameter
      par = new KVACQParam;
      par->SetName(par_name);
      if (!fExtParams) {
         fExtParams = new KVHashList;
         fExtParams->SetOwner(kTRUE);
      }
      fExtParams->Add(par);
   }
   return par;
}

//___________________________________________________________________________

Bool_t KVGANILDataReader::GetNextEvent()
{
   // Read next event in raw data file.
   // Returns false if no event found (end of file).
   // The list of all fired acquisition parameters is filled, and can be retrieved with
   // GetFiredDataParameters().
   // If SetUserTree(TTree*) has been called, the TTree is filled with the values of all
   // parameters in this event.

   Bool_t ok = fGanilData->Next();
   FillFiredParameterList();
   if (fUserTree) {
      if (make_arrays) {
         NbParFired = fFired->GetEntries();
         TIter next(fFired);
         KVACQParam* par;
         int i = 0;
         while ((par = (KVACQParam*)next())) {
            ParVal[i] = par->GetCoderData();
            ParNum[i] = par->GetNumber();
            i++;
         }
      }
      fUserTree->Fill();
   }
   return ok;
}

//___________________________________________________________________________

GTGanilData* KVGANILDataReader::NewGanTapeInterface()
{
   // Creates and returns new instance of class derived from GTGanilData used to read GANIL acquisition data
   // Actual class is determined by Plugin.GTGanilData in environment files and name of dataset.
   // If no dataset is defined, we use the default GTGanilData object.

   //check and load plugin library
   TString dsname = "any";
   if (gDataSet) dsname = gDataSet->GetName();
   TPluginHandler* ph = LoadPlugin("GTGanilData", dsname);
   if (!ph) {
      // default: GTGanilData
      return (new GTGanilData);
   }

   //execute constructor/macro for plugin
   return ((GTGanilData*) ph->ExecPlugin(0));
}

//____________________________________________________________________________

GTGanilData* KVGANILDataReader::GetGanTapeInterface()
{
   return fGanilData;
}

//____________________________________________________________________________

KVGANILDataReader* KVGANILDataReader::Open(const Char_t* filename, Option_t*)
{
   //Static method, used by KVDataSet::Open
   return new KVGANILDataReader(filename);
}

//____________________________________________________________________________

void KVGANILDataReader::FillFiredParameterList()
{
   // clears and then fills list fFired with all fired acquisition parameters in event
   fFired->Clear();
   TIter next(fParameters);
   KVACQParam* par;
   while ((par = (KVACQParam*)next())) if (par->Fired()) fFired->Add(par);
}

//____________________________________________________________________________
Bool_t KVGANILDataReader::HasScalerBuffer() const
{
   return fGanilData->IsScalerBuffer();
}

Int_t KVGANILDataReader::GetNumberOfScalers() const
{
   return fGanilData->GetScalers()->GetNbChannel();
}

//____________________________________________________________________________

UInt_t KVGANILDataReader::GetScalerCount(Int_t index) const
{
   return fGanilData->GetScalers()->GetScalerPtr(index)->GetCount();
}

//____________________________________________________________________________

Int_t KVGANILDataReader::GetScalerStatus(Int_t index) const
{
   return fGanilData->GetScalers()->GetScalerPtr(index)->GetStatus();
}

//____________________________________________________________________________

Int_t KVGANILDataReader::GetEventCount() const
{
   return fGanilData->GetEventCount();
}


