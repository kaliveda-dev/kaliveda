#include "TGFrame.h"
#include "TGMenu.h"
#include "TGListBox.h"
#include "TGButtonGroup.h"
#include "TGButton.h"
#include "KVIDGrid.h"
#include "RQ_OBJECT.h"
#include <TGComboBox.h>
#include <TGLabel.h>
#include <TGNumberEntry.h>
#include <TGTextEntry.h>
#include <TGProgressBar.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// KVMergeGridsDialog
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////

class KVMergeGridsDialog {

   RQ_OBJECT("KVMergeGridsDialog")

   TGTransientFrame* fMain;

   TGGroupFrame* fGFrame;

   TGHorizontalFrame* fG1IDFrame;
   TGLabel* fG1NameLabel;
   TGLabel* fG1IDminLabel;
   TGLabel* fG1IDmaxLabel;
   TGNumberEntry* fG1IDminEntry;
   TGNumberEntry* fG1IDmaxEntry;

   TGHorizontalFrame* fG2IDFrame;
   TGLabel* fG2NameLabel;
   TGLabel* fG2IDminLabel;
   TGLabel* fG2IDmaxLabel;
   TGNumberEntry* fG2IDminEntry;
   TGNumberEntry* fG2IDmaxEntry;

   TGHorizontalFrame* fButtonsFrame;
   TGTextButton* fOKButton;
   TGTextButton* fCancelButton;

   KVIDGrid* fGrid1;
   KVIDGrid* fGrid2;

   static Int_t G1IDmin;
   static Int_t G1IDmax;
   static Int_t G2IDmin;
   static Int_t G2IDmax;

public:
   KVMergeGridsDialog(KVIDGrid* g1, KVIDGrid* g2, const TGWindow* p =
                         0, const TGWindow* main = 0, UInt_t w =
                         1, UInt_t h = 1);
   virtual ~ KVMergeGridsDialog();

   void DoClose();
   void CloseWindow();
   void MergeGrids();

   ClassDef(KVMergeGridsDialog, 0)     //dialog box for merging ID grids
};
