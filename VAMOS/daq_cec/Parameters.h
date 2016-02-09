#ifndef _PARAMETERS_CLASS
#define _PARAMETERS_CLASS

#include "ParameterName.h"
#include "Riostream.h"
#include "Rtypes.h"
#include "TTree.h"
#include <cstdlib>

#define MAX_PAR_NUM 20000

#define NOTDC
#define NOTDC2
#define NOADC2
#define NOBIDON
#define NOQDC
#define NOV550
#define NOCENTRUM
#define NOTRIG
#define STRIPS
#define ECC
#define ECC20MEV
#define GOCCE
#define ESS
#define SED
#define NOTIARA
#define NOADC6414
#define NOADC
#define SI
#define ECHI
#define WCSI
#define INDRA
#define NOSELECTOR
#define NOPILA
#define NOETALON
#define NOMT4

class Parameters {

public:

   Parameters(void);
   virtual ~Parameters(void);

   void Check(void);
   void Set(TTree* theTree);
   void Clear(void);
   void GetData(Short_t* data);
   char* CopyParam(char* Dest, char* Source) const;
   void Fill(const char* buffparam);

   UShort_t  fMaxParameterNumber;
   UShort_t  fNumberofParameters;
   UShort_t* fParameterNumber;
   Short_t*  fParameterType;
   UShort_t* fParameterBits;

   ParameterName* fParameterName;

   UShort_t* fParameterMap;
   Short_t*  fParameterMapT;
   UShort_t* fParameterData;
   UShort_t* fParametersTouched;
   UShort_t  fNumberofParametersFree;
   UShort_t  fParametersM;

#ifdef ECHI
   bool     EchiB;
   Int_t    EchiM;
   UShort_t EchiNr[3 * 8];
   UShort_t Echi[3 * 8];
#endif

#ifdef STRIPS
   bool     STRB;
   Int_t    STRM1, STRM2, STRM3, STRM4;
   UShort_t STRNr[4 * 64];
   UShort_t STR[4 * 64];
#endif

#ifdef SED
   bool     SEDB;
   Int_t    SEDMX1, SEDMY1, SEDMX2, SEDMY2;
   UShort_t SEDQ[4 * 128];
   UShort_t SEDNr[4 * 128];
#endif
#ifdef CSI
   bool     CSIEB, CSITB, CSIPB;
   Int_t    CSIEM, CSITM, CSIPM;
   UShort_t CSIE[50];
   UShort_t CSIENr[50];
   UShort_t CSIT[50];
   UShort_t CSITNr[50];
   UShort_t CSIP[50];
   UShort_t CSIPNr[50];
#endif

#ifdef SI
   bool     SIEB;
   Int_t    SIEM;
   UShort_t SIENr[21];
   UShort_t SIE[21];
#endif

#ifdef WCSI
   bool     WCSIEB;
   Int_t    WCSIEM;
   UShort_t WCSIENr[96];
   UShort_t WCSIE[96];
#endif

#ifdef INDRA
   bool     ICSILB;
   bool     ICSIRB;
   bool     ICSITB;
   bool     ISILB;
   bool     ISIHB;
   bool     ISITB;
   bool     ICILB;
   bool     ICIHB;
   bool     ICITB;

   Int_t    ICSILM;
   UShort_t ICSILNr[408];
   UShort_t ICSIL[408];
   Int_t    ICSIRM;
   UShort_t ICSIRNr[408];
   UShort_t ICSIR[408];
   Int_t    ICSITM;
   UShort_t ICSITNr[408];
   UShort_t ICSIT[408];

   Int_t    ISILM;
   UShort_t ISILNr[408];
   UShort_t ISIL[408];
   Int_t    ISIHM;
   UShort_t ISIHNr[408];
   UShort_t ISIH[408];
   Int_t    ISITM;
   UShort_t ISITNr[408];
   UShort_t ISIT[408];

   Int_t    ICILM;
   UShort_t ICILNr[408];
   UShort_t ICIL[408];
   Int_t    ICIHM;
   UShort_t ICIHNr[408];
   UShort_t ICIH[408];
   Int_t    ICITM;
   UShort_t ICITNr[408];
   UShort_t ICIT[408];

#endif


#ifdef ECC
   bool     ECCEB;
   Int_t    ECCEM;
   UShort_t ECCENr[16 * 4];
   UShort_t ECCE[16 * 4];
   bool     ECCTB;
   Int_t    ECCTM;
   UShort_t ECCTNr[16 * 4];
   UShort_t ECCT[16 * 4];
#endif

#ifdef ECC20MEV
   bool     ECC20EB;
   Int_t    ECC20EM;
   UShort_t ECC20ENr[16 * 4];
   UShort_t ECC20E[16 * 4];
#endif

#ifdef GOCCE
   bool     GOCCEEB;
   Int_t    GOCCEEM;
   UShort_t GOCCEENr[16 * 4 * 4];
   UShort_t GOCCEE[16 * 4 * 4];
   bool     GOCCETB;
   Int_t    GOCCETM;
   UShort_t GOCCETNr[16 * 4 * 4];
   UShort_t GOCCET[16 * 4 * 4];
#endif

#ifdef ESS
   bool     ESSTB;
   Int_t    ESSTM;
   UShort_t ESSTNr[16];
   UShort_t ESST[16];
   bool     ESSQB;
   Int_t    ESSQM;
   UShort_t ESSQNr[16 * 4];
   UShort_t ESSQ[16 * 4];
#endif

   ClassDef(Parameters, 0) //VAMOS: The list of acquisition parameters

};

#endif
