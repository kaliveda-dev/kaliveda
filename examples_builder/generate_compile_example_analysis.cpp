#include <KVDataSet.h>
#include <KVDataSetManager.h>
#include <KVSimDirFilterAnalyser.h>
#include <unordered_map>
/*
   Generate and compile analysis classes for a given dataset & analysis task:

   Usage: generate_compile_example_analysis [dataset] [task keywords] [classname]

      [module] :        name of module in sources
      [dataset] :       name of dataset
      [task keywords] : keywords contained in title of analysis task
      [classname] :     name of generated class

      Returns 0 if class compiles, 1 if not
*/

int main(int argc, char* argv[])
{
   if (argc < 5) {
      std::cout << "Generate and compile analysis classes for a given dataset & analysis task:\n" << std::endl;
      std::cout << "Usage: generate_compile_example_analysis [dataset] [task keywords] [classname]\n" << std::endl;
      std::cout << "   [module] :        name of module in sources" << std::endl;
      std::cout << "   [dataset] :       name of dataset" << std::endl;
      std::cout << "   [task keywords] : type of analysis task (RawAnal;RawReconAnal;ReconAnal;SimAnal;FiltAnal)" << std::endl;
      std::cout << "   [classname] :     name of generated class" << std::endl;
      exit(1);
   }

   TString module = argv[1];
   TString dataset = argv[2];
   TString tasktype = argv[3];
   TString classname = argv[4];

   KVBase::InitEnvironment();

   if (tasktype == "SimAnal") {
      // generate simulated data analysis class
      KVSimDirAnalyser::Make(classname);
   }
   else if (tasktype == "FiltAnal") {
      // generate filtered simulated data analysis class
      KVSimDirFilterAnalyser::Make(classname);
   }
   else {
      std::unordered_map<std::string, TString> tasks {
         {"RawAnal", "Analysis of raw data"},
         {"RawReconAnal", "Analysis of reconstructed raw data"},
         {"ReconAnal", "Analysis of reconstructed events"}
      };
      // experimental data analysis
      KVDataSetManager drm;
      drm.Init();

      gDataSetManager->GetDataSet(dataset)->cd();
      // generate class
      gDataSet->MakeAnalysisClass(tasks[tasktype.Data()], classname);
   }
   // compile class
   int okCompiler = gSystem->CompileMacro(classname + ".cpp");

   if (okCompiler) {
      // clean up, then move to source directory
      TString src_dir = KVBase::GetKVSourceDir();
      src_dir += "/";
      src_dir += module;
      src_dir += "/examples/";
      gSystem->mkdir(src_dir);
      TString hdr_file = classname + ".h";
      TString src_file = classname + ".cpp";
      gSystem->CopyFile(hdr_file, src_dir + hdr_file, kTRUE);
      gSystem->CopyFile(src_file, src_dir + src_file, kTRUE);
      gSystem->Unlink(hdr_file);
      gSystem->Unlink(src_file);
      gSystem->Unlink(classname + "_cpp.d");
      gSystem->Unlink(classname + "_cpp_ACLiC_dict_rdict.pcm");
   }
   return !okCompiler;
}
