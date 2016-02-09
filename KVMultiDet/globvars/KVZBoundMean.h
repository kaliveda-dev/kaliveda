/*
$Id: KVZBoundMean.h,v 1.1 2009/04/07 12:04:11 franklan Exp $
$Revision: 1.1 $
$Date: 2009/04/07 12:04:11 $
*/

//Created by KVClassFactory on Mon Apr  6 11:15:08 2009
//Author: Francis Gagnon-Moisan

#ifndef __KVZBOUNDMEAN_H
#define __KVZBOUNDMEAN_H

#include "KVVarGlobMean.h"

class KVZBoundMean: public KVVarGlobMean {

public:
// Champs Statiques:
   static Int_t nb;
   static Int_t nb_crea;
   static Int_t nb_dest;


// Methodes


public:
   KVZBoundMean(void);
   KVZBoundMean(Char_t* nom);
   KVZBoundMean(const KVZBoundMean& a);

   virtual ~KVZBoundMean(void);

#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   virtual void Copy(TObject& obj) const;
#else
   virtual void Copy(TObject& obj);
#endif


   KVZBoundMean& operator = (const KVZBoundMean& a);

   void init_KVZBoundMean() ;
   void Fill(KVNucleus*) ;
   ClassDef(KVZBoundMean, 1) //Global variable returning the mean charge of products in a selected range of Z

};


#endif
