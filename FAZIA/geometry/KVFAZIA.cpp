//Created by KVClassFactory on Tue Jan 27 11:37:39 2015
//Author: ,,,

#include "KVFAZIA.h"
#include "KVGeoImport.h"
#include "KVSignal.h"
#include "KVFAZIADetector.h"
#include "KVGroup.h"
#include "KVFAZIABlock.h"
#include "KVDetectorEvent.h"
#include "KVTarget.h"
#include "TSystem.h"
#include "KVDataSet.h"
#include "KVConfig.h"

//#include "TGeoBox.h"
#include "TGeoCompositeShape.h"
#include "TGeoEltu.h"

#include <KVReconstructedNucleus.h>

#ifdef WITH_MFM
#include "MFMFaziaFrame.h"
#endif

#ifdef WITH_PROTOBUF
#include "FzEventSet.pb.h"
#include "KVFzDataReader.h"
#endif

ClassImp(KVFAZIA)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVFAZIA</h2>
<h4>Base class for description of the FAZIA set up</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////
KVFAZIA* gFazia;

static Char_t const* const FzDataType_str[] = { "QH1", "I1", "QL1", "Q2", "I2", "Q3", "ADC", "UNK" };
static Char_t const* const FzDetector_str[] = { "SI1", "SI1", "SI1", "SI2", "SI2", "CSI" };

KVFAZIA::KVFAZIA(const Char_t* title)
   : KVMultiDetArray("FAZIA", title)
{
   // Default constructor
   fStartingBlockNumber = 0;
   gFazia = this;
   fDetectorLabels = "";
   fSignalTypes = "QL1,I1,QH1,Q2,I2,Q3";
   SetGeometryImportParameters();
   CreateCorrespondence();

   // values of trapezoidal filter rise time set in the fpgas to be linked with a database...
   fQH1risetime    = GetSetupParameter("QH1.FPGARiseTime");
   fQ2risetime     = GetSetupParameter("Q2.FPGARiseTime");
   fQ3slowrisetime = GetSetupParameter("Q3.slow.FPGARiseTime");
   fQ3fastrisetime = GetSetupParameter("Q3.fast.FPGARiseTime");

   Info("KVFAZIA", "fpga shapers: %lf %lf %lf %lf", fQH1risetime, fQ2risetime, fQ3slowrisetime, fQ3fastrisetime);
}

//________________________________________________________________
Double_t KVFAZIA::GetSetupParameter(const Char_t* parname)
{

   Double_t lval = -1;
   if (gDataSet)  lval = gDataSet->GetDataSetEnv(parname, 0.0);
   else           lval = gEnv->GetValue(parname, 0.0);
   return lval;
}


KVFAZIA::~KVFAZIA()
{
   // Destructor

   if (gFazia == this) gFazia = nullptr;
}

void KVFAZIA::AddDetectorLabel(const Char_t* label)
{
   if (fDetectorLabels == "") fDetectorLabels += label;
   else if (!fDetectorLabels.Contains(label)) fDetectorLabels += Form(",%s", label);
}

void KVFAZIA::GenerateCorrespondanceFile()
{
   // Look for the geometry object <-> detector name correspondance file in the dataset directory
   //  If not found, we create it

   fCorrespondanceFile = "";
   if (gDataSet) fCorrespondanceFile = gDataSet->GetFullPathToDataSetFile(Form("%s.names", ClassName()));
   if (fCorrespondanceFile != "") return;

#ifdef WITH_GNU_INSTALL
   fCorrespondanceFile.Form("%s/%s.names", KVBase::WorkingDirectory(), ClassName());
#else
   fCorrespondanceFile.Form("%s/%s.names", gDataSet->GetDataSetDir(), ClassName());
#endif
   Info("GenerateCorrespondanceFile", "Creation de %s", fCorrespondanceFile.Data());
   KVEnv env;

   fDetectorLabels = "SI1,SI2,CSI";

   SetNameOfDetectors(env);
   if (env.GetTable() && env.GetTable()->GetEntries() > 0) {
      env.AddCommentLine(Form("Automatic generated file by %s::GenerateCorrespondanceFile", ClassName()));
      env.AddCommentLine("Make link between geometric ROOT objects and detector names");
      env.WriteFile(fCorrespondanceFile.Data());
   }
   fDetectorLabels = "";
}

void KVFAZIA::SetNameOfDetectors(KVEnv& env)
{
   //define the format of detectors name
   // label-index
   // where index = block*100+quartet*10+telescope
   // example :
   // SI1-123 is the Silicon 1 of the block 1, the quartet 2 and the telescope 3
   //

   for (Int_t bb = fStartingBlockNumber; bb < fNblocks; bb += 1) {
      for (Int_t qq = 1; qq <= 4; qq += 1) {
         for (Int_t tt = 1; tt <= 4; tt += 1) {
            fDetectorLabels.Begin(",");
            while (!fDetectorLabels.End()) {
               KVString sdet = fDetectorLabels.Next();
               env.SetValue(
                  Form("BLOCK_%d_QUARTET_%d_%s-T%d", bb, qq, sdet.Data(), tt),
                  Form("%s-%d", sdet.Data(), bb * 100 + qq * 10 + tt)
               );
            }
         }
      }
   }
}

void KVFAZIA::PerformClosedROOTGeometryOperations()
{
   // Finalise description of array performing all operations which require ROOT
   // geometry to be closed

   KVGeoImport imp(gGeoManager, KVMaterial::GetRangeTable(), this, kTRUE);
   if (fImport_Xorg != 0 || fImport_Yorg != 0 || fImport_Zorg != 0) imp.SetOrigin(fImport_Xorg, fImport_Yorg, fImport_Zorg);
   imp.SetDetectorPlugin(GetDataSetEnv(GetDataSet(), "FAZIADetectorPlugin", "FAZIADetector"));
   imp.SetNameCorrespondanceList(fCorrespondanceFile.Data());
   // any additional structure name formatting definitions
   DefineStructureFormats(imp);
   imp.AddAcceptedDetectorName("SI1-");
   imp.AddAcceptedDetectorName("SI2-");
   imp.AddAcceptedDetectorName("CSI-");

   // the following parameters are optimized for a 12-block compact
   // geometry placed at 80cm with rings 1-5 of INDRA removed.
   // make sure that the expected number of detectors get imported!
   imp.ImportGeometry(fImport_dTheta, fImport_dPhi, fImport_ThetaMin, fImport_PhiMin, fImport_ThetaMax, fImport_PhiMax);

   SetDetectorThicknesses();

   SetIdentifications();
   SetBit(kIsBuilt);
}

void KVFAZIA::GetGeometryParameters()
{
   //Called by the Build method
   AbstractMethod("GetGeometryParameters");
}

void KVFAZIA::BuildFAZIA()
{
   //Called by the Build method
   Info("BuildFAZIA", "to be defined in child class ...");

}

void KVFAZIA::BuildTarget()
{

   KVMaterial target_holder_mat("Al");
   new TGeoBBox("TARGET_FRAME", 3., 3., 0.1 / 2.);
   new TGeoEltu("TARGET_HOLE", 2., 2., 0.1 / 2.);
   TGeoCompositeShape* cs = new TGeoCompositeShape("TARGET_FRAME", "TARGET_FRAME - TARGET_HOLE");
   TGeoVolume* target_frame = new TGeoVolume("TARGET_FRAME", cs, target_holder_mat.GetGeoMedium());
   gGeoManager->GetTopVolume()->AddNode(target_frame, 1);

   KVTarget* T = GetTarget();
   if (T) {
      KVMaterial* targMat = (KVMaterial*)T->GetLayers()->First();
      TGeoVolume* target = gGeoManager->MakeEltu("TARGET", targMat->GetGeoMedium(), 2., 2., targMat->GetThickness() / 2.);
      gGeoManager->GetTopVolume()->AddNode(target, 1);
   }
}

void KVFAZIA::Build(Int_t)
{
   // Build the FAZIA array
   GetGeometryParameters();
   GenerateCorrespondanceFile();

   CreateGeoManager();

   BuildFAZIA();

   if (fBuildTarget)
      BuildTarget();

   if (fCloseGeometryNow) {
      gGeoManager->CloseGeometry();
      PerformClosedROOTGeometryOperations();
   }
}

void KVFAZIA::GetDetectorEvent(KVDetectorEvent* detev, const TSeqCollection* dets)
{
   // First step in event reconstruction based on current status of detectors in array.
   // Fills the given KVDetectorEvent with the list of all groups which have fired.
   // i.e. loop over all groups of the array and test whether KVGroup::Fired() returns true or false.
   //
   // If the list of fired acquisition parameters 'sigs' is not given,
   // KVMultiDetArray::GetDetectorEvent is called. We check also if the internal fFiredACQParams
   // list contains data.
   //

   if (!fHandledRawData) {
      //Info("GetDetectorEvent","i didnt handle any data...");
      return;
   }
   if (!dets || !dets->GetEntries()) {
      if (fFiredDetectors.GetEntries()) {
         dets = &fFiredDetectors;
         //Info("GetDetectorEvent", "using internal list");
      }
   }
   if (dets && dets->GetEntries()) {
      TIter next_det(dets);

      KVDetector* det = 0;
      while ((det = (KVDetector*)next_det())) {

         if (det->GetGroup()->Fired()) detev->AddGroup(det->GetGroup());

      }
   }
   else {
      //Info("GetDetectorEvent", "Calling base method");
      KVMultiDetArray::GetDetectorEvent(detev, 0);
   }
}

void KVFAZIA::FillDetectorList(KVReconstructedNucleus* rnuc, KVHashList* DetList, const KVString& DetNames)
{
   // Protected method, called when required to fill fDetList with pointers to
   // the detectors whose names are stored in fDetNames.
   // Also set all raw data values in the detectors.

   KVFAZIADetector* det = 0;

   DetList->Clear();
   DetNames.Begin("/");
   while (!DetNames.End()) {
      KVString sdet = DetNames.Next(kTRUE);
      det = (KVFAZIADetector*)GetDetector(sdet.Data());
      if (!det) {
         det = (KVFAZIADetector*)GetDetector(KVFAZIADetector::GetNewName(sdet.Data()));
      }

      if (det) {
         DetList->Add(det);
         // read and set from the particle's parameter list any relevant detector signal values
         // each signal is stored with a name "[detname].[signal name]"
         // except GTTag and DetTag which have the same value for all detectors of the same telescope
         // and so are only stored once with name "DetTag" or "GTTag".

         TIter it(&det->GetListOfDetectorSignals());
         KVDetectorSignal* ds;
         while ((ds = (KVDetectorSignal*)it())) {
            if (ds->IsRaw() && !ds->IsExpression())
               // only look for raw data, excluding any expressions based only on raw data
            {
               TString pname = Form("%s.%s", det->GetName(), ds->GetName());
               if (rnuc->GetParameters()->HasParameter(pname))
                  ds->SetValue(rnuc->GetParameters()->GetDoubleValue(pname));
            }
         }
         if (rnuc->GetParameters()->HasParameter("GTTag"))
            det->SetGTTag(rnuc->GetParameters()->GetIntValue("GTTag"));
         if (rnuc->GetParameters()->HasParameter("DetTag"))
            det->SetDetTag(rnuc->GetParameters()->GetIntValue("DetTag"));
      }
   }
}

KVGroupReconstructor* KVFAZIA::GetReconstructorForGroup(const KVGroup* g) const
{
   // Specialized group reconstructor for FAZIA

   KVGroupReconstructor* gr(nullptr);
   if (GetGroup(g->GetName())) { // make sure group belongs to us
      gr = KVGroupReconstructor::Factory("FAZIA");
   }
   return gr;
}

TString KVFAZIA::GetSignalName(Int_t bb, Int_t qq, Int_t tt, Int_t idsig)
{

   TString sname;
   if (bb == 4) {
      if (fDataSet == "FAZIASYM") {
         sname.Form("%s-RUTH", FzDataType_str[idsig]);
      }
      else {
         sname.Form("%s-%d", FzDataType_str[idsig], 100 * bb + 10 * qq + tt);
      }
   }
   else if (bb == 6) {
      if (fDataSet == "FAZIAPRE") {
         sname.Form("%s-RUTH", FzDataType_str[idsig]);
      }
      else {
         sname.Form("%s-%d", FzDataType_str[idsig], 100 * bb + 10 * qq + tt);
      }
   }
   else {
      sname.Form("%s-%d", FzDataType_str[idsig], 100 * bb + 10 * qq + tt);
   }
   return sname;

}

#ifdef WITH_PROTOBUF
Bool_t KVFAZIA::handle_raw_data_event_protobuf(KVProtobufDataReader& R)
{
   return treat_event(((KVFzDataReader&)R).get_fazia_event());
}

Double_t KVFAZIA::TreatEnergy(Int_t sigid, Int_t eid, UInt_t val)
{
   Int_t value = (val << 2);
   value >>= 2;
   Double_t dval = -1.;
   switch (sigid) {
      case DAQ::FzData_FzDataType_QH1:
         if (eid == 0) dval = value / (fQH1risetime * 1e3 / 10.);
         break;
      case DAQ::FzData_FzDataType_I1:
         break;
      case DAQ::FzData_FzDataType_QL1:
         break;
      case DAQ::FzData_FzDataType_Q2:
         if (eid == 0) dval = value / (fQ2risetime * 1e3 / 10.);
         break;
      case DAQ::FzData_FzDataType_I2:
         break;
      case DAQ::FzData_FzDataType_Q3:
         if (eid == 0) dval = value / (fQ3slowrisetime * 1e3 / 10.);
         if (eid == 1) dval = value / (fQ3fastrisetime * 1e3 / 10.);
         break;
   }
   return dval;
}

Bool_t KVFAZIA::treat_event(const DAQ::FzEvent& e)
{
   // Read raw data for an event

   Bool_t good = kTRUE;

   //get info from trigger
   int ts = e.trinfo_size();
   uint64_t dt = 0;
   uint64_t tot = 0;
   for (Int_t tr = ts - 1; tr >= 0; tr--) {
      const DAQ::FzTrigInfo& rdtrinfo = e.trinfo(tr);
      uint64_t triggervalue = rdtrinfo.value();
      if (tr == ts - 5)       fReconParameters.SetValue("FAZIA.TRIGPAT", (int)triggervalue);
      else if (tr == ts - 6)  fReconParameters.SetValue64bit("FAZIA.EC", ((triggervalue << 12) + e.ec()));
      else if (tr == ts - 8)  dt = triggervalue;
      else if (tr == ts - 9)  fReconParameters.SetValue("FAZIA.TRIGRATE.EXT", 1.*triggervalue / dt);
      else if (tr == ts - 10) fReconParameters.SetValue("FAZIA.TRIGRATE.MAN", 1.*triggervalue / dt);
      else if (tr == ts - 11) fReconParameters.SetValue(Form("FAZIA.TRIGRATE.PAT%d", tr - 2), 1.*triggervalue / dt);
      else if (tr == ts - 12) fReconParameters.SetValue(Form("FAZIA.TRIGRATE.PAT%d", tr - 2), 1.*triggervalue / dt);
      else if (tr == ts - 13) fReconParameters.SetValue(Form("FAZIA.TRIGRATE.PAT%d", tr - 2), 1.*triggervalue / dt);
      else if (tr == ts - 14) fReconParameters.SetValue(Form("FAZIA.TRIGRATE.PAT%d", tr - 2), 1.*triggervalue / dt);
      else if (tr == ts - 15) fReconParameters.SetValue(Form("FAZIA.TRIGRATE.PAT%d", tr - 2), 1.*triggervalue / dt);
      else if (tr == ts - 16) fReconParameters.SetValue(Form("FAZIA.TRIGRATE.PAT%d", tr - 2), 1.*triggervalue / dt);
      else if (tr == ts - 17) fReconParameters.SetValue(Form("FAZIA.TRIGRATE.PAT%d", tr - 2), 1.*triggervalue / dt);
      else if (tr == ts - 18) fReconParameters.SetValue(Form("FAZIA.TRIGRATE.PAT%d", tr - 2), 1.*triggervalue / dt);
      else if (tr == ts - 19) {
         fReconParameters.SetValue("FAZIA.TRIGRATE.TOT", 1.*triggervalue / dt);
         tot = triggervalue;
      }
      else if (tr == ts - 20) {
         fReconParameters.SetValue("FAZIA.TRIGRATE.VAL", 1.*triggervalue / dt);
         fReconParameters.SetValue("FAZIA.DEADTIME", 100.*(1. - 1.*triggervalue / tot));
      }
      else {}
   }

   for (int b = 0; b < e.block_size(); ++b) {

      // check block errors
      if (e.block(b).len_error() || e.block(b).crc_error() || (!good)) {
         //Warning("treat_event", "BLOCK LEN OR CRC ERROR B%03d", e.block(b).blkid());
         good = kFALSE;
         break;  //stop iteration on blocks
      }
      int fIdBlk = e.block(b).blkid();

      for (int f = 0; f < e.block(b).fee_size(); ++f) {

         const DAQ::FzFee& rdfee = e.block(b).fee(f);

         for (int h = 0; h < rdfee.hit_size(); ++h) {

            const DAQ::FzHit& rdhit = rdfee.hit(h);
            // check fee errors
            if (rdfee.len_error() || rdfee.crc_error() || (!good)) {
               Warning("treat_event", "FEE LEN OR CRC ERROR B%03d-FE%d", e.block(b).blkid(), rdfee.feeid());
               good = kFALSE;
               break;  //stop iteration on hits
            }
            int fIdFee = rdhit.feeid();
            int fIdTel = rdhit.telid();

            for (Int_t mm = 0; mm < rdhit.data_size(); mm++) {
               const DAQ::FzData& rdata = rdhit.data(mm);
               int fIdSignal = rdata.type();

               int DetTag = rdhit.dettag();
               int GTTag = rdhit.gttag();
               if (DetTag >= 16384 && GTTag < 16384) GTTag += 32768;

               //on decompile le HIT
               int fIdQuartet = fQuartet[fIdFee][fIdTel];
               int fIdTelescope = fTelescope[fIdFee][fIdTel];

               KVFAZIADetector* det = (KVFAZIADetector*)GetDetector(Form("%s-%d", FzDetector_str[fIdSignal], 100 * fIdBlk + 10 * fIdQuartet + fIdTelescope));
               if (!det) {
//                  Error("treat_event", "No detector %s-%d found in FAZIA geometry...", FzDetector_str[fIdSignal], 100 * fIdBlk + 10 * fIdQuartet + fIdTelescope);
                  continue;
               }
               det->SetDetTag(DetTag);
               det->SetGTTag(GTTag);

               if (!rdata.has_energy() && !rdata.has_waveform()) {
                  Warning("treat_event", "[NO DATA] [%s %s]", det->GetName(), FzDataType_str[fIdSignal]);
                  continue;
               }

               if (rdata.has_energy()) {
                  const DAQ::Energy& ren = rdata.energy();
                  for (Int_t ee = 0; ee < ren.value_size(); ee++) {
                     Double_t energy = TreatEnergy(fIdSignal, ee, ren.value(ee));
                     det->SetFPGAEnergy(fIdSignal, ee, energy);
                  }
                  fFiredDetectors.Add(det);
               }
               if (rdata.has_waveform()) {
                  const DAQ::Waveform& rwf = rdata.waveform();
                  Int_t supp;

                  if (fIdSignal <= 5) {
                     TString sname = GetSignalName(fIdBlk, fIdQuartet, fIdTelescope, fIdSignal);//QH1-123 etc.
                     if (sname == "")
                        Warning("treat_event", "signal name is empty !!! blk=%d qua=%d tel=%d\n", fIdBlk, fIdQuartet, fIdTelescope);

                     TGraph sig(rwf.sample_size());

                     for (Int_t nn = 0; nn < rwf.sample_size(); nn++) {
                        if (fIdSignal != DAQ::FzData::ADC) {
                           if (rwf.sample(nn) > 8191) {
                              supp = rwf.sample(nn) | 0xFFFFC000;
                           }
                           else {
                              supp = rwf.sample(nn);
                           }
                        }
                        else {
                           supp = rwf.sample(nn);
                        }
                        sig.SetPoint(nn, nn, supp);
                     }
                     det->SetSignal(&sig, sname);
                  }
                  else {
                     if (fIdSignal > 5)
                        Warning("treat_event", "datatype %d>5 - taille = %d\n", fIdSignal, rwf.sample_size());
                  }
               }
            }
         }
      }
   }

//   cout << "good=" << good << endl;
//   fFPGAParameters.ls();
//   fSignals.ls();

   return good;
}
#endif

#ifdef WITH_MFM
Bool_t KVFAZIA::handle_raw_data_event_mfmframe(const MFMCommonFrame& f)
{
   // Treatment of raw data in MFM frames with type MFM_FAZIA_FRAME_TYPE
   // The timestamp is extracted from the frame header and added to fReconParameters
   // in a 64 bit parameter with name "FAZIA.TS"

   if (f.GetFrameType() != MFM_FAZIA_FRAME_TYPE) return kFALSE;
   fReconParameters.SetValue64bit("FAZIA.TS", f.GetTimeStamp());

#ifdef WITH_PROTOBUF
   DAQ::FzEventSet fazia_set;
   DAQ::FzEvent fazia_event;
   // Parse protobuf data in MFM frame
   if (fazia_set.ParseFromArray(f.GetPointUserData(), ((MFMFaziaFrame&)f).GetEventSize())) {
      // Parsed an event set
      if (fazia_set.ev_size() > 1) {
         Warning("handle_raw_data_event_mfmframe",
                 "Got a FzEventSet from data: cannot handle multiple events at once!");
         return kFALSE;
      }
      return treat_event(fazia_set.ev(0));
   }
   else if (fazia_event.ParseFromArray(f.GetPointUserData(), ((MFMFaziaFrame&)f).GetEventSize())) {
      // Parsed an event
      return treat_event(fazia_event);
   }
#endif
   return kTRUE;
}
#endif

void KVFAZIA::CreateCorrespondence()
{
   // set up correspondence between FPGA number/FEE number (from acquisition)
   // and Quartet/Telescope numbers

   TString DataFilePath;
   if (!KVBase::SearchKVFile("ElecDetLink.env", DataFilePath, "data")) {
      Error("CreateCorrespondence", "ElecDetLink.env not found");
      return;
   }
   KVEnv DetLink;
   DetLink.ReadFile(DataFilePath, kEnvUser);
   for (int t = 1; t <= 4; t++) {
      for (int q = 1; q <= 4; q++) {
         TString elec = DetLink.GetValue(Form("T%1d-Q%1d", t, q), " ");
         if (!elec.IsWhitespace()) {
            int fee, fpga;
            sscanf(elec.Data(), "FPGA%d-FE%d", &fpga, &fee);
            fQuartet[fee][fpga] = q;
            fTelescope[fee][fpga] = t;
         }
         else {
            Error("CreateCorrespondence", "Problem reading FAZIA ElecDetLink.env file : T%1d-Q%1d = %s", t, q, elec.Data());
         }
      }
   }
}

