//Created by KVClassFactory on Wed Sep  9 10:51:47 2015
//Author: fable

#include "NewRooAddPdf.h"
#include "NewRooGlobalFunc.h"
#include "NewRooMinimizer.h"
#include "NewRooMinuit.h"
#include "NewRooFitResult.h"
//#include "NewRooAddition.h"
//#include "NewRooNLLVar.h"
//#include "NewRooAbsReal.h"

#include "RooFit.h"
#include "RooMsgService.h"
#include "TClass.h"
#include "Riostream.h"
#include "TMath.h"
#include "TObjString.h"
#include "TPaveText.h"
#include "TList.h"
#include "TH1.h"
#include "TH2.h"
#include "TMatrixD.h"
#include "TMatrixDSym.h"
#include "RooAbsPdf.h"
#include "RooDataSet.h"
#include "RooArgSet.h"
#include "RooArgProxy.h"
#include "RooRealProxy.h"
#include "RooRealVar.h"
#include "RooGenContext.h"
#include "RooBinnedGenContext.h"
#include "RooPlot.h"
#include "RooCurve.h"
#include "RooNLLVar.h"
#include "RooCategory.h"
#include "RooNameReg.h"
#include "RooCmdConfig.h"
#include "RooGlobalFunc.h"
#include "RooAddition.h"
#include "RooRandom.h"
#include "RooNumIntConfig.h"
#include "RooProjectedPdf.h"
#include "RooInt.h"
#include "RooCustomizer.h"
#include "RooConstraintSum.h"
#include "RooParamBinning.h"
#include "RooNumCdf.h"
#include "RooFitResult.h"
#include "RooNumGenConfig.h"
#include "RooCachedReal.h"
#include "RooXYChi2Var.h"
#include "RooChi2Var.h"
#include "RooRealIntegral.h"
#include "Math/CholeskyDecomp.h"
#include "RooRecursiveFraction.h"
#include "RooArgList.h"
#include "TNamed.h"
#include "RooConstVar.h"
#include "RooAddPdf.h"

using namespace RooFit;
using namespace std;

ClassImp(NewRooAddPdf)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>NewRooAddPdf</h2>
<h4>Modified RooFit class NewRooAddPdf with new fit options</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////


//Modified
//_____________________________________________________________________________
NewRooAddPdf::NewRooAddPdf() : RooAddPdf()
{
   // Default constructor used for persistence
}



//_____________________________________________________________________________
NewRooAddPdf::NewRooAddPdf(const char* name, const char* title) : RooAddPdf(name, title)
{
   // Dummy constructor
}



//_____________________________________________________________________________
NewRooAddPdf::NewRooAddPdf(const char* name, const char* title, RooAbsPdf& pdf1, RooAbsPdf& pdf2, RooAbsReal& coef1) : RooAddPdf(name, title, pdf1, pdf2, coef1)
{
   // Constructor with two PDFs and one coefficient
}



//_____________________________________________________________________________
NewRooAddPdf::NewRooAddPdf(const char* name, const char* title, const RooArgList& inPdfList, const RooArgList& inCoefList, Bool_t recursiveFractions) : RooAddPdf(name, title, inPdfList, inCoefList, recursiveFractions)
{
   // Generic constructor from list of PDFs and list of coefficients.
   // Each pdf list element (i) is paired with coefficient list element (i).
   // The number of coefficients must be either equal to the number of PDFs,
   // in which case extended MLL fitting is enabled, or be one less.
   //
   // All PDFs must inherit from RooAbsPdf. All coefficients must inherit from RooAbsReal
   //
   // If the recursiveFraction flag is true, the coefficients are interpreted as recursive
   // coefficients as explained in the class description.

   _refCoefNorm      = RooSetProxy("!refCoefNorm", "Reference coefficient normalization set", this, kFALSE, kFALSE);
   _refCoefRangeName = 0;
   _projectCoefs     = kFALSE;
   _projCacheMgr     = RooObjCacheManager(this, 10);
   _codeReg          = 10;
   _pdfList          = RooListProxy("!pdfs", "List of PDFs", this),
   _coefList         = RooListProxy("!coefficients", "List of coefficients", this),
   _haveLastCoef     = kFALSE;
   _allExtendable    = kFALSE;
   _recursive        = kFALSE;


   if (inPdfList.getSize() > inCoefList.getSize() + 1 || inPdfList.getSize() < inCoefList.getSize()) {
      coutE(InputArguments) << "NewRooAddPdf::NewRooAddPdf(" << GetName()
                            << ") number of pdfs and coefficients inconsistent, must have Npdf=Ncoef or Npdf=Ncoef+1" << std::endl ;
      assert(0) ;
   }

   if (recursiveFractions && inPdfList.getSize() != inCoefList.getSize() + 1) {
      coutW(InputArguments) << "NewRooAddPdf::NewRooAddPdf(" << GetName()
                            << ") WARNING inconsistent input: recursive fractions options can only be used if Npdf=Ncoef+1, ignoring recursive fraction setting" << std::endl ;
   }

   // Constructor with N PDFs and N or N-1 coefs
   TIterator* pdfIter = inPdfList.createIterator() ;
   TIterator* coefIter = inCoefList.createIterator() ;
   RooAbsPdf* pdf ;
   RooAbsReal* coef ;

   RooArgList partinCoefList ;

   Bool_t first(kTRUE) ;

   while ((coef = (RooAbsPdf*)coefIter->Next())) {
      pdf = (RooAbsPdf*) pdfIter->Next() ;
      if (!pdf) {
         coutE(InputArguments) << "NewRooAddPdf::NewRooAddPdf(" << GetName()
                               << ") number of pdfs and coefficients inconsistent, must have Npdf=Ncoef or Npdf=Ncoef+1" << std::endl ;
         assert(0) ;
      }
      if (!dynamic_cast<RooAbsReal*>(coef)) {
         coutE(InputArguments) << "NewRooAddPdf::NewRooAddPdf(" << GetName() << ") coefficient " << coef->GetName() << " is not of type RooAbsReal, ignored" << std::endl ;
         continue ;
      }
      if (!dynamic_cast<RooAbsReal*>(pdf)) {
         coutE(InputArguments) << "NewRooAddPdf::NewRooAddPdf(" << GetName() << ") pdf " << pdf->GetName() << " is not of type RooAbsPdf, ignored" << std::endl ;
         continue ;
      }
      _pdfList.add(*pdf) ;

      // Process recursive fraction mode separately
      if (recursiveFractions) {
         partinCoefList.add(*coef) ;
         if (first) {

            // The first fraction is the first plain fraction
            first = kFALSE ;
            _coefList.add(*coef) ;

         }
         else {

            // The i-th recursive fraction = (1-f1)*(1-f2)*...(fi) and is calculated from the list (f1,...,fi) by RooRecursiveFraction)
            RooAbsReal* rfrac = new RooRecursiveFraction(Form("%s_recursive_fraction_%s", GetName(), pdf->GetName()), "Recursive Fraction", partinCoefList) ;
            NewRooAddPdf::addOwnedComponents(*rfrac, kTRUE) ;
            _coefList.add(*rfrac, kTRUE) ;

         }

      }
      else {
         _coefList.add(*coef) ;
      }
   }

   pdf = (RooAbsPdf*) pdfIter->Next() ;
   if (pdf) {
      if (!dynamic_cast<RooAbsReal*>(pdf)) {
         coutE(InputArguments) << "NewRooAddPdf::NewRooAddPdf(" << GetName() << ") last pdf " << coef->GetName() << " is not of type RooAbsPdf, fatal error" << std::endl ;
         assert(0) ;
      }
      _pdfList.add(*pdf) ;

      // Process recursive fractions mode
      if (recursiveFractions) {

         // The last recursive fraction = (1-f1)*(1-f2)*...(1-fN) and is calculated from the list (f1,...,fN,1) by RooRecursiveFraction)
         partinCoefList.add(RooFit::RooConst(1)) ;
         RooAbsReal* rfrac = new RooRecursiveFraction(Form("%s_recursive_fraction_%s", GetName(), pdf->GetName()), "Recursive Fraction", partinCoefList) ;
         NewRooAddPdf::addOwnedComponents(*rfrac, kTRUE) ;
         _coefList.add(*rfrac, kTRUE) ;

         // In recursive mode we always have Ncoef=Npdf
         _haveLastCoef = kTRUE ;
      }

   }
   else {
      _haveLastCoef = kTRUE ;
   }

   delete pdfIter ;
   delete coefIter  ;

   _coefCache = new Double_t[_pdfList.getSize()] ;
   _coefErrCount = _errorCount ;
   _recursive = recursiveFractions ;

   TRACE_CREATE
}



//_____________________________________________________________________________
NewRooAddPdf::NewRooAddPdf(const char* name, const char* title, const RooArgList& inPdfList) : RooAddPdf(name, title, inPdfList)
{
   // Generic constructor from list of extended PDFs. There are no coefficients as the expected
   // number of events from each components determine the relative weight of the PDFs.
   //
   // All PDFs must inherit from RooAbsPdf.

   _refCoefNorm = RooSetProxy("!refCoefNorm", "Reference coefficient normalization set", this, kFALSE, kFALSE),
   _refCoefRangeName = 0;
   _projectCoefs = kFALSE;
   _projCacheMgr = RooObjCacheManager(this, 10);
   _pdfList = RooListProxy("!pdfs", "List of PDFs", this);
   _coefList = RooListProxy("!coefficients", "List of coefficients", this);
   _haveLastCoef = kFALSE;
   _allExtendable = kTRUE;
   _recursive = kFALSE;

   // Constructor with N PDFs
   TIterator* pdfIter = inPdfList.createIterator() ;
   RooAbsPdf* pdf ;
   while ((pdf = (RooAbsPdf*) pdfIter->Next())) {

      if (!dynamic_cast<RooAbsReal*>(pdf)) {
         coutE(InputArguments) << "NewRooAddPdf::NewRooAddPdf(" << GetName() << ") pdf " << pdf->GetName() << " is not of type RooAbsPdf, ignored" << std::endl ;
         continue ;
      }
      if (!pdf->canBeExtended()) {
         coutE(InputArguments) << "NewRooAddPdf::NewRooAddPdf(" << GetName() << ") pdf " << pdf->GetName() << " is not extendable, ignored" << std::endl ;
         continue ;
      }

      _pdfList.add(*pdf) ;
   }

   delete pdfIter ;

   _coefCache = new Double_t[_pdfList.getSize()] ;
   _coefErrCount = _errorCount ;
   TRACE_CREATE
}




//_____________________________________________________________________________
NewRooAddPdf::NewRooAddPdf(const NewRooAddPdf& other, const char* name) : RooAddPdf(other, name)
{
   // Copy constructor
}


//_____________________________________________________________________________
NewRooAddPdf::~NewRooAddPdf()
{
   // Destructor
}



////////////////////////////////////////////////////////////////////////////////
RooFitResult* NewRooAddPdf::improvedFitTo(RooDataHist& data, const RooCmdArg& arg1, const RooCmdArg& arg2, const RooCmdArg& arg3, const RooCmdArg& arg4,
      const RooCmdArg& arg5, const RooCmdArg& arg6, const RooCmdArg& arg7, const RooCmdArg& arg8,
      const RooCmdArg& arg9, const RooCmdArg& arg10, const RooCmdArg& arg11, const RooCmdArg& arg12)
{
   //RooLinkedList creation
   RooLinkedList l ;
   l.Add((TObject*)&arg1) ;
   l.Add((TObject*)&arg2) ;
   l.Add((TObject*)&arg3) ;
   l.Add((TObject*)&arg4) ;
   l.Add((TObject*)&arg5) ;
   l.Add((TObject*)&arg6) ;
   l.Add((TObject*)&arg7) ;
   l.Add((TObject*)&arg8) ;
   l.Add((TObject*)&arg9) ;
   l.Add((TObject*)&arg10);
   l.Add((TObject*)&arg11);
   l.Add((TObject*)&arg12);

   return improvedFitTo(data, l);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
RooFitResult* NewRooAddPdf::improvedFitTo(RooDataHist& data, const RooLinkedList& cmdList)
{
   // Fit PDF to given dataset. If dataset is unbinned, an unbinned maximum likelihood is performed. If the dataset
   // is binned, a binned maximum likelihood is performed. By default the fit is executed through the MINUIT
   // commands MIGRAD, HESSE and MINOS in succession.
   //
   // Some new commands: SetMaxIter(Int_t), SetMaxCalls(Int_t) and SetEpsilon(Double_t)
   // New commands added in a new "NewRooGlobalFunc" for covenience
   //
   // See RooAbsPdf::improvedFitTo(RooAbsData& data, RooCmdArg arg1, RooCmdArg arg2, RooCmdArg arg3, RooCmdArg arg4,
   //                                         RooCmdArg arg5, RooCmdArg arg6, RooCmdArg arg7, RooCmdArg arg8)
   //
   // for documentation of options


   // Select the pdf-specific commands
   RooCmdConfig pc(Form("RooAbsPdf::improvedFitTo(%s)", GetName())) ;

   RooLinkedList fitCmdList(cmdList) ;
   //Add options here
   RooLinkedList nllCmdList = pc.filterCmdList(fitCmdList, "ProjectedObservables,Extended,Range,RangeWithName,SumCoefRange,NumCPU,SplitRange,Constrained,Constrain,ExternalConstraints,CloneData,GlobalObservables,GlobalObservablesTag,OffsetLikelihood") ;

   pc.defineString("fitOpt", "FitOptions", 0, "") ;
   pc.defineInt("optConst", "Optimize", 0, 2) ;
   pc.defineInt("verbose", "Verbose", 0, 0) ;
   pc.defineInt("doSave", "Save", 0, 0) ;
   pc.defineInt("doTimer", "Timer", 0, 0) ;
   pc.defineInt("plevel", "PrintLevel", 0, 1) ;
   pc.defineInt("strat", "Strategy", 0, 1) ;
   pc.defineInt("initHesse", "InitialHesse", 0, 0) ;
   pc.defineInt("hesse", "Hesse", 0, 1) ;
   pc.defineInt("minos", "Minos", 0, 0) ;
   pc.defineInt("ext", "Extended", 0, 2) ;
   pc.defineInt("numcpu", "NumCPU", 0, 1) ;
   pc.defineInt("numee", "PrintEvalErrors", 0, 10) ;
   pc.defineInt("doEEWall", "EvalErrorWall", 0, 1) ;
   pc.defineInt("doWarn", "Warnings", 0, 1) ;
   pc.defineInt("doSumW2", "SumW2Error", 0, -1) ;
   pc.defineInt("doOffset", "OffsetLikelihood", 0, 0) ;
   pc.defineString("mintype", "Minimizer", 0, "OldMinuit") ;
   pc.defineString("minalg", "Minimizer", 1, "minuit") ;
   pc.defineObject("minosSet", "Minos", 0, 0) ;
   pc.defineSet("cPars", "Constrain", 0, 0) ;
   pc.defineSet("extCons", "ExternalConstraints", 0, 0) ;
   pc.defineMutex("FitOptions", "Verbose") ;
   pc.defineMutex("FitOptions", "Save") ;
   pc.defineMutex("FitOptions", "Timer") ;
   pc.defineMutex("FitOptions", "Strategy") ;
   pc.defineMutex("FitOptions", "InitialHesse") ;
   pc.defineMutex("FitOptions", "Hesse") ;
   pc.defineMutex("FitOptions", "Minos") ;
   pc.defineMutex("Range", "RangeWithName") ;
   pc.defineMutex("InitialHesse", "Minimizer") ;

   // New Commands
   pc.defineInt("numiter", "SetMaxIter", 0, 0);  //Modify number of max iteration
   pc.defineInt("numcalls", "SetMaxCalls", 0, 0); //Modify number of max calls
   pc.defineDouble("eps", "SetEpsilon", 0, 1.0); //Modify tolerance value of the fit (convergence)

   // Process and check varargs
   pc.process(fitCmdList) ;
   if (!pc.ok(kTRUE)) {
      return 0 ;
   }

   // Decode command line arguments
   const char* fitOpt = pc.getString("fitOpt", 0, kTRUE) ;
   Int_t optConst = pc.getInt("optConst") ;
   Int_t verbose  = pc.getInt("verbose") ;
   Int_t doSave   = pc.getInt("doSave") ;
   Int_t doTimer  = pc.getInt("doTimer") ;
   Int_t plevel    = pc.getInt("plevel") ;
   Int_t strat    = pc.getInt("strat") ;
   Int_t initHesse = pc.getInt("initHesse") ;
   Int_t hesse    = pc.getInt("hesse") ;
   Int_t minos    = pc.getInt("minos") ;
   Int_t numee    = pc.getInt("numee") ;
   Int_t doEEWall = pc.getInt("doEEWall") ;
   Int_t doWarn   = pc.getInt("doWarn") ;
   Int_t doSumW2  = pc.getInt("doSumW2") ;
   Int_t numiter  = pc.getInt("numiter") ;
   Int_t numcalls = pc.getInt("numcalls") ;
   Double_t eps   = pc.getDouble("eps");

   const RooArgSet* minosSet = static_cast<RooArgSet*>(pc.getObject("minosSet")) ;


#ifdef __ROOFIT_NOROOMINIMIZER
   const char* minType = 0 ;
#else
   const char* minType = pc.getString("mintype", "OldMinuit") ;
   const char* minAlg = pc.getString("minalg", "minuit") ;
#endif

   // Determine if the dataset has weights
   Bool_t weightedData = data.isNonPoissonWeighted() ;

   // Warn user that a SumW2Error() argument should be provided if weighted data is offered
   if (weightedData && doSumW2 == -1) {
      coutW(InputArguments) << "RooAbsPdf::improvedFitTo(" << GetName() << ") WARNING: a likelihood fit is request of what appears to be weighted data. " << std::endl
                            << "       While the estimated values of the parameters will always be calculated taking the weights into account, " << std::endl
                            << "       there are multiple ways to estimate the errors on these parameter values. You are advised to make an " << std::endl
                            << "       explicit choice on the error calculation: " << std::endl
                            << "           - Either provide SumW2Error(kTRUE), to calculate a sum-of-weights corrected HESSE error matrix " << std::endl
                            << "             (error will be proportional to the number of events)" << std::endl
                            << "           - Or provide SumW2Error(kFALSE), to return errors from original HESSE error matrix" << std::endl
                            << "             (which will be proportional to the sum of the weights)" << std::endl
                            << "       If you want the errors to reflect the information contained in the provided dataset, choose kTRUE. " << std::endl
                            << "       If you want the errors to reflect the precision you would be able to obtain with an unweighted dataset " << std::endl
                            << "       with 'sum-of-weights' events, choose kFALSE." << std::endl ;
   }


   // Warn user that sum-of-weights correction does not apply to MINOS errrors
   if (doSumW2 == 1 && minos) {
      coutW(InputArguments) << "RooAbsPdf::improvedFitTo(" << GetName() << ") WARNING: sum-of-weights correction does not apply to MINOS errors" << std::endl ;
   }

   RooAbsReal* nll = createNLL(data, nllCmdList) ;

   NewRooFitResult* ret = 0 ;

   // Instantiate MINUIT

   if (string(minType) != "OldMinuit") {

#ifndef __ROOFIT_NOROOMINIMIZER
      NewRooMinimizer m(*nll) ;

      m.setMinimizerType(minType) ;

      m.setEvalErrorWall(doEEWall) ;
      if (doWarn == 0) {
         // m.setNoWarn() ; WVE FIX THIS
      }

      //New commands
      if (numiter > 0) m.setMaxIterations(numiter);
      if (numcalls > 0) m.setMaxFunctionCalls(numcalls);
      m.setEps(eps);


      m.setPrintEvalErrors(numee) ;
      if (plevel != 1) {
         m.setPrintLevel(plevel) ;
      }

      if (optConst) {
         // Activate constant term optimization
         m.optimizeConst(optConst) ;
      }

      if (fitOpt) {

         // Play fit options as historically defined
         ret = m.fit(fitOpt) ;

      }
      else {

         if (verbose) {
            // Activate verbose options
            m.setVerbose(1) ;
         }
         if (doTimer) {
            // Activate timer options
            m.setProfile(1) ;
         }

         if (strat != 1) {
            // Modify fit strategy
            m.setStrategy(strat) ;
         }

         if (initHesse) {
            // Initialize errors with hesse
            m.hesse() ;
         }

         // Minimize using chosen algorithm
         m.minimize(minType, minAlg) ;

         if (hesse) {
            // Evaluate errors with Hesse
            m.hesse() ;
         }

         if (doSumW2 == 1 && m.getNPar() > 0) {
            // Make list of RooNLLVar components of FCN
            RooArgSet* comps = nll->getComponents();
            vector<RooNLLVar*> nllComponents;
            nllComponents.reserve(comps->getSize());
            TIterator* citer = comps->createIterator();
            RooAbsArg* arg;
            while ((arg = (RooAbsArg*)citer->Next())) {
               RooNLLVar* nllComp = dynamic_cast<RooNLLVar*>(arg);
               if (!nllComp) continue;
               nllComponents.push_back(nllComp);
            }
            delete citer;
            delete comps;

            // Calculated corrected errors for weighted likelihood fits
            NewRooFitResult* rw = m.save();
            for (vector<RooNLLVar*>::iterator it = nllComponents.begin(); nllComponents.end() != it; ++it) {
               (*it)->applyWeightSquared(kTRUE);
            }
            coutI(Fitting) << "RooAbsPdf::improvedFitTo(" << GetName() << ") Calculating sum-of-weights-squared correction matrix for covariance matrix" << std::endl ;
            m.hesse();
            NewRooFitResult* rw2 = m.save();
            for (vector<RooNLLVar*>::iterator it = nllComponents.begin(); nllComponents.end() != it; ++it) {
               (*it)->applyWeightSquared(kFALSE);
            }

            // Apply correction matrix
            const TMatrixDSym& matV = rw->covarianceMatrix();
            TMatrixDSym matC = rw2->covarianceMatrix();
            using ROOT::Math::CholeskyDecompGenDim;
            CholeskyDecompGenDim<Double_t> decomp(matC.GetNrows(), matC);
            if (!decomp) {
               coutE(Fitting) << "RooAbsPdf::improvedFitTo(" << GetName()
                              << ") ERROR: Cannot apply sum-of-weights correction to covariance matrix: correction matrix calculated with weight-squared is singular" << std::endl ;
            }
            else {
               // replace C by its inverse
               decomp.Invert(matC);
               // the class lies about the matrix being symmetric, so fill in the
               // part above the diagonal
               for (int i = 0; i < matC.GetNrows(); ++i)
                  for (int j = 0; j < i; ++j) matC(j, i) = matC(i, j);
               matC.Similarity(matV);
               // C now contiains V C^-1 V
               // Propagate corrected errors to parameters objects
               m.applyCovarianceMatrix(matC);
            }

            delete rw;
            delete rw2;
         }

         if (minos) {
            // Evaluate errs with Minos
            if (minosSet) {
               m.minos(*minosSet) ;
            }
            else {
               m.minos() ;
            }
         }

         // Optionally return fit result
         if (doSave) {
            string name = Form("fitresult_%s_%s", GetName(), data.GetName()) ;
            string title = Form("Result of fit of p.d.f. %s to dataset %s", GetName(), data.GetName()) ;
            ret = m.save(name.c_str(), title.c_str()) ;
         }

      }
      if (optConst) {
         m.optimizeConst(0) ;
      }

#endif

   }
   else {

      NewRooMinuit m(*nll) ;

      //New Commands
      m.setEps(eps);           //Default eps=1
      m.setMaxIter(numiter);   //Default numiter=5000

      m.setEvalErrorWall(doEEWall) ;
      if (doWarn == 0) {
         m.setNoWarn() ;
      }

      m.setPrintEvalErrors(numee) ;
      if (plevel != 1) {
         m.setPrintLevel(plevel) ;
      }

      if (optConst) {
         // Activate constant term optimization
         m.optimizeConst(optConst) ;
      }

      if (fitOpt) {

         // Play fit options as historically defined
         ret = m.fit(fitOpt) ;

      }
      else {

         if (verbose) {
            // Activate verbose options
            m.setVerbose(1) ;
         }
         if (doTimer) {
            // Activate timer options
            m.setProfile(1) ;
         }

         if (strat != 1) {
            // Modify fit strategy
            m.setStrategy(strat) ;
         }

         if (initHesse) {
            // Initialize errors with hesse
            m.hesse() ;
         }

         // Minimize using migrad
         m.migrad() ;

         if (hesse) {
            // Evaluate errors with Hesse
            m.hesse() ;
         }

         if (doSumW2 == 1 && m.getNPar() > 0) {

            // Make list of RooNLLVar components of FCN
            list<RooNLLVar*> nllComponents ;
            RooArgSet* comps = nll->getComponents() ;
            RooAbsArg* arg ;
            TIterator* citer = comps->createIterator() ;
            while ((arg = (RooAbsArg*)citer->Next())) {
               RooNLLVar* nllComp = dynamic_cast<RooNLLVar*>(arg) ;
               if (nllComp) {
                  nllComponents.push_back(nllComp) ;
               }
            }
            delete citer ;
            delete comps ;

            // Calculated corrected errors for weighted likelihood fits
            NewRooFitResult* rw = m.save() ;
            for (list<RooNLLVar*>::iterator iter1 = nllComponents.begin() ; iter1 != nllComponents.end() ; iter1++) {
               (*iter1)->applyWeightSquared(kTRUE) ;
            }
            coutI(Fitting) << "RooAbsPdf::improvedFitTo(" << GetName() << ") Calculating sum-of-weights-squared correction matrix for covariance matrix" << std::endl ;
            m.hesse() ;
            NewRooFitResult* rw2 = m.save() ;
            for (list<RooNLLVar*>::iterator iter2 = nllComponents.begin() ; iter2 != nllComponents.end() ; iter2++) {
               (*iter2)->applyWeightSquared(kFALSE) ;
            }

            // Apply correction matrix
            const TMatrixDSym& matV = rw->covarianceMatrix();
            TMatrixDSym matC = rw2->covarianceMatrix();
            using ROOT::Math::CholeskyDecompGenDim;
            CholeskyDecompGenDim<Double_t> decomp(matC.GetNrows(), matC);
            if (!decomp) {
               coutE(Fitting) << "RooAbsPdf::improvedFitTo(" << GetName()
                              << ") ERROR: Cannot apply sum-of-weights correction to covariance matrix: correction matrix calculated with weight-squared is singular" << std::endl ;
            }
            else {
               // replace C by its inverse
               decomp.Invert(matC);
               // the class lies about the matrix being symmetric, so fill in the
               // part above the diagonal
               for (int i = 0; i < matC.GetNrows(); ++i)
                  for (int j = 0; j < i; ++j) matC(j, i) = matC(i, j);
               matC.Similarity(matV);
               // C now contiains V C^-1 V
               // Propagate corrected errors to parameters objects
               m.applyCovarianceMatrix(matC);
            }

            delete rw ;
            delete rw2 ;
         }

         if (minos) {
            // Evaluate errs with Minos
            if (minosSet) {
               m.minos(*minosSet) ;
            }
            else {
               m.minos() ;
            }
         }

         // Optionally return fit result
         if (doSave) {
            string name = Form("fitresult_%s_%s", GetName(), data.GetName()) ;
            string title = Form("Result of fit of p.d.f. %s to dataset %s", GetName(), data.GetName()) ;
            ret = m.save(name.c_str(), title.c_str()) ;
         }

      }

      if (optConst) {
         m.optimizeConst(0) ;
      }

   }

   // Cleanup
   delete nll ;
   return ret ;
}



Bool_t NewRooAddPdf::addOwnedComponents(const RooArgSet& comps, Bool_t silent)
{
   // Take ownership of the contents of 'comps'

   if (!_ownedComponents) {
      _ownedComponents = new RooArgSet("owned components") ;
   }

   return _ownedComponents->addOwned(comps, silent) ;
}


//_____________________________________________________________________________
RooAbsReal* NewRooAddPdf::createNLL(RooAbsData& data, const RooCmdArg& arg1, const RooCmdArg& arg2, const RooCmdArg& arg3, const RooCmdArg& arg4,
                                    const RooCmdArg& arg5, const RooCmdArg& arg6, const RooCmdArg& arg7, const RooCmdArg& arg8)
{
   // Construct representation of -log(L) of PDFwith given dataset. If dataset is unbinned, an unbinned likelihood is constructed. If the dataset
   // is binned, a binned likelihood is constructed.
   //
   // The following named arguments are supported
   //
   // ConditionalObservables(const RooArgSet& set) -- Do not normalize PDF over listed observables
   // Extended(Bool_t flag)           -- Add extended likelihood term, off by default
   // Range(const char* name)         -- Fit only data inside range with given name
   // Range(Double_t lo, Double_t hi) -- Fit only data inside given range. A range named "fit" is created on the fly on all observables.
   //                                    Multiple comma separated range names can be specified.
   // SumCoefRange(const char* name)  -- Set the range in which to interpret the coefficients of RooAddPdf components
   // NumCPU(int num, int strat)      -- Parallelize NLL calculation on num CPUs
   //
   //                                    Strategy 0 = RooFit::BulkPartition (Default) --> Divide events in N equal chunks
   //                                    Strategy 1 = RooFit::Interleave --> Process event i%N in process N. Recommended for binned data with
   //                                                 a substantial number of zero-bins, which will be distributed across processes more equitably in this strategy
   //                                    Strategy 2 = RooFit::SimComponents --> Process each component likelihood of a RooSimultaneous fully in a single process
   //                                                 and distribute components over processes. This approach can be benificial if normalization calculation time
   //                                                 dominates the total computation time of a component (since the normalization calculation must be performed
   //                                                 in each process in strategies 0 and 1. However beware that if the RooSimultaneous components do not share many
   //                                                 parameters this strategy is inefficient: as most minuit-induced likelihood calculations involve changing
   //                                                 a single parameter, only 1 of the N processes will be active most of the time if RooSimultaneous components
   //                                                 do not share many parameters
   //                                    Strategy 3 = RooFit::Hybrid --> Follow strategy 0 for all RooSimultaneous components, except those with less than
   //                                                 30 dataset entries, for which strategy 2 is followed.
   //
   // Optimize(Bool_t flag)           -- Activate constant term optimization (on by default)
   // SplitRange(Bool_t flag)         -- Use separate fit ranges in a simultaneous fit. Actual range name for each
   //                                    subsample is assumed to by rangeName_{indexState} where indexState
   //                                    is the state of the master index category of the simultaneous fit
   // Constrain(const RooArgSet&pars) -- For p.d.f.s that contain internal parameter constraint terms, only apply constraints to given subset of parameters
   // ExternalConstraints(const RooArgSet& ) -- Include given external constraints to likelihood
   // GlobalObservables(const RooArgSet&) -- Define the set of normalization observables to be used for the constraint terms.
   //                                        If none are specified the constrained parameters are used
   // GlobalObservablesTag(const char* tagName) -- Define the set of normalization observables to be used for the constraint terms by a string attribute
   //                                              associated with pdf observables that match the given tagName
   // Verbose(Bool_t flag)           -- Constrols RooFit informational messages in likelihood construction
   // CloneData(Bool flag)           -- Use clone of dataset in NLL (default is true)
   // Offset(Bool_t)                  -- Offset likelihood by initial value (so that starting value of FCN in minuit is zero). This
   //                                    can improve numeric stability in simultaneously fits with components with large likelihood values
   //
   //

   RooLinkedList l ;
   l.Add((TObject*)&arg1) ;
   l.Add((TObject*)&arg2) ;
   l.Add((TObject*)&arg3) ;
   l.Add((TObject*)&arg4) ;
   l.Add((TObject*)&arg5) ;
   l.Add((TObject*)&arg6) ;
   l.Add((TObject*)&arg7) ;
   l.Add((TObject*)&arg8) ;
   return createNLL(data, l) ;
}




//_____________________________________________________________________________
RooAbsReal* NewRooAddPdf::createNLL(RooAbsData& data, const RooLinkedList& cmdList)
{
   // Construct representation of -log(L) of PDFwith given dataset. If dataset is unbinned, an unbinned likelihood is constructed. If the dataset
   // is binned, a binned likelihood is constructed.
   //
   // See RooAbsPdf::createNLL(RooAbsData& data, RooCmdArg arg1, RooCmdArg arg2, RooCmdArg arg3, RooCmdArg arg4,
   //                                    RooCmdArg arg5, RooCmdArg arg6, RooCmdArg arg7, RooCmdArg arg8)
   //
   // for documentation of options


   // Select the pdf-specific commands
   RooCmdConfig pc(Form("NewRooAddPdf::createNLL(%s)", GetName())) ;

   pc.defineString("rangeName", "RangeWithName", 0, "", kTRUE) ;
   pc.defineString("addCoefRange", "SumCoefRange", 0, "") ;
   pc.defineString("globstag", "GlobalObservablesTag", 0, "") ;
   pc.defineDouble("rangeLo", "Range", 0, -999.) ;
   pc.defineDouble("rangeHi", "Range", 1, -999.) ;
   pc.defineInt("splitRange", "SplitRange", 0, 0) ;
   pc.defineInt("ext", "Extended", 0, 2) ;
   pc.defineInt("numcpu", "NumCPU", 0, 1) ;
   pc.defineInt("interleave", "NumCPU", 1, 0) ;
   pc.defineInt("verbose", "Verbose", 0, 0) ;
   pc.defineInt("optConst", "Optimize", 0, 0) ;
   pc.defineInt("cloneData", "CloneData", 2, 0) ;
   pc.defineSet("projDepSet", "ProjectedObservables", 0, 0) ;
   pc.defineSet("cPars", "Constrain", 0, 0) ;
   pc.defineSet("glObs", "GlobalObservables", 0, 0) ;
   pc.defineInt("constrAll", "Constrained", 0, 0) ;
   pc.defineInt("doOffset", "OffsetLikelihood", 0, 0) ;
   pc.defineSet("extCons", "ExternalConstraints", 0, 0) ;
   pc.defineMutex("Range", "RangeWithName") ;
   pc.defineMutex("Constrain", "Constrained") ;
   pc.defineMutex("GlobalObservables", "GlobalObservablesTag") ;

   // Process and check varargs
   pc.process(cmdList) ;
   if (!pc.ok(kTRUE)) {
      return 0 ;
   }

   // Decode command line arguments
   const char* rangeName = pc.getString("rangeName", 0, kTRUE) ;
   const char* addCoefRangeName = pc.getString("addCoefRange", 0, kTRUE) ;
   const char* globsTag = pc.getString("globstag", 0, kTRUE) ;
   Int_t ext      = pc.getInt("ext") ;
   Int_t numcpu   = pc.getInt("numcpu") ;
   RooFit::MPSplit interl = (RooFit::MPSplit) pc.getInt("interleave") ;

   Int_t splitr   = pc.getInt("splitRange") ;
   Bool_t verbose = pc.getInt("verbose") ;
   Int_t optConst = pc.getInt("optConst") ;
   Int_t cloneData = pc.getInt("cloneData") ;
   Int_t doOffset = pc.getInt("doOffset") ;

   // If no explicit cloneData command is specified, cloneData is set to true if optimization is activated
   if (cloneData == 2) {
      cloneData = optConst ;
   }


   RooArgSet* cPars = pc.getSet("cPars") ;
   RooArgSet* glObs = pc.getSet("glObs") ;
   if (pc.hasProcessed("GlobalObservablesTag")) {
      if (glObs) delete glObs ;
      RooArgSet* allVars = getVariables() ;
      glObs = (RooArgSet*) allVars->selectByAttrib(globsTag, kTRUE) ;
      coutI(Minimization) << "User-defined specification of global observables definition with tag named '" <<  globsTag << "'" << std::endl ;
      delete allVars ;
   }
   else if (!pc.hasProcessed("GlobalObservables")) {

      // Neither GlobalObservables nor GlobalObservablesTag has been processed - try if a default tag is defined in the head node
      // Check if head not specifies default global observable tag
      const char* defGlobObsTag = getStringAttribute("DefaultGlobalObservablesTag") ;
      if (defGlobObsTag) {
         coutI(Minimization) << "p.d.f. provides built-in specification of global observables definition with tag named '" <<  defGlobObsTag << "'" << std::endl ;
         if (glObs) delete glObs ;
         RooArgSet* allVars = getVariables() ;
         glObs = (RooArgSet*) allVars->selectByAttrib(defGlobObsTag, kTRUE) ;
      }
   }


   Bool_t doStripDisconnected = kFALSE ;

   // If no explicit list of parameters to be constrained is specified apply default algorithm
   // All terms of RooProdPdfs that do not contain observables and share a parameters with one or more
   // terms that do contain observables are added as constraints.
   if (!cPars) {
      cPars = getParameters(data, kFALSE) ;
      doStripDisconnected = kTRUE ;
   }
   const RooArgSet* extCons = pc.getSet("extCons") ;

   // Process automatic extended option
   if (ext == 2) {
      ext = ((extendMode() == CanBeExtended || extendMode() == MustBeExtended)) ? 1 : 0 ;
      if (ext) {
         coutI(Minimization) << "p.d.f. provides expected number of events, including extended term in likelihood." << std::endl ;
      }
   }

   if (pc.hasProcessed("Range")) {
      Double_t rangeLo = pc.getDouble("rangeLo") ;
      Double_t rangeHi = pc.getDouble("rangeHi") ;

      // Create range with name 'fit' with above limits on all observables
      RooArgSet* obs = getObservables(&data) ;
      TIterator* iter = obs->createIterator() ;
      RooAbsArg* arg ;
      while ((arg = (RooAbsArg*)iter->Next())) {
         RooRealVar* rrv =  dynamic_cast<RooRealVar*>(arg) ;
         if (rrv) rrv->setRange("fit", rangeLo, rangeHi) ;
      }
      // Set range name to be fitted to "fit"
      rangeName = "fit" ;
   }

   RooArgSet projDeps ;
   RooArgSet* tmp = pc.getSet("projDepSet") ;
   if (tmp) {

      projDeps.add(*tmp) ;
   }

   // Construct NLL
   RooAbsReal::setEvalErrorLoggingMode(RooAbsReal::CollectErrors) ;
   RooAbsReal* nll ;
   string baseName = Form("nll_%s_%s", GetName(), data.GetName()) ;
   if (!rangeName || strchr(rangeName, ',') == 0) {
      // Simple case: default range, or single restricted range
      //cout<<"FK: Data test 1: "<<data.sumEntries()<<std::endl;

      nll = new RooNLLVar(baseName.c_str(), "-log(likelihood)", *this, data, projDeps, ext, rangeName, addCoefRangeName, numcpu, interl, verbose, splitr, cloneData) ;

   }
   else {
      // Composite case: multiple ranges
      RooArgList nllList ;
      const size_t bufSize = strlen(rangeName) + 1;
      char* buf = new char[bufSize] ;
      strlcpy(buf, rangeName, bufSize) ;
      char* token = strtok(buf, ",") ;
      while (token) {
         RooAbsReal* nllComp = new RooNLLVar(Form("%s_%s", baseName.c_str(), token), "-log(likelihood)", *this, data, projDeps, ext, token, addCoefRangeName, numcpu, interl, verbose, splitr, cloneData) ;

         nllList.add(*nllComp) ;
         token = strtok(0, ",") ;
      }
      delete[] buf ;
      nll = new RooAddition(baseName.c_str(), "-log(likelihood)", nllList, kTRUE) ;
   }
   RooAbsReal::setEvalErrorLoggingMode(RooAbsReal::PrintErrors) ;

   // Collect internal and external constraint specifications
   RooArgSet allConstraints ;
   if (cPars && cPars->getSize() > 0) {
      RooArgSet* constraints = getAllConstraints(*data.get(), *cPars, doStripDisconnected) ;

      allConstraints.add(*constraints, kTRUE) ;

      delete constraints ;

   }
   if (extCons) {

      allConstraints.add(*extCons, kTRUE) ;

   }

   // Include constraints, if any, in likelihood
   RooAbsReal* nllCons(0) ;
   if (allConstraints.getSize() > 0 && cPars) {

      coutI(Minimization) << " Including the following contraint terms in minimization: " << allConstraints << std::endl ;
      if (glObs) {
         coutI(Minimization) << "The following global observables have been defined: " << *glObs << std::endl ;
      }

      nllCons = new RooConstraintSum(Form("%s_constr", baseName.c_str()), "nllCons", allConstraints, glObs ? *glObs : *cPars) ;

      nllCons->setOperMode(ADirty) ;
      RooAbsReal* orignll = nll ;

      nll = new RooAddition(Form("%s_with_constr", baseName.c_str()), "nllWithCons", RooArgSet(*nll, *nllCons)) ;

      nll->addOwnedComponents(RooArgSet(*orignll, *nllCons)) ;
   }

   //debug
   //printf("LAST OPERATION CREATE NLL\n");

   if (optConst) {

      nll->constOptimizeTestStatistic(RooAbsArg::Activate, optConst > 1) ;
   }

   if (doStripDisconnected) {

      delete cPars ;
   }

   if (doOffset) {

      //nll->enableSilentOffsetting(kTRUE) ;   //Modified for no checkForDup() messages
   }

   return nll ;
}
