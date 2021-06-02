/***************************************************************************
                          kvbase.h  -  description
                             -------------------
    begin                : Thu May 16 2002
    copyright            : (C) 2002 by J.D. Frankland
    email                : frankland@ganil.fr

$Id: KVBase.h,v 1.42 2009/03/12 13:59:40 franklan Exp $
***************************************************************************/

#ifndef KVBASE_H
#define KVBASE_H

#include "KVConfig.h"
#include "TRandom3.h"
#include "TNamed.h"
#include "RVersion.h"
#include "TString.h"
#include "KVLockfile.h"
#include "Riostream.h"
#include "TPluginManager.h"
#include "TFile.h"
#include "TF1.h"
class KVString;

/**
  \class KVBase
\brief Base class for KaliVeda framework
\ingroup Core

This is the base class for many classes in the KaliVeda framework. Each
KVBase object has:

 - a name - Get/SetName()
 - a type - Get/SetType()
 - a number - Get/SetNumber()
 - a label - Get/SetLabel()

When objects are accessed through a TObject/TNamed base pointer, it is possible
to test whether an object is derived from KVBase, using the bit
`KVBase::kIsKaliVedaObject`:
~~~~~{.cpp}
    TObject* ob = (address of some object);
    if( ob->TestBit( KVBase::kIsKaliVedaObject ) ){  ... }
~~~~~
This class also provides a number of general utilities, often as static
(stand-alone) methods.

<h3>KaliVeda build/installation information</h3>

The static methods
~~~~~{.cpp}
    KVBase::GetKVSourceDir()
    KVBase::GetKVBuildDate()
    KVBase::GetKVBuildType()
    KVBase::GetKVBuildUser()
    KVBase::GetKVVersion()
~~~~~
give info on the sources, when and where they were built, and by whom.

<h3>Initialisation</h3>

The entire KaliVeda framework is initialised by the static method KVBase::InitEnvironment()


<h3>Finding/opening files</h3>

Static methods for easily locating and/or opening files within the
KaliVeda installation tree are given:
~~~~~~{.cpp}
    KVBase::SearchKVFile(...)
    KVBase::SearchAndOpenKVFile(...)
~~~~~~
Note that in the second case, two methods exist: one for reading, the
other for writing the (ascii) files. A global function for searching
files is also defined:
~~~~~{.cpp}
    Bool_t SearchFile(const Char_t* name, TString& fullpath, int ndirs, ...)
~~~~~
This will search for a
file in an arbitrary number of locations, return kTRUE if file is found
and put full path to file in 'fullpath':

 - 'name' is a filename (not an absolute pathname) i.e. "toto.dat"
 - 'fullpath' will contain the full path to the file if it is found (if file not found, fullpath="")
 - 'ndirs' is the number of directories to search in

The remaining arguments are the names of 'ndirs' paths to search in,

<h3>Finding class source files</h3>

Source files for a class can be found using static method
~~~~~{.cpp}
    KVBase::FindClassSourceFiles(...)
~~~~~
It will look for appropriately-named files corresponding to the header
and implementation file of a class, testing several popular suffixes
in each case.

<h3>Finding executables</h3>

To find an executable in the current user's 'PATH' (or elsewhere), use
static method
~~~~~{.cpp}
    KVBase::FindExecutable(...)
~~~~~
<h3>Temporary files</h3>

The static methods
~~~~~~{.cpp}
    KVBase::GetTempFileName(...)
    KVBase::OpenTempFile(...)
~~~~~~
can generate and handle uniquely-named temporary (ascii) files.

<h3>Backing-up files</h3>

The static method
~~~~~~{.cpp}
    KVBase::BackupFileWithDate(...)
~~~~~~
can be used to create a dated backup of an existing file before it is
replaced with a new version.

<h3>Handling plugins</h3>

As plugins are extensively used in the KaliVeda framework, a few
utilities for handling them are defined. They are static methods
~~~~~~~~~{.cpp}
    KVBase::LoadPlugin(...)
    KVBase::GetPluginURI(...)
~~~~~~~~~
*/

class KVBase: public TNamed {

private:
   static UInt_t fNbObj;        //! Total number of KaliVeda objects (actually counts number of calls to default ctor)
   UInt_t fNumber;              // for numbering objects
protected:
   TString fLabel;             //label for the object

private:
   static Bool_t fEnvIsInit;//! set when environment is initialised
   static TString fWorkingDirectory;//! user working directory for e.g. database files

   void init();
   static void ReadGUIMimeTypes();
   static void ReadConfigFiles();

public:

   enum EKaliVedaBits {
      kIsKaliVedaObject = BIT(23)   //all KVBase-derived objects have this bit set in TObject::fBits
   };

   static void InitEnvironment();
   KVBase();
   KVBase(const Char_t* name, const Char_t* title = "");
   KVBase(const KVBase&);
   KVBase& operator=(const KVBase&);
   virtual ~ KVBase();

   virtual void SetType(const Char_t* str)
   {
      SetTitle(str);
   }
   const Char_t* GetType() const
   {
      return GetTitle();
   }
   static const Char_t* WorkingDirectory()
   {
      return fWorkingDirectory.Data();
   }
   virtual Bool_t IsType(const Char_t* typ) const
   {
      // Returns true if GetType() returns the given type name
      return (fTitle == typ);
   }
   virtual Bool_t IsCalled(const Char_t* name) const
   {
      // Returns true if object has given name (test value returned by GetName())
      return !strcmp(GetName(), name);
   }
   void SetLabel(const Char_t* lab)
   {
      fLabel = lab;
   }
   const Char_t* GetLabel() const
   {
      return fLabel;
   }
   Bool_t HasLabel() const
   {
      return fLabel.Length();
   }
   Bool_t IsLabelled(const Char_t* l) const
   {
      // return true if object label is "l"
      return fLabel == l;
   }
   virtual void Print(Option_t* option = "") const;
   virtual void List()
   {
   }
   virtual void SetNumber(UInt_t num)
   {
      fNumber = num;
   }
   UInt_t GetNumber() const
   {
      return fNumber;
   }
   UInt_t GetNumberOfObjects() const
   {
      return fNbObj;
   }
   virtual void Clear(Option_t* opt = "");
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
   virtual void Copy(TObject&) const;
#else
   virtual void Copy(TObject&);
#endif

   static Bool_t SearchKVFile(const Char_t* name, TString& fullpath,
                              const Char_t* kvsubdir = "");
   static Bool_t SearchAndOpenKVFile(const Char_t* name, std::ifstream& file,
                                     const Char_t* kvsubdir = "", KVLockfile* locks = 0);
   static Bool_t SearchAndOpenKVFile(const Char_t* name, std::ofstream& file,
                                     const Char_t* kvsubdir = "", KVLockfile* locks = 0);
   static void BackupFileWithDate(const Char_t* path);
   static TPluginHandler* LoadPlugin(const Char_t* base,
                                     const Char_t* uri = "0");
   static const Char_t* GetPluginURI(const Char_t* base, const Char_t* plugin);
   static const Char_t* GetListOfPlugins(const Char_t* base);
   static const Char_t* GetListOfPluginURIs(const Char_t* base);
   static Bool_t IsThisAPlugin(const TString& uri, TString& base);
   static void OpenTempFile(TString& base, std::ofstream& fp);       /* open temp file with basename 'base' */
   static void GetTempFileName(TString& base);

   static const Char_t* GetKVVersion();
   static const Char_t* GetKVBuildDate();
   static const Char_t* GetKVBuildTime();
   static const Char_t* GetKVBuildType();
   static const Char_t* GetKVBuildUser();
   static const Char_t* GetKVSourceDir();
   static const Char_t* GetKVBuildDir();

#ifdef WITH_GIT_INFOS
   static const Char_t* gitBranch();
   static const Char_t* gitCommit();
#endif

   static const Char_t* GetETCDIRFilePath(const Char_t* namefile = "");
   static const Char_t* GetDATADIRFilePath(const Char_t* namefile = "");
   static const Char_t* GetDATABASEFilePath();
   static const Char_t* GetLIBDIRFilePath(const Char_t* namefile = "");
   static const Char_t* GetINCDIRFilePath(const Char_t* namefile = "");
   static const Char_t* GetBINDIRFilePath(const Char_t* namefile = "");
   static const Char_t* GetWORKDIRFilePath(const Char_t* namefile = "");
   static const Char_t* GetTEMPLATEDIRFilePath(const Char_t* namefile = "");
   static const Char_t* GetExampleFilePath(const Char_t* library, const Char_t* namefile);
   static bool is_gnuinstall()
   {
      // return true if built with option -Dgnuinstall=yes
      // i.e. GNU-style installation directories, any generated files
      // stored in user's working directory i.e. not in installation path
#ifdef WITH_GNU_INSTALL
      return true;
#else
      return false;
#endif
   }

   static Bool_t FindExecutable(TString& exec, const Char_t* path =
                                   "$(PATH)");

   static const Char_t* FindFile(const Char_t* search, TString& wfil);
   static Bool_t FindClassSourceFiles(const Char_t* class_name, KVString& imp_file,
                                      KVString& dec_file, const Char_t* dir_name = ".");

   static Bool_t AreEqual(Double_t x, Double_t y, Long64_t maxdif = 1);

   static Bool_t OpenContextMenu(const char* method, TObject* obj, const char* alt_method_name = "");

   static void CombineFiles(const Char_t* file1, const Char_t* file2, const Char_t* newfilename, Bool_t keep = kTRUE);

   virtual TObject* GetObject() const;

   static void PrintSplashScreen();
   static const Char_t* GetDataSetEnv(const Char_t* dataset, const Char_t* type, const Char_t* defval);
   static Double_t GetDataSetEnv(const Char_t* dataset, const Char_t* type, Double_t defval);
   static Bool_t GetDataSetEnv(const Char_t* dataset, const Char_t* type, Bool_t defval);

   Double_t ProtectedGetX(const TF1* func, Double_t val, int& status, Double_t xmin = 0.0, Double_t xmax = 0.0) const;

   ClassDef(KVBase, 4)          //Base class for all KaliVeda objects
};

//this function is implemented in TSystem.cxx
//it can be used with TSystem::ConcatFileName to mop up the returned char* pointer
extern void AssignAndDelete(TString& target, char* tobedeleted);
//Search for file in an arbitrary number of locations, return kTRUE if file found and put full path to file in 'fullpath"
Bool_t SearchFile(const Char_t* name, TString& fullpath, int ndirs, ...);
#endif
