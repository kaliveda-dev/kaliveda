//Created by KVClassFactory on Fri Sep 11 23:31:56 2020
//Author: henri

#ifndef __KVSIMREADER_DIT_H
#define __KVSIMREADER_DIT_H

#include "KVSimReader.h"
#include "KVString.h"

class KVSimReader_DIT : public KVSimReader {
public :

   enum DITCollType {
      DITCollError       = -1,  /**< An error occured, the trajectory must be replayed */
      DITCollOk          = 0,  /**< Normal DIT collision */
      DITCollFusionType1 = 1,  /**< Fusion occured and the collective energy(Ekin + Pot) < 1 MeV */
      DITCollFusionType2 = 2,  /**< Fusion occured and a forbidden region for collective motion is reached */
      DITCollFusionType3 = 3,  /**< Fusion occured and the system is trapped in the interacction potential */
      DITCollFusionType4 = 4,  /**< Fusion occured and the system is too compact ( SMIN the minimal distance of approach(fm) between nuclear surfaces is lower than -3 fm) */
   } ;


   KVSimReader_DIT(KVString, KVString, KVString, Double_t);

   virtual ~KVSimReader_DIT();

   void ConvertEventsInFile(KVString);
   void ReadFile();
   Bool_t ReadEvent();

   void StoreEventInVariables();
   void FillEvent();


private :

   void InitReader();
   void SkipFirstLine();


   KVString projectile;  //!< Projectile of the collision in the format '58Ni'
   KVString target;  //!< Projectile of the collision in the format '64Ni'

   Double_t energy;  //!< Projectile energy in MeV/A
   Double_t weight;  //!< Weight (?) of the collision
   Double_t inicident_partial_wave;  //!< Incident partial wave in hbar (to determine the impact parameter) in the center of mass
   Double_t impact_parameter;  //!< Impact parameter of the collision
   Double_t excitation_energy_proj_like;  //!< Projectile like excitation energy (thermal + rotational) in MeV
   Double_t excitation_energy_targ_like;  //!< Target like excitation energy (thermal + rotational) in MeV
   Double_t spin_proj_like;  //!< Projectile like spin in hbar (only components normal to the reaction plane)
   Double_t spin_targ_like;  //!< Target like spin in hbar (only components normal to the reaction plane)
   Double_t energy_proj_like;  //!< Projectile like energy in the laboratory frame in MeV
   Double_t energy_targ_like;  //!< Target like energy in the laboratory frame in MeV
   Double_t theta_proj_like;  //!< Projectile like theta scattering angle in the laboratory frame in degrees
   Double_t theta_targ_like;  //!< Target like theta scattering angle in the laboratory frame in degrees
   Double_t theta_cm;  //!< Center of mass theta scattering angle in degrees
   Double_t q_reaction;  //!< Reaction Q value in MeV
   Double_t min_dist;  //!< Minimal distance of approach(fm) between nuclear surfaces

   Int_t itype;  //!< Collision type
   Int_t a_proj_like;  //!< Projectile like mass number
   Int_t a_targ_like;  //!< Target like mass number
   Int_t z_proj_like;  //!< Projectile like charge number
   Int_t z_targ_like;  //!< Target like charge number
   Int_t k_n;  //!< Number of neutrons exchanged
   Int_t k_p;  //!< Number of protons exchanged

   ClassDef(KVSimReader_DIT, 1) //
};

#endif

