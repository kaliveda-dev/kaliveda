//Created by KVClassFactory on Fri Apr 13 12:31:16 2012
//Author: John Frankland

#ifndef __KVTreeAnalyzer_H
#define __KVTreeAnalyzer_H
#ifndef ROOT_TGDockableFrame
#include "TGDockableFrame.h"
#endif
#ifndef ROOT_TGMenu
#include "TGMenu.h"
#endif
#ifndef ROOT_TGMdiDecorFrame
#include "TGMdiDecorFrame.h"
#endif
#ifndef ROOT_TG3DLine
#include "TG3DLine.h"
#endif
#ifndef ROOT_TGMdiFrame
#include "TGMdiFrame.h"
#endif
#ifndef ROOT_TGMdiMainFrame
#include "TGMdiMainFrame.h"
#endif
#ifndef ROOT_TGMdiMenu
#include "TGMdiMenu.h"
#endif
#ifndef ROOT_TGListBox
#include "TGListBox.h"
#endif
#ifndef ROOT_TGNumberEntry
#include "TGNumberEntry.h"
#endif
#ifndef ROOT_TGScrollBar
#include "TGScrollBar.h"
#endif
#ifndef ROOT_TGComboBox
#include "TGComboBox.h"
#endif
#ifndef ROOT_TRootBrowser
#include "TRootBrowser.h"
#endif
#ifndef ROOT_TGFrame
#include "TGFrame.h"
#endif
#ifndef ROOT_TGFileDialog
#include "TGFileDialog.h"
#endif
#ifndef ROOT_TGShutter
#include "TGShutter.h"
#endif
#ifndef ROOT_TGButtonGroup
#include "TGButtonGroup.h"
#endif
#ifndef ROOT_TGCommandPlugin
#include "TGCommandPlugin.h"
#endif
#ifndef ROOT_TGCanvas
#include "TGCanvas.h"
#endif
#ifndef ROOT_TGFSContainer
#include "TGFSContainer.h"
#endif
#ifndef ROOT_TGColorSelect
#include "TGColorSelect.h"
#endif
#ifndef ROOT_TGTextEdit
#include "TGTextEdit.h"
#endif
#ifndef ROOT_TGButton
#include "TGButton.h"
#endif
#ifndef ROOT_TGFSComboBox
#include "TGFSComboBox.h"
#endif
#ifndef ROOT_TGLabel
#include "TGLabel.h"
#endif
#ifndef ROOT_TGView
#include "TGView.h"
#endif
#ifndef ROOT_TGFileBrowser
#include "TGFileBrowser.h"
#endif
#ifndef ROOT_TGTab
#include "TGTab.h"
#endif
#ifndef ROOT_TGListView
#include "TGListView.h"
#endif
#ifndef ROOT_TGSplitter
#include "TGSplitter.h"
#endif
#ifndef ROOT_TGTextEditor
#include "TGTextEditor.h"
#endif
#ifndef ROOT_TRootCanvas
#include "TRootCanvas.h"
#endif
#ifndef ROOT_TGStatusBar
#include "TGStatusBar.h"
#endif
#ifndef ROOT_TGListTree
#include "TGListTree.h"
#endif
#ifndef ROOT_TGToolTip
#include "TGToolTip.h"
#endif
#ifndef ROOT_TGToolBar
#include "TGToolBar.h"
#endif
#ifndef ROOT_TGHtmlBrowser
#include "TGHtmlBrowser.h"
#endif

#include "TEnv.h"
#include "Riostream.h"
#include "KVListView.h"
#include "TChain.h"
#include "TCutG.h"
#include "TH1.h"
#include "TEntryList.h"
#include "KVUniqueNameList.h"
#include "TBrowser.h"

class KVHistogram;

/**
\class KVTreeAnalyzer
\brief GUI for simple intuitive analysis of data in TTree
\ingroup GUI,Analysis

<h5>Configure initial state of interface</h5>
In .kvrootrc file, user can configure the following options, corresponding to check boxes in the interface:<br>
<pre>
KVTreeAnalyzer.LogScale:         off
KVTreeAnalyzer.UserBinning:           off
KVTreeAnalyzer.UserWeight:       off
KVTreeAnalyzer.NewCanvas:      off
KVTreeAnalyzer.Normalize:      off
KVTreeAnalyzer.Stats:      off
</pre>
Change value to 'on' if required.
*/

class KVTreeAnalyzer : public TNamed {
private:
   const Char_t* get_leaf_type_name(const TNamed* l);
   static KVList* fgAnalyzerList;//! static list of all analyzers in memory

   void init();
   Bool_t fDeletedByGUIClose;//!

   TTree* fTree;//! for backwards compatibility
   TChain* fChain;// the analyzed TTree or TChain
   KVUniqueNameList fSelections;// list of TEntryList user selections

   TString fSaveAnalysisFileName;//!
   Bool_t  fAnalysisModifiedSinceLastSave;//!

   /* leaves */
   TGGroupFrame* fMain_leaflist;//! GUI for access to TTree leaves and aliases
   KVListView* G_leaflist;//! GUI list of TTree leaves and aliases
   KVList fLeafList;//! clones of leaves in TChain
   TGLabel* G_leaf_expr;//!
   TGPictureButton* G_leaf_draw;//!
   TString fLeafExpr;//!
   KVList* fSelectedLeaves;//!
   TNamed* fXLeaf;//!
   TNamed* fYLeaf;//!
   TNamed* fZLeaf;//!
   TGTextEntry* G_alias_text;//!
   TGCheckButton* G_histo_prof;//!
   Bool_t fProfileHisto;//!
   TGCheckButton* G_histo_bin;//!
   Bool_t fUserBinning;//!
   TGCheckButton* G_histo_weight;//!
   Bool_t fUserWeight;//!
   TString fWeight;//!

   Int_t fNx, fNy; //
   Double_t fXmin, fXmax, fYmin, fYmax; //
   Int_t fNxF;//
   Double_t fXminF, fXmaxF; //

   Int_t fNxD, fNyD;//
   Int_t fOrderedDalitz;//

   /* histos */
   TGMainFrame* fMain_histolist;//! GUI for handling histograms
   TGPopupMenu* fMenuFile;//!
   TGPopupMenu* fMenuSelections;//!
   TGPopupMenu* fSelCombMenu;//!
   TGPopupMenu* fOptionMenu;//!
   TGPopupMenu* fSelGenerate;//!
   enum {
      MH_OPEN_FILE,
      MH_OPEN_CHAIN,
      MH_ADD_FRIEND,
      MH_SAVE_FILE,
      MH_SAVE,
      MH_CLOSE,
      MH_QUIT,
      OPT_PROOF,
      MH_APPLY_ANALYSIS,
      SEL_COMB_AND,
      SEL_COMB_OR,
      SEL_UPDATE,
      SEL_DELETE,
      SEL_GEN_CONST_XSEC
   };
   void HistoFileMenu_Open();
   void OpenChain();
   void HistoFileMenu_Save();
   TGLayoutHints* fMenuBarItemLayout;//!
   TGMenuBar* fMenuBar;//!
   KVListView* G_histolist;//! GUI list of histograms
   TGPictureButton* G_histo_del;//!
   TGPictureButton* G_histo_add;//!
   TGCheckButton* G_histo_same;//!
   TGCheckButton* G_histo_app_sel;//!
   TGCheckButton* G_histo_log;//!
   TGCheckButton* G_histo_new_can;//!
   TGCheckButton* G_histo_norm;//!
   TGCheckButton* G_histo_norm_events;//!
   TGCheckButton* G_histo_stats;//!
   TGCheckButton* G_histo_autosave;//!
   TGComboBox* G_histo_draw_option;//!
   TString fDrawOption;//!
   Bool_t fDrawSame;//! =kTRUE: draw histograms in same plot
   Int_t fSameColorIndex;//!
   Bool_t fDrawLog;//! =kTRUE: draw histograms with log-Y (1-D) or log-Z (2-D) scale
   Bool_t fApplySelection;//! =kTRUE: apply current selection to existing histogram
   Bool_t fNewCanvas;//! =kTRUE: draw each histogram in a new canvas
   Bool_t fNormHisto;//! =kTRUE: generate normalised histograms (normalise to integral of histo)
   Bool_t fNormHistoEvents;//! =kTRUE: generate normalised histograms (normalise to number of events)
   Bool_t fStatsHisto;//! =kTRUE: display histo stats box
   Bool_t fAutoSaveHisto;//! =kTRUE: on draw, generate image file of current displayed histo
   TString fAutoSaveDir;//! directory for autosaving histos
   TString fAutoSaveType;//! filetype for autosaving histos
   TList* fSelectedHistos;//!
   TGTextButton* G_make_ip_scale;//!
   TGTextButton* G_fit1;//!
   TGTextButton* G_fit2;//!
   TGTextButton* G_fit3;//!
   TGTextButton* G_fitGG1;//!
   TGTextButton* G_fitGG2;//!
   TGTextButton* G_fitGG3;//!
   TGTextEntry* G_make_ip_selection;//!
   TGLabel* G_ip_histo;//!
//    KVGumbelDistribution *GDfirst;//!
//    KVGumbelDistribution *GDsecond;//!
//    KVGumbelDistribution *GDthird;//!
//    KVGausGumDistribution *GausGum1;//!
//    KVGausGumDistribution *GausGum2;//!
//    KVGausGumDistribution *GausGum3;//!

   /* selections */
   TGGroupFrame* fMain_selectionlist;//! GUI for handling selections
   KVListView* G_selectionlist;//! GUI list of TEntryList selections
   TGStatusBar* G_selection_status;//! status bar in selections GUI
   TGTextEntry* G_selection_text;//!
   TList* fSelectedSelections;//!

   KVList fHistolist;//list of generated histograms
   TString fTreeName;//name of analyzed TTree
   TString fTreeFileName;//name of file containing analyzed TTree
   Int_t fHistoNumber;//used for automatic naming of histograms
   Int_t fSelectionNumber;//used for automatic naming of selections
   Int_t fAliasNumber;//used for automatic naming of TTree aliases
   KVList fAliasList;//list of TTree aliases
   Bool_t fNoGui;//! =kTRUE if no graphical interface is required

   Bool_t fMethodCalled;//! allows to know if context menu methods are called

   TH1* HistoToAdd1; //!
   TH1* HistoToAdd2; //!
   TH1* HistoAddResult; //!

   TString fRelativePathToAnalysisFile;//!
   void SetRelativePathToAnalysisFile(const Char_t* p)
   {
      fRelativePathToAnalysisFile = p;
   }

   TString fAnalysisSaveDir;//!

   Bool_t fPROOFEnabled;//!

   void ResetMethodCalled()
   {
      fMethodCalled = kFALSE;
   }
   Bool_t MethodNotCalled()
   {
      return !fMethodCalled;
   }

   void AddHisto(TH1*);
   void AddCut(TCutG*);
   void AddSelection(TEntryList*);
   void ReconnectTree();

   void GenerateHistoTitle(TString& title, const Char_t* exp, const Char_t* sel, const Char_t* weight = "");
   void FillLeafList();

   void AnalysisSaveCheck();
   void SetAnalysisModifiedSinceLastSave(Bool_t);

   void SetEntryList(TEntryList*);

public:
   KVTreeAnalyzer(Bool_t nogui = kTRUE);
   KVTreeAnalyzer(TTree*, Bool_t nogui = kFALSE);
   virtual ~KVTreeAnalyzer();
   void DeleteThis()
   {
      delete this;
   }

   void Copy(TObject& obj) const;

   void SetTree(TTree* t);
   void OpenGUI();
   void GUIClosed();
   TH1* MakeHisto(const Char_t* expr, const Char_t* selection, Int_t nX, Int_t nY = 0, const Char_t* weight = "");
   TH1* RemakeHisto(TH1* h, const Char_t* expr, const Char_t* weight = "");
   TH1* MakeIntHisto(const Char_t* expr, const Char_t* selection, Int_t Xmin, Int_t Xmax, const Char_t* weight = "");
   const KVList* GetHistoList() const
   {
      return &fHistolist;
   }
   TH1* GetHistogram(const Char_t* name) const;
   TList* GetHistosByData(const Char_t* expr);
   TList* GetHistosBySelection(const Char_t* expr);
   TH1* GetHisto(const Char_t* expr, const Char_t* selection, const Char_t* weight = "");
   KVHistogram* GetHistoByTitle(const Char_t* title);
   void DeleteHisto(const Char_t* expr, const Char_t* selection, const Char_t* weight);
   void DeleteSelectedHisto();
   void AddSelectedHistos();
   void HistoAddition(Double_t c1 = 1, Double_t c2 = 1);

   Bool_t MakeSelection(const Char_t* selection);
   void UpdateEntryLists();
   void GenerateSelection();
   //void MakeIPScale();
   //void GenerateIPSelection();
   void CombineSelectionsAnd();
   void CombineSelectionsOr();
   void DeleteSelections();
   void SelectionChanged();
   void SetAlias(const Char_t* name, const Char_t* expr);
   void GenerateAlias();
   TNamed* GetAlias(const Char_t* expr)
   {
      return (TNamed*)fAliasList.FindObjectByType(expr);
   }
   void ShowSelections()
   {
      fSelections.ls();
   }
   void ShowVariables()
   {
      fTree->GetListOfLeaves()->ls();
   }
   void ShowAliases()
   {
      fAliasList.ls();
   }
   void ShowHistos()
   {
      fHistolist.ls();
   }
   void CurrentSelection();
   Bool_t IsCurrentSelection(const Char_t* sel);
   void SetSelection(TObject*);
   void SetSelection(const Char_t*);
   TEntryList* GetSelection(const Char_t*);

   void EnablePROOF(Bool_t yes = kTRUE);
   Bool_t IsPROOFEnabled() const
   {
      return fPROOFEnabled;
   }
   Bool_t IsPROOFEnabledForSelections() const
   {
      // Generation of TEntryList with PROOF only works from 20/11/2015 onwards
      // Variable in .kvrootrc file
      //   KVTreeAnalyzer.PROOFSelections:  on
      // controls whether to use it or not (default: no/off/false)
      return (gEnv->GetValue("KVTreeAnalyzer.PROOFSelections", 0) && fPROOFEnabled);
   }

   static KVTreeAnalyzer* OpenFile(const Char_t* filename, Bool_t nogui = kFALSE);
   void ReadFromFile(const Char_t* filename);
   void ReadFromFile(TFile* f);
   void OpenAnyFile(const Char_t* filepath);
   void OpenChain(const TString& treename, const TString& treetitle, const TSeqCollection* files);

   void DrawHisto(TObject* o, Bool_t gen = kTRUE);
   void DrawCut(TCutG*);
   void DrawLeaf(TObject*);
   void DrawLeafExpr();
   void DrawAsDalitz();
   void SetUserBinning(Bool_t ub)
   {
      fUserBinning = ub;
   }
   void SetUserWeight(Bool_t uw)
   {
      fUserWeight = uw;
   }

   void SetAutoSaveHisto(Bool_t yes = kTRUE)
   {
      fAutoSaveHisto = yes;
      if (yes) SetUpHistoAutoSave();
   }
   void SetStatsHisto(Bool_t yes = kTRUE)
   {
      fStatsHisto = yes;
   }
   void SetNormHisto(Bool_t yes = kTRUE)
   {
      fNormHisto = yes;
      fNormHistoEvents = (yes ? kFALSE : fNormHistoEvents);
      G_histo_norm_events->SetState((EButtonState)fNormHistoEvents);
   }
   void SetNormHistoEvents(Bool_t yes = kTRUE)
   {
      fNormHistoEvents = yes;
      fNormHisto = (yes ? kFALSE : fNormHisto);
      G_histo_norm->SetState((EButtonState)fNormHisto);
   }
   void SetDrawOption(Option_t* option = "")
   {
      fDrawOption = option;
   }

   void SetNewCanvas(Bool_t yes = kTRUE)
   {
      fNewCanvas = yes;
      // deselect 'same' if it was selected
      if (G_histo_same->IsOn()) {
         G_histo_same->SetState((EButtonState)kFALSE);
         fDrawSame = kFALSE;
      }
   }
   void SetDrawSame(Bool_t yes = kTRUE)
   {
      fSameColorIndex = 0;
      fDrawSame = yes;
      // deselect 'new canvas' if it was selected
      if (G_histo_new_can->IsOn()) {
         G_histo_new_can->SetState((EButtonState)kFALSE);
         fNewCanvas = kFALSE;
      }
   }
   void SetDrawLog(Bool_t yes = kTRUE)
   {
      fDrawLog = yes;
   }
   void SetApplySelection(Bool_t yes = kTRUE)
   {
      fApplySelection = yes;
   }
   void SetProfileHisto(Bool_t yes = kTRUE)
   {
      fProfileHisto = yes;
   }
   void LeafChanged();
   void HistoSelectionChanged();
   void Save();
   void SaveAs(const char* filename = "", Option_t* option = "") const;
//    void FitGum1();
//    void FitGum2();
//    void FitGum3();
//    void FitGausGum1();
//    void FitGausGum2();
//    void FitGausGum3();

   Int_t GetNx()
   {
      return fNx;
   }
   Int_t GetNy()
   {
      return fNy;
   }
   Double_t GetXmin()
   {
      return fXmin;
   }
   Double_t GetXmax()
   {
      return fXmax;
   }
   Double_t GetYmin()
   {
      return fYmin;
   }
   Double_t GetYmax()
   {
      return fYmax;
   }

   Int_t GetNxD()
   {
      return fNxD;
   }
   Int_t GetNyD()
   {
      return fNxD;
   }
   Bool_t GetOrderedDalitz()
   {
      return fOrderedDalitz;
   }

   Int_t GetNxF()
   {
      return fNxF;
   }
   Double_t GetXminF()
   {
      return fXminF;
   }
   Double_t GetXmaxF()
   {
      return fXmaxF;
   }

   const char* GetWeight()
   {
      return fWeight;
   }

   void DefineUserBinning(Int_t Nx, Int_t Ny, Double_t Xmin, Double_t Xmax, Double_t Ymin, Double_t Ymax);// *MENU* *ARGS={Nx=>fNx,Ny=>fNy,Xmin=>fXmin,Xmax=>fXmax,Ymin=>fYmin,Ymax=>fYmax}
   void DefineUserBinning1F(Int_t NxF, Double_t XminF, Double_t XmaxF);// *MENU* *ARGS={NxF=>fNxF,XminF=>fXminF,XmaxF=>fXmaxF}
   void DefineUserBinningD(Int_t NxD, Int_t NyD, Int_t ordered);// *MENU* *ARGS={NxD=>fNxD,NyD=>fNyD,ordered=>fOrderedDalitz}
   void DefineWeight(const Char_t* Weight);// *MENU* *ARGS={Weight=>fWeight}

   void SetUpHistoAutoSave();

   void HandleHistoFileMenu(Int_t);
   void HandleSelectionsMenu(Int_t);
   void AutoSaveHisto(TH1* h);

   // For applying existing analysis to new data
   void GenerateAllSelections(TCollection*);
   void GenerateAllHistograms(TCollection*);
   void GenerateAllAliases(TCollection* list);
   void HistoFileMenu_Apply();
   void ReapplyAnyFile(const Char_t* filepath);
   void SetTreeFileName(TTree* t);

   static KVList* GetListOfAnalyzers()
   {
      return fgAnalyzerList;
   }
   static KVTreeAnalyzer* GetAnalyzer(const Char_t* title)
   {
      return (KVTreeAnalyzer*)fgAnalyzerList->FindObjectByTitle(title);
   }

   void MakeAbsoluteIPScale(const char* name, Double_t sigmaTot);
   void GenerateConstantXSecSelections(const char* name, Double_t sigmaTot, Double_t sigmaBin);// *MENU*

   void HandleOptionsMenu(Int_t opt);
   void OpenAnyFriendFile(const Char_t* filepath);
   void HistoFileMenu_OpenFriend();
   Long64_t GetEntriesInCurrentSelection() const;
   ClassDef(KVTreeAnalyzer, 5) //KVTreeAnalyzer
   void OpenSingleFile(TFile* file);

   TChain* GetChain() const
   {
      return fChain;
   }
};
//................  global variable
R__EXTERN  KVTreeAnalyzer* gTreeAnalyzer;

#endif
