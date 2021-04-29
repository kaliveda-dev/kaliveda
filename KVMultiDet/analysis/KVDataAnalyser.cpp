/*
$Id: KVDataAnalyser.cpp,v 1.40 2009/01/14 16:15:46 franklan Exp $
$Revision: 1.40 $
$Date: 2009/01/14 16:15:46 $
$Author: franklan $
*/

#include "KVBase.h"
#include "KVDataAnalyser.h"
#include "KVDataAnalysisTask.h"
#include "KVDataSetManager.h"
#include "KVString.h"
#include "TObjString.h"
#include "TObjArray.h"
#include "Riostream.h"
#include "KVBatchSystemManager.h"
#include "TPluginManager.h"
#include "TSystemDirectory.h"
#include "TROOT.h"
#include "TClass.h"
#include "THashList.h"
#include "KVError.h"

using namespace std;
Bool_t KVDataAnalyser::fCleanAbort = kFALSE;

ClassImp(KVDataAnalyser)

KVDataAnalyser* gDataAnalyser = 0;

KVDataAnalyser::KVDataAnalyser()
{
   //Default constructor.
   fParent = nullptr;
   fBatch = kFALSE;
   fTask = nullptr;
   fQuit = kFALSE;
   fSubmit = kFALSE;
   nbEventToRead = -1;
   fUserClassIsOK = kFALSE;
   fUserClass = "";
   fBatchEnv = new TEnv(".KVBatchrc");
   if (!gBatchSystemManager) new KVBatchSystemManager;
   fBatchSystem = 0;
   fChoseRunMode = kFALSE;
   fWorkDirInit = fWorkDirEnd = 0;
   fMenus = kFALSE;
#ifdef WITH_CPP11
   fProofMode = EProofMode::None;
#else
   fProofMode = None;
#endif
   fUseBaseClassSubmitTask = kFALSE;
}

KVDataAnalyser::~KVDataAnalyser()
{
   //Default destructor.
   delete fBatchEnv;
   SafeDelete(fWorkDirInit);
   SafeDelete(fWorkDirEnd);
   if (gDataAnalyser == this)gDataAnalyser = nullptr;
}

void KVDataAnalyser::Reset()
{
   fDataType = "";
   fTask = nullptr;
   fQuit = kFALSE;
   fSubmit = kFALSE;
   fUserClassIsOK = kFALSE;
   fUserClass = "";
   fUserClassOptions = "";
   nbEventToRead = -1;
   fBatchSystem = nullptr;
   fChoseRunMode = kFALSE;
#ifdef WITH_CPP11
   fProofMode = EProofMode::None;
#else
   fProofMode = None;
#endif
}

//_________________________________________________________________

void KVDataAnalyser::Run()
{
   //Check all task variables, then run analyser

   fSubmit = kFALSE;
   //print welcome message for main (parent) analyser
   if (!fParent && !fMenus) KVBase::PrintSplashScreen();

   if (CheckTaskVariables()) {
      if (fBatchSystem && !BatchMode()) {
         //if batch mode is requested, the job is submitted to the chosen batch system
         fBatchSystem->SubmitTask(this);
      }
      else {
         if (BatchMode() && fBatchSystem && !fParent) fBatchSystem->Print("log");
         if (!RunningInLaunchDirectory() && fParent) {
            //when batch job runs in directory different to launch directory,
            //we scan the list of files present in the current working directory
            //just prior to running the analysis task
            ScanWorkingDirectory(&fWorkDirInit);
         }
         if (!PreSubmitCheck()) return;

         // The following horrific kludge is supposed to solve the following problem:
         //   when running in batch mode (GridEngine) the data analyser created in
         //   KaliVedaAnalysis.cpp has to be of the right type (KVSimDirAnalyser or KVDataSetAnalyser)
         //   in order for it to read all necessary informations from the batch env file
         //   by calling the appropriate ReadBatchEnvFile override.
         //   However, when Run() is called for this object from KaliVedaAnalysis.cpp,
         //   we have to call the KVDataAnalyser::SubmitTask method, not the override in the
         //   derived class.
         if (fUseBaseClassSubmitTask)
            KVDataAnalyser::SubmitTask();
         else
            SubmitTask();

         if (!RunningInLaunchDirectory() && fParent) {
            //when batch job runs in directory different to launch directory,
            //we scan the list of files present in the current working directory
            //just after running the analysis task
            ScanWorkingDirectory(&fWorkDirEnd);
            //any files which are present in the second list which were not present
            //in the first list will be copied back to the launch directory
            CopyAnalysisResultsToLaunchDirectory();
         }
         if (BatchMode() && fBatchSystem && !fParent) {
            // at end of batch jobs,
            // remove .[jobname] and [jobname].status files from $HOME directory
            // remove .[jobname].bak and [jobname].status.bak files from $HOME directory
            TString ff;
            AssignAndDelete(ff, gSystem->ConcatFileName(gSystem->Getenv("HOME"), Form(".%s", GetBatchName())));
            gSystem->Unlink(ff);
            AssignAndDelete(ff, gSystem->ConcatFileName(gSystem->Getenv("HOME"), Form(".%s.bak", GetBatchName())));
            gSystem->Unlink(ff);
            AssignAndDelete(ff, gSystem->ConcatFileName(gSystem->Getenv("HOME"), Form("%s.status", GetBatchName())));
            gSystem->Unlink(ff);
            AssignAndDelete(ff, gSystem->ConcatFileName(gSystem->Getenv("HOME"), Form("%s.status.bak", GetBatchName())));
            gSystem->Unlink(ff);
         }
      }
   }
   PostRunReset();
}

//_________________________________________________________________

void KVDataAnalyser::RunMenus()
{
   //Run data analyser in menu-driven mode

   fMenus = kTRUE;
   Reset();
   KVBase::PrintSplashScreen();

   while (!fQuit) {
      if (NeedToChooseWhatToDo())
         ChooseWhatToDo();
      else if (NeedToChooseWhatToAnalyse())
         ChooseWhatToAnalyse();
      else if (fSubmit) {
         Run();
         Reset();
         KVBase::PrintSplashScreen();
      }
   }
}

//_________________________________________________________________

Bool_t KVDataAnalyser::CheckTaskVariables()
{
   //Checks the task variables
   //In batch mode, we first set the task variables by reading the
   //batch env file associated with the name set for the batch job

   if (BatchMode()) ReadBatchEnvFile(Form(".%s", GetBatchName()));

   //if (!CheckWhatToAnalyseAndHow()) return kFALSE;

   if (fTask->WithUserClass() && fUserClass != ClassName()) {
      //task requires user analysis class
      if (fUserClass == "") {
         ChooseUserClass();
      }

      if (!CheckIfUserClassIsValid(fUserClassAlternativeBaseClass)) {
         cout << "============> Warning <=============" << endl;
         cout << GetUserClass() << " is not a valid " << fTask->GetUserBaseClass() << endl;
         cout << "Analysis aborted." << endl;
         cout << "====================================" << endl;
         /*if (BatchMode())*/ return kFALSE; // avoid infinite loop in batch mode
         //ChooseUserClass();
      }
   }

   if (nbEventToRead < 0) {
      ChooseNbEventToRead();
   }

   return kTRUE;
}

Bool_t KVDataAnalyser::CheckWhatToAnalyseAndHow()
{
   if (NeedToChooseWhatToDo()) {
      return kFALSE;
   }

   if (NeedToChooseWhatToAnalyse()) {
      return kFALSE;
   }
   return kTRUE;
}



//_________________________________________________________________

void KVDataAnalyser::SetAnalysisTask(KVDataAnalysisTask* at)
{
   //Set analysis task and data type
   //For ways of obtaining pointers to data analysis tasks for any given dataset,
   //see method KVDataSet::GetAnalysisTask(const Char_t* keywords) const.
   fTask = at;
   if (at)
      fDataType = at->GetPrereq();
   else
      fDataType = "";
}


void KVDataAnalyser::SetUserIncludes(const Char_t* incDirs)
{
// Add to the includes paths the user's includes paths
// the includes paths have to be separated by a white space

   fIncludes = "";
   if (!incDirs) {
      return;
   }
   TString tmp = incDirs;
   TString curIncDir = gSystem->GetIncludePath();
   TObjArray* oa = tmp.Tokenize(" ");
   oa->SetOwner(kTRUE);
   TIter next(oa);
   TObjString* st = 0;
   while ((st = (TObjString*)next())) {
      TString id = st->GetString();
      if (id.Length()) {
         fIncludes += id.Data();
         fIncludes += " ";
         if (!curIncDir.Contains(id.Data())) {
            cout << "Include path \"" << id.Data() << "\" added." << endl;
            id.Prepend("-I");
            gSystem->AddIncludePath(id.Data());
         }
      }
   }
   delete oa;
}

//_________________________________________________________________

void KVDataAnalyser::SetUserLibraries(const Char_t* libs)
{
// Load the user's libraries
// the libraries have to be separated by a white space

   fLibraries = "";
   if (!libs) {
      return;
   }
   KVString tmp = libs;
   KVString slib = gSystem->GetLibraries("", "D");

   tmp.Begin(" ");
   while (!tmp.End()) {

      KVString id = tmp.Next();

      Bool_t loaded = kFALSE;
      slib.Begin(" ");
      while (!slib.End() && !loaded) {
         KVString ss = slib.Next();
         if (ss == id) {
            Info("SetUserLibraries", "%s already load", id.Data());
            loaded = kTRUE;
         }
         else {
         }
      }
      if (!loaded) {
         Info("SetUserLibraries", "Library \"%s\"added.", id.Data());
         gSystem->Load(id.Data());
      }
      fLibraries += id.Data();
      fLibraries += " ";
   }

}

//__________________________________________________________________________________//

void KVDataAnalyser::ChooseNbEventToRead()
{
   // Ask user to set number of events to read

   SetNbEventToRead(-1);
   while (nbEventToRead < 0) {
      cout << "Give the number of events to read [<RET>=all]:" << endl;
      KVString ntr;
      ntr.ReadToDelim(cin);
      if (ntr.IsDigit() || !ntr.Length()) {
         if (ntr.Length()) {
            nbEventToRead = (Long64_t) ntr.Atoi();
         }
         else {
            nbEventToRead = 0;
         }
      }
      else {
         cout << "\"" << ntr.
              Data() << "\" is not a number. Please retry." << endl;
      }
   }
}

//__________________________________________________________________________________//

KVDataAnalyser* KVDataAnalyser::GetAnalyser(const Char_t* plugin)
{
   //Creates an instance of a class derived from KVDataAnalyser defined as a plugin

   //check and load plugin library
   TPluginHandler* ph;
   if (!(ph = KVBase::LoadPlugin("KVDataAnalyser", plugin)))
      return 0;

   //execute constructor
   KVDataAnalyser* da = (KVDataAnalyser*) ph->ExecPlugin(0);

   return da;
}

//__________________________________________________________________________________//

void KVDataAnalyser::ChooseUserClass()
{
   //Choose the user's analysis class
   fUserClass = "";
   while (!fUserClass.Length()) {
      cout << "Give the name of the analysis class derived from " << fTask->GetUserBaseClass() << ":" << endl;
      fUserClass.ReadLine(cin);
      fUserClassIsOK = kFALSE;
   }
}

//__________________________________________________________________________________//

Bool_t KVDataAnalyser::DoUserClassFilesExist()
{
   //Check if files containing user's class are present in the working directory.
   //The names of the implementation and header files are stored in fUserClassImp and fUserClassDec.

   return KVBase::FindClassSourceFiles(fUserClass.Data(), fUserClassImp, fUserClassDec);
}

//__________________________________________________________________________________//

Bool_t KVDataAnalyser::CheckIfUserClassIsValid(const KVString& alternative_base_class)
{
   //Return kTRUE if the name of the class given by the user (fUserClass) is valid
   //for the analysis task. This is so if one of the following is true:
   //  - the class library has already been loaded. In this case the class will exist
   //    in the dictionary (gROOT->GetClass()); we check if it derived from the
   //    base class defined for the analysis task
   //  - a plugin exists defining this class as an extension of the base class defined
   //    for the analysis task (gROOT->GetPluginManager()->FindHandler(...): the URI for
   //    the plugin must be the same as the name of the class)
   //  - source files for the class are present in the working directory. In this case
   //    we can add a plugin handler for the class.
   //In the latter two cases, the class is valid if compilation succeeds.
   //
   //If the user's class may in fact be derived from an alternative base class, rather
   //than the base class defined for this analysis task (see KVDataAnalysisTask::SetUserBaseClass)
   //you can supply the name of this class (or a comma-separated list of base classes).

   TObject* o = GetInstanceOfUserClass(alternative_base_class);
   if (o) {
      delete o;
      return kTRUE;
   }
   return kFALSE;
}

//__________________________________________________________________________________//

const Char_t* KVDataAnalyser::GetACliCMode()
{
   // Returns string to be appended to name of user class for compilation with ACliC in
   // GetInstanceOfUserClass. This depends on the boolean resources:
   //
   // KVDataAnalyser.UserClass.Debug:  ( "yes" => "g" )
   // KVDataAnalyser.UserClass.Optimise:   ( "yes" =>  "O" )
   // KVDataAnalyser.UserClass.ForceRecompile:  ( "no" => "+"; "yes" => "++" )
   //
   // Note that if both Debug and Optimise are set to "yes/true", we use Debug mode.
   // (can't have BOTH debug & optimisation).

   static TString aclic;
   if (gEnv->GetValue("KVDataAnalyser.UserClass.ForceRecompile", kFALSE)) aclic = "++";
   else aclic = "+";
   if (gEnv->GetValue("KVDataAnalyser.UserClass.Debug", kFALSE)) aclic += "g";
   else if (gEnv->GetValue("KVDataAnalyser.UserClass.Optimise", kFALSE)) aclic += "O";
   return aclic.Data();
}

void KVDataAnalyser::PostRunReset()
{
   fBatchSystem = nullptr;
}

//__________________________________________________________________________________//

TObject* KVDataAnalyser::GetInstanceOfUserClass(const KVString& alternative_base_class)
{
   //Return an instance of the class given by the user (fUserClass), if it is valid.
   //If the user class is given in the form of source code, it will be (re)compiled
   //if it has not already been loaded and/or the source has changed since the last
   //build, using ACliC. If the resource
   //
   //  KVDataAnalyser.UserClass.Debug:    yes
   //
   //is set, the user's class will be compiled with extra debugging information
   //
   //Once compiled, we check that the user's class is indeed derived from the base
   //class defined for this analysis task (see KVDataAnalysisTask::SetUserBaseClass).
   //If the user's class may in fact be derived from an alternative base class, you
   //can supply the name of this class (or comma-separated list of base classes).

   // make sure any required plugin library defining base class for user's analysis class is loaded
   if (!fTask->CheckUserBaseClassIsLoaded()) return 0x0;

   //do we have a plugin ?
   TPluginHandler* ph = gROOT->GetPluginManager()->FindHandler(fTask->GetUserBaseClass(), fUserClass.Data());
   if (!ph) {//no plugin defined

      //if it is a precompiled class (i.e. already part of KaliVeda),
      //it will be in the dictionary already
      TClass* cl = gROOT->GetClass(fUserClass.Data());

      //do we have source files ?
      if (DoUserClassFilesExist()) {
         //compile & load user's source files using ACLIC. ACliC options read by GetACliCMode() from .kvrootrc
         TString cmd;
         cmd.Form(".L %s%s", fUserClassImp.Data(), GetACliCMode());
         gROOT->ProcessLine(cmd.Data());
         //class will be in dictionary if compilation successful
         cl = gROOT->GetClass(fUserClass.Data());
      }
      else if (!cl) {
         //class not in dictionary and no source files. help!
         Info("GetInstanceOfUserClass", "Class %s is unknown and no source files available",
              fUserClass.Data());
         return 0;
      }
      if (!cl) {
         //compilation of user class has failed
         Info("GetInstanceOfUserClass", "Compilation of class %s failed. Correct the mistakes and try again",
              fUserClass.Data());
         return 0;
      }
      if (!cl->GetBaseClass(fTask->GetUserBaseClass())) {
         // class does not inherit from base class defined by analysis task
         if (alternative_base_class == "") { // no alternative base classes provided
            Info("GetInstanceOfUserClass", "Class %s does not inherit from correct base class (%s), or compilation of class %s failed. Correct the mistakes and try again",
                 fUserClass.Data(), fTask->GetUserBaseClass(), fUserClass.Data());
            return nullptr;
         }
         else {
            // check alternative base class(es) - it may still be good!
            bool got_good_base = false;
            TString good_base = "";
            alternative_base_class.Begin(",");
            while (!alternative_base_class.End()) {
               good_base = alternative_base_class.Next(kTRUE);
               if (cl->GetBaseClass(good_base)) {
                  got_good_base = true;
                  break;
               }
            }
            if (got_good_base) {
               Info("GetInstanceOfUserClass", "Class %s inherits from alternative base class %s: OK!",
                    fUserClass.Data(), good_base.Data());
            }
            else {
               Info("GetInstanceOfUserClass", "Class %s does not inherit from task-defined base class (%s) or any provided alternative base classes (%s), or compilation of class %s failed. Correct the mistakes and try again",
                    fUserClass.Data(), fTask->GetUserBaseClass(), alternative_base_class.Data(), fUserClass.Data());
               return nullptr;
            }
         }
      }
      //EVERYTHING OK!! now instanciate an object of the new class
      return (TObject*)cl->New();
   }
   else {
      Info("GetInstanceOfUserClass", "Found plugin handler for class %s",
           fUserClass.Data());
      //load class from plugin
      ph = KVBase::LoadPlugin(fTask->GetUserBaseClass(), fUserClass.Data());
      if (!ph) {
         Info("GetInstanceOfUserClass", "KVBase::LoadPlugin failed for %s", fUserClass.Data());
         return 0;
      }
      TObject* obj = (TObject*)ph->ExecPlugin(0);
      if (obj) {
         //Info("GetInstanceOfUserClass", "constructor OK for %s", fUserClass.Data());
         if (obj->InheritsFrom(fTask->GetUserBaseClass())) return obj;
         Info("GetInstanceOfUserClass", "%s does not inherit from %s", fUserClass.Data(), fTask->GetUserBaseClass());
         return 0;
      }
      else {
         Info("GetInstanceOfUserClass", "constructor not OK for %s", fUserClass.Data());
         return 0;
      }
   }
   return 0;
}

//__________________________________________________________________________________//

void KVDataAnalyser::SetUserClass(const Char_t* kvs, Bool_t check)
{
   //Set name of user analysis class.
   //If check=kTRUE (default), we check the validity of the class
   //if check=kFALSE we do not check and assume that the class is valid

   fUserClass = kvs;
   if (check) {
      fUserClassIsOK = CheckIfUserClassIsValid();
   }
   else {
      fUserClassIsOK = kTRUE;
   }
}

//__________________________________________________________________________________//

void KVDataAnalyser::WriteBatchEnvFile(const Char_t* jobname, Bool_t save)
{
   //Save (in the TEnv fBatchEnv) all necessary information on analysis task which can be used to execute it later
   //(i.e. when batch processing system executes the job).
   //If save=kTRUE (default), write the information in a file whose name is given by ".jobname"
   //where 'jobname' is the name of the job as given to the batch system.

   delete fBatchEnv;
   fBatchEnv = new TEnv(Form(".%s", jobname));
   if (fBatchSystem) {
      fBatchEnv->SetValue("BatchSystem", fBatchSystem->GetName());
      fBatchSystem->WriteBatchEnvFile(fBatchEnv);
   }
   fBatchEnv->SetValue("AnalysisTask", fTask->GetType());

   if (fTask->WithUserClass()) {
      fBatchEnv->SetValue("UserClass", GetUserClass());
      if (fUserClassImp == "" || fUserClassDec == "") {
         if (!DoUserClassFilesExist()) {
            Warning("WriteBatchEnvFile", "Source files for user class %s do not exist. Job will not work.",
                    GetUserClass());
         }
      }
      fBatchEnv->SetValue("UserClassOptions", fUserClassOptions);
      fBatchEnv->SetValue("UserClassImp", fUserClassImp);
      fBatchEnv->SetValue("UserClassDec", fUserClassDec);
   }
   else {
      // a task without a user class may still need to pass options to the predefined analysis class
      if (fUserClassOptions != "") fBatchEnv->SetValue("UserClassOptions", fUserClassOptions);
   }
   fBatchEnv->SetValue("NbToRead", (Double_t)nbEventToRead);
   fBatchEnv->SetValue("LaunchDirectory", gSystem->WorkingDirectory());
   if (fIncludes.Length()) {
      fBatchEnv->SetValue("UserIncludes", fIncludes.Data());
   }
   if (fLibraries.Length()) {
      fBatchEnv->SetValue("UserLibraries", fLibraries.Data());
   }

   if (save) fBatchEnv->SaveLevel(kEnvUser);
}

//_________________________________________________________________

Bool_t KVDataAnalyser::ReadBatchEnvFile(const Char_t* filename)
{
   //Read the batch env file "filename" and initialise the analysis task using the
   //informations in the file
   //Returns kTRUE if all goes well

   Bool_t ok = kFALSE;

   delete fBatchEnv;
   fBatchEnv = new TEnv(filename);
   KVString val = fBatchEnv->GetValue("AnalysisTask", "");
   fTask = 0;
   if (val != "") {
      if (!gDataSetManager) {
         gDataSetManager = new KVDataSetManager;
         gDataSetManager->Init();
      }
      SetAnalysisTask(gDataSetManager->GetAnalysisTaskAny(val.Data()));
   }
   else {
      Error("ReadBatchEnvFile", "Name of analysis task not given");
      return ok;
   }
   if (!fTask) {
      Error("ReadBatchEnvFile", "Analysis task \"%s\"not found for dataset %s",
            val.Data(), gDataSet->GetName());
      return ok;
   }

   nbEventToRead = (Long64_t)fBatchEnv->GetValue("NbToRead", -1);
   SetUserIncludes(fBatchEnv->GetValue("UserIncludes", ""));
   SetUserLibraries(fBatchEnv->GetValue("UserLibraries", ""));

   //batch system
   if (strcmp(fBatchEnv->GetValue("BatchSystem", ""), "")) {
      fBatchSystem = gBatchSystemManager->GetBatchSystem(fBatchEnv->GetValue("BatchSystem", ""));
      fBatchSystem->ReadBatchEnvFile(fBatchEnv);
      fBatchSystem->cd(); // make gBatchSystem point to it
      fBatchSystem->SetAnalyser(this);
   }

   //User files
   if (fTask->WithUserClass()) {
      fUserClass = fBatchEnv->GetValue("UserClass", "");
      if (fUserClass == "") {
         Error("ReadBatchEnvFile", "Name of user class not given");
         return ok;
      }
      fUserClassOptions = fBatchEnv->GetValue("UserClassOptions", "");
      fUserClassImp = fBatchEnv->GetValue("UserClassImp", "");
      if (fUserClassImp == "") {
         Error("ReadBatchEnvFile", "Name of user class implementation file not given");
         return ok;
      }
      fUserClassDec = fBatchEnv->GetValue("UserClassDec", "");
      if (fUserClass == "") {
         Error("ReadBatchEnvFile", "Name of user class header file not given");
         return ok;
      }
      fUserClassAlternativeBaseClass = fBatchEnv->GetValue("UserClassAlternativeBaseClass", "");

      //If current working directory is not the same as the launch directory,
      //we have to copy the user's files here
      if (!RunningInLaunchDirectory()) {
         TString launchDir = fBatchEnv->GetValue("LaunchDirectory", gSystem->WorkingDirectory());
         TString path_src, path_trg;
         //copy user's implementation file
         AssignAndDelete(path_src, gSystem->ConcatFileName(launchDir.Data(), fUserClassImp.Data()));
         AssignAndDelete(path_trg, gSystem->ConcatFileName(gSystem->WorkingDirectory(), fUserClassImp.Data()));
         gSystem->CopyFile(path_src.Data(), path_trg.Data());
         //copy user's header file
         AssignAndDelete(path_src, gSystem->ConcatFileName(launchDir.Data(), fUserClassDec.Data()));
         AssignAndDelete(path_trg, gSystem->ConcatFileName(gSystem->WorkingDirectory(), fUserClassDec.Data()));
         gSystem->CopyFile(path_src.Data(), path_trg.Data());
      }
   }
   else {
      // a task without a user class may still need to pass options to the predefined analysis class
      fUserClassOptions = fBatchEnv->GetValue("UserClassOptions", "");
   }

   ok = kTRUE;

   return ok;
}

//_________________________________________________________________

Bool_t KVDataAnalyser::RunningInLaunchDirectory()
{
   //Returns kTRUE if current working directory is same as launch directory for batch job
   //When not in batch mode, always returns kTRUE.
   if (!BatchMode() || !fBatchEnv) return kTRUE;
   TString launchDir = fBatchEnv->GetValue("LaunchDirectory", gSystem->WorkingDirectory());
   return (launchDir == gSystem->WorkingDirectory());
}

//_________________________________________________________________

void KVDataAnalyser::set_up_analyser_for_task(KVDataAnalyser* the_analyser)
{
   the_analyser->SetParent(this);
   the_analyser->SetAnalysisTask(fTask);
   the_analyser->SetNbEventToRead(GetNbEventToRead());
   the_analyser->SetUserIncludes(fIncludes.Data());
   the_analyser->SetUserLibraries(fLibraries.Data());
}

void KVDataAnalyser::SubmitTask()
{
   //In interactive mode, the data analysis task is performed by
   //instanciating and initialising the KVDataAnalyser child class specified by the task,
   //and then calling its Run() method.
   //In batch mode, the job is submitted to the chosen batch system.

   KVString task_data_analyser = fTask->GetDataAnalyser();
   Info("SubmitTask", "fTask->GetDataAnalyser()=%s", task_data_analyser.Data());
   unique_ptr<KVDataAnalyser> the_analyser;
   if (task_data_analyser == "UserClass") {
      //the user-provided class is to be used as analyser
      the_analyser.reset((KVDataAnalyser*)GetInstanceOfUserClass());
   }
   else {
      the_analyser.reset(GetAnalyser(fTask->GetDataAnalyser()));
   }
   if (!the_analyser.get())
      Fatal("SubmitTask", "the_analyser is 0x0, go to crash");

   set_up_analyser_for_task(the_analyser.get());

   if (fTask->WithUserClass()) {
      the_analyser->SetUserClass(GetUserClass(), kFALSE);
      the_analyser->SetUserClassOptions(fUserClassOptions);
   }
   else if (strcmp(fTask->GetUserBaseClass(), "")) the_analyser->SetUserClass(fTask->GetUserBaseClass(), kFALSE);
   if (!BatchMode()) {
      //when not in batch mode i.e. when submitting a task, we ask the user to supply
      //any further information required by the task, and then ask whether to run in
      //interactive or batch mode
      the_analyser->CheckTaskVariables();
      if (!fChoseRunMode) ChooseRunningMode();
   }
   the_analyser->SetBatchMode(BatchMode());
   the_analyser->SetBatchName(GetBatchName());
   the_analyser->SetBatchSystem(fBatchSystem);
   the_analyser->SetProofMode(GetProofMode());
   //set global pointer to analyser object which performs the analysis
   //this allows e.g. user class to obtain information on the analysis task
   gDataAnalyser = the_analyser.get();
   the_analyser->Run();
}

//__________________________________________________________________________________//

TString KVDataAnalyser::ExpandAutoBatchName(const Char_t* format) const
{
   //Replace any 'special' symbols in "format" with their current values
   //
   //  $Date   : current date and time
   //  $User  :  name of user
   //  $UserClass  :  name of user's analysis class

   KVString tmp = format;
   TDatime now;
   KVString stDate = now.AsSQLString();
   stDate.ReplaceAll(" ", "-");
   tmp.ReplaceAll("$Date", stDate.Data());
   if (fUserClass.Length()) tmp.ReplaceAll("$UserClass", fUserClass.Data());
   else if (fTask) tmp.ReplaceAll("$UserClass", fTask->GetDataAnalyser());
   tmp.ReplaceAll("$User", gSystem->GetUserInfo()->fUser.Data());
   return tmp;
}

const Char_t* KVDataAnalyser::GetRecognisedAutoBatchNameKeywords() const
{
   static TString keywords = "Will be expanded: $Date, $User, $UserClass";
   return keywords;
}


//__________________________________________________________________________________//


const Char_t* KVDataAnalyser::GetLaunchDirectory() const
{
   // Returns full path to job submission directory for batch jobs.
   // Returns current working directory for non-batch jobs.

   if (!BatchMode() || !fBatchEnv) return gSystem->WorkingDirectory();
   return fBatchEnv->GetValue("LaunchDirectory", gSystem->WorkingDirectory());
}

const Char_t* KVDataAnalyser::GetBatchStatusFileName() const
{
   // Returns full path to file used to store status of running batch jobs

   if (!BatchMode() || !fBatchEnv) return "";

   static TString filename = "";
   TString statfile;
   statfile.Form("%s.status", gBatchSystem->GetJobName());

   TString launchDir = GetLaunchDirectory();
   AssignAndDelete(filename, gSystem->ConcatFileName(launchDir.Data(), statfile.Data()));
   return filename;
}

void KVDataAnalyser::UpdateBatchStatusFile(Int_t totev, Int_t evread, TString disk) const
{
   // Update infos in batch status file

   if (!BatchMode() || !fBatchEnv) return;

   TEnv stats(GetBatchStatusFileName());
   stats.SetValue("TotalEvents", totev);
   stats.SetValue("EventsRead", evread);
   disk.Remove(TString::kTrailing, '\t');
   disk.Remove(TString::kTrailing, ' ');
   disk.Remove(TString::kTrailing, '\t');
   stats.SetValue("DiskUsed", disk.Data());
   stats.SaveLevel(kEnvLocal);
}

void KVDataAnalyser::DeleteBatchStatusFile() const
{
   // Delete batch status file (and backup - '.bak') for batch job

   if (!BatchMode() || !fBatchEnv) return;
   TString stats = GetBatchStatusFileName();
   gSystem->Unlink(stats);
   stats += ".bak";
   gSystem->Unlink(stats);
}

Bool_t KVDataAnalyser::CheckStatusUpdateInterval(Long64_t nevents) const
{
   // Returns kTRUE if the number of events coincides with the interval
   // set for status updates for the current data analysis task
   return (!(nevents % fTask->GetStatusUpdateInterval()) && nevents);
}

void KVDataAnalyser::DoStatusUpdate(Long64_t nevents) const
{
   // Print infos on events treated, disk usage, memory usage

   cout << " +++ " << nevents << " events processed +++ " << endl;
   ProcInfo_t pid;
   if (gSystem->GetProcInfo(&pid) == 0) {
      cout << "     ------------- Process infos -------------" << endl;
      printf(" CpuSys = %f  s.    CpuUser = %f s.    ResMem = %f MB     VirtMem = %f MB\n",
             pid.fCpuSys,  pid.fCpuUser, pid.fMemResident / 1024.,
             pid.fMemVirtual / 1024.);
   }
}

//__________________________________________________________________________________//

void KVDataAnalyser::ChooseRunningMode()
{
   //Ask user to choose between immediate or batch execution
   //If the choice is batch, we ask to choose a batch system and whether or not
   //to use the "multijobs" mode

   fChoseRunMode = kTRUE;
   KVString tmp;
   do {
      cout << endl << "Run in Interactive or Batch mode (I or B) ? : ";
      tmp.ReadLine(cin);
   }
   while (tmp != "i" && tmp != "I" && tmp != "b" && tmp != "B");
   tmp.ToUpper();
   //interactive mode - no more to do
   if (tmp == "I") {
      fBatchSystem = 0;
      return;
   }
   cout << endl << "Choose the batch system to use : " << endl;
   gBatchSystemManager->Print();
   do {
      cout << "(enter a number) : " << endl;
      tmp.ReadLine(cin);
   }
   while (!tmp.IsDigit());
   fBatchSystem = gBatchSystemManager->GetBatchSystem(tmp.Atoi());
   fBatchSystem->Clear();
}

//__________________________________________________________________________________//

void KVDataAnalyser::ScanWorkingDirectory(TList** ls)
{
   //Fill TList with list of files in current working directory.
   //If ls!=0 it is deleted beforehand
   if (*ls) delete (*ls);
   TSystemDirectory dir("LocDir", gSystem->WorkingDirectory());
   (*ls) = dir.GetListOfFiles();
}

//__________________________________________________________________________________//

void KVDataAnalyser::CopyAnalysisResultsToLaunchDirectory()
{
   //Compare the two lists of files in the current working directory, before and after analysis;
   //and copy any files which were created during the analysis to the launch directory.
   //Files with the same names in the launch directory will be overwritten if they exist.

   if (!fWorkDirInit || !fWorkDirEnd) return;
   TString launchDir = fBatchEnv->GetValue("LaunchDirectory", gSystem->WorkingDirectory());
   TIter next_new_file(fWorkDirEnd);
   TObject* file;
   while ((file = next_new_file())) {
      if (!fWorkDirInit->FindObject(file->GetName())) {
         TString fname;
         fname.Form("%s", file->GetName());
         //ajout d une condition pour eviter le transfert des file*.so generes par les KVParticleCondition
         //et aussi les .d generes par les KVParticleCondition
         if (!(fname.BeginsWith("KVParticleCondition_") || fname.EndsWith(".so") || fname.EndsWith(".d") || fname.EndsWith(".pcm") || fname.EndsWith(".bak"))) {
            TString path_src, path_trg;
            AssignAndDelete(path_trg, gSystem->ConcatFileName(launchDir.Data(), file->GetName()));
            AssignAndDelete(path_src, gSystem->ConcatFileName(gSystem->WorkingDirectory(),
                            file->GetName()));
            Info("CopyAnalysisResultsToLaunchDirectory", "Copying analysis results file :\n%s ---> %s",
                 path_src.Data(), path_trg.Data());
            //copy & overwrite any existing file in launch directory
            if (gSystem->CopyFile(path_src.Data(), path_trg.Data(), kTRUE) == 0) {
               Info("CopyAnalysisResultsToLaunchDirectory", "File copied correctly");
            }
            else {
               Info("CopyAnalysisResultsToLaunchDirectory", " **** ERROR copying file !!! ");
            }
         }
      }
   }
}

//_________________________________________________________________

void KVDataAnalyser::WriteBatchInfo(TTree* tt)
{
   // Store lots of useful information about the current version of KaliVeda,
   // ROOT, etc. etc. in a TEnv object which will be added to the TTree's
   // list of user infos (TTree::GetUserInfo).

   tt->GetUserInfo()->Add(new TEnv());
   TEnv* kvenv = (TEnv*)tt->GetUserInfo()->FindObject("TEnv");

//----
   THashList* hh = gEnv->GetTable();
   KVString tamp;
   for (Int_t kk = 0; kk < hh->GetEntries(); kk += 1) {
      tamp.Form("%s", hh->At(kk)->GetName());
      if (tamp.BeginsWith("Plugin.")) {}
      else kvenv->SetValue(hh->At(kk)->GetName(), ((TEnvRec*)hh->At(kk))->GetValue(), kEnvUser);
   }

   kvenv->SetValue("KVBase::GetKVVersion()", KVBase::GetKVVersion(), kEnvUser);
   kvenv->SetValue("KVBase::GetKVBuildDate()", KVBase::GetKVBuildDate(), kEnvUser);
   kvenv->SetValue("KVBase::GetKVBuildUser()", KVBase::GetKVBuildUser(), kEnvUser);
   kvenv->SetValue("KVBase::GetKVSourceDir()", KVBase::GetKVSourceDir(), kEnvUser);

#ifdef WITH_BZR_INFOS
   kvenv->SetValue("KVBase::bzrRevisionId()", KVBase::bzrRevisionId(), kEnvUser);
   kvenv->SetValue("KVBase::bzrRevisionDate()", KVBase::bzrRevisionDate(), kEnvUser);
   kvenv->SetValue("KVBase::bzrBranchNick()", KVBase::bzrBranchNick(), kEnvUser);
   kvenv->SetValue("KVBase::bzrRevisionNumber()", KVBase::bzrRevisionNumber());
   kvenv->SetValue("KVBase::bzrIsBranchClean()", KVBase::bzrIsBranchClean());
#endif
#ifdef WITH_GIT_INFOS
   kvenv->SetValue("KVBase::gitBranch()", KVBase::gitBranch(), kEnvUser);
   kvenv->SetValue("KVBase::gitCommit()", KVBase::gitCommit(), kEnvUser);
#endif

   kvenv->SetValue("gROOT->GetVersion()", gROOT->GetVersion(), kEnvUser);

   kvenv->SetValue("gSystem->GetBuildArch()", gSystem->GetBuildArch(), kEnvUser);
   kvenv->SetValue("gSystem->GetBuildCompiler()", gSystem->GetBuildCompiler(), kEnvUser);
   kvenv->SetValue("gSystem->GetBuildCompilerVersion()", gSystem->GetBuildCompilerVersion(), kEnvUser);
   kvenv->SetValue("gSystem->GetBuildNode()", gSystem->GetBuildNode(), kEnvUser);
   kvenv->SetValue("gSystem->GetBuildDir()", gSystem->GetBuildDir(), kEnvUser);

   kvenv->SetValue("gSystem->GetUserInfo()->fUser", gSystem->GetUserInfo()->fUser, kEnvUser);
   kvenv->SetValue("gSystem->HostName()", gSystem->HostName(), kEnvUser);

   if (fBatchEnv) {
      THashList* hh = fBatchEnv->GetTable();
      for (Int_t kk = 0; kk < hh->GetEntries(); kk += 1) {
         tamp.Form("%s", hh->At(kk)->GetName());
         if (!strcmp(kvenv->GetValue(hh->At(kk)->GetName(), "rien"), "rien"))
            kvenv->SetValue(hh->At(kk)->GetName(), ((TEnvRec*)hh->At(kk))->GetValue(), kEnvUser);
      }
   }


}

void KVDataAnalyser::RunAnalyser(const Char_t* uri)
{
   //Set up and run data analysis task.
   //This allows to choose a dataset and a data analysis task and then execute the task or submit a batch job.
   //The behaviour of the data analyser object (base class KVDataAnalyser) can be modified by choosing
   //a plugin class corresponding to one of the plugins defined in $KVROOT/KVFiles/.kvrootrc.

   KVDataAnalyser* datan = 0;
   TString tmp(uri);
   if (tmp != "") {
      //got plugin ?
      TPluginHandler* ph = KVBase::LoadPlugin("KVDataAnalyser", uri);
      if (!ph)
         ::Warning("KVDataAnalyser::RunAnalyser", "No plugin %s found for KVDataAnalyser",
                   uri);
      else
         datan = (KVDataAnalyser*) ph->ExecPlugin(0);
   }
   if (datan == 0)
      datan = new KVDataAnalyser;
   datan->RunMenus();
   delete datan;
}

Bool_t KVDataAnalyser::IsRunningBatchAnalysis()
{
   // Static method KVDataAnalyser::IsRunningBatchAnalysis()
   // Returns kTRUE if an analysis task is being performed in batch mode
   // Returns kFALSE if no analysis task is in interactive mode, or no analysis task running

   if (gDataAnalyser) return (gDataAnalyser->BatchMode() && gDataAnalyser->fBatchSystem);
   return kFALSE;
}

void KVDataAnalyser::AddJobDescriptionList(TList* l)
{
   // Create a KVNameValueList called "JobDescriptionList" and add it to
   // the TList. The parameters in the list describe the properties of the
   // current job. The TList pointer could be, for example, the address of
   // the TSelector::fInput list used by PROOF.

   KVNameValueList* jdl = new KVNameValueList("JobDescriptionList", "Job parameters");

   jdl->SetValue("AnalysisTask", fTask->GetType());
   jdl->SetValue("PROOFMode", GetProofMode());

   l->Add(jdl);
}

void KVDataAnalyser::ChooseWhatToAnalyse()
{
   // TO IMPLEMENT ?
}

void KVDataAnalyser::ChooseWhatToDo()
{
   // TO IMPLEMENT ?
}
