//Created by KVClassFactory on Wed Jul  7 12:15:56 2010
//Author: bonnet

#ifndef __KVSIMREADER_HIPSE_H
#define __KVSIMREADER_HIPSE_H

#include "KVSimReader.h"
#include "TH1F.h"

class KVSimReader_HIPSE : public KVSimReader {

protected:
   TH1F* h1;

public:
   KVSimReader_HIPSE();
   KVSimReader_HIPSE(KVString filename);

   virtual ~KVSimReader_HIPSE();

   void init()
   {
      Info("init", "passe");
      tree_name = "HIPSE";
      Info("init", "%s", branch_name.Data());
      h1 = 0;
   }

   void ReadFile();
   Bool_t ReadHeader();
   Bool_t ReadEvent();
   Bool_t ReadNucleus();

   virtual void SetPotentialHardness(Double_t val)
   {
      KVString sval;
      sval.Form("%lf", val);
      AddInfo("Hardness of the potential", sval.Data());
   }
   virtual void SetExchangePercentage(Double_t val)
   {
      KVString sval;
      sval.Form("%lf", val);
      AddInfo("Percentage of exchange", sval.Data());
   }
   virtual void SetNNCollisionPercentage(Double_t val)
   {
      KVString sval;
      sval.Form("%lf", val);
      AddInfo("Percentage of N-N collisions", sval.Data());
   }

   ClassDef(KVSimReader_HIPSE, 1) //Read ascii file for events of the HIPSE code after clusterization
};

#endif
