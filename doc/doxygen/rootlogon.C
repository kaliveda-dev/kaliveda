{
   gROOT->ProcessLine(".L DocConverter.cxx++g");
   ConvertAllModules("kaliveda.git/KVMultiDet");
   ConvertAllModules("kaliveda.git/KVIndra");
   ConvertAllModules("kaliveda.git/VAMOS");
   ConvertAllModules("kaliveda.git/FAZIA");
   ConvertAllModules("kaliveda.git/MicroStat");
   ConvertAllModules("kaliveda.git/BackTrack");
}
