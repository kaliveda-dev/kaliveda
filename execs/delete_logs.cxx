#include "KVNumberList.h"
#include "Riostream.h"
#include "TSystem.h"
#include "KVString.h"
#include "TSystemDirectory.h"
#include "TSystemFile.h"
#include "TList.h"

using namespace std;

int main(int argc, char** argv)
{
   if (argc < 2) {
      cout << "\t\tDelete (CCIN2P3) batch log files" << endl << endl;
      cout << "\tdelete_logs [jobname] [list of runs]" << endl << endl;
      cout << "\targuments are:" << endl;
      cout << "\t\t[jobname] common root of name of all jobs to analyse" << endl;
      cout << "\t\t      e.g. PbAu29MeV for jobs with names like PbAu29MeV_R8213 etc." << endl;
      cout << "\t\t[list of runs]" << endl;
      cout << "\t\t      e.g. \"1-10 13 26-54\" (don't forget the \"\" !)" << endl;
      cout << "\t\t      The programme will delete all files with names like" << endl;
      cout << "\t\t          PbAu29MeV_R1.o5647093" << endl;
      cout << "\t\t          PbAu29MeV_R2.o5647094" << endl;
      cout << "\t\t      etc. in the current directory" << endl;
      return 0;
   }

   // take root of job name and add suffix "_R"
   KVString nameForm(argv[1]);
   nameForm += "_R";
   KVString nameFormLR = nameForm + "%d"; //format for KVLogReader

   KVNumberList runs(argv[2]);
   runs.Begin();
   KVString filename;
   TList list_of_files;
   list_of_files.SetOwner();
   while (!runs.End()) {
      filename.Form(nameFormLR.Data(), runs.Next());
      list_of_files.Add(new TNamed(filename.Data(), "to be deleted"));
   }
   //loop over files in current directory
   TSystemDirectory thisDir(".", ".");
   TList* fileList = thisDir.GetListOfFiles();
   TIter nextFile(fileList);
   TSystemFile* aFile = 0;
   while ((aFile = (TSystemFile*)nextFile())) {
      KVString afilename = aFile->GetName();
      afilename.Begin(".");
      TString base, suffix;
      if (!afilename.End()) base = afilename.Next();
      if (!afilename.End()) suffix = afilename.Next();
      if (base != "" && suffix.BeginsWith("o")) {
         if (list_of_files.FindObject(base.Data())) {
            cout << "Deleting file " << afilename << endl;
            gSystem->Unlink(afilename.Data());
         }
      }
   }
   return 0;
}
