/*
$Id: KVDatedFileManager.cpp,v 1.3 2007/06/14 10:39:55 franklan Exp $
$Revision: 1.3 $
$Date: 2007/06/14 10:39:55 $
*/

//Created by KVClassFactory on Wed May  2 14:50:40 2007
//Author: franklan

#include "KVDatedFileManager.h"
#include "TSystemDirectory.h"
#include "TSystem.h"
#include "KVError.h"

ClassImp(KVDatedFileManager)


KVDatedFileManager::KVDatedFileManager(const Char_t* base, const Char_t* dir): fBaseName(base), fDirectory(dir)
{
   //Read all versions of file with basename 'base' in directory 'dir' and put them
   //in a list sorted according to their timestamps
   //Environment variables ($KVROOT) and other special symbols ('~') can be used
   //in the directory name.
   ReadFiles();
}

//__________________________________________________________________________________//

void KVDatedFileManager::ReadFiles()
{
   //Read all versions of file with given basename in directory and put them
   //in a list sorted according to their timestamps
   gSystem->ExpandPathName(fDirectory);
   TSystemDirectory DIR("dir", fDirectory.Data());
   TList* files = DIR.GetListOfFiles();
   if (!files) {
      Error(KV__ERROR(ReadFiles), "Cannot read directory %s", fDirectory.Data());
      return;
   }
   TIter next(files);
   TObject* file;
   while ((file = next())) {
      KVString nom(file->GetName());
      if (nom.BeginsWith(fBaseName)) fFileList.Add(new KVSortableDatedFile(nom, fBaseName));
   }
   delete files;
   if (fFileList.GetEntries()) fFileList.Sort();
}

//__________________________________________________________________________________//

const Char_t* KVDatedFileManager::GetPreviousVersion(const Char_t* name)
{
   //Returns name of file with timestamp immediately prior to the given name.
   //Example:
   //   to know the name of the most recent backup version, give the base name of the file
   //
   //If no older version exists, returns empty string

   TObject* obj = fFileList.FindObject(name);
   if (!obj) {
      Error(KV__ERROR(GetPreviousVersion), "File %s does not exist", name);
      return "";
   }
   Int_t index = fFileList.IndexOf(obj);
   if (index == (fFileList.GetEntries() - 1)) {
      //file is already oldest version
      Warning(KV__ERROR(GetPreviousVersion), "File %s is oldest known version", name);
      return "";
   }
   return fFileList.At(index + 1)->GetName();
}

//__________________________________________________________________________________//

const Char_t* KVDatedFileManager::GetOldestVersion()
{
   //Returns name of file with earliest timestamp
   return fFileList.Last()->GetName();
}

//__________________________________________________________________________________//

void KVDatedFileManager::Update()
{
   //Updates list of files
   fFileList.Clear();
   ReadFiles();
}

//__________________________________________________________________________________//

ClassImp(KVSortableDatedFile)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVSortableDatedFile</h2>
<h4>A filename with an SQL-format date extension which can be used to sort a list according to date</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVSortableDatedFile::KVSortableDatedFile(const Char_t* filename, const Char_t* basename)
   : TNamed(filename, basename)
{
   //filename = full name of file (including timestamp)
   //basename = basename of file
   //Example:
   //        KVSortableDatedFile("Runlist.csv.2007-05-02_14:15:58", "Runlist.csv")
   //The timestamp extension is extracted from the filename.
   //If the filename given is the same as the basename (i.e. no timestamp in name),
   //the timestamps is set to the current date & time
   KVString f(filename), b(basename);
   if (f != b) {
      f.Remove(0, b.Length() + 1);
      f.ReplaceAll("_", " ");
      fTimestamp.SetSQLDate(f.Data());
   }
}

//___________________________________________________________________________

Int_t KVSortableDatedFile::Compare(const TObject* obj) const
{
   // Compare two files according to their timestamp.
   // Used to sort lists, most recent files appear first

   KVSortableDatedFile* f =
      dynamic_cast < KVSortableDatedFile* >(const_cast < TObject* >(obj));
   return (f->fTimestamp == fTimestamp ? 0 : (f->fTimestamp > fTimestamp ? 1 : -1));
}

//___________________________________________________________________________

Bool_t KVSortableDatedFile::IsEqual(const TObject* obj) const
{
   //Two files are the same if they have the same basename and timestamp

   KVSortableDatedFile* f =
      dynamic_cast < KVSortableDatedFile* >(const_cast < TObject* >(obj));
   return (f->fTimestamp == fTimestamp && !strcmp(GetName(), obj->GetName()));
}
