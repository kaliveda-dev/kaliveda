//Created by KVClassFactory on Thu Jun 24 14:22:17 2010
//Author: John Frankland,,,

#ifndef __ClassTraj_H
#define __ClassTraj_H

#include "KVRungeKutta.h"
#include "KVWilckeReactionParameters.h"

#include "TArc.h"
#include "TGraph.h"
#include "TCanvas.h"
#include "TMarker.h"
#include "TTree.h"
#include "TFile.h"
#include "TLatex.h"

class ClassTraj : public KVRungeKutta {
   Double_t* coords;
   Double_t* RP;
   Double_t* RT;
   Double_t* VP;
   Double_t* VT;
   Double_t RCM[2];//! position of centre of mass
   Double_t VCM[2];//! velocity of centre of mass

   KVWilckeReactionParameters fWilcke;
   Double_t ELAB, E0;
   Double_t alpha;//friction parameter

   //CM trajectory
   TArc* ProjARC;
   TArc* TargARC;
   TGraph* ProjTRAJ;
   Int_t nTRAJPoints;
   TCanvas* CMPositionPad;
   TLatex* infox;
   //lab trajectory
   TArc* ProjARClab;
   TArc* TargARClab;
   TGraph* ProjTRAJlab;
   TCanvas* LabPositionPad;
   Int_t nTRAJPointslab;
   Double_t minDist;
   Double_t EtotDeb, EtotFin, TKEL, TKE, VPOT;
   Double_t ThetaProjLab, absThetaProjLab;
   Double_t EProjLab;
   Double_t ImpactParameter;
   Double_t AngMom, RotationAngle;
   Bool_t fused, inelastic, incoming, interacting, outgoing;
   Double_t tcon, tsep, RotationTime;
   Double_t randomX;

   TString sysName;
   TString colName;

   TFile* theFile;
   TTree* theTree;

   Bool_t fVisualization;//true = show trajectories

public:
   ClassTraj(const KVNucleus& proj, const KVNucleus& targ, Bool_t viz = kTRUE);
   virtual ~ClassTraj();

   void SetViz(Bool_t on = kTRUE)
   {
      fVisualization = on;
   }

   void InitTrajectory(Double_t b, Double_t e, Double_t a = 0);

   Double_t dr(Double_t* rp, Double_t* rt) const
   {
      return sqrt(pow(rp[0] - rt[0], 2) + pow(rp[1] - rt[1], 2));
   }
   Double_t dv(Double_t* vp, Double_t* vt) const
   {
      return sqrt(pow(vp[0] - vt[0], 2) + pow(vp[1] - vt[1], 2));
   }
   Double_t dr_x(Double_t* rp, Double_t* rt) const
   {
      // x-component of unit vector from target to projectile
      return (rp[0] - rt[0]) / dr(rp, rt);
   }
   Double_t dr_y(Double_t* rp, Double_t* rt) const
   {
      // y-component of unit vector from target to projectile
      return (rp[1] - rt[1]) / dr(rp, rt);
   }
   Double_t dv_x(Double_t* vp, Double_t* vt) const
   {
      // x-component of unit vector from target to projectile
      return (vp[0] - vt[0]) / dv(vp, vt);
   }
   Double_t dv_y(Double_t* vp, Double_t* vt) const
   {
      // y-component of unit vector from target to projectile
      return (vp[1] - vt[1]) / dv(vp, vt);
   }
   void UpdateARCPositions(Double_t time);
   Double_t ThetaLabProj()
   {
      return TMath::Sign(TMath::RadToDeg() * acos(
                            (VP[0] + VCM[0]) /
                            sqrt(pow(VP[0] + VCM[0], 2) + pow(VP[1] + VCM[1], 2))), VP[1] + VCM[1]);
   };
   Double_t ELabProj()
   {
      return 0.5 * fWilcke.GetAP() * KVNucleus::u() * (pow(VP[0] + VCM[0], 2) + pow(VP[1] + VCM[1], 2));
   };
   Double_t TotalAngularMomentum()
   {
      Double_t lz = dv_x(VP, VT) * dr_y(RP, RT) - dv_y(VP, VT) * dr_x(RP, RT);
      lz *= fWilcke.GetMu() * KVNucleus::u() * dr(RP, RT) * dv(VP, VT) / KVNucleus::hbar;
      return lz;
   }
   Double_t TotalEnergy() const;

   virtual void CalcDerivs(Double_t /*X*/, Double_t* /*Y*/, Double_t* /*DY/DX*/);
   void Run(Double_t tmin, Double_t tmax, Double_t dt);
   void Reset()
   {
      // Reset visualization ready for new trajectories calculation

      if (CMPositionPad) delete CMPositionPad;
      if (LabPositionPad) delete LabPositionPad;

      if (!ProjARC) {
         ProjARC = new TArc(0, 0, fWilcke.GetCP());
         ProjARC->SetFillColor(kBlue);
      }
      if (!TargARC) {
         TargARC = new TArc(0, 0, fWilcke.GetCT());
         TargARC->SetFillColor(kRed);
      }
      if (!ProjARClab) {
         ProjARClab = new TArc(0, 0, 2 * fWilcke.GetCP());
         ProjARClab->SetFillColor(kBlue);
      }
      if (!TargARClab) {
         TargARClab = new TArc(0, 0, 2 * fWilcke.GetCT());
         TargARClab->SetFillColor(kRed);
      }
      CMPositionPad = new TCanvas("CMPOS", "CM Positions", 800, 500);
      CMPositionPad->DrawFrame(-50, -40, 50, 40);
      TMarker* cmark = new TMarker(0, 0, 2);
      cmark->SetMarkerSize(2);
      cmark->Draw();
      TArc* rint = new TArc(0, 0, fWilcke.GetRint());
      rint->SetLineStyle(2);
      rint->SetFillStyle(0);
      rint->Draw();
      if (infox) delete infox;
      infox = new TLatex;
      UpdateInfos(0, 0);
      infox->Draw();
      LabPositionPad = new TCanvas("LABPOS", "Lab Positions", 800, 500);
      LabPositionPad->DrawFrame(-50, -300, 550, 300); //(-100,-80, 100,80);
      cmark = new TMarker(0, 0, 2);
      cmark->SetMarkerSize(2);
      cmark->Draw();
   }
   void UpdateInfos(Double_t b, Double_t time)
   {
      if (infox) {
         if (outgoing)infox->SetText(-40, -30, Form("b=%.1ffm t=%.0ffm/c OUTGOING", b, time));
         else if (incoming)infox->SetText(-40, -30, Form("b=%.1ffm t=%.0ffm/c INCOMING", b, time));
         else if (interacting)infox->SetText(-40, -30, Form("b=%.1ffm t=%.0ffm/c INTERACTING", b, time));
         else infox->SetText(-40, -30, Form("b=%.1ffm t=%.0ffm/c", b, time));
      }
   }
   void CalcTrajectories(Double_t e, Double_t bmin, Double_t bmax, Double_t db, Double_t tmin, Double_t tmax, Double_t dt, Double_t friction = 0);
   void CalcTrajectories(Double_t e, Int_t ntraj, Double_t bmin, Double_t bmax, Double_t tmin, Double_t tmax, Double_t dt, Double_t friction = 0);

   void InitTree(Double_t e);
   void DoOneTraj(Double_t tmin, Double_t friction, Double_t b, Double_t e, Double_t tmax, Double_t dt);
   ClassDef(ClassTraj, 0) //Classical trajectory for heavy ion collisions
};

#endif
