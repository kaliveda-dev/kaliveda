/*****************************************************************************
 *                                                                           *
 *       Projet       :  Bibiotheque GANIL_TAPE                              *
 *                                                                           *
 *       Programme    :  GAN_TAPE_MOUNT.h                                    *
 *                                                                           *
 *       Auteur       :  OPDEBECK                                            *
 *       Date         :  15 avril 1996                                       *
 *                       Modifie pour UNIX le 13 juin 1996                   *
 *                                                                           *
 *       Objet        :  Realisation des fonctions de base de gestion des    *
 *                    bandes : acq_mt_mount                                  *
 *                                                                           *
 *****************************************************************************/


#ifndef __GAN_TAPE_MOUNT_H
#define __GAN_TAPE_MOUNT_H

#ifdef __cplusplus
extern "C" {
#endif
#include "gan_tape_general.h"

#if defined ( __VMS ) || defined ( VMS )            /***** Cas du VMS ******/

typedef struct ITEM_LIST {   /* Structure de passage de parametres pour la */
   /* fonction sys$mount.               */
   short Buf_Len, Item_Code;
   void*  Buf_Adr;
   int*   Return_Adr;

} item_list;

#endif


int acq_mt_mount_c(gan_tape_desc, int, int);
int acq_mt_dismount_c(gan_tape_desc, int);

#ifdef __cplusplus
}
#endif

#endif
