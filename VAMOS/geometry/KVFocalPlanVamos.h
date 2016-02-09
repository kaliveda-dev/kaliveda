/*
$Id: KVClassFactory.cpp,v 1.19 2009/01/21 08:04:20 franklan Exp $
$Revision: 1.19 $
$Date: 2009/01/21 08:04:20 $
*/

//Created by KVClassFactory on Wed May 13 12:31:05 2009
//Author: Abdelouahao Chbihi

#ifndef __KVKVFocalPlanVamos_H
#define __KVKVFocalPlanVamos_H

#include "KVMultiDetArray.h"
#include "KVList.h"
#include "KVTelescope.h"

class KVFocalPlanVamos : public KVMultiDetArray {

public:
   KVFocalPlanVamos();
   virtual ~KVFocalPlanVamos();

   inline KVList* GetListOfChIo() const
   {
      return fChIo;
   };
   inline KVList* GetListOfSi() const
   {
      return fSi;
   };
   inline KVList* GetListOfCsI() const
   {
      return fCsI;
   };
   inline KVList* GetListOfGap() const
   {
      return fGap;
   };
   KVLayer* GetSiLayer();
   void SetNamesDetectors();
protected:
   KVList* fSi;                 //->List of Si detectors of FocPlanVamos
   KVList* fCsI;                //->List of CsI detectors of FocPlanVamos
   KVList* fGap;     //->List of gap detectors of FocPlanVamos
   KVList* fChIo;               //->List of Ionisation Chambers of FocPlanVamos

   KVLayer* fSiLayer;   //reference to Si Layerof the Vamos FP

   virtual void MakeListOfDetectorTypes();
   //virtual void MakeListOfDetectors();
   virtual void PrototypeTelescopes();
   virtual void BuildGeometry();
   virtual void SetGroupsAndIDTelescopes();
   //void FillListsOfDetectorsByType();


   ClassDef(KVFocalPlanVamos, 1) //Si and CsI of vamos at FP
};

#endif
