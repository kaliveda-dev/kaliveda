/*
$Id: KVINDRADB_e503.cpp,v 1.1 2009/01/22 15:39:26 franklan Exp $
$Revision: 1.1 $
$Date: 2009/01/22 15:39:26 $
*/

//Created by KVClassFactory on Wed Jan 21 13:44:30 2009
//Author: Chbihi

#include "KVINDRADB_e503.h"
#include "KVDBParameterSet.h"

using namespace std;

ClassImp(KVINDRADB_e503)


void KVINDRADB_e503::init()
{
   fDeltaPed = AddTable("DeltaPedestal", "Pedestal correction value of detectors");
}

KVINDRADB_e503::KVINDRADB_e503()
{
   // Default constructor
   init();
}

//________________________________________________________________

KVINDRADB_e503::KVINDRADB_e503(const Char_t* name) : KVINDRADB(name)
{
   init();
}

//________________________________________________________________

KVINDRADB_e503::~KVINDRADB_e503()
{
   // Destructor
}

//________________________________________________________________

void KVINDRADB_e503::Build()
{
   // Read runlist and systems list, then read VAMOS scalers
   // Read Brho and angle of Vamos for each run

   KVINDRADB::Build();
   ReadVamosScalers();
   ReadVamosBrhoAndAngle();
   ReadPedestalCorrection();
}

//________________________________________________________________

void KVINDRADB_e503::ReadVamosScalers()
{
   ifstream fin;

   if (!OpenCalibFile("VamosScaler", fin)) {
      Warning("ReadVamosScalers", "VAMOS scalers file not found : %s",
              GetCalibFileName("VamosScaler"));
      return;
   }
   Info("ReadVamosScalers", "Reading in Vamos Scaler file : %s",
        GetCalibFileName("VamosScaler"));

   Float_t  NormScaler = 0;
   Float_t DT = 0;
   Float_t NormVamos = 0;
   TString sline;
   Int_t irun = 0;
   Int_t ScalerValue;
   Char_t Crunnumber[40];
   Char_t str[30];
   while (fin.good()) {         //reading the file
      sline.ReadLine(fin);
      if (!fin.eof()) {          //fin du fichier
         if (sline.Sizeof() > 1 && !sline.BeginsWith("#")) {
            if (sline.BeginsWith("ScalersOfRunNumber")) {
               sscanf(sline.Data(), "%30s %d ", Crunnumber, &irun);
               KVINDRADBRun* idb = GetRun(irun);
               if (idb) {
                  for (Int_t i = 0; i < 5; i++) {  //retrouve dans les 4 prochaines lignes NormVamos,normSca etDT
                     sline.ReadLine(fin);

                     if (sline.BeginsWith("NormVamos")) {
                        sscanf(sline.Data(), "%20s %f ", str, &NormVamos);
                        idb->Set(str, NormVamos);
                     }
                     if (sline.BeginsWith("NormScalers")) {
                        sscanf(sline.Data(), "%20s %f ", str, &NormScaler);
                        idb->Set(str, NormScaler);
                     }
                     if (sline.BeginsWith("DT")) {
                        sscanf(sline.Data(), "%20s %f ", str, &DT);
                        idb->Set(str, DT);
                     }

                  }
                  for (Int_t i = 0; i < 40; i++) {
                     sline.ReadLine(fin);
                     sscanf(sline.Data(), "%s %d ", str, &ScalerValue);
                     idb->SetScaler(str, ScalerValue);
                  }

               }
            }
         }
      }
   }
   fin.close();
}

//________________________________________________________________

void KVINDRADB_e503::ReadVamosBrhoAndAngle()
{
   TString sline;
   ifstream fin;

   if (!OpenCalibFile("VamosBrhoAngle", fin)) {
      Warning("VamosBrhoAngle", "VAMOS Brho and angle file not found : %s",
              GetCalibFileName("VamosBrhoAngle"));
      return;
   }

   Info("ReadVamosBrhoAngle", "Reading in VamosBrho and angle file : %s",
        GetCalibFileName("VamosBrhoAngle"));

   Int_t run = 0;
   Float_t Brho = -1;
   Float_t theta = -1.;
   while (fin.good()) {         //reading the file
      sline.ReadLine(fin);
      if (!fin.eof()) {          //fin du fichier
         if (sline.Sizeof() > 1 && !sline.BeginsWith("#")) {
            sscanf(sline.Data(), "%d %f %f ", &run, &Brho, &theta);
//                cout<<" run = "<<run<<", Brho = "<<Brho<<", Theta = "<<theta<<endl;
            if (Brho == 0) {
               Brho = -1.;
               theta = -1.;
            }
            KVINDRADBRun* idb = GetRun(run);
            if (idb) {
               idb->Set("Brho", Brho);
               idb->Set("Theta", theta);
            }
         }
      }
   }

   fin.close();
}
//________________________________________________________________

void KVINDRADB_e503::ReadPedestalCorrection()
{
   // Reading the pedestal correction (DeltaPed). DeltaPed depends on
   // the run number but it is the same for all detectors connected to the
   // same QDC. The corrected pedestal is given by the reference
   // pedestal + DeltaPed.
   ifstream fin;

   if (!OpenCalibFile("PedestalCorrections", fin)) {
      Warning("ReadPedestalCorrection", "Pedestal correction file not found : %s",
              GetCalibFileName("PedestalCorrections"));
      return;
   }
   Info("ReadPedestalCorrection", "Reading in pedestal correction file : %s",
        GetCalibFileName("PedestalCorrections"));


   TString sline;
   while (fin.good()) {         //reading the file
      sline.ReadLine(fin);
      // Skip comment line
      if (sline.BeginsWith("#")) continue;

      if (sline.BeginsWith("+DeltaPed")) ReadDeltaPedestal(fin);
   }
   fin.close();
}
//________________________________________________________________

void KVINDRADB_e503::ReadDeltaPedestal(ifstream& ifile)
{
   // Reading the pedestal correction (DeltaPed). Method called by
   // KVINDRADB_e503::ReadPedestalCorrection(). DeltaPed is equal to
   // DeltaNoise for high gain coder data. For anyother coder data,
   // DetaPed is given by DeltaGene if DeltaGene is less than 20 channels,
   // otherwise it is given by DeltaNoise. DeltaGene (DeltaNoise) is the
   // difference between pedestal positions from the generator (noise)
   // of the current run and of the reference run.

   KVString sline, signal, parname;
   KVDBParameterSet* parset = NULL;
   Int_t numQDC = -1;
   KVNumberList runs;
   Double_t Dped = -1., Dgene = -1., Dbruit = -1.;

   while (ifile.good()) {         //reading the file
      sline.ReadLine(ifile);
      // Skip comment line
      if (sline.BeginsWith("#")) continue;
      // End character
      if (sline.BeginsWith("!")) return;

      // QDC number
      if (sline.BeginsWith("QDC=")) {
         sline.Remove(0, 4);
         numQDC = sline.Atoi();
         continue;
      }
      // DeltaPed for each signal
      Ssiz_t idx = sline.Index(":");
      runs.SetList(TString(sline).Remove(idx));
      sline.Remove(0, idx + 1);
      sline.Begin(" ");
      while (!sline.End()) {
         signal = sline.Next(kTRUE);
         Dgene  = sline.Next(kTRUE).Atof();
         Dbruit = sline.Next(kTRUE).Atof();

         // Marie-France's selection
         Dped = ((Dgene > 20) || !strcmp(signal.Data(), "GG") ? Dbruit : Dgene);
         if (Dped > 20) Warning("ReadDeltaPedestal", "DeltaPed>20 for runs %s, signal %s", runs.GetList(), signal.Data());

         parname.Form("QDC%.2d", numQDC);
         parset = new KVDBParameterSet(parname.Data(), signal.Data(), 1);
         parset->SetParameter(Dped);
         fDeltaPed->AddRecord(parset);
         LinkRecordToRunRange(parset, runs);
      }
   }
}
//----------------------------------------------------------------------------------
void KVINDRADB_e503::ReadPedestalList()
{
//Read the names of pedestal files to use for each run range, found
//in file with name defined by the environment variable:
//   [dataset name].INDRADB.Pedestals:    ...

   ifstream fin;
   if (!OpenCalibFile("Pedestals", fin)) {
      Error("ReadPedestalList()", "Could not open file %s",
            GetCalibFileName("Pedestals"));
      return;
   }
   Info("ReadPedestalList()", "Reading pedestal file list...");

   KVString line;
   Char_t filename_chio[80], filename_csi[80], filename_etalons[80];
   UInt_t runlist[1][2];

   while (fin.good()) {         //lecture du fichier

// lecture des informations
      line.ReadLine(fin);

//recherche une balise '+'
      if (line.BeginsWith('+')) {       //balise trouvee

         line.Remove(0, 1);

         if (sscanf
               (line.Data(), "Run Range : %u %u", &runlist[0][0],
                &runlist[0][1]) != 2) {
            Warning("ReadPedestalList()", "Format problem in line \n%s",
                    line.Data());
         }

         line.ReadLine(fin);
         sscanf(line.Data(), "%s", filename_chio);

         line.ReadLine(fin);
         sscanf(line.Data(), "%s", filename_csi);

         line.ReadLine(fin);
         sscanf(line.Data(), "%s", filename_etalons);

         TList RRList;
         KVDBRecord* dummy = 0;
         dummy = new KVDBRecord(filename_chio, "ChIo/Si pedestals");
         dummy->AddKey("Runs", "Runs for which to use this pedestal file");
         fPedestals->AddRecord(dummy);
         RRList.Add(dummy);
         dummy = new KVDBRecord(filename_csi, "CsI pedestals");
         dummy->AddKey("Runs", "Runs for which to use this pedestal file");
         fPedestals->AddRecord(dummy);
         RRList.Add(dummy);
         dummy = new KVDBRecord(filename_etalons, "Si75/SiLi pedestals");
         dummy->AddKey("Runs", "Runs for which to use this pedestal file");
         fPedestals->AddRecord(dummy);
         RRList.Add(dummy);
         LinkListToRunRanges(&RRList, 1, runlist);
      }                         // balise trouvee
   }                            // lecture du fichier
   fin.close();
   cout << "Pedestals Read" << endl;
}
