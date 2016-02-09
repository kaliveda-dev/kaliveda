//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Wed Apr  7 12:22:22 2010 by ROOT version 5.26/00
// from TTree ReconstructedEvents/Run 511 :  : recon events created from raw data
// found on file: root://ccxroot.in2p3.fr:1999//hpss/in2p3.fr/group/indra/e503/recon/run511.recon.root.2007-09-27_08:27:28
//////////////////////////////////////////////////////////

#ifndef KVIVSelector_h
#define KVIVSelector_h

#include <KVSelector.h>
#include "KVIVReconEvent.h"

class KVIVSelector : public KVSelector {


#define VERY_BIG_TABLEAU 500

public :

//   Int_t           EchiM;
//   UShort_t        Echi[VERY_BIG_TABLEAU];   //[EchiM]
//   UShort_t        EchiNr[VERY_BIG_TABLEAU];   //[EchiM]
//   Int_t           SEDMX1;
//   Int_t           SEDMY1;
//   Int_t           SEDMX2;
//   Int_t           SEDMY2;
//   UShort_t        SEDX1[VERY_BIG_TABLEAU];   //[SEDMX1]
//   UShort_t        SEDX2[VERY_BIG_TABLEAU];   //[SEDMX2]
//   UShort_t        SEDY1[VERY_BIG_TABLEAU];   //[SEDMY1]
//   UShort_t        SEDY2[VERY_BIG_TABLEAU];   //[SEDMY2]
//   UShort_t        SEDNrX1[VERY_BIG_TABLEAU];   //[SEDMX1]
//   UShort_t        SEDNrX2[VERY_BIG_TABLEAU];   //[SEDMX2]
//   UShort_t        SEDNrY1[VERY_BIG_TABLEAU];   //[SEDMY1]
//   UShort_t        SEDNrY2[VERY_BIG_TABLEAU];   //[SEDMY2]
//   Int_t           STRM1;
//   Int_t           STRM2;
//   Int_t           STRM3;
//   Int_t           STRM4;
//   UShort_t        STR1[VERY_BIG_TABLEAU];   //[STRM1]
//   UShort_t        STR2[VERY_BIG_TABLEAU];   //[STRM2]
//   UShort_t        STR3[VERY_BIG_TABLEAU];   //[STRM3]
//   UShort_t        STR4[VERY_BIG_TABLEAU];   //[STRM4]
//   UShort_t        STRNr1[VERY_BIG_TABLEAU];   //[STRM1]
//   UShort_t        STRNr2[VERY_BIG_TABLEAU];   //[STRM2]
//   UShort_t        STRNr3[VERY_BIG_TABLEAU];   //[STRM3]
//   UShort_t        STRNr4[VERY_BIG_TABLEAU];   //[STRM4]
//   Int_t           SIEM;
//   UShort_t        SIE[VERY_BIG_TABLEAU];   //[SIEM]
//   UShort_t        SIENr[VERY_BIG_TABLEAU];   //[SIEM]
//   Int_t           WCSIEM;
//   UShort_t        WCSIE[VERY_BIG_TABLEAU];   //[WCSIEM]
//   UShort_t        WCSIENr[VERY_BIG_TABLEAU];   //[WCSIEM]
//   Int_t           ICSILM;
//   UShort_t        ICSIL[VERY_BIG_TABLEAU];   //[ICSILM]
//   UShort_t        ICSILNr[VERY_BIG_TABLEAU];   //[ICSILM]
//   Int_t           ICSIRM;
//   UShort_t        ICSIR[VERY_BIG_TABLEAU];   //[ICSIRM]
//   UShort_t        ICSIRNr[VERY_BIG_TABLEAU];   //[ICSIRM]
//   Int_t           ICSITM;
//   UShort_t        ICSIT[VERY_BIG_TABLEAU];   //[ICSITM]
//   UShort_t        ICSITNr[VERY_BIG_TABLEAU];   //[ICSITM]
//   Int_t           ISILM;
//   UShort_t        ISIL[VERY_BIG_TABLEAU];   //[ISILM]
//   UShort_t        ISILNr[VERY_BIG_TABLEAU];   //[ISILM]
//   Int_t           ISIHM;
//   UShort_t        ISIH[VERY_BIG_TABLEAU];   //[ISIHM]
//   UShort_t        ISIHNr[VERY_BIG_TABLEAU];   //[ISIHM]
//   Int_t           ISITM;
//   UShort_t        ISIT[VERY_BIG_TABLEAU];   //[ISITM]
//   UShort_t        ISITNr[VERY_BIG_TABLEAU];   //[ISITM]
//   Int_t           ICILM;
//   UShort_t        ICIL[VERY_BIG_TABLEAU];   //[ICILM]
//   UShort_t        ICILNr[VERY_BIG_TABLEAU];   //[ICILM]
//   Int_t           ICIHM;
//   UShort_t        ICIH[VERY_BIG_TABLEAU];   //[ICIHM]
//   UShort_t        ICIHNr[VERY_BIG_TABLEAU];   //[ICIHM]
//   Int_t           ICITM;
//   UShort_t        ICIT[VERY_BIG_TABLEAU];   //[ICITM]
//   UShort_t        ICITNr[VERY_BIG_TABLEAU];   //[ICITM]
//   UShort_t        INCONF;
//   UShort_t        GATCONF;
//   UShort_t        INV_SIE_05;
//   UShort_t        INV_SIE_06;
//   UShort_t        INV_SIE_08;
//   UShort_t        ADC1_4;
//   UShort_t        ADC1_5;
//   UShort_t        ADC1_6;
//   UShort_t        SI_IND_61;
//   UShort_t        CSI_IND_1224;
//   UShort_t        EFIL_1;
//   UShort_t        EFIL_2;
//   UShort_t        TFIL_1;
//   UShort_t        TFIL_2;
//   UShort_t        TSED1_HF;
//   UShort_t        TSED2_HF;
//   UShort_t        TSI_HF;
//   UShort_t        TSI_INDRA;
//   UShort_t        TINDRA_SI;
//   UShort_t        TSED1_SED2;
//   UShort_t        TSI_SED1;
//   UShort_t        TSED1_INDRA;
//   UShort_t        TCSI_HF;
//   UShort_t        TINDRA_CSI;
//   UShort_t        TINDRA_HF;
//   UShort_t        TSI_MCP;
//   UShort_t        TMCP_HF;
//   UShort_t        TINDRA_MCP;
//   UShort_t        TSED1_MCP;
//
//   // List of branches
//   TBranch        *b_EchiM;   //!
//   TBranch        *b_Echi;   //!
//   TBranch        *b_EchiNr;   //!
//   TBranch        *b_SEDMX1;   //!
//   TBranch        *b_SEDMY1;   //!
//   TBranch        *b_SEDMX2;   //!
//   TBranch        *b_SEDMY2;   //!
//   TBranch        *b_SEDX1;   //!
//   TBranch        *b_SEDX2;   //!
//   TBranch        *b_SEDY1;   //!
//   TBranch        *b_SEDY2;   //!
//   TBranch        *b_SEDNrX1;   //!
//   TBranch        *b_SEDNrX2;   //!
//   TBranch        *b_SEDNrY1;   //!
//   TBranch        *b_SEDNrY2;   //!
//   TBranch        *b_STRM1;   //!
//   TBranch        *b_STRM2;   //!
//   TBranch        *b_STRM3;   //!
//   TBranch        *b_STRM4;   //!
//   TBranch        *b_STR1;   //!
//   TBranch        *b_STR2;   //!
//   TBranch        *b_STR3;   //!
//   TBranch        *b_STR4;   //!
//   TBranch        *b_STRNr1;   //!
//   TBranch        *b_STRNr2;   //!
//   TBranch        *b_STRNr3;   //!
//   TBranch        *b_STRNr4;   //!
//   TBranch        *b_SIEM;   //!
//   TBranch        *b_SIE;   //!
//   TBranch        *b_SIENr;   //!
//   TBranch        *b_WCSIEM;   //!
//   TBranch        *b_WCSIE;   //!
//   TBranch        *b_WCSIENr;   //!
//   TBranch        *b_ICSILM;   //!
//   TBranch        *b_ICSIL;   //!
//   TBranch        *b_ICSILNr;   //!
//   TBranch        *b_ICSIRM;   //!
//   TBranch        *b_ICSIR;   //!
//   TBranch        *b_ICSIRNr;   //!
//   TBranch        *b_ICSITM;   //!
//   TBranch        *b_ICSIT;   //!
//   TBranch        *b_ICSITNr;   //!
//   TBranch        *b_ISILM;   //!
//   TBranch        *b_ISIL;   //!
//   TBranch        *b_ISILNr;   //!
//   TBranch        *b_ISIHM;   //!
//   TBranch        *b_ISIH;   //!
//   TBranch        *b_ISIHNr;   //!
//   TBranch        *b_ISITM;   //!
//   TBranch        *b_ISIT;   //!
//   TBranch        *b_ISITNr;   //!
//   TBranch        *b_ICILM;   //!
//   TBranch        *b_ICIL;   //!
//   TBranch        *b_ICILNr;   //!
//   TBranch        *b_ICIHM;   //!
//   TBranch        *b_ICIH;   //!
//   TBranch        *b_ICIHNr;   //!
//   TBranch        *b_ICITM;   //!
//   TBranch        *b_ICIT;   //!
//   TBranch        *b_ICITNr;   //!
//   TBranch        *b_INCONF;   //!
//   TBranch        *b_GATCONF;   //!
//   TBranch        *b_INV_SIE_05;   //!
//   TBranch        *b_INV_SIE_06;   //!
//   TBranch        *b_INV_SIE_08;   //!
//   TBranch        *b_ADC1_4;   //!
//   TBranch        *b_ADC1_5;   //!
//   TBranch        *b_ADC1_6;   //!
//   TBranch        *b_SI_IND_61;   //!
//   TBranch        *b_CSI_IND_1224;   //!
//   TBranch        *b_EFIL_1;   //!
//   TBranch        *b_EFIL_2;   //!
//   TBranch        *b_TFIL_1;   //!
//   TBranch        *b_TFIL_2;   //!
//   TBranch        *b_TSI_HF;   //!
//   TBranch        *b_TINDRA_SI;   //!
//   TBranch        *b_TCSI_HF;   //!
//   TBranch        *b_TINDRA_CSI;   //!
//   TBranch        *b_TINDRA_HF;   //!
//   TBranch        *b_TSI_MCP;   //!
//   TBranch        *b_TMCP_HF;   //!
//   TBranch        *b_TINDRA_MCP;   //!
//   TBranch        *b_TSED1_HF;   //!
//   TBranch        *b_TSED2_HF;   //!
//   TBranch        *b_TSI_INDRA;   //!
//   TBranch        *b_TSED1_SED2;   //!
//   TBranch        *b_TSI_SED1;   //!
//   TBranch        *b_TSED1_INDRA;   //!
//   TBranch        *b_TSED1_MCP;   //!

   KVIVSelector(TTree* /*tree*/ = 0);
   virtual ~KVIVSelector();
   virtual void    Init(TTree* tree);

   KVIVReconEvent* GetEvent()
   {
      return (KVIVReconEvent*)data;
   };

   static void Make(const Char_t* kvsname = "MyOwnKVIVSelector");
   virtual Bool_t Notify();

   ClassDef(KVIVSelector, 0);
};

#endif

#ifdef KVIVSelector_cxx
void KVIVSelector::Init(TTree* tree)
{
   // The Init() function is called when the selector needs to initialize
   // a new tree or chain. Typically here the branch addresses and branch
   // pointers of the tree will be set.
   // It is normally not necessary to make changes to the generated
   // code, but the routine can be extended by the user if needed.
   // Init() will be called many times when running on PROOF
   // (once per file to be processed).

   if (!tree) return;
   TBranch* br = (TBranch*)tree->GetListOfBranches()->First();

   if (TClass::GetClass(br->GetClassName())->InheritsFrom("KVINDRAReconEvent"))
      SetINDRAReconEventBranchName(br->GetName());

   KVSelector::Init(tree);

//   fChain->SetBranchAddress("EchiM", &EchiM, &b_EchiM);
//   fChain->SetBranchAddress("Echi", Echi, &b_Echi);
//   fChain->SetBranchAddress("EchiNr", EchiNr, &b_EchiNr);
//   fChain->SetBranchAddress("SEDMX1", &SEDMX1, &b_SEDMX1);
//   fChain->SetBranchAddress("SEDMY1", &SEDMY1, &b_SEDMY1);
//   fChain->SetBranchAddress("SEDMX2", &SEDMX2, &b_SEDMX2);
//   fChain->SetBranchAddress("SEDMY2", &SEDMY2, &b_SEDMY2);
//   fChain->SetBranchAddress("SEDX1", SEDX1, &b_SEDX1);
//   fChain->SetBranchAddress("SEDX2", SEDX2, &b_SEDX2);
//   fChain->SetBranchAddress("SEDY1", SEDY1, &b_SEDY1);
//   fChain->SetBranchAddress("SEDY2", SEDY2, &b_SEDY2);
//   fChain->SetBranchAddress("SEDNrX1", SEDNrX1, &b_SEDNrX1);
//   fChain->SetBranchAddress("SEDNrX2", SEDNrX2, &b_SEDNrX2);
//   fChain->SetBranchAddress("SEDNrY1", SEDNrY1, &b_SEDNrY1);
//   fChain->SetBranchAddress("SEDNrY2", SEDNrY2, &b_SEDNrY2);
//   fChain->SetBranchAddress("STRM1", &STRM1, &b_STRM1);
//   fChain->SetBranchAddress("STRM2", &STRM2, &b_STRM2);
//   fChain->SetBranchAddress("STRM3", &STRM3, &b_STRM3);
//   fChain->SetBranchAddress("STRM4", &STRM4, &b_STRM4);
//   fChain->SetBranchAddress("STR1", STR1, &b_STR1);
//   fChain->SetBranchAddress("STR2", STR2, &b_STR2);
//   fChain->SetBranchAddress("STR3", STR3, &b_STR3);
//   fChain->SetBranchAddress("STR4", STR4, &b_STR4);
//   fChain->SetBranchAddress("STRNr1", STRNr1, &b_STRNr1);
//   fChain->SetBranchAddress("STRNr2", STRNr2, &b_STRNr2);
//   fChain->SetBranchAddress("STRNr3", STRNr3, &b_STRNr3);
//   fChain->SetBranchAddress("STRNr4", STRNr4, &b_STRNr4);
//   fChain->SetBranchAddress("SIEM", &SIEM, &b_SIEM);
//   fChain->SetBranchAddress("SIE", SIE, &b_SIE);
//   fChain->SetBranchAddress("SIENr", SIENr, &b_SIENr);
//   fChain->SetBranchAddress("WCSIEM", &WCSIEM, &b_WCSIEM);
//   fChain->SetBranchAddress("WCSIE", WCSIE, &b_WCSIE);
//   fChain->SetBranchAddress("WCSIENr", WCSIENr, &b_WCSIENr);
//   fChain->SetBranchAddress("ICSILM", &ICSILM, &b_ICSILM);
//   fChain->SetBranchAddress("ICSIL", ICSIL, &b_ICSIL);
//   fChain->SetBranchAddress("ICSILNr", ICSILNr, &b_ICSILNr);
//   fChain->SetBranchAddress("ICSIRM", &ICSIRM, &b_ICSIRM);
//   fChain->SetBranchAddress("ICSIR", ICSIR, &b_ICSIR);
//   fChain->SetBranchAddress("ICSIRNr", ICSIRNr, &b_ICSIRNr);
//   fChain->SetBranchAddress("ICSITM", &ICSITM, &b_ICSITM);
//   fChain->SetBranchAddress("ICSIT", ICSIT, &b_ICSIT);
//   fChain->SetBranchAddress("ICSITNr", ICSITNr, &b_ICSITNr);
//   fChain->SetBranchAddress("ISILM", &ISILM, &b_ISILM);
//   fChain->SetBranchAddress("ISIL", ISIL, &b_ISIL);
//   fChain->SetBranchAddress("ISILNr", ISILNr, &b_ISILNr);
//   fChain->SetBranchAddress("ISIHM", &ISIHM, &b_ISIHM);
//   fChain->SetBranchAddress("ISIH", ISIH, &b_ISIH);
//   fChain->SetBranchAddress("ISIHNr", ISIHNr, &b_ISIHNr);
//   fChain->SetBranchAddress("ISITM", &ISITM, &b_ISITM);
//   fChain->SetBranchAddress("ISIT", ISIT, &b_ISIT);
//   fChain->SetBranchAddress("ISITNr", ISITNr, &b_ISITNr);
//   fChain->SetBranchAddress("ICILM", &ICILM, &b_ICILM);
//   fChain->SetBranchAddress("ICIL", ICIL, &b_ICIL);
//   fChain->SetBranchAddress("ICILNr", ICILNr, &b_ICILNr);
//   fChain->SetBranchAddress("ICIHM", &ICIHM, &b_ICIHM);
//   fChain->SetBranchAddress("ICIH", ICIH, &b_ICIH);
//   fChain->SetBranchAddress("ICIHNr", ICIHNr, &b_ICIHNr);
//   fChain->SetBranchAddress("ICITM", &ICITM, &b_ICITM);
//   fChain->SetBranchAddress("ICIT", &ICIT, &b_ICIT);
//   fChain->SetBranchAddress("ICITNr", &ICITNr, &b_ICITNr);
//   fChain->SetBranchAddress("INCONF", &INCONF, &b_INCONF);
//   fChain->SetBranchAddress("GATCONF", &GATCONF, &b_GATCONF);
//   fChain->SetBranchAddress("INV_SIE_05", &INV_SIE_05, &b_INV_SIE_05);
//   fChain->SetBranchAddress("INV_SIE_06", &INV_SIE_06, &b_INV_SIE_06);
//   fChain->SetBranchAddress("INV_SIE_08", &INV_SIE_08, &b_INV_SIE_08);
//   fChain->SetBranchAddress("ADC1_4", &ADC1_4, &b_ADC1_4);
//   fChain->SetBranchAddress("ADC1_5", &ADC1_5, &b_ADC1_5);
//   fChain->SetBranchAddress("ADC1_6", &ADC1_6, &b_ADC1_6);
//   fChain->SetBranchAddress("SI_IND_61", &SI_IND_61, &b_SI_IND_61);
//   fChain->SetBranchAddress("CSI_IND_1224", &CSI_IND_1224, &b_CSI_IND_1224);
//   fChain->SetBranchAddress("EFIL_1", &EFIL_1, &b_EFIL_1);
//   fChain->SetBranchAddress("EFIL_2", &EFIL_2, &b_EFIL_2);
//   fChain->SetBranchAddress("TSED1_HF", &TSED1_HF, &b_TSED1_HF);
//   fChain->SetBranchAddress("TSED2_HF", &TSED2_HF, &b_TSED2_HF);
//   fChain->SetBranchAddress("TSI_HF", &TSI_HF, &b_TSI_HF);
//   fChain->SetBranchAddress("TSI_INDRA", &TSI_INDRA, &b_TSI_INDRA);
//   fChain->SetBranchAddress("TSED1_SED2", &TSED1_SED2, &b_TSED1_SED2);
//   fChain->SetBranchAddress("TSI_SED1", &TSI_SED1, &b_TSI_SED1);
//   fChain->SetBranchAddress("TSED1_INDRA", &TSED1_INDRA, &b_TSED1_INDRA);
//   fChain->SetBranchAddress("TSED1_MCP", &TSED1_MCP, &b_TSED1_MCP);
//   fChain->SetBranchAddress("TFIL_1", &TFIL_1, &b_TFIL_1);
//   fChain->SetBranchAddress("TFIL_2", &TFIL_2, &b_TFIL_2);
//   fChain->SetBranchAddress("TINDRA_SI", &TINDRA_SI, &b_TINDRA_SI);
//   fChain->SetBranchAddress("TCSI_HF", &TCSI_HF, &b_TCSI_HF);
//   fChain->SetBranchAddress("TINDRA_CSI", &TINDRA_CSI, &b_TINDRA_CSI);
//   fChain->SetBranchAddress("TINDRA_HF", &TINDRA_HF, &b_TINDRA_HF);
//   fChain->SetBranchAddress("TSI_MCP", &TSI_MCP, &b_TSI_MCP);
//   fChain->SetBranchAddress("TMCP_HF", &TMCP_HF, &b_TMCP_HF);
//   fChain->SetBranchAddress("TINDRA_MCP", &TINDRA_MCP, &b_TINDRA_MCP);
}
#endif // #ifdef KVIVSelector_cxx
