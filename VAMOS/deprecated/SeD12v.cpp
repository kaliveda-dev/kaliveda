#include "SeD12v.h"

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

ClassImp(SeD12v)

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>SeD12v</h2>
<p>
Part of the VAMOS analysis package kindly contributed by Maurycy Rejmund (GANIL).
</p>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

SeD12v::SeD12v(LogFile* Log, SeDv* SeD1, SeDv* SeD2)
{
#ifdef DEBUG
   cout << "SeD12v::Constructor" << endl;
#endif

   Ready = kFALSE;

   char line[255];
   int len = 255;
   int i, j;

   L = Log;
   S1 = SeD1;
   S2 = SeD2;


   for (i = 0; i < 14; i++)
      Counter[i] = 0;



   Rnd = new Random;


   ifstream inf1;
   if (!gDataSet->OpenDataSetFile("SeD12Ref.cal", inf1)) {
      cout << "Could not open the Reference file SeD12Ref.cal !" << endl;
      return;
   } else {
      cout.setf(ios::showpoint);
      cout << "Reading SeD12Ref.cal" << endl;
      L->Log << "Reading SeD12Ref.cal" << endl;
      inf1.getline(line, len);
      cout << line << endl;
      L->Log << line << endl;
      for (i = 0; i < 2; i++) {
         inf1 >> XRef[i];
         inf1 >> YRef[i];
      }

      inf1.getline(line, len);
      inf1.getline(line, len);
      cout << line << endl;
      L->Log << line << endl;
      inf1 >> FocalPos;

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
      for (i = 0; i < 1; i++)
         for (j = 0; j < 2; j++)
            inf1 >> TCoef[i][j];
      cout << endl;
      L->Log << endl;
   }
   inf1.close();

   SetMatX();
   SetMatY();

   Ready = kTRUE;
}
SeD12v::~SeD12v(void)
{
#ifdef DEBUG
   cout << "SeD12v::Destructor" << endl;
#endif

   PrintCounters();
}

void SeD12v::PrintCounters(void)
{
#ifdef DEBUG
   cout << "SeD12v::PrintCounters" << endl;
#endif


   cout << endl;
   cout << "SeD12v::PrintCounters" << endl;
   cout << "Called: " << Counter[0] << endl;
   cout << "SeD1 Present: " << Counter[1] << endl;
   cout << "SeD2 Present: " << Counter[2] << endl;
   cout << "Sed1 and SeD2 Present: " << Counter[3] << endl;
   cout << "Tf: " << Counter[4] << " Xf: " << Counter[5] << endl;
   cout << "Pf: " << Counter[6] << " Yf: " << Counter[7] << endl;
   cout << "Present: " << Counter[8];
   L->Log << endl;
   L->Log << "SeD12v::PrintCounters" << endl;
   L->Log << "Called: " << Counter[0] << endl;
   L->Log << "SeD1 Present: " << Counter[1] << endl;
   L->Log << "SeD2 Present: " << Counter[2] << endl;
   L->Log << "Sed1 and SeD2 Present: " << Counter[3] << endl;
   L->Log << "Tf: " << Counter[4] << endl;
   L->Log << "Tf: " << Counter[4] << " Xf: " << Counter[5] << endl;
   L->Log << "Pf: " << Counter[6] << " Yf: " << Counter[7] << endl;
   L->Log << "Present: " << Counter[8];

}
void SeD12v::Calibrate(void)
{
#ifdef DEBUG
   cout << "SeD12v::Calibrate" << endl;
#endif

   Int_t i, j;

   for (i = 0; i < 1; i++)
      if (T_Raw[i] > 0) {
         Rnd->Next();
         for (j = 0; j < 2; j++)
            T[i] += powf((Float_t) T_Raw[i] + Rnd->Value(),
                         (Float_t) j) * TCoef[i][j];
      }


}
void SeD12v::SetMatX(void)
{
   Int_t i, j;
   Double_t A[2][2];
   Double_t Det;
#ifdef DEBUG
   cout << "SeD12v::SetMatX" << endl;
#endif

   if (fabs(AngleFocal[0]) > 0.000001)
      TanFocal[0] = 1.0 / tan(3.1415922654 / 180.*AngleFocal[0]);
   else
      TanFocal[0] = 1.0e08;


   for (i = 0; i < 2; i++)
      for (j = 0; j < 2; j++)
         A[i][j] = MatX[i][j] = 0.;

   for (i = 0; i < 2; i++) {
      A[0][0] += pow(XRef[i], 2.);
      A[0][1] += XRef[i];
      A[1][1] += 1.0;
   }
   A[1][0] = A[0][1];



   Det = A[0][0] * A[1][1] - A[0][1] * A[1][0];

   if (Det == 0.0) {
      cout << "SeD12v::SetMatX: Det == 0 !" << endl;
      exit(EXIT_FAILURE);
   } else {
      MatX[0][0] = A[1][1] / Det;
      MatX[1][1] = A[0][0] / Det;
      MatX[1][0] = -1.0 * A[0][1] / Det;
      MatX[0][1] = -1.0 * A[1][0] / Det;
   }

}

void SeD12v::SetMatY(void)
{
   Int_t i, j;
   Float_t A[2][2];
   Float_t Det;
#ifdef DEBUG
   cout << "SeD12v::SetMatY" << endl;
#endif

   if (fabs(AngleFocal[1]) > 0.000001)
      TanFocal[1] = 1.0 / tan(3.1415922654 / 180.*AngleFocal[1]);
   else
      TanFocal[1] = 1.0e08;

   for (i = 0; i < 2; i++)
      for (j = 0; j < 2; j++)
         A[i][j] = MatY[i][j] = 0.;

   for (i = 0; i < 2; i++) {
      A[0][0] += pow(YRef[i], 2.);
      A[0][1] += YRef[i];
      A[1][1] += 1.0;
   }
   A[1][0] = A[0][1];

   Det = A[0][0] * A[1][1] - A[0][1] * A[1][0];

   if (Det == 0.0) {
      cout << "SeD12v::SetMatY: Det == 0 !" << endl;
      exit(EXIT_FAILURE);
   } else {
      MatY[0][0] = A[1][1] / Det;
      MatY[1][1] = A[0][0] / Det;
      MatY[0][1] = -1.0 * A[1][0] / Det;
      MatY[1][0] = -1.0 * A[0][1] / Det;
   }

}


void SeD12v::Init(void)
{
#ifdef DEBUG
   cout << "SeD12v::Init" << endl;
#endif
   Present = false;

   Xf = Yf = Tf = Pf = -500.0;
   T[0] = 0.;
   V = 0;
}


void SeD12v::Focal(void)
{
#ifdef DEBUG
   cout << "SeD12v::Focal" << endl;
#endif
   if (S1->Present)
      Counter[1]++;
   if (S2->Present)
      Counter[2]++;

   if (S1->Present && S2->Present) { // have both SeDs
      Counter[3]++;
      X[0] = S1->X[0];
      Y[0] = S1->X[1];
      X[1] = S2->X[0];
      Y[1] = S2->X[1];

      //      cout << "A " << S1->X[0] + S1->XRef[0] << ' ' <<
      // S2->X[0] + S2->XRef[0] << ' ' <<
      // X[0] << ' ' << X[1] << endl;
      FocalX();
      FocalY();
      if (Xf > -500. && Tf > -500. &&
            Yf > -500. && Pf > -500.) {
         if (T[0] > 0.)
            V = 100. / (cos(Tf / 1000.) * cos(Pf / 1000.)) / T[0];

         Present = true;
         Counter[8]++;
      } else {
         Xf = Yf = Tf = Pf = -500.0;
      }
   }

}

void SeD12v::FocalX(void)
{
   Int_t i;
   Double_t A[2];
   Double_t B[2];

#ifdef DEBUG
   cout << "SeD12v::FocalX" << endl;
#endif
   for (i = 0; i < 2; i++)
      A[i] = B[i] = 0.0;


   for (i = 0; i < 2; i++) {
      A[0] += ((Double_t) X[i]) * XRef[i];
      A[1] += (Double_t) X[i];
   }
   B[0] = A[0] * MatX[0][0] + A[1] * MatX[0][1];
   B[1] = A[0] * MatX[1][0] + A[1] * MatX[1][1];

   //  cout << "B " << A[0] << ' ' <<A[1] <<' ' <<
   //    B[0] << ' ' <<B[1] << endl;

   // Tf in mrad
   Tf = (Float_t)(1000.*atan(B[0]));
   Counter[4]++;

   if ((TanFocal[0] - B[0]) != 0.) {
      Xf = (Float_t)(B[0] * (TanFocal[0] * FocalPos + B[1]) / (TanFocal[0] - B[0]) + B[1]);
      Counter[5]++;
   } else
      Xf = -500.0;

   //  cout << "C " << Tf << ' ' << Xf << endl;

}
void SeD12v::FocalY(void)
{
   Int_t i;
   Float_t A[2];
   Float_t B[2];

#ifdef DEBUG
   cout << "SeD12v::FocalY" << endl;
#endif

   for (i = 0; i < 2; i++)
      A[i] = B[i] = 0.0;


   for (i = 0; i < 2; i++) {
      A[0] += ((Double_t) Y[i]) * YRef[i];
      A[1] += Y[i];
   }
   B[0] = A[0] * MatY[0][0] + A[1] * MatY[0][1];
   B[1] = A[0] * MatY[1][0] + A[1] * MatY[1][1];

   // Pf in mrad
   Pf = (Float_t)(1000.*atan(B[0]));
   Counter[6]++;

   if ((TanFocal[1] - B[0]) != 0.) {
      Yf = (Float_t)(B[0] * (TanFocal[1] * FocalPos + B[1]) / (TanFocal[1] - B[0]) + B[1]);
      Counter[7]++;
   } else
      Yf = -500.0;
}


void SeD12v::Treat(void)
{
#ifdef DEBUG
   cout << "SeD12v::Treat" << endl;
#endif

   if (!Ready) return;
   Counter[0]++;
   Init();
   Calibrate();
   Focal();
#ifdef DEBUG
   Show();
#endif

}

void SeD12v::inAttach(TTree* inT)
{
   char strnam[9];

#ifdef DEBUG
   cout << "SeD12v::inAttach " << endl;
#endif

#ifdef DEBUG
   cout << "Attaching SeD12 variables" << endl;
#endif
#ifdef ACTIVEBRANCHES
   cout << "Activating Branches: SeD12v" << endl;
   L->Log << "Activating Branches: SeD12v" << endl;

   sprintf(strnam, "TCSI_HF");
   inT->SetBranchStatus(strnam, 1);
#endif
   sprintf(strnam, "TCSI_HF");
   inT->SetBranchAddress(strnam, &T_Raw[0]);


}
void SeD12v::outAttach(TTree* outT)
{

#ifdef DEBUG
   cout << "SeD12v::outAttach " << endl;
#endif

#ifdef DEBUG
   cout << "Attaching SeD12 variables" << endl;
#endif

   outT->Branch("Xf", &Xf, "Xf/F");
   outT->Branch("Tf", &Tf, "Tf/F");
   outT->Branch("Yf", &Yf, "Yf/F");
   outT->Branch("Pf", &Pf, "Pf/F");
   outT->Branch("TSeD12", &T[0], "TSeD12/F");
   outT->Branch("V", &V, "V/F");


}


void SeD12v::CreateHistograms(void)
{

#ifdef DEBUG
   cout << "SeD12v::CreateHistograms " << endl;
#endif
}
void SeD12v::FillHistograms(void)
{
#ifdef DEBUG
   cout << "SeD12v::FillHistograms " << endl;
#endif


}


void SeD12v::Show(void)
{
#ifdef DEBUG
   cout << "SeD12v::Show" << endl;
#endif
   cout.setf(ios::showpoint);

   cout << "X1: " << S1->X[0] << " Y1: " << S1->X[1] << endl;
   cout << "X2: " << S2->X[0] << " Y2: " << S2->X[1] << endl;

   cout << "Xf: " << Xf << " Tf: " << Tf << endl;
   cout << "Yf: " << Yf << " Pf: " << Pf << endl;
}
