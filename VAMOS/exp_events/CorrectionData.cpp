
///
/// @file CorrectionData.cpp
///
/// @section Description
///
/// Basic class acting as a container for any identification date which need to
/// be passed into the idc::ApplyCorrections function. We prefer to pass in this
/// data manually as the relevant getters in KVVAMOSReconNuc are liable to
/// recalculate values we don't want it to!
///
/// @author Peter C. Wigg <peter.wigg.314159@gmail.com>
/// @date Wed 20 Jan 14:03:01  2016
///

#include "CorrectionData.h"

ClassImp(idc::CorrectionData)

namespace idc {

   CorrectionData::CorrectionData() :
#if __cplusplus < 201103L
      idt(NULL),
#else
      idt(nullptr),
#endif
      z_value(-1),
      a_value(-1),
      z_real(-1.),
      a_real(-1.),
      pid(-1.),
      energy(-1.)
   {

   }

   CorrectionData::~CorrectionData()
   {

   }

   void CorrectionData::Copy(const CorrectionData& rhs)
   {
      idt = rhs.idt;
      z_value = rhs.z_value;
      a_value = rhs.a_value;
      z_real = rhs.z_real;
      a_real = rhs.a_real;
      pid = rhs.pid;
      energy = rhs.energy;
   }

   CorrectionData::CorrectionData(const CorrectionData& rhs)
   {
      Copy(rhs);
   }

   CorrectionData& CorrectionData::operator=(const CorrectionData& rhs)
   {
      Copy(rhs);
      return *this;
   }

} // namespace idc
