/*****************************************************************************
 *                                                                           *
 *       Projet       :  Bibiotheque GANIL_TAPE                              *
 *                                                                           *
 *       Programme    :  GAN_TAPE_ALLOC.h                                    *
 *                                                                           *
 *       Auteur       :  OPDEBECK                                            *
 *       Date         :  15 avril 1996                                       *
 *                       Modifie pour UNIX le 13 juin 1996                   *
 *                                                                           *
 *       Objet        :  Realisation des fonctions de base de gestion des    *
 *                    bandes : acq_mt_alloc                                  *
 *                                                                           *
 *****************************************************************************/


#ifndef __GAN_TAPE_ALLOC_H
#define __GAN_TAPE_ALLOC_H

#ifdef __cplusplus
extern "C" {
#endif
#include "gan_tape_general.h"


int acq_mt_alloc_c(gan_tape_desc);
int acq_mt_dealloc_c(gan_tape_desc);

#ifdef __cplusplus
}
#endif
#endif

