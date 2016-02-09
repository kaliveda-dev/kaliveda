#include "SeDv.h"

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

using namespace std;

ClassImp(SeDv)

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>SeDv</h2>
<p>
Part of the VAMOS analysis package kindly contributed by Maurycy Rejmund (GANIL).
</p>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

SeDv::SeDv(LogFile* Log, short nr)
{
#ifdef DEBUG
   cout << "SeDv::Constructor" << endl;
#endif
   Ready = kFALSE;

   char line[255];
   int len = 255;
   int i, j, k;
   char fname[20];

   L = Log;
   lnr = nr;

   for (i = 0; i < 14; i++)
      Counter[i] = 0;
   for (i = 0; i < 4; i++)
      for (j = 0; j < 5; j++)
         Counter1[i][j] = 0;

   InitRaw();
   Zero();

   Rnd = new Random;

   NStrips = 3;
   cout << "Number of considered strips: " << NStrips << endl;

   L->Log << "Number of considered strips: " << NStrips << endl;


   sprintf(fname, "SeD%1dRef.cal", lnr);
   ifstream inf1;
   if (!gDataSet->OpenDataSetFile(fname, inf1)) {
      cout << "Could not open the Reference file SeDRef.cal !" << endl;
      return;
   } else {
      cout.setf(ios::showpoint);
      cout << "Reading " << fname << endl;
      L->Log << "Reading " << fname << endl;
      inf1.getline(line, len);
      cout << line << endl;
      L->Log << line << endl;
      for (i = 0; i < 2; i++)
         inf1 >> XRef[i];

      inf1.getline(line, len);
      inf1.getline(line, len);
      cout << line << endl;
      L->Log << line << endl;
      inf1 >> QThresh;
      cout << QThresh << endl;
      L->Log << QThresh << endl;

      cout << endl;
      L->Log << endl;
   }
   inf1.close();


   sprintf(fname, "SeD%1d.cal", lnr);
   ifstream inf;

   if (!gDataSet->OpenDataSetFile(fname, inf)) {
      cout << "Could not open the calibration file " << fname << endl;
      return;
   } else {
      cout.setf(ios::showpoint);
      cout << "Reading " << fname << endl;
      L->Log << "Reading " << fname << endl;


      inf.getline(line, len);
      cout << line << endl;
      L->Log << line << endl;
      for (i = 0; i < 3; i++)
         for (j = 0; j < 2; j++)
            inf >> ECoef[i][j];

      inf.getline(line, len);
      inf.getline(line, len);
      cout << line << endl;
      L->Log << line << endl;
      for (i = 0; i < 1; i++)
         for (j = 0; j < 2; j++)
            inf >> TCoef[i][j];

      inf.getline(line, len);
      inf.getline(line, len);
      cout << line << endl;
      L->Log << line << endl;

      for (k = 0; k < 1; k++)
         for (i = 0; i < 128; i++)
            for (j = 0; j < 3; j++)
               inf >> QCoef[i][k][j];

      inf.getline(line, len);
      inf.getline(line, len);
      cout << line << endl;
      L->Log << line << endl;

      for (k = 1; k < 2; k++)
         for (i = 0; i < 48; i++)
            for (j = 0; j < 3; j++)
               inf >> QCoef[i][k][j];

   }
   inf.close();
   Ready = kTRUE;
}
SeDv::~SeDv(void)
{
#ifdef DEBUG
   cout << "SeDv::Destructor" << endl;
#endif

   PrintCounters();
}

void SeDv::PrintCounters(void)
{
   Int_t i;
#ifdef DEBUG
   cout << "SeDv::PrintCounters" << endl;
#endif

   cout << endl;
   cout << "SeDv::PrintCounters" << endl;
   cout << "Called: " << Counter[0] << endl;
   cout << "E1: " << Counter[1] << endl;
   cout << "E2: " << Counter[2] << endl;
   cout << "T1: " << Counter[3] << endl;
   cout << "T2: " << Counter[4] << endl;
   cout << "PresentWires: " << Counter[5] << endl;
   cout << "STRIP Mult >= " << NStrips << endl;
   for (i = 0; i < 4; i++)
      cout << Counter1[i][0] << " ";
   cout << endl;
   cout << "PresentStrips: " << Counter[6] << endl;
   cout << "X: Sequence Hiperbolique" << endl;
   for (i = 0; i < 2; i++)
      cout << Counter1[i][1] << " ";
   cout << endl;
   cout << "X: Weighted Average" << endl;
   for (i = 0; i < 2; i++)
      cout << Counter1[i][2] << " ";
   cout << endl;
   cout << "X: MultiplePeak" << endl;
   for (i = 0; i < 2; i++)
      cout << Counter1[i][3] << " ";
   cout << endl;
   cout << "PresentSubseqX: " << Counter[7] << endl;
   cout << "PresentSubseqY: " << Counter[8] << endl;
   cout << "Present: " << Counter[9] << endl;


   L->Log << endl;
   L->Log << "SeDv::PrintCounters" << endl;
   L->Log << "Called: " << Counter[0] << endl;
   L->Log << "E1: " << Counter[1] << endl;
   L->Log << "E2: " << Counter[2] << endl;
   L->Log << "T1: " << Counter[3] << endl;
   L->Log << "T2: " << Counter[4] << endl;
   L->Log << "PresentWires: " << Counter[5] << endl;
   L->Log << "STRIP Mult >= " << NStrips << endl;
   for (i = 0; i < 2; i++)
      L->Log << Counter1[i][0] << " ";
   L->Log << endl;
   L->Log << "PresentStrips: " << Counter[6] << endl;
   L->Log << "X: Sequence Hiperbolique" << endl;
   for (i = 0; i < 2; i++)
      L->Log << Counter1[i][1] << " ";
   L->Log << endl;
   L->Log << "X: Weighted Average" << endl;
   for (i = 0; i < 2; i++)
      L->Log << Counter1[i][2] << " ";
   L->Log << endl;
   L->Log << "X: MultiplePeak" << endl;
   for (i = 0; i < 2; i++)
      L->Log << Counter1[i][3] << " ";
   L->Log << endl;
   L->Log << "PresentSubseqX: " << Counter[7] << endl;
   L->Log << "PresentSubseqY: " << Counter[8] << endl;
   L->Log << "Present: " << Counter[9] << endl;


}

void SeDv::Zero(void)
{
#ifdef DEBUG
   cout << "SeDv::Zero" << endl;
#endif
   Present = PresentWires = PresentStrips = false;


   E[0] = E[1] = E[2] = T[0] = 0.0;
   Mult[0] = Mult[1] = 0;
   for (Int_t j = 0; j < 2; j++)
      for (Int_t i = 0; i < 128; i++) {
         Q[i][j] = 0.0;
         N[i][j] = 0 ;
      }
   X[0] = X[1] = -500.0;
   XS[0] = XS[1] = -500.0;
   XWA[0] = XWA[1] = -500.0;
}

void SeDv::InitRaw(void)
{
#ifdef DEBUG
   cout << "SeDv::InitRaw" << endl;
#endif
   for (Int_t i = 0; i < 3; i++)
      E_Raw[i] = 0;
   T_Raw[0] = 0;

   for (Int_t i = 0; i < 128; i++)
      for (Int_t j = 0; j < 2; j++) {
         Q_Raw[j * 128 + i] = 0;
         Q_Raw_Nr[j * 128 + i] = 0;
      }

   for (Int_t j = 0; j < 2; j++)
      Q_RawM[j] = 0;

}

void SeDv::Init(void)
{
#ifdef DEBUG
   cout << "SeDv::Init" << endl;
#endif
   Present = PresentWires = PresentStrips = false;

   E[0] = E[1] = E[2] = T[0] = 0.0;
   Mult[0] = Mult[1]  = 0;
   X[0] = X[1] = -500.0;
   XS[0] = XS[1] = -500.0;
   XWA[0] = XWA[1] = -500.0;
}

void SeDv::Calibrate(void)
{
#ifdef DEBUG
   cout << "SeDv::Calibrate" << endl;
#endif

   Int_t i, j, k;
   Float_t QTmp;

   for (i = 0; i < 3; i++)
      if (E_Raw[i] > 0) {
         Rnd->Next();
         for (j = 0; j < 2; j++)
            E[i] += powf((Float_t)E_Raw[i] + Rnd->Value(),
                         (Float_t) j) * ECoef[i][j];
      }
   for (i = 0; i < 1; i++)
      if (T_Raw[i] > 0) {
         Rnd->Next();
         for (j = 0; j < 2; j++)
            T[i] += powf((Float_t) T_Raw[i] + Rnd->Value(),
                         (Float_t) j) * TCoef[i][j];
      }

   if (E[0] > 0.0) Counter[1]++;
   if (E[1] > 0.0) Counter[2]++;
   if (E[2] > 0.0) Counter[3]++;
   if (T[0] > 0.0) Counter[4]++;

   if ((E[0] > 0.0 || E[1] > 0.0 || E[2] > 0.0) && T[0] > 0.0) {
      PresentWires = true;
      Counter[5]++;
   } else {
      E[0] = E[1] = E[2] = T[0] = 0.0;
   }

   if (!PresentWires) {
      for (i = 0; i < 2; i++) //loop
         for (j = 0; j < Q_RawM[i]; j++) { //loop over strips
            QTmp = 0.0;
            Rnd->Next();
            for (k = 0; k < 3; k++) //loop over coeff
               QTmp += powf((Float_t) Q_Raw[j + i * 128] + Rnd->Value(),
                            (Float_t)k) * QCoef[Q_Raw_Nr[j + i * 128]][i][k];
            if (QTmp > QThresh) {
               Mult[i]++;
               Q[Mult[i] - 1][i] = QTmp;
               N[Mult[i] - 1][i] = Q_Raw_Nr[j + i * 128];
            }
         }

      for (i = 0; i < 2; i++)
         if (Mult[i] >= NStrips) Counter1[i][0]++;

      if (Mult[0] >= NStrips && Mult[1] >= NStrips) {
         PresentStrips = true;
         Counter[6]++;
      }
   }



}

void SeDv::FocalSubseqX(void)
{

   Int_t i, j, k;
   Float_t QTmp[128];
   Float_t QMax;
   Int_t NMax;
   Int_t FStrip[128];
#ifdef WEIGHTEDAVERAGE
   Int_t StripsWA;
#endif
   bool Neighbours;
   Float_t v[6];


#ifdef DEBUG
   cout << "SeDv::FocalSubseqX" << endl;
#endif

   for (i = 0; i < 2; i++) { //Loop over Chambers

      for (j = 0; j < Mult[i]; j++) //loop over Strips
         QTmp[j] = Q[j][i];
      for (k = 0; k < NStrips; k++) { //searching for NStrips Strips
         QMax = 0.0;
         NMax = 0;
         for (j = 0; j < Mult[i]; j++) { //loop over Strips
            if (QTmp[j] > QMax) {
               QMax = QTmp[j];
               NMax = j;
            }
         }
         QTmp[NMax] = 0.0;
         FStrip[k] = NMax;
      }

#ifdef MULTIPLEPEAK
      bool MultiplePeak;
      MultiplePeak = false;
      for (j = 0; j < FStrip[0] - 1; j++)
         if ((Q[j][i] > Q[j + 1][i]))
            if (Q[j][i] >= 0.4 * Q[FStrip[0]][i]) {
               MultiplePeak = true;
               Counter1[i][3]++;
#ifdef DEBUG
               cout << "Multiplepeak: " << i  << " " << N[j][i] << endl;
#endif
               break;
            }
      for (j = FStrip[0]; j < Mult[i] - 1; j++)
         if (Q[j][i] < Q[j + 1][i])
            if (Q[j + 1][i] >= 0.4 * Q[FStrip[0]][i]) {
               MultiplePeak = true;
               Counter1[i][3]++;
#ifdef DEBUG
               cout << "Multiplepeak1: " << i << " " <<  N[j][i] << endl;
#endif
               break;
            }
#endif


      if (NStrips != 3) {
         cout << "NStrips != 3 but " << NStrips << " Program it first" << endl;
         exit(EXIT_FAILURE);
      }

      Neighbours = false;

#ifdef SECHIP
      if (abs(N[FStrip[0]][i] - N[FStrip[1]][i]) == 1
            &&
            abs(N[FStrip[0]][i] - N[FStrip[2]][i]) == 1)
         Neighbours = true;
#endif


#ifdef MULTIPLEPEAK
      if (!MultiplePeak) {
#endif
         if (Neighbours) {
            v[0] = sqrtf(Q[FStrip[0]][i] / Q[FStrip[2]][i]);
            v[1] = sqrtf(Q[FStrip[0]][i] / Q[FStrip[1]][i]);
            v[2] = 0.5 * (v[0] + v[1]);
            v[3] = logf(v[2] + sqrtf(powf(v[2], 2.) - 1.0));
            v[4] = (v[0] - v[1]) / (2.0 * sinhf(v[3]));
            v[5] = 0.5 * logf((1.0 + v[4]) / (1.0 - v[4]));
            X[i] = (Float_t) N[FStrip[0]][i] -
                   (Float_t)(N[FStrip[0]][i] - N[FStrip[1]][i]) *
                   v[5] / v[3];
            if (i == 0)
               XS[i] = X[i] *= 3.2; //Goes mm
            else
               XS[i] = X[i] = X[i] * 3. / sqrtf(2.); //Goes mm
            if (!(X[i] < 500. && X[i] > 0.)) XS[i] = X[i] = -500.;
            if (X[i] > 0.) Counter1[i][1]++;
         } else { // the case of Weighted Average
#ifdef WEIGHTEDAVERAGE
            if (Mult[i] > NStrips) {
               //Looking for entire peak for W.A.
               //The Strips are ordered  0-128
               //Could be done earlier but ....
               StripsWA = 0;
               for (j = FStrip[0] - 1; j >= 0; j--)
                  if (abs(N[j + 1][i] - N[j][i]) == 1) {
                     if ((Q[j][i] <= Q[j + 1][i])) {
                        StripsWA++;
                        FStrip[StripsWA] = j;
                     } else
                        break;
                  } else
                     break;

               for (j = FStrip[0]; j < Mult[i] - 1; j++)
                  if (abs(N[j + 1][i] - N[j][i]) == 1) {
                     if (Q[j][i] >= Q[j + 1][i]) {
                        StripsWA++;
                        FStrip[StripsWA] = j + 1;
                     } else
                        break;
                  } else
                     break;

               if (StripsWA >= NStrips) {
                  v[0] = v[1] = 0.0;
                  for (k = 0; k <= StripsWA; k++) {
                     v[0] += Q[FStrip[k]][i] * ((Float_t) N[FStrip[k]][i]);
                     v[1] += Q[FStrip[k]][i];
                  }

#ifdef DEBUG
                  cout << "SeDv::FocalSubseqX::Weithed Average: " << i << endl;
                  cout << "StripsWA: " << StripsWA << endl;
                  for (k = 0; k <= StripsWA; k++)
                     cout << "STR: " << N[FStrip[k]][i] << " Q: " << Q[FStrip[k]][i] << endl;
#endif
                  X[i] = v[0] / v[1];
                  if (i == 0)
                     XWA[i] = X[i] *= 3.2; //Goes mm
                  else
                     XWA[i] = X[i] = X[i] * 3.0 / sqrtf(2.); //Goes mm
                  //       XWA[i] = X[i] = X[i]*3.0; //Goes mm

                  if (!(X[i] < 500. && X[i] > 0.)) XWA[i] = X[i] = -500.;
                  if (X[i] > 0.) Counter1[i][2]++;
               }
            }
#endif
         }
#ifdef MULTIPLEPEAK
      }
#endif
   }
   if (X[0] > 0.)
      Counter[7]++;
   if (X[1] > 0.)
      Counter[8]++;


   if (X[0] > 0. && X[1] > 0.) {
      Present = true;
      for (i = 0; i < 2; i++) {
         X[i] -= (Float_t) XRef[i];
         XS[i] -= (Float_t) XRef[i];
         XWA[i] -= (Float_t) XRef[i];
      }
      Counter[9]++;
   } else {
      for (i = 0; i < 2; i++)
         X[i] = -500.0;
   }
}

void SeDv::Treat(void)
{
#ifdef DEBUG
   cout << "SeDv::Treat" << endl;
#endif

   if (!Ready) return;
   Counter[0]++;
   Init();
   Calibrate();
   if (PresentStrips) { //PresentWires by definition

      FocalSubseqX();
   }

#ifdef DEBUG
   Show_Raw();
   Show();
#endif

}

void SeDv::inAttach(TTree* inT)
{
   char strnam[20];
   Int_t i;

#ifdef DEBUG
   cout << "SeDv::inAttach " << endl;
#endif

#ifdef DEBUG
   cout << "Attaching Sed variables" << endl;
#endif
#ifdef ACTIVEBRANCHES
   cout << "Activating Branches: SeDv" << endl;
   L->Log << "Activating Branches: SeDv" << endl;
   for (i = 0; i < 3; i++) {
      sprintf(strnam, "QSED%1d_%1d", lnr, i + 1);
      inT->SetBranchStatus(strnam, 1);
   }
   sprintf(strnam, "TSED%1d_HF", lnr);
   inT->SetBranchStatus(strnam, 1);

   sprintf(strnam, "SEDMX%1d", lnr);
   inT->SetBranchStatus(strnam, 1);
   sprintf(strnam, "SEDMY%1d", lnr);
   inT->SetBranchStatus(strnam, 1);
   sprintf(strnam, "SEDX%1d", lnr);
   inT->SetBranchStatus(strnam, 1);
   sprintf(strnam, "SEDY%1d", lnr);
   inT->SetBranchStatus(strnam, 1);
   sprintf(strnam, "SEDNrX%1d", lnr);
   inT->SetBranchStatus(strnam, 1);
   sprintf(strnam, "SEDNrY%1d", lnr);
   inT->SetBranchStatus(strnam, 1);


#endif

   for (i = 0; i < 3; i++) {
      sprintf(strnam, "QSED%1d_%1d", lnr, i + 1);
      inT->SetBranchAddress(strnam, &E_Raw[i]);
   }
   sprintf(strnam, "TSED%1d_HF", lnr);
   inT->SetBranchAddress(strnam, &T_Raw[0]);

   sprintf(strnam, "SEDMX%1d", lnr);
   inT->SetBranchAddress(strnam, Q_RawM + 0);
   sprintf(strnam, "SEDMY%1d", lnr);
   inT->SetBranchAddress(strnam, Q_RawM + 1);
   sprintf(strnam, "SEDX%1d", lnr);
   inT->SetBranchAddress(strnam, Q_Raw + 0 * 128);
   sprintf(strnam, "SEDY%1d", lnr);
   inT->SetBranchAddress(strnam, Q_Raw + 1 * 128);
   sprintf(strnam, "SEDNrX%1d", lnr);
   inT->SetBranchAddress(strnam, Q_Raw_Nr + 0 * 128);
   sprintf(strnam, "SEDNrY%1d", lnr);
   inT->SetBranchAddress(strnam, Q_Raw_Nr + 1 * 128);

}
void SeDv::outAttach(TTree* outT)
{

   char strnam[20];
   char strnam1[20];
   Int_t i;

#ifdef DEBUG
   cout << "SeDv::outAttach " << endl;
#endif

#ifdef DEBUG
   cout << "Attaching SeD variables" << endl;
#endif
   for (i = 0; i < 3; i++) {
      sprintf(strnam, "SeDEWire%1d_%1d", lnr, i);
      sprintf(strnam1, "SeDEWire%1d_%1d/F", lnr, i);
      outT->Branch(strnam, &E[i], strnam1);
   }
   sprintf(strnam, "SeDTWire%1d", lnr);
   sprintf(strnam1, "SeDTWire%1d/F", lnr);
   outT->Branch(strnam, &T[0], strnam1);

   sprintf(strnam, "SeDX%1d", lnr);
   sprintf(strnam1, "SeDX%1d/F", lnr);
   outT->Branch(strnam, &X[0], strnam1);

   sprintf(strnam, "SeDY%1d", lnr);
   sprintf(strnam1, "SeDY%1d/F", lnr);
   outT->Branch(strnam, &X[1], strnam1);

   sprintf(strnam, "SeDXS%1d", lnr);
   sprintf(strnam1, "SeDXS%1d/F", lnr);
   outT->Branch(strnam, &XS[0], strnam1);

   sprintf(strnam, "SeDYS%1d", lnr);
   sprintf(strnam1, "SeDYS%1d/F", lnr);
   outT->Branch(strnam, &XS[1], strnam1);

   sprintf(strnam, "SeDXWA%1d", lnr);
   sprintf(strnam1, "SeDXWA%1d/F", lnr);
   outT->Branch(strnam, &XWA[0], strnam1);

   sprintf(strnam, "SeDYWA%1d", lnr);
   sprintf(strnam1, "SeDYWA%1d/F", lnr);
   outT->Branch(strnam, &XWA[1], strnam1);


}


void SeDv::CreateHistograms(void)
{

#ifdef DEBUG
   cout << "SeDv::CreateHistograms " << endl;
#endif
}
void SeDv::FillHistograms(void)
{
#ifdef DEBUG
   cout << "SeDv::FillHistograms " << endl;
#endif

}


void SeDv::Show_Raw(void)
{
   Int_t i, j;
#ifdef DEBUG
   cout << "SeDv::Show_Raw" << endl;
#endif
   cout.setf(ios::showpoint);

   cout << "E1_Raw: " << E_Raw[0] << " E2_Raw: " << E_Raw[1] << " E3_Raw: " << E_Raw[2] << endl;
   cout << "T1_Raw: " << T_Raw[0] << endl;

   for (i = 0; i < 2; i++) //loop over Chambers
      for (j = 0; j < Q_RawM[i]; j++) //loop over strips
         cout << "STR: " << Q_Raw_Nr[j + i * 128] << " SED: " << i << " Q_Raw: " <<  Q_Raw[j + i * 128] << endl;
}
void SeDv::Show(void)
{
   Int_t i, j;
#ifdef DEBUG
   cout << "SeDv::Show" << endl;
#endif
   cout.setf(ios::showpoint);

   cout << "PresentWires: " << PresentWires << endl;
   if (PresentWires) {
      cout << "E1: " << E[0] << " E2: " << E[1] << " E3: " << E[2] << endl;
      cout << "T1: " << T[0] << endl;
   }
   cout << "PresentStrips: " << PresentStrips << endl;
   if (PresentStrips) {
      for (i = 0; i < 2; i++) { //loop over Chambers
         cout << " Mult " << i << ": " << Mult[i] << endl;
         for (j = 0; j < Mult[i]; j++) //loop over strips touched
            cout << "STR: " << N[j][i] << " SED: " << i << " Q: " <<  Q[j][i] << endl;
      }
   }
   cout << "Xi: ";
   for (i = 0; i < 2; i++) //loop over Chambers
      cout << X[i] << " ";
   cout << endl;
   cout << "Present: " << Present << endl;
}
