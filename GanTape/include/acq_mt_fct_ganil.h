/*****************************************************************************
 *                                                                           *
 *       Projet       :  Bibiotheque GANIL_TAPE                              *
 *                                                                           *
 *       Programme    :  ACQ_MT_FCT_GANIL.h                                  *
 *                                                                           *
 *       Auteur       :  OPDEBECK                                            *
 *       Date         :  16 juillet 1996                                     *
 *                                                       *
 *       Objet        :  Source des differentes fonctions de traitement et   *
 *       de relecture de buffer fournies par le GANIL pour    *
 *                      les environnements VMS et UNIX --> Prototypes.       *
 *                                                                           *
 *****************************************************************************/

#ifndef __ACQ_FCT_GANIL_H
#define __ACQ_FCT_GANIL_H

#include "GanTape/GEN_TYPE.H"
#include "GanTape/gan_acq_buf.h"


#ifdef __cplusplus
extern "C" {
#endif
/*    Prototype de acq_mt_ini_run_c (..) , fonction permettant d'initialiser
 * le contexte du run. Cette fonction se trouve dans le fichier ACQ_MT_INI_RUN.C.
 */
int acq_mt_ini_run_c(int, in2p3_buffer_struct*, int, int*, char*, int);


/*    Prototype de get_next_event (..) , fonction permettant d'obtenir le
 * parametre suivant dans un buffer.
 */
int get_next_event(in2p3_buffer_struct*, int, short int*, int,
                   int*);


/*    Prototype de get_next_event (..) , fonction permettant d'obtenir le
 * parametre suivant dans un buffer.
 */
int get_next_param(int, char*, int, int, int);




/*  Attention, les sources des fonctions suivantes sont places dans des
 * fichiers portant le meme nom, a inclure lors de la compilation. Le passage
 * des parametres est conditionne poar le fonctionnement meme de ces fonctions.
 */


/*    Prototype de rd_evstr (..) , fonction permettant de placer un evenement
 * en memoire. Cette fonction se trouve dans le fichier RD_EVSTR.C.
 */
int rd_evstr(char* sev_filnam, char* sev_memadd, int sev_memlen);


/* Prototype de s_evct0 (..) , fonction permettant de transformer un
 * evenement brut en un evenement de control.
 */
int s_evctrl(INT16* evtbuf, INT16* evtctrl, char* sev_memadd, int* pCtrlForm);


#ifdef __cplusplus
}
#endif
#endif
