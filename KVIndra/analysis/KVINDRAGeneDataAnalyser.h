/*
$Id: KVINDRAGeneDataAnalyser.h,v 1.1 2007/06/06 10:13:17 franklan Exp $
$Revision: 1.1 $
$Date: 2007/06/06 10:13:17 $
*/

//Created by KVClassFactory on Mon Jun  4 19:24:11 2007
//Author: e_ivamos

#ifndef __KVINDRAGENEDATAANALYSER_H
#define __KVINDRAGENEDATAANALYSER_H

#include "KVDataAnalyser.h"

class KVINDRAGeneDataAnalyser : public KVDataAnalyser {
protected:
   virtual KVNumberList PrintAvailableRuns(KVString& datatype);

public:

   KVINDRAGeneDataAnalyser();
   virtual ~KVINDRAGeneDataAnalyser();
   virtual void SubmitTask();

   ClassDef(KVINDRAGeneDataAnalyser, 1) //For analysing INDRA gene data
};

#endif
