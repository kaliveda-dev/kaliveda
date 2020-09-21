//Created by KVClassFactory on Tue Apr 30 09:30:00 2013
//Author: dgruyer

#include "KVLauncherGUI.h"

ClassImp(KVLauncherGUI)

//________________________________________________________________

KVLauncherGUI::KVLauncherGUI() : TGMainFrame(0, 1, 1, kVerticalFrame)
{
   //  SetDefault();
   //  SetName(ClassName());
   //  Build();
   //  MapAll();
}

//________________________________________________________________
void KVLauncherGUI::SetDefault()
{
   fButtonList.SetOwner(kFALSE);
   SetCleanup(kDeepCleanup);
}

void KVLauncherGUI::Build()
{
   Error("Build", "This method should be overwritten in child classes !");
}

//________________________________________________________________
void KVLauncherGUI::MapAll()
{
   AddButtons();
   DontCallClose();
   ULong_t ucolor;        // will reflect user color changes
   gClient->GetColorByName("#ffffff", ucolor);
   ChangeBackground(ucolor);
   MapSubwindows();
   Resize(GetDefaultSize());
   SetWindowName(GetName());
   MapWindow();
}

//________________________________________________________________
void KVLauncherGUI::AddButtons()
{
   AddButton("Close", "Close");

}

//________________________________________________________________
KVLauncherGUI::~KVLauncherGUI()
{
   // Destructor
}

//________________________________________________________________
TGTextButton* KVLauncherGUI::AddButton(const char* name, const char* method, TObject* obj)
{
   // Add a button to the launcher with given "name".
   // When clicked, this button will call "method" of whatever object obj is pointing to.
   //    - "method" is the name of an argument-less method, without parentheses:
   //       i.e. to call method DummyMethod() when button is clicked, give method="DummyMethod"
   // If obj=0 (default), 'this' will be used.
   // If "method" is not a public interface method of the class of 'obj' an error
   // will be printed and no button added.

   if (!obj) obj = this;
   if (!obj->IsA()->GetListOfAllPublicMethods()->FindObject(method)) {
      Error("AddButton", "%s is not a public method of %s", method, obj->ClassName());
      return NULL;
   }

   ULong_t ucolor;
   gClient->GetColorByName("#66ccff", ucolor);

   TGTextButton* fButton = new TGTextButton(this, name);
   fButton->Connect("Clicked()", obj->ClassName(), obj, Form("%s()", method));
   fButton->SetName(name);
   fButton->SetMargins(5, 5, 5, 5);
   fButton->ChangeBackground(ucolor);

   AddFrame(fButton, new TGLayoutHints(kLHintsRight | kLHintsExpandX, 10, 10, 5, 5));
   fButtonList.AddLast(fButton);

   return fButton;
}

//________________________________________________________________
TGTextButton* KVLauncherGUI::GetButton(const char* method)
{
   return (TGTextButton*)fButtonList.FindObject(method);
}

//________________________________________________________________
void KVLauncherGUI::SetEnabled(const char* method, Bool_t e)
{
   TGTextButton* fButton = GetButton(method);
   if (fButton) fButton->SetEnabled(e);
   else Warning("SetEnabled", "No button corresponding to the method '%s'.", method);
}

//________________________________________________________________
Bool_t KVLauncherGUI::IsEnabled(const char* method)
{
   TGTextButton* fButton = GetButton(method);
   if (fButton) return fButton->IsEnabled();
   else {
      Warning("IsEnabled", "No button corresponding to the method '%s'.", method);
      return kFALSE;
   }
}

//________________________________________________________________
void KVLauncherGUI::DummyMethod()
{

   TGTextButton* fButton = GetButton("DummyMethod");
   Info("DummyMethod", "%s !", fButton->GetName());
   SetEnabled("DummyMethod", !IsEnabled("DummyMethod"));
}

void KVLauncherGUI::Close()
{
   CloseWindow();
}










