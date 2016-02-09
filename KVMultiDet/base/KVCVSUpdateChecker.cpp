/*
$Id: KVCVSUpdateChecker.cpp,v 1.3 2009/01/23 15:26:06 franklan Exp $
$Revision: 1.3 $
$Date: 2009/01/23 15:26:06 $
*/

//Created by KVClassFactory on Tue Mar  4 17:49:56 2008
//Author: franklan

#include "KVCVSUpdateChecker.h"
#include "TSystem.h"

ClassImp(KVCVSUpdateChecker)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVCVSUpdateChecker</h2>
<h4>Checks local sources against CVS version and updates if necessary</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVCVSUpdateChecker::KVCVSUpdateChecker()
{
   // Default constructor
}

KVCVSUpdateChecker::~KVCVSUpdateChecker()
{
   // Destructor
}

Bool_t KVCVSUpdateChecker::NeedsUpdate(const Char_t* filename)
{
   // Returns true if "filename" is out of date compared to repository.
   // If "filename" is not in the top-level directory, give the full-path relative
   // to the top-level directory.
   // If "filename" is not given (default), checks the whole source tree.
   //
   // Checking is done by executing the following shell commands:
   //     cd [source directory]
   //     cvs -q -z3 status [filename] | grep "Needs Patch" >& /dev/null
   //     cd [working directory]
   // If the return value of the second line is 0, then the file(s) need updating.

   KVString cmd("cvs -q -z3 status ");
   if (strcmp(filename, "")) cmd += filename;
   cmd += " | grep \"Needs Patch\" >& /dev/null";
   GotoSourceDir();
   Int_t retcod = gSystem->Exec(cmd.Data());
   GotoWorkingDir();
   return (retcod == 0); // return value = 0 means at least one file Needs Patch
}

void KVCVSUpdateChecker::Update(const Char_t* filename)
{
   // Updates "filename" to synchronize it with the repository version.
   // If "filename" is not in the top-level directory, give the full-path relative
   // to the top-level directory.
   // If "filename" is not given (default), updates the whole source tree.

   KVString cmd("cvs -q -z3 update -dP ");
   if (strcmp(filename, "")) cmd += filename;
   //cmd+=" >& /dev/null";
   GotoSourceDir();
   /*Int_t retcod = */gSystem->Exec(cmd.Data());
   GotoWorkingDir();
}
