/*****************************************************************************
 *****************************************************************************
 *                                                                           *
 *       Projet       :  Bibiotheque GANIL_TAPE                              *
 *                                                                           *
 *       Programme    :  GAN_TAPE_ALLOC.c                                    *
 *                                                                           *
 *       Auteur       :  OPDEBECK                                            *
 *       Date         :  22 avril 1996                                       *
 *                       Modifie pour UNIX le 13 juin 1996                   *
 *                                                                           *
 *       Objet        :  Fonctions de gestion des bandes sous VMS ou U~~X.   *
 *       Fonctions    :  Alloc, DeAlloc                                      *
 *                                                                           *
 *****************************************************************************
 *****************************************************************************/


#include <stdlib.h>
#include <stdio.h>

#include "gan_tape_alloc.h"  /*   !! Les chemins d'appel des fichiers */
#include "gan_tape_test.h"   /* entete doivent etre definis lors de   */
#include "gan_tape_general.h"         /* l'appel du compilateur.      */

#include "gan_tape_erreur.h"


#if defined ( __VMS ) || defined ( VMS )     /***** Cas de VMS ******/

#include <ssdef.h>

extern int sys$alloc();   /* Fonction VMS d'allocation      */
extern int sys$dalloc();  /* Fonction VMS de deallocation   */

#endif

/*****************************************************************************
 *                                                                           *
 *           Function  : acq_mt_alloc_c                                      *
 *           Objet     : Allocation d'un device au precessus appelant.       *
 *                                                                           *
 *           Entree    : DeviceName -- Nom du device sous forme d'une chaine *
 *                       de caracteres sous VMS et sous UNIX.                *
 *                                                                           *
 *           Retour    : Status -- Etat d'execution de la commande           *
 *                              - ACQ_OK si aucun probleme.                  *
 *                              - ACQ_ISNOTATAPE si le device est un disque. *
 *                              - ACQ_NOTALLOC si le device est alloue a un  *
 *                                autre processus.                           *
 *                              - Code d'erreur VMS ou UNIX si une autre     *
 *                                erreur est intervenue.                     *
 *                                                                           *
 *****************************************************************************/

int acq_mt_alloc_c(gan_tape_desc DeviceName)
{

   int Status;

#if defined ( __VMS ) || defined ( VMS )

   DESC Descr_In;
   Descr_In = create_descriptor(DeviceName.DevName);

   if (acq_dev_is_tape_c(DeviceName) == ACQ_OK) {
      Status = sys$alloc(&Descr_In, 0, 0, 0, 0);

      if (Status == SS$_NORMAL ||
            Status == SS$_BUFFEROVF ||
            Status == SS$_DEVALRALLOC)

         Status = ACQ_OK;

      else if (Status == SS$_DEVALLOC)

         Status = ACQ_NOTALLOC;
   } else Status = ACQ_ISNOTATAPE;


#elif defined ( __unix__ ) || ( __unix )

   Status = ACQ_OK;

#else    /****************************************************************/
#error "Ce package n'est pas compatible avec votre OS"
#endif /*****************************************************************/


   return (Status);

}


/*****************************************************************************
 *                                                                           *
 *           Function  : acq_mt_dealloc_c                                    *
 *           Objet     : De-allocation d'un device du precessus appelant.    *
 *                                                                           *
 *           Entree    : DeviceName -- Nom du device sous forme d'une chaine *
 *                       de caracteres sous VMS et sous UNIX.                *
 *                                                                           *
 *           Retour    : Status -- Etat d'execution de la commande           *
 *                              - ACQ_OK si aucun probleme.                  *
 *                              - ACQ_ISNOTATAPE si le device est un disque. *
 *                              - ACQ_NOTALLOC si le device est alloue a un  *
 *                                autre processus.                           *
 *                              - Code d'erreur VMS ou UNIX si une autre     *
 *                                erreur est intervenue.                     *
 *                                                                           *
 *****************************************************************************/


int acq_mt_dealloc_c(gan_tape_desc DeviceName)
{


   int Status;

#if defined ( __VMS ) || defined ( VMS )

   DESC Descr_In;
   Descr_In = create_descriptor(DeviceName.DevName);

   if (acq_dev_is_tape_c(DeviceName) == ACQ_OK) {
      Status = sys$dalloc(&Descr_In, 0, 0, 0, 0);

      if (Status == SS$_NORMAL)
         Status = ACQ_OK;

      else if (Status == SS$_DEVNOTALLOC)
         Status = ACQ_NOTALLOC;

   } else Status = ACQ_ISNOTATAPE;

#elif defined ( __unix__ ) || ( __unix )

   Status = ACQ_OK;

#else    /****************************************************************/
#error "Ce package n'est pas compatible avec votre OS"
#endif /*****************************************************************/

   return (Status);

}

/********************************* END **************************************/
