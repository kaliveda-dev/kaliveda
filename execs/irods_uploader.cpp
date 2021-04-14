// upload files to irods server at cc.in2p3 and (optionally)
// delete them after successful transfer
//
// Usage:
//
//  irods_uploader [-D][-A] [directory to scan] [dataset]
//
//    options: -D  - if given, delete each file which has been successfully transferred
//             -A  - if given, treat last file found (only use when data taking is finished)
//
//    for each run R, there can be a series of files:
//        run_R.dat.[date/time]
//        run_R.dat.[date/time].1
//        run_R.dat.[date/time].2
//        run_R.dat.[date/time].3
//    etc.
//
//    - a file run_R.*.I is only transferrable if file run_R.*.I+1 or run_R+1.* exists
//    in the directory, this means that writing file run_R.*.I has finished.
//    - if file run_R.*.I is found on the irods server AND has the same file size as the local file,
//    then the local file can be deleted (if option -D is given)

#include "KVString.h"
#include <IRODS.h>
#include <KVDataRepositoryManager.h>
#include <KVDataSetManager.h>
#include <KVDataRepository.h>
#include <KVSystemDirectory.h>
#include <KVSystemFile.h>
#include <iostream>
#include <map>
#include <KVMFMDataFileReader.h>
#include "KVAvailableRunsFile.h"
using namespace std;

struct runfile_t {
   KVString name;
   KVString date;
   Long64_t size;
};

struct run_t {
   map<int, runfile_t> files;
};

class file_helper {
public:
   KVString scan_dir, dataset;
   KVString file_format;
   KVString working_directory;
   int index_multiplier;

   bool file_has_index_number(const KVString& name, int& index)
   {
      // return true if filename has an index number
      index = 0;
      if (name.GetNValues(".") == 3) return false;
      name.Begin(".");
      for (int i = 0; i < 3; i++) name.Next();
      index = name.Next().Atoi();
      return true;
   }

   bool find_next_sequential_file(int& run0, int& index0, runfile_t& runfile0, int& run, int& index, runfile_t& runfile, bool first_call = false)
   {
      // look for next file after (run0,index0) in directory
      // return false if no file found
      // if run0!=0 when function first called, set first_call=true

      KVSystemDirectory dir("data", scan_dir);
      TList* files = dir.GetListOfFiles();

      // sort files into ordered map run[index]=filename
      map<int, run_t> runs;
      TIter nxt(files);
      KVSystemFile* f;
      while ((f = (KVSystemFile*)nxt())) {
         if (!f->IsDirectory()) {
            int ir;
            if ((ir = KVAvailableRunsFile::IsRunFileName(file_format, f->GetName(), index_multiplier))) {
               int idx = ir % index_multiplier;
               ir /= index_multiplier;
               runfile_t rf;
               rf.name = f->GetName();
               rf.size = f->GetSize();
               rf.date = f->GetDate();
               runs[ir].files[idx] = rf;
            }
         }
      }

      if (!runs.size()) return false; // there are no run files in the directory

      if (!run0) {
         // use first file found in directory as current run
         map<int, run_t>::iterator first_run = runs.begin();
         run0 = first_run->first;
         map<int, runfile_t>::iterator first_index = first_run->second.files.begin();
         index0 = first_index->first;
      }
      else if (first_call) {
         // starting from a run which is not the first one in the directory
         // find first file corresponding to run
         map<int, run_t>::iterator first_run = runs.find(run0);
         if (first_run == runs.end()) return false;
         // find first file corresponding to run
         map<int, runfile_t>::iterator first_index = first_run->second.files.begin();
         index0 = first_index->first;
      }
      // update infos on current file (whose size may have changed since it was first seen)
      // unless this method was called with run0=0
      // the current file was previously found with this method therefore it exists
      runfile0 = runs[run0].files[index0];

      run = run0;
      index = index0 + 1;

      // now look for (run,index)
      // if not found, look for (run+1,0)
      map<int, run_t>::iterator find_run = runs.find(run);
      if (find_run != runs.end()) {
         map<int, runfile_t>::iterator find_index = find_run->second.files.find(index);
         if (find_index != find_run->second.files.end()) {
            runfile = find_index->second;
            return true;
         }
      }
      // look for (run+1,0)
      ++run;
      index = 0;
      find_run = runs.find(run);
      if (find_run != runs.end()) {
         map<int, runfile_t>::iterator find_index = find_run->second.files.find(index);
         if (find_index != find_run->second.files.end()) {
            runfile = find_index->second;
            return true;
         }
      }
      // look for (run+2,0)
      ++run;
      index = 0;
      find_run = runs.find(run);
      if (find_run != runs.end()) {
         map<int, runfile_t>::iterator find_index = find_run->second.files.find(index);
         if (find_index != find_run->second.files.end()) {
            runfile = find_index->second;
            return true;
         }
      }
      return false;
   }
   void read_and_store_infos_on_file(const runfile_t& runfile)
   {
      KVString path_to_file;
      path_to_file.Form("%s/%s", scan_dir.Data(), runfile.name.Data());
      int run_num = KVAvailableRunsFile::IsRunFileName(file_format, runfile.name, index_multiplier);
      KVDatime when;
      KVAvailableRunsFile::ExtractDateFromFileName(file_format, runfile.name, when);
      KVNameValueList infos;
      infos.SetName(Form("run%010d", run_num));
      infos.SetValue("Run", run_num);
      infos.SetValue("Start", when.AsSQLString());
      infos.SetValue("End", runfile.date);
      infos.SetValue("Size", (int)runfile.size);

      KVMFMDataFileReader reader(path_to_file);
      ULong64_t events = 0;
      // correct start date/time from infos in file ?
      if (reader.GetRunInfos().HasStringParameter("FileCreationTime")) {
         when.SetGanacqNarvalDate(reader.GetRunInfos().GetStringValue("FileCreationTime"));
         infos.SetValue("Start", when.AsSQLString());
      }
      while (reader.GetNextEvent()) {
         cout << "\xd" << "Reading events " << ++events << flush;
      }
      cout << endl;
      infos.SetValue64bit("Events", events);
      infos.Print();

      TFile f(Form("%s/runinfos.root", working_directory.Data()), "update");
      infos.Write();
   }
};

int main(int argc, char* argv[])
{
   if (argc < 3 || argc > 7) {
      cout << "Usage: irods_uploader [-I][-D][-A][-r firstrun] [directory to scan] [dataset]" << endl << endl;
      cout << "  options: -I  - do not upload, only extract run infos (fill runinfos.root)" << endl;
      cout << "           -D  - if given, delete each file which has been successfully transferred" << endl;
      cout << "           -A  - if given, treat last file found (only use when data taking is finished)" << endl;
      cout << "           -r firstrun  - if given, start from given run number" << endl;
      exit(1);
   }

   file_helper FILE_H;
   FILE_H.working_directory = gSystem->WorkingDirectory();

   bool delete_files(false), all_files(false), do_upload(true);
   int firstrun = 0;
   int iarg = 1;
   if (argc > 3) {
      // look for optional arguments
      while (iarg < argc - 2) {
         if (!strcmp(argv[iarg], "-I")) do_upload = false;
         else if (!strcmp(argv[iarg], "-D")) delete_files = true;
         else if (!strcmp(argv[iarg], "-A")) all_files = true;
         else if (!strcmp(argv[iarg], "-r")) {
            ++iarg;
            firstrun = TString(argv[iarg]).Atoi();
         }
         ++iarg;
      }
   }
   if (!do_upload) {
      // not uploading, just extracting infos
      // by default: DO NOT DELETE FILES, TREAT ALL FILES
      delete_files = false;
      all_files = true;
   }
   FILE_H.scan_dir = argv[iarg];
   FILE_H.dataset = argv[iarg + 1];

   cout << "Directory to scan: " << FILE_H.scan_dir << "  dataset: " << FILE_H.dataset << endl;
   if (do_upload) cout << "Files will be uploaded to IRODS server at cca.in2p3.fr" << endl;
   cout << "Informations extracted from files will be written in runinfos.root" << endl;
   if (delete_files) cout << "Any transferred files will be deleted from local disk" << endl;
   if (all_files) cout << "All files including the last one will be treated (data-taking is finished)" << endl;
   if (firstrun) cout << "Starting scan from run " << firstrun << endl;

   KVDataRepositoryManager drm;
   drm.Init();
   if (do_upload) drm.GetRepository("ccali")->cd();
   KVDataSet* dataset = gDataSetManager->GetDataSet(FILE_H.dataset);
   FILE_H.file_format = dataset->GetDataSetEnv("DataSet.RunFileName.raw", "");
   FILE_H.index_multiplier = dataset->GetDataSetEnv("DataSet.RunFileIndexMultiplier.raw", -1.);

   int sleeptime(10), totalsleep(0);
   int current_run(firstrun), current_index(0);
   runfile_t current_file;
   int next_run(0), next_index(0);
   runfile_t next_file;
   bool got_next_file;
   bool first_call = true;
   while (1) {
      while ((got_next_file = FILE_H.find_next_sequential_file(current_run, current_index, current_file, next_run, next_index, next_file, first_call))
             || all_files) {
         totalsleep = 0;

         cout << got_next_file << " " << current_file.name << " " << current_run << " " << current_index << endl;

         if (do_upload) {
            // check if current run has been uploaded
            FileStat_t fs;
            if (gDataRepository->GetFileInfo(dataset, "raw", current_file.name, fs)) {
               // check size of uploaded file
               if (fs.fSize == current_file.size) {
                  cout << "File " << current_file.name << " has been uploaded successfully & ";
                  if (delete_files) {
                     cout << "will be deleted" << endl;
                     KVString path;
                     path.Form("%s/%s", FILE_H.scan_dir.Data(), current_file.name.Data());
                     gSystem->Unlink(path);
                     cout << "File deleted: " << path << endl;
                  }
                  else
                     cout << "can be deleted" << endl;
               }
               else {
                  cout << "File " << current_file.name << " partially uploaded, size IRODS=" << fs.fSize << " size local disk=" << current_file.size << endl;
               }
            }
            else {
               cout << "File " << current_file.name << " ready for upload [next file: " << next_file.name << "]" << endl;
               FILE_H.read_and_store_infos_on_file(current_file);
               if (gDataRepository->CopyFileToRepository(
                        Form("%s/%s", FILE_H.scan_dir.Data(), current_file.name.Data()),
                        dataset, "raw", current_file.name.Data()
                     ) == 0) {
                  // successful transfer
                  cout << "File upload successful" << endl;
                  continue;
               }
               else {
                  cout << "            *************** ERROR uploading file *************** " << endl;
               }
            }
         }
         else {
            // no upload - just read & store run infos
            FILE_H.read_and_store_infos_on_file(current_file);
         }
         if (all_files && !got_next_file) { //last file has been treated
            exit(0);
         }
         current_run = next_run;
         current_index = next_index;
         current_file = next_file;
         first_call = false;
      }
      first_call = false;
      if (do_upload) {
         cout << "Waiting " << totalsleep << " sec. for next file after " << current_file.name << " ... " << endl;
         sleep(sleeptime);
         totalsleep += sleeptime;
      }
   }
}
