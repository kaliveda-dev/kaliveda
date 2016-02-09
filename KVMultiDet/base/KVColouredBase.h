//Created by KVClassFactory on Wed Jun  4 17:27:05 2014
//Author: get_indra

#ifndef __KVCOLOUREDBASE_H
#define __KVCOLOUREDBASE_H

#include "KVBase.h"

class KVColouredBase : public KVBase {
protected:
   TString fColour;

public:
   KVColouredBase();
   KVColouredBase(const Char_t* name, const Char_t* title = "", const Char_t* colour = "white");
   virtual ~KVColouredBase();

   void SetColour(const Char_t* c)
   {
      fColour = c;
   }
   const Char_t* GetLVEntryColour() const
   {
      return fColour;
   }

   ClassDef(KVColouredBase, 1) //A basic coloured object for coloured list views
};

#endif
