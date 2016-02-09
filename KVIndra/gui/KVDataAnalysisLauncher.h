//
// Entete de la classe fenetre principale
//
#ifndef KVDataAnalysisLauncher_h
#define KVDataAnalysisLauncher_h

#include "TROOT.h"
#include "TSystem.h"
#include "TGClient.h"
#include "TGButton.h"
#include "TGTextEntry.h"
#include "TGComboBox.h"
#include "TGSlider.h"
#include "TGListBox.h"
#include "TGNumberEntry.h"
#include "TSortedList.h"
#include "TGLabel.h"
#include "TGTab.h"
#include "TEnv.h"
#include "TApplication.h"

#include "KVNumberList.h"
#include "KVDataAnalyser.h"
#include "KVListView.h"
#include "KVConfig.h"

#define USE_KVTIMEENTRY
#ifdef USE_KVTIMEENTRY
#include "KVTimeEntry.h"
#endif

class KVGFileList : public TGTransientFrame {
   enum ElementId {
      B_Add,
      B_Remove,
      B_AllRemove,
      LB_Files,
      TE_FileName
   };

protected:
   TGListBox*   lbFileList;
   TGTextEntry* teFileName;
   TGTextButton*   boutAdd;
   TGTextButton*   boutRem;
   TGTextButton*   boutAllRem;
   Int_t    entryMax;
   TString*     fileListString;
   TString     fileDialogDir;

   virtual void Init(TString& fileList, const Char_t* title);
   virtual void InitFileList();

public:
   KVGFileList(TString& st, const Char_t* titre = "File list",
               const TGWindow* p = 0, const TGWindow* main = 0, Bool_t ok = kTRUE);
   ~KVGFileList();

   virtual void AddFile(void);
   virtual void RemoveFiles(void);
   virtual void RemoveAllFiles(void);
   virtual void Done(void);
   virtual Char_t* GetFileFromDialog(void);
   virtual Bool_t CanAdd(const Char_t* s);

   ClassDef(KVGFileList, 0)
};

class KVGDirectoryList : public KVGFileList {

protected:
   virtual void Init(TString& fileList, const Char_t* title);
public:
   KVGDirectoryList(TString& st, const Char_t* titre = "File list",
                    const TGWindow* p = 0, const TGWindow* main = 0, Bool_t ok = kTRUE);
   ~KVGDirectoryList();

   virtual Char_t* GetFileFromDialog(void);
   virtual Bool_t CanAdd(const Char_t* s);

   ClassDef(KVGDirectoryList, 0)
};


class KVDataAnalysisLauncher : public TGMainFrame {
#ifdef __WITHOUT_TGCOMBOBOX_REMOVEALL
   void RemoveAll(TGComboBox* box);
   void RemoveAll(TGListBox* box);
#endif
   enum ElementId {
      CB_DataRepository,
      CB_AnalysisTask,
      CB_DataSet,
      CB_System,
      CB_Trigger,
      LB_Runs,
      B_Libs,
      B_Incs,
      B_Process,
      B_Quit
   };

   // Declaration des boutons de la fenetre principale
protected:
   TEnv*      GUIenv;
   TList*      ResourceNames;//used by Get/SetResource
   Int_t       NbResNames;//number of names in list
   void SetResource(const Char_t* name, const Char_t* value);
   const Char_t* GetSavedResource(const Char_t* name, const Char_t* defaultvalue = "");
   const Char_t* GetResource(const Char_t* name, const Char_t* defaultvalue = "");
   void BuildResourceName(const Char_t* name, TString&, TString&);

   KVNumberList  listOfRuns;
   KVNumberList listOfSystemRuns;

   TGComboBox* cbRepository;
   TGComboBox* cbTask;
   TGComboBox* cbDataSet;
   //TGComboBox *cbSystem;
   KVListView* lvSystems;
   KVDBSystem* lastSelectedSystem;
   //TGComboBox *cbTrigger;
   //TGListBox  *lbRuns;
   KVListView* lvRuns;
   TGLabel*    selectedRuns;
   Int_t     entryMax;
   Bool_t       selAll;

   KVDataAnalyser* ia;
   TGCompositeFrame* cfAnalysis;
   //TGTextEntry   *teSelector;
   TGComboBox* cbUserClass;
   TGPictureButton* btEditClass;
   TGLabel*         fUserClassLabel;
   TGTextEntry*   teDataSelector;
   TGTextEntry*   teUserOptions;
   TGLabel*         fDataSelectorLabel;
   TGNumberEntry* teNbToRead;

   TGRadioButton* rbInteractive;
   TGRadioButton* rbBatch;
   TGTextEntry*   teBatchName;
   TGTextEntry*   teBatchNameFormat;
   TGCheckButton* chIsBatchNameAuto;

   TGTextEntry*   teBatchMemory;
   TGLabel*         fBatchMemLab;
   TGTextEntry*   teBatchDisk;
   TGLabel*        fBatchDiskLab;
#ifdef USE_KVTIMEENTRY
   KVTimeEntry*   teBatchTime;
#else
   TGNumberEntry* teBatchTime;
#endif
   TGLabel*        fBatchTimeLab;
   TGNumberEntry* runsPerJob;
   TString         fUserLibraries;
   TString         fUserIncludes;

   Bool_t         checkCompilation;
   Bool_t withBatchParams;
   Bool_t noSystems;
   KVDataAnalyser* GetDataAnalyser(KVDataAnalysisTask* task = 0);

   TList* UserClassNames; // list of user classes present in working directory
   void FillListOfUserClasses();
   void SetUserClassList();
   void SetUserClass(const Char_t*);
   const Char_t* GetUserClass();
   void DisableUserClassList();
   void EnableUserClassList();

   UInt_t fMainGuiWidth;//width of main window in pixels
   UInt_t fMainGuiHeight;//heigth of main window in pixels


public:
   KVDataAnalysisLauncher(const Char_t* ExpType, const TGWindow* p = 0, UInt_t w = 200, UInt_t h = 400);
   ~KVDataAnalysisLauncher();
   virtual Bool_t ProcessMessage(Long_t msg, Long_t par1, Long_t par2);

   virtual void Exit(void)
   {
      gApplication->SetReturnFromRun(kTRUE);
      gApplication->Terminate(0);
   }
   virtual void SetRepositoryList(void);
   virtual void SetDataSetList(Char_t* s);
   virtual void SetTaskList(Char_t* s);
   virtual void SetSystemList(Int_t s);
   virtual void SetRunsList();
   void UserClassSelected(char*);
   virtual void SelectAll(void);
   virtual void DeselectAll(void);
   virtual void Process(void);
   virtual void EnterRunlist(void);

   void SystemSelectionChanged();

   virtual const Char_t* GetRepository(void);
   virtual const Char_t* GetDataSet(void);
   virtual const Char_t* GetTask(void);
   virtual const Char_t* GetSystem(void);
   virtual const Char_t* GetRuns(void);

   virtual void SetRepository(const Char_t* r = "");
   virtual void SetDataSet(const Char_t* ds = "");
   virtual void SetTask(const Char_t* t = "");
   virtual void SetSystem(const Char_t* s = "");
   virtual void SetRuns(const Char_t* s = "");

   virtual Bool_t IsBatch(void)
   {
      return !rbInteractive->IsDown();
   }
   virtual void SetBatch(void);
   virtual Bool_t IsBatchNameAuto(void)
   {
      return chIsBatchNameAuto->IsDown();
   }
   virtual void SetBatchNameAuto(void);
   virtual void SetRunsPerJobLimits()
   {
      runsPerJob->SetLimits(TGNumberFormat::kNELLimitMinMax, 1.0, (Double_t)listOfRuns.GetNValues());
   };

   virtual const Char_t* SystemBatchName(void);
   virtual void SetAutoBatchName(void);

   virtual void SetUserLibraries(void);
   virtual void SetUserIncludes(void);

   virtual Bool_t WarningBox(const char* title = "Warning", const char* msg = "Warning", Bool_t confirm = kFALSE);
   void UpdateListOfSelectedRuns();
   void ClearListOfSelectedRuns();
   void EditUserClassFiles();

   ClassDef(KVDataAnalysisLauncher, 0) //Graphical interface for launching analysis tasks: KaliVedaGUI
};

#endif
