#include "KVTelescope.h"
#include "KVDetector.h"
#include "KVNucleus.h"
#include "KVUnits.h"
#include "TGraph.h"
#include "Riostream.h"
#include "TString.h"
#include "TROOT.h"
#include "TMath.h"
#include "TGeoManager.h"
#include "TGeoMatrix.h"

using namespace std;

ClassImp(KVTelescope)
/////////////////////////////////////////////////////////////////////////
// KVTelescope
//

KVTelescope::KVTelescope()
{
   init();
};


void KVTelescope::init()
{
   //default initialisation. a KVList is created to hold telescope detectors.
   //the telescope owns its detectors and will delete them when deleted itself.
   fNdets = 0;
   fDepth = 0;
   SetType("TELESCOPE");
   SetOwnsDetectors();
}

//____________________________________________________________________________

KVTelescope::~KVTelescope()
{
   if (fDepth)
      delete[]fDepth;
   fDepth = 0;
   fNdets = 0;
}

//_________________________________________________________________________________
void KVTelescope::Add(KVBase* element)
{
   // Add a detector or a daughter structure to this telescope.
   // For detectors we set up the KVDetectorNode (in front of - behind
   // relationships) information.

   if (element->InheritsFrom(KVDetector::Class())) {
      Int_t ndets = GetSize();
      if (ndets) {
         KVDetector* d = dynamic_cast<KVDetector*>(element);
         KVDetector* ld = GetDetector(ndets);
         ld->GetNode()->AddBehind(d);
         d->GetNode()->AddInFront(ld);
      }
   }
   KVGeoStrucElement::Add(element);
}

void KVTelescope::DetectParticle(KVNucleus* kvp, KVNameValueList* nvl)
{
   //Simulates the passage of a charged particle through all detectors of the telescope, in the order in which they
   //were added to it (corresponding to rank given by GetDetectorRank()).
   //Begin_Html
   //<img src="http://indra.in2p3.fr/KaliVedaDoc/images/kvtelescope_detectparticle.gif">
   //End_Html
   //It should be noted that
   // (1) the small variations in effective detector thickness due to the particle's angle of incidence are not, for the moment, taken into account
   // (2) the simplified description of detector geometry used here does not take into account trajectories such as that marked "b" shown in the figure.
   //      All particles impinging on the first detector of the telescope are assumed to pass through all subsequent detectors as in "c"
   //    (unless they stop in one of the detectors)
   //
   //The KVNameValueList, if it's defined, allows to store
   //the energy loss in the different detectors the particle goes through
   //exemple : for a Silicon-CsI telescope named SI_CSI_0401 , you will obtain:
   //      {
   //         KVNucleus nn(6,12); nn.SetKE(1000);
   //         KVTelescope* tel = gMultiDetArray->GetTelescope("SI_CSI_0401");
   //         KVNameValueList* nvl = new KVNameValueList;
   //         tel->DetectParticle(&nn,nvl);
   //         nvl->Print();
   //      }
   //      Collection name='KVNameValueList', class='KVNameValueList', size=2
   //       OBJ: TNamed   SI_0401  8.934231
   //       OBJ: TNamed   CSI_0401 991.065769
   //The energy loss in each detector corresponds to those lost in active layer

   KVDetector* obj;

   TIter next(GetDetectors());
   while ((obj = (KVDetector*) next())) {
      Double_t ebefore = obj->GetEnergy(); //Energie dans le detecteur avant passage
      obj->DetectParticle(kvp);               //Detection de la particule
      ebefore -= obj->GetEnergy();            //la difference d energie avant et apres passage (donc l energie laissee par la particule)
      if (nvl)
         nvl->SetValue(obj->GetName(), TMath::Abs(ebefore)); //Enregistrement de la perte d energie
      if (kvp->GetEnergy() <= 0.0)
         break;
   }
}

//_________________________________________________________________________
Int_t KVTelescope::GetDetectorRank(KVDetector* kvd)
{
   //returns position (1=front, 2=next, etc.) detector in the telescope structure
   if ((KVTelescope*)kvd->GetParentStructure("TELESCOPE") != this) {
      Warning("GetDetectorRank", "Detector does not belong to this telescope!");
      cout << endl;
      return 0;
   }
   TIter next(GetDetectors());
   KVDetector* d;
   UInt_t i = 1;
   while ((d = (KVDetector*) next())) {
      if (d == kvd)
         return i;
      i++;
   }
   return 0;
}


//_______________________________________________________________________________
void KVTelescope::ResetDetectors()
{
   // Reset Energy losses to be ready for the next event
   const_cast<KVSeqCollection*>(GetDetectors())->Execute("Clear", "");
}

//__________________________________________________________________________________

void KVTelescope::SetDepth(UInt_t ndet, Float_t depth)
{
   //set the depth of detector number ndet(=1,2,...) in mm.

   fNdets = GetSize();
   if (!fNdets) {
      Error("SetDepth",
            "Add detectors to telescope before setting depth.");
      return;
   }
   if ((int) ndet > fNdets) {
      Error("SetDepth",
            "Cannot set depth for detector %d in %d-member telescope.",
            ndet, fNdets);
      return;
   }
   if (!fDepth)
      fDepth = new Float_t[fNdets];
   fDepth[ndet - 1] = depth;
   GetDetector(ndet)->SetDistance(GetDistance() + depth * KVUnits::mm);
}

//__________________________________________________________________________________

Float_t KVTelescope::GetDepth(Int_t ndet) const
{
   //get depth of detector ndet(=1,2,...) in mm
   if (!fDepth) {
      Error("GetDepth", "Depths have not been set.");
      return -1.0;
   }
   if ((int) ndet > fNdets) {
      Error("SetDepth",
            "Cannot get depth for detector %d in %d-member telescope.",
            ndet, fNdets);
      return -1.0;
   }
   return fDepth[ndet - 1];
}

//__________________________________________________________________________________

Float_t KVTelescope::GetDepth() const
{
   //get depth of entire telescope in mm (sum of depths of detectors)
   if (!fDepth) {
      Error("GetDepth", "Depths have not been set.");
      return -1.0;
   }
   Float_t sum = 0.0;
   for (int ndet = 0; ndet < fNdets; ndet++) {
      sum += fDepth[ndet];
   }
   return sum;
}

//___________________________________________________________________________________________

TGeoVolume* KVTelescope::GetGeoVolume()
{
   // Create and return TGeoVolume representing detectors in this telescope.

   int no_of_dets = GetDetectors()->GetEntries();
   if (no_of_dets == 1) {
      // single detector "telescope": just return detector volume
      return GetDetector(1)->GetGeoVolume();
   }
   TGeoVolume* mother_vol = gGeoManager->MakeVolumeAssembly(Form("%s_TEL", GetName()));
   // total length of telescope = depth of last detector + thickness of last detector
   Double_t tot_len_tel =  GetTotalLengthInCM();
   //**** BUILD & ADD DETECTOR VOLUMES ****
   TIter next(GetDetectors());
   KVDetector* det;
   while ((det = (KVDetector*)next())) {
      TGeoVolume* det_vol = det->GetGeoVolume();
      // position detector in telescope
      Double_t dist = -tot_len_tel / 2. + det->GetDepthInTelescope() + det->GetTotalThicknessInCM() / 2.;
      TGeoTranslation* tran = new TGeoTranslation(0., 0., dist);
      mother_vol->AddNode(det_vol, 1, tran);
   }
   return mother_vol;
}

void KVTelescope::AddToGeometry()
{
   // Construct and position a TGeoVolume shape to represent this telescope in the current geometry
   if (!gGeoManager) return;

   // get volume for telescope
   TGeoVolume* vol = GetGeoVolume();

   // rotate telescope to orientation corresponding to (theta,phi)
   Double_t theta = GetTheta();
   Double_t phi = GetPhi();
   TGeoRotation rot1, rot2;
   rot2.SetAngles(phi + 90., theta, 0.);
   rot1.SetAngles(-90., 0., 0.);
   Double_t tot_len_tel = GetTotalLengthInCM();
   // distance to telescope centre = distance to telescope + half total lenght of telescope
   Double_t dist = GetDistance() + tot_len_tel / 2.;
   // translate telescope to correct distance from target (note: reference is CENTRE of telescope)
   Double_t trans_z = dist;

   TGeoTranslation tran(0, 0, trans_z);
   TGeoHMatrix h = rot2 * tran * rot1;
   TGeoHMatrix* ph = new TGeoHMatrix(h);

   // add telescope volume to geometry
   gGeoManager->GetTopVolume()->AddNode(vol, 1, ph);
}

Double_t KVTelescope::GetTotalLengthInCM() const
{
   // calculate total length of telescope from entrance of first detector to
   // backside of last detector
   int no_of_dets = GetDetectors()->GetEntries();
   Double_t tot_len_tel = GetDepthInCM(no_of_dets) + GetDetector(no_of_dets)->GetTotalThicknessInCM();
   return tot_len_tel;
}

