#include "Reconstructionv.h"

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

ClassImp(Reconstructionv)

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>Reconstructionv</h2>
<p>
Part of the VAMOS analysis package kindly contributed by Maurycy Rejmund (GANIL).
</p>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

Reconstructionv::Reconstructionv(LogFile* Log, DriftChamberv* Drift)
{
//#ifdef DEBUG
   cout << "Reconstructionv::Constructor" << endl;
//#endif
   Ready = kFALSE;
   char line[255];
   char fname[30];
   int len = 255;

   L = Log;
   Dr = Drift;


   for (Int_t i = 0; i < 6; i++)
      Counter[i] = 0;

   cout << "Start Init Reconstructionv" << endl << flush;
   Init();
   cout << "Finish Init Reconstructionv" << endl << flush;
   Rnd = new Random;

//===================================================
//Phi acceptance correction
   ifstream file;
   TString sline;
   int tmp = 0;

   Float_t delta1;
   Float_t delta2;
   Float_t deg1;
   Float_t deg2;
   Float_t mean1;
   Float_t rms1;
   Float_t etendue1;
   Float_t stat1;
   Float_t facteur1;

   for (Int_t i = 0; i < 600; i++) {
      Delta1[i] = 0.0;
      Delta2[i] = 0.0;
      Deg1[i] = 0.0;
      Deg2[i] = 0.0;
      Facteur[i] = 0.0;
      Stat[i] = 0.0;
      Etendue[i] = 0.0;
   }

   cout << "Start Reading pl_proj.dat" << endl << flush;
   if (!gDataSet->OpenDataSetFile("pl_proj.dat", file)) {
      cout << "Could not open the file for the phiL correction !" << endl;
      return;
   }

   while (file.good()) {         //reading the file
      sline.ReadLine(file);
      if (!file.eof()) {          //fin du fichier
         if (sline.Sizeof() > 1 && !sline.BeginsWith("#")) {
            sscanf(sline.Data(), "%f %f %f %f %f %f %f %f %f", &delta1, &delta2, &deg1, &deg2, &mean1, &rms1, &etendue1, &stat1, &facteur1);
            Delta1[tmp] = delta1;
            Delta2[tmp] = delta2;
            Deg1[tmp] = deg1;
            Deg2[tmp] = deg2;
            Facteur[tmp] = facteur1;
            Stat[tmp] = stat1;
            Etendue[tmp] = etendue1;
            tmp++;
         }
      }
   }
   file.close();
   cout << "Finish Reading pl_proj.dat" << endl << flush;
//===================================================


//===================================================
//Tagging the event to not count events inside the different overlap regions

   /*ifstream file2;
   TString sline2;
   int tmp2=0;

   Float_t brho0;
   Int_t run1;
   Int_t run2;
   Float_t brhomin;
   Float_t brhomax;

   for(Int_t i=0;i<600;i++){
      Brho_min[i] = -10.0;
      Brho_max[i] = -10.0;
   }

   if(!gDataSet->OpenDataSetFile("Brho_tag.dat", file2))
   {
      cout << "Could not open the file for the tag !" << endl;
         return;
   }

   while (file2.good()) {         //reading the file
         sline2.ReadLine(file2);
         if (!file2.eof()) {          //fin du fichier
         if (sline2.Sizeof() > 1 && !sline2.BeginsWith("#")){
            sscanf(sline2.Data(),"%f %d %d %f %f", &brho0, &run1, &run2, &brhomin, &brhomax);
               for(Int_t i=run1; i<run2; i++){
                  Brho_min[i] = brhomin;
                  Brho_max[i] = brhomax;
                  }
               if(run1==run2)
                  {
                  Brho_min[run1] = brhomin;
                  Brho_max[run2] = brhomax;
                  }
               if(TMath::Abs(run2-run1)==1)
                  {
                  Brho_min[run1] = brhomin;
                  Brho_max[run1] = brhomax;
                  Brho_min[run2] = brhomin;
                  Brho_max[run2] = brhomax;
                  }
         }
      }
   }
   file2.close(); */

//===================================================
   cout << "Start Reading Vamos_distance.dat" << endl << flush;

   ifstream inf3;
   if (!gDataSet->OpenDataSetFile("Vamos_distance.dat", inf3)) {
      cout << "Could not open the calibration file Vamos_distance.dat !" << endl;
      return;
   } else {
      cout << "Reading Vamos_distance.dat " << endl;
      L->Log << "Reading Vamos_distance.dat " << endl;


      inf3.getline(line, len);
      inf3.getline(line, len);
      inf3 >> DDC1;
      cout << DDC1 << endl;
      L->Log << DDC1 << endl;

      inf3.getline(line, len);
      inf3.getline(line, len);
      inf3 >> DSED1;
      cout << DSED1 << endl;
      L->Log << DSED1 << endl;

      inf3.getline(line, len);
      inf3.getline(line, len);
      inf3 >> DDC2;
      cout << DDC2 << endl;
      L->Log << DDC2 << endl;

      inf3.getline(line, len);
      inf3.getline(line, len);
      inf3 >> DCHIO;
      cout << DCHIO << endl;
      L->Log << DCHIO << endl;

      inf3.getline(line, len);
      inf3.getline(line, len);
      inf3 >> DSI;
      cout << DSI << endl;
      L->Log << DSI << endl;

      inf3.getline(line, len);
      inf3.getline(line, len);
      inf3 >> DCSI;
      cout << DCSI << endl;
      L->Log << DCSI << endl;
   }
   inf3.close();
   cout << "Finish Reading Vamos_distance.dat" << endl << flush;

   cout << "Start Reading Reconstruction.cal" << endl << flush;
   ifstream inf;
   if (!gDataSet->OpenDataSetFile("Reconstruction.cal", inf)) {
      cout << "Could not open the calibration file Reconstruction.cal !" << endl;
      return;
   } else {
      cout.setf(ios::showpoint);
      cout << "Reading Reconstruction.cal" << endl;
      L->Log << "Reading Reconstruction.cal" << endl;


      inf.getline(line, len);
      cout << line << endl;
      L->Log << line << endl;
      inf >> BrhoRef;
      cout << BrhoRef << endl;
      L->Log << BrhoRef << endl;

      inf.getline(line, len);

      for (Int_t j = 0; j < 4; j++) {
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

            for (Int_t i = 0; i < 330; i++)
               inf1 >> Coef[j][i];

         }
         inf1.close();
      }
      inf.getline(line, len);
      cout << line << endl;
      L->Log << line << endl;
      inf >> PathOffset;
      cout << "PathOffset: " << PathOffset << endl;
      L->Log << "PathOffset: " << PathOffset << endl;
   }
   inf.close();
   Ready = kTRUE;

   cout << "Finish Reading Reconstruction.cal" << endl << flush;
}
Reconstructionv::~Reconstructionv(void)
{
#ifdef DEBUG
   cout << "Reconstructionv::Destructor" << endl;
#endif

   PrintCounters();
}

void Reconstructionv::PrintCounters(void)
{
#ifdef DEBUG
   cout << "Reconstructionv::PrintCounters" << endl;
#endif
   cout << endl;
   cout << "Reconstructionv::PrintCounters" << endl;
   cout << "Called :" << Counter[0] << endl;
   cout << "Called with Drift Present :" << Counter[1] << endl;
   cout << "Present :" << Counter[2] << endl;

   L->Log << endl;
   L->Log << "Reconstructionv::PrintCounters" << endl;
   L->Log << "Called :" << Counter[0] << endl;
   L->Log << "Called with Drift Present :" << Counter[1] << endl;
   L->Log << "Present :" << Counter[2] << endl;


}

void Reconstructionv::Init(void)
{
#ifdef DEBUG
   cout << "Reconstructionv::Init" << endl;
#endif
   Present = false;

   //BB = ttheta = 0;
   Brho = Theta = Phi = Path = -500;
   ThetaL = PhiL = Thetadeg = Phideg = ThetaLdeg = PhiLdeg = corr_pl = deltat = -500.;
}

void Reconstructionv::Calculate(void)
{
#ifdef DEBUG
   cout << "Reconstructionv::Calculate" << endl;
#endif

   Double_t Brhot, Thetat, Phit, Patht;
   //Double_t Brhot_y,Thetat_y,Patht_y;
   Double_t Vec[5], Vecp;

   Int_t i, j[10];

   Brhot = Thetat = Phit = Patht = 0.00000;
   Vec[0] = 1.000000;
   Vec[1] = (Double_t)(-1. * (Dr->Xf) / 1000.);
   Vec[2] = 0.; //(Double_t) (-1. * (Dr->Yf)/1000.);
   Vec[3] = (Double_t)(-1. * (Dr->Tf) / 1000.);
   Vec[4] = 0.; //(Double_t) (-1. * atan(tan((Dr->Pf)/1000.)*cos((Dr->Tf)/1000.)));
   //goes to zgoubi coordinates

//cout<<"Xf = "<<Dr->Xf<<endl;
//cout<<"Tf = "<<Dr->Tf<<endl;

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
   Vec[1] = (Double_t)(-1. * (Dr->Xf) / 1000.);
   Vec[2] = (Double_t)(-1. * (Dr->Yf) / 1000.);
   Vec[3] = (Double_t)(-1. * (Dr->Tf) / 1000.);
   Vec[4] = (Double_t)(-1. * atan(tan((Dr->Pf) / 1000.) * cos((Dr->Tf) / 1000.)));

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
   //L->Log<<"-----------"<<endl;
   //L->Log << "Brhot = "<< Brhot << " " << "Thetat = "<< Thetat << " " <<"Phit = "<< Phit <<" "<<"Patht = "<<Patht<<endl;
   //L->Log<<"-----------"<<endl;
   //  cout << i << " " << Dr->Xf/10. << " " << Dr->Tf << " " << Dr->Yf/10. << " " << Dr->Pf << endl;

   //cout<<"Brho : "<<BrhoRef*((Float_t) Brhot)<<endl;
   //cout<<"Brhot : "<<Brhot <<" Thetat : "<< Thetat<<" Phit : "<<Phit<<" Patht : "<<Patht<<endl;

   Brho_always = BrhoRef * ((Float_t) Brhot);
   Theta_always = (Float_t)((Thetat * -1.) / 1000.) * TMath::RadToDeg();
   Phi_always = (Float_t)((Phit * -1.) / 1000.) * TMath::RadToDeg();
   Path_always = (Float_t) Patht;


   if (Brhot > 0.001 && Thetat > -300. && Thetat < 300.
         && Phit > -300. && Phit < 300. && Patht > 0 && Patht < 2000.) {
      Counter[2]++;
      Present = true;
      Brho = BrhoRef * ((Float_t) Brhot);
      Theta = (Float_t) Thetat * -1;
      Phi = (Float_t) Phit * -1;
      Path = (Float_t) Patht + PathOffset;

      TVector3 myVec(sin(Theta / 1000.)*cos(Phi / 1000.), sin(Phi / 1000.), cos(Theta / 1000.)*cos(Phi / 1000.));
      myVec.RotateY(AngleVamos * TMath::DegToRad());
      ThetaL = myVec.Theta();
      PhiL = myVec.Phi();
      Thetadeg = (Theta / 1000.) * TMath::RadToDeg();
      Phideg = (Phi / 1000.) * TMath::RadToDeg();
      ThetaLdeg = ThetaL * TMath::RadToDeg();
      PhiLdeg = PhiL * TMath::RadToDeg();

      // with Yf and Pf dependence in reconstruction:
      /*Brho_y = BrhoRef*((Float_t) Brhot_y);
      Theta_y = (Float_t) Thetat_y*-1;
      Path_y = (Float_t) Patht_y + PathOffset;

      TVector3 *myVec_y;
      myVec_y = new TVector3(sin(Theta_y/1000.)*cos(Phi/1000.),sin(Phi/1000.),cos(Theta_y/1000.)*cos(Phi/1000.));
      myVec_y->RotateY(20.*TMath::DegToRad()); //VAMOS angle
      ThetaL_y = myVec_y->Theta();
      ThetaLdeg_y = ThetaL_y*TMath::RadToDeg();
      PhiL_y = myVec_y->Phi();*/

      //cout << Brho << " " << Theta << " " << Phi << endl;
      //L->Log<<"-----------"<<endl;
      //L->Log << "Brho = "<< Brho << " " << "Theta = "<< Theta << " " <<"Phi = "<< Phi <<" "<<"Path = "<<Path<<endl;
      //L->Log << "ThetaL = "<< ThetaL << " " <<"PhiL = "<< PhiL <<endl;
      //L->Log<<"-----------"<<endl;

//Tagging the event...
      /* if(Brho_min[gIndra->GetCurrentRunNumber()] < Brho && Brho < Brho_max[gIndra->GetCurrentRunNumber()]){
            Brho_tag = 1;
            }
         else{
            Brho_tag = 0;
            }*/



//Warning : Correction (corr_pl) is applied for thetaL<0.12 rad and phiL between -1 and 1 rad.
      deltat = double(Brho / BrhoRef);
      //L->Log<<"Delta : "<<deltat<<endl;

      for (Int_t j = 0; j < 450; j++) {
         if (Delta1[j] < deltat && deltat < Delta2[j] && Deg1[j] < ThetaLdeg && ThetaLdeg < Deg2[j]) {
            corr_pl = double(Facteur[j]);
            //cout<<"corr_pl  = "<<corr_pl<<endl;
            break;
         } else if (Delta1[j] < deltat && Delta2[j] > deltat && Deg2[j - 1] < ThetaLdeg && ThetaLdeg < Deg1[j]) {
            corr_pl = double((Facteur[j] + Facteur[j - 1]) / 2);
            //cout<<"corr_pl moy = "<<corr_pl<<endl;
            break;
         }
      }
   }

}


void Reconstructionv::Treat(void)
{
#ifdef DEBUG
   cout << "Reconstructionv::Treat" << endl;
#endif

   if (!Ready) return;

   Counter[0]++;
   Init();
   if (Dr->Present) {
      Counter[1]++;
      Calculate();
#ifdef DEBUG
      Show();
#endif
   }
}

void Reconstructionv::outAttach(TTree* outT)
{

#ifdef DEBUG
   cout << "Reconstructionv::outAttach " << endl;
#endif

#ifdef DEBUG
   cout << "Attaching Reconstructionv variables" << endl;
#endif

   outT->Branch("Brho", &Brho, "Brho/F");
   outT->Branch("Thetadeg", &Thetadeg, "Thetadeg/F");
   outT->Branch("Phideg", &Phideg, "Phideg/F");
   outT->Branch("ThetaLdeg", &ThetaLdeg, "ThetaLdeg/F");
   outT->Branch("PhiLdeg", &PhiLdeg, "PhiLdeg/F");
   outT->Branch("corr_pl", &corr_pl, "corr_pl/D");
   outT->Branch("Delta", &deltat, "deltat/D");

   //outT->Branch("Theta",&Theta,"Theta/F");
   //outT->Branch("Phi",&Phi,"Phi/F");
   outT->Branch("Path", &Path, "Path/F");
   outT->Branch("Brho_always", &Brho_always, "Brho_always/F");
   outT->Branch("Theta_always", &Theta_always, "Theta_always/F");
   outT->Branch("Phi_always", &Phi_always, "Phi_always/F");
   outT->Branch("Path_always", &Path_always, "Path_always/F");
   //outT->Branch("ThetaL",&ThetaL,"ThetaL/F");
   //outT->Branch("PhiL",&PhiL,"PhiL/F");*/

   //outT->Branch("Brho_tag",&Brho_tag,"Brho_tag/I");

   /*outT->Branch("Brho_y",&Brho_y,"Brho_y/F");
   outT->Branch("Theta_y",&Theta_y,"Theta_y/F");
   outT->Branch("Path_y",&Path_y,"Path_y/F");
   outT->Branch("ThetaL_y",&ThetaL_y,"ThetaL_y/F");
   outT->Branch("ThetaLdeg_y",&ThetaLdeg_y,"ThetaLdeg_y/F");
   outT->Branch("PhiL_y",&PhiL_y,"PhiL_y/F");*/

}


void Reconstructionv::CreateHistograms(void)
{

#ifdef DEBUG
   cout << "Reconstructionv::CreateHistograms " << endl;
#endif
}
void Reconstructionv::FillHistograms(void)
{
#ifdef DEBUG
   cout << "Reconstructionv::FillHistograms " << endl;
#endif

}


void Reconstructionv::Show(void)
{
#ifdef DEBUG
   cout << "Reconstructionv::Show" << endl;
#endif
   cout.setf(ios::showpoint);

}

