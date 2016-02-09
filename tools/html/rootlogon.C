{
   gROOT->ProcessLine("TGeoHMatrix p;");//kludge to get round autoload problem
   //with libGeom.so
   gROOT->ProcessLine("#include \"RVersion.h\"");
   KVString libs = gSystem->GetFromPipe("kaliveda-config --libs");
   libs.Begin(" ");
   while (! libs.End())
   {
      TString lib = libs.Next();
      lib.ReplaceAll("-l", "lib");
#if ROOT_VERSION_CODE < ROOT_VERSION(6,0,0)
      gSystem->Load(lib);
#else
      const char* path = gSystem->FindDynamicLibrary(lib, kTRUE);
      if (path) gSystem->Load(lib);
#endif
   }
   KVBase::InitEnvironment();
   gROOT->ProcessLine(".L KVPathDef.cpp+");
   gROOT->ProcessLine(".L KVFileDef.cpp+");
   gROOT->ProcessLine(".L makedoc.C+");
}
