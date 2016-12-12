//Created by KVClassFactory on Wed Sep 23 16:07:38 2009
//Author: Chbihi

#ifndef __KVGANILDATAREADER_H
#define __KVGANILDATAREADER_H

#include "KVRawDataReader.h"
#include "KVBase.h"
#include "KVACQParam.h"
#include "KVHashList.h"
#include "TTree.h"
class GTGanilData;

class KVGANILDataReader : public KVRawDataReader {
protected:
   GTGanilData* fGanilData;//object used to read GANIL acquisition file
   TTree* fUserTree;//user TTree to fill with data
   Int_t NbParFired;
   UShort_t* ParVal;
   UInt_t* ParNum;
   Bool_t make_arrays;
   Bool_t make_leaves;

   virtual void ConnectRawDataParameters();

   KVHashList* fParameters;//list of all data parameters contained in file
   KVHashList* fExtParams;//list of data parameters in file not defined by gMultiDetArray
   KVHashList* fFired;//list of fired parameters in one event

   virtual GTGanilData* NewGanTapeInterface();
   virtual KVACQParam* CheckACQParam(const Char_t*);

   void FillFiredParameterList();

public:
   KVGANILDataReader()
   {
      init();
   };
   KVGANILDataReader(const Char_t*);
   virtual void OpenFile(const Char_t*);
   virtual ~KVGANILDataReader();

   void init();
   virtual Bool_t GetNextEvent();
   virtual GTGanilData* GetGanTapeInterface();

   virtual void SetUserTree(TTree*, Option_t* = "arrays");

   const KVSeqCollection* GetUnknownParameters() const
   {
      return fExtParams;
   };
   const KVSeqCollection* GetRawDataParameters() const
   {
      return fParameters;
   };

   KVSeqCollection* GetFiredDataParameters() const
   {
      // returns pointer to list of fired acquisition parameters of current event.
      // this list is filled automatically when GetNextEvent() is called.
      return fFired;
   };

   static KVGANILDataReader* Open(const Char_t* filename, Option_t* opt = "");

   Bool_t HasScalerBuffer() const;
   Int_t GetNumberOfScalers() const;
   UInt_t GetScalerCount(Int_t index) const;
   Int_t GetScalerStatus(Int_t index) const;
   Int_t GetEventCount() const;

   ClassDef(KVGANILDataReader, 0) //Reads GANIL acquisition files
};

#endif
