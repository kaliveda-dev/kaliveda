// Author: Peter C. Wigg
// Created Wed Jun 11 15:00:35 CEST 2014

////////////////////////////////////////////////////////////////////////////////
//
// KVIVDB_e503.cpp
//
// Description
//
// Database class for the E503 experiment. This class inherits from that of our
// sister experiment (E494s) and adds the functionality required to implement
// the VAMOS identification corrections.
//
// Peter C. Wigg <peter.wigg.314159@gmail.com>
// Wed Jun 11 15:00:35 CEST 2014
//
////////////////////////////////////////////////////////////////////////////////

#include "KVIVDB_e503.h"

ClassImp(KVIVDB_e503)

//______________________________________________________________________________

KVIVDB_e503::KVIVDB_e503() :
#if __cplusplus < 201103L
   id_correction_parameters_(NULL),
#else
   // C++11 is supported, use nullptr
   id_correction_parameters_(nullptr),
#endif
   verbose_(kTRUE)
{
   Init();
}

//______________________________________________________________________________

KVIVDB_e503::KVIVDB_e503(const Char_t* name) :
   KVIVDB(name),
#if __cplusplus < 201103L
   id_correction_parameters_(NULL),
#else
   // C++11 is supported, use nullptr
   id_correction_parameters_(nullptr),
#endif
   verbose_(kTRUE)
{
   Init();
}

//______________________________________________________________________________

void KVIVDB_e503::Init()
{
   id_correction_parameters_ =
      AddTable("VAMOSIDCorrectionParameters",
               "VAMOS ID Correction Parameters");

   assert(id_correction_parameters_);
}

//______________________________________________________________________________

KVIVDB_e503::~KVIVDB_e503()
{

}

//______________________________________________________________________________

void KVIVDB_e503::Build()
{
   KVIVDB::Build();

   Info("Build", "Reading ID correction parameters...");

   if (!ReadIdCorrectionParameters()) {
      Error("Build", "Terminating Program...");
      // TODO: Throw exception
      std::exit(EXIT_FAILURE);
   }

}

//______________________________________________________________________________

Bool_t KVIVDB_e503::ReadIdCorrectionParameters()
{
   std::ifstream f;

   const TString filename_env("VAMOSIDCorrectionList");

   if (!OpenCalibFile(filename_env.Data(), f)) {
      Error("ReadIdCorrectionParameters", "Could not open file %s",
            GetCalibFileName(filename_env.Data()));
      return kFALSE;
   }

   TString line;
   TString line_stripped;;
   TString path;

   while (line.ReadLine(f)) {

      line_stripped = line.Strip(TString::kBoth);
      if (line_stripped.BeginsWith("#")) continue;

      path = gDataSet->GetDataSetDir();
      path += "/";
      path += line_stripped;

      Info("ReadIdCorrectionParameters", "Reading %s...",
           line_stripped.Data());

      if (!ReadCorrectorFile(path)) {
         Error("ReadIdCorrectionParameters",
               "Error(s) encountered while reading correction parameters");
         f.close();
         return kFALSE;
      }
   }

   f.close();

   Info("KVIVDB_e503::ReadIdCorrectionParameters", "ALL OK");

   return kTRUE;
}

//______________________________________________________________________________

Bool_t KVIVDB_e503::ReadCorrectorFile(const TString& filename)
{
   std::ifstream f;
   f.open(filename.Data());

   if (!f.good()) {
      Error("ReadCorrectorFile", "Failed to open file %s",
            filename.Data());
      return kFALSE;
   }

   TString line;
   TString line_stripped;

   Bool_t entry(kFALSE);

   TObjArray tokens;
   TString content;
   TString substr;

   KVNumberList run_list;

   std::vector<Double_t> parameters;

   TString stripped_parameter;
   TString stripped_runlist;
   TString record_id;

#if __cplusplus < 201103L
   KVDBParameterSet* parset(NULL);
#else
   KVDBParameterSet* parset(nullptr);
#endif

   Ssiz_t index(0);
   Bool_t status(kTRUE);

   while (line.ReadLine(f)) {

      line_stripped = line.Strip(TString::kBoth);
      if (line_stripped.BeginsWith("#")) continue;

      if (line_stripped.BeginsWith("[entry]")) {
         if (entry) {
            // push back last entry
            if (verbose_) {
               Info("ReadCorrectorFile", "Adding record for %s",
                    record_id.Data());
            }

            // Table takes ownership of the parameter sets (do not delete)
            parset = new KVDBParameterSet(
               record_id.Data(),
               Form("Correction parameters for %s", record_id.Data()),
               parameters.size()
            );

            parset->SetParameters(&parameters);
            id_correction_parameters_->AddRecord(parset);

            if (verbose_) {
               Info("ReadCorrectorFile", "Linking record to %s",
                    run_list.GetList());
            }

            LinkRecordToRunRange(parset, run_list);
            parameters.clear();
         }

         // kTRUE hereafter (just used to ignore the first entry until we
         // have its data and can create and link the record).
         entry = kTRUE;

         continue;
      }


      if (line_stripped.BeginsWith("+record_id:")) {
         index = line_stripped.Index("=");

         if (index < 0) {
            Error("ReadCorrectorFile",
                  "Failed to find value for key \'record_id\', "
                  "missing \'=\'?\n"
                  "> %s", line_stripped.Data());

            status = kFALSE;
            continue;
         }

         substr = line_stripped(
                     index + 1,
                     line_stripped.Length() - (index + 1));

         record_id = substr.Strip(TString::kBoth);

      } else if (line_stripped.BeginsWith("+run_range:")) {
         index = line_stripped.Index("=");

         if (index < 0) {
            Error("ReadCorrectorFile",
                  "Failed to find value for key \'run_range\', "
                  "missing \'=\'?\n"
                  "> %s", line_stripped.Data());

            status = kFALSE;
            continue;
         }

         substr = line_stripped(
                     index + 1,
                     line_stripped.Length() - (index + 1));

         stripped_runlist = substr.Strip(TString::kBoth);
         run_list.SetList(stripped_runlist);

      } else if (line_stripped.BeginsWith("+parameter:")) {
         index = line_stripped.Index("=");

         if (index < 0) {
            Error("ReadCorrectorFile",
                  "Failed to find value for key \'parameter\', "
                  "missing \'=\'?\n"
                  "> %s", line_stripped.Data());

            status = kFALSE;
            continue;
         }

         substr = line_stripped(
                     index + 1,
                     line_stripped.Length() - (index + 1));

         stripped_parameter = substr.Strip(TString::kBoth);
         parameters.push_back(stripped_parameter.Atof());

      }

   }

   f.close();
   return status;
}

//______________________________________________________________________________

void KVIVDB_e503::SetVerbose(Bool_t status)
{
   verbose_ = status;
}

