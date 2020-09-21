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

/**
  \namespace NDT
  \brief Define key-value pair for nuclear data tables
  \ingroup NucProp
  */
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

/**
\class KVNuclDataTable
\brief Abstract base class for nuclear data table
\ingroup NucProp

General methods:
<ul>
   <li>Bool_t IsInTable(Int_t zz, Int_t aa) - returns kTRUE for nuclei which appear in
   the read table, kFALSE for others</li>
</ul>
Specific implementations must define the methods:
<ul>
   <li>Initialize() - called once to initialize mass table, for example by
   reading in values from a file</li>
   <li>Double_t GetValue(Int_t zz, Int_t aa) - returns value of the tabulated quantity</li>
   <li>CreateTable(Int_t ntot) - Create table where for each nucleus, a pointeur of the class will be stored</li>
   <li>CreateElement(Int_t idx)- Create element which correspond to the class pointeur associated to the nucleus</li>
   For example :
   If we consider a class KVLifeTime where is stored the value of life time of a nucleus
   ~~~~{.cpp}
   void KVNuclDataTable::CreateTable(Int_t ntot)
   {
      theTable = new KVLifeTime*[ntot];
   }

   void KVNuclDataTable::CreateElement(Int_t idx)
   {
      current_idx = idx;
      theTable[idx] = new KVLifeTime();
   }
   ~~~~
   For further detail see the KVLifeTimeTable and KVLifeTime class
</ul>
*/

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
