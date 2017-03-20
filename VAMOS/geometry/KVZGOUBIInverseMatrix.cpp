//Created by KVClassFactory on Tue Feb 14 10:53:57 2017
//Author: Patrick St-Onge,,,

#include "KVZGOUBIInverseMatrix.h"

ClassImp(KVZGOUBIInverseMatrix)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVZGOUBIInverseMatrix</h2>
<h4>Class used to reconstruct trajectories in VAMOS</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVZGOUBIInverseMatrix::KVZGOUBIInverseMatrix()
   : KVBase()
{
   // Default constructor
   nbstep_xf = 30;
   nbstep_yf = 30;
   nbstep_thetaf = 30;
   nbstep_phif = 30;
   characteristicdistance_xf = 1.;
   characteristicdistance_yf = 1.;
   characteristicdistance_thetaf = 1.;
   characteristicdistance_phif = 1.;
}

//____________________________________________________________________________//

KVZGOUBIInverseMatrix::KVZGOUBIInverseMatrix(Int_t nbstepxft, Int_t nbstepthetaft, Int_t nbstepyft, Int_t nbstepphift, TChain* ch)
   : KVBase()
{
   // Default constructor
   nbstep_xf = nbstepxft;
   nbstep_yf = nbstepyft;
   nbstep_thetaf = nbstepthetaft;
   nbstep_phif = nbstepphift;
   SetZGOUBIDatabase(ch);
   FindExtremum();
   if (yfmin == 0) {
      yfmin = -1000;
      yfmax = 1000;
      phifmin = -1000;
      phifmax = 1000;
   }
   FindDeltaParameters();
   PrintExtremum();
   initZGOUBIInversePositionDatabase();
   SetZGOUBIInversePositionDatabase();
}

//____________________________________________________________________________//

KVZGOUBIInverseMatrix::KVZGOUBIInverseMatrix(const KVZGOUBIInverseMatrix& obj) : KVBase()
{
   // Copy constructor
   // Use this constructor to copy an existing object
   obj.Copy(*this);
}

//____________________________________________________________________________//

KVZGOUBIInverseMatrix::KVZGOUBIInverseMatrix(const Char_t* name, const Char_t* title)
   : KVBase(name, title)
{
   // Constructor inherited from KVBase
}

//____________________________________________________________________________//

KVZGOUBIInverseMatrix::~KVZGOUBIInverseMatrix()
{

}

//____________________________________________________________________________//

void KVZGOUBIInverseMatrix::Copy(TObject& obj) const
{
   // This method copies the current state of 'this' KVZGOUBIInversePosition
   // object into 'obj'.

   KVBase::Copy(obj);
   KVZGOUBIInverseMatrix& CastedObj = (KVZGOUBIInverseMatrix&)obj;
   CastedObj.ZGOUBIDatabase = ZGOUBIDatabase;
   CastedObj.ZGOUBIInversePositionDatabase = ZGOUBIInversePositionDatabase ;
   CastedObj.xfmin = xfmin ;
   CastedObj.xfmax = xfmax ;
   CastedObj.yfmin = yfmin ;
   CastedObj.yfmax = yfmax ;
   CastedObj.thetafmin = thetafmin ;
   CastedObj.thetafmax = thetafmax ;
   CastedObj.phifmin = phifmin ;
   CastedObj.phifmax = phifmax ;
   CastedObj.delta_xf = delta_xf ;
   CastedObj.delta_yf = delta_yf ;
   CastedObj.delta_thetaf = delta_thetaf ;
   CastedObj.delta_phif = delta_phif ;
   CastedObj.nbstep_xf = nbstep_xf;
   CastedObj.nbstep_yf = nbstep_yf ;
   CastedObj.nbstep_thetaf = nbstep_thetaf ;
   CastedObj.nbstep_phif = nbstep_phif ;
   CastedObj.characteristicdistance_xf = characteristicdistance_xf;
   CastedObj.characteristicdistance_yf = characteristicdistance_yf;
   CastedObj.characteristicdistance_thetaf = characteristicdistance_thetaf;
   CastedObj.characteristicdistance_phif = characteristicdistance_phif ;

}
//____________________________________________________________________________//

void KVZGOUBIInverseMatrix::SetZGOUBIDatabase(TChain* ch)
{
   Float_t ThetaVt, PhiVt, Deltat, XFt, ThetaFt, YFt, PhiFt, Patht;
   //enable reading of all branch
   ch->SetBranchStatus("*", 1);
   ch->SetBranchAddress("ThetaV", &ThetaVt);
   ch->SetBranchAddress("PhiV", &PhiVt);
   ch->SetBranchAddress("Delta", &Deltat);
   ch->SetBranchAddress("XF", &XFt);
   ch->SetBranchAddress("ThetaF", &ThetaFt);
   ch->SetBranchAddress("YF", &YFt);
   ch->SetBranchAddress("PhiF", &PhiFt);
   ch->SetBranchAddress("Path", &Patht);
   int Database_size = (int) ch->GetEntries();
   for (int i = 0; i < Database_size; i++) {
      ch->GetEntry(i);
      AddZGOUBITrajectory(ThetaVt, PhiVt, Deltat, XFt, ThetaFt, YFt, PhiFt, Patht);
   }
}


void KVZGOUBIInverseMatrix::AddZGOUBITrajectory(Float_t ThetaVt, Float_t PhiVt, Float_t Deltat, Float_t XFt, Float_t ThetaFt, Float_t YFt, Float_t PhiFt, Float_t Patht)
{
   KVZGOUBITrajectory atemp(ThetaVt, PhiVt, Deltat, XFt, ThetaFt, YFt, PhiFt, Patht);
   ZGOUBIDatabase.push_back(atemp);
}

void KVZGOUBIInverseMatrix::FindExtremum()
{
   Float_t xftempmax = 0;
   Float_t xftempmin = 0;
   Float_t yftempmax = 0;
   Float_t yftempmin = 0;
   Float_t thetaftempmax = 0;
   Float_t thetaftempmin = 0;
   Float_t phiftempmax = 0;
   Float_t phiftempmin = 0;


   for (int i = 0; i < (int) ZGOUBIDatabase.size(); i++) {
      if (ZGOUBIDatabase[i].GetXF() > xftempmax) {
         xftempmax = ZGOUBIDatabase[i].GetXF();
      }
      if (ZGOUBIDatabase[i].GetXF() < xftempmin) {
         xftempmin = ZGOUBIDatabase[i].GetXF();
      }

      if (ZGOUBIDatabase[i].GetYF() > yftempmax) {
         yftempmax = ZGOUBIDatabase[i].GetYF();
      }
      if (ZGOUBIDatabase[i].GetYF() < yftempmin) {
         yftempmin = ZGOUBIDatabase[i].GetYF();
      }

      if (ZGOUBIDatabase[i].GetThetaF() > thetaftempmax) {
         thetaftempmax = ZGOUBIDatabase[i].GetThetaF();
      }
      if (ZGOUBIDatabase[i].GetThetaF() < thetaftempmin) {
         thetaftempmin = ZGOUBIDatabase[i].GetThetaF();
      }

      if (ZGOUBIDatabase[i].GetPhiF() > phiftempmax) {
         phiftempmax = ZGOUBIDatabase[i].GetPhiF();
      }
      if (ZGOUBIDatabase[i].GetPhiF() < phiftempmin) {
         phiftempmin = ZGOUBIDatabase[i].GetPhiF();
      }

   }

   xfmin = (Float_t) std::floor(xftempmin * 1.1);
   xfmax = (Float_t) std::ceil(xftempmax * 1.1);
   yfmin = (Float_t) std::floor(yftempmin * 1.1);
   yfmax = (Float_t) std::ceil(yftempmax * 1.1);
   thetafmin = (Float_t) std::floor(thetaftempmin * 1.1);
   thetafmax = (Float_t) std::ceil(thetaftempmax * 1.1);
   phifmin = (Float_t) std::floor(phiftempmin * 1.1);
   phifmax = (Float_t) std::ceil(phiftempmax * 1.1);
}

void KVZGOUBIInverseMatrix::PrintExtremum()
{
   std::cout << "XF" << xfmin << " " << xfmax << " " << delta_xf << " " << nbstep_xf << std::endl;
   std::cout << "YF" << yfmin << " " << yfmax << " " << delta_yf << " " << nbstep_yf << std::endl;
   std::cout << "ThetaF" << thetafmin << " " << thetafmax << " " << delta_thetaf << " " << nbstep_thetaf << std::endl;
   std::cout << "PhiF" << phifmin << " " << phifmax << " " << delta_phif << " " << nbstep_phif << std::endl;
}

void KVZGOUBIInverseMatrix::FindDeltaParameters()
{
   delta_xf = (xfmax - xfmin) / nbstep_xf;
   delta_yf = (yfmax - yfmin) / nbstep_yf;
   delta_thetaf = (thetafmax - thetafmin) / nbstep_thetaf;
   delta_phif = (phifmax - phifmin) / nbstep_phif;
}

void KVZGOUBIInverseMatrix::SetNbSteps(Int_t nbstepxft, Int_t nbstepthetaft, Int_t nbstepyft, Int_t nbstepphift)
{
   nbstep_xf = nbstepxft;
   nbstep_yf = nbstepyft;
   nbstep_thetaf = nbstepthetaft;
   nbstep_phif = nbstepphift;
}

void KVZGOUBIInverseMatrix::AddZGOUBIInversePosition()
{
   KVZGOUBIInversePosition atemp;
   ZGOUBIInversePositionDatabase.push_back(atemp);
}


void KVZGOUBIInverseMatrix::initZGOUBIInversePositionDatabase()
{
   for (int i = 0; i < nbstep_xf * nbstep_yf * nbstep_thetaf * nbstep_phif; i++) {
      AddZGOUBIInversePosition();
   }
}

void KVZGOUBIInverseMatrix::SetZGOUBIInversePositionDatabase()
{
   double xftemp;
   double yftemp;
   double phiftemp;
   double thetaftemp;

   Int_t ZGOUBIDatabase_position;
   for (int i = 0; i < (int) ZGOUBIDatabase.size(); i++) {
      xftemp = ZGOUBIDatabase[i].GetXF();
      yftemp = ZGOUBIDatabase[i].GetYF();
      thetaftemp = ZGOUBIDatabase[i].GetThetaF();
      phiftemp = ZGOUBIDatabase[i].GetPhiF();

      ZGOUBIDatabase_position = GetZGOUBIDatabase_position(xftemp, thetaftemp, yftemp, phiftemp);
      if (ZGOUBIDatabase_position > (int) ZGOUBIInversePositionDatabase.size()) {
         std::cout << "Problem acceptvectorposition size" << std::endl;
      } else {
         ZGOUBIInversePositionDatabase[ZGOUBIDatabase_position].AddtoInversePositionVector(i);
      }
   }
}


Int_t KVZGOUBIInverseMatrix::GetZGOUBIDatabase_position(Float_t XFt, Float_t ThetaFt, Float_t YFt, Float_t PhiFt)
{
   Int_t ZGOUBIDatabase_position = -1;
   Int_t positionxf;
   Int_t positionyf;
   Int_t positionthetaf;
   Int_t positionphif;


   if (XFt <= xfmax && XFt >= xfmin && YFt <= yfmax && YFt >= yfmin && ThetaFt <= thetafmax && ThetaFt >= thetafmin && PhiFt <= phifmax && PhiFt >= phifmin) {
      positionxf = (XFt - xfmin) / delta_xf;
      positionyf = (YFt - yfmin) / delta_yf;
      positionthetaf = (ThetaFt - thetafmin) / delta_thetaf;
      positionphif = (PhiFt - phifmin) / delta_phif;

      ZGOUBIDatabase_position = positionxf * nbstep_yf * nbstep_thetaf * nbstep_phif + positionyf * nbstep_thetaf * nbstep_phif + positionthetaf * nbstep_phif + positionphif;
      return ZGOUBIDatabase_position;
   } else {
      return -1;
   }
}

std::vector<Int_t> KVZGOUBIInverseMatrix::GetClosest4DVoxels(Float_t XFt, Float_t ThetaFt, Float_t YFt, Float_t PhiFt, Int_t nbneighbors)
{
   if (nbneighbors < 0) {
      nbneighbors = 0;
   }
   if (nbneighbors > 100) {
      nbneighbors = 100;
   }

   Int_t positionxf;
   Int_t positionyf;
   Int_t positionthetaf;
   Int_t positionphif;
   Int_t positionxfmin;
   Int_t positionyfmin;
   Int_t positionthetafmin;
   Int_t positionphifmin;
   Int_t positionxfmax;
   Int_t positionyfmax;
   Int_t positionthetafmax;
   Int_t positionphifmax;
   std::vector<Int_t> Closest4DVoxelsvector;

   if (XFt <= xfmax && XFt >= xfmin && YFt <= yfmax && YFt >= yfmin && ThetaFt <= thetafmax && ThetaFt >= thetafmin && PhiFt <= phifmax && PhiFt >= phifmin) {
      positionxf = (XFt - xfmin) / delta_xf;
      positionxfmin = positionxf - nbneighbors;
      if (positionxfmin < 0) {
         positionxfmin = positionxf;
      }
      positionxfmax = positionxf + nbneighbors;
      if (positionxfmax >= nbstep_xf) {
         positionxfmax = positionxf;
      }

      positionyf = (YFt - yfmin) / delta_yf;
      positionyfmin = positionyf - nbneighbors;
      if (positionyfmin < 0) {
         positionyfmin = positionyf;
      }
      positionyfmax = positionyf + nbneighbors;
      if (positionyfmax >= nbstep_yf) {
         positionyfmax = positionyf;
      }

      positionthetaf = (ThetaFt - thetafmin) / delta_thetaf;
      positionthetafmin = positionthetaf - nbneighbors;
      if (positionthetafmin < 0) {
         positionthetafmin = positionthetaf;
      }
      positionthetafmax = positionthetaf + nbneighbors;
      if (positionthetafmax >= nbstep_thetaf) {
         positionthetafmax = positionthetaf;
      }

      positionphif = (PhiFt - phifmin) / delta_phif;
      positionphifmin = positionphif - nbneighbors;
      if (positionphifmin < 0) {
         positionphifmin = positionphif;
      }
      positionphifmax = positionphif + nbneighbors;
      if (positionphifmax >= nbstep_phif) {
         positionphifmax = positionphif;
      }
      for (int positionxftemp = positionxfmin; positionxftemp <= positionxfmax; positionxftemp++) {
         for (int positionyftemp = positionyfmin; positionyftemp <= positionyfmax; positionyftemp++) {
            for (int positionthetaftemp = positionthetafmin; positionthetaftemp <= positionthetafmax; positionthetaftemp++) {
               for (int positionphiftemp = positionphifmin; positionphiftemp <= positionphifmax; positionphiftemp++) {
                  Closest4DVoxelsvector.push_back(positionxftemp * nbstep_yf * nbstep_thetaf * nbstep_phif + positionyftemp * nbstep_thetaf * nbstep_phif + positionthetaftemp * nbstep_phif + positionphiftemp);
               }
            }
         }
      }
      return Closest4DVoxelsvector;
   } else {
      return Closest4DVoxelsvector;
   }
}



Float_t KVZGOUBIInverseMatrix::GetDistance(Float_t xf, Float_t thetaf, Float_t yf, Float_t phif, Float_t xf_line, Float_t thetaf_line, Float_t yf_line, Float_t phif_line)
{
   Float_t distance = 0;
   Float_t distancexf = sqrt(pow(xf - xf_line, 2)) / characteristicdistance_xf;
   Float_t distanceyf = sqrt(pow(yf - yf_line, 2)) / characteristicdistance_yf;
   Float_t distancethetaf = sqrt(pow(thetaf - thetaf_line, 2)) / characteristicdistance_thetaf;
   Float_t distancephif = sqrt(pow(phif - phif_line, 2)) / characteristicdistance_phif;
   distance = sqrt(pow(distancexf, 2) + pow(distanceyf, 2) + pow(distancethetaf, 2) + pow(distancephif, 2));
   return distance;
}

Float_t KVZGOUBIInverseMatrix::GetDistance(Float_t xf, Float_t thetaf, Float_t yf, Float_t phif, Int_t linenb)
{
   Float_t distance = 0;
   Float_t distancexf = sqrt(pow(xf - ZGOUBIDatabase[linenb].GetXF(), 2)) / characteristicdistance_xf;
   Float_t distanceyf = sqrt(pow(yf - ZGOUBIDatabase[linenb].GetYF(), 2)) / characteristicdistance_yf;
   Float_t distancethetaf = sqrt(pow(thetaf - ZGOUBIDatabase[linenb].GetThetaF(), 2)) / characteristicdistance_thetaf;
   Float_t distancephif = sqrt(pow(phif - ZGOUBIDatabase[linenb].GetPhiF(), 2)) / characteristicdistance_phif;
   distance = sqrt(pow(distancexf, 2) + pow(distanceyf, 2) + pow(distancethetaf, 2) + pow(distancephif, 2));
   return distance;
}

Float_t KVZGOUBIInverseMatrix::GetDistance(Int_t linenb1, Int_t linenb2)
{
   Float_t distance = 0;
   Float_t distancexf = sqrt(pow(ZGOUBIDatabase[linenb1].GetXF() - ZGOUBIDatabase[linenb2].GetXF(), 2)) / characteristicdistance_xf;
   Float_t distanceyf = sqrt(pow(ZGOUBIDatabase[linenb1].GetYF() - ZGOUBIDatabase[linenb2].GetYF(), 2)) / characteristicdistance_yf;
   Float_t distancethetaf = sqrt(pow(ZGOUBIDatabase[linenb1].GetThetaF() - ZGOUBIDatabase[linenb2].GetThetaF(), 2)) / characteristicdistance_thetaf;
   Float_t distancephif = sqrt(pow(ZGOUBIDatabase[linenb1].GetPhiF() - ZGOUBIDatabase[linenb2].GetPhiF(), 2)) / characteristicdistance_phif;
   distance = sqrt(pow(distancexf, 2) + pow(distanceyf, 2) + pow(distancethetaf, 2) + pow(distancephif, 2));
   return distance;
}


Int_t KVZGOUBIInverseMatrix::GetNearestLinenb(Float_t XFt, Float_t ThetaFt, Float_t YFt, Float_t PhiFt)
{
   Int_t Maxnbneighbors = 100;
   std::vector<Int_t> Closest4DVoxelsvector;
   Int_t MultZGOUBIInversePosition;
   Int_t ZGOUBITrajectoryposition;
   Int_t ZGOUBIInversePositionDatabaseposition;
   Float_t xf_line;
   Float_t yf_line;
   Float_t thetaf_line;
   Float_t phif_line;
   Float_t distance = 1;
   Float_t distancemin = 10000000000000.;
   Int_t  ZGOUBIDatabase_positionmin = -1;
   Bool_t ZGOUBITrajectoryAvailable = false;
   Int_t nbneighbors = 1;
   while (ZGOUBITrajectoryAvailable == false && nbneighbors <= Maxnbneighbors) {
      Closest4DVoxelsvector = GetClosest4DVoxels(XFt, ThetaFt, YFt, PhiFt, nbneighbors);
      if (Closest4DVoxelsvector.size() <= 0) {
         // std::cout<<"Value outside full range"<<std::endl;
      } else {
         for (int i = 0; i < (int) Closest4DVoxelsvector.size(); i++) {
            ZGOUBIInversePositionDatabaseposition = Closest4DVoxelsvector[i];
            MultZGOUBIInversePosition = ZGOUBIInversePositionDatabase[ZGOUBIInversePositionDatabaseposition].GetInversePositionVectorSize();
            if (MultZGOUBIInversePosition > 0) {
               ZGOUBITrajectoryAvailable = true;
               for (int j = 0; j < MultZGOUBIInversePosition; j++) {
                  ZGOUBITrajectoryposition = ZGOUBIInversePositionDatabase[ZGOUBIInversePositionDatabaseposition].GetInversePositionVectorValue(j);
                  xf_line = ZGOUBIDatabase[ZGOUBITrajectoryposition].GetXF();
                  yf_line = ZGOUBIDatabase[ZGOUBITrajectoryposition].GetYF();
                  thetaf_line = ZGOUBIDatabase[ZGOUBITrajectoryposition].GetThetaF();
                  phif_line = ZGOUBIDatabase[ZGOUBITrajectoryposition].GetPhiF();
                  distance = GetDistance(XFt, ThetaFt, YFt, PhiFt, xf_line, thetaf_line, yf_line, phif_line);
                  if (distance < distancemin) {
                     distancemin = distance;
                     ZGOUBIDatabase_positionmin = ZGOUBITrajectoryposition;
                  }
               }
            }
         }
      }
      nbneighbors++;
   }
   return  ZGOUBIDatabase_positionmin;
}

std::vector<Int_t> KVZGOUBIInverseMatrix::GetNearestLinenbs(Float_t XFt, Float_t ThetaFt, Float_t YFt, Float_t PhiFt, Int_t nblines)
{
   Int_t Maxnbneighbors = 100;
   std::vector<Int_t> Closest4DVoxelsvector;
   Int_t MultZGOUBIInversePosition;
   Int_t ZGOUBITrajectoryposition;
   Int_t ZGOUBIInversePositionDatabaseposition;
   Float_t xf_line;
   Float_t yf_line;
   Float_t thetaf_line;
   Float_t phif_line;
   Float_t distance = 1;
   Bool_t ZGOUBITrajectoryAvailable = false;
   Int_t nbneighbors = 1;

   std::vector<Int_t> ZGOUBIDatabase_positionmin;
   std::vector<Float_t> ZGOUBIDatabase_positionmin_distance;
   Int_t positionmax = 0;
   Float_t distancemax = 0;
   std::vector<Float_t>::iterator result_max_element;
   if (XFt > xfmin && XFt < xfmax && YFt > yfmin && YFt < yfmax && ThetaFt > thetafmin && ThetaFt < thetafmax && PhiFt > phifmin && PhiFt < phifmax) {
      while (ZGOUBITrajectoryAvailable == false && nbneighbors <= Maxnbneighbors) {
         Closest4DVoxelsvector = GetClosest4DVoxels(XFt, ThetaFt, YFt, PhiFt, nbneighbors);
         if (Closest4DVoxelsvector.size() <= 0) {
            // std::cout<<"Value outside full range"<<std::endl;
         } else {
            for (int i = 0; i < (int) Closest4DVoxelsvector.size(); i++) {
               ZGOUBIInversePositionDatabaseposition = Closest4DVoxelsvector[i];
               MultZGOUBIInversePosition = ZGOUBIInversePositionDatabase[ZGOUBIInversePositionDatabaseposition].GetInversePositionVectorSize();
               if (MultZGOUBIInversePosition > 0) {
                  ZGOUBITrajectoryAvailable = true;
                  for (int j = 0; j < MultZGOUBIInversePosition; j++) {
                     ZGOUBITrajectoryposition = ZGOUBIInversePositionDatabase[ZGOUBIInversePositionDatabaseposition].GetInversePositionVectorValue(j);
                     xf_line = ZGOUBIDatabase[ZGOUBITrajectoryposition].GetXF();
                     yf_line = ZGOUBIDatabase[ZGOUBITrajectoryposition].GetYF();
                     thetaf_line = ZGOUBIDatabase[ZGOUBITrajectoryposition].GetThetaF();
                     phif_line = ZGOUBIDatabase[ZGOUBITrajectoryposition].GetPhiF();
                     distance = GetDistance(XFt, ThetaFt, YFt, PhiFt, xf_line, thetaf_line, yf_line, phif_line);
                     if ((int) ZGOUBIDatabase_positionmin.size() < nblines) {
                        ZGOUBIDatabase_positionmin.push_back(ZGOUBITrajectoryposition);
                        ZGOUBIDatabase_positionmin_distance.push_back(distance);
                        result_max_element = std::max_element(ZGOUBIDatabase_positionmin_distance.begin(), ZGOUBIDatabase_positionmin_distance.end());
                        positionmax = std::distance(ZGOUBIDatabase_positionmin_distance.begin(), result_max_element);
                        distancemax = ZGOUBIDatabase_positionmin_distance[positionmax];
                     } else if (distance < distancemax) {
                        ZGOUBIDatabase_positionmin[positionmax] = ZGOUBITrajectoryposition;
                        ZGOUBIDatabase_positionmin_distance[positionmax] = distance;
                        result_max_element = std::max_element(ZGOUBIDatabase_positionmin_distance.begin(), ZGOUBIDatabase_positionmin_distance.end());
                        positionmax = std::distance(ZGOUBIDatabase_positionmin_distance.begin(), result_max_element);
                        distancemax = ZGOUBIDatabase_positionmin_distance[positionmax];
                     }
                  }
               }
            }
         }
         nbneighbors++;
      }
   }
   return  ZGOUBIDatabase_positionmin;
}



std::vector<Int_t> KVZGOUBIInverseMatrix::GetLinesinRadius(Float_t XFt, Float_t ThetaFt, Float_t YFt, Float_t PhiFt, Float_t radius)
{
   std::vector<Int_t> vectorLinesinRadius;
   Int_t nbxf = (Int_t) ceil(radius / delta_xf);
   Int_t nbyf = (Int_t) ceil(radius / delta_yf);
   Int_t nbthetaf = (Int_t) ceil(radius / delta_thetaf);
   Int_t nbphif = (Int_t) ceil(radius / delta_phif);

   Int_t nbmax = 0;
   if (nbxf > nbmax) nbmax = nbxf;
   if (nbyf > nbmax) nbmax = nbyf;
   if (nbthetaf > nbmax) nbmax = nbthetaf;
   if (nbphif > nbmax) nbmax = nbphif;

   //nbmax++;

   std::vector<Int_t> Closest4DVoxelsvector;
   Int_t MultZGOUBIInversePosition;
   Int_t ZGOUBITrajectoryposition;
   Int_t ZGOUBIInversePositionDatabaseposition;
   Float_t distance = 1;
   Closest4DVoxelsvector = GetClosest4DVoxels(XFt, ThetaFt, YFt, PhiFt, nbmax);
   if (Closest4DVoxelsvector.size() <= 0) {
      // std::cout<<"Value outside full range"<<std::endl;
   } else {
      for (int i = 0; i < (int) Closest4DVoxelsvector.size(); i++) {
         ZGOUBIInversePositionDatabaseposition = Closest4DVoxelsvector[i];
         MultZGOUBIInversePosition = ZGOUBIInversePositionDatabase[ZGOUBIInversePositionDatabaseposition].GetInversePositionVectorSize();
         if (MultZGOUBIInversePosition > 0) {
            for (int j = 0; j < MultZGOUBIInversePosition; j++) {
               ZGOUBITrajectoryposition = ZGOUBIInversePositionDatabase[ZGOUBIInversePositionDatabaseposition].GetInversePositionVectorValue(j);
               distance = GetDistance(XFt, ThetaFt, YFt, PhiFt, ZGOUBITrajectoryposition);
               if (distance < radius) {
                  vectorLinesinRadius.push_back(ZGOUBITrajectoryposition);
               }
            }
         }
      }
   }
   return vectorLinesinRadius;
}

/*std::vector<Float_t> KVZGOUBIInverseMatrix::testGetLinesinRadius_bari(Float_t XFt, Float_t ThetaFt, Float_t YFt, Float_t PhiFt, Float_t radiusmultiplier)
{
   std::vector<Int_t> linenbs = GetNearestLinenbs(XFt, ThetaFt, YFt, PhiFt);
   Float_t distancemin1 = GetDistance(linenbs[0], linenbs[1]);
   Float_t distancemin2 = GetDistance(XFt, ThetaFt, YFt, PhiFt, linenbs[1]);
   Float_t distancemin = distancemin1;
   if (distancemin2 > distancemin1) {
      distancemin = distancemin2;
   }
   std::vector<Int_t> vectorlines = GetLinesinRadius(XFt, ThetaFt, YFt, PhiFt, distancemin * radiusmultiplier);
   Float_t XFtemp = 0;
   Float_t YFtemp = 0;
   Float_t ThetaFtemp = 0;
   Float_t PhiFtemp = 0;
   Float_t ThetaVtemp = 0;
   Float_t PhiVtemp = 0;
   Float_t Deltatemp = 0;
   Float_t Pathtemp = 0;
   std::vector<Float_t> results(10, 0);
   Int_t sizevectorlines = (int) vectorlines.size();

   std::vector<Float_t> Distance(sizevectorlines, 0);
   for (int i = 0; i < sizevectorlines; i++) {
      Distance[i] = GetDistance(XFt, ThetaFt, YFt, PhiFt, vectorlines[i]);
      if (Distance[i] == 0) {
         results[0] = ZGOUBIDatabase[vectorlines[i]].GetXF();
         results[1] = ZGOUBIDatabase[vectorlines[i]].GetYF();
         results[2] = ZGOUBIDatabase[vectorlines[i]].GetThetaF();
         results[3] = ZGOUBIDatabase[vectorlines[i]].GetPhiF();
         results[4] = ZGOUBIDatabase[vectorlines[i]].GetThetaV();
         results[5] = ZGOUBIDatabase[vectorlines[i]].GetPhiV();
         results[6] = ZGOUBIDatabase[vectorlines[i]].GetDelta();
         results[7] = ZGOUBIDatabase[vectorlines[i]].GetPath();
         results[8] = distancemin;
         results[9] = sizevectorlines;
         return results;
      }
   }

   for (int i = 0; i < sizevectorlines; i++) {
      XFtemp += ZGOUBIDatabase[vectorlines[i]].GetXF();
      YFtemp += ZGOUBIDatabase[vectorlines[i]].GetYF();
      ThetaFtemp += ZGOUBIDatabase[vectorlines[i]].GetThetaF();
      PhiFtemp += ZGOUBIDatabase[vectorlines[i]].GetPhiF();
      ThetaVtemp += ZGOUBIDatabase[vectorlines[i]].GetThetaV();
      PhiVtemp += ZGOUBIDatabase[vectorlines[i]].GetPhiV();
      Deltatemp += ZGOUBIDatabase[vectorlines[i]].GetDelta();
      Pathtemp += ZGOUBIDatabase[vectorlines[i]].GetPath();
   }
   XFtemp = XFtemp / sizevectorlines;
   YFtemp = YFtemp / sizevectorlines;
   ThetaFtemp = ThetaFtemp / sizevectorlines;
   PhiFtemp = PhiFtemp / sizevectorlines;
   ThetaVtemp = ThetaVtemp / sizevectorlines;
   PhiVtemp = PhiVtemp / sizevectorlines;
   Deltatemp = Deltatemp / sizevectorlines;
   Pathtemp = Pathtemp / sizevectorlines;

   results[0] = XFtemp;
   results[1] = YFtemp;
   results[2] = ThetaFtemp;
   results[3] = PhiFtemp;
   results[4] = ThetaVtemp;
   results[5] = PhiVtemp;
   results[6] = Deltatemp;
   results[7] = Pathtemp;
   results[8] = distancemin;
   results[9] = sizevectorlines;

   return results;
}

std::vector<Float_t> KVZGOUBIInverseMatrix::testGetLinesinRadius_weight(Float_t XFt, Float_t ThetaFt, Float_t YFt, Float_t PhiFt, Float_t radiusmultiplier)
{
   std::vector<Int_t> linenbs = GetNearestLinenbs(XFt, ThetaFt, YFt, PhiFt);
   std::vector<Float_t> results(10, -1);
   if (linenbs[0] != -1 && linenbs[1] != -1) {
      Float_t distancemin1 = GetDistance(linenbs[0], linenbs[1]);
      Float_t distancemin2 = GetDistance(XFt, ThetaFt, YFt, PhiFt, linenbs[1]);
      Float_t distancemin = distancemin1;
      if (distancemin2 > distancemin1) {
         distancemin = distancemin2;
      }
      std::vector<Int_t> vectorlines = GetLinesinRadius(XFt, ThetaFt, YFt, PhiFt, distancemin * radiusmultiplier);
      Float_t XFtemp = 0;
      Float_t YFtemp = 0;
      Float_t ThetaFtemp = 0;
      Float_t PhiFtemp = 0;
      Float_t ThetaVtemp = 0;
      Float_t PhiVtemp = 0;
      Float_t Deltatemp = 0;
      Float_t Pathtemp = 0;

      Int_t sizevectorlines = (int) vectorlines.size();
      std::vector<Float_t> Distance(sizevectorlines, 0);
      Float_t inversedistancetotal = 0;
      for (int i = 0; i < sizevectorlines; i++) {
         Distance[i] = GetDistance(XFt, ThetaFt, YFt, PhiFt, vectorlines[i]);
         if (Distance[i] == 0) {
            results[0] = ZGOUBIDatabase[vectorlines[i]].GetXF();
            results[1] = ZGOUBIDatabase[vectorlines[i]].GetYF();
            results[2] = ZGOUBIDatabase[vectorlines[i]].GetThetaF();
            results[3] = ZGOUBIDatabase[vectorlines[i]].GetPhiF();
            results[4] = ZGOUBIDatabase[vectorlines[i]].GetThetaV();
            results[5] = ZGOUBIDatabase[vectorlines[i]].GetPhiV();
            results[6] = ZGOUBIDatabase[vectorlines[i]].GetDelta();
            results[7] = ZGOUBIDatabase[vectorlines[i]].GetPath();
            results[8] = distancemin;
            results[9] = sizevectorlines;
            return results;
         }
         inversedistancetotal += 1. / Distance[i];
      }

      for (int i = 0; i < sizevectorlines; i++) {
         XFtemp += 1. / Distance[i] * ZGOUBIDatabase[vectorlines[i]].GetXF();
         YFtemp += 1. / Distance[i] * ZGOUBIDatabase[vectorlines[i]].GetYF();
         ThetaFtemp += 1. / Distance[i] * ZGOUBIDatabase[vectorlines[i]].GetThetaF();
         PhiFtemp += 1. / Distance[i] * ZGOUBIDatabase[vectorlines[i]].GetPhiF();
         ThetaVtemp += 1. / Distance[i] * ZGOUBIDatabase[vectorlines[i]].GetThetaV();
         PhiVtemp += 1. / Distance[i] * ZGOUBIDatabase[vectorlines[i]].GetPhiV();
         Deltatemp += 1. / Distance[i] * ZGOUBIDatabase[vectorlines[i]].GetDelta();
         Pathtemp += 1. / Distance[i] * ZGOUBIDatabase[vectorlines[i]].GetPath();
      }
      XFtemp = XFtemp / inversedistancetotal;
      YFtemp = YFtemp / inversedistancetotal;
      ThetaFtemp = ThetaFtemp / inversedistancetotal;
      PhiFtemp = PhiFtemp / inversedistancetotal;
      ThetaVtemp = ThetaVtemp / inversedistancetotal;
      PhiVtemp = PhiVtemp / inversedistancetotal;
      Deltatemp = Deltatemp / inversedistancetotal;
      Pathtemp = Pathtemp / inversedistancetotal;

      results[0] = XFtemp;
      results[1] = YFtemp;
      results[2] = ThetaFtemp;
      results[3] = PhiFtemp;
      results[4] = ThetaVtemp;
      results[5] = PhiVtemp;
      results[6] = Deltatemp;
      results[7] = Pathtemp;
      results[8] = distancemin;
      results[9] = sizevectorlines;
   }
   return results;
}
*/

std::vector<Float_t> KVZGOUBIInverseMatrix::testGetResults_weight(Float_t XFt, Float_t ThetaFt, Float_t YFt, Float_t PhiFt, Int_t nblines)
{
   std::vector<Int_t> linenbs = GetNearestLinenbs(XFt, ThetaFt, YFt, PhiFt, nblines);
   std::vector<Float_t> results(10, -1);
   std::vector<Float_t> Distance;
   Float_t inversedistancetotal = 0;
   if (linenbs.size() > 0) {
      for (int i = 0; i < (int) linenbs.size(); i++) {
         Distance.push_back(GetDistance(XFt, ThetaFt, YFt, PhiFt, linenbs[i]));
         if (Distance[i] == 0) {
            results[0] = ZGOUBIDatabase[linenbs[i]].GetXF();
            results[1] = ZGOUBIDatabase[linenbs[i]].GetYF();
            results[2] = ZGOUBIDatabase[linenbs[i]].GetThetaF();
            results[3] = ZGOUBIDatabase[linenbs[i]].GetPhiF();
            results[4] = ZGOUBIDatabase[linenbs[i]].GetThetaV();
            results[5] = ZGOUBIDatabase[linenbs[i]].GetPhiV();
            results[6] = ZGOUBIDatabase[linenbs[i]].GetDelta();
            results[7] = ZGOUBIDatabase[linenbs[i]].GetPath();
            results[8] = 0;
            results[9] = linenbs.size();
            return results;
         }
         inversedistancetotal += 1. / Distance[i];
      }
      Float_t XFtemp = 0;
      Float_t YFtemp = 0;
      Float_t ThetaFtemp = 0;
      Float_t PhiFtemp = 0;
      Float_t ThetaVtemp = 0;
      Float_t PhiVtemp = 0;
      Float_t Deltatemp = 0;
      Float_t Pathtemp = 0;
      for (int i = 0; i < (int) linenbs.size(); i++) {
         XFtemp += 1. / Distance[i] * ZGOUBIDatabase[linenbs[i]].GetXF();
         YFtemp += 1. / Distance[i] * ZGOUBIDatabase[linenbs[i]].GetYF();
         ThetaFtemp += 1. / Distance[i] * ZGOUBIDatabase[linenbs[i]].GetThetaF();
         PhiFtemp += 1. / Distance[i] * ZGOUBIDatabase[linenbs[i]].GetPhiF();
         ThetaVtemp += 1. / Distance[i] * ZGOUBIDatabase[linenbs[i]].GetThetaV();
         PhiVtemp += 1. / Distance[i] * ZGOUBIDatabase[linenbs[i]].GetPhiV();
         Deltatemp += 1. / Distance[i] * ZGOUBIDatabase[linenbs[i]].GetDelta();
         Pathtemp += 1. / Distance[i] * ZGOUBIDatabase[linenbs[i]].GetPath();
      }
      XFtemp = XFtemp / inversedistancetotal;
      YFtemp = YFtemp / inversedistancetotal;
      ThetaFtemp = ThetaFtemp / inversedistancetotal;
      PhiFtemp = PhiFtemp / inversedistancetotal;
      ThetaVtemp = ThetaVtemp / inversedistancetotal;
      PhiVtemp = PhiVtemp / inversedistancetotal;
      Deltatemp = Deltatemp / inversedistancetotal;
      Pathtemp = Pathtemp / inversedistancetotal;

      results[0] = XFtemp;
      results[1] = YFtemp;
      results[2] = ThetaFtemp;
      results[3] = PhiFtemp;
      results[4] = ThetaVtemp;
      results[5] = PhiVtemp;
      results[6] = Deltatemp;
      results[7] = Pathtemp;
      results[8] = 0;
      results[9] = linenbs.size();
   }
   return results;
}

std::vector<Float_t> KVZGOUBIInverseMatrix::testGetResults_weight_comb(Float_t XFt, Float_t ThetaFt, Float_t YFt, Float_t PhiFt, Int_t nblines)
{
   std::vector<Int_t> linenbs = GetNearestLinenbs(XFt, ThetaFt, YFt, PhiFt, nblines);
   std::vector<std::vector<int>> listofcomb = comb.GetComb((int) linenbs.size());
   std::vector<Float_t> results(10, -1);
   std::vector<Float_t> Distance;

   if (linenbs.size() > 0) {
      for (int i = 0; i < (int) linenbs.size(); i++) {
         Distance.push_back(GetDistance(XFt, ThetaFt, YFt, PhiFt, linenbs[i]));
         if (Distance[i] == 0) {
            results[0] = ZGOUBIDatabase[linenbs[i]].GetXF();
            results[1] = ZGOUBIDatabase[linenbs[i]].GetYF();
            results[2] = ZGOUBIDatabase[linenbs[i]].GetThetaF();
            results[3] = ZGOUBIDatabase[linenbs[i]].GetPhiF();
            results[4] = ZGOUBIDatabase[linenbs[i]].GetThetaV();
            results[5] = ZGOUBIDatabase[linenbs[i]].GetPhiV();
            results[6] = ZGOUBIDatabase[linenbs[i]].GetDelta();
            results[7] = ZGOUBIDatabase[linenbs[i]].GetPath();
            results[8] = 0;
            results[9] = linenbs.size();
            return results;
         }
      }



      Float_t XFtemp = 0;
      Float_t YFtemp = 0;
      Float_t ThetaFtemp = 0;
      Float_t PhiFtemp = 0;
      Float_t inversedistancetotal = 0;
      Float_t distancemin = 1000000;
      Float_t configurationmin = 0;
      Float_t distancecomb;

      for (int comb_test = 0; comb_test < (int) listofcomb.size(); comb_test++) {
         XFtemp = 0;
         YFtemp = 0;
         ThetaFtemp = 0;
         PhiFtemp = 0;
         inversedistancetotal = 0;
         for (int i = 0; i < (int) listofcomb[comb_test].size(); i++) {
            inversedistancetotal += 1. / Distance[listofcomb[comb_test][i]];
         }
         for (int i = 0; i < (int) listofcomb[comb_test].size(); i++) {
            XFtemp += 1. / Distance[listofcomb[comb_test][i]] * ZGOUBIDatabase[linenbs[listofcomb[comb_test][i]]].GetXF();
            YFtemp += 1. / Distance[listofcomb[comb_test][i]] * ZGOUBIDatabase[linenbs[listofcomb[comb_test][i]]].GetYF();
            ThetaFtemp += 1. / Distance[listofcomb[comb_test][i]] * ZGOUBIDatabase[linenbs[listofcomb[comb_test][i]]].GetThetaF();
            PhiFtemp += 1. / Distance[listofcomb[comb_test][i]] * ZGOUBIDatabase[linenbs[listofcomb[comb_test][i]]].GetPhiF();
         }
         XFtemp = XFtemp / inversedistancetotal;
         YFtemp = YFtemp / inversedistancetotal;
         ThetaFtemp = ThetaFtemp / inversedistancetotal;
         PhiFtemp = PhiFtemp / inversedistancetotal;
         distancecomb = GetDistance(XFtemp, ThetaFtemp, YFtemp, PhiFtemp, XFt, ThetaFt, YFt, PhiFt);
         if (distancecomb < distancemin) {
            distancemin = distancecomb;
            configurationmin = comb_test;
         }
      }

      //  std::cout<<listofcomb[configurationmin].size()<<" "<< configurationmin<<" "<<distancemin<<std::endl;
      XFtemp = 0;
      YFtemp = 0;
      ThetaFtemp = 0;
      PhiFtemp = 0;
      Float_t ThetaVtemp = 0;
      Float_t PhiVtemp = 0;
      Float_t Deltatemp = 0;
      Float_t Pathtemp = 0;
      inversedistancetotal = 0;
      for (int i = 0; i < (int) listofcomb[configurationmin].size(); i++) {
         inversedistancetotal += 1. / Distance[listofcomb[configurationmin][i]];
      }
      for (int i = 0; i < (int) listofcomb[configurationmin].size(); i++) {
         XFtemp += 1. / Distance[listofcomb[configurationmin][i]] * ZGOUBIDatabase[linenbs[listofcomb[configurationmin][i]]].GetXF();
         YFtemp += 1. / Distance[listofcomb[configurationmin][i]] * ZGOUBIDatabase[linenbs[listofcomb[configurationmin][i]]].GetYF();
         ThetaFtemp += 1. / Distance[listofcomb[configurationmin][i]] * ZGOUBIDatabase[linenbs[listofcomb[configurationmin][i]]].GetThetaF();
         PhiFtemp += 1. / Distance[listofcomb[configurationmin][i]] * ZGOUBIDatabase[linenbs[listofcomb[configurationmin][i]]].GetPhiF();
         ThetaVtemp += 1. / Distance[listofcomb[configurationmin][i]] * ZGOUBIDatabase[linenbs[listofcomb[configurationmin][i]]].GetThetaV();
         PhiVtemp += 1. / Distance[listofcomb[configurationmin][i]] * ZGOUBIDatabase[linenbs[listofcomb[configurationmin][i]]].GetPhiV();
         Deltatemp += 1. / Distance[listofcomb[configurationmin][i]] * ZGOUBIDatabase[linenbs[listofcomb[configurationmin][i]]].GetDelta();
         Pathtemp += 1. / Distance[listofcomb[configurationmin][i]] * ZGOUBIDatabase[linenbs[listofcomb[configurationmin][i]]].GetPath();
      }
      XFtemp = XFtemp / inversedistancetotal;
      YFtemp = YFtemp / inversedistancetotal;
      ThetaFtemp = ThetaFtemp / inversedistancetotal;
      PhiFtemp = PhiFtemp / inversedistancetotal;
      ThetaVtemp = ThetaVtemp / inversedistancetotal;
      PhiVtemp = PhiVtemp / inversedistancetotal;
      Deltatemp = Deltatemp / inversedistancetotal;
      Pathtemp = Pathtemp / inversedistancetotal;

      results[0] = XFtemp;
      results[1] = YFtemp;
      results[2] = ThetaFtemp;
      results[3] = PhiFtemp;
      results[4] = ThetaVtemp;
      results[5] = PhiVtemp;
      results[6] = Deltatemp;
      results[7] = Pathtemp;
      results[8] = 0;
      results[9] = linenbs.size();
   }
   return results;
}


KVZGOUBITrajectory KVZGOUBIInverseMatrix::GetZGOUBITrajectory(Int_t Trajectorynb)
{
   return ZGOUBIDatabase[Trajectorynb];
}


