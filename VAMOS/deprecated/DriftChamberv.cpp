#include "DriftChamberv.h"

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

ClassImp(DriftChamberv)

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>DriftChamberv</h2>
<p>
Part of the VAMOS analysis package kindly contributed by Maurycy Rejmund (GANIL).
</p>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

DriftChamberv::DriftChamberv(LogFile* Log, Sive503* SiD):
   L(Log),
   Si(SiD),
   Rnd(NULL),
   Dr_ST_Q_11(NULL),
   Dr_ST_Q_12(NULL),
   Dr_ST_Q_21(NULL),
   Dr_ST_Q_22(NULL)

{
#ifdef DEBUG
   cout << "DriftChamberv::Constructor" << endl;
#endif
   Ready = kFALSE;
   char line[255];
   int len = 255;
   int i, j, k;

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

   Int_t  num;
   Float_t ref;
   Float_t propre;
   TString sline;

   ifstream in2;

   if (!gDataSet->OpenDataSetFile("DriftChamber_time.cal", in2)) {
      cout << "Could not open the calibration file Offset.cal !!!" << endl;
      return;
   } else {
      cout << "Reading DriftChamber_time.cal" << endl;
      L->Log << "Reading DriftChamber_time.cal" << endl;
      while (!in2.eof()) {
         sline.ReadLine(in2);
         if (!in2.eof()) {
            if (!sline.BeginsWith("+") && !sline.BeginsWith("|")) {
               sscanf(sline.Data(), "%d %f %f", &num , &ref, &propre);
               //L->Log << "SI_" << num << ": Ref : "<< ref << endl;
               T_DCRef[num] = ref;
               T_DCpropre_el[num] = propre;
            }
         }
      }
   }
   in2.close();

   ifstream inf1;
   if (!gDataSet->OpenDataSetFile("DriftChamberRef.cal", inf1)) {
      cout << "Could not open the Reference file DriftChamberRef.cal !" << endl;
      return;
   } else {
      cout.setf(ios::showpoint);
      cout << "Reading DriftChamberRef.cal" << endl;
      L->Log << "Reading DriftChamberRef.cal" << endl;
      inf1.getline(line, len);
      cout << line << endl;
      L->Log << line << endl;
      for (i = 0; i < 4; i++)
         for (j = 0; j < 2; j++)
            inf1 >> XRef[i][j];

      inf1.getline(line, len);
      inf1.getline(line, len);
      cout << line << endl;
      L->Log << line << endl;
      for (i = 0; i < 2; i++)
         for (j = 0; j < 2; j++)
            inf1 >> YRef[i][j];

      inf1.getline(line, len);
      inf1.getline(line, len);
      cout << line << endl;
      L->Log << line << endl;
      inf1 >> FocalPos;

      inf1.getline(line, len);
      inf1.getline(line, len);
      cout << line << endl;
      L->Log << line << endl;
      inf1 >> DriftVelocity;
      cout << DriftVelocity << endl;
      L->Log << DriftVelocity << endl;

      inf1.getline(line, len);
      inf1.getline(line, len);
      cout << line << endl;
      L->Log << line << endl;
      inf1 >> QThresh;
      cout << QThresh << endl;
      L->Log << QThresh << endl;

      inf1.getline(line, len);
      inf1.getline(line, len);
      cout << line << endl;
      L->Log << line << endl;
      for (i = 0; i < 2; i++)
         inf1 >> AngleFocal[i];
      for (i = 0; i < 2; i++) {
         cout << AngleFocal[i] << " " ;
         L->Log << AngleFocal[i] << " " ;
      }
      cout << endl;
      L->Log << endl;

      inf1.getline(line, len);
      inf1.getline(line, len);
      cout << line << endl;
      L->Log << line << endl;
      for (i = 0; i < 2; i++)
         inf1 >> AngleFocal1[i];
      for (i = 0; i < 1; i++) {
         cout << AngleFocal1[i] << " " ;
         L->Log << AngleFocal1[i] << " " ;
      }
      cout << endl;
      L->Log << endl;
   }
   inf1.close();

   SetMatX();
   SetMatY();


   ifstream inf;

   if (!gDataSet->OpenDataSetFile("DriftChamber.cal", inf)) {
      cout << "Could not open the calibration file DriftChamber.cal !" << endl;
      return;
   } else {
      cout.setf(ios::showpoint);
      cout << "Reading DriftChamber.cal" << endl;
      L->Log << "Reading DriftChamber.cal" << endl;


      inf.getline(line, len);
      cout << line << endl;
      L->Log << line << endl;
      for (i = 0; i < 2; i++)
         for (j = 0; j < 2; j++)
            inf >> ECoef[i][j];

      inf.getline(line, len);
      inf.getline(line, len);
      cout << line << endl;
      L->Log << line << endl;
      for (i = 0; i < 2; i++)
         for (j = 0; j < 2; j++)
            inf >> TCoef[i][j];

      inf.getline(line, len);
      inf.getline(line, len);
      cout << line << endl;
      L->Log << line << endl;

      for (k = 0; k < 4; k++)
         for (i = 0; i < 64; i++)
            for (j = 0; j < 3; j++)
               if ((k - 1) % 2 != 0)
                  inf >> QCoef[i][k][j];
               else
                  inf >> QCoef[63 - i][k][j];

   }
   inf.close();
   Ready = kTRUE;
}
DriftChamberv::~DriftChamberv(void)
{
#ifdef DEBUG
   cout << "DriftChamberv::Destructor" << endl;
#endif

   PrintCounters();
}

void DriftChamberv::PrintCounters(void)
{
   Int_t i;
#ifdef DEBUG
   cout << "DriftChamberv::PrintCounters" << endl;
#endif


   cout << endl;
   cout << "DriftChamberv::PrintCounters" << endl;
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
   for (i = 0; i < 4; i++)
      cout << Counter1[i][1] << " ";
   cout << endl;
   cout << "X: Weighted Average" << endl;
   for (i = 0; i < 4; i++)
      cout << Counter1[i][2] << " ";
   cout << endl;
   cout << "X: MultiplePeak" << endl;
   for (i = 0; i < 4; i++)
      cout << Counter1[i][3] << " ";
   cout << endl;
   cout << "PresentSubseqX: " << Counter[7] << endl;
   cout << "Y: " << endl;
   for (i = 0; i < 2; i++)
      cout << Counter1[i][4] << " ";
   cout << endl;
   cout << "PresentSubseqY: " << Counter[8] << endl;
   cout << "Xf: " << Counter[9] << " Tf: " << Counter[10] <<
        " Yf: " << Counter[11] << " Pf: " << Counter[12] << endl;
   cout << "Present: " << Counter[12] << endl;


   L->Log << endl;
   L->Log << "DriftChamberv::PrintCounters" << endl;
   L->Log << "Called: " << Counter[0] << endl;
   L->Log << "E1: " << Counter[1] << endl;
   L->Log << "E2: " << Counter[2] << endl;
   L->Log << "T1: " << Counter[3] << endl;
   L->Log << "T2: " << Counter[4] << endl;
   L->Log << "PresentWires: " << Counter[5] << endl;
   L->Log << "STRIP Mult >= " << NStrips << endl;
   for (i = 0; i < 4; i++)
      L->Log << Counter1[i][0] << " ";
   L->Log << endl;
   L->Log << "PresentStrips: " << Counter[6] << endl;
   L->Log << "X: Sequence Hiperbolique" << endl;
   for (i = 0; i < 4; i++)
      L->Log << Counter1[i][1] << " ";
   L->Log << endl;
   L->Log << "X: Weighted Average" << endl;
   for (i = 0; i < 4; i++)
      L->Log << Counter1[i][2] << " ";
   L->Log << endl;
   L->Log << "X: MultiplePeak" << endl;
   for (i = 0; i < 4; i++)
      L->Log << Counter1[i][3] << " ";
   L->Log << endl;
   L->Log << "PresentSubseqX: " << Counter[7] << endl;
   L->Log << "Y: " << endl;
   for (i = 0; i < 2; i++)
      L->Log << Counter1[i][4] << " ";
   L->Log << endl;
   L->Log << "PresentSubseqY: " << Counter[8] << endl;
   L->Log << "Xf: " << Counter[10] << " Tf: " << Counter[9] <<
          " Yf: " << Counter[12] << " Pf: " << Counter[11] << endl;
   L->Log << "Present: " << Counter[13] << endl;

}

void DriftChamberv::SetMatX(void)
{
   Int_t i, j;
   Double_t A[2][2];
   Double_t Det;
#ifdef DEBUG
   cout << "DriftChamberv::SetMatX" << endl;
#endif

   if (fabs(AngleFocal[0]) > 0.000001)
      TanFocal[0] = 1.0 / tan(3.1415922654 / 180.*AngleFocal[0]);
   else
      TanFocal[0] = 1.0e08;

   if (fabs(AngleFocal1[0]) > 0.000001)
      TanFocal1[0] = 1.0 / tan(3.1415922654 / 180.*AngleFocal1[0]);
   else
      TanFocal1[0] = 1.0e08;


   for (i = 0; i < 2; i++)
      for (j = 0; j < 2; j++)
         A[i][j] = MatX[i][j] = 0.;

   for (i = 0; i < 4; i++) {
      A[0][0] += pow(XRef[i][1], 2.);
      A[0][1] += XRef[i][1];
      A[1][1] += 1.0;
   }
   A[1][0] = A[0][1];



   Det = A[0][0] * A[1][1] - A[0][1] * A[1][0];
   L->Log << "Déterminant SetMatX" << Det << endl;
   if (Det == 0.0) {
      cout << "DriftChamberv::SetMatX: Det == 0 !" << endl;
      exit(EXIT_FAILURE);
   } else {
      MatX[0][0] = A[1][1] / Det;
      MatX[1][1] = A[0][0] / Det;
      MatX[1][0] = -1.0 * A[0][1] / Det;
      MatX[0][1] = -1.0 * A[1][0] / Det;
   }

   L->Log << "Det = " << Det << " " << "MatX : " << MatX[0][0] << " " << MatX[1][1] << " " << MatX[1][0] << " " << MatX[0][1] << endl;

}

void DriftChamberv::SetMatY(void)
{
   Int_t i, j;
   Float_t A[2][2];
   Float_t Det;
#ifdef DEBUG
   cout << "DriftChamberv::SetMatY" << endl;
#endif

   if (fabs(AngleFocal[1]) > 0.000001)
      TanFocal[1] = 1.0 / tan(3.1415922654 / 180.*AngleFocal[1]);
   else
      TanFocal[1] = 1.0e08;

   for (i = 0; i < 2; i++)
      for (j = 0; j < 2; j++)
         A[i][j] = MatY[i][j] = 0.;

   for (i = 0; i < 2; i++) {
      A[0][0] += pow(YRef[i][1], 2.);
      A[0][1] += YRef[i][1];
      A[1][1] += 1.0;
   }
   A[1][0] = A[0][1];

   Det = A[0][0] * A[1][1] - A[0][1] * A[1][0];

   if (Det == 0.0) {
      cout << "DriftChamberv::SetMatY: Det == 0 !" << endl;
      exit(EXIT_FAILURE);
   } else {
      MatY[0][0] = A[1][1] / Det;
      MatY[1][1] = A[0][0] / Det;
      MatY[0][1] = -1.0 * A[1][0] / Det;
      MatY[1][0] = -1.0 * A[0][1] / Det;
   }

   L->Log << "Det = " << Det << " " << "MatY : " << MatY[0][0] << " " << MatY[1][1] << " " << MatY[1][0] << " " << MatY[0][1] << endl;
}


void DriftChamberv::Zero(void)
{
#ifdef DEBUG
   cout << "DriftChamberv::Zero" << endl;
#endif
   Present = PresentWires = PresentStrips = false;
   PresentSubseqX = PresentSubseqY = false;


   E[0] = E[1] = T[0] = T[1] = 0.0;
   Mult[0] = Mult[1] = Mult[2] = Mult[3] = 0;
   for (Int_t j = 0; j < 4; j++)
      for (Int_t i = 0; i < 64; i++) {
         Q[i][j] = 0.0;
         N[i][j] = 0 ;
      }
   X[0] = X[1] = X[2] = X[3] = -500.0;
   XS[0] = XS[1] = XS[2] = XS[3] = -500.0;
   XWA[0] = XWA[1] = XWA[2] = XWA[3] = -500.0;
   Y[0] = Y[1] = -500.0;
   Xf = Xf1 = Yf = Tf = Pf = -500.0;
}

void DriftChamberv::InitRaw(void)
{
#ifdef DEBUG
   cout << "DriftChamberv::InitRaw" << endl;
#endif
   for (Int_t i = 0; i < 2; i++) {
      E_Raw[i] = 0;
      T_Raw[i] = 0;
   }          // = 0 !!!!
   for (Int_t i = 0; i < 64; i++) {
      for (Int_t j = 0; j < 4; j++) {
         Q_Raw[j * 64 + i] = 0;
         Q_Raw_Nr[j * 64 + i] = 0;
      }
   }
   for (Int_t j = 0; j < 4; j++) {
      Q_RawM[j] = 0;
   }
}

void DriftChamberv::Init(void)
{
#ifdef DEBUG
   cout << "DriftChamberv::Init" << endl;
#endif
   Present = PresentWires = PresentStrips = false;
   PresentSubseqX = PresentSubseqY = false;

   E[0] = E[1] = T[0] = T[1] = 0.0;
   Mult[0] = Mult[1] = Mult[2] = Mult[3] = 0;
   X[0] = X[1] = X[2] = X[3] = -500.0;
   XS[0] = XS[1] = XS[2] = XS[3] = -500.0;
   XWA[0] = XWA[1] = XWA[2] = XWA[3] = -500.0;
   Y[0] = Y[1] = -500.0;
   Xf = Xf1 = Yf = Tf = Pf = -500.0;
}

void DriftChamberv::Calibrate(void)
{
#ifdef DEBUG
   cout << "DriftChamberv::Calibrate" << endl;
#endif

   Int_t i, j, k;
   Float_t QTmp;
//L->Log<<"E_raw : "<<E_Raw[0]<<" "<<E_Raw[1]<<endl;
//L->Log<<"T_raw : "<<T_Raw[0]<<" "<<T_Raw[1]<<endl;
   for (i = 0; i < 2; i++)
      if (E_Raw[i] > 0) {
         //L->Log<<"E_raw : "<<E_Raw[i]<<endl;
         Rnd->Next();
         for (j = 0; j < 2; j++)
            E[i] += powf((Float_t)E_Raw[i] + Rnd->Value(),
                         (Float_t) j) * ECoef[i][j];
      }
   for (i = 0; i < 2; i++)
      if (T_Raw[i] > 0) {
         //L->Log<<"T_raw : "<<T_Raw[i]<<endl;
         Rnd->Next();
         for (j = 0; j < 2; j++)
            T[i] += powf((Float_t) T_Raw[i] + Rnd->Value(),
                         (Float_t) j) * TCoef[i][j];
         T_DCfrag[i] = T_DCRef[i] + T_DCpropre_el[i] - T[i];
         if (Si->Present) {
            //       T[i] += Si->Offset[1] - Si->T[1];
         }
      }

   //L->Log << "T_DCRef[0] : " << T_DCRef[0] << " T_DCpropre_el[0] : "<<T_DCpropre_el[0]<<" T[0] : "<<T[0]<<" T_DCfrag[0] = "<<T_DCfrag[0]<<endl;
   //L->Log << "T_DCRef[1] : " << T_DCRef[1] << " T_DCpropre_el[1] : "<<T_DCpropre_el[1]<<" T[1] : "<<T[1]<<" T_DCfrag[1] = "<<T_DCfrag[1]<<endl;

   if (E[0] > 0.0) Counter[1]++;
   if (E[1] > 0.0) Counter[2]++;
   if (T[0] > 0.0) Counter[3]++;
   if (T[1] > 0.0) Counter[4]++;
   //L->Log<<"E[0] : "<<E[0]<<" "<<"E[1] : "<<E[1]<<" "<<"T[0] : "<<T[0]<<" "<<"T[1] : "<<T[1]<<endl;

   if (T[0] > 0.0 && T[1] > 0.0) { //E[0] > 0.0 && E[1] > 0.0 && T[0] > 0.0 && T[1] > 0.0
      PresentWires = true;
      Counter[5]++;
   } else {
      E[0] = E[1] = T[0] = T[1] = 0.0;
   }
   //L->Log<<"PresentWires : ::Calibrate() : "<<PresentWires<<endl;
   //L->Log<<"Q_RawM[0] : "<<Q_RawM[0]<<" "<<"Q_RawM[1] : "<<Q_RawM[1]<<" "<<"Q_RawM[2] : "<<Q_RawM[2]<<" "<<"Q_RawM[3] : "<<Q_RawM[3]<<endl;

   if (PresentWires) {
      for (i = 0; i < 4; i++) //loop over Chambers
         if ((i - 1) % 2 != 0) {
            for (j = 0; j < Q_RawM[i]; j++) { //loop over strips
               QTmp = 0.0;
               Rnd->Next();
               for (k = 0; k < 3; k++) //loop over coeff
                  QTmp += powf((Float_t) Q_Raw[j + i * 64] + Rnd->Value(),
                               (Float_t)k) * QCoef[Q_Raw_Nr[j + i * 64]][i][k];
               if (QTmp > QThresh) {
                  Mult[i]++;
                  Q[Mult[i] - 1][i] = QTmp;
                  N[Mult[i] - 1][i] = Q_Raw_Nr[j + i * 64];
               }
            }
         } else {
            for (j = 0; j < Q_RawM[i]; j++) { //loop over strips
               QTmp = 0.0;
               Rnd->Next();
               for (k = 0; k < 3; k++) //loop over coeff
                  QTmp += powf((Float_t)Q_Raw[j + i * 64] + Rnd->Value(),
                               (Float_t)k) * QCoef[63 - Q_Raw_Nr[j + i * 64]][i][k];
               if (QTmp > QThresh) {
                  Mult[i]++;
                  Q[Mult[i] - 1][i] = QTmp;
                  N[Mult[i] - 1][i] = 63 - Q_Raw_Nr[j + i * 64];
               }
            }
         }
      //L->Log<<"QTmp : "<<QTmp<<endl;
      for (i = 0; i < 4; i++)
         if (Mult[i] >= NStrips) Counter1[i][0]++;

      if (Mult[0] >= NStrips && Mult[1] >= NStrips && Mult[2] >= NStrips && Mult[3] >= NStrips) {
         PresentStrips = true;
         Counter[6]++;
      }
      //L->Log<<"Mult[0] : "<<Mult[0]<<" "<<"Mult[1] : "<<Mult[1]<<" "<<"Mult[2] : "<<Mult[2]<<" "<<"Mult[3] : "<<Mult[3]<<endl;
      //L->Log<<"PresentStrips : ::Calibrate() : "<<PresentStrips<<endl;
   }



}

void DriftChamberv::Focal(void)
{
#ifdef DEBUG
   cout << "DriftChamberv::Focal" << endl;
#endif
   //L->Log<<"PresentStrips ::Focal(): "<<PresentStrips<<endl;
   if (PresentStrips) { //PresentWires by definition
      FocalSubseqX();
      FocalSubseqY();
   }
   //L->Log<<"PresentSubseqX && PresentSubseqY ::Focal() : "<<PresentSubseqX<<" "<<PresentSubseqY<<endl;
   if (PresentSubseqX && PresentSubseqY) { //PresentWires by definition
      FocalX();
      FocalY();
      if (Xf > -500. && Tf > -500. &&
            Yf > -500. && Pf > -500.) {
         Present = true;
         Counter[13]++;
      } else
         Xf = Xf1 = Yf = Tf = Pf = -500.0;
   }


}
void DriftChamberv::FocalSubseqX(void)
{

   Int_t i, j, k;
   Float_t QTmp[64];
   Float_t QMax;
   Int_t NMax;
   Int_t FStrip[64];
   Int_t StripsWA;
   bool Neighbours;
   //bool MultiplePeak;
   Float_t v[6];


#ifdef DEBUG
   cout << "DriftChamberv::FocalSubseqX" << endl;
#endif
   for (i = 0; i < 4; i++) { //Loop over Chambers

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
      MultiplePeak = false;
      for (j = 0; j < FStrip[0] - 1; j++)
         if ((Q[j][i] > Q[j + 1][i]))
            if (Q[j][i] >= 0.4 * Q[FStrip[0]][i]) {
               MultiplePeak = true;
               Counter1[i][3]++;
#ifdef DEBUG
               cout << "Multiplepeak: " << i << endl;
#endif
               break;
            }
      for (j = FStrip[0]; j < Mult[i] - 1; j++)
         if (Q[j][i] < Q[j + 1][i])
            if (Q[j + 1][i] >= 0.4 * Q[FStrip[0]][i]) {
               MultiplePeak = true;
               Counter1[i][3]++;
#ifdef DEBUG
               cout << "Multiplepeak: " << i << endl;
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
         if (Neighbours) { //Because SECHIP is not defined here, Neighbours is always false...
            v[0] = sqrtf(Q[FStrip[0]][i] / Q[FStrip[2]][i]);
            v[1] = sqrtf(Q[FStrip[0]][i] / Q[FStrip[1]][i]);
            v[2] = 0.5 * (v[0] + v[1]);
            v[3] = logf(v[2] + sqrtf(powf(v[2], 2.) - 1.0));
            v[4] = (v[0] - v[1]) / (2.0 * sinhf(v[3]));
            v[5] = 0.5 * logf((1.0 + v[4]) / (1.0 - v[4]));
            X[i] = (Float_t) N[FStrip[0]][i] -
                   (Float_t)(N[FStrip[0]][i] - N[FStrip[1]][i]) *
                   v[5] / v[3];
            if ((i - 1) % 2 != 0)
               X[i] -= 0.5;
            XS[i] = X[i] *= 6.3; //Goes mm
            if (!(X[i] < 500. && X[i] > 0.)) XS[i] = X[i] = -500.;
            if (X[i] > 0.) Counter1[i][1]++;
         } else { // the case of Weighted Average
#ifdef WEIGHTEDAVERAGE
            if (Mult[i] > NStrips) {
               //Looking for entire peak for W.A.
               //The Strips are ordered  0-64
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
                  cout << "DriftChamberv::FocalSubseqX::Weithed Average: " << i << endl;
                  cout << "StripsWA: " << StripsWA << endl;
                  for (k = 0; k <= StripsWA; k++)
                     cout << "STR: " << N[FStrip[k]][i] << " Q: " << Q[FStrip[k]][i] << endl;
#endif
                  X[i] = v[0] / v[1];
                  if ((i - 1) % 2 != 0)
                     X[i] -= 0.5;

                  XWA[i] = X[i] *= 6.3; //Goes mm
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
   //L->Log<<"X[0] : "<<X[0]<<" "<<"X[1] : "<<X[1]<<" "<<"X[2] : "<<X[2]<<" "<<"X[3] : "<<X[3]<<endl;

   if (X[0] > 0. && X[1] > 0. && X[2] > 0. && X[3] > 0.) {
      PresentSubseqX = true;
      for (i = 0; i < 4; i++) {
         X[i] -= (Float_t) XRef[i][0];
         XS[i] -= (Float_t) XRef[i][0];
         XWA[i] -= (Float_t) XRef[i][0];
      }
      Counter[7]++;
   } else {
      for (i = 0; i < 4; i++)
         X[i] = -500.0;
   }
}

void DriftChamberv::FocalSubseqY(void)
{
   Int_t i;

#ifdef DEBUG
   cout << "DriftChamberv::FocalSubseqY" << endl;
#endif

   //DriftVelocity is in cm/us
   for (i = 0; i < 2; i++) {
      Y[i] = T[i] * DriftVelocity * 10. / 1000.;
   }
   for (i = 0; i < 2; i++)
      if (Y[i] > 0.) Counter1[i][4]++;

   if (Y[0] > 0. && Y[1] > 0.) {
      PresentSubseqY = true;
      Counter[8]++;
      for (i = 0; i < 2; i++) {
         Y[i] -= (Float_t) YRef[i][0];
         //The wires are upside
         Y[i] *= -1.0;
         //    cout << "aa " << i <<" " <<  Y[i] << " " << T[i] <<  " "<<DriftVelocity << " " << YRef[i][0] << endl;
      }
   } else {
      for (i = 0; i < 2; i++)
         Y[i] = -500.0;
   }
}

void DriftChamberv::FocalX(void)
{
   Int_t i;
   Double_t A[2];
   Double_t B[2];

#ifdef DEBUG
   cout << "DriftChamberv::FocalX" << endl;
#endif
   for (i = 0; i < 2; i++)
      A[i] = B[i] = 0.0;


   for (i = 0; i < 4; i++) {
      A[0] += ((Double_t) X[i]) * XRef[i][1];
      A[1] += (Double_t) X[i];
   }
   B[0] = A[0] * MatX[0][0] + A[1] * MatX[0][1];
   B[1] = A[0] * MatX[1][0] + A[1] * MatX[1][1];

   // Tf in mrad
   Tf = (Float_t)(1000.*atan(B[0]));
   Counter[9]++;
   //L->Log<<"TanFocal[0] : "<<TanFocal[0]<<" "<<"B[0] : "<<B[0]<<endl;
   if ((TanFocal[0] - B[0]) != 0.) {
      Xf = (Float_t)(B[0] * (TanFocal[0] * FocalPos + B[1]) / (TanFocal[0] - B[0]) + B[1]);
      Counter[10]++;
   } else
      Xf = -500.0;

   if ((TanFocal1[0] - B[0]) != 0.) {
      Xf1 = (Float_t)(B[0] * (TanFocal1[0] * FocalPos + B[1]) / (TanFocal1[0] - B[0]) + B[1]);
   } else
      Xf1 = -500.0;


}
void DriftChamberv::FocalY(void)
{
   Int_t i;
   Float_t A[2];
   Float_t B[2];

#ifdef DEBUG
   cout << "DriftChamberv::FocalY" << endl;
#endif

   for (i = 0; i < 2; i++)
      A[i] = B[i] = 0.0;


   for (i = 0; i < 2; i++) {
      A[0] += ((Double_t) Y[i]) * YRef[i][1];
      A[1] += Y[i];
   }
   B[0] = A[0] * MatY[0][0] + A[1] * MatY[0][1];
   B[1] = A[0] * MatY[1][0] + A[1] * MatY[1][1];

   // Pf in mrad
   Pf = (Float_t)(1000.*atan(B[0]));
   Counter[11]++;
   //L->Log<<"TanFocal[1] : "<<TanFocal[1]<<" "<<"B[0] : "<<B[0]<<endl;
   if ((TanFocal[1] - B[0]) != 0.) {
      Yf = (Float_t)(B[0] * (TanFocal[1] * FocalPos + B[1]) / (TanFocal[1] - B[0]) + B[1]);
      //Accounting for the width of the Chamber
      //      cout << Yf << " " << Pf << " ";
      Yf -= fabs(tan(Pf / 1000.)) * 50.;
      //      cout << Yf << endl;
      Counter[12]++;
   } else
      Yf = -500.0;
}


void DriftChamberv::Treat(void)
{
#ifdef DEBUG
   cout << "DriftChamberv::Treat" << endl;
#endif

   if (!Ready) return;

   Counter[0]++;
   Init();
   Calibrate();
   Focal();
   //  for(Int_t i=0;i<2;i++)
   //    cout << i << " " << Y[i] << endl;
#ifdef DEBUG
   Show_Raw();
   Show();
#endif

}

void DriftChamberv::inAttach(TTree* inT)
{

#ifdef DEBUG
   cout << "DriftChamberv::inAttach " << endl;
#endif

#ifdef DEBUG
   cout << "Attaching Drift variables" << endl;
#endif
#ifdef ACTIVEBRANCHES
   cout << "Activating Branches: DriftChamberv" << endl;
   L->Log << "Activating Branches: DriftChamberv" << endl;
   inT->SetBranchStatus("EFIL_1", 1);
   inT->SetBranchStatus("EFIL_2", 1);
   inT->SetBranchStatus("TFIL_1", 1);
   inT->SetBranchStatus("TFIL_2", 1);

   inT->SetBranchStatus("STRM1", 1);
   inT->SetBranchStatus("STRM2", 1);
   inT->SetBranchStatus("STRM3", 1);
   inT->SetBranchStatus("STRM4", 1);
   inT->SetBranchStatus("STR1", 1);
   inT->SetBranchStatus("STR2", 1);
   inT->SetBranchStatus("STR3", 1);
   inT->SetBranchStatus("STR4", 1);
   inT->SetBranchStatus("STRNr1", 1);
   inT->SetBranchStatus("STRNr2", 1);
   inT->SetBranchStatus("STRNr3", 1);
   inT->SetBranchStatus("STRNr4", 1);

#endif

   inT->SetBranchAddress("EFIL_1", &E_Raw[0]);
   inT->SetBranchAddress("EFIL_2", &E_Raw[1]);
   inT->SetBranchAddress("TFIL_1", &T_Raw[0]);
   inT->SetBranchAddress("TFIL_2", &T_Raw[1]);

   inT->SetBranchAddress("STRM1", Q_RawM + 0);
   inT->SetBranchAddress("STRM2", Q_RawM + 1);
   inT->SetBranchAddress("STRM3", Q_RawM + 2);
   inT->SetBranchAddress("STRM4", Q_RawM + 3);
   inT->SetBranchAddress("STR1", Q_Raw + 0 * 64);
   inT->SetBranchAddress("STR2", Q_Raw + 1 * 64);
   inT->SetBranchAddress("STR3", Q_Raw + 2 * 64);
   inT->SetBranchAddress("STR4", Q_Raw + 3 * 64);
   inT->SetBranchAddress("STRNr1", Q_Raw_Nr + 0 * 64);
   inT->SetBranchAddress("STRNr2", Q_Raw_Nr + 1 * 64);
   inT->SetBranchAddress("STRNr3", Q_Raw_Nr + 2 * 64);
   inT->SetBranchAddress("STRNr4", Q_Raw_Nr + 3 * 64);

}
void DriftChamberv::outAttach(TTree* outT)
{

#ifdef DEBUG
   cout << "DriftChamberv::outAttach " << endl;
#endif

#ifdef DEBUG
   cout << "Attaching Drift variables" << endl;
#endif

//==============================================
// Test des charges des DC
   outT->Branch("STRM1", Q_RawM + 0, "Q_RawM+0/I");
   outT->Branch("STRM2", Q_RawM + 1, "Q_RawM+0/I");
   outT->Branch("STRM3", Q_RawM + 2, "Q_RawM+0/I");
   outT->Branch("STRM4", Q_RawM + 3, "Q_RawM+0/I");

   outT->Branch("STR1", Q_Raw + 0 * 64, "Q_Raw+0*64/S");
   outT->Branch("STR2", Q_Raw + 1 * 64, "Q_Raw+1*64/S");
   outT->Branch("STR3", Q_Raw + 2 * 64, "Q_Raw+2*64/S");
   outT->Branch("STR4", Q_Raw + 3 * 64, "Q_Raw+3*64/S");
//==============================================
   outT->Branch("DcEWire1", &E[0], "DcEWire1/F");
   outT->Branch("DcEWire2", &E[1], "DcEWire2/F");
   outT->Branch("DcTWire1", &T_DCfrag[0], "DcTWire1/F");
   outT->Branch("DcTWire2", &T_DCfrag[1], "DcTWire2/F");

   outT->Branch("DcX1", &X[0], "DcX1/F");
   outT->Branch("DcX2", &X[1], "DcX2/F");
   outT->Branch("DcX3", &X[2], "DcX3/F");
   outT->Branch("DcX4", &X[3], "DcX4/F");

   //  outT->Branch("DcXS1",&XS[0],"DcXS1/F");
   //  outT->Branch("DcXS2",&XS[1],"DcXS2/F");
   //  outT->Branch("DcXS3",&XS[2],"DcXS3/F");
   //  outT->Branch("DcXS4",&XS[3],"DcXS4/F");

   //  outT->Branch("DcXWA1",&XWA[0],"DcXWA1/F");
   //  outT->Branch("DcXWA2",&XWA[1],"DcXWA2/F");
   //  outT->Branch("DcXWA3",&XWA[2],"DcXWA3/F");
   //  outT->Branch("DcXWA4",&XWA[3],"DcXWA4/F");

   outT->Branch("DcY1", &Y[0], "DcY1/F");
   outT->Branch("DcY2", &Y[1], "DcY2/F");

   outT->Branch("Xf", &Xf, "Xf/F");
   outT->Branch("Tf", &Tf, "Tf/F");
   outT->Branch("Yf", &Yf, "Yf/F");
   outT->Branch("Pf", &Pf, "Pf/F");

   //  outT->Branch("Xf1",&Xf1,"Xf1/F");

}


void DriftChamberv::CreateHistograms(void)
{

#ifdef DEBUG
   cout << "DriftChamberv::CreateHistograms " << endl;
#endif
   Dr_ST_Q_11 = new TH2S("Dr_ST_Q_11", "Dr_ST_Q_11", 64, 0, 64, 512, 0, 3000);
   Dr_ST_Q_12 = new TH2S("Dr_ST_Q_12", "Dr_ST_Q_12", 64, 0, 64, 512, 0, 3000);
   Dr_ST_Q_21 = new TH2S("Dr_ST_Q_21", "Dr_ST_Q_21", 64, 0, 64, 512, 0, 3000);
   Dr_ST_Q_22 = new TH2S("Dr_ST_Q_22", "Dr_ST_Q_22", 64, 0, 64, 512, 0, 3000);
}
void DriftChamberv::FillHistograms(void)
{
   Int_t i;
#ifdef DEBUG
   cout << "DriftChamberv::FillHistograms " << endl;
#endif
   if (PresentStrips) {
      for (i = 0; i < Mult[0]; i++)
         Dr_ST_Q_11->Fill(N[i][0], (UShort_t) Q[i][0]);
      for (i = 0; i < Mult[1]; i++)
         Dr_ST_Q_12->Fill(N[i][1], (UShort_t) Q[i][1]);
      for (i = 0; i < Mult[2]; i++)
         Dr_ST_Q_21->Fill(N[i][2], (UShort_t) Q[i][2]);
      for (i = 0; i < Mult[3]; i++)
         Dr_ST_Q_22->Fill(N[i][3], (UShort_t) Q[i][3]);
   }


}


void DriftChamberv::Show_Raw(void)
{
   Int_t i, j;
#ifdef DEBUG
   cout << "DriftChamberv::Show_Raw" << endl;
#endif
   cout.setf(ios::showpoint);

   cout << "E1_Raw: " << E_Raw[0] << " E2_Raw: " << E_Raw[1] << endl;
   cout << "T1_Raw: " << T_Raw[0] << " T2_Raw: " << T_Raw[1] << endl;

   for (i = 0; i < 4; i++) //loop over Chambers
      for (j = 0; j < Q_RawM[i]; j++) //loop over strips
         cout << "STR: " << Q_Raw_Nr[j + i * 64] << " DR: " << i << " Q_Raw: " <<  Q_Raw[j + i * 64] << endl;
}
void DriftChamberv::Show(void)
{
   Int_t i, j;
#ifdef DEBUG
   cout << "DriftChamberv::Show" << endl;
#endif
   cout.setf(ios::showpoint);

   cout << "PresentWires: " << PresentWires << endl;
   if (PresentWires) {
      cout << "E1: " << E[0] << " E2: " << E[1] << endl;
      cout << "T1: " << T[0] << " T2: " << T[1] << endl;
   }
   cout << "PresentStrips: " << PresentStrips << endl;
   if (PresentStrips) {
      for (i = 0; i < 4; i++) { //loop over Chambers
         cout << " Mult " << i << ": " << Mult[i] << endl;
         for (j = 0; j < Mult[i]; j++) //loop over strips touched
            cout << "STR: " << N[j][i] << " DR: " << i << " Q: " <<  Q[j][i] << endl;
      }
   }
   cout << "PresentSubsequentX: " << PresentSubseqX << endl;
   cout << "Xi: ";
   for (i = 0; i < 4; i++) //loop over Chambers
      cout << X[i] << " ";
   cout << endl;
   cout << "PresentSubsequentY: " << PresentSubseqY << endl;
   cout << "Yi: ";
   for (i = 0; i < 2; i++) //loop over Chambers
      cout << Y[i] << " ";
   cout << endl;
   cout << "Present: " << Present << endl;
   if (Present)
      cout << " Xf Tf Yf Pf: " <<
           Xf << " " << Tf << " " <<
           Yf << " " << Pf << endl;
}

