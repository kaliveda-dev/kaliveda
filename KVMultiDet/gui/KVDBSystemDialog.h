/*
$Id: KVDBSystemDialog.h,v 1.7 2007/05/03 15:49:02 franklan Exp $
$Revision: 1.7 $
$Date: 2007/05/03 15:49:02 $
*/

//Created by KVClassFactory on Wed Apr  4 12:10:28 2007
//Author: franklan

#ifndef __KVDBSYSTEMDIALOG_H
#define __KVDBSYSTEMDIALOG_H

#include "TGButton.h"
#include "RQ_OBJECT.h"
#include "KVNumberList.h"
#include "KVDatedFileManager.h"
#include "Riostream.h"
class TGTransientFrame;
class TGComboBox;
class TGTextEntry;
class TGListBox;
class TGLabel;
class TGNumberEntry;
class KVDBSystem;
class KVList;
class KVTarget;
class KVMaterial;

/**
\class KVDBSystemDialog
\brief Dialog box for setting system parameters associated to runs
\ingroup GUI
*/

class KVDBSystemDialog {

   RQ_OBJECT("KVDBSystemDialog")

   TGTransientFrame* fMainFrame1475;//the main window

   TGComboBox* fComboBox1476;//combo box displaying list of systems for current dataset

   TGCheckButton* fCheckButton1376;//check button to define projectile
   TGTextEntry* fTextEntry1490;//symbol of projectile
   TGNumberEntry* fNumberEntry1493;//Z of projectile
   TGNumberEntry* fNumberEntry1499;//A of projectile
   TGNumberEntry* fNumberEntry1509;//E/A of projectile

   TGNumberEntry* fNumberEntry1532;//target angle to beam
   TGComboBox* fComboBox1515;//layers in current target
   TGNumberEntry* fNumberEntry1526;//thickness of current layer
   TGLabel* fLabel1530;//units for thickness
   TGNumberEntry* fNumberEntry1537;//atomic mass of current layer
   TGComboBox* fComboBox1542;//list of materials for creating target layers
   TGTextButton* fTextButton1553;//add new layer to target
   TGTextButton* fTextButton1554;//remove current layer from target
   TGTextButton* fCreateSystem;//button to create new system
   TGTextButton* fSaveButton;//save changes
   TGTextButton* fSetRuns;//associate runs with selected system
   KVDBSystem* fSystem;//currently selected system in combo box fComboBox1476
   KVTarget* fTarget;//current target
   KVMaterial* fLayer;//current target layer
   KVNumberList fRuns;//runs selected by user

   unique_ptr<TObjArray> fMaterialsList;//list of all available materials

   KVDatedFileManager* fUndo;//allows to undo changes to Systems.dat
   KVString fCurrentSystemsFile;//name (including timestamp) of currently used Systems.dat

   void RemoveAll(TGComboBox*);
   void RemoveAll(TGListBox*);

   Bool_t fNeedSave;
   void SetNeedSave(Bool_t k = kTRUE)
   {
      fNeedSave = k;
      fSaveButton->SetEnabled(k && (!fCreateSystem->IsEnabled()));
   };
   Bool_t NeedSave() const
   {
      return fNeedSave;
   };

public:

   KVDBSystemDialog(const TGWindow* p, const TGWindow* main, KVDBSystem* init_sys, const KVNumberList& runs,
                    UInt_t w = 1, UInt_t h = 1);
   virtual ~KVDBSystemDialog();

   void DoClose();
   void CloseWindow();

   void CreateMainWindow(const TGWindow* p, const TGWindow* main,
                         UInt_t w, UInt_t h);
   void FillSystemList();
   void SelectSystem(Int_t);
   void DeleteSystem();

   void UpdateSystemProperties();
   void UpdateProjectileProperties();
   void UpdateTargetProperties();
   void UpdateTargetLayerProperties(Int_t);

   void EnableProjectileProperties(Bool_t);

   void ProjectileSymbolChanged();
   void ProjectileZChanged(Long_t);
   void ProjectileAChanged(Long_t);
   void ProjectileEChanged(Long_t);

   void AddNewTargetLayer();
   void RemoveTargetLayer();

   void TargetLayerThicknessChanged(Long_t);
   void TargetLayerAChanged(Long_t);
   void TargetAngleChanged(Long_t);

   void CreateNewSystem();
   void SetRuns();
   void Undo();

   //Signal emitted to tell KVINDRARunSheetGUI to update the runlist
   void UpdateRunlist()
   {
      Emit("UpdateRunlist()");   //  *SIGNAL*
   }

   Bool_t IsProjectileDefined()
   {
      return fCheckButton1376->IsOn();
   };
   Bool_t IsTargetDefined()
   {
      return (fTarget != 0);
   };

   void SaveSystems();

   ClassDef(KVDBSystemDialog, 0) //Dialog box for setting system parameters associated to runs
};

#endif
