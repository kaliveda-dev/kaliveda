#include "Sive503.h"

//Author: Maurycy Rejmund

/**
   WARNING: This class has been deprecated and will eventually be removed. Do
   not use!

   This class is only compiled if the deprecated code is enabled in the build
   configuration (e.g. cmake -DUSE_DEPRECATED_VAMOS=yes). If you enable the
   deprecated code then a large number of warnings will be printed to the
   terminal. To disable these warnings (not advised) compile VAMOS with
   -Wno-deprecated-declarations. Despite these warnings the code should compile
   just fine. The warnings are there to prevent the unwitting use of the
   deprecated code (which should be strongly discouraged).

   BY DEFAULT THIS CLASS IS NOT COMPILED.

   Deprecated by: Peter Wigg (peter.wigg.314159@gmail.com)
   Date:          Thu 17 Dec 17:24:38 GMT 2015
*/

ClassImp(Sive503)

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>Sive503</h2>
<p>
Part of the VAMOS analysis package kindly contributed by Maurycy Rejmund (GANIL).
</p>
<body>
Energy and Tof calibrations for the Silicon detectors:
   <ul>
      <li> A loop is done on raw Si multiplicity, even if the signal is in the pedestal.
      <li> If the signal is above the pedestal, the energy is calculated and the Tof offset is applied.
      <li> The multiplcity (above the pedestal) is also calculated
   </ul>
</body>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

Sive503::Sive503(LogFile* Log)
{
#ifdef DEBUG
   cout << "Si::Constructor" << endl;
#endif

   char line[255];
   int len = 255;
   Float_t tmp1, tmp2;

   L = Log;

   NbSilicon  = gEnv->GetValue("VAMOS.NbSilicon", -1);
   MaxRun = gEnv->GetValue("VAMOS.MaxRun", -1);
   cout << "NbSilicon : " << NbSilicon << endl;
   if (NbSilicon < 0) {
      cout << "Not Reading VAMOS.NbSilicon in Siv Class" << endl;
   }
   if (MaxRun < 0) {
      cout << "Not Reading VAMOS.MaxRun in Siv Class" << endl;
   }

   E_Raw = new UShort_t[NbSilicon];
   SiERaw = new Int_t[NbSilicon];
   E_Raw_Nr = new UShort_t[NbSilicon];

   //Calibration coeff
   si_ped = new Float_t[NbSilicon];
   ECoef = new Float_t* [NbSilicon];
   TOff = new Float_t* [NbSilicon];
   for (Int_t i = 0; i < NbSilicon; i++) {
      ECoef[i] = new Float_t[3];
      TOff[i] = new Float_t[2];
   }

   TRef = new Double_t[NbSilicon];
   Tpropre_el = new Double_t[NbSilicon];
   Tfrag = new Double_t[NbSilicon];
   T_final = new Double_t[NbSilicon];
   Offset_relativiste = new Float_t[MaxRun];

   si_thick =  new Double_t[NbSilicon];

   E = new Float_t[NbSilicon];
   DetSi = new Int_t[NbSilicon];

   InitSavedQuantities();
   Init();

   Rnd = new TRandom3();

   ifstream inf;

   int tmp = 0;
   Float_t p0, p1, p2;
   p0 = p1 = p2 = 0.0;
   Float_t pped;
   pped = 0.0;

   //  if(!gDataSet->OpenDataSetFile("Si_calib2.dat",inf))
   if (!gDataSet->OpenDataSetFile("Si.cal", inf)) {
      cout << "Could not open the calibration file Si.cal !" << endl;
      return;
   } else {
      cout.setf(ios::showpoint);
      cout << "Reading Si.cal" << endl;
      L->Log << "Reading Si.cal" << endl;
      inf.getline(line, len);
      //cout << line << endl;
      //L->Log << line << endl;
      while (!inf.eof()) {
         inf.getline(line, len);
         if (!inf.eof() && (tmp < 45)) { //originalement tmp<45
            if (tmp < 21) {
               //sscanf(line,"%f %f %f %f %f %f", ECoef[tmp]+0, ECoef[tmp]+1, ECoef[tmp]+2, &tmp1,&tmp2, si_ped[tmp]);
               sscanf(line, "%f %f %f %f %f %f", &p0, &p1, &p2, &tmp1, &tmp2, &pped);
               if (tmp < NbSilicon) {
                  ECoef[tmp][0] = p0 * tmp1;
                  ECoef[tmp][1] = p1 * tmp1;
                  ECoef[tmp][2] = p2 * tmp1;
                  si_ped[tmp] = pped;
                  //ECoef[tmp][i] *= tmp1;
               }
               //L->Log<<"ECoef : "<<ECoef[tmp][0]<<" "<<"-524 : "<<ECoef[0][0]<<endl;
               tmp++;
            } else if (tmp < 42) { //originalement tmp<42
               sscanf(line, "%f %f", &p0, &p1);  //Don't use those values 0
               //L->Log<<  "TO " <<TOff[tmp-21][0] << " " << TOff[tmp-21][1]<< endl;
               tmp++;
            } else {
               sscanf(line, "%f %f %f %f %f", TCoef[tmp - 42] + 0, TCoef[tmp - 42] + 1, TCoef[tmp - 42] + 2, TCoef[tmp - 42] + 3, TCoef[tmp - 42] + 4); // originalement tmp-42
               //L->Log <<  "TC " <<TCoef[tmp-42][0] << " " << TCoef[tmp-42][1]<< endl;
               tmp++;
            }
         }
      }
   }
#ifdef DEBUG
   for (int h = 0; h < NbSilicon; h++) {
      cout << "Ecoef[" << h << "][0]=" << ECoef[h][0] << " Ecoef[" << h << "][1]=" << ECoef[h][1] << "Ecoef[" << h << "][2]=" << ECoef[h][2] << endl << flush;
   }
#endif

   inf.close();


   Int_t  num;
   Float_t ref;
   Float_t propre;

   TString sline;

   ifstream in2;

   //ToF calibration and offsets according to the system and the run number
   if (!gDataSet->OpenDataSetFile("Offset.cal", in2)) {
      cout << "Could not open the calibration file Offset.cal !!!" << endl;
      return;
   } else {
      cout << "Reading Offset.cal" << endl;
      L->Log << "Reading Offset.cal" << endl;
      while (!in2.eof()) {
         sline.ReadLine(in2);
         if (!in2.eof()) {
            if (!sline.BeginsWith("+") && !sline.BeginsWith("|")) {
               sscanf(sline.Data(), "%d %f %f", &num , &ref, &propre);
               //L->Log << "SI_" << num << ": Ref : "<< ref << endl;
               TRef[num] = ref;
               Tpropre_el[num] = propre;

               //L->Log << "TRef[num] : " << TRef[num] << " Tpropre_el[num] :  "<< Tpropre_el[num] << endl;
               //cout<< "TRef[num] : " << TRef[num] << " Tpropre_el[num] :  "<< Tpropre_el[num] << endl;
            }
         }
      }
   }
   in2.close();

   TString sline3;
   ifstream in3;
   Float_t brho0, aq, sig_aq, off_relativiste;
   Int_t run1, run2;


   //ToF Offset for each run
   if (!gDataSet->OpenDataSetFile("Offset_relativiste.dat", in3)) {
      cout << "Could not open the calibration file Offset_relativiste.cal !!!" << endl;
      return;
   } else {
      cout << "Reading Offset_relativiste.dat" << endl;
      L->Log << "Reading Offset_relativiste.dat" << endl;
      while (!in3.eof()) {
         sline3.ReadLine(in3);
         if (!in3.eof()) {
            if (!sline3.BeginsWith("+") && !sline3.BeginsWith("#")) {
               sscanf(sline3.Data(), "%f %d %d %f %f %f", &brho0, &run1, &run2, &off_relativiste, &aq, &sig_aq);
               for (Int_t i = run1; i < run2 + 1; i++) {
                  Offset_relativiste[i] = off_relativiste;
                  //cout<<"offset relativiste : "<<i<<"  "<<Offset_relativiste[i]<<endl;
               }
            }
         }
      }
   }
   in3.close();

   Int_t  numm;
   Int_t thi = 0;
   ifstream in;
   TString sline2;

   if (!gDataSet->OpenDataSetFile("thick.dat", in)) {
      cout << "Could not open the calibration file thick.dat !!!" << endl;
      return;
   } else {
      while (!in.eof()) {
         sline2.ReadLine(in);
         if (!in.eof()) {
            if (!sline2.BeginsWith("#")) {
               sscanf(sline2.Data(), "%d %d", &numm, &thi);
               si_thick[numm] = thi;
            }
         }
      }
   }
   in.close();

   //Ready=kTRUE;
}
Sive503::~Sive503(void)
{
   delete [] E_Raw;
   delete [] SiERaw;
   delete []  E_Raw_Nr;

   //Calibration coeff
   delete [] si_ped ;
   delete [] ECoef;
   delete [] TOff;

   delete [] TRef;
   delete [] Tpropre_el;
   delete [] Tfrag ;
   delete [] T_final;
   delete [] Offset_relativiste;    //(MAXRUN)

   delete [] si_thick;

   delete [] E;
   delete [] DetSi;

   delete Rnd;

#ifdef DEBUG
   cout << "Si::Destructor" << endl;
#endif

   PrintCounters();
}

void Sive503::PrintCounters(void)
{
#ifdef DEBUG
   cout << "Si::PrintCounters" << endl;
#endif

   cout << endl;
   cout << "Si::PrintCounters" << endl;
   cout << "Called: " << Counter[0] << endl;
   cout << "Present: " << Counter[1] << endl;

   L->Log << endl;
   L->Log << "Si::PrintCounters" << endl;
   L->Log << "Called: " << Counter[0] << endl;
   L->Log << "Present: " << Counter[1] << endl;

}


void Sive503::InitSavedQuantities(void)
{
#ifdef DEBUG
   cout << "Si::InitSavedQuantities" << endl;
#endif

   for (Int_t i = 0; i < NbSilicon; i++) {
      SiERaw[i] = -10;

      E[i] = -10.0;
      Tfrag[i] = -10.0;
      T_final[i] = -10.0;
      DetSi[i] = -10;
   }
   for (Int_t i = 0; i < 3; i++) {
      T[i] = -10.0;
   }
   EMSI = 0;
}

void Sive503::Init(void)
{
#ifdef DEBUG
   cout << "Sive503::Init" << endl;
#endif
   Present = false;

   E_RawM = -10;
   T_Raw[0] = -10;

   for (Int_t i = 0; i < NbSilicon; i++) {
      E_Raw[i] = -10;
      E_Raw_Nr[i] = -10;

      si_ped[i] = -10.0;
      TRef[i] = -10.0;
      Tpropre_el[i] = -10.0;
      si_thick[i] = -10.0;

      for (Int_t j = 0; j < 3; j++) {
         ECoef[i][j] = -10.0;
         if (j < 2)  TOff[i][j] = -10.0;
      }
   }

   for (Int_t i = 0; i < 3; i++) {
      for (Int_t j = 0; j < 5; j++) {
         TCoef[i][j] = -10.0;
      }
   }
   for (Int_t i = 0; i < MaxRun; i++) {
      Offset_relativiste[i] = 0.0;
   }
   for (Int_t i = 0; i < 2; i++)
      Counter[i] = 0;
   Ready = kFALSE;

   fTofOffset = -10.0;
}

void Sive503::SetOffsetRelativisteRun(Int_t runnumber)
{

   fTofOffset = Offset_relativiste[runnumber];
   Ready = kTRUE;
   if (fTofOffset == -10.0) {
      Ready = kFALSE;
      cout << "Offset not defined for this run. Check file Offset_relativiste.dat" << endl;
   }
   return;
}


void Sive503::Calibrate(void)
{

   InitSavedQuantities();

   Int_t i, k;

   Float_t run_offset = fTofOffset;

#ifdef DEBUG
   cout << "Si::Calibrate" << endl;
#endif

   //One value of T_Raw for all silicon detectors, because the start is given by the OR of Si and the stop by the HF.
   for (i = 0; i < 1; i++) { //T[0] : TSi-HF, T[1] : TSi-SED, T[2] : TSED-HF.   Don't have SED in this experiement (i<1)
      if (T_Raw[i] > TCoef[i][4])
         T[i] = (Float_t) TCoef[i][1] + (T_Raw[i] * TCoef[i][2]);
      if (T_Raw[i] <= TCoef[i][4])
         T[i] = (Float_t) TCoef[i][1] + (T_Raw[i] * TCoef[i][2]) + (TCoef[i][3] * powf((T_Raw[i] - TCoef[i][4]), 2));
   }


   //L->Log<<"Sive503.cpp :   "<<"T_Raw[0] : "<<T_Raw[0]<<" "<<"T[0] : "<<T[0]<<endl;
   //L->Log<<"TCoef[0][1] : "<<TCoef[0][1]<<" "<<"TCoef[0][2] : "<<TCoef[0][2]<<" "<<"TCoef[0][3] : "<<TCoef[0][3]<<" "<<"TCoef[0][4] : "<<TCoef[0][4]<<endl;


   for (i = 0; i < E_RawM; i++) { //Loop on all hit Si detectors, even if the signal is in the pedestal
      Rnd->SetSeed(clock());

      if (E_Raw[i] > si_ped[E_Raw_Nr[i]]) {   //if the signal is above the pedestal.
         //We save the channel value, the detector number, the calibrated energy, the ToF and increase the multiplicty of hit Si.
         E[EMSI] = 0.0;
         for (k = 0; k < 3; k++) { //energy calibration. Pedestal is taken into account in the formula
            E[EMSI] += powf((Float_t) E_Raw[i] +   Rnd->Uniform(0, 1),
                            (Float_t) k) * ECoef[E_Raw_Nr[i]][k];
            //     cout<<"ECoeff="<<ECoef[E_Raw_Nr[i]][k]<<endl<<flush;
         }
         SiERaw[EMSI] = E_Raw[i];
         DetSi[EMSI] = E_Raw_Nr[i] + 1; //numérotation 1-18

         Tfrag[EMSI] = TRef[E_Raw_Nr[i]] + Tpropre_el[E_Raw_Nr[i]] - T[0];
         T_final[EMSI] = Tfrag[i] + run_offset;

         //cout<<"===Si==="<<endl;
         //cout<<"SiERaw : "<<SiERaw[EMSI]<<"   DetSi[EMSI] : "<<DetSi[EMSI]<<"  E : "<<E[EMSI]<<" T_final[EMSI] : "<<T_final[EMSI]<<endl;

         Counter[1]++;
         EMSI++;
      }

   }
   //cout<<"Mult Si : "<<EMSI<<endl;
   //cout<<"====="<<endl;
}

void Sive503::Treat(void)
{
#ifdef DEBUG
   cout << "Sive503::Treat" << endl;
#endif
   if (!Ready) {
      cout << "Call Sive503 Constructor and call SetOffsetRelativisteRun(runnumber) for each run" << endl;
      return;
   }
   Counter[0]++;
   Calibrate();
#ifdef DEBUG
   Show_Raw();
   Show();
#endif
}

void Sive503::inAttach(TTree* inT)
{

#ifdef DEBUG
   cout << "Si::inAttach" << endl;
#endif

#ifdef DEBUG
   cout << "Attaching Si variables" << endl;
#endif

#ifdef ACTIVEBRANCHES
   cout << "Activating Branches: Si" << endl;
   L->Log << "Activating Branches: Si" << endl;

   inT->SetBranchStatus("SIEM", 1);
   inT->SetBranchStatus("SIE", 1);
   inT->SetBranchStatus("SIENr", 1);

   inT->SetBranchStatus("TSI_HF", 1);
   inT->SetBranchStatus("TSI_SED", 1);
   inT->SetBranchStatus("TSED_HF", 1);



#endif

   inT->SetBranchAddress("SIEM", &E_RawM);
   inT->SetBranchAddress("SIE", E_Raw);
   inT->SetBranchAddress("SIENr", E_Raw_Nr);

   inT->SetBranchAddress("TSI_HF", T_Raw + 0);
   //inT->SetBranchAddress("TSI_SED",T_Raw+1);
   //inT->SetBranchAddress("TSED_HF",T_Raw+2);

}
void Sive503::outAttach(TTree* outT)
{

#ifdef DEBUG
   cout << "Si::inAttach" << endl;
#endif

#ifdef DEBUG
   cout << "Attaching Si variables" << endl;
#endif

   outT->Branch("SiEM", &EMSI, "EMSI/I");

   outT->Branch("SiRaw", SiERaw, "SiERaw[EMSI]/I");
   outT->Branch("ESi", E, "E[EMSI]/F");

   //outT->Branch("SiE",E,"SiE[21]/F");
   //outT->Branch("SiET",&ETotal,"SiET/F");

   //outT->Branch("SiERaw",&E_Raw[0],"SiERaw/S");
   //outT->Branch("SiERaw",SiRaw,"SiERaw[21]/S");
   outT->Branch("TSI_HF_raw", T_Raw + 0, "TSI_HF_raw/s"); //Raw Signal
   outT->Branch("TSi_HFpropre", &T[0], "TSi_HFpropre/F"); //Raw signal in ns
   outT->Branch("T_wooff", Tfrag, "Tfrag[EMSI]/D");   //ToF calibrated before the offset'(depending on the run number)
   outT->Branch("T_final", T_final, "T_final[EMSI]/D"); //Final ToF with offset'(depending on the run number)

   outT->Branch("DetSi", DetSi, "DetSi[EMSI]/I");

}
void Sive503::CreateHistograms(void)
{

#ifdef DEBUG
   cout << "Si::CreateHistograms" << endl;
#endif

}
void Sive503::FillHistograms(void)
{

#ifdef DEBUG
   cout << "Si::FillHistograms" << endl;
#endif

}


void Sive503::Show_Raw(void)
{
   Int_t i;
#ifdef DEBUG
   cout << "Si::Show_Raw" << endl;
#endif

   cout.setf(ios::showpoint);

   for (i = 0; i < E_RawM; i++) {
      cout << "CrER: " << E_Raw_Nr[i] << " " << E_Raw[i] << endl;
   }

}
void Sive503::Show(void)
{
   Int_t i;
#ifdef DEBUG
   cout << "Si::Show" << endl;
#endif

   cout.setf(ios::showpoint);

   cout << "Present: " << Present << endl;
   if (Present) {
      cout << "EM: " << EMSI << endl;
      for (i = 0; i < 21; i++) {
         if (E[i] > 0.) cout << "SiE: " << i << " " << E[i] << endl;
      }
   }
}

