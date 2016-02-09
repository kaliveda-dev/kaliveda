/*
$Id: KVSource.h,v 1.3 2007/06/27 14:53:12 franklan Exp $
$Revision: 1.3 $
$Date: 2007/06/27 14:53:12 $
*/

//Created by KVClassFactory on Wed Jul  5 17:37:01 2006
//Author: Daniel CUSSOL

#ifndef __KVSource_H
#define __KVSource_H

#include <KVVarGlob.h>
#include <KVNucleus.h>

class KVSource: public KVVarGlob {
// Champs Statiques:
public:

// Champ proteges:
private:
   KVNucleus fSource;
   Double_t fVal[8];//! used by GetValuePtr
   Int_t fMult;//! source multiplicity

// Methodes
protected:
   void init_KVSource(void);
   virtual Double_t getvalue_void() const;
   virtual Double_t getvalue_int(Int_t i);


public:
   KVSource(void);         // constructeur par defaut
   KVSource(const Char_t* nom);
   KVSource(const KVSource& a);  // constructeur par Copy

   virtual ~KVSource(void);      // destructeur

   virtual void Copy(TObject& obj) const;

   virtual void Init(void);
   virtual void Reset(void);
   virtual void Fill(KVNucleus* c);
   virtual Double_t* GetValuePtr(void);
   virtual TObject* GetObject(void) const;
   virtual Char_t GetValueType(Int_t) const;

   ClassDef(KVSource, 1) //Simple source reconstruction global variable

};
#endif
