//Created by KVClassFactory on Wed Jun 19 16:07:04 2013
//Author: dgruyer

#include "KVGaxis.h"
#include "TMath.h"

#include "Riostream.h"
#include "TROOT.h"
#include "TGaxis.h"
#include "TVirtualPad.h"
//#include "TPad.h"
#include "TVirtualX.h"
#include "TLine.h"
#include "TLatex.h"
#include "TStyle.h"
#include "TF1.h"
#include "TAxis.h"
#include "THashList.h"
#include "TObjString.h"
#include "TMath.h"
#include "THLimitsFinder.h"
#include "TColor.h"
#include "TClass.h"
#include "TTimeStamp.h"
#include "TSystem.h"

Int_t TGaxis::fgMaxDigits = 5;
const Int_t kHori = BIT(9);

ClassImp(KVGaxis)

////////////////////////////////////////////////////////////////////////////////
// BEGIN_HTML <!--
/* -->
<h2>KVGaxis</h2>
<h4>TAxis optimised for KVGraph visualisation</h4>
<!-- */
// --> END_HTML
////////////////////////////////////////////////////////////////////////////////

KVGaxis::KVGaxis() : TGaxis()
{
   fNbins = 0;
   fBins  = 0;
   fLabels = 0;
   fFormat = "%lf";
   // Default constructor
}

//________________________________________________________________

//KVGaxis::KVGaxis(const KVGaxis& obj) : TGaxis()
//{
//   // Copy constructor
//   // This ctor is used to make a copy of an existing object (for example
//   // when a method returns an object), and it is always a good idea to
//   // implement it.
//   // If your class allocates memory in its constructor(s) then it is ESSENTIAL :-)

//   obj.Copy(*this);
//}

//________________________________________________________________

KVGaxis::KVGaxis(Double_t xmin, Double_t ymin, Double_t xmax, Double_t ymax, Double_t wmin, Double_t wmax, Int_t ndiv, Option_t* chopt, Double_t gridlength) :
   TGaxis(xmin, ymin, xmax, ymax, wmin, wmax, ndiv, chopt, gridlength)
{
   fNbins = 0;
   fBins  = 0;
   fLabels = 0;
   fFormat = "%lf";
   // Write your code here
}

//________________________________________________________________

KVGaxis::KVGaxis(Double_t xmin, Double_t ymin, Double_t xmax, Double_t ymax, const char* funcname, Int_t ndiv, Option_t* chopt, Double_t gridlength) :
   TGaxis(xmin, ymin, xmax, ymax, funcname, ndiv, chopt, gridlength)
{
   fNbins = 0;
   fBins  = 0;
   fLabels = 0;
   fFormat = "%lf";
   // Write your code here
}

KVGaxis::KVGaxis(Double_t xmin, Double_t ymin, Double_t xmax, Double_t ymax, Int_t nbins, Double_t* bins, Double_t wmin, Double_t wmax, Int_t ndiv, Option_t* chopt, Double_t gridlength, char** labels):
   TGaxis(xmin, ymin, xmax, ymax, (wmin == wmax ? (xmin == xmax ? ymin : xmin) : wmin), (wmin == wmax ? (xmin == xmax ? ymax : xmax) : wmax), ndiv, chopt, gridlength)
{
   fNbins = nbins;
   fBins  = bins;
   fNdiv  = fNbins;
   fFormat = "%lf";
   fLabels = labels;
}


KVGaxis::~KVGaxis()
{
   // Destructor
}

//________________________________________________________________

//void KVGaxis::Copy(TObject& obj) const
//{
//   // This method copies the current state of 'this' object into 'obj'
//   // You should add here any member variables, for example:
//   //    (supposing a member variable KVGaxis::fToto)
//   //    CastedObj.fToto = fToto;
//   // or
//   //    CastedObj.SetToto( GetToto() );

//   TGaxis::Copy(obj);
//   //KVGaxis& CastedObj = (KVGaxis&)obj;
//}


void KVGaxis::PaintAxis(Double_t xmin, Double_t ymin, Double_t xmax, Double_t ymax,
                        Double_t& wmin, Double_t& wmax, Int_t& ndiv,   Option_t* chopt,
                        Double_t gridlength, Bool_t drawGridOnly)
{
   const char* where = "PaintAxis";

   TString opt(chopt);
   if (!fNbins) return TGaxis::PaintAxis(xmin, ymin, xmax, ymax, wmin, wmax, ndiv, chopt, gridlength, drawGridOnly);

   Double_t alfa, beta, ratio1, ratio2, grid_side;
   Double_t axis_lengthN = 0;
   Double_t axis_length0 = 0;
   Double_t axis_length1 = 0;
   Double_t axis_length;
   Double_t atick[3];
   Double_t tick_side;
   Double_t charheight;
   Double_t phil, phi, sinphi, cosphi, asinphi, acosphi;
   Double_t binLow = 0.,  binLow2 = 0.,  binLow3 = 0.;
   Double_t binHigh = 0., binHigh2 = 0., binHigh3 = 0.;
   Double_t binWidth = 0., binWidth2 = 0., binWidth3 = 0.;
   Double_t xpl1, xpl2, ypl1, ypl2;
   Double_t xtick = 0;
   Double_t xtick0, xtick1, dxtick = 0;
   Double_t ytick, ytick0, ytick1;
   Double_t wlabel, dwlabel;
   Double_t xfactor, yfactor;
   Double_t xlabel, ylabel, dxlabel;
   Double_t xone, xtwo;
   Double_t rlab;
   Double_t x0, x1, y0, y1, xx0, xx1, yy0, yy1;
   xx0 = xx1 = yy0 = yy1 = 0;
   Double_t xxmin, xxmax, yymin, yymax;
   xxmin = xxmax = yymin = yymax = 0;
   Double_t xlside, xmside;
   Double_t ww, af, rne;
   Double_t xx, yy;
   Double_t xmnlog, x00, x11, h2, h2sav, axmul, y;
   Float_t chupxvsav, chupyvsav;
   Double_t rtxw, rtyw;
   Int_t nlabels, nticks, nticks0, nticks1;
   Int_t i, j, k, l, decade, ltick;
   Int_t mside, lside;
   Int_t nexe  = 0;
   Int_t lnlen = 0;
   Int_t iexe, if1, if2, na, nf, ih1, ih2, nbinin, nch, kmod;
   Int_t optionLog, optionBlank, optionVert, optionPlus, optionMinus, optionUnlab, optionPara;
   Int_t optionDown, optionRight, optionLeft, optionCent, optionEqual, optionDecimals = 0, optionDot;
   Int_t optionY, optionText, optionGrid, optionSize, optionNoopt, optionInt, optionM, optionUp, optionX;
   Int_t optionTime;
   Int_t first, last, labelnumber;
   Int_t xalign, yalign;
   Int_t nn1, nn2, nn3, n1a, n2a, n3a, nb2, nb3;
   Int_t nbins = 10, n1aold, nn1old;
   n1aold = nn1old = 0;
   Int_t ndyn;
   Int_t nhilab = 0;
   Int_t idn;
   Bool_t flexe = 0;
   Bool_t flexpo, flexne;
   char* label;
   char* chtemp;
   char* coded;
   char chlabel[256];
   char kchtemp[256];
   char chcoded[8];
   TLine* linegrid;
   TString timeformat;
   time_t timelabel;
   Double_t timed, wTimeIni;
   struct tm* utctis;
   Double_t rangeOffset = 0;

   Double_t epsilon   = 1e-5;
   const Double_t kPI = TMath::Pi();
   //*-*-______________________________________

   Double_t rwmi = wmin;
   Double_t rwma = wmax;
   chtemp = &kchtemp[0];
   label  = &chlabel[0];
   linegrid  = 0;

   fFunction = (TF1*)gROOT->GetFunction(fFunctionName.Data());

   Bool_t noExponent = TestBit(TAxis::kNoExponent);

   //*-*- If moreLogLabels = kTRUE more Log Intermediate Labels are drawn.
   Bool_t moreLogLabels = TestBit(TAxis::kMoreLogLabels);

   //*-*- the following parameters correspond to the pad range in NDC
   //*-*- and the WC coordinates in the pad

   Double_t padh   = gPad->GetWh() * gPad->GetAbsHNDC();
   Double_t rwxmin = gPad->GetX1();
   Double_t rwxmax = gPad->GetX2();
   Double_t rwymin = gPad->GetY1();
   Double_t rwymax = gPad->GetY2();

   if (strchr(chopt, 'G')) optionLog  = 1;
   else optionLog  = 0;
   if (strchr(chopt, 'B')) optionBlank = 1;
   else optionBlank = 0;
   if (strchr(chopt, 'V')) optionVert = 1;
   else optionVert = 0;
   if (strchr(chopt, '+')) optionPlus = 1;
   else optionPlus = 0;
   if (strchr(chopt, '-')) optionMinus = 1;
   else optionMinus = 0;
   if (strchr(chopt, 'U')) optionUnlab = 1;
   else optionUnlab = 0;
   if (strchr(chopt, 'P')) optionPara = 1;
   else optionPara = 0;
   if (strchr(chopt, 'O')) optionDown = 1;
   else optionDown = 0;
   if (strchr(chopt, 'R')) optionRight = 1;
   else optionRight = 0;
   if (strchr(chopt, 'L')) optionLeft = 1;
   else optionLeft = 0;
   if (strchr(chopt, 'C')) optionCent = 1;
   else optionCent = 0;
   if (strchr(chopt, '=')) optionEqual = 1;
   else optionEqual = 0;
   if (strchr(chopt, 'Y')) optionY    = 1;
   else optionY    = 0;
   if (strchr(chopt, 'T')) optionText = 1;
   else optionText = 0;
   if (strchr(chopt, 'W')) optionGrid = 1;
   else optionGrid = 0;
   if (strchr(chopt, 'S')) optionSize = 1;
   else optionSize = 0;
   if (strchr(chopt, 'N')) optionNoopt = 1;
   else optionNoopt = 0;
   if (strchr(chopt, 'I')) optionInt  = 1;
   else optionInt  = 0;
   if (strchr(chopt, 'M')) optionM    = 1;
   else optionM    = 0;
   if (strchr(chopt, '0')) optionUp   = 1;
   else optionUp   = 0;
   if (strchr(chopt, 'X')) optionX    = 1;
   else optionX    = 0;
   if (strchr(chopt, 't')) optionTime = 1;
   else optionTime = 0;
   if (strchr(chopt, '.')) optionDot  = 1;
   else optionDot  = 0;
   if (TestBit(TAxis::kTickPlus))     optionPlus  = 2;
   if (TestBit(TAxis::kTickMinus))    optionMinus = 2;
   if (TestBit(TAxis::kCenterLabels)) optionM     = 1;
   if (TestBit(TAxis::kDecimals))     optionDecimals = 1;
   if (!gStyle->GetStripDecimals())   optionDecimals = 1;
   if (fAxis) {
      if (fAxis->GetLabels()) {
         optionM    = 1;
         optionText = 1;
         ndiv = fAxis->GetLast() - fAxis->GetFirst() + 1;
      }
   }
   if (ndiv < 0) {
      Error(where, "Invalid number of divisions: %d", ndiv);
      return;
   }

   if (fNbins) {
      optionNoopt = kTRUE;
      optionSize = 1;
   }

   //*-*-              Set the grid length

   if (optionGrid) {
      if (gridlength == 0) gridlength = 0.8;
      linegrid = new TLine();
      linegrid->SetLineColor(gStyle->GetGridColor());
      if (linegrid->GetLineColor() == 0) linegrid->SetLineColor(GetLineColor());
      linegrid->SetLineStyle(gStyle->GetGridStyle());
      linegrid->SetLineWidth(gStyle->GetGridWidth());
   }

   //*-*-              No labels if the axis label offset is big.
   //*-*-              In that case the labels are not visible anyway.

   if (GetLabelOffset() > 1.1) optionUnlab = 1;

   //*-*-              Determine time format

   Int_t idF = fTimeFormat.Index("%F");
   if (idF >= 0) {
      timeformat = fTimeFormat(0, idF);
   } else {
      timeformat = fTimeFormat;
   }

   //GMT option
   if (fTimeFormat.Index("GMT") >= 0) optionTime = 2;

   // Determine the time offset and correct for time offset not being integer.
   Double_t timeoffset = 0;
   if (optionTime) {
      if (idF >= 0) {
         Int_t lnF = fTimeFormat.Length();
         TString stringtimeoffset = fTimeFormat(idF + 2, lnF);
         Int_t year, mm, dd, hh, mi, ss;
         if (sscanf(stringtimeoffset.Data(), "%d-%d-%d %d:%d:%d", &year, &mm, &dd, &hh, &mi, &ss) == 6) {
            struct tm tp;
            tp.tm_year   = year - 1900;
            tp.tm_mon    = mm - 1;
            tp.tm_mday   = dd;
            tp.tm_hour   = hh;
            tp.tm_min    = mi;
            tp.tm_sec    = ss;
            tp.tm_isdst  = -1; //automatic determination of daylight saving time
            TString tz   = (TString)gSystem->Getenv("TZ"); //save timezone
            Bool_t isUTC = kFALSE;
            if (gSystem->Getenv("TZ") && tz.Length() == 0) isUTC = kTRUE;
            gSystem->Setenv("TZ", "UTC"); //sets timezone to UTC
            tzset();
            timeoffset  = mktime(&tp);
            //restore TZ
            if (tz.Length()) {
               gSystem->Setenv("TZ", tz.Data());
            } else {
               if (isUTC) gSystem->Setenv("TZ", "");
               else       gSystem->Unsetenv("TZ");
            }
            tzset();
            // Add the time offset's decimal part if it is there
            Int_t ids   = stringtimeoffset.Index("s");
            if (ids >= 0) {
               Float_t dp;
               Int_t lns   = stringtimeoffset.Length();
               TString sdp = stringtimeoffset(ids + 1, lns);
               sscanf(sdp.Data(), "%g", &dp);
               timeoffset += dp;
            }
         } else {
            Error(where, "Time offset has not the right format");
         }
      } else {
         timeoffset = gStyle->GetTimeOffset();
      }
      wmin += timeoffset - (int)(timeoffset);
      wmax += timeoffset - (int)(timeoffset);

      // correct for time offset at a good limit (min, hour, day, month, year)
      struct tm* tp0;
      time_t timetp = (time_t)((Long_t)(timeoffset));
      Double_t range = wmax - wmin;
      Long_t rangeBase = 60;
      if (range > 60)       rangeBase = 60 * 20;   // minutes
      if (range > 3600)     rangeBase = 3600 * 20; // hours
      if (range > 86400)    rangeBase = 86400 * 20; // days
      if (range > 2419200)  rangeBase = 31556736;  // months (average # days)
      rangeOffset = (Double_t)((Long_t)(timeoffset) % rangeBase);
      if (range > 31536000) {
         tp0 = gmtime(&timetp);
         tp0->tm_mon   = 0;
         tp0->tm_mday  = 1;
         tp0->tm_hour  = 0;
         tp0->tm_min   = 0;
         tp0->tm_sec   = 0;
         tp0->tm_isdst = 1; // daylight saving time is on.
         rangeBase = (timetp - mktime(tp0)); // years
         rangeOffset = (Double_t)(rangeBase);
      }
      wmax += rangeOffset;
      wmin += rangeOffset;
   }

   //*-*-              Determine number of divisions 1, 2 and 3
   n1a   = ndiv % 100;
   n2a   = (ndiv % 10000 - n1a) / 100;
   n3a   = ndiv / 10000;
   nn3   = TMath::Max(n3a, 1);
   nn2   = TMath::Max(n2a, 1) * nn3;
   nn1   = TMath::Max(n1a, 1) * nn2 + 1;
   nticks = nn1;

   nn3 = 1;
   nn2 = 1;
   nn1 = fNbins;
   nticks = nn1;


   //*-*-              Axis bining optimization is ignored if:
   //*-*-                - the first and the last label are equal
   //*-*-                - the number of divisions is 0
   //*-*-                - less than 1 primary division is requested
   //*-*-                - logarithmic scale is requested

   if (wmin == wmax || ndiv == 0 || n1a <= 1 || optionLog) {
      optionNoopt = 1;
      optionInt   = 0;
   }

   //*-*-              Axis bining optimization
   if ((wmax - wmin) < 1 && optionInt) {
      Error(where, "option I not available");
      optionInt = 0;
   }
   if (!optionNoopt || optionInt) {

      //*-*- Primary divisions optimization
      //*-*- When integer labelling is required, Optimize is invoked first
      //*-*- and only if the result is not an integer labelling, AdjustBinSize is invoked.

      THLimitsFinder::Optimize(wmin, wmax, n1a, binLow, binHigh, nbins, binWidth, fChopt.Data());
      if (optionInt) {
         if (binLow != Double_t(int(binLow)) || binWidth != Double_t(int(binWidth))) {
            AdjustBinSize(wmin, wmax, n1a, binLow, binHigh, nbins, binWidth);
         }
      }
      if ((wmin - binLow)  > epsilon) {
         binLow  += binWidth;
         nbins--;
      }
      if ((binHigh - wmax) > epsilon) {
         binHigh -= binWidth;
         nbins--;
      }
      if (xmax == xmin) {
         rtyw  = (ymax - ymin) / (wmax - wmin);
         xxmin = xmin;
         xxmax = xmax;
         yymin = rtyw * (binLow - wmin)  + ymin;
         yymax = rtyw * (binHigh - wmin) + ymin;
      } else {
         rtxw  = (xmax - xmin) / (wmax - wmin);
         xxmin = rtxw * (binLow - wmin)  + xmin;
         xxmax = rtxw * (binHigh - wmin) + xmin;
         if (ymax == ymin) {
            yymin = ymin;
            yymax = ymax;
         } else {
            alfa  = (ymax - ymin) / (xmax - xmin);
            beta  = (ymin * xmax - ymax * xmin) / (xmax - xmin);
            yymin = alfa * xxmin + beta;
            yymax = alfa * xxmax + beta;
         }
      }
      if (fFunction) {
         yymin = ymin;
         yymax = ymax;
         xxmin = xmin;
         xxmax = xmax;
      } else {
         wmin = binLow;
         wmax = binHigh;
      }

      //*-*- Secondary divisions optimization
      nb2 = n2a;
      if (!optionNoopt && n2a > 1 && binWidth > 0) {
         THLimitsFinder::Optimize(wmin, wmin + binWidth, n2a, binLow2, binHigh2, nb2, binWidth2, fChopt.Data());
      }

      //*-*- Tertiary divisions optimization
      nb3 = n3a;
      if (!optionNoopt && n3a > 1 && binWidth2 > 0) {
         THLimitsFinder::Optimize(binLow2, binLow2 + binWidth2, n3a, binLow3, binHigh3, nb3, binWidth3, fChopt.Data());
      }
      n1aold = n1a;
      nn1old = nn1;
      n1a    = nbins;
      nn3    = TMath::Max(nb3, 1);
      nn2    = TMath::Max(nb2, 1) * nn3;
      nn1    = TMath::Max(n1a, 1) * nn2 + 1;
      nticks = nn1;
   }

   //*-*-              Coordinates are normalized

   ratio1 = 1 / (rwxmax - rwxmin);
   ratio2 = 1 / (rwymax - rwymin);
   x0     = ratio1 * (xmin - rwxmin);
   x1     = ratio1 * (xmax - rwxmin);
   y0     = ratio2 * (ymin - rwymin);
   y1     = ratio2 * (ymax - rwymin);
   if (!optionNoopt || optionInt) {
      xx0 = ratio1 * (xxmin - rwxmin);
      xx1 = ratio1 * (xxmax - rwxmin);
      yy0 = ratio2 * (yymin - rwymin);
      yy1 = ratio2 * (yymax - rwymin);
   }

   if ((x0 == x1) && (y0 == y1)) {
      Error(where, "length of axis is 0");
      return;
   }

   //*-*-              Return wmin, wmax and the number of primary divisions
   if (optionX) {
      ndiv = n1a;
      return;
   }

   Int_t maxDigits = 5;
   if (fAxis) maxDigits = fgMaxDigits;

   TLatex* textaxis = new TLatex();
   SetLineStyle(1); // axis line style
   textaxis->SetTextColor(GetTextColor());
   textaxis->SetTextFont(GetTextFont());

   if (!gPad->IsBatch()) {
      gVirtualX->GetCharacterUp(chupxvsav, chupyvsav);
      gVirtualX->SetClipOFF(gPad->GetCanvasID());
   }

   //*-*-              Compute length of axis
   axis_length = TMath::Sqrt((x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0));
   if (axis_length == 0) {
      Error(where, "length of axis is 0");
      goto L210;
   }
   if (!optionNoopt || optionInt) {
      axis_lengthN = TMath::Sqrt((xx1 - xx0) * (xx1 - xx0) + (yy1 - yy0) * (yy1 - yy0));
      axis_length0 = TMath::Sqrt((xx0 - x0) * (xx0 - x0) + (yy0 - y0) * (yy0 - y0));
      axis_length1 = TMath::Sqrt((x1 - xx1) * (x1 - xx1) + (y1 - yy1) * (y1 - yy1));
      if (axis_lengthN < epsilon) {
         optionNoopt = 1;
         optionInt   = 0;
         wmin        = rwmi;
         wmax        = rwma;
         n1a         = n1aold;
         nn1         = nn1old;
         nticks      = nn1;
         if (optionTime) {
            wmin        += timeoffset - (int)(timeoffset) + rangeOffset;
            wmax        += timeoffset - (int)(timeoffset) + rangeOffset;
         }
      }
   }

   if (x0 == x1) {
      phi  = 0.5 * kPI;
      phil = phi;
   } else {
      phi = TMath::ATan2((y1 - y0), (x1 - x0));
      Int_t px0 = gPad->UtoPixel(x0);
      Int_t py0 = gPad->VtoPixel(y0);
      Int_t px1 = gPad->UtoPixel(x1);
      Int_t py1 = gPad->VtoPixel(y1);
      if (x0 < x1) phil = TMath::ATan2(Double_t(py0 - py1), Double_t(px1 - px0));
      else         phil = TMath::ATan2(Double_t(py1 - py0), Double_t(px0 - px1));
   }
   cosphi  = TMath::Cos(phi);
   sinphi  = TMath::Sin(phi);
   acosphi = TMath::Abs(cosphi);
   asinphi = TMath::Abs(sinphi);
   if (acosphi <= epsilon) {
      acosphi = 0;
      cosphi  = 0;
   }
   if (asinphi <= epsilon) {
      asinphi = 0;
      sinphi  = 0;
   }

   //*-*-              mside positive, tick marks on positive side
   //*-*-              mside negative, tick marks on negative side
   //*-*-              mside zero, tick marks on both sides
   //*-*-              Default is positive except for vertical axis

   mside = 1;
   if (x0 == x1 && y1 > y0)       mside = -1;
   if (optionPlus)                mside = 1;
   if (optionMinus)               mside = -1;
   if (optionPlus && optionMinus) mside = 0;
   xmside = mside;
   lside = -mside;
   if (optionEqual) lside = mside;
   if (optionPlus && optionMinus) {
      lside = -1;
      if (optionEqual) lside = 1;
   }
   xlside = lside;

   //*-*-              Tick marks size
   if (xmside >= 0) tick_side = 1;
   else            tick_side = -1;
   if (optionSize) atick[0] = tick_side * axis_length * fTickSize;
   else            atick[0] = tick_side * axis_length * 0.03;

   atick[1] = 0.5 * atick[0];
   atick[2] = 0.5 * atick[1];

   //*-*-             Set the side of the grid
   if ((x0 == x1) && (y1 > y0))  grid_side = -1;
   else                          grid_side = 1;

   //*-*-             Compute Values if Function is given
   if (fFunction) {
      rwmi = fFunction->Eval(wmin);
      rwma = fFunction->Eval(wmax);
      if (rwmi > rwma) {
         Double_t t = rwma;
         rwma = rwmi;
         rwmi = t;
      }
   }

   //*-*-              Draw the axis if needed...
   if (!optionBlank) {
      xpl1 = x0;
      xpl2 = x1;
      ypl1 = y0;
      ypl2 = y1;
      PaintLineNDC(xpl1, ypl1, xpl2, ypl2);
   }

   //*-*-              Draw axis title if it exists
   if (!drawGridOnly && strlen(GetTitle())) {
      textaxis->SetTextSize(GetTitleSize());
      charheight = GetTitleSize();
      if ((GetTextFont() % 10) > 2) {
         charheight = charheight / gPad->GetWh();
      }
      Double_t toffset = GetTitleOffset();
      //////if (toffset < 0.1) toffset = 1; // Negative offset should be allowed
      if (x1 == x0) ylabel = xlside * 1.6 * charheight * toffset;
      else          ylabel = xlside * 1.3 * charheight * toffset;
      if (y1 == y0) ylabel = xlside * 1.6 * charheight * toffset;
      Double_t axispos;
      if (TestBit(TAxis::kCenterTitle)) axispos = 0.5 * axis_length;
      else                       axispos = axis_length;
      if (TestBit(TAxis::kRotateTitle)) {
         if (x1 >= x0) {
            if (TestBit(TAxis::kCenterTitle)) textaxis->SetTextAlign(22);
            else                              textaxis->SetTextAlign(12);
            Rotate(axispos, ylabel, cosphi, sinphi, x0, y0, xpl1, ypl1);
         } else {
            if (TestBit(TAxis::kCenterTitle)) textaxis->SetTextAlign(22);
            else                                 textaxis->SetTextAlign(32);
            Rotate(axispos, ylabel, cosphi, sinphi, x0, y0, xpl1, ypl1);
         }
         textaxis->PaintLatex(gPad->GetX1() + xpl1 * (gPad->GetX2() - gPad->GetX1()),
                              gPad->GetY1() + ypl1 * (gPad->GetY2() - gPad->GetY1()),
                              phil = (kPI + phil) * 180 / kPI,
                              GetTitleSize(),
                              GetTitle());
      } else {
         if (x1 >= x0) {
            if (TestBit(TAxis::kCenterTitle)) textaxis->SetTextAlign(22);
            else                              textaxis->SetTextAlign(32);
            Rotate(axispos, ylabel, cosphi, sinphi, x0, y0, xpl1, ypl1);
         } else {
            if (TestBit(TAxis::kCenterTitle)) textaxis->SetTextAlign(22);
            else                                 textaxis->SetTextAlign(12);
            Rotate(axispos, ylabel, cosphi, sinphi, x0, y0, xpl1, ypl1);
         }
         textaxis->PaintLatex(gPad->GetX1() + xpl1 * (gPad->GetX2() - gPad->GetX1()),
                              gPad->GetY1() + ypl1 * (gPad->GetY2() - gPad->GetY1()),
                              phil * 180 / kPI,
                              GetTitleSize(),
                              GetTitle());
      }
   }

   //*-*-              No bining

   if (ndiv == 0)goto L210;
   if (wmin == wmax) {
      Error(where, "wmin (%f) == wmax (%f)", wmin, wmax);
      goto L210;
   }

   //*-*-              Labels preparation:
   //*-*-              Get character height
   //*-*-              Compute the labels orientation in case of overlaps
   //*-*-              (with alphanumeric labels for horizontal axis).

   charheight = GetLabelSize();
   if (optionText && GetLabelFont() % 10 != 3) charheight *= 0.66666;
   textaxis->SetTextFont(GetLabelFont());
   if ((GetLabelFont() % 10 < 2) && optionLog) // force TLatex mode in PaintLatex
      textaxis->SetTextFont((Int_t)(GetLabelFont() / 10) * 10 + 2);
   textaxis->SetTextColor(GetLabelColor());
   textaxis->SetTextSize(charheight);
   textaxis->SetTextAngle(GetTextAngle());
   if (GetLabelFont() % 10 > 2) {
      charheight /= padh;
   }
   if (!optionUp && !optionDown && !optionY && !optionUnlab) {
      if (!drawGridOnly && optionText && ((ymin == ymax) || (xmin == xmax))) {
         textaxis->SetTextAlign(32);
         optionText = 2;
         Int_t nl = fAxis->GetLast() - fAxis->GetFirst() + 1;
         Double_t angle     = 0;
         for (i = fAxis->GetFirst(); i <= fAxis->GetLast(); i++) {
            textaxis->SetText(0, 0, fAxis->GetBinLabel(i));
            if (textaxis->GetXsize() < (xmax - xmin) / nl) continue;
            angle = -20;
            break;
         }
         for (i = fAxis->GetFirst(); i <= fAxis->GetLast(); i++) {
            if ((!strcmp(fAxis->GetName(), "xaxis") && !gPad->TestBit(kHori))
                  || (!strcmp(fAxis->GetName(), "yaxis") &&  gPad->TestBit(kHori))) {
               if (nl > 50) angle = 90;
               if (fAxis->TestBit(TAxis::kLabelsHori)) angle = 0;
               if (fAxis->TestBit(TAxis::kLabelsVert)) angle = 90;
               if (fAxis->TestBit(TAxis::kLabelsUp))   angle = 20;
               if (fAxis->TestBit(TAxis::kLabelsDown)) angle = -20;
               if (angle ==   0) textaxis->SetTextAlign(23);
               if (angle == -20) textaxis->SetTextAlign(12);
               Double_t s = -3;
               if (ymin == gPad->GetUymax()) {
                  if (angle == 0) textaxis->SetTextAlign(21);
                  s = 3;
               }
               textaxis->PaintLatex(fAxis->GetBinCenter(i),
                                    ymin + s * fAxis->GetLabelOffset() * (gPad->GetUymax() - gPad->GetUymin()),
                                    angle,
                                    textaxis->GetTextSize(),
                                    fAxis->GetBinLabel(i));
            } else if ((!strcmp(fAxis->GetName(), "yaxis") && !gPad->TestBit(kHori))
                       || (!strcmp(fAxis->GetName(), "xaxis") &&  gPad->TestBit(kHori))) {
               Double_t s = -3;
               if (xmin == gPad->GetUxmax()) {
                  textaxis->SetTextAlign(12);
                  s = 3;
               }
               textaxis->PaintLatex(xmin + s * fAxis->GetLabelOffset() * (gPad->GetUxmax() - gPad->GetUxmin()),
                                    fAxis->GetBinCenter(i),
                                    0,
                                    textaxis->GetTextSize(),
                                    fAxis->GetBinLabel(i));
            } else {
               textaxis->PaintLatex(xmin - 3 * fAxis->GetLabelOffset() * (gPad->GetUxmax() - gPad->GetUxmin()),
                                    ymin + (i - 0.5) * (ymax - ymin) / nl,
                                    0,
                                    textaxis->GetTextSize(),
                                    fAxis->GetBinLabel(i));
            }
         }
      }
   }

   //*-*-              Now determine orientation of labels on axis
   if (!gPad->IsBatch()) {
      if (cosphi > 0) gVirtualX->SetCharacterUp(-sinphi, cosphi);
      else            gVirtualX->SetCharacterUp(sinphi, -cosphi);
      if (x0 == x1)   gVirtualX->SetCharacterUp(0, 1);
      if (optionVert) gVirtualX->SetCharacterUp(0, 1);
      if (optionPara) gVirtualX->SetCharacterUp(-sinphi, cosphi);
      if (optionDown) gVirtualX->SetCharacterUp(cosphi, sinphi);
   }

   //*-*-              Now determine text alignment
   xalign = 2;
   yalign = 1;
   if (x0 == x1)    xalign = 3;
   if (y0 != y1)    yalign = 2;
   if (optionCent)  xalign = 2;
   if (optionRight) xalign = 3;
   if (optionLeft)  xalign = 1;
   if (TMath::Abs(cosphi) > 0.9) {
      xalign = 2;
   } else {
      if (cosphi * sinphi > 0)  xalign = 1;
      if (cosphi * sinphi < 0)  xalign = 3;
   }
   textaxis->SetTextAlign(10 * xalign + yalign);

   //*-*-              Position of labels in Y
   if (x0 == x1) {
      if (optionPlus && !optionMinus) {
         if (optionEqual) ylabel =  fLabelOffset / 2 + atick[0];
         else             ylabel = -fLabelOffset;
      } else {
         ylabel = fLabelOffset;
         if (lside < 0)  ylabel += atick[0];
      }
   } else if (y0 == y1) {
      if (optionMinus && !optionPlus) {
         if ((GetLabelFont() % 10) == 3) {
            ylabel = fLabelOffset + 0.5 *
                     ((gPad->AbsPixeltoY(0) - gPad->AbsPixeltoY((Int_t)fLabelSize)) /
                      (gPad->GetY2() - gPad->GetY1()));
         } else {
            ylabel = fLabelOffset + 0.5 * fLabelSize;
         }
         ylabel += TMath::Abs(atick[0]);
      } else {
         ylabel = -fLabelOffset;
         if (mside <= 0) ylabel -= TMath::Abs(atick[0]);
      }
      if (optionLog)  ylabel -= 0.5 * charheight;
   } else {
      if (mside + lside >= 0) ylabel =  fLabelOffset;
      else                  ylabel = -fLabelOffset;
   }
   if (optionText) ylabel /= 2;

   //*-*-              Draw the linear tick marks if needed...
   if (!optionLog) {
      if (ndiv) {
         if (fNbins) nticks = fNbins;
         if (fFunction) {
            dxtick = (binHigh - binLow) / Double_t(nticks - 1);
         } else {
            if (optionNoopt && !optionInt) dxtick = axis_length / Double_t(nticks - 1);
            else                           dxtick = axis_lengthN / Double_t(nticks - 1);
            if (fNbins) dxtick = axis_length / (fWmax - fWmin);
         }
         for (k = 0; k < nticks; k++) {
            ltick = 2;
            if (k % nn3 == 0) ltick = 1;
            if (k % nn2 == 0) ltick = 0;
            if (fFunction) {
               Double_t xf = binLow + Double_t(k) * dxtick;
               Double_t zz = fFunction->Eval(xf) - rwmi;
               xtick = zz * axis_length / TMath::Abs(rwma - rwmi);
            } else {
               if (fNbins) xtick = (fBins[k] - fWmin) * dxtick;
               else xtick = Double_t(k) * dxtick;
            }
            ytick = 0;
            if (!mside) ytick -= atick[ltick];
            if (optionNoopt && !optionInt) {
               Rotate(xtick, ytick, cosphi, sinphi, x0, y0, xpl2, ypl2);
               Rotate(xtick, atick[ltick], cosphi, sinphi, x0, y0, xpl1, ypl1);
            } else {
               Rotate(xtick, ytick, cosphi, sinphi, xx0, yy0, xpl2, ypl2);
               Rotate(xtick, atick[ltick], cosphi, sinphi, xx0, yy0, xpl1, ypl1);
            }
            if (optionVert) {
               if ((x0 != x1) && (y0 != y1)) {
                  if (mside) {
                     xpl1 = xpl2;
                     if (cosphi > 0) ypl1 = ypl2 + atick[ltick];
                     else            ypl1 = ypl2 - atick[ltick];
                  } else {
                     xpl1 = 0.5 * (xpl1 + xpl2);
                     xpl2 = xpl1;
                     ypl1 = 0.5 * (ypl1 + ypl2) + atick[ltick];
                     ypl2 = 0.5 * (ypl1 + ypl2) - atick[ltick];
                  }
               }
            }
            if (!drawGridOnly) PaintLineNDC(xpl1, ypl1, xpl2, ypl2);

            if (optionGrid) {
               if (ltick == 0) {
                  if (optionNoopt && !optionInt) {
                     Rotate(xtick, 0, cosphi, sinphi, x0, y0, xpl2, ypl2);
                     Rotate(xtick, grid_side * gridlength, cosphi, sinphi, x0, y0, xpl1, ypl1);
                  } else {
                     Rotate(xtick, 0, cosphi, sinphi, xx0, yy0, xpl2, ypl2);
                     Rotate(xtick, grid_side * gridlength, cosphi, sinphi, xx0, yy0, xpl1, ypl1);
                  }
                  linegrid->PaintLineNDC(xpl1, ypl1, xpl2, ypl2);
               }
            }
         }
         xtick0 = 0;
         xtick1 = xtick;

         if (fFunction) axis_length0 = binLow - wmin;
         if ((!optionNoopt || optionInt) && axis_length0) {
            nticks0 = Int_t(axis_length0 / dxtick);
            if (nticks0 > 1000) nticks0 = 1000;
            for (k = 0; k <= nticks0; k++) {
               ltick = 2;
               if (k % nn3 == 0) ltick = 1;
               if (k % nn2 == 0) ltick = 0;
               ytick0 = 0;
               if (!mside) ytick0 -= atick[ltick];
               if (fFunction) {
                  xtick0 = (fFunction->Eval(binLow - Double_t(k) * dxtick) - rwmi)
                           * axis_length / TMath::Abs(rwma - rwmi);
               }
               Rotate(xtick0, ytick0, cosphi, sinphi, xx0, yy0, xpl2, ypl2);
               Rotate(xtick0, atick[ltick], cosphi, sinphi, xx0, yy0, xpl1, ypl1);
               if (optionVert) {
                  if ((x0 != x1) && (y0 != y1)) {
                     if (mside) {
                        xpl1 = xpl2;
                        if (cosphi > 0) ypl1 = ypl2 + atick[ltick];
                        else            ypl1 = ypl2 - atick[ltick];
                     } else {
                        xpl1 = 0.5 * (xpl1 + xpl2);
                        xpl2 = xpl1;
                        ypl1 = 0.5 * (ypl1 + ypl2) + atick[ltick];
                        ypl2 = 0.5 * (ypl1 + ypl2) - atick[ltick];
                     }
                  }
               }
               if (!drawGridOnly) PaintLineNDC(xpl1, ypl1, xpl2, ypl2);

               if (optionGrid) {
                  if (ltick == 0) {
                     Rotate(xtick0, 0, cosphi, sinphi, xx0, yy0, xpl2, ypl2);
                     Rotate(xtick0, grid_side * gridlength, cosphi, sinphi, xx0, yy0, xpl1, ypl1);
                     linegrid->PaintLineNDC(xpl1, ypl1, xpl2, ypl2);
                  }
               }
               xtick0 -= dxtick;
            }
         }

         if (fFunction) axis_length1 = wmax - binHigh;
         if ((!optionNoopt || optionInt) && axis_length1) {
            nticks1 = int(axis_length1 / dxtick);
            if (nticks1 > 1000) nticks1 = 1000;
            for (k = 0; k <= nticks1; k++) {
               ltick = 2;
               if (k % nn3 == 0) ltick = 1;
               if (k % nn2 == 0) ltick = 0;
               ytick1 = 0;
               if (!mside) ytick1 -= atick[ltick];
               if (fFunction) {
                  xtick1 = (fFunction->Eval(binHigh + Double_t(k) * dxtick) - rwmi)
                           * axis_length / TMath::Abs(rwma - rwmi);
               }
               Rotate(xtick1, ytick1, cosphi, sinphi, xx0, yy0, xpl2, ypl2);
               Rotate(xtick1, atick[ltick], cosphi, sinphi, xx0, yy0, xpl1, ypl1);
               if (optionVert) {
                  if ((x0 != x1) && (y0 != y1)) {
                     if (mside) {
                        xpl1 = xpl2;
                        if (cosphi > 0) ypl1 = ypl2 + atick[ltick];
                        else            ypl1 = ypl2 - atick[ltick];
                     } else {
                        xpl1 = 0.5 * (xpl1 + xpl2);
                        xpl2 = xpl1;
                        ypl1 = 0.5 * (ypl1 + ypl2) + atick[ltick];
                        ypl2 = 0.5 * (ypl1 + ypl2) - atick[ltick];
                     }
                  }
               }
               if (!drawGridOnly) PaintLineNDC(xpl1, ypl1, xpl2, ypl2);
               if (optionGrid) {
                  if (ltick == 0) {
                     Rotate(xtick1, 0, cosphi, sinphi, xx0, yy0, xpl2, ypl2);
                     Rotate(xtick1, grid_side * gridlength, cosphi, sinphi, xx0, yy0, xpl1, ypl1);
                     linegrid->PaintLineNDC(xpl1, ypl1, xpl2, ypl2);
                  }
               }
               xtick1 += dxtick;
            }
         }
      }
   }

   //*-*-              Draw the numeric labels if needed...
   if (!drawGridOnly && !optionUnlab) {
      if (!optionLog) {
         if (n1a) {
            //*-*-              Spacing of labels
            if ((wmin == wmax) || (ndiv == 0)) {
               Error(where, "wmin (%f) == wmax (%f), or ndiv == 0", wmin, wmax);
               goto L210;
            }
            wlabel  = wmin;
            dwlabel = (wmax - wmin) / Double_t(n1a);
            if (optionNoopt && !optionInt) dxlabel = axis_length / Double_t(n1a);
            else                           dxlabel = axis_lengthN / Double_t(n1a);
            if (fNbins) dxlabel = axis_length / (fWmax - fWmin);

            if (!optionText && !optionTime) {

               //*-*-              We have to decide what format to generate
               //*-*-              (for numeric labels only)
               //*-*-              Test the magnitude, decide format
               flexe  = kFALSE;
               nexe   = 0;
               flexpo = kFALSE;
               flexne = kFALSE;
               ww     = TMath::Max(TMath::Abs(wmin), TMath::Abs(wmax));

               //*-*-              First case : (wmax-wmin)/n1a less than 0.001
               //*-*-              (0.001 fgMaxDigits of 5 (fgMaxDigits) characters). Then we use x 10 n
               //*-*-              format. If af >=0 x10 n cannot be used
               Double_t xmicros = 0.00099;
               if (maxDigits) xmicros = TMath::Power(10, -maxDigits);
               if (!noExponent && (TMath::Abs(wmax - wmin) / Double_t(n1a)) < xmicros) {
                  af    = TMath::Log10(ww) + epsilon;
                  if (af < 0) {
                     flexe   = kTRUE;
                     nexe    = int(af);
                     iexe    = TMath::Abs(nexe);
                     if (iexe % 3 == 1)     iexe += 2;
                     else if (iexe % 3 == 2) iexe += 1;
                     if (nexe < 0) nexe = -iexe;
                     else          nexe =  iexe;
                     wlabel  = wlabel * TMath::Power(10, iexe);
                     dwlabel = dwlabel * TMath::Power(10, iexe);
                     if1     = maxDigits;
                     if2     = maxDigits - 2;
                     goto L110;
                  }
               }
               if (ww >= 1) af = TMath::Log10(ww);
               else         af = TMath::Log10(ww * 0.0001);
               af += epsilon;
               nf  = Int_t(af) + 1;
               if (!noExponent && nf > maxDigits)  flexpo = kTRUE;
               if (!noExponent && nf < -maxDigits) flexne = kTRUE;

               //*-*-              Use x 10 n format. (only powers of 3 allowed)

               if (flexpo) {
                  flexe = kTRUE;
                  while (1) {
                     nexe++;
                     ww      /= 10;
                     wlabel  /= 10;
                     dwlabel /= 10;
                     if (nexe % 3 == 0 && ww <= TMath::Power(10, maxDigits - 1)) break;
                  }
               }

               if (flexne) {
                  flexe = kTRUE;
                  rne   = 1 / TMath::Power(10, maxDigits - 2);
                  while (1) {
                     nexe--;
                     ww      *= 10;
                     wlabel  *= 10;
                     dwlabel *= 10;
                     if (nexe % 3 == 0 && ww >= rne) break;
                  }
               }

               na = 0;
               for (i = maxDigits - 1; i > 0; i--) {
                  if (TMath::Abs(ww) < TMath::Power(10, i)) na = maxDigits - i;
               }
               ndyn = n1a;
               while (ndyn) {
                  Double_t wdyn = TMath::Abs((wmax - wmin) / ndyn);
                  if (wdyn <= 0.999 && na < maxDigits - 2) {
                     na++;
                     ndyn /= 10;
                  } else break;
               }

               if2 = na;
               if1 = TMath::Max(nf + na, maxDigits) + 1;
L110:
               if (TMath::Min(wmin, wmax) < 0)if1 = if1 + 1;
               if1 = TMath::Min(if1, 32);

               //*-*- In some cases, if1 and if2 are too small....
               while (dwlabel < TMath::Power(10, -if2)) {
                  if1++;
                  if2++;
               }
               coded = &chcoded[0];
               if (if1 > 14) if1 = 14;
               if (if2 > 14) if2 = 14;
               if (if2) snprintf(coded, 8, "%%%d.%df", if1, if2);
               else     snprintf(coded, 8, "%%%d.%df", if1 + 1, 1);
            }

            //*-*-              We draw labels

            snprintf(chtemp, 256, "%g", dwlabel);
            Int_t ndecimals = 0;
            if (optionDecimals) {
               char* dot = strchr(chtemp, '.');
               if (dot) {
                  ndecimals = chtemp + strlen(chtemp) - dot;
               } else {
                  char* exp;
                  exp = strstr(chtemp, "e-");
                  if (exp) {
                     sscanf(&exp[2], "%d", &ndecimals);
                     ndecimals++;
                  }
               }
            }
            if (optionM) nlabels = n1a - 1;
            else         nlabels = n1a;
            wTimeIni = wlabel;
            if (fNbins) nlabels = fNbins - 1;
            for (k = 0; k <= nlabels; k++) {
               if (fFunction) {
                  Double_t xf = binLow + Double_t(k * nn2) * dxtick;
                  Double_t zz = fFunction->Eval(xf) - rwmi;
                  wlabel = xf;
                  xlabel = zz * axis_length / TMath::Abs(rwma - rwmi);
               } else {
                  if (fNbins) xlabel = dxlabel * (fBins[k] - fWmin);
                  else       xlabel = dxlabel * k;
               }
               if (optionM)    xlabel += 0.5 * dxlabel;

               if (!optionText && !optionTime) {
                  snprintf(label, 256, &chcoded[0], wlabel);
                  if (fNbins) label = Form(fFormat, fBins[k]);
                  if (fLabels) label = Form("%s", fLabels[k]);
                  label[28] = 0;
                  wlabel += dwlabel;

                  LabelsLimits(label, first, last); //Eliminate blanks

                  if (label[first] == '.') { //check if '.' is preceded by a digit
                     strncpy(chtemp, "0", 256);
                     strlcat(chtemp, &label[first], 256);
                     strncpy(label, chtemp, 256);
                     first = 1;
                     last = strlen(label);
                  }
                  if (label[first] == '-' && label[first + 1] == '.') {
                     strncpy(chtemp, "-0", 256);
                     strlcat(chtemp, &label[first + 1], 256);
                     strncpy(label, chtemp, 256);
                     first = 1;
                     last = strlen(label);
                  }

                  //*-*-              We eliminate the non significant 0 after '.'
                  if (ndecimals) {
                     char* adot = strchr(label, '.');
                     if (adot) adot[ndecimals] = 0;
                  } else {
                     while (label[last] == '0') {
                        label[last] = 0;
                        last--;
                     }
                  }

                  //*-*-              We eliminate the dot, unless dot is forced.
                  if (label[last] == '.') {
                     if (!optionDot) {
                        label[last] = 0;
                        last--;
                     }
                  }

                  //*-*-            Make sure the label is not "-0"
                  if (last - first == 1 && label[first] == '-'
                        && label[last]  == '0') {
                     strncpy(label, "0", 256);
                     label[last] = 0;
                  }
                  if (fLabels) label = Form("%s", fLabels[k]);
               }

               //*-*-              Generate the time labels

               if (optionTime) {
                  timed = wlabel + (int)(timeoffset) - rangeOffset;
                  timelabel = (time_t)((Long_t)(timed));
                  if (optionTime == 1) {
                     utctis = localtime(&timelabel);
                  } else {
                     utctis = gmtime(&timelabel);
                  }
                  TString timeformattmp;
                  if (timeformat.Length() < 220) timeformattmp = timeformat;
                  else timeformattmp = "#splitline{Format}{too long}";

                  //*-*-              Appends fractionnal part if seconds displayed
                  if (dwlabel < 0.9) {
                     double tmpdb;
                     int tmplast;
                     snprintf(label, 256, "%%S%7.5f", modf(timed, &tmpdb));
                     tmplast = strlen(label) - 1;

                     //*-*-              We eliminate the non significiant 0 after '.'
                     while (label[tmplast] == '0') {
                        label[tmplast] = 0;
                        tmplast--;
                     }

                     timeformattmp.ReplaceAll("%S", label);
                     //*-*-              replace the "0." at the begining by "s"
                     timeformattmp.ReplaceAll("%S0.", "%Ss");

                  }

                  strftime(label, 256, timeformattmp.Data(), utctis);
                  strncpy(chtemp, &label[0], 256);
                  first = 0;
                  last = strlen(label) - 1;
                  wlabel = wTimeIni + (k + 1) * dwlabel;
               }

               //*-*-              We generate labels (numeric or alphanumeric).

//                    if(fLabels) label = fLabels[k];
               if (optionNoopt && !optionInt)
                  Rotate(xlabel, ylabel, cosphi, sinphi, x0, y0, xx, yy);
               else     Rotate(xlabel, ylabel, cosphi, sinphi, xx0, yy0, xx, yy);
               if (y0 == y1 && !optionDown && !optionUp) {
                  yy -= 0.80 * charheight;
               }
               if (optionVert) {
                  if (x0 != x1 && y0 != y1) {
                     if (optionNoopt && !optionInt)
                        Rotate(xlabel, 0, cosphi, sinphi, x0, y0, xx, yy);
                     else  Rotate(xlabel, 0, cosphi, sinphi, xx0, yy0, xx, yy);
                     if (cosphi > 0) yy += ylabel;
                     if (cosphi < 0) yy -= ylabel;
                  }
               }
               if (!optionY || (x0 == x1)) {
                  if (!optionText) {
                     if (first > last)  strncpy(chtemp, " ", 256);
                     else               strncpy(chtemp, &label[first], 256);
                     textaxis->PaintLatex(gPad->GetX1() + xx * (gPad->GetX2() - gPad->GetX1()),
                                          gPad->GetY1() + yy * (gPad->GetY2() - gPad->GetY1()),
                                          0,
                                          textaxis->GetTextSize(),
                                          chtemp);
//                            Info(where,"label : %s,%s",chtemp,fLabels[k]);
                  } else  {
                     if (optionText == 1) textaxis->PaintLatex(gPad->GetX1() + xx * (gPad->GetX2() - gPad->GetX1()),
                              gPad->GetY1() + yy * (gPad->GetY2() - gPad->GetY1()),
                              0,
                              textaxis->GetTextSize(),
                              fAxis->GetBinLabel(k + fAxis->GetFirst()));
                  }
               } else {

                  //*-*-       Text alignment is down
                  if (!optionText)     lnlen = last - first + 1;
                  else {
                     if (k + 1 > nhilab) lnlen = 0;
                  }
                  for (l = 1; l <= lnlen; l++) {
                     if (!optionText) *chtemp = label[first + l - 2];
                     else {
                        if (lnlen == 0) strncpy(chtemp, " ", 256);
                        else            strncpy(chtemp, "1", 256);
                     }
                     textaxis->PaintLatex(gPad->GetX1() + xx * (gPad->GetX2() - gPad->GetX1()),
                                          gPad->GetY1() + yy * (gPad->GetY2() - gPad->GetY1()),
                                          0,
                                          textaxis->GetTextSize(),
                                          chtemp);
                     yy -= charheight * 1.3;
                  }
               }
            }

            //*-*-                We use the format x 10 ** n

            if (flexe && !optionText && nexe)  {
               snprintf(label, 256, "#times10^{%d}", nexe);
               if (x0 != x1) {
                  xfactor = x1 - x0 + 0.1 * charheight;
                  yfactor = 0;
               } else          {
                  xfactor = y1 - y0 + 0.1 * charheight;
                  yfactor = 0;
               }
               Rotate(xfactor, yfactor, cosphi, sinphi, x0, y0, xx, yy);
               textaxis->SetTextAlign(11);
               if (GetLabelFont() % 10 < 2) // force TLatex mode in PaintLatex
                  textaxis->SetTextFont((Int_t)(GetLabelFont() / 10) * 10 + 2);
               textaxis->PaintLatex(gPad->GetX1() + xx * (gPad->GetX2() - gPad->GetX1()),
                                    gPad->GetY1() + yy * (gPad->GetY2() - gPad->GetY1()),
                                    0,
                                    textaxis->GetTextSize(),
                                    label);
            }
         }
      }
   }

   //*-*-              Log axis

   if (optionLog && ndiv) {
      UInt_t xi1 = 0, xi2, wi, yi1 = 0, yi2, hi, xl, xh;
      Bool_t firstintlab = kTRUE, overlap = kFALSE;
      if ((wmin == wmax) || (ndiv == 0))  {
         Error(where, "wmin (%f) == wmax (%f), or ndiv == 0", wmin, wmax);
         goto L210;
      }
      if (wmin <= 0)   {
         Error(where, "negative logarithmic axis");
         goto L210;
      }
      if (wmax <= 0)     {
         Error(where, "negative logarithmic axis");
         goto L210;
      }
      xmnlog = TMath::Log10(wmin);
      if (xmnlog > 0) xmnlog += 1.E-6;
      else            xmnlog -= 1.E-6;
      x00    = 0;
      x11    = axis_length;
      h2     = TMath::Log10(wmax);
      h2sav  = h2;
      if (h2 > 0) h2 += 1.E-6;
      else        h2 -= 1.E-6;
      ih1    = int(xmnlog);
      ih2    = 1 + int(h2);
      nbinin = ih2 - ih1 + 1;
      axmul  = (x11 - x00) / (h2sav - xmnlog);

      //*-*-              Plot decade and intermediate tick marks
      decade      = ih1 - 2;
      labelnumber = ih1;
      if (xmnlog > 0 && (xmnlog - Double_t(ih1) > 0)) labelnumber++;
      for (j = 1; j <= nbinin; j++) {

         //*-*-              Plot decade
         firstintlab = kTRUE, overlap = kFALSE;
         decade++;
         if (x0 == x1 && j == 1) ylabel += charheight * 0.33;
         if (y0 == y1 && j == 1) ylabel -= charheight * 0.65;
         xone = x00 + axmul * (Double_t(decade) - xmnlog);
         //the following statement is a trick to circumvent a gcc bug
         if (j < 0) printf("j=%d\n", j);
         if (x00 > xone) goto L160;
         if ((xone - x11) > epsilon) break;
         xtwo = xone;
         y    = 0;
         if (!mside) y -= atick[0];
         Rotate(xone, y, cosphi, sinphi, x0, y0, xpl2, ypl2);
         Rotate(xtwo, atick[0], cosphi, sinphi, x0, y0, xpl1, ypl1);
         if (optionVert) {
            if ((x0 != x1) && (y0 != y1)) {
               if (mside) {
                  xpl1 = xpl2;
                  if (cosphi > 0) ypl1 = ypl2 + atick[0];
                  else            ypl1 = ypl2 - atick[0];
               } else {
                  xpl1 = 0.5 * (xpl1 + xpl2);
                  xpl2 = xpl1;
                  ypl1 = 0.5 * (ypl1 + ypl2) + atick[0];
                  ypl2 = 0.5 * (ypl1 + ypl2) - atick[0];
               }
            }
         }
         if (!drawGridOnly) PaintLineNDC(xpl1, ypl1, xpl2, ypl2);

         if (optionGrid) {
            Rotate(xone, 0, cosphi, sinphi, x0, y0, xpl2, ypl2);
            Rotate(xone, grid_side * gridlength, cosphi, sinphi, x0, y0, xpl1, ypl1);
            linegrid->PaintLineNDC(xpl1, ypl1, xpl2, ypl2);
         }

         if (!drawGridOnly && !optionUnlab)  {

            //*-*-              We generate labels (numeric only).
//                if(fLabels) label = fLabels[k];
            if (noExponent) {
               rlab = TMath::Power(10, labelnumber);
               snprintf(label, 256, "%f", rlab);
               LabelsLimits(label, first, last);
               while (last > first) {
                  if (label[last] != '0') break;
                  label[last] = 0;
                  last--;
               }
               if (label[last] == '.') {
                  label[last] = 0;
                  last--;
               }
            } else {
               snprintf(label, 256, "%d", labelnumber);
               LabelsLimits(label, first, last);
            }
            Rotate(xone, ylabel, cosphi, sinphi, x0, y0, xx, yy);
            if ((x0 == x1) && !optionPara) {
               if (lside < 0) {
                  if (mside < 0) {
                     if (labelnumber == 0) nch = 1;
                     else                  nch = 2;
                     xx    += nch * charheight;
                  } else {
                     xx += 0.25 * charheight;
                  }
               }
               xx += 0.25 * charheight;
            }
            if ((y0 == y1) && !optionDown && !optionUp) {
               if (noExponent) yy += 0.33 * charheight;
            }
            if (n1a == 0)goto L210;
            kmod = nbinin / n1a;
            if (kmod == 0) kmod = 1000000;
            if ((nbinin <= n1a) || (j == 1) || (j == nbinin) || ((nbinin > n1a)
                  && (j % kmod == 0))) {
               if (labelnumber == 0) {
                  textaxis->PaintTextNDC(xx, yy, "1");
               } else if (labelnumber == 1) {
                  textaxis->PaintTextNDC(xx, yy, "10");
               } else {
                  if (noExponent) {
                     textaxis->PaintTextNDC(xx, yy, &label[first]);
                  } else {
                     snprintf(chtemp, 256, "10^{%d}", labelnumber);
                     textaxis->PaintLatex(gPad->GetX1() + xx * (gPad->GetX2() - gPad->GetX1()),
                                          gPad->GetY1() + yy * (gPad->GetY2() - gPad->GetY1()),
                                          0, textaxis->GetTextSize(), chtemp);

                  }
               }
            }
            labelnumber++;
         }
L160:
         for (k = 2; k < 10; k++) {

            //*-*-              Plot intermediate tick marks
            xone = x00 + axmul * (TMath::Log10(Double_t(k)) + Double_t(decade) - xmnlog);
            if (x00 > xone) continue;
            if (xone > x11) goto L200;
            y = 0;
            if (!mside)  y -= atick[1];
            xtwo = xone;
            Rotate(xone, y, cosphi, sinphi, x0, y0, xpl2, ypl2);
            Rotate(xtwo, atick[1], cosphi, sinphi, x0, y0, xpl1, ypl1);
            if (optionVert) {
               if ((x0 != x1) && (y0 != y1)) {
                  if (mside) {
                     xpl1 = xpl2;
                     if (cosphi > 0) ypl1 = ypl2 + atick[1];
                     else            ypl1 = ypl2 - atick[1];
                  } else {
                     xpl1 = 0.5 * (xpl1 + xpl2);
                     xpl2 = xpl1;
                     ypl1 = 0.5 * (ypl1 + ypl2) + atick[1];
                     ypl2 = 0.5 * (ypl1 + ypl2) - atick[1];
                  }
               }
            }
            idn = n1a * 2;
            if ((nbinin <= idn) || ((nbinin > idn) && (k == 5))) {
               if (!drawGridOnly) PaintLineNDC(xpl1, ypl1, xpl2, ypl2);

               //*-*- Draw the intermediate LOG labels if requested

               if (moreLogLabels && !optionUnlab && !drawGridOnly && !overlap) {
                  if (noExponent) {
                     rlab = Double_t(k) * TMath::Power(10, labelnumber - 1);
                     snprintf(chtemp, 256, "%g", rlab);
                  } else {
                     if (labelnumber - 1 == 0) {
                        snprintf(chtemp, 256, "%d", k);
                     } else if (labelnumber - 1 == 1) {
                        snprintf(chtemp, 256, "%d", 10 * k);
                     } else {
                        snprintf(chtemp, 256, "%d#times10^{%d}", k, labelnumber - 1);
                     }
                  }
                  Rotate(xone, ylabel, cosphi, sinphi, x0, y0, xx, yy);
                  if ((x0 == x1) && !optionPara) {
                     if (lside < 0) {
                        if (mside < 0) {
                           if (labelnumber == 0) nch = 1;
                           else                  nch = 2;
                           xx    += nch * charheight;
                        } else {
                           if (labelnumber >= 0) xx    += 0.25 * charheight;
                           else                  xx    += 0.50 * charheight;
                        }
                     }
                     xx += 0.25 * charheight;
                  }
                  if ((y0 == y1) && !optionDown && !optionUp) {
                     if (noExponent) yy += 0.33 * charheight;
                  }
                  if (optionVert) {
                     if ((x0 != x1) && (y0 != y1)) {
                        Rotate(xone, ylabel, cosphi, sinphi, x0, y0, xx, yy);
                        if (cosphi > 0) yy += ylabel;
                        else            yy -= ylabel;
                     }
                  }
                  textaxis->SetTitle(chtemp);
                  Double_t u = gPad->GetX1() + xx * (gPad->GetX2() - gPad->GetX1());
                  Double_t v = gPad->GetY1() + yy * (gPad->GetY2() - gPad->GetY1());
                  if (firstintlab) {
                     textaxis->GetBoundingBox(wi, hi);
                     wi = (UInt_t)(wi * 1.3);
                     hi *= (UInt_t)(hi * 1.3);
                     xi1 = gPad->XtoAbsPixel(u);
                     yi1 = gPad->YtoAbsPixel(v);
                     firstintlab = kFALSE;
                     textaxis->PaintLatex(u, v, 0, textaxis->GetTextSize(), chtemp);
                  } else {
                     xi2 = gPad->XtoAbsPixel(u);
                     yi2 = gPad->YtoAbsPixel(v);
                     xl = TMath::Min(xi1, xi2);
                     xh = TMath::Max(xi1, xi2);
                     if ((x0 == x1 && yi1 - hi <= yi2) || (y0 == y1 && xl + wi >= xh)) {
                        overlap = kTRUE;
                     } else {
                        xi1 = xi2;
                        yi1 = yi2;
                        textaxis->GetBoundingBox(wi, hi);
                        wi = (UInt_t)(wi * 1.3);
                        hi *= (UInt_t)(hi * 1.3);
                        textaxis->PaintLatex(u, v, 0, textaxis->GetTextSize(), chtemp);
                     }
                  }
               }

               //*-*- Draw the intermediate LOG grid if only three decades are requested
               if (optionGrid && nbinin <= 5 && ndiv > 100) {
                  Rotate(xone, 0, cosphi, sinphi, x0, y0, xpl2, ypl2);
                  Rotate(xone, grid_side * gridlength, cosphi, sinphi, x0, y0, xpl1, ypl1);
                  linegrid->PaintLineNDC(xpl1, ypl1, xpl2, ypl2);
               }
            }  //endif ((nbinin <= idn) ||
         }  //endfor (k=2;k<10;k++)
      } //endfor (j=1; j<=nbinin; j++)
L200:
      Int_t kuku = 0;
      if (kuku) { }
   }  //endif (optionLog && ndiv)


L210:
   if (optionGrid) delete linegrid;
   delete textaxis;
}

