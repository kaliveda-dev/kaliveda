//Created by KVClassFactory on Tue Aug 11 16:38:54 2009
//Author: marini
//2010-10-04 : Files CsI.cal and CsI.pied have been modified to take account the right distances between detectors

#include "CsIv.h"

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

ClassImp(CsIv)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>CsIv</h2>
<h4>CsI VAMOS</h4>
<body>
For the CsIv class :
   <ul>
      <li> Signals over the pedestal are collected
      <li> Multiplicity is calculated
   </ul>

</body>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

CsIv::CsIv(LogFile* Log) :
   L(Log),
   Rnd(NULL)
{

   NbCsI = gEnv->GetValue("VAMOS.NbCsI", -1);
   if (NbCsI < 0) {
      cout << "Not Reading VAMOS.NbCsI in CsIv Class" << endl;
   }

   Ped = new Float_t[NbCsI];       //CsI pedestal
   ECoef = new Float_t* [NbCsI]; //CsI calibration coefficients
   for (Int_t i = 0; i < NbCsI; i++) {
      ECoef[i] = new Float_t[3];
   }

   //energy Raw
   E_Raw = new UShort_t[NbCsI];
   CsIERaw = new Int_t[NbCsI];
   E_Raw_Nr = new UShort_t[NbCsI];

   //energy time Calibrated
   DetCsI = new Int_t[NbCsI];

   InitSavedQuantities();
   Init();
   // Default constructor


   //initialization of calibration parameter
   for (Int_t i = 0; i < 80; i++) {
      Ped[i] = -1.;
   }
   for (Int_t j = 0; j < 80; j++) {
      for (Int_t k = 0; k < 3; k++) {
         ECoef[j][k] = -1.;
      }
   }

   //pedestal reading
   Int_t num = 0;
   Float_t pied = 0.;
   Float_t dummy1, dummy2, dummy3;
   TString sline;
   ifstream in2;

   //reading the piedestal for CsI from P.Wigg work - 2010-10-04
   if (!gDataSet->OpenDataSetFile("CsI.pied", in2)) {
      cout << "Could not open the calibration file CsI.pied !!!" << endl;
      return;
   } else {
      cout << "Reading CsI.pied" << endl;
      while (!in2.eof()) {
         sline.ReadLine(in2);
         if (!in2.eof()) {
            if (!sline.BeginsWith("#")) {
               sscanf(sline.Data(), "%d %f %f %f %f", &num, &dummy1, &dummy2, &dummy3, &pied);
               Ped[num - 1] = pied;
            }
         }
      }
   }
   in2.close();

   //    cout<<"piedestal= "<<pied<<endl;


   Int_t  num2;
   Float_t a1 = 0., a2 = 0., a3 = 0.;
   ifstream in3;

   if (!gDataSet->OpenDataSetFile("CsI.cal", in3)) {
      cout << "Could not open the calibration file CsI.cal !!!" << endl;
      return;
   } else {
      cout << "Reading CsI.cal" << endl;
      while (!in3.eof()) {
         sline.ReadLine(in3);
         if (!in3.eof()) {
            if (!sline.BeginsWith("+") && !sline.BeginsWith("|") && !sline.BeginsWith("#")) {
               sscanf(sline.Data(), "%d %d %f %f %f %f %f", &num, &num2, &a1, &a2, &a3, &dummy1, &dummy2);
               ECoef[num2 - 1][0] = a1;
               ECoef[num2 - 1][1] = a2;
               ECoef[num2 - 1][2] = a3;
               //cout<<"a1 : "<<ECoef[num2-1][0]<<" a2 : "<<ECoef[num2-1][1]<<"   a3 :"<<ECoef[num2-1][2]<<endl;
            }
         }
      }
   }
   in3.close();

}

CsIv::~CsIv()
{
   delete [] Ped;
   delete [] ECoef;

   //energy Raw
   delete [] E_Raw;
   delete [] CsIERaw;
   delete [] E_Raw_Nr;

   //energy time Calibrated
   delete [] DetCsI;

   // Destructor
}


void CsIv::InitSavedQuantities(void)
{
   //cout << "CsI::InitRaw" << endl;

   for (Int_t i = 0; i < NbCsI; i++) {
      CsIERaw[i] = -10;
      DetCsI[i] = -10;
   }
   EMCSI = 0;

}


void CsIv::Init()
{
   //cout<<"Init()"<<endl;

   E_RawM = -10;

   //initialization of calibration parameter
   for (Int_t i = 0; i < NbCsI; i++) {
      Ped[i] = -10.0;
      E_Raw[i] = -10;
      E_Raw_Nr[i] = -10;
   }
   for (Int_t j = 0; j < NbCsI; j++) {
      for (Int_t k = 0; k < 3; k++) {
         ECoef[j][k] = -1.;
      }
   }
}

void CsIv::Treat(void)
{
   InitSavedQuantities();

   for (Int_t i = 0; i < E_RawM; i++) {

      //if(E_Raw[i]==0)cout<<"***********************************CsI Raw before : "<<E_Raw[i]<<" Pied : "<<Ped[int(E_Raw_Nr[i])]<<endl;
      if (float(E_Raw[i]) > Ped[int(E_Raw_Nr[i])]) {
         //if(E_Raw[i]==0)cout<<"***********************************CsI Raw after : "<<E_Raw[i]<<" Pied : "<<Ped[int(E_Raw_Nr[i])]<<endl;
         //cout<<"CsI Raw after : "<<E_Raw[i]<<" Pied : "<<Ped[int(E_Raw_Nr[i])]<<endl;
         CsIERaw[EMCSI] = E_Raw[i];
         DetCsI[EMCSI] = E_Raw_Nr[i] + 1; //numérotation 1-80
         //cout<<"===CsI==="<<endl;
         //cout<<DetCsI[EMCSI]<<" "<<CsIERaw[EMCSI]<<endl;
         EMCSI++;



      }
   }

   //cout<<"Mult CsI : "<<EMCSI<<endl;
   //cout<<"====="<<endl;

}

void CsIv::inAttach(TTree* inT)
{

#ifdef DEBUG
   cout << "CsI::inAttach" << endl;
#endif

#ifdef DEBUG
   cout << "Attaching CsI variables" << endl;
#endif

   inT->SetBranchAddress("WCSIEM", &E_RawM);
   inT->SetBranchAddress("WCSIE", E_Raw);
   inT->SetBranchAddress("WCSIENr", E_Raw_Nr);

}

void CsIv::outAttach(TTree* outT)
{

#ifdef DEBUG
   cout << "CsI::inAttach" << endl;
#endif

#ifdef DEBUG
   cout << "Attaching CsI variables" << endl;
#endif

   outT->Branch("CsIEM", &EMCSI, "EMCSI/I");
   outT->Branch("CsIRaw", CsIERaw, "CsIERaw[EMCSI]/I");
   outT->Branch("DetCsI", DetCsI, "DetCsI[EMCSI]/I");

}

void CsIv::CreateHistograms(void)
{

#ifdef DEBUG
   cout << "CsI::CreateHistograms" << endl;
#endif

}
void CsIv::FillHistograms(void)
{

#ifdef DEBUG
   cout << "CsI::FillHistograms" << endl;
#endif

}

