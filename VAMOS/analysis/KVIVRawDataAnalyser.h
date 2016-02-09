/*
$Id: KVIVRawDataAnalyser.h,v 1.1 2009/01/14 15:59:49 franklan Exp $
$Revision: 1.1 $
$Date: 2009/01/14 15:59:49 $
*/

//Created by KVClassFactory on Thu Jan  8 11:46:27 2009
//Author: John Frankland,,,

#ifndef __KVIVRAWDATAANALYSER_H
#define __KVIVRAWDATAANALYSER_H

#include "KVINDRARawDataAnalyser.h"
#include "TTree.h"

class KVIVRawDataAnalyser : public KVINDRARawDataAnalyser {

   TTree* fBidonTree;//! just a dummy for VAMOS Parameters class

public:
   KVIVRawDataAnalyser();
   virtual ~KVIVRawDataAnalyser();
   virtual void postInitRun();
   static void Make(const Char_t* kvsname = "MyOwnRawDataAnalyser");

   inline TTree* GetTree()
   {
      return fBidonTree;
   }
   ClassDef(KVIVRawDataAnalyser, 1) //Analysis of raw INDRA-VAMOS data
};

#endif
