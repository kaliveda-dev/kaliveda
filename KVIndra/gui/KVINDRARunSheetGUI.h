#include "TGFrame.h"
#include "TG3DLine.h"
#include "TGMenu.h"
#include "TGListBox.h"
#include "TGButtonGroup.h"
#include "TGButton.h"
#include "RQ_OBJECT.h"
#include <TGComboBox.h>
#include <TGLabel.h>
#include <TGNumberEntry.h>
#include <TGTextEntry.h>
#include <TGProgressBar.h>
#include <TGProgressBar.h>
#include "KVINDRARunSheetReader.h"
#include "KVString.h"
#include "TEnv.h"
#include "KVListView.h"
#include "KVConfig.h"

class KVINDRADBRun;
class KVDataSet;

//_________________________________________________//

class KVIRSGChooseDataSetDialog {

   RQ_OBJECT("KVIRSGChooseDataSetDialog")

   TGTransientFrame* fMain;
   TGLabel* fLabel;
   ULong_t ucolor;
   TGComboBox* fComboBox;
   TGTextButton* fTextButton;
   Int_t fDataSetIndex;

public:

   KVIRSGChooseDataSetDialog()
   {
      fMain = 0;
   };
   KVIRSGChooseDataSetDialog(const TGWindow* p, const TGWindow* main,
                             UInt_t w = 1, UInt_t h = 1);
   virtual ~ KVIRSGChooseDataSetDialog();

   void DoClose();
   void CloseWindow();
   void YouPressedOK();
   void SetDataSet(Int_t i)
   {
      fDataSetIndex = i;
   };

   ClassDef(KVIRSGChooseDataSetDialog, 0)       //Run-sheet reader dialog box for choosing dataset
};

//_________________________________________________//

class KVINDRARunSheetGUI: public TGMainFrame {

   TEnv*  GUIenv;

   KVINDRARunSheetReader fReader;       //used to read runsheets

   TGLabel* fDirLabel;
   TString fDirLabelMess;
   TGLabel* fFormLabel;
   TGHorizontalFrame* fHframe;
   TGVButtonGroup* fButtonsFrame;
//    TGListBox *fRunList;
   KVListView* fRunList;
   TGTextButton* fUpdate;
#ifdef __WITHOUT_TGBUTTON_SETENABLED
   TGTextButton* fViewTreeButton;
#endif
   TGTextButton* fSetSystem;
   TGTextButton* fUnSetSystem;
   TGTextButton* fChangeDataSet;
   TGTextButton* fSaveDB;
   TGTextButton* fQuit;
   TGTextButton* fAddComment;
   TGTextButton* fPrintRun;
   TGTextButton* fAddCahier;
   TGTextButton* fSetTrigger;
   TList* fButList;
   Int_t fFirstRun;
   Int_t fLastRun;
   KVINDRADBRun* fSelectedRun;
   TList* fSelectedEntries;
   KVString fInputString;

public:

   KVINDRARunSheetGUI();
   virtual ~ KVINDRARunSheetGUI();

   void CloseWindow();

   void ChangeDataSet();

   void DisableButtons();
   void EnableButtons();

   void FillListOfRuns();
   void ClearListOfRuns();
   void UpdateListOfRuns();

   void SelectionChanged();
   Int_t GetNSelected();

   void UpdateList();
   void SaveAll();

   void SetRunSheetDir();
   void SetFileFormat();
   void SetRunComment();
   void SetRunCahier();
   void SetRunTrigger();
   void SetSystem();
   void UnsetSystem();
   void StartTreeViewer();

   void PrintRun();

#ifdef __WITHOUT_TGBUTTON_SETENABLED
   void SetViewTreeEnabled(Bool_t on)
   {
      fViewTreeButton->SetState(on ? kButtonUp : kButtonDisabled);
   };
#endif

   ClassDef(KVINDRARunSheetGUI, 0)     //A GUI for managing run sheet information during experiment
};
