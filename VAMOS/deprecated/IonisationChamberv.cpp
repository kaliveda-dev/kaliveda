#include "IonisationChamberv.h"

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

ClassImp(IonisationChamberv)
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>IonisationChamberv</h2>
<p>
Part of the VAMOS analysis package kindly contributed by Maurycy Rejmund (GANIL).
</p>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

IonisationChamberv::IonisationChamberv(LogFile* Log)
{
#ifdef DEBUG
   cout << "IonisationChamberv::Constructor" << endl;
#endif
   Ready = kFALSE;

   L = Log;

   NbChio = gEnv->GetValue("VAMOS.NbIC", -1);
   if (NbChio < 0) {
      cout << "Not Reading VAMOS.NbIC in IonisationChamberv Class" << endl;
   }
   //energy Raw
   E_Raw = new UShort_t[NbChio];
   E_Raw_Nr = new UShort_t[NbChio];
   IcRaw = new Int_t[NbChio];

   DetChio = new Int_t [NbChio];

   //Calibration coeff
   a = new Float_t[NbChio];
   b = new Float_t[NbChio];
   Vnorm = new Float_t[NbChio];

   //energy Calibrated
   E = new Float_t[NbChio];

   InitSavedQuantities();
   Init();

   Rnd = new Random;

   ifstream inf;
   Int_t num = 0;
   Float_t dummy1, dummy2, dummy3;
   TString sline;

   if (!gDataSet->OpenDataSetFile("IonisationChamber.cal", inf)) {
      cout << "Could not open the calibration file IonisationChamber.cal !" << endl;
      return;
   } else {
      cout << "Reading IonisationChamber.cal" << endl;
      while (!inf.eof()) {
         sline.ReadLine(inf);
         if (!inf.eof()) {
            if (!sline.BeginsWith("#")) {
               sscanf(sline.Data(), "%d %f %f %f", &num, &dummy1, &dummy2, &dummy3);
               a[num] = dummy1;
               b[num] = dummy2;
               Vnorm[num] = dummy3;
            }
         }
      }
   }
   inf.close();
   Ready = kTRUE;

}
IonisationChamberv::~IonisationChamberv(void)
{
#ifdef DEBUG
   cout << "IonisationChamberv::Destructor" << endl;
#endif
   //energy Raw
   delete [] E_Raw;
   delete [] E_Raw_Nr;
   delete [] IcRaw;

   delete [] DetChio;

   //Calibration coeff
   delete [] a;
   delete [] b;
   delete [] Vnorm;

   //energy Calibrated
   delete [] E ;

   PrintCounters();
}

void IonisationChamberv::PrintCounters(void)
{
#ifdef DEBUG
   cout << "IonisationChamberv::PrintCounters" << endl;
#endif

   cout << endl;
   cout << "IonisationChamberv::PrintCounters" << endl;
   cout << "Called: " << Counter[0] << endl;
   for (Int_t i = 0; i < 3; i++)
      cout << "E" << i << ": " << Counter[i + 1] << endl;
   cout << "Present: " << Counter[4] << endl;


   L->Log << endl;
   L->Log << "IonisationChamberv::PrintCounters" << endl;
   L->Log << "Called: " << Counter[0] << endl;
   for (Int_t i = 0; i < 3; i++)
      L->Log << "E" << i << ": " << Counter[1] << endl;
   L->Log << "Present: " << Counter[4] << endl;

}


void IonisationChamberv::InitSavedQuantities(void)
{
#ifdef DEBUG
   cout << "IonisationChamberv::InitRaw" << endl;
#endif

   for (Int_t j = 0; j < NbChio; j++) {
      DetChio[j] = -10;
      IcRaw[j] = -10;
      E[j] = -10.0;
   }

   EMIC = 0;
}

void IonisationChamberv::Init(void)
{
#ifdef DEBUG
   cout << "IonisationChamberv::Init" << endl;
#endif
   //Present = false;

   eloss = 7.54; //Energy loss (MeV) for the elastic pic form the active zone of the IC

   for (Int_t j = 0; j < NbChio; j++) {
      E_Raw[j] = -10;
      E_Raw_Nr[j] = -10;

      a[j] = -10.0;
      b[j] = -10.0;
      Vnorm[j] = -10.0;
   }

   E_RawM = -10;

   for (Int_t i = 0; i < 5; i++)
      Counter[i] = 0;
}

void IonisationChamberv::Calibrate(void)
{
   InitSavedQuantities();

#ifdef DEBUG
   cout << "IonisationChamberv::Calibrate" << endl;
#endif

//L->Log<<"E_RawM Chio"<<E_RawM<<endl;

   for (Int_t i = 0; i < E_RawM; i++) {

      if (E_Raw[i] > 160) { //Piedestal ~160
         //cout<<"Eraw : "<<E_Raw[i]<<"   Nb : "<<E_Raw_Nr[i]<<"  a : "<<a[E_Raw_Nr[i]]<<" b : "<<b[E_Raw_Nr[i]]<<" Vnorm : "<<Vnorm[E_Raw_Nr[i]]<<" EMIC : "<<EMIC<<endl<<flush;

         Rnd->Next();
         E[EMIC] = ((((Float_t) E_Raw[i] + Rnd->Value()) * a[E_Raw_Nr[i]]) + b[E_Raw_Nr[i]]) * (eloss / Vnorm[E_Raw_Nr[i]]); //The linear fit * k/Vnorm
         DetChio[EMIC] = E_Raw_Nr[i] + 1;
         IcRaw[EMIC] = E_Raw[i];

         //cout<<"===Chio==="<<endl;
         //cout<<" E : "<<E[EMIC]<<" DetChio : "<<DetChio[EMIC]<<" IcRaw[EMIC] : "<<IcRaw[EMIC]<<endl;
         EMIC++;
      }
   }
   //cout<<"Mult IC : "<<EMIC<<endl;
   //cout<<"====="<<endl;

   //cout<<"Calibrate done"<<endl;
}

void IonisationChamberv::Treat(void)
{
#ifdef DEBUG
   cout << "IonisationChamberv::Treat" << endl;
#endif
   if (!Ready) return;
   Counter[0]++;
   Calibrate();
#ifdef DEBUG
   Show_Raw();
   Show();
#endif
}

void IonisationChamberv::inAttach(TTree* inT)
{

#ifdef DEBUG
   cout << "IonisationChamberv::inAttach" << endl;
#endif

#ifdef DEBUG
   cout << "Attaching IonisationChamberv variables" << endl;
#endif

#ifdef ACTIVEBRANCHES
   cout << "Activating Branches: IonisationChamberv" << endl;
   L->Log << "Activating Branches: IonisationChamberv" << endl;

   inT->SetBranchStatus("EchiM", 1);
   inT->SetBranchStatus("Echi", 1);
   inT->SetBranchStatus("EchiNr", 1);

#endif

   inT->SetBranchAddress("EchiM", &E_RawM);
   inT->SetBranchAddress("Echi", E_Raw);
   inT->SetBranchAddress("EchiNr", E_Raw_Nr);

}
void IonisationChamberv::outAttach(TTree* outT)
{

#ifdef DEBUG
   cout << "IonisationChamberv::outAttach" << endl;
#endif

#ifdef DEBUG
   cout << "Attaching IonisationChamberv variables" << endl;
#endif
   //outT->Branch("IcEA",&ES[0],"IcEA/F");
   //outT->Branch("IcEB",&ES[1],"IcEB/F");
   //outT->Branch("IcEC",&ES[2],"IcEC/F");

   outT->Branch("IcM", &EMIC, "EMIC/I");

   outT->Branch("IcRaw", IcRaw, "IcRaw[EMIC]/I");
   outT->Branch("IcET", E, "E[EMIC]/F");;

   outT->Branch("IcNb", DetChio, "DetChio[EMIC]/I");


}
void IonisationChamberv::CreateHistograms(void)
{

#ifdef DEBUG
   cout << "IonisationChamberv::CreateHistograms" << endl;
#endif

}
void IonisationChamberv::FillHistograms(void)
{

#ifdef DEBUG
   cout << "IonisationChamberv::FillHistograms" << endl;
#endif

}


void IonisationChamberv::Show_Raw(void)
{
   Int_t i;
#ifdef DEBUG
   cout << "IonisationChamberv::Show_Raw" << endl;
#endif

   cout.setf(ios::showpoint);

   int a, b;

   cout << "ERawM: " << E_RawM << endl;
   for (i = 0; i < E_RawM; i++) {
      a = (E_Raw_Nr[i] + 1) / 8;
      b = ((E_Raw_Nr[i] + 1) % 8) - 1;
      if (b != -1) {
         cout << E_Raw_Nr[i] << endl;
         cout << "PAD " << a << " " << b << ": ";
         cout << E_Raw[i] << " ";
         cout << endl;
      }
   }


}
void IonisationChamberv::Show(void)
{
   //Int_t i;
   //int a, b;

#ifdef DEBUG
   cout << "IonisationChamberv::Show" << endl;
#endif

   //cout.setf(ios::showpoint);

   //cout << "Present: " << Present << endl;
   //if(Present)
   //{
   //cout << "EM: " << EM << endl;
   /*for(i=0;i<24;i++)
   {
     a=(i+1)/8;
     b=((i+1)%8)-1;
     if(b!=-1)
       {
         if(E[i] > 0)
     {
       cout << "ROW " << a << " PAD " << b << ": ";
       cout << E[i] <<endl;
     }
       }
   }
        for(i=0;i<3;i++)*/
   //cout << "SUM ROW:" << i << " " << ES[i] << endl;
   //cout << "ETotal: " << ETotal << endl;
   //}
}

