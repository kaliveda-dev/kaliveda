#include "ReconstructionSeDv.h"

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

ClassImp(ReconstructionSeDv)

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>ReconstructionSeDv</h2>
<p>
Part of the VAMOS analysis package kindly contributed by Maurycy Rejmund (GANIL).
</p>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

ReconstructionSeDv::ReconstructionSeDv(LogFile* Log, SeD12v* SeD12)
{
#ifdef DEBUG
   cout << "ReconstructionSeDv::Constructor" << endl;
#endif
   Ready = kFALSE;
   char line[255];
   char fname[30];
   int len = 255;
   int i, j;

   L = Log;
   S12 = SeD12;


   for (i = 0; i < 6; i++)
      Counter[i] = 0;

   Init();

   Rnd = new Random;



   ifstream inf;

   if (!gDataSet->OpenDataSetFile("ReconstructionSeD.cal", inf)) {
      cout << "Could not open the calibration file ReconstructionSeD.cal !" << endl;
      return;
   } else {
      cout.setf(ios::showpoint);
      cout << "Reading ReconstructionSeD.cal" << endl;
      L->Log << "Reading ReconstructionSeD.cal" << endl;


      inf.getline(line, len);
      cout << line << endl;
      L->Log << line << endl;
      inf >> BrhoRef;
      cout << BrhoRef << endl;
      L->Log << BrhoRef << endl;

      inf.getline(line, len);

      for (j = 0; j < 4; j++) {
         inf.getline(line, len);
         cout << line << endl;
         L->Log << line << endl;
         inf.getline(line, len);
         sscanf(line, "%s", fname);


         ifstream inf1;

         if (!gDataSet->OpenDataSetFile(fname, inf1)) {
            cout << "Could not open the calibration file " << fname << endl;
            return;
         } else {
            cout.setf(ios::showpoint);
            cout << "Reading " << fname << endl;
            L->Log << "Reading " << fname << endl;
            inf1.getline(line, len);
            cout << line << endl;
            L->Log << line << endl;

            for (i = 0; i < 1001; i++)
               inf1 >> Coef[j][i];

         }
         inf1.close();
      }
   }
   inf.close();
   Ready = kTRUE;
}
ReconstructionSeDv::~ReconstructionSeDv(void)
{
#ifdef DEBUG
   cout << "ReconstructionSeDv::Destructor" << endl;
#endif

   PrintCounters();
}

void ReconstructionSeDv::PrintCounters(void)
{
#ifdef DEBUG
   cout << "ReconstructionSeDv::PrintCounters" << endl;
#endif
   cout << endl;
   cout << "ReconstructionSeDv::PrintCounters" << endl;
   cout << "Called :" << Counter[0] << endl;
   cout << "Called with SeDs Present :" << Counter[1] << endl;
   cout << "Present :" << Counter[2] << endl;

   L->Log << endl;
   L->Log << "ReconstructionSeDv::PrintCounters" << endl;
   L->Log << "Called :" << Counter[0] << endl;
   L->Log << "Called with SeDs Present :" << Counter[1] << endl;
   L->Log << "Present :" << Counter[2] << endl;


}

void ReconstructionSeDv::Init(void)
{
#ifdef DEBUG
   cout << "ReconstructionSeDv::Init" << endl;
#endif
   Present = false;

   Brho = Theta = Phi = Path = -500;
   ThetaL = PhiL = -500.;
}

void ReconstructionSeDv::Calculate(void)
{
#ifdef DEBUG
   cout << "ReconstructionSeDv::Calculate" << endl;
#endif
   Double_t Brhot, Thetat, Phit, Patht;
   Double_t Vec[5], Vecp;

   Int_t i, j[10];
   for (Int_t nn = 0; nn < 10; nn += 1) j[nn] = 0;

   Brhot = Thetat = Phit = Patht = 0.00000;
   Vec[0] = 1.000000;
   Vec[1] = (Double_t)(-1. * (S12->Xf) / 1000.);
   Vec[2] = 0.; //(Double_t) (-1. * (S12->Yf)/1000.);
   Vec[3] = (Double_t)(-1. * (S12->Tf) / 1000.);
   Vec[4] = 0.; //(Double_t) (-1. * atan(tan((S12->Pf)/1000.)*cos((S12->Tf)/1000.)));
   //goes to zgoubi coordinates

   i = 0;
   for (j[0] = 0; j[0] < 5; j[0]++)
      for (j[1] = j[0]; j[1] < 5; j[1]++)
         for (j[2] = j[1]; j[2] < 5; j[2]++)
            for (j[3] = j[2]; j[3] < 5; j[3]++)
               for (j[4] = j[3]; j[4] < 5; j[4]++)
                  for (j[5] = j[4]; j[5] < 5; j[5]++)
                     for (j[6] = j[5]; j[6] < 5; j[6]++)
                        //for(j[7]=j[6];j[7]<5;j[7]++)
                        //for(j[8]=j[7];j[8]<5;j[8]++)
                        //for(j[9]=j[8];j[9]<5;j[9]++)
                     {
                        Vecp = Vec[j[0]] * Vec[j[1]] * Vec[j[2]] * Vec[j[3]] * Vec[j[4]] * Vec[j[5]] * Vec[j[6]]; //*Vec[j[7]]*Vec[j[8]]*Vec[j[9]];
                        Brhot += Coef[0][i] * Vecp;
                        Thetat += Coef[1][i] * Vecp;
                        Patht += Coef[3][i] * Vecp;
                        i++;
                     }
   Vec[0] = 1.000000;
   Vec[1] = (Double_t)(-1. * (S12->Xf) / 1000.);
   Vec[2] = (Double_t)(-1. * (S12->Yf) / 1000.);
   Vec[3] = (Double_t)(-1. * (S12->Tf) / 1000.);
   Vec[4] = (Double_t)(-1. * atan(tan((S12->Pf) / 1000.) * cos((S12->Tf) / 1000.)));

   i = 0;
   for (j[0] = 0; j[0] < 5; j[0]++)
      for (j[1] = j[0]; j[1] < 5; j[1]++)
         for (j[2] = j[1]; j[2] < 5; j[2]++)
            for (j[3] = j[2]; j[3] < 5; j[3]++)
               for (j[4] = j[3]; j[4] < 5; j[4]++)
                  for (j[5] = j[4]; j[5] < 5; j[5]++)
                     for (j[6] = j[5]; j[6] < 5; j[6]++)
                        //for(j[7]=j[6];j[7]<5;j[7]++)
                        //for(j[8]=j[7];j[8]<5;j[8]++)
                        //for(j[9]=j[8];j[9]<5;j[9]++)
                     {
                        Vecp = Vec[j[0]] * Vec[j[1]] * Vec[j[2]] * Vec[j[3]] * Vec[j[4]] * Vec[j[5]] * Vec[j[6]]; //*Vec[j[7]]*Vec[j[8]]*Vec[j[9]];
                        Phit += Coef[2][i] * Vecp;
                        /*Brhot_y += Coef[0][i] *Vecp;
                        Thetat_y += Coef[1][i] *Vecp;
                        Patht_y += Coef[3][i] *Vecp;*/
                        i++;
                     }


   //    cout << i << " " << S12->Xf/10. << " " << S12->Tf << " " << S12->Yf/10. << " " << S12->Pf << endl;
   //    cout << Brhot << " " << Thetat << " " << Phit << " " << Patht<< endl;
   if (Brhot > 0.001 && Thetat > -300. && Thetat < 300.
         && Phit > -300. && Phit < 300. && Patht > 0 && Patht < 2000.) {
      Counter[2]++;
      Present = true;
      Brho = BrhoRef * ((Float_t) Brhot);
      Theta = (Float_t) Thetat * -1;
      Phi = (Float_t) Phit * -1;
      Path = (Float_t) Patht;

      TVector3 myVec(sin(Theta / 1000.)*cos(Phi / 1000.), sin(Phi / 1000.), cos(Theta / 1000.)*cos(Phi / 1000.));
      myVec.RotateY(0.*3.141592654 / 180.);
      ThetaL = myVec.Theta();
      PhiL = myVec.Phi();

      //      cout << Brho << " " << Theta << " " << Phi << endl;
   }


}


void ReconstructionSeDv::Treat(void)
{
#ifdef DEBUG
   cout << "ReconstructionSeDv::Treat" << endl;
#endif

   if (!Ready) return;

   Counter[0]++;
   if (S12->Present) {
      Counter[1]++;
      Init();
      Calculate();
#ifdef DEBUG
      Show();
#endif
   }
}

void ReconstructionSeDv::outAttach(TTree* outT)
{

#ifdef DEBUG
   cout << "ReconstructionSeDv::outAttach " << endl;
#endif

#ifdef DEBUG
   cout << "Attaching ReconstructionSeDv variables" << endl;
#endif

   outT->Branch("Brho", &Brho, "Brho/F");
   outT->Branch("Theta", &Theta, "Theta/F");
   outT->Branch("Phi", &Phi, "Phi/F");
   outT->Branch("Path", &Path, "Path/F");
   outT->Branch("ThetaL", &ThetaL, "ThetaL/F");
   outT->Branch("PhiL", &PhiL, "PhiL/F");


}


void ReconstructionSeDv::CreateHistograms(void)
{

#ifdef DEBUG
   cout << "ReconstructionSeDv::CreateHistograms " << endl;
#endif
}
void ReconstructionSeDv::FillHistograms(void)
{
#ifdef DEBUG
   cout << "ReconstructionSeDv::FillHistograms " << endl;
#endif

}


void ReconstructionSeDv::Show(void)
{
#ifdef DEBUG
   cout << "ReconstructionSeDv::Show" << endl;
#endif
   cout.setf(ios::showpoint);

}

