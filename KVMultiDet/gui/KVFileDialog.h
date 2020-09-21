#ifndef ROOT_KVFileDialog
#define ROOT_KVFileDialog

/**
   \class KVFileDialog
   \brief Modified version of TGFileDialog file selection dialog
   \ingroup GUI

 This class creates a file selection dialog. It contains a combo box
 to select the desired directory. A listview with the different
 files in the current directory and a combo box with which you can
 select a filter (on file extensions).

 When creating a file dialog one passes a pointer to a TGFileInfo
 object. In this object you can set the fFileTypes and fIniDir to
 specify the list of file types for the filter combo box and the
 initial directory.

 When the KVFileDialog ctor returns the selected
 file name can be found in the TGFileInfo::fFilename field and the
 selected directory in TGFileInfo::fIniDir.

The fFilename and fIniDir are deleted by the TGFileInfo dtor.
*/

#ifndef ROOT_TGFrame
#include "TGFrame.h"
#endif
#include "TGFileDialog.h"

enum EKVFileDialogMode {
   kKVFDOpen,
   kKVFDDirectory,
   kKVFDSave
};


class TGTextBuffer;
class TGTextEntry;
class TGComboBox;
class TGPictureButton;
class TGTextButton;
class TGCheckButton;
class TGListView;
class TGFileContainer;
class TGFSComboBox;

class KVFileDialog : public TGTransientFrame {

protected:
   TGTextBuffer*      fTbfname;  // text buffer of file name
   TGTextEntry*       fName;     // file name text entry
   TGComboBox*        fTypes;    // file type combo box
   TGFSComboBox*      fTreeLB;   // file system path combo box
   TGPictureButton*   fCdup;     // top toolbar button
   TGPictureButton*   fNewf;     // top toolbar button
   TGPictureButton*   fList;     // top toolbar button
   TGPictureButton*   fDetails;  // top toolbar button
   TGCheckButton*     fCheckB;   // set on/off file overwriting for Open dialog
   // OR set on/off multiple file selection for SaveAs dialog
   const TGPicture*   fPcdup;    // picture for fCdup
   const TGPicture*   fPnewf;    // picture for fNewf
   const TGPicture*   fPlist;    // picture for fList
   const TGPicture*   fPdetails; // picture for fDetails
   TGTextButton*      fOk;       // ok button
   TGTextButton*      fCancel;   // cancel button
   TGListView*        fFv;       // file list view
   TGFileContainer*   fFc;       // file list view container (containing the files)
   TGFileInfo*        fFileInfo; // file info passed to this dialog
   EKVFileDialogMode fDlgMode;
private:
   KVFileDialog(const KVFileDialog&);              // not implemented
   KVFileDialog& operator=(const KVFileDialog&);   // not implemented

public:
   KVFileDialog(const TGWindow* p = 0, const TGWindow* main = 0,
                EKVFileDialogMode dlg_type = kKVFDOpen, TGFileInfo* file_info = 0);
   virtual ~KVFileDialog();

   virtual Bool_t ProcessMessage(Long_t msg, Long_t parm1, Long_t parm2);
   virtual void CloseWindow();

   ClassDef(KVFileDialog, 0) //File or directory selection dialog
};

#endif
