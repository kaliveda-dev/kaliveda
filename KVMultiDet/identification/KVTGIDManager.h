/***************************************************************************
                          KVTGIDManager.h  -  description
                             -------------------
    begin                : 21 July 2005
    copyright            : (C) 2005 by J.D. Frankland
    email                : frankland@ganil.fr

$Id: KVTGIDManager.h,v 1.10 2008/04/04 09:06:25 franklan Exp $
***************************************************************************/

#ifndef KVTGIDMAN__H
#define KVTGIDMAN__H

#include "Rtypes.h"
#include "KVList.h"
#include "KVTGID.h"

class KVTGIDGrid;

/**
\class KVTGIDManager
\brief Handles a set of Tassan-Got functional-based identifications (KVTGID objects)  for use by a KVIDTelescope.
\ingroup Identification

IdentZ and IdentA are, respectively, Z and A identification procedures.
Their purpose is to retrieve the correct KVTGID identification object
from the manager's list, and then use KVTGID::GetIdentification.

Status code for last identification with IdentZ and IdentA can be
obtained using GetStatus/GetStatusString.
~~~~
 0 KVTGIDManager::kStatus_OK            "ok",
 1 KVTGIDManager::kStatus_noTGID        "no KVTGID for requested identification",
 2 KVTGIDManager::kStatus_OutOfIDRange  "point to identify outside of identification range of function",
 3 KVTGIDManager::kStatus_ZtooSmall     "IdentA called with Z<1",
 4 KVTGIDManager::kStatus_ZtooLarge     "IdentA called with Z larger than max Z defined for KVTGIDZA isotopic identification object"
~~~~
*/

class KVTGIDManager {
protected:
   KVList fIDList;              //KVTGID objects for identifications
   Double_t fID_max;            //maximum ID of all TGID objects
   Int_t fStatus;               //! transient member used to hold status of last call to IdentZ/IdentA

public:

   //status codes for IdentZ/IdentA
   enum ETGIDMStatus {
      kStatus_OK,               //normal identification
      kStatus_noTGID,           //no KVTGID corresponding to requested identification
      kStatus_OutOfIDRange,     //point to identify outside of identification range of KVTGID
      kStatus_ZtooSmall,        //Z passed to IdentA too small (<1)
      kStatus_ZtooLarge,        //Z passed to IdentA larger than max Z defined by KVTGIDZA
      kCurrentStatus            //default argument for GetStatusString
   };

   KVTGIDManager();
   virtual ~ KVTGIDManager();

   virtual void AddTGID(KVTGID*);
   KVTGID* GetTGID(const Char_t* name);
   virtual void RemoveTGID(const Char_t* name);
   virtual void RemoveAllTGID();
   virtual KVTGID* GetTGID(const Char_t* idt_name, const Char_t* id_type,
                           const Char_t* grid_type);
   const Char_t* GetTGIDName(const Char_t* idt_name,
                             const Char_t* id_type,
                             const Char_t* grid_type);

   const KVList& GetListOfIDFunctions() const
   {
      return fIDList;
   };
   Double_t GetIDmax() const
   {
      return fID_max;
   };

   virtual Double_t IdentZ(const Char_t*, Double_t, Double_t, Double_t& funLTG,
                           const Char_t* grid_type);
   virtual Double_t IdentA(const Char_t*, Double_t, Double_t, Double_t& funLTG,
                           const Char_t* grid_type, Int_t Z);

   virtual KVTGIDGrid* GetTGIDGrid(KVTGID* tgid, Double_t xmax,
                                   Double_t xmin = 0., Int_t ID_min =
                                      0, Int_t ID_max = 0, Int_t npoints = 100, Bool_t logscale = kFALSE);

   virtual KVTGIDGrid* GetTGIDGrid(const Char_t* tgid_name, Double_t xmax,
                                   Double_t xmin = 0., Int_t ID_min =
                                      0, Int_t ID_max = 0, Int_t npoints = 100, Bool_t logscale = kFALSE);
   virtual KVTGIDGrid* GetTGIDGrid(const Char_t* idt_name,
                                   const Char_t* id_type,
                                   const Char_t* grid_type, Double_t xmax,
                                   Double_t xmin = 0., Int_t ID_min =
                                      0, Int_t ID_max = 0, Int_t npoints = 100, Bool_t logscale = kFALSE);

   inline virtual Int_t GetStatus() const
   {
      return fStatus;
   };
   inline void SetStatus(ETGIDMStatus s)
   {
      fStatus = s;
   };
   virtual const Char_t* GetStatusString(ETGIDMStatus s =
         kCurrentStatus) const;

   ClassDef(KVTGIDManager, 1)   //Handles a set of Tassan-Got Z & A identification functionals
};
#endif
