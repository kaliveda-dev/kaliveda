#include "THtml.h"
#include "TSystem.h"
#include "KVBase.h"
#include "KVString.h"
#include "KVPathDef.h"
#include "KVFileDef.h"

void makedoc(const TString& output_dir = "htmldoc")
{
   // load all libraries and generate class reference
   THtml h;
   h.SetProductName(Form("KaliVeda v%s", KVBase::GetKVVersion()));
   TString source_root = KVBase::GetKVSourceDir();
   h.SetInputDir(source_root);
   TString version = KVBase::GetKVVersion();
   TString maj_vers = version;
   maj_vers.Remove(maj_vers.Index("/"));
   version.ReplaceAll("/", ".");
   h.SetOutputDir(output_dir);
   h.SetViewCVS(Form("https://github.com/kaliveda-dev/kaliveda/blob/%s/", maj_vers.Data()));
   h.SetPathDefinition(KVPathDef());
   h.SetFileDefinition(KVFileDef());
   h.SetAuthorTag("//Author:");
   h.SetCopyrightTag("//Created");
   h.SetHomepage("http://indra.in2p3.fr/KaliVedaDoc");

   // "hide" the CMakeCache.txt & install_manifest.txt files
   // or links will be generated for them under "Further reading..."
   TString path = TString(KVBase::GetKVBuildDir()) + "/";
   TString f1 = path + "CMakeCache.txt";
   TString f2 = path + "install_manifest.txt";
   TString f11 = f1 + ".bck";
   TString f22 = f2 + ".bck";
   gSystem->Rename(f1, f11);
   gSystem->Rename(f2, f22);

   TString htmloutdir;
   if (gSystem->IsAbsoluteFileName(output_dir)) htmloutdir = output_dir;
   else htmloutdir = path + output_dir;
   TString our_head = path + "header.html";
   TString our_head_out = htmloutdir + "/header.html";
   gSystem->CopyFile(our_head, our_head_out);
   gSystem->Unlink(our_head);
   h.SetHeader(our_head_out);
   TString our_foot = path + "footer.html";
   TString our_foot_out = htmloutdir + "/footer.html";
   gSystem->CopyFile(our_foot, our_foot_out);
   gSystem->Unlink(our_foot);
   h.SetFooter(our_foot_out);

   h.MakeAll(kTRUE);

   gSystem->Rename(f11, f1);
   gSystem->Rename(f22, f2);

   TString our_css = path + "ROOT.css";
   TString our_css_out = htmloutdir + "/ROOT.css";
   gSystem->CopyFile(our_css, our_css_out);
   TString our_banner = path + "htmldoc_banner.png";
   TString our_banner_out = htmloutdir + "/htmldoc_banner.png";
   gSystem->CopyFile(our_banner, our_banner_out);
}
