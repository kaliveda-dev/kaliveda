/*****************************************************************************
 *                                                                           *
 *       Projet       :  Bibiotheque GANIL_TAPE                              *
 *                                                                           *
 *       Programme    :  GAN_TAPE_TEST.h                                     *
 *                                                                           *
 *       Auteur       :  OPDEBECK                                            *
 *       Date         :  15 avril 1996                                       *
 *                       Modifie pour UNIX le 13 juin 1996                   *
 *                                                                           *
 *       Objet        :  Realisation des fonctions de base de gestion des    *
 *                    bandes : acq_dev_is_tape, get_dev_type,                *
 *                    create_descriptor                                      *
 *                                                                           *
 *****************************************************************************/


#ifndef __GAN_TAPE_TEST_H
#define __GAN_TAPE_TEST_H

#ifdef __cplusplus
extern "C" {
#endif
#include "gan_tape_general.h"

#if defined ( __VMS ) || defined ( VMS )            /***** Cas du VMS ******/

#include <descrip.h>

#define DESC struct dsc$descriptor_s

DESC create_descriptor(char*);

#endif


int acq_dev_is_tape_c(gan_tape_desc);
int acq_get_tape_type_c(gan_tape_desc, char*);
int acq_dev_is_alloc_c(gan_tape_desc);
int acq_dev_is_mount_c(gan_tape_desc);
int acq_dev_is_wr_protect_c(gan_tape_desc);
int acq_real_dev_name_c(gan_tape_desc, char*);
#ifdef __cplusplus
}
#endif

#endif

