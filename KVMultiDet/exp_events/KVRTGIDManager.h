//Created by KVClassFactory on Thu Oct 11 11:51:14 2012
//Author: Guilain Ademard

#ifndef __KVRTGIDMANAGER_H
#define __KVRTGIDMANAGER_H

#include "KVTGIDManager.h"

class KVRTGIDManager : public KVTGIDManager {
protected:

   static KVList* fIDGlobalList;  //! Global list of TGID of all the KVRTGIDManager

   virtual void  SetTGID(KVTGID*);

public:
   KVRTGIDManager();
   virtual ~KVRTGIDManager();

   virtual void   AddTGID(KVTGID*);
   virtual KVTGID* GetTGID(const Char_t* idt_name, const Char_t* id_type,
                           const Char_t* grid_type);

   virtual void   RemoveAllTGID();
   virtual void   RemoveTGID(const Char_t* name);

   //------- static methods --------------------------//

   static void    AddTGIDToGlobalList(KVTGID*);
   static void    BuildGridForAllTGID(const Char_t* idtype = "", Double_t xmin = 0., Double_t xmax = 4096., Int_t ID_min = 0, Int_t ID_max = 0, Int_t npoints = 50, Bool_t logscale = kTRUE);
   static void    Clear();
   static void    DeleteTGID(const Char_t* name);
   static KVList* GetGlobalListOfIDFunctions()
   {
      return fIDGlobalList;
   };
   static Bool_t  GetTGIDfromIDGridManager(TList* tgid_list);
   static Bool_t  ReadAsciiFile(const Char_t* filename);
   static void    SetIDFuncInTelescopes(UInt_t run);
   static Int_t  UpdateListFromIDGridManager();
   static Int_t WriteAsciiFile(const Char_t* filename, const TCollection* selection = 0, Bool_t update = kTRUE);
   static TCollection* GetIDTelescopesForTGID(KVTGID*);

   ClassDef(KVRTGIDManager, 1) //Run-dependant KVTGIDManager
};

#endif
