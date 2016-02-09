/*
$Id: KVFoxH2.h,v 1.2 2009/01/23 15:25:52 franklan Exp $
$Revision: 1.2 $
$Date: 2009/01/23 15:25:52 $
*/

//Created by KVClassFactory on Tue Nov 27 17:47:49 2007
//Author: franklan

#ifndef __KVFOXH2_H
#define __KVFOXH2_H

#include "KVVarGlob1.h"

class KVFoxH2: public KVVarGlob1 {
// Static fields (if any):
public:

// Protected fields (if any):
protected:
   Double_t num;
   Double_t den;

// Methods
protected:
   void init_KVFoxH2(void);
   virtual Double_t getvalue_void(void) const;

public:
   KVFoxH2(void);       // default constructor
   KVFoxH2(Char_t* nom);      // constructor with aname
   KVFoxH2(const KVFoxH2& a);// copy constructor

   void Init(void);
   void Reset(void);
   virtual ~KVFoxH2(void);    // destructor

#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   virtual void Copy(TObject& obj) const;
#else
   virtual void Copy(TObject& obj);
#endif

   KVFoxH2& operator = (const KVFoxH2& a); // operator =

   void Fill2(KVNucleus* n1, KVNucleus* n2);    // Filling method

   ClassDef(KVFoxH2, 1) //Event shape global variable : second Fox-Wolfram moment, H(2)

};


#endif
