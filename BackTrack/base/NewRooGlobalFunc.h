 //Based exactly on the RooGlobalFunc.h
 //Add some new commands for the method FitTo() in GenericModel_Binned
 
 #include "RooCmdArg.h"
 
 namespace RooFit
  { 
   RooCmdArg SetEpsilon(Double_t eps);   
   RooCmdArg SetMaxIter(Int_t maxiter);  
   RooCmdArg SetMaxCalls(Int_t maxcalls);
  }
