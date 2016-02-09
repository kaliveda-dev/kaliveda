//Created by KVClassFactory on Wed Feb  9 11:06:42 2011
//Author: bonnet

#ifndef __KVNUCLDATATABLE_H
#define __KVNUCLDATATABLE_H

#include "TNamed.h"
#include "TMap.h"
#include "TObject.h"
#include "TClass.h"
#include "TObjArray.h"

#include "KVString.h"
#include "KVNuclData.h"


namespace NDT {
   class key : public TNamed {
   public:
      key(int z, int a)
      {
         SetName(Form("%d:%d", z, a));
      };
      virtual ~key() {};
      ClassDef(key, 0)
   };
   class value : public TObject {
      int idx;
   public:
      value(int i) : idx(i) {};
      virtual ~value() {};
      int Index() const
      {
         return idx;
      };
      ClassDef(value, 0)
   };
};

class KVNuclDataTable : public TNamed {

protected:

   TClass* cl;          //pointeur pour gerer les heritages de classes de KVNuclData
   TMap* nucMap;        //mapping (Z,A) -> nucleus index
   Int_t current_idx;   //current index
   Int_t NbNuc;         //nbre de noyaux presents dans la table

   KVString kcomments;  //Commentaire provenant de la lecture fichier
   KVString kclassname;

   //KVNumberList plageZ;

   TObjArray* tobj;  //! array where all nucldata objects are
   //TObjArray* tobj_rangeA;  //! array where range of A associated to each Z is stored via KVIntegerList

   KVNuclData* GetCurrent() const
   {
      return (KVNuclData*)tobj->At(current_idx);
   }
   void CreateTable(Int_t ntot); //Creation of the table
   void CreateElement(Int_t idx);//a new KVNuclData pointeur is created and added
   void InfoOnMeasured() const;

   virtual NDT::value* getNDTvalue(Int_t zz, Int_t aa) const;

public:
   KVNuclDataTable();
   KVNuclDataTable(KVString classname);
   virtual ~KVNuclDataTable();
   void init();
   virtual void Initialize() = 0;

   virtual void GiveIndexToNucleus(Int_t zz, Int_t aa, Int_t ntot);
   Bool_t IsInTable(Int_t zz, Int_t aa) const;

   KVNuclData* GetData(Int_t zz, Int_t aa) const;
   Double_t GetValue(Int_t zz, Int_t aa)  const;
   void SetValue(Int_t zz, Int_t aa, Double_t val);
   const Char_t*  GetUnit(Int_t zz, Int_t aa) const;
   Bool_t  IsMeasured(Int_t zz, Int_t aa) const;

   Int_t GetNumberOfNuclei() const;
   const Char_t*   GetReadFileName() const;
   KVString GetCommentsFromFile() const;

   ClassDef(KVNuclDataTable, 1) //Store information on nuclei

};

#endif
