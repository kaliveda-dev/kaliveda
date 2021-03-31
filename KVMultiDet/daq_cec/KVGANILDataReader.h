//Created by KVClassFactory on Wed Sep 23 16:07:38 2009
//Author: Chbihi

#ifndef __KVGANILDATAREADER_H
#define __KVGANILDATAREADER_H

#include "KVRawDataReader.h"
#include "KVBase.h"
#include "KVACQParam.h"
#include "KVUniqueNameList.h"
#include "TTree.h"
class GTGanilData;

/**
\class KVGANILDataReader
\brief Reads GANIL acquisition files (EBYEDAT)
  \ingroup DAQ

   Open and initialise a GANIL data file in EBYEDAT format for reading.
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
runfile->ConnectRawDataParameters();
runfile->SetUserTree(T);
while( runfile->GetNextEvent() ) ;
file->Write();
file->Close();
</pre><br>
See method SetUserTree() for more details.
See below if you want to include a TTree containing scaler data in the file.
<h4>Scaler buffers management</h4>
By default, scaler buffers are ignored (<a href="GTGanilData.html#GTGanilData:SetScalerBuffersManagement">GTGanilData::SetScalerBuffersManagement</a>(GTGanilData::kSkipScaler)).
This can be changed by changing the value of<br>
<pre>
KVGANILDataReader.ScalerBuffersManagement:       kSkipScaler
</pre>
<br>
For possible values, see GTGanilData::SetScalerBuffersManagement().<br><br>
You can also add a second TTree to the user tree generated above containing the values
of all scaler buffers written in the data file. The TTree will be called 'Scalers'.
You need to add "SCALERS" to the option given to method <a href="#KVGANILDataReader:SetUserTree">SetUserTree()</a> (see below).
*/

class KVGANILDataReader : public KVRawDataReader {
protected:
   GTGanilData* fGanilData;//object used to read GANIL acquisition file
   TTree* fUserTree;//user TTree to fill with data
   Int_t NbParFired;
   UShort_t* ParVal;
   UInt_t* ParNum;
   Bool_t make_arrays;
   Bool_t make_leaves;

   KVUniqueNameList fParameters;//list of all data parameters contained in file
   KVUniqueNameList fFired;//list of fired parameters in one event

   GTGanilData* NewGanTapeInterface(Option_t* dataset);

   void FillFiredParameterList();

public:
   KVGANILDataReader()
   {
      init();
   }
   KVGANILDataReader(const Char_t*, Option_t*);
   void OpenFile(const Char_t*, Option_t* dataset);
   void ConnectRawDataParameters();
   virtual ~KVGANILDataReader();

   void init();
   virtual Bool_t GetNextEvent();
   virtual GTGanilData* GetGanTapeInterface();

   virtual void SetUserTree(TTree*, Option_t* = "arrays");

   const KVSeqCollection& GetRawDataParameters() const
   {
      return fParameters;
   }

   const KVSeqCollection& GetFiredDataParameters() const
   {
      // returns list of fired acquisition parameters of current event.
      // this list is filled automatically when GetNextEvent() is called.
      return fFired;
   }

   static KVGANILDataReader* Open(const Char_t* filename, Option_t* opt = "");

   Bool_t HasScalerBuffer() const;
   Int_t GetNumberOfScalers() const;
   UInt_t GetScalerCount(Int_t index) const;
   Int_t GetScalerStatus(Int_t index) const;
   Int_t GetEventCount() const;

   TString GetDataFormat() const
   {
      return "EBYEDAT";
   }
   Int_t GetRunNumberReadFromFile() const;

   ClassDef(KVGANILDataReader, 0) //Reads GANIL acquisition files
};

#endif
