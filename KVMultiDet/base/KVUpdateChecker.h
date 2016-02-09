/*
$Id: KVUpdateChecker.h,v 1.1 2008/03/05 16:13:45 franklan Exp $
$Revision: 1.1 $
$Date: 2008/03/05 16:13:45 $
*/

//Created by KVClassFactory on Tue Mar  4 17:49:07 2008
//Author: franklan

#ifndef __KVUPDATECHECKER_H
#define __KVUPDATECHECKER_H

#include "KVString.h"

class KVUpdateChecker {
protected:
   KVString fSourceDir; // top-level source directory
   KVString fWorkDir; //! current working directory

   void GotoSourceDir();
   void GotoWorkingDir();

public:
   KVUpdateChecker();
   virtual ~KVUpdateChecker();

   // Returns true if "filename" is out of date compared to repository.
   // If "filename" is not in the top-level directory, give the full-path relative
   // to the top-level directory.
   // If "filename" is not given (default), checks the whole source tree.
   virtual Bool_t NeedsUpdate(const Char_t* filename = "") = 0;

   // Updates "filename" to synchronize it with the repository version.
   // If "filename" is not in the top-level directory, give the full-path relative
   // to the top-level directory.
   // If "filename" is not given (default), updates the whole source tree.
   virtual void Update(const Char_t* filename = "") = 0;

   virtual void SetSourceDir(const Char_t* toplevel)
   {
      // Set top-level source directory
      fSourceDir = toplevel;
   };

   // Returns top-level source directory set with SetSourceDir(const Char_t*)
   virtual const Char_t* GetSourceDir() const
   {
      return fSourceDir.Data();
   };

   ClassDef(KVUpdateChecker, 1) //Checks local sources against version in a given repository and updates if necessary
};

#endif
