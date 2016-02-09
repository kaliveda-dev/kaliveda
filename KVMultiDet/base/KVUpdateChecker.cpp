/*
$Id: KVUpdateChecker.cpp,v 1.1 2008/03/05 16:13:45 franklan Exp $
$Revision: 1.1 $
$Date: 2008/03/05 16:13:45 $
*/

//Created by KVClassFactory on Tue Mar  4 17:49:07 2008
//Author: franklan

#include "KVUpdateChecker.h"
#include "TSystem.h"

ClassImp(KVUpdateChecker)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVUpdateChecker</h2>
<h4>Checks local sources against version in a given repository and updates if necessary</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVUpdateChecker::KVUpdateChecker()
{
   // Default constructor
}

KVUpdateChecker::~KVUpdateChecker()
{
   // Destructor
}

void KVUpdateChecker::GotoSourceDir()
{
   // Memorise current working directory and then change directory
   // to top-level source directory
   fWorkDir = gSystem->WorkingDirectory();
   gSystem->ChangeDirectory(fSourceDir.Data());
}

void KVUpdateChecker::GotoWorkingDir()
{
   // Resent current directory to working directory memorised before
   // changing to top-level source directory
   gSystem->ChangeDirectory(fWorkDir.Data());
}

