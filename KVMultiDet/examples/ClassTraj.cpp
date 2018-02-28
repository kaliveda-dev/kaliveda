//Created by KVClassFactory on Thu Jun 24 14:22:17 2010
//Author: John Frankland,,,

#include "ClassTraj.h"
#include "Riostream.h"
using namespace std;
#include "KVTreeAnalyzer.h"

ClassImp(ClassTraj)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>ClassTraj</h2>
<h4>Classical trajectory for heavy ion collisions</h4>

Friction parameter
==================
alpha=240 gives maximum fusion X-section for 40Ca+60Ni@6MeV/u (bfus<=5.25fm: 866mb)

alpha=750 gives 66mb of fusion for 84Kr(605 MeV)+238U
   - measured fusion-fission Xsec from PRL 33, 502 (1974): 55+/-15 mb
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

ClassTraj::ClassTraj(const KVNucleus& proj, const KVNucleus& targ, Bool_t viz)
   : KVRungeKutta(8), fWilcke(proj, targ), fVisualization(viz)
{
   // 2-D problem
   // x-axis = beam
   // y-axis = impact parameter
   // projectile comes from -x direction
   alpha = 0;
   coords = new Double_t [8];
   RP = coords;
   RT = coords + 2;
   VP = coords + 4;
   VT = coords + 6;

   sysName.Form("%s + %s", proj.GetSymbol(), targ.GetSymbol());
   colName.Form("%s%s", proj.GetSymbol(), targ.GetSymbol());

   fWilcke.Print();

   CMPositionPad = LabPositionPad = 0;
   ProjARC = TargARC = ProjARClab = TargARClab = 0;
   infox = 0;
   if (fVisualization) Reset();
}

void ClassTraj::UpdateARCPositions(Double_t t)
{
   ProjARC->SetX1(RP[0]);
   ProjARC->SetY1(RP[1]);
   TargARC->SetX1(RT[0]);
   TargARC->SetY1(RT[1]);
   ProjTRAJ->SetPoint(nTRAJPoints++, RP[0], RP[1]);
   ProjARClab->SetX1(RP[0] + RCM[0] + VCM[0]*t);
   ProjARClab->SetY1(RP[1] + RCM[1] + VCM[1]*t);
   TargARClab->SetX1(RT[0] + RCM[0] + VCM[0]*t);
   TargARClab->SetY1(RT[1] + RCM[1] + VCM[1]*t);
   ProjTRAJlab->SetPoint(nTRAJPointslab++, RP[0] + RCM[0] + VCM[0]*t, RP[1] + RCM[1] + VCM[1]*t);
}


ClassTraj::~ClassTraj()
{
   delete [] coords;
}

void ClassTraj::InitTrajectory(Double_t b, Double_t e, Double_t fric)
{
   // set up trajectory with impact parameter b
   // energy e mev/nucleon

   if (fVisualization)UpdateInfos(b, 0);
   alpha = fric;
   if (fVisualization) {
      ProjTRAJ = new TGraph;
      ProjTRAJ->SetLineWidth(2);
      ProjTRAJlab = new TGraph;
      ProjTRAJlab->SetLineWidth(2);
   }
   ELAB = e;
   Double_t R0 = minDist;
   // lab coordinates
   RP[0] = -R0;
   RP[1] = b;
   RT[0] = RT[1] = 0;
   for (int i = 0; i < 2; i++) {
      RCM[i] = (fWilcke.GetAP() * RP[i] + fWilcke.GetAT() * RT[i]) / (1.*fWilcke.GetAC());
      RP[i] -= RCM[i];
      RT[i] -= RCM[i];
   }
//    cout << "CENTRE OF MASS : (" << RCM[0] << "," << RCM[1] << ")" << endl;
//    cout << "INITIAL DISTANCE :" << dr(RP,RT) << " fm" << endl;
//    cout << "INITIAL POSITIONS :" << endl;
//    cout << "   PROJECTILE : (" << RP[0] << "," << RP[1] << ")" << endl;
//    cout << "   TARGET  : (" << RT[0] << "," << RT[1] << ")" << endl;
//    cout << "POTENTIAL ENERGY : " << fWilcke.GetTotalPotential()->Eval(dr(RP,RT)) << " MeV" << endl;
//    cout << "CM TOTAL ENERGY : " << fWilcke.ECM(ELAB) << " MeV" << endl;
   E0 = fWilcke.ECM(ELAB) - fWilcke.GetTotalPotential()->Eval(dr(RP, RT));
//    cout << "INITIAL CM KINETIC ENERGY : " << E0 << " MeV" << endl;
   Double_t V0 = sqrt(2.*E0 / fWilcke.GetMu() / KVNucleus::kAMU);
//    cout << "INITIAL PROJECTILE VELOCITY : " << V0*KVNucleus::C() << " cm/ns" << endl;
   // lab velocities
   VP[0] = V0;
   VP[1] = VT[0] = VT[1] = 0.;
   for (int i = 0; i < 2; i++) {
      VCM[i] = (fWilcke.GetAP() * VP[i] + fWilcke.GetAT() * VT[i]) / (1.*fWilcke.GetAC());
      VP[i] -= VCM[i];
      VT[i] -= VCM[i];
   }
//    cout << "CENTRE OF MASS VELOCITY : " << VCM[0]*KVNucleus::C() << " cm/ns" << endl;
//    cout << "INITIAL CM VELOCITIES :" << endl;
//    cout << "   PROJECTILE : " << VP[0]*KVNucleus::C() << " cm/ns"  << endl;
//    cout << "   TARGET  : " << VT[0]*KVNucleus::C() << " cm/ns" << endl;

   if (fVisualization) {
      nTRAJPoints = 0;
      nTRAJPointslab = 0;
      UpdateARCPositions(0);
      CMPositionPad->cd();
      ProjARC->Draw();
      TargARC->Draw();
      ProjTRAJ->Draw("l");
      CMPositionPad->Modified();
      CMPositionPad->Update();
      LabPositionPad->cd();
      ProjARClab->Draw();
      TargARClab->Draw();
      ProjTRAJlab->Draw("l");
      LabPositionPad->Modified();
      LabPositionPad->Update();
   }
}

void ClassTraj::CalcDerivs(Double_t t, Double_t* Y, Double_t* DYDX)
{
   Double_t* rp = Y;
   Double_t* rt = Y + 2;
   Double_t* vp = Y + 4;
   Double_t* vt = Y + 6;
   Double_t* drp = DYDX;
   Double_t* drt = DYDX + 2;
   Double_t* dvp = DYDX + 4;
   Double_t* dvt = DYDX + 6;

   Double_t distance = dr(rp, rt);
   Double_t uv_x = dr_x(rp, rt);
   Double_t uv_y = dr_y(rp, rt);

   Double_t Force = -fWilcke.GetTotalPotential()->Derivative(distance);
   if (TMath::IsNaN(Force)) Force = 0;

   for (int i = 0; i < 2; i++) {
      drp[i] = vp[i];
      drt[i] = vt[i];
   }
   Double_t fp = Force / (fWilcke.GetAP() * KVNucleus::kAMU);
   dvp[0] = fp * uv_x;
   dvp[1] = fp * uv_y;
   Double_t ft = -Force / (fWilcke.GetAT() * KVNucleus::kAMU);
   dvt[0] = ft * uv_x;
   dvt[1] = ft * uv_y;

   if (fInitialDeriv) {
      if (distance <= minDist)minDist = distance;
      if (incoming && distance < fWilcke.GetRint() + 10.) {
         incoming = kFALSE;
         interacting = kTRUE;
      } else if (interacting && distance > fWilcke.GetRint() + 10.) {
         interacting = kFALSE;
         outgoing = kTRUE;
      }
      if (tcon < 0 && distance < fWilcke.GetRint()) tcon = t;
      if (tcon > 0 && tsep < 0) {
         if (t > tcon && distance > fWilcke.GetRint()) tsep = t;
      }
   }
   // friction force
   if (alpha > 0) {
      if (fInitialDeriv) {
         // draw random number with mean=0 and sigma=1
         randomX = gRandom->Gaus();
      }
      if (distance < fWilcke.GetRint()) {
         Double_t vrel = dv(vp, vt);
         Double_t vv_x = dv_x(vp, vt);
         Double_t vv_y = dv_y(vp, vt);
         Double_t vpar = vrel * (vv_x * uv_x + vv_y * uv_y);
         Double_t FRICTION = -alpha * vpar;
         Double_t fforce = FRICTION * randomX + FRICTION;
         fp = fforce / (fWilcke.GetAP() * KVNucleus::kAMU);
         dvp[0] += fp * (uv_x);
         dvp[1] += fp * (uv_y);
         ft = -fforce / (fWilcke.GetAT() * KVNucleus::kAMU);
         dvt[0] += ft * (uv_x);
         dvt[1] += ft * (uv_y);
      }
   }
}

void ClassTraj::Run(Double_t tmin, Double_t tmax, Double_t dt)
{
   // Perform the simulation from time 'tmin' to 'tmax'.

   Double_t Time = tmin;
   tcon = tsep = -1.;
   incoming = kTRUE;
   interacting = kFALSE;
   outgoing = kFALSE;
   while (Time < tmax) {
      //cout << "ETOT = " << TotalEnergy() << endl;
      // take Runge-Kutta step from Time to Time+dt

      if (outgoing) {
         dt *= 1.5;
         if (Time + dt > tmax) {
            dt = tmax - Time;
         }
      }
      Integrate(coords, Time, Time + dt, dt);

      Time += dt;
      if (fVisualization) {
         UpdateInfos(ImpactParameter, Time);
         UpdateARCPositions(Time);
         CMPositionPad->Modified();
         CMPositionPad->Update();
         LabPositionPad->Modified();
         LabPositionPad->Update();
      }
   }

   if (fVisualization) {
      ProjTRAJ->SetLineWidth(1);
      ProjTRAJlab->SetLineWidth(1);
      ProjTRAJ->SetLineStyle(2);
      ProjTRAJlab->SetLineStyle(2);
   }
}

Double_t ClassTraj::TotalEnergy() const
{
   Double_t ekin = 0.5 * KVNucleus::u() * (
                      fWilcke.GetAP() * (VP[0] * VP[0] + VP[1] * VP[1])
                      + fWilcke.GetAT() * (VT[0] * VT[0] + VT[1] * VT[1]));
   Double_t epot = fWilcke.GetTotalPotential()->Eval(dr(RP, RT));
   return ekin + epot;
}

void ClassTraj::CalcTrajectories(Double_t e, Double_t bmin, Double_t bmax, Double_t db, Double_t tmin, Double_t tmax, Double_t dt, Double_t friction)
{
   alpha = friction;
   InitTree(e);

   for (Double_t b = bmin; b <= bmax; b += db) {

      DoOneTraj(tmin, friction, b, e, tmax, dt);
   }
   theFile->Write();
   theFile->Close();
}

void ClassTraj::InitTree(Double_t e)
{
   TString simTitle = Form("%s %f MeV/u (alpha=%f)", sysName.Data(), e, alpha);
   TString filename = Form("ClassTraj_%s_%fAMeV_alpha%f.root", colName.Data(), e, alpha);
   theFile = new TFile(filename, "recreate");
   theTree = new TTree("ClassTrajSim", simTitle);
   theTree->Branch("ImpactParameter", &ImpactParameter);
   theTree->Branch("AngMom", &AngMom);
   theTree->Branch("TKEL", &TKEL);
   theTree->Branch("TKE", &TKE);
   theTree->Branch("VPOT", &VPOT);
   theTree->Branch("minDist", &minDist);
   theTree->Branch("ThetaProjLab", &ThetaProjLab);
   theTree->Branch("absThetaProjLab", &absThetaProjLab);
   theTree->Branch("EProjLab", &EProjLab);
   theTree->Branch("fused", &fused);
   theTree->Branch("inelastic", &inelastic);
   theTree->Branch("RotationTime", &RotationTime);
   theTree->Branch("RotationAngle", &RotationAngle);
   theTree->Branch("tcon", &tcon);
   theTree->Branch("tsep", &tsep);
}

void ClassTraj::DoOneTraj(Double_t tmin, Double_t friction, Double_t b, Double_t e, Double_t tmax, Double_t dt)
{
   minDist = 500;
   InitTrajectory(b, e, friction);

   ImpactParameter = b;
   AngMom = fWilcke.k(e) * b;

   Run(tmin, tmax, dt);

   ThetaProjLab = ThetaLabProj();
   EProjLab = ELabProj();
   absThetaProjLab = abs(ThetaProjLab);
   TKE = TotalEnergy();
   TKEL = fWilcke.ECM(ELAB) - TKE;
   RotationTime = (tsep > 0 ? tsep - tcon : tmax - tcon);
   VPOT = fWilcke.GetTotalPotential()->Eval(dr(RP, RT));
   inelastic = fused = kFALSE;

   if (dr(RP, RT) < fWilcke.GetRint()) fused = kTRUE;
   if (TKEL > 1.e-4 && !fused) inelastic = kTRUE;

   //cout << "Trot="<<RotationTime<<"fm/c L="<<AngMom<<"hbar Rmin="<<minDist<<"fm"<<endl;
   Double_t Period = 2 * TMath::Pi() * fWilcke.GetMu() * KVNucleus::kAMU * minDist * minDist / (AngMom * KVNucleus::hbar);
   RotationAngle = 2 * TMath::Pi() * RotationTime / Period * TMath::RadToDeg();
   //cout<<"Period = "<<Period<<"fm/c  --  Angle turned = "<<RotationAngle<<"deg."<<endl;

   theTree->Fill();
}

void ClassTraj::CalcTrajectories(Double_t e, Int_t ntraj, Double_t bmin, Double_t bmax, Double_t tmin, Double_t tmax, Double_t dt, Double_t friction)
{
   alpha = friction;
   InitTree(e);

   while (ntraj--) {

      if (!(ntraj % 5000)) cout << ntraj << " more to go..." << endl;

      Double_t b = sqrt(gRandom->Uniform(bmin * bmin, bmax * bmax));
      DoOneTraj(tmin, friction, b, e, tmax, dt);
   }
   theFile->Write();
//    theFile->Close();
   theTree->StartViewer();
}
