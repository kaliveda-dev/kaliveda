/***************************************************************************
                          kvINDRADB.h  -  description
                             -------------------
    begin                : 9/12 2003
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

#ifndef KV_INDRADB_H
#define KV_INDRADB_H

#include "KVExpDB.h"
#include "KVDataSet.h"
#include "KVINDRADBRun.h"
#include "KVDBSystem.h"
#include "KVDBTape.h"
#include "KVDetector.h"
#include "Riostream.h"
#include "TEnv.h"
#include "KVINDRARunListReader.h"
#include "KVINDRAPulserDataTree.h"
#include "KVNumberList.h"

//dimension of run_range arrays
#define MAX_NUM_RUN_RANGES 100

class KVNumberList;

/**
  \class KVINDRADB
  \ingroup INDRADB
  \brief DataBase of parameters for an INDRA campaign

Database containing information on runs, systems, calibrations etc. for an INDRA
experiment or set of experiments (dataset).

Each dataset is described by a KVDataSet object which is initialised by the KVDataSetManager.

For each dataset, a directory exists under $KVROOT/KVFiles/name_of_dataset,
where it is assumed the database for this dataset can be found.

If DataBase.root does not exist, KVDataSet will try to rebuild it using the appropriate
KVINDRADB class (see Plugins in $KVROOT/KVFiles/.kvrootrc).

By default, KVINDRADB::Build() will read any or all of the standard format files Runlist.csv,
Systems.dat and ChIoPressures.dat which may be found in the dataset's directory.
*/

class KVINDRADB: public KVExpDB, public KVINDRARunListReader {

private:

   std::ifstream __ifpeaks;          //!ifstream for calibration peaks file

protected:

   //General information

   KVDBTable* fChIoPressures;   //-> table of chio pressures
   KVDBTable* fGains;           //(optional) table of detector gains, in case they change from run to run
   KVDBTable* fTapes;           //-> table of data tapes
   KVDBTable* fCsILumCorr;      //-> table of CsI gain corrections for total light output
   KVDBTable* fPedestals;       //-> table of pedestal files
   KVDBTable* fChanVolt;        //-> ChIo/Si channel-volt calibration parameters
   KVDBTable* fVoltMeVChIoSi;   //-> ChIo/Si volt-energy calibration
   KVDBTable* fLitEnerCsI;      //->CsI light-energy calibration for Z>1
   KVDBTable* fLitEnerCsIZ1;     //->CsI light-energy calibration for Z=1
   KVDBTable* fAbsentDet;        //(optional) Liste les detecteurs absents
   KVDBTable* fOoODet;           //(optional) Liste les detecteurs hors service
   KVDBTable* fOoOACQPar;     //(optional) Liste des parametres d acquisition ne marchant pas

   KVINDRAPulserDataTree* fPulserData;  //! mean values of pulsers for all detectors & runs

   virtual void ReadGainList();
   virtual void ReadChIoPressures();
   virtual void ReadCsITotalLightGainCorrections();
   virtual void ReadChannelVolt();
   virtual void ReadVoltEnergyChIoSi();
   virtual void ReadLightEnergyCsI(const Char_t*, KVDBTable*);
   virtual void ReadCalibCsI();
   virtual void ReadPedestalList();

   virtual void ReadAbsentDetectors();
   virtual void ReadOoODetectors();
   virtual void ReadOoOACQParams();


   //calibration peaks database
   Bool_t OpenCalibrationPeakFile();
   void CloseCalibrationPeakFile();
   std::ifstream& GetPeakFileStream()
   {
      return __ifpeaks;
   };

   mutable TF1* fMeanGDir;//! mean frequency of GENE_DIRECT vs. run number
   void _init_MeanGDir_func() const;

   void init();

public:
   KVINDRADB();
   KVINDRADB(const Char_t* name);
   virtual ~ KVINDRADB();

   virtual void Build();
   virtual void cd();

   void AddTape(KVDBTape* r)
   {
      fTapes->AddRecord(r);
   }
   virtual KVDBTape* GetTape(Int_t tape) const
   {
      return (KVDBTape*) fTapes->GetRecord(tape);
   }
   virtual KVSeqCollection* GetTapes() const
   {
      return fTapes->GetRecords();
   }

   virtual void GoodRunLine();
   KVINDRADBRun* GetRun(Int_t run) const
   {
      return (KVINDRADBRun*) GetDBRun(run);
   }
   void ReadNewRunList();

   KVList* GetCalibrationPeaks(Int_t run, KVDetector* detector = 0,
                               Int_t peak_type = -1, Int_t signal_type = 0,
                               Double_t peak_energy = -1.0);

   Double_t GetEventCrossSection(Int_t run, Double_t Q_apres_cible,
                                 Double_t Coul_par_top = 1.e-10) const;
   Double_t GetTotalCrossSection(Int_t run, Double_t Q_apres_cible,
                                 Double_t Coul_par_top = 1.e-10) const;
   Double_t GetEventCrossSection(Int_t run1, Int_t run2,
                                 Double_t Q_apres_cible,
                                 Double_t Coul_par_top = 1.e-10) const;
   Double_t GetTotalCrossSection(Int_t run1, Int_t run2,
                                 Double_t Q_apres_cible,
                                 Double_t Coul_par_top = 1.e-10) const;
   Double_t GetEventCrossSection(KVNumberList runs,
                                 Double_t Q_apres_cible,
                                 Double_t Coul_par_top = 1.e-10) const;
   Double_t GetTotalCrossSection(KVNumberList runs,
                                 Double_t Q_apres_cible,
                                 Double_t Coul_par_top = 1.e-10) const;
   Double_t GetEventCrossSection(const Char_t* system, Int_t Mult_trigger,
                                 Double_t Q_apres_cible,
                                 Double_t Coul_par_top = 1.e-10) const;
   Double_t GetTotalCrossSection(const Char_t* system, Int_t Mult_trigger,
                                 Double_t Q_apres_cible,
                                 Double_t Coul_par_top = 1.e-10) const;
   Double_t GetTotalCrossSection(TH1* events_histo, Double_t Q_apres_cible, Double_t Coul_par_top = 1.e-10);

   Double_t GetMeanGDirFreq(Int_t run) const;

   KVINDRAPulserDataTree* GetPulserData()
   {
      return fPulserData;
   }
   virtual void WriteObjects(TFile*);
   virtual void ReadObjects(TFile*);

   virtual Double_t GetRunLengthFromGeneDirect(int run) const;

   ClassDef(KVINDRADB, 5)       //DataBase of parameters for an INDRA campaign
};

//........ global variable
R__EXTERN KVINDRADB* gIndraDB;

#endif
