 //Based exactly on the RooGlobalFunc.h
 //Add some new commands for the method FitTo() in GenericModel_Binned
 
 #include "NewRooGlobalFunc.h"
 
 namespace RooFit
  { 
   RooCmdArg SetEpsilon(Double_t eps) {return RooCmdArg("SetEpsilon",0,0,eps,0,0,0,0,0) ; };              //Modify the convergence criterion (according to the minimizer)
   RooCmdArg SetMaxIter(Int_t maxiter) {return RooCmdArg("SetMaxIter",maxiter,0,0,0,0,0,0,0) ; };         //Modify max of iterations
   RooCmdArg SetMaxCalls(Int_t maxcalls) {return RooCmdArg("SetMaxCalls",maxcalls,0,0,0,0,0,0,0) ; };     //Modify max calls 
  }
