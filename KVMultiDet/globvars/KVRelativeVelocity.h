/*
$Id: KVRelativeVelocity.h,v 1.2 2007/11/29 13:12:22 franklan Exp $
$Revision: 1.2 $
$Date: 2007/11/29 13:12:22 $
*/

//Created by KVClassFactory on Wed Nov 28 11:05:04 2007
//Author: bonnet

#ifndef __KVRELATIVEVELOCITY_H
#define __KVRELATIVEVELOCITY_H

#include "KVVarGlobMean.h"

class KVRelativeVelocity: public KVVarGlobMean {
// Static fields (if any):
public:

// Protected fields (if any):
protected:

// Methods
protected:
   TList* heaviest;            //list of pointers to nuclei
   void init_KVRelativeVelocity(void);

public:
   KVRelativeVelocity(void);        // default constructor
   KVRelativeVelocity(Char_t* nom);    // constructor with aname
   KVRelativeVelocity(const KVRelativeVelocity& a);// copy constructor

   virtual ~KVRelativeVelocity(void);     // destructor

#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   virtual void Copy(TObject& obj) const;
#else
   virtual void Copy(TObject& obj);
#endif

   KVRelativeVelocity& operator = (const KVRelativeVelocity& a); // operator =

   //virtual void Fill2(KVNucleus *ci,KVNucleus *cj);    // Filling method
   virtual void Fill(KVNucleus* c);     // Remplissage de la variable.
   virtual void Reset(void);    // Remise a zero de la charge et du
   // pointeur.
   virtual void Init(void);     // Remise a zero de la charge et du

   ClassDef(KVRelativeVelocity, 1) //calculate for a given a couple of particles the magnitude of relative velocity

};


#endif
