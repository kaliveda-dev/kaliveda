#include "Parameters.h"

//Author: Maurycy Rejmund

using namespace std;

ClassImp(Parameters)

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>Analysisv</h2>
<h4>Handles GANIL acquisition parameters for VAMOS</h4>
<p>
Part of the VAMOS analysis package kindly contributed by Maurycy Rejmund (GANIL).
</p>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

Parameters::Parameters(void) :
#if __cplusplus < 201103L
   fParameterNumber(NULL),
   fParameterType(NULL),
   fParameterBits(NULL),
   fParameterName(NULL),
   fParameterMap(NULL),
   fParameterMapT(NULL),
   fParameterData(NULL),
   fParametersTouched(NULL)
#else
   fParameterNumber(nullptr),
   fParameterType(nullptr),
   fParameterBits(nullptr),
   fParameterName(nullptr),
   fParameterMap(nullptr),
   fParameterMapT(nullptr),
   fParameterData(nullptr),
   fParametersTouched(nullptr)
#endif
{

#ifdef ECHI
   EchiB = false;
#endif
#ifdef STRIPS
   STRB = false;
#endif
#ifdef SED
   SEDB = false;
#endif
#ifdef SI
   SIEB = false;
#endif
#ifdef WCSI
   WCSIEB = false;
#endif
#ifdef INDRA
   ICSILB = ICSIRB = ICSITB = ISILB = ISIHB = ISITB = ICILB = ICIHB = ICITB = false;
#endif
#ifdef ECC
   ECCTB = ECCEB = false;
#endif
#ifdef ECC20MEV
   ECC20EB = false;
#endif
#ifdef GOCCE
   GOCCETB = GOCCEEB = false;
#endif
#ifdef ESS
   ESSTB = ESSQB = false;
#endif
#ifdef CSI
   CSIEB = CSITB = CSIPB = false;
#endif

   fMaxParameterNumber = 0;
   fNumberofParameters = 0;
   fNumberofParametersFree = 0;

   // Allocate the temporary stokage of Parameters

   fParameterNumber = new UShort_t[MAX_PAR_NUM];
   if (!fParameterNumber) {
      cout << "Could not allocate fParameterNumber" << endl;
      exit(EXIT_FAILURE);
   }
   fParameterType = new Short_t[MAX_PAR_NUM];
   if (!fParameterType) {
      cout << "Could not allocate fParameterType" << endl;
      exit(EXIT_FAILURE);
   }
   fParameterBits = new UShort_t[MAX_PAR_NUM];
   if (!fParameterBits) {
      cout << "Could not allocate fParameterBits" << endl;
      exit(EXIT_FAILURE);
   }
   fParameterName = new ParameterName[MAX_PAR_NUM];
   if (!fParameterName) {
      cout << "Could not allocate fParameterName" << endl;
      exit(EXIT_FAILURE);
   }

   for (UShort_t i = 0; i < MAX_PAR_NUM; i++) {
      fParameterNumber[i] = fParameterType[i] = fParameterBits[i] = 0;
      // Allocate memory for parameter names
      fParameterName[i].Init();
   }

}


Parameters::~Parameters(void)
{
   if (fParameterMap) {
      delete [] fParameterMap;
#if __cplusplus < 201103L
      fParameterMap = NULL;
#else
      fParameterMap = nullptr;
#endif
   }

   if (fParameterMapT) {
      delete [] fParameterMapT;
#if __cplusplus < 201103L
      fParameterMapT = NULL;
#else
      fParameterMapT = nullptr;
#endif
   }

   if (fParameterData) {
      delete [] fParameterData;
#if __cplusplus < 201103L
      fParameterData = NULL;
#else
      fParameterData = nullptr;
#endif
   }

   if (fParametersTouched) {
      delete [] fParametersTouched;
#if __cplusplus < 201103L
      fParametersTouched = NULL;
#else
      fParametersTouched = nullptr;
#endif
   }

   if (fParameterNumber) {
      delete[] fParameterNumber;
#if __cplusplus < 201103L
      fParameterNumber = NULL;
#else
      fParameterNumber = nullptr;
#endif
   }

   if (fParameterType) {
      delete[] fParameterType;
#if __cplusplus < 201103L
      fParameterType = NULL;
#else
      fParameterType = nullptr;
#endif
   }
   if (fParameterBits) {
      delete[] fParameterBits;
#if __cplusplus < 201103L
      fParameterBits = NULL;
#else
      fParameterBits = nullptr;
#endif
   }

   if (fParameterName) {
      delete[] fParameterName;
#if __cplusplus < 201103L
      fParameterName = NULL;
#else
      fParameterName = nullptr;
#endif
   }
}



void Parameters::Check(void)
{

   if (
      fNumberofParameters - 1 > MAX_PAR_NUM ||
      fParameterNumber[fNumberofParameters - 1] > MAX_PAR_NUM
   ) {
      cout << "Number of Parameters or Parameter Number too large !" << endl;
      cout << "fParameterName[fNumberofParameters-1] " << fParameterNumber[fNumberofParameters - 1] << " " << fNumberofParameters << endl;
      exit(EXIT_FAILURE);
   }

   if (fParameterNumber[fNumberofParameters - 1] > fMaxParameterNumber)
      fMaxParameterNumber = fParameterNumber[fNumberofParameters - 1];

   int num_csi; //for testing TIARA CsI parameters

#ifdef NOBIDON
   if (
      strncmp(fParameterName[fNumberofParameters - 1].Name, "BIDON", 5) == 0 ||
      strncmp(fParameterName[fNumberofParameters - 1].Name, "BID", 3) == 0 ||
      strncmp(fParameterName[fNumberofParameters - 1].Name, "BIDE", 4) == 0
   )
      fNumberofParameters--; //Removing from the Tree
#endif
#ifdef NOTIARA
   if (
      strncmp(fParameterName[fNumberofParameters - 1].Name, "ADC32", 5) == 0 ||
      strncmp(fParameterName[fNumberofParameters - 1].Name, "S9418", 5) == 0 ||
      strncmp(fParameterName[fNumberofParameters - 1].Name, "V767", 4) == 0

   )
      fNumberofParameters--; //Removing from the Tree
#endif
#ifdef NOV550
   if (
      strncmp(fParameterName[fNumberofParameters - 1].Name, "V550", 4) == 0
   )
      fNumberofParameters--; //Removing from the Tree
#endif
#ifdef NOADC
   else if (
      strncmp(fParameterName[fNumberofParameters - 1].Name, "ADC_", 4) == 0
   )
      fNumberofParameters--; //Removing from the Tree
#endif
#ifdef NOADC2
   else if (
      strncmp(fParameterName[fNumberofParameters - 1].Name, "ADC2", 4) == 0
   )
      fNumberofParameters--; //Removing from the Tree
#endif
#ifdef NOADC6414
   else if (
      strncmp(fParameterName[fNumberofParameters - 1].Name, "ADC6414", 7) == 0
   )
      fNumberofParameters--; //Removing from the Tree
#endif
#ifdef NOTDC
   else if (
      strncmp((fParameterName[fNumberofParameters - 1]).Name, "TDC_", 4) == 0
   )
      fNumberofParameters--; //Removing from the Tree
#endif
#ifdef NOTDC2
   else if (
      strncmp((fParameterName[fNumberofParameters - 1]).Name, "TDC2_", 5) == 0
   )
      fNumberofParameters--; //Removing from the Tree
#endif
#ifdef NOQDC
   else if (
      strncmp((fParameterName[fNumberofParameters - 1]).Name, "QDC_", 4) == 0
   )
      fNumberofParameters--; //Removing from the Tree
#endif
#ifdef NOCENTRUM
   else if (
      strncmp(fParameterName[fNumberofParameters - 1].Name, "Centr", 5) == 0 ||
      strncmp(fParameterName[fNumberofParameters - 1].Name, "CENTR", 5) == 0
   )
      fNumberofParameters--; //Removing from the Tree
#endif
#ifdef NOTRIG
   else if (
      strncmp(fParameterName[fNumberofParameters - 1].Name, "Trig", 4) == 0 ||
      strncmp(fParameterName[fNumberofParameters - 1].Name, "TRIG", 4) == 0
   )
      fNumberofParameters--; //Removing from the Tree
#endif
#ifdef NOSED
   else if (
      strncmp(fParameterName[fNumberofParameters - 1].Name, "SED", 3) == 0
   )
      fNumberofParameters--; //Removing from the Tree
#endif
#ifdef NOESS
   else if (
      strncmp(fParameterName[fNumberofParameters - 1].Name, "ESS", 3) == 0
   )
      fNumberofParameters--; //Removing from the Tree
#endif
#ifdef NOGOCCE
   else if (
      strncmp(fParameterName[fNumberofParameters - 1].Name, "GOCCE", 5) == 0
   )
      fNumberofParameters--; //Removing from the Tree
#endif
#ifdef NOECC
   else if (
      strncmp(fParameterName[fNumberofParameters - 1].Name, "ECC", 3) == 0
   )
      fNumberofParameters--; //Removing from the Tree
#endif
#ifdef NOECC20MEV
   else if (
      strncmp(fParameterName[fNumberofParameters - 1].Name, "ECC", 3) == 0 &&
      (
         strncmp(fParameterName[fNumberofParameters - 1].Name + 7, "20MEV", 5) == 0 ||
         strncmp(fParameterName[fNumberofParameters - 1].Name + 7, "20MeV", 5) == 0 ||
         strncmp(fParameterName[fNumberofParameters - 1].Name + 8, "20MEV", 5) == 0 ||
         strncmp(fParameterName[fNumberofParameters - 1].Name + 8, "20MeV", 5) == 0
      )
   )
      fNumberofParameters--; //Removing from the Tree
#endif
#ifdef NOSELECTOR
   else if (
      strncmp(fParameterName[fNumberofParameters - 1].Name, "CONFIG", 6) == 0 ||
      strncmp(fParameterName[fNumberofParameters - 1].Name, "R_DEC", 5) == 0 ||
      strncmp(fParameterName[fNumberofParameters - 1].Name, "STAT_EVE", 8) == 0
   )
      fNumberofParameters--; //Removing from the Tree
#endif
#ifdef NOETALON
   else if (
      strncmp(fParameterName[fNumberofParameters - 1].Name, "SI75_", 5) == 0 ||
      strncmp(fParameterName[fNumberofParameters - 1].Name, "SILI_", 5) == 0
   )
      fNumberofParameters--; //Removing from the Tree
#endif
#ifdef NOPILA
   else if (
      strncmp(fParameterName[fNumberofParameters - 1].Name, "PILA_", 5) == 0
   )
      fNumberofParameters--; //Removing from the Tree
#endif
#ifdef NOMT4
   else if (
      strncmp(fParameterName[fNumberofParameters - 1].Name, "MT4_", 4) == 0
   )
      fNumberofParameters--; //Removing from the Tree
#endif
#ifdef STRIPS
   else if (
      strncmp(fParameterName[fNumberofParameters - 1].Name, "STR_11", 6) == 0
   ) {
      fParameterType[fNumberofParameters - 1] = 4;
      STRB = true;
   } else if (
      strncmp(fParameterName[fNumberofParameters - 1].Name, "STR_12", 6) == 0
   ) {
      fParameterType[fNumberofParameters - 1] = 5;
      STRB = true;
   } else if (
      strncmp(fParameterName[fNumberofParameters - 1].Name, "STR_21", 6) == 0
   ) {
      fParameterType[fNumberofParameters - 1] = 6;
      STRB = true;
   } else if (
      strncmp(fParameterName[fNumberofParameters - 1].Name, "STR_22", 6) == 0
   ) {
      fParameterType[fNumberofParameters - 1] = 7;
      STRB = true;
   }
#endif


#ifdef ECHI
   else if (
      strncmp(fParameterName[fNumberofParameters - 1].Name, "ECHI_A", 6) == 0
      &&
      strncmp(fParameterName[fNumberofParameters - 1].Name + 8, "OLD", 3) == 0
   ) {
      fNumberofParameters--;   //Removing from the Tree
   } else if (
      strncmp(fParameterName[fNumberofParameters - 1].Name, "ECHI_A", 6) == 0
   ) {
      fParameterType[fNumberofParameters - 1] = 1;
      EchiB = true;
   } else if (
      strncmp(fParameterName[fNumberofParameters - 1].Name, "ECHI_B", 6) == 0
   ) {
      fParameterType[fNumberofParameters - 1] = 2;
      EchiB = true;
   } else if (
      strncmp(fParameterName[fNumberofParameters - 1].Name, "ECHI_C", 6) == 0
   ) {
      fParameterType[fNumberofParameters - 1] = 3;
      EchiB = true;
   }
#endif

#ifdef ECC
   else if (
      strncmp((fParameterName[fNumberofParameters - 1]).Name, "ECC", 3) == 0 &&
      (
         strncmp(fParameterName[fNumberofParameters - 1].Name + 7, "6MeV", 4) == 0 ||
         strncmp(fParameterName[fNumberofParameters - 1].Name + 7, "6MEV", 4) == 0 ||
         strncmp(fParameterName[fNumberofParameters - 1].Name + 8, "6MeV", 4) == 0 ||
         strncmp(fParameterName[fNumberofParameters - 1].Name + 8, "6MEV", 4) == 0
      )
   ) {
      fParameterType[fNumberofParameters - 1] = 8;
      ECCEB = true;
   } else if (
      strncmp(fParameterName[fNumberofParameters - 1].Name, "ECC", 3) == 0 &&
      (
         strncmp(fParameterName[fNumberofParameters - 1].Name + 7, "TAC", 3) == 0 ||
         strncmp(fParameterName[fNumberofParameters - 1].Name + 8, "TAC", 3) == 0
      )
   ) {
      fParameterType[fNumberofParameters - 1] = 9;
      ECCTB = true;
   }
#endif
#ifdef ECC20MEV
   else if (
      strncmp((fParameterName[fNumberofParameters - 1]).Name, "ECC", 3) == 0 &&
      (
         strncmp(fParameterName[fNumberofParameters - 1].Name + 7, "20MeV", 5) == 0 ||
         strncmp(fParameterName[fNumberofParameters - 1].Name + 7, "20MEV", 5) == 0 ||
         strncmp(fParameterName[fNumberofParameters - 1].Name + 8, "20MeV", 5) == 0 ||
         strncmp(fParameterName[fNumberofParameters - 1].Name + 8, "20MEV", 5) == 0
      )
   ) {
      fParameterType[fNumberofParameters - 1] = 23;
      ECC20EB = true;
   }
#endif


#ifdef GOCCE
   else if (
      strncmp((fParameterName[fNumberofParameters - 1]).Name, "GOCCE", 5) == 0 &&
      (
         strncmp(fParameterName[fNumberofParameters - 1].Name + 11, "E", 1) == 0 ||
         strncmp(fParameterName[fNumberofParameters - 1].Name + 12, "E", 1) == 0
      )
   ) {
      fParameterType[fNumberofParameters - 1] = 21;
      GOCCEEB = true;
   } else if (
      strncmp(fParameterName[fNumberofParameters - 1].Name, "GOCCE", 5) == 0 &&
      (
         strncmp(fParameterName[fNumberofParameters - 1].Name + 11, "T", 1) == 0 ||
         strncmp(fParameterName[fNumberofParameters - 1].Name + 12, "T", 1) == 0
      )
   ) {
      fParameterType[fNumberofParameters - 1] = 22;
      GOCCETB = true;
   }
#endif
#ifdef SI
   else if (
      strncmp((fParameterName[fNumberofParameters - 1]).Name, "SIE", 3) == 0
   ) {
      fParameterType[fNumberofParameters - 1] = 19;
      SIEB = true;
   }
#endif

#ifdef INDRA
   else if (
      strncmp((fParameterName[fNumberofParameters - 1]).Name, "CSI_", 4) == 0 &&
      strncmp((fParameterName[fNumberofParameters - 1]).Name + 8, "_L", 2) == 0
   ) {
      fParameterType[fNumberofParameters - 1] = 25;
      ICSILB = true;
   } else if (
      strncmp((fParameterName[fNumberofParameters - 1]).Name, "CSI_", 4) == 0 &&
      strncmp((fParameterName[fNumberofParameters - 1]).Name + 8, "_R", 2) == 0
   ) {
      fParameterType[fNumberofParameters - 1] = 26;
      ICSIRB = true;
   } else if (
      strncmp((fParameterName[fNumberofParameters - 1]).Name, "CSI_", 4) == 0 &&
      strncmp((fParameterName[fNumberofParameters - 1]).Name + 8, "_T", 2) == 0
   ) {
      fParameterType[fNumberofParameters - 1] = 27;
      ICSITB = true;
   } else if (
      strncmp((fParameterName[fNumberofParameters - 1]).Name, "SI_", 3) == 0 &&
      strncmp((fParameterName[fNumberofParameters - 1]).Name + 7, "_PG", 3) == 0
   ) {
      fParameterType[fNumberofParameters - 1] = 28;
      ISILB = true;
   } else if (
      strncmp((fParameterName[fNumberofParameters - 1]).Name, "SI_", 3) == 0 &&
      strncmp((fParameterName[fNumberofParameters - 1]).Name + 7, "_GG", 3) == 0
   ) {
      fParameterType[fNumberofParameters - 1] = 29;
      ISIHB = true;
   } else if (
      strncmp((fParameterName[fNumberofParameters - 1]).Name, "SI_", 3) == 0 &&
      strncmp((fParameterName[fNumberofParameters - 1]).Name + 7, "_T", 2) == 0
   ) {
      fParameterType[fNumberofParameters - 1] = 30;
      ISITB = true;
   } else if (
      strncmp((fParameterName[fNumberofParameters - 1]).Name, "CI_", 3) == 0 &&
      strncmp((fParameterName[fNumberofParameters - 1]).Name + 7, "_PG", 3) == 0
   ) {
      fParameterType[fNumberofParameters - 1] = 31;
      ICILB = true;
   } else if (
      strncmp((fParameterName[fNumberofParameters - 1]).Name, "CI_", 3) == 0 &&
      strncmp((fParameterName[fNumberofParameters - 1]).Name + 7, "_GG", 3) == 0
   ) {
      fParameterType[fNumberofParameters - 1] = 32;
      ICIHB = true;
   } else if (
      strncmp((fParameterName[fNumberofParameters - 1]).Name, "CI_", 3) == 0 &&
      strncmp((fParameterName[fNumberofParameters - 1]).Name + 7, "_T", 2) == 0
   ) {
      fParameterType[fNumberofParameters - 1] = 33;
      ICITB = true;
   }
#endif

#ifdef WCSI
   else if (
      strncmp((fParameterName[fNumberofParameters - 1]).Name, "ADC-CSI", 7) == 0
   ) {
      fParameterType[fNumberofParameters - 1] = 24;
      WCSIEB = true;
   } else if (
      strncmp((fParameterName[fNumberofParameters - 1]).Name, "CSI", 3) == 0
      && sscanf((fParameterName[fNumberofParameters - 1]).Name, "CSI%02d", &num_csi) == 1
   ) {
      fParameterType[fNumberofParameters - 1] = 34;
      WCSIEB = true;
   }
#endif

#ifdef ESS
   else if (
      strncmp(fParameterName[fNumberofParameters - 1].Name, "ESS", 3) == 0 &&
      (
         strncmp(fParameterName[fNumberofParameters - 1].Name + 5, "TDC", 3) == 0 ||
         strncmp(fParameterName[fNumberofParameters - 1].Name + 6, "TDC", 3) == 0
      ) &&
      (
         strncmp(fParameterName[fNumberofParameters - 1].Name + 9, "Q", 1) == 0 ||
         strncmp(fParameterName[fNumberofParameters - 1].Name + 10, "Q", 1) == 0
      )
   ) {
      fParameterType[fNumberofParameters - 1] = 10;
      ESSQB = true;
   } else if (
      strncmp(fParameterName[fNumberofParameters - 1].Name, "ESS", 3) == 0 &&
      (
         strncmp(fParameterName[fNumberofParameters - 1].Name + 5, "TDC", 3) == 0 ||
         strncmp(fParameterName[fNumberofParameters - 1].Name + 6, "TDC", 3) == 0
      )
   ) {
      fParameterType[fNumberofParameters - 1] = 11;
      ESSTB = true;
   } else if (
      strncmp(fParameterName[fNumberofParameters - 1].Name, "ESS", 3) == 0 &&
      (
         strncmp(fParameterName[fNumberofParameters - 1].Name + 5, "PAT", 3) == 0 ||
         strncmp(fParameterName[fNumberofParameters - 1].Name + 6, "PAT", 3) == 0 ||
         strncmp(fParameterName[fNumberofParameters - 1].Name + 5, "ENE", 3) == 0 ||
         strncmp(fParameterName[fNumberofParameters - 1].Name + 6, "ENE", 3) == 0
      )
   )
      fNumberofParameters--; //Removing from the Tree
#endif
#ifdef SED
   else if (
      strncmp(fParameterName[fNumberofParameters - 1].Name, "SED1_X_", 7) == 0
   ) {
      fParameterType[fNumberofParameters - 1] = 12;
      SEDB = true;
   } else if (
      strncmp(fParameterName[fNumberofParameters - 1].Name, "SED1_Y_", 7) == 0
   ) {
      fParameterType[fNumberofParameters - 1] = 13;
      SEDB = true;
   } else if (
      strncmp(fParameterName[fNumberofParameters - 1].Name, "SED2_X_", 7) == 0
   ) {
      fParameterType[fNumberofParameters - 1] = 14;
      SEDB = true;
   } else if (
      strncmp(fParameterName[fNumberofParameters - 1].Name, "SED2_Y_", 7) == 0
   ) {
      fParameterType[fNumberofParameters - 1] = 15;
      SEDB = true;
   }
#endif
#ifdef CSI
   else if ((
               strncmp((fParameterName[fNumberofParameters - 1]).Name, "CsI", 3) == 0
               ||
               strncmp((fParameterName[fNumberofParameters - 1]).Name, "CSI", 3) == 0
            ) &&
            (
               strncmp(fParameterName[fNumberofParameters - 1].Name + 5, "Ene", 3) == 0 ||
               strncmp(fParameterName[fNumberofParameters - 1].Name + 5, "ENE", 3) == 0 ||
               strncmp(fParameterName[fNumberofParameters - 1].Name + 6, "Ene", 3) == 0 ||
               strncmp(fParameterName[fNumberofParameters - 1].Name + 6, "ENE", 3) == 0
            )
           ) {
      fParameterType[fNumberofParameters - 1] = 16;
      CSIEB = true;
   } else if ((
                 strncmp((fParameterName[fNumberofParameters - 1]).Name, "CsI", 3) == 0
                 ||
                 strncmp((fParameterName[fNumberofParameters - 1]).Name, "CSI", 3) == 0
              ) &&
              (
                 strncmp(fParameterName[fNumberofParameters - 1].Name + 5, "Tim", 3) == 0 ||
                 strncmp(fParameterName[fNumberofParameters - 1].Name + 5, "TIM", 3) == 0 ||
                 strncmp(fParameterName[fNumberofParameters - 1].Name + 6, "TIM", 3) == 0 ||
                 strncmp(fParameterName[fNumberofParameters - 1].Name + 6, "Tim", 3) == 0
              )
             ) {
      fParameterType[fNumberofParameters - 1] = 17;
      CSITB = true;
   } else if ((
                 strncmp((fParameterName[fNumberofParameters - 1]).Name, "CsI", 3) == 0
                 ||
                 strncmp((fParameterName[fNumberofParameters - 1]).Name, "CSI", 3) == 0
              ) &&
              (
                 strncmp(fParameterName[fNumberofParameters - 1].Name + 5, "PI", 2) == 0 ||
                 strncmp(fParameterName[fNumberofParameters - 1].Name + 6, "PI", 2) == 0
              )
             ) {
      fParameterType[fNumberofParameters - 1] = 18;
      CSIPB = true;
   }
#endif
   else {
      fParameterType[fNumberofParameters - 1] = -1;
      fNumberofParametersFree++;
   }
}

//____________________________________________________________________________________________//

void Parameters::Set(TTree* theTree)
{
   TString parName;
   TString parType;
   UShort_t Index = 0;
   UShort_t Index1;

   fMaxParameterNumber++;

   fParameterMap = new UShort_t[fMaxParameterNumber];
   if (!fParameterMap) {
      cout << "Could not allocate fParameterMap" << endl;
      exit(EXIT_FAILURE);
   }
   fParameterMapT = new Short_t[fMaxParameterNumber];
   if (!fParameterMapT) {
      cout << "Could not allocate fParameterMapT" << endl;
      exit(EXIT_FAILURE);
   }
   fParameterData = new UShort_t[fNumberofParametersFree];
   if (!fParameterData) {
      cout << "Could not allocate fParameterData" << endl;
      exit(EXIT_FAILURE);
   }
   fParametersTouched = new UShort_t[fNumberofParametersFree];
   if (!fParametersTouched) {
      cout << "Could not allocate fParametersTouched" << endl;
      exit(EXIT_FAILURE);
   }
   for (UShort_t i = 0; i < fMaxParameterNumber; i++) {
      fParameterMap[i] = 0;
      fParameterMapT[i] = 0;
   }

   for (UShort_t i = 0; i < fNumberofParametersFree; i++) {
      fParameterData[i] = 0;
      fParametersTouched[i] = 0;
   }

   fParametersM = 0;



   UShort_t Pos;
   Pos = 0;
#ifdef ECHI
   if (EchiB) {
      EchiM = 0;
      theTree->Branch("EchiM", &EchiM, "EchiM/I");
      theTree->Branch("Echi", Echi, "Echi[EchiM]/s");
      theTree->Branch("EchiNr", EchiNr, "EchiNr[EchiM]/s");
   }
#endif
#ifdef SED
   if (SEDB) {
      SEDMX1 = SEDMY1 = SEDMX2 = SEDMY2 = 0;
      theTree->Branch("SEDMX1", &SEDMX1, "SEDMX1/I");
      theTree->Branch("SEDMY1", &SEDMY1, "SEDMY1/I");
      theTree->Branch("SEDMX2", &SEDMX2, "SEDMX2/I");
      theTree->Branch("SEDMY2", &SEDMY2, "SEDMY2/I");
      theTree->Branch("SEDX1", SEDQ + 0, "SEDQ[SEDMX1]/s");
      theTree->Branch("SEDX2", SEDQ + 128, "SEDQ[SEDMX2]/s");
      theTree->Branch("SEDY1", SEDQ + 2 * 128, "SEDQ[SEDMY1]/s");
      theTree->Branch("SEDY2", SEDQ + 3 * 128, "SEDQ[SEDMY2]/s");
      theTree->Branch("SEDNrX1", SEDNr + 0, "SEDNrX1[SEDMX1]/s");
      theTree->Branch("SEDNrX2", SEDNr + 128, "SEDNrX2[SEDMX2]/s");
      theTree->Branch("SEDNrY1", SEDNr + 2 * 128, "SEDNrY1[SEDMY1]/s");
      theTree->Branch("SEDNrY2", SEDNr + 3 * 128, "SEDNrY2[SEDMY2]/s");
   }
#endif
#ifdef STRIPS
   if (STRB) {
      STRM1 = STRM2 = STRM3 = STRM4 = 0;
      theTree->Branch("STRM1", &STRM1, "STRM1/I");
      theTree->Branch("STRM2", &STRM2, "STRM2/I");
      theTree->Branch("STRM3", &STRM3, "STRM3/I");
      theTree->Branch("STRM4", &STRM4, "STRM4/I");
      theTree->Branch("STR1", STR + 0, "STR[STRM1]/s");
      theTree->Branch("STR2", STR + 64, "STR[STRM2]/s");
      theTree->Branch("STR3", STR + 2 * 64, "STR[STRM3]/s");
      theTree->Branch("STR4", STR + 3 * 64, "STR[STRM4]/s");
      theTree->Branch("STRNr1", STRNr + 0, "STRNr[STRM1]/s");
      theTree->Branch("STRNr2", STRNr + 64, "STRNr[STRM2]/s");
      theTree->Branch("STRNr3", STRNr + 2 * 64, "STRNr[STRM3]/s");
      theTree->Branch("STRNr4", STRNr + 3 * 64, "STRNr[STRM4]/s");
   }
#endif
#ifdef ECC
   if (ECCTB) {
      ECCTM = 0;
      theTree->Branch("ECCTM", &ECCTM, "ECCTM/I");
      theTree->Branch("ECCT", &ECCT, "ECCT[ECCTM]/s");
      theTree->Branch("ECCTNr", &ECCTNr, "ECCTNr[ECCTM]/s");
   }
   if (ECCEB) {
      ECCEM = 0;
      theTree->Branch("ECCEM", &ECCEM, "ECCEM/I");
      theTree->Branch("ECCE", &ECCE, "ECCE[ECCEM]/s");
      theTree->Branch("ECCENr", &ECCENr, "ECCENr[ECCEM]/s");
   }
#endif
#ifdef ECC20MEV
   if (ECC20EB) {
      ECC20EM = 0;
      theTree->Branch("ECC20EM", &ECC20EM, "ECC20EM/I");
      theTree->Branch("ECC20E", &ECC20E, "ECC20E[ECC20EM]/s");
      theTree->Branch("ECC20ENr", &ECC20ENr, "ECC20ENr[ECC20EM]/s");
   }
#endif
#ifdef GOCCE
   if (GOCCETB) {
      GOCCETM = 0;
      theTree->Branch("GOCCETM", &GOCCETM, "GOCCETM/I");
      theTree->Branch("GOCCET", &GOCCET, "GOCCET[GOCCETM]/s");
      theTree->Branch("GOCCETNr", &GOCCETNr, "GOCCETNr[GOCCETM]/s");
   }
   if (GOCCEEB) {
      GOCCEEM = 0;
      theTree->Branch("GOCCEEM", &GOCCEEM, "GOCCEEM/I");
      theTree->Branch("GOCCEE", &GOCCEE, "GOCCEE[GOCCEEM]/s");
      theTree->Branch("GOCCEENr", &GOCCEENr, "GOCCEENr[GOCCEEM]/s");
   }
#endif

#ifdef ESS
   if (ESSTB) {
      ESSTM = 0;
      theTree->Branch("ESSTM", &ESSTM, "ESSTM/I");
      theTree->Branch("ESST", &ESST, "ESST[ESSTM]/s");
      theTree->Branch("ESSTNr", &ESSTNr, "ESSTNr[ESSTM]/s");
   }
   if (ESSQB) {
      ESSQM = 0;
      theTree->Branch("ESSQM", &ESSQM, "ESSQM/I");
      theTree->Branch("ESSQ", &ESSQ, "ESSQ[ESSQM]/s");
      theTree->Branch("ESSQNr", &ESSQNr, "ESSQNr[ESSQM]/s");
   }
#endif


#ifdef SI
   if (SIEB) {
      SIEM = 0;
      theTree->Branch("SIEM", &SIEM, "SIEM/I");
      theTree->Branch("SIE", &SIE, "SIE[SIEM]/s");
      theTree->Branch("SIENr", &SIENr, "SIENr[SIEM]/s");
   }
#endif

#ifdef WCSI
   if (WCSIEB) {
      WCSIEM = 0;
      theTree->Branch("WCSIEM", &WCSIEM, "WCSIEM/I");
      theTree->Branch("WCSIE", &WCSIE, "WCSIE[WCSIEM]/s");
      theTree->Branch("WCSIENr", &WCSIENr, "WCSIENr[WCSIEM]/s");
   }
#endif

#ifdef INDRA
   if (ICSILB) {
      ICSILM = 0;
      theTree->Branch("ICSILM", &ICSILM, "ICSILM/I");
      theTree->Branch("ICSIL", &ICSIL, "ICSIL[ICSILM]/s");
      theTree->Branch("ICSILNr", &ICSILNr, "ICSILNr[ICSILM]/s");
   }
   if (ICSIRB) {
      ICSIRM = 0;
      theTree->Branch("ICSIRM", &ICSIRM, "ICSIRM/I");
      theTree->Branch("ICSIR", &ICSIR, "ICSIR[ICSIRM]/s");
      theTree->Branch("ICSIRNr", &ICSIRNr, "ICSIRNr[ICSIRM]/s");
   }
   if (ICSITB) {
      ICSITM = 0;
      theTree->Branch("ICSITM", &ICSITM, "ICSITM/I");
      theTree->Branch("ICSIT", &ICSIT, "ICSIT[ICSITM]/s");
      theTree->Branch("ICSITNr", &ICSITNr, "ICSITNr[ICSITM]/s");
   }

   if (ISILB) {
      ISILM = 0;
      theTree->Branch("ISILM", &ISILM, "ISILM/I");
      theTree->Branch("ISIL", &ISIL, "ISIL[ISILM]/s");
      theTree->Branch("ISILNr", &ISILNr, "ISILNr[ISILM]/s");
   }
   if (ISIHB) {
      ISIHM = 0;
      theTree->Branch("ISIHM", &ISIHM, "ISIHM/I");
      theTree->Branch("ISIH", &ISIH, "ISIH[ISIHM]/s");
      theTree->Branch("ISIHNr", &ISIHNr, "ISIHNr[ISIHM]/s");
   }
   if (ISITB) {
      ISITM = 0;
      theTree->Branch("ISITM", &ISITM, "ISITM/I");
      theTree->Branch("ISIT", &ISIT, "ISIT[ISITM]/s");
      theTree->Branch("ISITNr", &ISITNr, "ISITNr[ISITM]/s");
   }


   if (ICILB) {
      ICILM = 0;
      theTree->Branch("ICILM", &ICILM, "ICILM/I");
      theTree->Branch("ICIL", &ICIL, "ICIL[ICILM]/s");
      theTree->Branch("ICILNr", &ICILNr, "ICILNr[ICILM]/s");
   }
   if (ICIHB) {
      ICIHM = 0;
      theTree->Branch("ICIHM", &ICIHM, "ICIHM/I");
      theTree->Branch("ICIH", &ICIH, "ICIH[ICIHM]/s");
      theTree->Branch("ICIHNr", &ICIHNr, "ICIHNr[ICIHM]/s");
   }
   if (ICITB) {
      ICITM = 0;
      theTree->Branch("ICITM", &ICITM, "ICITM/I");
      theTree->Branch("ICIT", &ICIT, "ICIT[ICITM]/s");
      theTree->Branch("ICITNr", &ICITNr, "ICITNr[ICITM]/s");
   }

#endif

#ifdef CSI
   if (CSIEB) {
      CSIEM = 0;
      theTree->Branch("CSIEM", &CSIEM, "CSIEM/I");
      theTree->Branch("CSIE", &CSIE, "CSIE[CSIEM]/s");
      theTree->Branch("CSIENr", &CSIENr, "CSIENr[CSIEM]/s");
   }
   if (CSITB) {
      CSITM = 0;
      theTree->Branch("CSITM", &CSITM, "CSITM/I");
      theTree->Branch("CSIT", &CSIT, "CSIT[CSITM]/s");
      theTree->Branch("CSITNr", &CSITNr, "CSITNr[CSITM]/s");
   }
   if (CSIPB) {
      CSIPM = 0;
      theTree->Branch("CSIPM", &CSIPM, "CSIPM/I");
      theTree->Branch("CSIP", &CSIP, "CSIP[CSIPM]/s");
      theTree->Branch("CSIPNr", &CSIPNr, "CSIPNr[CSIPM]/s");
   }
#endif


   int tmp;
   int tmp1;

   for (UShort_t i = 0; i < fNumberofParameters; i++) {
      fParameterMapT[fParameterNumber[i]] = fParameterType[i];
      switch (fParameterType[i]) {
         case -1:

            fParameterMap[fParameterNumber[i]] = Pos;
            parName = fParameterName[i].Name;
            parType = parName + "/s";
            theTree->Branch(parName, fParameterData + Pos, parType);
            cout << "BRANCH : param#" << i << "   " << parName << endl;
            Pos++;
            break;

#ifdef ECHI
         case 1:
            sscanf(fParameterName[i].Name + 7, "%1d", &tmp);
            fParameterMap[fParameterNumber[i]] = tmp;
            fParameterMap[fParameterNumber[i]] += -1;
            break;

         case 2:

            sscanf(fParameterName[i].Name + 7, "%1d", &tmp);
            fParameterMap[fParameterNumber[i]] = tmp;
            fParameterMap[fParameterNumber[i]] += 8 - 1;
            break;

         case 3:
            sscanf(fParameterName[i].Name + 7, "%1d", &tmp);
            fParameterMap[fParameterNumber[i]] = tmp;;
            fParameterMap[fParameterNumber[i]] += 2 * 8 - 1;
            break;
#endif
#ifdef STRIPS
         case 4:
            sscanf(fParameterName[i].Name + 7, "%02d", &tmp);
            fParameterMap[fParameterNumber[i]] = tmp;
            fParameterMap[fParameterNumber[i]] += -1;
            break;

         case 5:
            sscanf(fParameterName[i].Name + 7, "%02d", &tmp);
            fParameterMap[fParameterNumber[i]] = tmp;
            fParameterMap[fParameterNumber[i]] += -1;
            break;

         case 6:
            sscanf(fParameterName[i].Name + 7, "%02d", &tmp);
            fParameterMap[fParameterNumber[i]] = tmp;
            fParameterMap[fParameterNumber[i]] += -1;
            break;

         case 7:
            sscanf(fParameterName[i].Name + 7, "%02d", &tmp);
            fParameterMap[fParameterNumber[i]] = tmp;;
            fParameterMap[fParameterNumber[i]] += -1;
            break;
#endif
#ifdef SED
         case 12:
         case 13:
         case 14:
         case 15:
            sscanf(fParameterName[i].Name + 7, "%03d", &tmp);
            fParameterMap[fParameterNumber[i]] = tmp;
            fParameterMap[fParameterNumber[i]] += -1;
            break;
#endif
#ifdef ECC
         case 8:
            if (strncmp((fParameterName[i]).Name + 6, "_", 1) == 0) {
               sscanf(fParameterName[i].Name + 3, "%1d", &tmp);
               fParameterMap[fParameterNumber[i]] = tmp;
               if (strncmp((fParameterName[i]).Name + 5, "A", 1) == 0)
                  Index = 0;
               else if (strncmp(fParameterName[i].Name + 5, "B", 1) == 0)
                  Index = 1;
               else if (strncmp(fParameterName[i].Name + 5, "C", 1) == 0)
                  Index = 2;
               else if (strncmp(fParameterName[i].Name + 5, "D", 1) == 0)
                  Index = 3;
               else {
                  cout << "Unknown Index " << Index << endl;
                  exit(EXIT_FAILURE);
               }
            } else {
               sscanf(fParameterName[i].Name + 3, "%2d", &tmp);
               fParameterMap[fParameterNumber[i]] = tmp;
               if (strncmp(fParameterName[i].Name + 6, "A", 1) == 0)
                  Index = 0;
               else if (strncmp(fParameterName[i].Name + 6, "B", 1) == 0)
                  Index = 1;
               else if (strncmp(fParameterName[i].Name + 6, "C", 1) == 0)
                  Index = 2;
               else if (strncmp(fParameterName[i].Name + 6, "D", 1) == 0)
                  Index = 3;
               else {
                  cout << "Unknown Index " << Index << endl;
                  exit(EXIT_FAILURE);
               }
            }
            fParameterMap[fParameterNumber[i]] *= 4;
            fParameterMap[fParameterNumber[i]] += Index;
            break;
         case 9:
            if (strncmp(fParameterName[i].Name + 6, "_", 1) == 0) {
               sscanf(fParameterName[i].Name + 3, "%1d", &tmp);
               fParameterMap[fParameterNumber[i]] = tmp;
               if (strncmp(fParameterName[i].Name + 5, "A", 1) == 0)
                  Index = 0;
               else if (strncmp(fParameterName[i].Name + 5, "B", 1) == 0)
                  Index = 1;
               else if (strncmp(fParameterName[i].Name + 5, "C", 1) == 0)
                  Index = 2;
               else if (strncmp(fParameterName[i].Name + 5, "D", 1) == 0)
                  Index = 3;
               else {
                  cout << "Unknown Index " << fParameterName[i].Name << Index << endl;
                  exit(EXIT_FAILURE);
               }
            } else {
               sscanf(fParameterName[i].Name + 3, "%2d", &tmp);
               fParameterMap[fParameterNumber[i]] = tmp;
               if (strncmp((fParameterName[i]).Name + 6, "A", 1) == 0)
                  Index = 0;
               else if (strncmp(fParameterName[i].Name + 6, "B", 1) == 0)
                  Index = 1;
               else if (strncmp(fParameterName[i].Name + 6, "C", 1) == 0)
                  Index = 2;
               else if (strncmp(fParameterName[i].Name + 6, "D", 1) == 0)
                  Index = 3;
               else {
                  cout << "Unknown Index " << Index << endl;
                  exit(EXIT_FAILURE);
               }
            }
            fParameterMap[fParameterNumber[i]] *= 4;
            fParameterMap[fParameterNumber[i]] += Index;
            break;
#endif
#ifdef ECC20MEV
         case 23:
            if (strncmp((fParameterName[i]).Name + 6, "_", 1) == 0) {
               sscanf(fParameterName[i].Name + 3, "%1d", &tmp);
               fParameterMap[fParameterNumber[i]] = tmp;
               if (strncmp((fParameterName[i]).Name + 5, "A", 1) == 0)
                  Index = 0;
               else if (strncmp(fParameterName[i].Name + 5, "B", 1) == 0)
                  Index = 1;
               else if (strncmp(fParameterName[i].Name + 5, "C", 1) == 0)
                  Index = 2;
               else if (strncmp(fParameterName[i].Name + 5, "D", 1) == 0)
                  Index = 3;
               else {
                  cout << "Unknown Index " << Index << endl;
                  exit(EXIT_FAILURE);
               }
            } else {
               sscanf(fParameterName[i].Name + 3, "%2d", &tmp);
               fParameterMap[fParameterNumber[i]] = tmp;
               if (strncmp(fParameterName[i].Name + 6, "A", 1) == 0)
                  Index = 0;
               else if (strncmp(fParameterName[i].Name + 6, "B", 1) == 0)
                  Index = 1;
               else if (strncmp(fParameterName[i].Name + 6, "C", 1) == 0)
                  Index = 2;
               else if (strncmp(fParameterName[i].Name + 6, "D", 1) == 0)
                  Index = 3;
               else {
                  cout << "Unknown Index " << Index << endl;
                  exit(EXIT_FAILURE);
               }
            }
            fParameterMap[fParameterNumber[i]] *= 4;
            fParameterMap[fParameterNumber[i]] += Index;
            break;
#endif

#ifdef ESS
         case 10:
            if (strncmp((fParameterName[i]).Name + 4, "_", 1) == 0) {
               sscanf(fParameterName[i].Name + 3, "%1d", &tmp);
               fParameterMap[fParameterNumber[i]] = tmp;
               if (strncmp((fParameterName[i]).Name + 10, "A", 1) == 0)
                  Index = 0;
               else if (strncmp(fParameterName[i].Name + 10, "B", 1) == 0)
                  Index = 1;
               else if (strncmp(fParameterName[i].Name + 10, "C", 1) == 0)
                  Index = 2;
               else if (strncmp(fParameterName[i].Name + 10, "D", 1) == 0)
                  Index = 3;
               else {
                  cout << "Unknown Index " << Index << endl;
                  exit(EXIT_FAILURE);
               }
            } else {
               sscanf(fParameterName[i].Name + 3, "%2d", &tmp);
               fParameterMap[fParameterNumber[i]] = tmp;
               if (strncmp(fParameterName[i].Name + 11, "A", 1) == 0)
                  Index = 0;
               else if (strncmp(fParameterName[i].Name + 11, "B", 1) == 0)
                  Index = 1;
               else if (strncmp(fParameterName[i].Name + 11, "C", 1) == 0)
                  Index = 2;
               else if (strncmp(fParameterName[i].Name + 11, "D", 1) == 0)
                  Index = 3;
               else {
                  cout << "Unknown Index " << Index << endl;
                  exit(EXIT_FAILURE);
               }
            }
            fParameterMap[fParameterNumber[i]] *= 4;
            fParameterMap[fParameterNumber[i]] += Index;
            break;
         case 11:
            if (strncmp(fParameterName[i].Name + 4, "_", 1) == 0) {
               sscanf(fParameterName[i].Name + 3, "%1d", &tmp);
               fParameterMap[fParameterNumber[i]] = tmp;
            } else {
               sscanf(fParameterName[i].Name + 3, "%2d", &tmp);
               fParameterMap[fParameterNumber[i]] = tmp;
            }
            break;
#endif



#ifdef GOCCE
         case 21:
            if (strncmp((fParameterName[i]).Name + 6, "_", 1) == 0) {
               sscanf(fParameterName[i].Name + 5, "%1d", &tmp);
               fParameterMap[fParameterNumber[i]] = tmp;
               if (strncmp((fParameterName[i]).Name + 7, "A", 1) == 0)
                  Index = 0;
               else if (strncmp(fParameterName[i].Name + 7, "B", 1) == 0)
                  Index = 1;
               else if (strncmp(fParameterName[i].Name + 7, "C", 1) == 0)
                  Index = 2;
               else if (strncmp(fParameterName[i].Name + 7, "D", 1) == 0)
                  Index = 3;
               else {
                  cout << "Unknown Index " << Index << endl;
                  exit(EXIT_FAILURE);
               }
               sscanf(fParameterName[i].Name + 9, "%1d", &tmp);
               Index1 = (UShort_t) tmp;
            } else {
               sscanf(fParameterName[i].Name + 5, "%2d", &tmp);
               fParameterMap[fParameterNumber[i]] = tmp;
               if (strncmp(fParameterName[i].Name + 8, "A", 1) == 0)
                  Index = 0;
               else if (strncmp(fParameterName[i].Name + 8, "B", 1) == 0)
                  Index = 1;
               else if (strncmp(fParameterName[i].Name + 8, "C", 1) == 0)
                  Index = 2;
               else if (strncmp(fParameterName[i].Name + 8, "D", 1) == 0)
                  Index = 3;
               else {
                  cout << "Unknown Index " << Index << endl;
                  exit(EXIT_FAILURE);
               }
               sscanf(fParameterName[i].Name + 10, "%1d", &tmp);
               Index1 = (UShort_t) tmp;
            }
            fParameterMap[fParameterNumber[i]] *= 16;
            fParameterMap[fParameterNumber[i]] += Index * 4;
            fParameterMap[fParameterNumber[i]] += Index1;
            break;
         case 22:
            if (strncmp((fParameterName[i]).Name + 6, "_", 1) == 0) {
               sscanf(fParameterName[i].Name + 5, "%1d", &tmp);
               fParameterMap[fParameterNumber[i]] = tmp;
               if (strncmp((fParameterName[i]).Name + 7, "A", 1) == 0)
                  Index = 0;
               else if (strncmp(fParameterName[i].Name + 7, "B", 1) == 0)
                  Index = 1;
               else if (strncmp(fParameterName[i].Name + 7, "C", 1) == 0)
                  Index = 2;
               else if (strncmp(fParameterName[i].Name + 7, "D", 1) == 0)
                  Index = 3;
               else {
                  cout << "Unknown Index " << Index << endl;
                  exit(EXIT_FAILURE);
               }
               sscanf(fParameterName[i].Name + 9, "%1d", &tmp);
               Index1 = (UShort_t) tmp;
            } else {
               sscanf(fParameterName[i].Name + 5, "%2d", &tmp);
               fParameterMap[fParameterNumber[i]] = tmp;
               if (strncmp(fParameterName[i].Name + 8, "A", 1) == 0)
                  Index = 0;
               else if (strncmp(fParameterName[i].Name + 8, "B", 1) == 0)
                  Index = 1;
               else if (strncmp(fParameterName[i].Name + 8, "C", 1) == 0)
                  Index = 2;
               else if (strncmp(fParameterName[i].Name + 8, "D", 1) == 0)
                  Index = 3;
               else {
                  cout << "Unknown Index " << Index << endl;
                  exit(EXIT_FAILURE);
               }
               sscanf(fParameterName[i].Name + 10, "%1d", &tmp);
               Index1 = (UShort_t) tmp;
            }
            fParameterMap[fParameterNumber[i]] *= 16;
            fParameterMap[fParameterNumber[i]] += Index * 4;
            fParameterMap[fParameterNumber[i]] += Index1;
            break;
#endif
#ifdef SI
         case 19:
            sscanf(fParameterName[i].Name + 4, "%2d", &tmp);
            fParameterMap[fParameterNumber[i]] = tmp - 1;
            //       cout << fParameterName[i].Name << ' ' << fParameterNumber[i] << endl;
            break;
#endif

#ifdef WCSI
         case 24: // TIARA wall of csi with parameter names "ADC-CSI_1_VOIE_12"
            sscanf(fParameterName[i].Name + 7, "%1d", &tmp);
            sscanf(fParameterName[i].Name + 14, "%2d", &tmp1);

            fParameterMap[fParameterNumber[i]] = tmp1 + (tmp - 1) * 32;
            //       cout << fParameterName[i].Name << ' ' << fParameterNumber[i] << endl;
            break;
         case 34: // TIARA wall of csi with parameter names "CSI01"
            sscanf(fParameterName[i].Name + 3, "%2d", &tmp);

            fParameterMap[fParameterNumber[i]] = tmp - 1;
            //       cout << fParameterName[i].Name << ' ' << fParameterNumber[i] << endl;
            break;
#endif

#ifdef INDRA
         case 25:
         case 26:
         case 27:
            sscanf(fParameterName[i].Name + 4, "%2d%2d", &tmp, &tmp1);
            fParameterMap[fParameterNumber[i]] = tmp1 - 1 + (tmp - 1) * 24;
            //       cout << fParameterName[i].Name << ' ' << fParameterNumber[i] << endl;
            break;

         case 28:
         case 29:
         case 30:
            sscanf(fParameterName[i].Name + 3, "%2d%2d", &tmp, &tmp1);
            fParameterMap[fParameterNumber[i]] = tmp1 - 1 + (tmp - 1) * 24;
            //       cout << fParameterName[i].Name << ' ' << fParameterNumber[i] << endl;
            break;
         case 31:
         case 32:
         case 33:
            sscanf(fParameterName[i].Name + 3, "%2d%2d", &tmp, &tmp1);
            fParameterMap[fParameterNumber[i]] = tmp1 - 1 + (tmp - 1) * 24;
            //       cout << fParameterName[i].Name << ' ' << fParameterNumber[i] << endl;
            break;

#endif



#ifdef CSI
         case 16:
         case 17:
         case 18:
            if (strncmp((fParameterName[i]).Name + 4, "_", 1) == 0) {
               sscanf(fParameterName[i].Name + 3, "%1d", &tmp);
               fParameterMap[fParameterNumber[i]] = tmp;
            } else {
               sscanf(fParameterName[i].Name + 3, "%2d", &tmp);
               fParameterMap[fParameterNumber[i]] = tmp;
            }
            break;
#endif

         default:
            cout << "shouldn't happen " << fParameterName[i].Name << endl;
            exit(EXIT_FAILURE);
      }
   }


   if (fParameterNumber) {
      delete[] fParameterNumber;
#if __cplusplus < 201103L
      fParameterNumber = NULL;
#else
      fParameterNumber = nullptr;
#endif
   }

   if (fParameterType) {
      delete[] fParameterType;
#if __cplusplus < 201103L
      fParameterType = NULL;
#else
      fParameterType = nullptr;
#endif
   }
   if (fParameterBits) {
      delete[] fParameterBits;
#if __cplusplus < 201103L
      fParameterBits = NULL;
#else
      fParameterBits = nullptr;
#endif
   }

   if (fParameterName) {
      delete[] fParameterName;
#if __cplusplus < 201103L
      fParameterName = NULL;
#else
      fParameterName = nullptr;
#endif
   }

}
void Parameters::GetData(Short_t* data)
{
   if (data[0] < fMaxParameterNumber) {


      switch (fParameterMapT[data[0]]) {
         case 0: //Removed Parameter
            //      cout << "Removed Parameter Nr: " << data[0] << endl;
            break;
         case -1: // free Parameter
            fParameterData[fParameterMap[data[0]]] = data[1];
            fParametersTouched[fParametersM] = fParameterMap[data[0]];
            fParametersM++;
            break;
#ifdef ECHI
         case 1:
         case 2:
         case 3:
            if ((data[1] > 0)) { //was previously >200
               EchiNr[EchiM] = fParameterMap[data[0]];
               Echi[EchiM] = data[1];
               EchiM++;
            }
            break;
#endif
#ifdef STRIPS
         case 4:
            STRNr[0 * 64 + STRM1] = fParameterMap[data[0]];
            STR[0 * 64 + STRM1] = data[1];
            STRM1++;
            break;
         case 5:
            STRNr[1 * 64 + STRM2] = fParameterMap[data[0]];
            STR[1 * 64 + STRM2] = data[1];
            STRM2++;
            break;
         case 6:
            STRNr[2 * 64 + STRM3] = fParameterMap[data[0]];
            STR[2 * 64 + STRM3] = data[1];
            STRM3++;
            break;
         case 7:
            STRNr[3 * 64 + STRM4] = fParameterMap[data[0]];
            STR[3 * 64 + STRM4] = data[1];
            STRM4++;
            break;
#endif
#ifdef SED
         case 12:
            SEDNr[0 * 128 + SEDMX1] = fParameterMap[data[0]];
            SEDQ[0 * 128 + SEDMX1] = data[1];
            SEDMX1++;
            break;
         case 13:
            SEDNr[2 * 128 + SEDMY1] = fParameterMap[data[0]];
            SEDQ[2 * 128 + SEDMY1] = data[1];
            SEDMY1++;
            break;
         case 14:
            SEDNr[1 * 128 + SEDMX2] = fParameterMap[data[0]];
            SEDQ[1 * 128 + SEDMX2] = data[1];
            SEDMX2++;
            break;
         case 15:
            SEDNr[3 * 128 + SEDMY2] = fParameterMap[data[0]];
            SEDQ[3 * 128 + SEDMY2] = data[1];
            SEDMY2++;
            break;
#endif
#ifdef ECC
         case 8:
            if (ECCEM < 16 * 4 - 1) {
               ECCENr[ECCEM] = fParameterMap[data[0]];
               ECCE[ECCEM] = data[1];
               ECCEM++;
            } else
               cout << "ECCEM: " << ECCEM << endl;
            break;
         case 9:
            if (ECCTM < 16 * 4 - 1) {
               ECCTNr[ECCTM] = fParameterMap[data[0]];
               ECCT[ECCTM] = data[1];
               ECCTM++;
            } else
               cout << "ECCTM: " << ECCTM << endl;
            break;
#endif
#ifdef ECC20MEV
         case 23:
            if (ECC20EM < 16 * 4 - 1) {
               ECC20ENr[ECC20EM] = fParameterMap[data[0]];
               ECC20E[ECC20EM] = data[1];
               ECC20EM++;
            } else
               cout << "ECC20EM: " << ECC20EM << endl;
            break;
#endif
#ifdef GOCCE
         case 21:
            if (GOCCEEM < 16 * 16 - 1) {
               GOCCEENr[GOCCEEM] = fParameterMap[data[0]];
               GOCCEE[GOCCEEM] = data[1];
               GOCCEEM++;
            } else
               cout << "GOCCEEM: " << GOCCEEM << endl;
            break;
         case 22:
            if (GOCCETM < 16 * 16 - 1) {
               GOCCETNr[GOCCETM] = fParameterMap[data[0]];
               GOCCET[GOCCETM] = data[1];
               GOCCETM++;
            } else
               cout << "GOCETM: " << GOCCETM << endl;
            break;
#endif
#ifdef ESS
         case 10:
            if (ESSQM < 16 * 4 - 1) {
               ESSQNr[ESSQM] = fParameterMap[data[0]];
               ESSQ[ESSQM] = data[1];
               ESSQM++;
            } else
               cout << "ESSQM: " << ESSQM << endl;
            break;
         case 11:
            if (ESSTM < 16 - 1) {
               ESSTNr[ESSTM] = fParameterMap[data[0]];
               ESST[ESSTM] = data[1];
               ESSTM++;
            } else
               cout << "ESSTM: " << ESSTM << endl;
            break;
#endif




#ifdef CSI
         case 16:
            CSIENr[CSIEM] = fParameterMap[data[0]];
            CSIE[CSIEM] = data[1];
            CSIEM++;
            break;
         case 17:
            CSITNr[CSITM] = fParameterMap[data[0]];
            CSIT[CSITM] = data[1];
            CSITM++;
            break;
         case 18:
            CSIPNr[CSIPM] = fParameterMap[data[0]];
            CSIP[CSIPM] = data[1];
            CSIPM++;
            break;
#endif
#ifdef SI
         case 19:
            if (data[1] > 0) { //was previously > 250
               SIENr[SIEM] = fParameterMap[data[0]];
               SIE[SIEM] = data[1];
               SIEM++;
            }
            break;
#endif


#ifdef WCSI
         case 24:
            if (data[1] > 0) { //was >0
               WCSIENr[WCSIEM] = fParameterMap[data[0]];
               WCSIE[WCSIEM] = data[1];
               WCSIEM++;
            }
            break;
         case 34:
            if (data[1] > 0) { //was >0
               WCSIENr[WCSIEM] = fParameterMap[data[0]];
               WCSIE[WCSIEM] = data[1];
               WCSIEM++;
            }
            break;
#endif

#ifdef INDRA
         case 25:
            if (data[1] > 0) {
               ICSILNr[ICSILM] = fParameterMap[data[0]];
               ICSIL[ICSILM] = data[1];
               ICSILM++;
            }
            break;
         case 26:
            if (data[1] > 0) {
               ICSIRNr[ICSIRM] = fParameterMap[data[0]];
               ICSIR[ICSIRM] = data[1];
               ICSIRM++;
            }
            break;
         case 27:
            if (data[1] > 0) {
               ICSITNr[ICSITM] = fParameterMap[data[0]];
               ICSIT[ICSITM] = data[1];
               ICSITM++;
            }
            break;
         case 28:
            if (data[1] > 0) {
               ISILNr[ISILM] = fParameterMap[data[0]];
               ISIL[ISILM] = data[1];
               ISILM++;
            }
            break;
         case 29:
            if (data[1] > 0) {
               ISIHNr[ISIHM] = fParameterMap[data[0]];
               ISIH[ISIHM] = data[1];
               ISIHM++;
            }
            break;
         case 30:
            if (data[1] > 0) {
               ISITNr[ISITM] = fParameterMap[data[0]];
               ISIT[ISITM] = data[1];
               ISITM++;
            }
            break;
         case 31:
            if (data[1] > 0) {
               ICILNr[ICILM] = fParameterMap[data[0]];
               ICIL[ICILM] = data[1];
               ICILM++;
            }
            break;
         case 32:
            if (data[1] > 0) {
               ICIHNr[ICIHM] = fParameterMap[data[0]];
               ICIH[ICIHM] = data[1];
               ICIHM++;
            }
            break;
         case 33:
            if (data[1] > 0) {
               ICITNr[ICITM] = fParameterMap[data[0]];
               ICIT[ICITM] = data[1];
               ICITM++;
            }
            break;

#endif



         default :
            cout << "Should not happen Parameter Number : " << data[0] << " Map:" << fParameterMapT[data[0]] << endl;
            break;
      }
   } else
      cout << "Parameter Number " << data[0] << " out of range (" << fMaxParameterNumber << ")" << endl;

}
void Parameters::Clear(void)
{

   //For each event

   if (fParametersM)
      for (UShort_t i = 0; i < fParametersM; i++)
         fParameterData[fParametersTouched[i]] = 0;
   fParametersM = 0;

#ifdef ECHI
   if (EchiB) {
      EchiM = 0;
   }
#endif

#ifdef STRIPS
   if (STRB) {
      STRM1 = STRM2 = STRM3 = STRM4 = 0;
   }
#endif
#ifdef SED
   if (SEDB) {
      SEDMX1 = SEDMY1 = SEDMX2 = SEDMY2 = 0;
   }
#endif
#ifdef ECC
   if (ECCEB) {
      ECCEM = 0;
   }
   if (ECCTB) {
      ECCTM = 0;
   }
#endif
#ifdef ECC20MEV
   if (ECC20EB) {
      ECC20EM = 0;
   }
#endif
#ifdef GOCCE
   if (GOCCEEB) {
      GOCCEEM = 0;
   }
   if (GOCCETB) {
      GOCCETM = 0;
   }
#endif
#ifdef ESS
   if (ESSQB) {
      ESSQM = 0;
   }
   if (ESSTB) {
      ESSTM = 0;
   }
#endif

#ifdef CSI
   if (CSIEB) {
      CSIEM = 0;
   }
   if (CSITB) {
      CSITM = 0;
   }
   if (CSIPB) {
      CSIPM = 0;
   }

#endif
#ifdef SI
   if (SIEB) {
      SIEM = 0;
   }
#endif

#ifdef WCSI
   if (WCSIEB) {
      WCSIEM = 0;
   }
#endif
#ifdef INDRA
   if (ICSILB) {
      ICSILM = 0;
   }
   if (ICSIRB) {
      ICSIRM = 0;
   }
   if (ICSITB) {
      ICSITM = 0;
   }
   if (ISILB) {
      ISILM = 0;
   }
   if (ISIHB) {
      ISIHM = 0;
   }
   if (ISITB) {
      ISITM = 0;
   }
   if (ICILB) {
      ICILM = 0;
   }
   if (ICIHB) {
      ICIHM = 0;
   }
   if (ICITB) {
      ICITM = 0;
   }
#endif

}

//__________________________________________________________________________//

char* Parameters::CopyParam(char* Dest, char* Source) const
{
   // Small utility routine to copy a char string.
   char c;
   std::size_t len(0);

   do {
      len++;
      c = *Source++;
      if ((c == ',') || (c == 0x0d))
         c = '\0';
      if (len <= ParameterName::maxlen) // 20
         *Dest++ = c;
      else
         *Dest = '\0';
   } while (c != '\0');
   /* Verifier qu'on n'a pas plusieurs 0x0D de suite */
   while (*Source == 0x0d) {
      *Source = '\0';
      Source++;
   }
   return (Source);
}

//__________________________________________________________________________//

void Parameters::Fill(const char* buffParam)
{
   // Data from the parameter buffet (buffParam) have been read from disk and is
   // parsed by this routine who create the list of labels and corresponding
   // index.

   char* CurrPointer;
#if __cplusplus < 201103L
   char* tmp(NULL);
#else
   char* tmp(nullptr);
#endif

   // Ideally should be updated to use smart pointers but this whole class could
   // do with updating really.
   tmp = new char[ParameterName::maxlen];

   CurrPointer = (char*)buffParam;

   while (strncmp(CurrPointer, "    ", 4) != 0 &&  strncmp(CurrPointer, "!!!!", 4) != 0) {

      fNumberofParameters++;
      CurrPointer = CopyParam(fParameterName[fNumberofParameters - 1].Name, CurrPointer);
      CurrPointer = CopyParam(tmp, CurrPointer);
      sscanf(tmp, "%hu", &fParameterNumber[fNumberofParameters - 1]);
      CurrPointer = CopyParam(tmp, CurrPointer);
      sscanf(tmp, "%hu", &fParameterBits[fNumberofParameters - 1]);
      if (!((fParameterNumber[fNumberofParameters - 1] == 0) || (fParameterBits[fNumberofParameters - 1] == 0))) {
         Check();
      } else
         fNumberofParameters--;
   }

   if (tmp) {
      delete[] tmp;
#if __cplusplus < 201103L
      tmp = NULL;
#else
      tmp = nullptr;
#endif
   }
}

