//Created by KVClassFactory on Thu May  7 14:35:05 2015
//Author: John Frankland,,,

#ifndef __MCSAMPLER_H
#define __MCSAMPLER_H

#include "KVBase.h"
#include "KVTemplateEvent.h"
#include "TTree.h"
#include "TLegend.h"
#include "StatWeight.h"

namespace MicroStat {

   /**
      \class MCSampler
      \brief Monte-Carlo sampling of events with statistical weights
      \ingroup MicroStat \ingroup Simulation
   */

   class MCSampler : public KVBase {
   private:
      void init();
      StatWeight*              fLastPicked;//! weight of channel picked by call to PickRandomChannel()
      Double_t                 fLegendProbaMin;//!minimum probability for which channels are included in automatically generated TLegend when PlotProbabilities is called
      TLegend*                 fTheLegend;//!automatically generated legend for PlotProbabilities

      void SetBranch(TTree* theTree, const TString& name, void* variable, const TString& vartype);

   protected:
      Long64_t                 fPartitions;//! number of partitions in TTree/TChain
      TBranch*                 fBranch;    //! branch containing events
      KVEvent*                 fPartition;     //! event read from fPartitionList tree
      TClass*                  fWeight;    //! statistical weight class
      TClonesArray*            fWeightList;//! list of weights for all events
      Double_t                 fSumWeights;//! sum of all weights
      // variables for TTree branches
      Double_t                 ESTAR;//! the excitation energy (Exx)
      Double_t                 EDISP;//! the available kinetic energy
      Long64_t                 IPART;//! the partition index

      Bool_t                   fModifyMasses;//! if nuclear masses are modified

      void initialiseWeightList();

   public:
      MCSampler();
      MCSampler(const Char_t* name, const Char_t* title = "");
      virtual ~MCSampler();

      void SetEventList(TTree* t, const TString& branchname);
      void SetStatWeight(const TString&);

      KVEvent* GetPartition(Long64_t i)
      {
         // Return pointer to partition with index i in the TTree/TChain fPartitionList

         fBranch->GetEntry(i);
         return fPartition;
      }
      void SetModifyMasses(Bool_t yes = kTRUE)
      {
         fModifyMasses = yes;
      }
      void UpdateMasses();

      void CalculateWeights(Double_t excitation_energy);
      TClonesArray* GetWeights() const
      {
         return fWeightList;
      }
      StatWeight* GetWeight(Int_t i) const
      {
         return (StatWeight*)(*fWeightList)[i];
      }
      Double_t GetSumWeights() const
      {
         return fSumWeights;
      }

      Long64_t PickRandomChannel();
      StatWeight* GetRandomChannelWeight() const
      {
         return fLastPicked;
      }

      void SetUpTreeBranches(KVEvent*& event, TTree* theTree, const TString& bname);
      void GenerateEvents(TTree*, KVEvent* event, Double_t, Long64_t npartitions, Long64_t nev_part = 10);
      Bool_t SetExcitationEnergy(Double_t Exx);
      Bool_t SetDecayChannel();
      void GenerateEvent(TTree* theTree, KVEvent* event);

      void PlotMultiplicities(double emin = 0., double emax = 100., double estep = 1., Option_t* opt = "");
      void PlotProbabilities(double emin = 0., double emax = 100., double estep = 1., Option_t* opt = "");
      void SetLegendProbaMin(Double_t p)
      {
         // set minimum probability for which channels are included
         // in automatically generated TLegend when PlotProbabilities is called
         // this legend can be displayed by calling ShowLegend immediately
         // after the call to PlotProbabilities.
         // default is to include all channels.
         fLegendProbaMin = p;
      };
      Double_t GetLegendProbaMin() const
      {
         return fLegendProbaMin;
      };
      void ShowLegend()
      {
         // call immediately after a call to PlotProbabilities or PlotParticleMultiplicities
         // in order to display the automatically generated TLegend
         // containing all channels with probability > fLegendProbaMin
         // or all nuclei with multiplicity > fLegendMultMin
         // (set previously by call to SetLegendProbaMin/SetLegendMultMin).
         if (fTheLegend) fTheLegend->Draw();
      };

      ClassDef(MCSampler, 1) //Monte-Carlo sampling of events with statistical weights
   };

}
#endif
