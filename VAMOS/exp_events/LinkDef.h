#ifdef __CINT__
#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ global gVamos;

#pragma link C++ class KVINDRA_VAMOS+;
#pragma link C++ class KVIVReconEvent+;
#pragma link C++ class KVVAMOSReconEvent-;
#pragma link C++ class KVVAMOSReconNuc-;

// Identification correction (idc) namespace
#pragma link C++ namespace idc;
#pragma link C++ class idc::CorrectionData+;
#pragma link C++ class idc::AbsorberEnergies+;
#pragma link C++ function idc::Identify;
#pragma link C++ function idc::ApplyCorrections;
#pragma link C++ function idc::ApplyIcSiCorrections;
#pragma link C++ function idc::ApplySiCsiCorrections;
#pragma link C++ function idc::CorrectAoverQ;

#endif
