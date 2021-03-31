/***************************************************************************
//                        GTGanilData.H  -  Main Header to ROOTGAnilTape
//                             -------------------
//    begin                : Thu Jun 14 2001
//    copyright            : (C) 2001 by Garp
//    email                : patois@ganil.fr
//////////////////////////////////////////////////////////////////////////
//
// Part of ROOTGanilTape
//
// GTGanilData
//
// Main header for the user interface class
//
//
//////////////////////////////////////////////////////////////////////////
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef GT_GTGanilData_H
#define GT_GTGanilData_H

// ROOT headers
#include "Rtypes.h"
#include <TFile.h>
#include <TString.h>
#include <TTree.h>
#include <TNtuple.h>

#include "GTDataParameters.h"
#include "GTScalers.h"
#include "GanTape/GEN_TYPE.H"
#include "GanTape/STR_EVT.H"
#include "GanTape/gan_acq_buf.h"
#ifndef __GAN_TAPE_GENERAL_H
typedef struct {
   int bidon;
} gan_tape_desc;
#endif

/**
  \class GTGanilData
  \brief Read GANIL formatted tapes or files
  \ingroup DAQ

 Usage can be as simple as:
 GTGanilData myData("filename.dat");

 It allow connection of data parameters values by Connect() routines
 that can operate with the index or the parameter name.

 Values of parameters are stored in an array of UShort_t (0 - 65535)
 Before reading a new event, every parameter is set to 65535
 Casting back this value to a Short_t (-32768 to 32768) gives "-1" which is the signal that the parameter's coder
 did not fire in the event that was read.
*/
class GTGanilData {
public:
   // What to do with scaler buffer
   typedef enum {
      kSkipScaler,     // Skip it.
      kDumpScaler,     // Print all values of scalers in buffer
      kReportScaler,   // Report the existance of a scaler buffer. WARNING! user
      // have to take care of it.
      kAutoWriteScaler // Automatic scaler management: wrote on disk
   } ScalerWhat_t;


public:
   GTGanilData(const TString filename);
   GTGanilData();
   virtual ~GTGanilData(void);

   void            SetFileName(const TString filename);
   void            Open(void);
   void            PrintRunParameters(void) const;
   void            PrintDataParameters(void) const;
   void            DumpEvent(void) const;
   void            DumpParameterName(void) const;
   void            Connect(const Int_t index, UShort_t** p) const;
   void            ConnectFired(const Int_t index, Bool_t** p) const;
   bool            Next(void);
   void            MakeTree(const TString filename = "", UInt_t nEvents = kMaxUInt);
   inline bool     IsScalerBuffer(void) const
   {
      return (fIsScalerBuffer);
   };
   GTScalers* GetScalers(void)
   {
      fIsScalerBuffer = kFALSE;
      return fScaler;
   };
   Int_t           GetStatus(void) const
   {
      return (fStatus);
   }
   void            SetScalerBuffersManagement(const ScalerWhat_t sc);
   Int_t           GetRunNumber(void) const;

   Bool_t          IsOpen(void) const;

   GTDataParameters* GetDataParameters() const
   {
      return fDataParameters;
   };
   const TList* GetListOfDataParameters() const
   {
      return fDataParameters->GetList();
   };
   Int_t GetEventCount() const
   {
      return fEventCount;
   }

   virtual void SetUserTree(TTree*);
   TString GetRunStartDate() const
   {
      return fDateStart;
   }

protected:
   void InitDefault(const Int_t argc = 0, char** argv = NULL);
   virtual void ReadParameters(void);
   void ReadBuffer(void);
   bool ReadNextEvent(void);
   bool ReadNextEvent_EBYEDAT(void);
   virtual bool EventUnravelling(CTRL_EVENT*);

   TString        fFileName;      // Filename, can be a tape drive
   Int_t          fStatus;        // Status, 0 is OK, any other value suspect
   Int_t          fLun;           // Logical number of the device
   gan_tape_desc* fDevice;        //! Device structure (ganil_tape interface)
   Int_t          fDensity;       // Tape density
   in2p3_buffer_struct* fBuffer;  //! Brut data buffer (ganil_tape interface)
   char*          fStructEvent;   //! ??? (ganil_tape interface)
   Int_t          fBufSize;       // Buffer size
   Int_t          fCtrlForm;      // Fix/variable length events
   Int_t          fEvbsize;       // Size of the brut data buffer
   Int_t          fEvcsize;       // Size of the ctrl data buffer
   UShort_t*      fEventBrut;     //! Brut data buffer
   UShort_t*      fEventCtrl;     //! Control data buffer
   bool           fAllocated;     // True if the Tape is allocated (useless?)
   Int_t          fRunNumber;     // current file run number
   char           fHeader[9];     // Buffer header
   UShort_t*      fDataArray;     //! Physical data array
   Bool_t*        fFired;         //! fired parameters in each event
   Int_t          fDataArraySize; // Data array size
   Int_t          fEventNumber;   // Local event number in current buffer (should be renamed)
   Int_t          fEventCount;    // Our event counter
   bool           fIsCtrl;        // We are currently in a CTRL buffer
   bool           fIsScalerBuffer;// The current buffer is a scaler buffer
   ScalerWhat_t   fWhatScaler;    // What do we do with scalers buffers
   GTDataParameters* fDataParameters; //! Data parameters names class
   GTScalers*     fScaler;        //! Scaler array
   TTree*         fScalerTree;    // Scaler tree for automatic filling
   TString        fDateStart;     // Date/time of start of run read from file
   Int_t          fCTRLEVNT_HD;   // size (in 16-bit words) of CTRL_EVNT header
public:
   ClassDef(GTGanilData, 0)       // Interface to a GANIL formated Tape or File
};

#endif
