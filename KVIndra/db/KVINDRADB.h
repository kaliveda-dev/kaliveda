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

#include "KVDataBase.h"
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

class KVINDRADB: public KVDataBase, public KVINDRARunListReader {

private:

   std::ifstream __ifpeaks;          //!ifstream for calibration peaks file

protected:

   Bool_t OpenCalibFile(const Char_t* type, std::ifstream& fs) const;

   //General information

   Int_t kFirstRun;
   Int_t kLastRun;

   KVDBTable* fRuns;            //-> table of runs
   KVDBTable* fSystems;         //-> table of systems
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

   virtual void LinkListToRunRanges(TList* list, UInt_t rr_number,
                                    UInt_t run_ranges[][2]);
   virtual void LinkRecordToRunRanges(KVDBRecord* rec, UInt_t rr_number,
                                      UInt_t run_ranges[][2]);
   virtual void LinkRecordToRunRange(KVDBRecord* rec, UInt_t first_run,
                                     UInt_t last_run);
   virtual void LinkListToRunRange(TList* list, KVNumberList nl);
   virtual void LinkRecordToRunRange(KVDBRecord* rec,  KVNumberList nl);
   virtual void LinkRecordToRun(KVDBRecord* rec,  Int_t run);

   virtual void ReadSystemList();
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

   void init();

public:
   KVINDRADB();
   KVINDRADB(const Char_t* name);
   virtual ~ KVINDRADB();

   virtual void Build();
   virtual void GoodRunLine();

   void AddRun(KVDBRun* r)
   {
      fRuns->AddRecord(r);
   };
   void AddTape(KVDBTape* r)
   {
      fTapes->AddRecord(r);
   };
   void AddSystem(KVDBSystem* r)
   {
      fSystems->AddRecord(r);
   };

   virtual void Save(const Char_t*);

   KVINDRADBRun* GetRun(Int_t run) const;
//   KVINDRADBRun *GetRun(const Char_t * run) const;

   virtual KVSeqCollection* GetRuns() const;
   virtual KVDBSystem* GetSystem(const Char_t* system) const;
   virtual KVSeqCollection* GetSystems() const;

   virtual KVDBTape* GetTape(Int_t tape) const;
   virtual KVSeqCollection* GetTapes() const;

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

   void WriteRunListFile() const;
   void ReadNewRunList();
   void WriteSystemsFile() const;

   const Char_t* GetDBEnv(const Char_t* type) const;
   const Char_t* GetCalibFileName(const Char_t* type) const
   {
      return GetDBEnv(type);
   };

   inline virtual void cd();
   virtual void WriteObjects(TFile*);
   virtual void ReadObjects(TFile*);

   KVINDRAPulserDataTree* GetPulserData()
   {
      return fPulserData;
   };

   virtual void PrintRuns(KVNumberList&) const;

   ClassDef(KVINDRADB, 4)       //DataBase of parameters for an INDRA campaign
};

//........ global variable
R__EXTERN KVINDRADB* gIndraDB;

//.............inline functions

inline void KVINDRADB::cd()
{
   KVDataBase::cd();
   gIndraDB = this;
}

inline KVSeqCollection* KVINDRADB::GetRuns() const
{
   return fRuns->GetRecords();
}

inline KVDBSystem* KVINDRADB::GetSystem(const Char_t* system) const
{
   return (KVDBSystem*) fSystems->GetRecord(system);
}

inline KVSeqCollection* KVINDRADB::GetSystems() const
{
   return fSystems->GetRecords();
}

inline KVDBTape* KVINDRADB::GetTape(Int_t tape) const
{
   return (KVDBTape*) fTapes->GetRecord(tape);
}

inline KVSeqCollection* KVINDRADB::GetTapes() const
{
   return fTapes->GetRecords();
}


inline KVINDRADBRun* KVINDRADB::GetRun(Int_t run) const
{
   //Returns KVINDRADBRun describing run number 'run'
   return (KVINDRADBRun*) fRuns->GetRecord(run);
}

//inline KVINDRADBRun *KVINDRADB::GetRun(const Char_t * run) const
//{
//   //Returns KVINDRADBRun describing run with name "run"
//   return (KVINDRADBRun *) GetRuns()->FindObject(run);
//}

#endif
