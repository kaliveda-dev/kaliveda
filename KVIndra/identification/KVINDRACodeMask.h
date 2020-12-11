//$Id: KVINDRACodeMask.h,v 1.4 2008/02/14 10:30:18 franklan Exp $
//Author: $Author: franklan $

#ifndef __KVINDRACODEMASK__
#define __KVINDRACODEMASK__

#include "TObject.h"

////////////////////////////////////////////////////////////////////////////////////////

/* Global bits for code masks */
enum EVedaCodes {
   kIDCode0 = BIT(0),           //these bits concern KVINDRACodeMask::fIDMask
   kIDCode1 = BIT(1),
   kIDCode2 = BIT(2),
   kIDCode3 = BIT(3),
   kIDCode4 = BIT(4),
   kIDCode5 = BIT(5),
   kIDCode6 = BIT(6),
   kIDCode7 = BIT(7),
   kIDCode8 = BIT(8),
   kIDCode9 = BIT(9),
   kIDCode10 = BIT(10),
   kIDCode11 = BIT(11),
   kIDCode14 = BIT(12),
   kIDCode15 = BIT(13),
   kECode0 = BIT(0),            //these bits concern KVINDRACodeMask::fEMask
   kECode1 = BIT(1),
   kECode2 = BIT(2),
   kECode3 = BIT(3),
   kECode11 = BIT(4),
   kECode15 = BIT(5),
   kECode4 = BIT(6),
   kECode5 = BIT(7)
};

enum EIDCodes {
   kIDCode_Gamma = kIDCode0,
   kIDCode_Neutron = kIDCode1,
   kIDCode_CsI = kIDCode2,
   kIDCode_Phoswich = kIDCode2,
   kIDCode_SiCsI = kIDCode3,
   kIDCode_SiLiCsI = kIDCode3,
   kIDCode_Si75SiLi = kIDCode3,
   kIDCode_ChIoSi = kIDCode4,
   kIDCode_ChIoCsI = kIDCode4,
   kIDCode_ChIoSi75 = kIDCode4,
   kIDCode_Bragg = kIDCode5,
   kIDCode_Zmin = kIDCode5,
   kIDCode_ZminCsI = kIDCode9,
   kIDCode_ArretChIo = kIDCode5,
   kIDCode_ArretSi = kIDCode5,
   kIDCode_MultihitChIo = kIDCode8
};

/**
  \class KVIDCodeMask
  \ingroup INDRAIdent
  \brief Identification code structure for INDRA data (obsolete)

Handles bit-masks used to describe identification & calibration codes for particles,
and also used to define which particles are to be considered "ok" or "good" for analysis
(i.e. which particles have IsOK()=kTRUE).

For compatibility with old VEDA6 codes, a set of bit-mask constants are defined to
describe ID and energy codes:
~~~~
  ID codes:  kIDCode0, kIDCode1, ..., kIDCode15
  E  codes:  kECode0, kECode1, ..., kECode15
~~~~
_____________________________________________________________________________________

Let us recall the meaning of the VEDA_6.INCL codes :
~~~~
                         code = 0 : gamma
                         code = 1 : neutron (seulement couronnes 2 a 9)
                         code = 2 : part. ident. dans CsI ou Phoswich (OK + 8Be)
                         code = 3 : fragment identifie dans Si-CsI
                                                ou fragment ident. dans Si75-SiLi ou SiLi-CsI
                         code = 4 : fragment identifie dans ChIo-Si
                                                ou fragment ou part. identifie dans ChIo-Si75
                         code = 5 : fragment non-ident. (montee de Bragg)(Z min.)
                                                                 ou stoppe dans Chio (Z min)
                         code = 6 : frag. cree par la coherence CsI  (stoppe ds Si.)
                         code = 7 : frag. cree par la coherence ChIo (stoppe ds ChIo)
                         code = 8 : multiple comptage dans ChIo avec arret
                                                                 - dans les siliciums (cour. 2-9)
                                                                 - dans les CsI (cour. 10-17)
                         code = 9 : ident. incomplete dans CsI ou Phoswich (Z.min)
                         code =10 : ident. "entre les lignes" dans CsI
                         code =11 : ident. Z=2 due a l'inversion CHIO (Argon)

                         code =14 : Particule restee non identifiee (a rejeter)
                         code =15 : frag. ou part. rejete
~~~~
    Nota: Pour le code 8 la contribution ChIo est partagee egalement entre
               toutes les particules qui s'arretent dans les modules suivants.
               Le Z individuel est donc surement faux mais cela permet
               d'avoir une assez bonne estimation du Z.total.

               Les codes 9 et 10 sont des codes qui ont ete ajoutes pour donner
               au code 2 la notion d'identification sans ambiguites.

____________________________________________________________________________________

NEW ID CODE MASKS
-----------------

For greater ease of use, we also define new bit-mask constants which are equivalent
to the previous ones, but have names which make their meaning more clear (and easier to
remember) :
~~~~{.cpp}
              kIDCode_Gamma = kIDCode0,
              kIDCode_Neutron = kIDCode1,
              kIDCode_CsI = kIDCode2,
              kIDCode_Phoswich = kIDCode2,
              kIDCode_SiCsI = kIDCode3,
              kIDCode_SiLiCsI = kIDCode3,
              kIDCode_Si75SiLi = kIDCode3,
              kIDCode_ChIoSi = kIDCode4,
              kIDCode_ChIoCsI = kIDCode4,
              kIDCode_ChIoSi75 = kIDCode4,
              kIDCode_Bragg = kIDCode5,
              kIDCode_Zmin = kIDCode5,
              kIDCode_ZminCsI = kIDCode9,
              kIDCode_ArretChIo = kIDCode5,
              kIDCode_ArretSi = kIDCode5,
              kIDCode_MultihitChIo = kIDCode8
~~~~

DEFINING CODE MASKS
-------------------
Code masks defining acceptable particles for events are defined using the predefined
bit-masks and the bit-wise "OR" as follows:

~~~~{.cpp}
  SetIDMask( kIDCode2 | kIDCode3 | kIDCode 4 | kIDCode6 );   // ID codes 2-4 & 6 are accepted, all others rejected
  SetEMask ( kECode1 | kECode2 );                            // accept calibration codes 1 & 2
~~~~

TESTING CODE MASKS
------------------
You can test the identification code of a particle using the TestIDCode method (see
KVINDRAReconNuc on how to obtain the particle's code mask):
~~~~{.cpp}
      Bool_t idchiosi = TestIDCode( kIDCode_ChIoSi );
~~~~
In this example, idchiosi=kTRUE if the particle's ID code indicates detection in a
ChIo-Si telescope.

This method also works to test if a given code is accepted by the mask set for an
event (see KVINDRAReconEvent).
*/
class KVINDRACodeMask: public TObject {

private:
   UShort_t fIDMask;            //16-bit mask with id codes
   UChar_t fEMask;              //8-bit mask with e-calibration codes

public:
   void Copy(TObject& object) const
   {
      KVINDRACodeMask& i = (KVINDRACodeMask&)object;
      i.fIDMask = fIDMask;
      i.fEMask = fEMask;
      TObject::Copy(object);
   }

   KVINDRACodeMask()
   {
      //Default ctor - no mask defined
      fIDMask = 0;
      fEMask = 0;
   };
   virtual ~ KVINDRACodeMask()
   {
      //Default dtor
      fIDMask = 0;
      fEMask = 0;
   };

   UShort_t GetIDMask()
   {
      return fIDMask;
   }
   UChar_t GetEMask()
   {
      return fEMask;
   }
   void SetIDMask(UShort_t codes)
   {
      fIDMask = codes;
   }
   Bool_t TestIDCode(UShort_t code);
   Bool_t TestECode(UChar_t code);
   void SetEMask(UChar_t codes)
   {
      fEMask = codes;
   }

   Bool_t operator&(KVINDRACodeMask& msk)
   {
      return (TestIDCode(msk.GetIDMask()) && TestECode(msk.GetEMask()));
   };

   void Clear(Option_t* = "")
   {
      fIDMask = 0;
      fEMask = 0;
      TObject::Clear();
   };

   Bool_t IsNull() const
   {
      // Returns kTRUE if no mask is set
      return (!fIDMask && !fEMask);
   }

   ClassDef(KVINDRACodeMask, 1);        //Handles setting and testing code masks for particle calibration quality control
};


#endif
