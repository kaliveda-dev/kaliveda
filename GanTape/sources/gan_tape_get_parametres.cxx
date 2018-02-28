/*****************************************************************************
 *                                                                           *
 *       Projet       :  Bibiotheque GANIL_TAPE                              *
 *                                                                           *
 *       Programme    :  GET_PARAMETRES.c                                    *
 *                                                                           *
 *       Auteur       :  OPDEBECK                                            *
 *       Date         :  12 mars 1997                                        *
 *                                                                           *
 *       Objet        :  Fonction acq_get_param_env ( .. ) associee a        *
 *       get_parameters.h pour la recuperation des parametres *
 *       dynamiquement. Reutilisable pour d'autres projets.   *
 *                                                                           *
 *****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>

#include "gan_tape_get_parametres.h"
#include "gan_tape_param.h"
#include "gan_tape_general.h"
#include "GanTape/GEN_TYPE.H"
#include "GanTape/gan_acq_buf.h"

#include "gan_tape_erreur.h"


/*****************************************************************************
 *                                                                           *
 *           Function  : acq_get_param_env                                   *
 *           Objet     : Fournit les variables de parametrage dans un format *
 *                      correct.                                             *
 *                                                                           *
 *           Entree    : Type_param -- Type de parametre demande.         *
 *                       Retour     -- Parametre retourne.                   *
 *        Nb_lc      -- passage de argc.             *
 *        Chaine_lc  -- passage de argv.             *
 *                                                                           *
 *           Retour    : Status -- Etat d'execution de la commande           *
 *                              - ACQ_OK si aucun probleme.                  *
 *          - ACQ_ERRPARAM si parametre entree invalide. *
 *                              - -1 si parametre pris par defaut,chaine     *
 *          dans Retour.                 *
 *                                                                           *
 *****************************************************************************/

int acq_get_param_env(param_id Type_param, val_ret* Retour, int Nb_lc,
                      char** Chaine_lc)
{


   param_struct parametres[] = {

      device_id,  "gt_device",   caract,     0,
      ctrlform_id,   "gt_ctrlform", entier,     CTRLFORM,
      autoswbuf_id,  "gt_autoswapbuf", logic,    AutoSwapBuf,
      swbufon_id, "gt_swapbufon",   logic,      SwapBufOn,
      density_id, "gt_density",  entier,     DENSITY,
      bufsize_id, "gt_bufsize",  entier,     BUFSIZE,
      evbsize_id, "gt_evbsize",  entier,     EVBSIZE,
      evcsize_id, "gt_evcsize",  entier,     EVCSIZE,
      lun_id,     "gt_lun",   entier,     0
   };

   param_struct* Struct_parametre = parametres;
   bool Suite = true;
   char* Chaine_caract, Chaine[MAX_CARACTERES];
   int Entier_ret, Index, Status, Taille_chaine;
   bool Logic_ret;

   /* Des le depart, on se place sur le parametre desire */
   Struct_parametre = Struct_parametre + Type_param;

   Taille_chaine = strlen(Struct_parametre -> Nom_param);


   /* Ici on recupere les parametres sur la ligne de commande  */
   for (Index = 1 ; Index < Nb_lc && Suite == true ; Index++) {

      sprintf(Chaine, "%s", Chaine_lc[Index]);
      /* On cherche maintenant la chaine du param voulu */
      if (strncmp(Chaine, Struct_parametre -> Nom_param,
                  Taille_chaine) == 0) {
         strcpy(Chaine, (Chaine + Taille_chaine + 1));
         Suite = false;
      } else
         Suite = true;
   }

   /* Ici on les recupere en variable d'environnement */
   if (Suite == true) {

      Chaine_caract = getenv((const char*)(Struct_parametre->Nom_param));
      if (Chaine_caract == NULL)
         Suite = true;
      else {
         strcpy(Chaine, Chaine_caract);
         Suite = false;
      }

   }

   /* En dernier recours, on applique les valeurs par defaut puis on renvoi*/
   /* sous le bon format.                    */

   /* Teste s'il s'agit de DEVICE pour renvoyer une erreur  */
   if (Type_param == device_id && Suite == true) {
      Status = ACQ_ERRPARAM;
      return (Status);
   }

   if (Struct_parametre->Typ_param == entier) {
      if (Suite == true) {   /* Prend valeur par defaut */
         Retour->Val_INT = Struct_parametre->Val_defaut;
         Status = -1;
      } else {
         if (Type_param == ctrlform_id) {
            if (strcmp(Chaine, "evct_var") == 0)
               Retour->Val_INT = EVCT_VAR;
            else Retour->Val_INT = EVCT_FIX;
         } else {
            Retour->Val_INT = atoi(Chaine);
            Status = ACQ_OK;
         }
      }
   }

   else if (Struct_parametre->Typ_param == logic) {
      if (Suite == true) {
         Logic_ret = Struct_parametre->Val_defaut;
         Status = -1;
      } else {
         if (strcmp(Chaine, "true") == 0)
            Logic_ret = true;
         else Logic_ret = false;
         Status = ACQ_OK;
      }
      Retour->Val_BOL = Logic_ret;
   }

   else {
      strcpy(Retour->Val_CAR, Chaine);
      Status = ACQ_OK;
   }

   return (Status);

}
/************************************ END ************************************/
