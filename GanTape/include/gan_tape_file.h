/*****************************************************************************
 *                                                                           *
 *       Projet       :  Bibiotheque GANIL_TAPE                              *
 *                                                                           *
 *       Programme    :  GAN_TAPE_FILE.h                                     *
 *                                                                           *
 *       Auteur       :  OPDEBECK                                            *
 *       Date         :  15 avril 1996                                       *
 *                       Modifie pour UNIX le 13 juin 1996                   *
 *                                                                           *
 *       Objet        :  Realisation des fonctions de base de gestion des    *
 *                    bandes : acq_mt_open,acq_mt_read, acq_mt_write, et     *
 *                    skip.                                                  *
 *                                                                           *
 *****************************************************************************/


#ifndef __GAN_TAPE_FILE_H
#define __GAN_TAPE_FILE_H

#ifdef __cplusplus
extern "C" {
#endif
#include "gan_tape_general.h"


int acq_mt_open_c(gan_tape_desc*, mode_r_w, int*);
int acq_mt_close_c(gan_tape_desc);
int acq_mt_read_c(int, char*, int*);
int acq_mt_write_c(int, char*, int);


int acq_mt_rewind_c(gan_tape_desc);
int acq_mt_skip_file_c(gan_tape_desc, int);
int acq_mt_skip_block_c(gan_tape_desc, int);
int acq_mt_skip_to_eot_c(gan_tape_desc, int*);

#ifdef __cplusplus
}
#endif

#endif
