//Created by KVClassFactory on Mon Jul  5 16:21:41 2010
//Author: bonnet

#ifndef __KVSIMREADER_SMF_H
#define __KVSIMREADER_SMF_H

#include "KVSimReader.h"

class KVSimReader_SMF : public KVSimReader {

public:
   KVSimReader_SMF();
   KVSimReader_SMF(KVString filename);

   void init()
   {
      tree_name = "SMF";
   }

   virtual ~KVSimReader_SMF();

   virtual void ReadFile();
   virtual Bool_t ReadHeader();
   virtual Bool_t ReadEvent();
   virtual Bool_t ReadNucleus();


   void SetTimeBranching(Double_t val)
   {
      KVString sval;
      sval.Form("%lf", val);
      AddInfo("Branching time", sval.Data());
   }



   ClassDef(KVSimReader_SMF, 1) //Read ascii file for events of the SMF code after clusterization
};

#endif
