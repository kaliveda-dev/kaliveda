//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Fri Nov 29 14:37:16 2013 by ROOT version 5.34/08
// from TTree tree_PSA_signals/Tree for PSA
// found on file: root://ccxroot.in2p3.fr:1999//hpss/in2p3.fr/group/fazia/LNS_2011/raw/run_4023.root
//////////////////////////////////////////////////////////

#ifndef KVFAZIARawDataReader_h
#define KVFAZIARawDataReader_h

#include <TROOT.h>
#include <KVFAZIAReader.h>

// Header file for the classes stored in the TTree if any.

// Fixed size dimensions of array or collections stored in the TTree if any.

class KVFAZIARawDataReader : public KVFAZIAReader {
public :

   KVFAZIARawDataReader() : KVFAZIAReader() { }
   virtual ~KVFAZIARawDataReader() { }

   virtual void    Begin(TTree* tree);
   virtual void    SlaveBegin(TTree* tree);

   virtual void    SlaveTerminate();
   virtual void    Terminate();


   ClassDef(KVFAZIARawDataReader, 0);
};

#endif
