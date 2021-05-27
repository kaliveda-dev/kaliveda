//Created by KVClassFactory on Wed Jun 30 17:45:01 2010
//Author: bonnet

#include "KVSimReader.h"
#include "TDirectory.h"

ClassImp(KVSimReader)



//____________________________________________________
void KVSimReader::init()
{

   kmode = kTRUE;
   tree = 0;
   evt = 0;
   nuc = 0 ;
   nevt = 0;
   fMultiFiles = kFALSE;
   fFileIndex = 1;
   fOutputDirectory = "./";

   tree_name = "SIMULATION_NAME";
   tree_title = "SIMULATION";
   root_file_name = "Output.root";
   //ascii_file_name = "";
   branch_name = "Simulated_evts";

   nv = new KVNameValueList();

   CreateObjectList();
   CreateInfoList();

}

//____________________________________________________
KVSimReader::KVSimReader()
{
   // Default constructor
   init();
}

void KVSimReader::ConvertEventsInFile(KVString filename)
{
   // Method called by constructors with KVString filename argument
   if (!OpenFileToRead(filename)) return;

   Run();

   CloseFile();
}

//____________________________________________________
KVSimReader::~KVSimReader()
{
   // Destructor
   CleanAll();

   delete nv;
   delete linked_objects;
   delete linked_info;

}

//____________________________________________________
void KVSimReader::CleanAll()
{

   nv->Clear();
   GetSimuInfo()->Clear();
   GetLinkedObjects()->Clear();

}

//____________________________________________________
void KVSimReader::DeclareTree(Option_t* option)
{
   // Use root_file_name as basename for file
   // If multiple files are being read, they will be named
   //    [root_file_name].1
   //    [root_file_name].2
   //    [root_file_name].etc
   // Files are written in fOutputDirectory (default: "./")

   TString filename;
   filename = root_file_name;
   if (fMultiFiles) filename += Form(".%d", fFileIndex);
   if (!fOutputDirectory.EndsWith("/")) fOutputDirectory += "/";
   filename.Prepend(fOutputDirectory);
   Info("DeclareTree", "Ouverture du fichier de stockage %s", filename.Data());
   file = new TFile(filename, option);

   tree = new TTree(tree_name, tree_title);
   KVEvent::MakeEventBranch(tree, branch_name, evt);
}

//____________________________________________________
void KVSimReader::ReadFile()
{

   Info("ReadFile", "To be defined in child class");

   /*
   while (f_in.good()){
      if (ReadHeader()){
         for (Int_t nn=0;nn<nv->GetIntValue("stat"));nn+=1)
            if (!ReadEvent()) break;
            else if (HasToFill()) FillTree();
            else {}
      }

   }

   */
}

//____________________________________________________
Bool_t KVSimReader::ReadHeader()
{

   Info("ReadHeader", "To be defined in child class");
   /*
   Int_t res = ReadLineAndCheck(2," ");
   switch (res){
   case 0:
      Info("ReadHeader","case 0 line est vide");
      return kFALSE;
   case 1:
      nv->SetValue("J",GetDoubleReadPar(0));
      nv->SetValue("stat",GetDoubleReadPar(1));

      Info("ReadHeader","case 1 moment angulaire %1.0lf stat %1.0lf",nv->GetIntValue("J"),nv->GetIntValue("stat"));
      return kTRUE;
   default:
      Info("ReadHeader","case 2 nombre de par lus %d differents de celui attendu %d",toks->GetEntries(),2);
      //Info("ReadHeader","line %s",line.Data());
      return kFALSE;
   }
   */
   return kTRUE;
}

//____________________________________________________
Bool_t KVSimReader::ReadEvent()
{

   Info("ReadEvent", "To be defined in child class");
   /*
   evt->Clear();
   Int_t mult=0;
   Int_t res = ReadLineAndCheck(1," ");
   switch (res){
   case 0:
      Info("ReadEvent","case 0 line est vide");
      return kFALSE;

   case 1:
      evt->SetNumber(nevt);
      mult = GetIntReadPar(0);
      evt->GetParameters()->SetValue("ll",kSIM_var1);

      for (Int_t mm=0; mm<mult; mm+=1){
         nuc = (KVSimNucleus* )evt->AddParticle();
         ReadNucleus();
      }

      nevt+=1;
      return kTRUE;

   default:
      Info("ReadEvent","case 2 nombre de par lus %d differents de celui attendu %d",toks->GetEntries(),1);
      //Info("ReadHeader","line %s",line.Data());
      return kFALSE;
   }
   */
   return kTRUE;

}

//____________________________________________________
Bool_t KVSimReader::ReadNucleus()
{

   Info("ReadNucleus", "To be defined in child class");
   /*
   Int_t res = ReadLineAndCheck(5," ");
   switch (res){
   case 0:
      Info("ReadNucleus","case 0 line est vide");
      return kFALSE;

   case 1:

      nuc->SetZ( GetIntReadPar(0) );
      nuc->SetA( GetIntReadPar(1) );
      nuc->SetKE( GetDoubleReadPar(2) );
      nuc->SetTheta( GetDoubleReadPar(3) * TMath::RadToDeg() );
      nuc->SetPhi( GetDoubleReadPar(4) * TMath::RadToDeg() );

      return kTRUE;

   default:

      Info("ReadNucleus","case 2 nombre de par lus %d differents de celui attendu %d",toks->GetEntries(),5);
      //Info("ReadHeader","line %s",line.Data());
      return kFALSE;
   }
   */
   return kTRUE;
}

void KVSimReader::ConvertAndSaveEventsInFile(KVString filename)
{
   // Read events, convert and save in ROOT file
   ConvertEventsInFile(filename);
   SaveTree();
}

KVSimReader* KVSimReader::MakeSimReader(const char* model_uri)
{
   // Static method
   // Returns an instance of a child class specific to given model.

   TPluginHandler* ph = LoadPlugin("KVSimReader", model_uri);
   if (!ph) {
      ::Error("KVSimReader::MakeSimReader", "No plugin defined with name %s", model_uri);
      return nullptr;
   }
   return (KVSimReader*)ph->ExecPlugin(0);
}

//____________________________________________________
void KVSimReader::Run(Option_t* option)
{

   AddInfo("ascii file read", GetFileName().Data());

   TStopwatch chrono;
   chrono.Start();

   evt = new KVSimEvent();
   if (HasToFill()) DeclareTree(option);
   nuc = 0;
   nevt = 0;

   ReadFile();

   if (HasToFill())
      GetTree()->ResetBranchAddress(GetTree()->GetBranch(branch_name.Data()));

   delete evt;
   chrono.Stop();

   //Info("Run","%d evts lus en %lf seconds",GetNumberOfEvents(),chrono.RealTime());

   KVString snevt;
   snevt.Form("%d", nevt);
   AddInfo("number of events read", snevt.Data());
   AddInfo("date", GetDate().Data());
   AddInfo("user", gSystem->GetUserInfo()->fUser.Data());

}

//____________________________________________________
void KVSimReader::SaveTree()
{

   WriteObjects();
   WriteInfo();

   GetTree()->Write();
   CleanAll();
   file->Close();

}

//____________________________________________________
KVList* KVSimReader::GetSimuInfo()
{
   //
   return linked_info;
}

//____________________________________________________
void KVSimReader::AddInfo(const Char_t* name, const Char_t* val)
{
   GetSimuInfo()->Add(new TNamed(name, val));
}

//____________________________________________________
void KVSimReader::AddInfo(TNamed* named)
{

   GetSimuInfo()->Add(named);
}

//____________________________________________________
void KVSimReader::WriteInfo()
{
   TList* list = GetTree()->GetUserInfo();
   Int_t nentries = linked_info->GetEntries();
   for (Int_t ii = 0; ii < nentries; ii += 1) {
      list->Add(linked_info->RemoveAt(0));
   }
}

//____________________________________________________
void KVSimReader::CreateInfoList()
{
   linked_info = new KVList();
   linked_info->SetOwner(kFALSE);
   linked_info->SetName("Simulation info");
}


//____________________________________________________
KVList* KVSimReader::GetLinkedObjects()
{
   return linked_objects;
}

//____________________________________________________
void KVSimReader::AddObject(TObject* obj)
{
   GetLinkedObjects()->Add(obj);
}

//____________________________________________________
void KVSimReader::WriteObjects()
{

   TList* list = GetTree()->GetUserInfo();
   Int_t nentries = linked_objects->GetEntries();
   for (Int_t ii = 0; ii < nentries; ii += 1) {
      list->Add(linked_objects->RemoveAt(0));
   }

}

//____________________________________________________
void KVSimReader::CreateObjectList()
{
   linked_objects = new KVList(kFALSE);
   linked_objects->SetName("List of objects");
}

/*

fichier correspondant au format de lecture:

0 2
10
1 1 5.810748 2.580167 -2.821796
1 1 9.990623 2.013791 3.080918
1 1 12.384371 1.813083 1.459004
1 1 7.553329 1.807012 1.629090
0 1 1.493379 2.652475 2.525657
0 1 0.971787 2.151803 0.616233
0 1 1.889498 0.789652 -1.659597
1 1 5.860981 1.636198 2.878590
1 1 4.296264 2.359454 2.377290
50 109 1.424833 0.958413 -0.798826
9
1 1 7.719303 0.954321 0.703780
1 1 13.956905 0.957730 1.756477
1 1 6.952527 1.496332 0.294119
1 1 7.750355 2.158870 -2.119215
1 1 5.972726 2.073538 1.496040
1 1 6.617680 1.352951 3.015066
1 1 7.526133 1.842921 3.073465
0 1 0.435096 1.950363 2.852936
49 110 0.426353 1.688514 -1.100454
10 3
9
0 1 2.385526 2.187467 -2.383691
2 4 13.103434 1.147666 2.958037
1 1 7.084269 1.675831 0.720834
0 1 0.742568 1.607419 0.278490
1 1 9.079510 2.765702 -0.297527
1 1 9.300734 0.170849 -0.891740
1 1 8.797631 1.098980 -0.344032
1 1 4.680343 1.923954 -3.007086
49 107 0.133186 2.320088 -0.156727
8
1 1 7.249172 1.496348 -0.976620
2 4 17.685099 0.551523 -2.905849
2 4 27.911638 1.766578 0.524513
1 1 8.419456 1.628637 -1.882687
0 1 0.680420 0.786916 0.688403
1 1 4.465025 1.745583 -1.592567
1 1 4.812778 2.287951 -3.126746
48 105 0.364456 2.255794 2.579171
9
1 1 12.462525 1.826489 1.113706
1 1 7.000116 2.608909 1.919405
0 1 2.846559 2.288652 -1.294329
0 1 1.650262 0.578845 -0.605950
1 1 6.264917 2.046585 1.862553
1 1 5.637622 0.790498 -2.418233
0 1 1.533256 0.051853 -0.220994
2 4 13.625276 0.567988 0.888917
50 107 0.708753 2.151470 -1.940892

*/
