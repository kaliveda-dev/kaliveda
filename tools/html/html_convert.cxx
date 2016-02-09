#include <unistd.h>
#include <cstdlib>
#include <string>
#include <iostream>
#include <fstream>

#include "THtml.h"
#include "TClass.h"
#include "TInterpreter.h"
#include "TSystem.h"
#include "TROOT.h"
#include "KVBase.h"
using namespace std;

int main(int argc, char* argv[])
{
   // html_convert [-c] [-i example index file] [-o output] [-t title/classname] input
   //  options:
   //     -c          convert a class, 'input' is class source (.cpp/.cxx/.C file)
   //                 give name of class to '-t' option
   //     -i          give path to examples index
   //     -o output   output directory, default is htmldoc
   //     -t title    title for converted function. if -c is given, name of class.

   int opt;
   string input, output, title, index_file;
   bool is_class = false;

   while ((opt = getopt(argc, argv, "co:t:i:")) != -1) {
      switch (opt) {
         case 'c':
            is_class = true;
            break;
         case 'o':
            output = optarg;
            break;
         case 't':
            title = optarg;
            break;
         case 'i':
            index_file = optarg;
            break;
         default: /* '?' */
            cerr << "Usage: " << argv[0] << " [-c] [-i example index file] [-o output] [-t title] input" << endl;
            exit(EXIT_FAILURE);
      }
   }

   if (optind >= argc) {
      cerr << "Expected argument after options" << endl;
      exit(EXIT_FAILURE);
   }

   input = argv[optind];

   KVBase::InitEnvironment();

   THtml html;
   html.LoadAllLibs();
   html.SetProductName(Form("KaliVeda v%s", KVBase::GetKVVersion()));
   TString source_root = KVBase::GetKVSourceDir();
   html.SetInputDir(source_root);
   if (output != "") html.SetOutputDir(output.c_str());

   gSystem->SetBuildDir(KVBase::GetKVBuildDir(), kTRUE);

   string aclic = input + "+";
   if (is_class) {
      gROOT->LoadMacro(aclic.c_str());
      string class_desc = gInterpreter->ClassInfo_Title(TClass::GetClass(title.c_str())->GetClassInfo());
      // update example index file
      ofstream index;
      index.open(index_file.c_str(), ios_base::app);
      index << "<li><a href=\"" << title << ".html\">" << title << "</a> - " << class_desc << "</li>\n";
      index.close();
      html.MakeClass(title.c_str());
   } else {
      gROOT->LoadMacro(aclic.c_str());
      // update example index file
      ofstream index;
      index.open(index_file.c_str(), ios_base::app);
      index << "<li><a href=\"examples/" << gSystem->BaseName(input.c_str()) << ".html\">" << gSystem->BaseName(input.c_str()) << "</a> - " << title << "</li>\n";
      index.close();
      html.Convert(input.c_str(), title.c_str());
   }

   exit(EXIT_SUCCESS);
}
