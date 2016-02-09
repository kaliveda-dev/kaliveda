//$Id: KVDataBaseBrowser.cpp,v 1.2 2006/10/19 14:32:43 franklan Exp $
#include "Riostream.h"
#include "KVDataBaseBrowser.h"
#include "KVMaterial.h"
#include "KVWidget.h"
#include "KVDataBase.h"
#include "KVWidgetList.h"

using namespace std;

ClassImp(KVDataBaseBrowser)
///////////////////////////////////////////////////////////////////////////////////////////////////////
// KaliVeda DataBase Browser and Configuration Tool
//////////////////////////////////////////////////////////////////////////////////////////////////////
//______________________________________________________________________________________
KVDataBaseBrowser::KVDataBaseBrowser(const TGWindow* p,
                                     KVDataBase* d, UInt_t w,
                                     UInt_t h): TGMainFrame(p, w, h)
{

   // pointer to DataBase under examination
   fDbase = d;
//_______________________________________________________________________________________
// Create top level window.
//______________________________________________________________________________________

   //combo box for showing all tables in database
   KVWidget* widg = new KVWidget(this, kDB_TABLE_LIST);
   fCB_Tables = new TGComboBox(this, widg->GetID());
   fCB_Tables->Associate(this);
   widg->SetWidget(fCB_Tables);
   AddToWidgetList(widg);
   // fill list with all available tables
   TIter next_tab(fDbase->GetTables());
   KVDBTable* tab;
   Int_t i = 0;
   while ((tab = (KVDBTable*) next_tab())) {
      fCB_Tables->AddEntry(tab->GetName(), i++);
   }
   //highlight first table
   fCB_Tables->Select(0);
   //layout
   TGLayoutHints* lay =
      new TGLayoutHints(kLHintsCenterX | kLHintsTop, 2, 2, 2, 2);
   AddFrame(fCB_Tables, lay);
   fCB_Tables->Resize(150, 20);
   AddToCleanupList(lay);

   //combo box for showing all records in the table selected in fCB_Tables
   widg = new KVWidget(fDbase, kDB_RECORD_LIST);
   fCB_Records = new TGComboBox(this, widg->GetID());
   fCB_Records->Associate(this);
   widg->SetWidget(fCB_Records);
   AddToWidgetList(widg);
   //fill list with records from first (selected) table
   FillRecordsList(0);
   //layout
   AddFrame(fCB_Records, lay);
   fCB_Records->Resize(150, 20);
   AddToCleanupList(lay);

//______________________________________________________________________________________
   // Display main window
   MapSubwindows();
   Layout();
   Resize(200, 200);

   SetWindowName(fDbase->GetName());
   SetIconName("KVDataBaseBrowser");
   MapWindow();
}

//_________________________________________________________________________________________

void KVDataBaseBrowser::CloseWindow()
{
   //fDbase->CloseBrowser();
}

//________________________________________________________________________________

Bool_t KVDataBaseBrowser::ProcessMessage(Long_t msg, Long_t parm1,
      Long_t parm2)
{
   // Handle messages send to the TestMainFrame object. E.g. all menu button
   // messages.

   // get associated widget from widget list using calling widget ID (parm1)
   KVWidget* widg = 0;
   switch (GET_MSG(msg)) {

      case kC_COMMAND:
         switch (GET_SUBMSG(msg)) {

            case kCM_BUTTON:
               //printf("Button was pressed, id = %ld, widget action...\n", parm1);
               if (GetWidgetList())
                  widg = GetWidgetList()->GetWidget(parm1);
               if (widg)
                  widg->Action();
               break;

            case kCM_COMBOBOX:
               if (GetWidgetList())
                  widg = GetWidgetList()->GetWidget(parm1);
               if (widg)
                  widg->Action(parm2);

            default:
               break;
         }

      case kC_TEXTENTRY:
         switch (GET_SUBMSG(msg)) {

            case kTE_TEXTCHANGED:
               cout << "TEXTCHANGED" << endl;
               if (GetWidgetList())
                  widg = GetWidgetList()->GetWidget(parm1);
               if (widg)
                  widg->Action();
         }

      default:
         break;
   }
   return kTRUE;
}

//__________________________________________________________________________________________//

void KVDataBaseBrowser::FillRecordsList(Long_t table)
{
   //Fill fCB_Records with records found in table number "table" in the GetTables() list of dataBase

   //How many entries in fCB_Records already ?
   Int_t n_records = fCB_Records->GetListBox()->GetNumberOfEntries();

   //Clear out any existing records in list
   if (n_records)
      fCB_Records->RemoveEntries(0, n_records - 1);

   //Get list of records for our table
   KVDBTable* dbtab = (KVDBTable*) fDbase->GetTables()->At(table);

   // fill list with all available tables
   TIter next_rec(dbtab->GetRecords());
   KVDBRecord* rec;
   Int_t i = 0;
   while ((rec = (KVDBRecord*) next_rec())) {
      fCB_Records->AddEntry(rec->GetName(), i++);
   }
   //highlight first table
   fCB_Records->Select(0);
}
