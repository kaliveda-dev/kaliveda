/*
$Id: KVINDRARRMValidator.cpp,v 1.2 2006/10/19 14:32:43 franklan Exp $
$Revision: 1.2 $
$Date: 2006/10/19 14:32:43 $
$Author: franklan $
*/

#include "KVINDRARRMValidator.h"

ClassImp(KVINDRARRMValidator)
//////////////////////////////////////////////////////////////////////////////////////
//Utility base class used to define validity of various INDRA-related objects (identification grids, calibration
//parameters, etc. etc.) in terms of:
//      - a list of runs
//      - a list of rings
//      - a list of modules
//Each list is kept in an object of the KVNumberList, which handles very general lists such
//as "1-3, 7, 10-19, 21".
KVINDRARRMValidator::KVINDRARRMValidator()
{
   //Default constructor. Does nothing.
}

KVINDRARRMValidator::~KVINDRARRMValidator()
{
   //Destructor. Does nothing.
}
