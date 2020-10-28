//Created by KVClassFactory on Thu Jun 21 17:01:26 2012
//Author: John Frankland,,,

#ifndef __KVEventFiltering_H
#define __KVEventFiltering_H

#include "KVEventSelector.h"
#include "KVClassMonitor.h"
#include "KVReconstructedEvent.h"
#include <KVSimEvent.h>
#ifdef WITH_GEMINI
#include "KVGemini.h"
#endif

//#define DEBUG_FILTER 1

class KVDBSystem;

/**
\class KVEventFiltering
\brief Filter simulated events with multidetector response
\ingroup Simulation

Use this KVEventSelector on simulated data TTrees containing a branch with KVSimEvent-derived objects:
~~~~{.cpp}
    my_tree->Process("KVEventFiltering", "[options]");
~~~~
where `"[options]"` is the list of options in the form `"BranchName=toto,Dataset=titi,System=tata, ..."`.

The following options __must__ be given:

 - `SimFileName`= name of file containing simulated events
 - `SimTitle`=   description of simulation
 - `BranchName`= name of branch containing simulated events
 - `Dataset`=    name of experimental dataset (defines multidetector array to use etc.)
 - `System`=     name of experimental dataset system (defines collision kinematics etc.)
 - `Geometry`=   type of geometry, either 'KV' (KaliVeda geometry) or 'ROOT' (ROOT TGeometry package)
 - `Filter`=     type of filter, either `'Geo'` (geometric filter), `'GeoThresh'` (geometry + detector thresholds),
             or `'Full'` (full simulation of detector response, including experimental identification
             and calibration routines)
 - `OutputDir`=  directory path in which to write filtered data file
 - `Kinematics`=  kinematical frame for simulation, either `"cm"` or `"lab"`. if `"cm"`, we use the c.m. velocity
                     of the selected `System` to transform events into the detector (laboratory) frame.
                     if `"lab"` no transformation is performed: simulated events are already in laboratory frame.

The following are optional options:

 - `Run`:        run number to use for detector status, setup, parameters, etc.
              if not given, first run of the given experimental system is used.
 - `PhiRot`:     by default, a random rotation around the beam axis will be performed before
              simulating detection of the event. If you don't want this to happen,
              give option PhiRot=no
              If used, filtered events will have a parameter "RANDOM_PHI" with the applied
              rotation (in radians)
 - `Gemini`:     if option Gemini=yes, then each event will be "decayed" with Gemini++,
              if KaliVeda has been compiled with Gemini++ support. See below for extra
              information on Gemini decay stored in particle parameter lists.
 - `GemDecayPerEvent`: if option Gemini=yes then by default 1 Gemini++ decay will be performed for each event.
                    you can change this by giving a value for this option

The filtered data will be written in the directory given as option "OutputDir".
The filename is built up from the original simulation filename and the values
of various options:
~~~~
      [simfile]_geo=[geometry]_filt=[filter-type]_[dataset]_[system]_run=[run-number].root
~~~~
The data will be stored in a TTree with name 'ReconstructedEvents', in a branch with name
'ReconEvent'. The class used for reconstructed events depends on the dataset,
it is given by KVDataSet::GetReconstructedEventClassName().

Each filtered event will have some or all of the following parameters in its list:

 - SIMEVENT_TREE_ENTRY = index of simulated event in TTree we are reading
 - SIMEVENT_NUMBER = event number of simulated event in TTree we are reading (if defined i.e. if !=0)
 - RANDOM_PHI = rotation around beam axis [in radians]

When using Gemini++ to decay simulated events before filtering, each particle will have
the following parameter defined:

 - GEMINI_PARENT_INDEX = index of parent nucleus in simulated event

The combination of SIMEVENT_TREE_ENTRY/_NUMBER and SIMEVENT_NUC allows to connect primary
fragments in the simulated events with their detected decay products in the filtered event.

Example: suppose 'recev' is a filtered event and 'sim_tree' is the TTree containing the original
simulation, while 'simev' is a pointer to a KVSimEvent object connected to the appropriate
branch in 'sim_tree'. Then we can retrieve the parent nucleus responsible for producing
a particular nucleus in 'recev' like so:
~~~~{.cpp}
    sim_tree.GetEntry( recev.GetParameters()->GetIntValue("SIMEVENT_TREE_ENTRY" ) );
    KVSimNucleus* parent_nuc = (KVSimNucleus*)simev->GetParticle( recev.GetNucleus(15)->GetParameters()->GetIntValue("GEMINI_PARENT_INDEX") );    // parent nucleus of 15th nucleus in filtered event 'recev'
~~~~

*/

class KVEventFiltering : public KVEventSelector {
#ifdef DEBUG_FILTER
   KVClassMonitor* memory_check;
#endif
   Long64_t fEVN;//event number counter
   Bool_t fRotate;//true if random phi rotation should be applied [default: yes]
#ifdef WITH_GEMINI
   Bool_t fGemini;//true if Gemini++ decay should be performed before detection [default: no]
   Bool_t fGemAddRotEner;//true if rotational energy has to be added to excitation energy [default: no]
   Int_t fGemDecayPerEvent;//number of Gemini++ decays to be performed for each event [default:1]
   KVSimEvent fGemEvent;//event after decay with Gemini
   KVGemini GEM;
#endif

   const char* fIdCalMode; //! original exp setup hasIDandCalib to be reset in case of modifications

   void RandomRotation(KVEvent* to_rotate, const TString& frame_name = "") const;
public:
   KVEventFiltering();
   KVEventFiltering(const KVEventFiltering&) ;
   virtual ~KVEventFiltering();
   void Copy(TObject&) const;

   Bool_t Analysis();
   void EndAnalysis();
   void EndRun();
   void InitAnalysis();
   void InitRun();
   void OpenOutputFile(KVDBSystem*, Int_t);

   TFile* fFile;
   TTree* fTree;
   KVReconstructedEvent* fReconEvent;
   TVector3 fCMVelocity;
   TVector3 fProjVelocity;
   Bool_t fTransformKinematics;//=kTRUE if simulation not in lab frame
   TString fNewFrame;   //allow the definition of a specific frame

   ClassDef(KVEventFiltering, 1) //Filter simulated events with multidetector response
};

#endif
