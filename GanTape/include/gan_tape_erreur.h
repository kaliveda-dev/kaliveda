/*****************************************************************************
 *                                                                           *
 *       Projet       :  Bibiotheque GANIL_TAPE                              *
 *                                                                           *
 *       Programme    :  gan_tape_erreur.h                                   *
 *                                                                           *
 *       Auteur       :  OPDEBECK                                            *
 *       Date         :  13 juin 1996                                        *
 *       Modifie le   :   8 juillet 1999                                     *
 *                                                                           *
 *       Objet        :  Fichier entete pour les erreurs.                    *
 *                                                                           *
 *****************************************************************************/

#ifndef __DEF_ERR_H
#define __DEF_ERR_H

#ifdef __cplusplus
extern "C" {
#endif
#if defined ( __VMS ) || defined ( VMS )

#include "acq_codes_erreur.h"

#elif defined ( __unix__ ) || ( __unix )

#define ACQ_OK             0
#define ACQ_ERRPARAM      -1
#define ACQ_ISNOTATAPE    -2
#define ACQ_NOTALLOC      -3
#define ACQ_ALRALLOC      -4
#define ACQ_NOTMOUNT      -5
#define ACQ_ALREADYMOUNT  -6
#define ACQ_ENDOFFILE     -7
#define ACQ_ENDOFTAPE     -8
#define ACQ_ERRDURREAD    -9
#define ACQ_DEVWRITLOCK   -10
#define ACQ_ENDOFBUFFER   -11
#define ACQ_INVDATABUF    -12
#define ACQ_RAFBUFOVF     -13
#define ACQ_ERREVNTLEN    -14
#define ACQ_INVARG        -15
#define ACQ_ERRDATABUF    -16
#define ACQ_BADEVENTFORM  -17

#else    /****************************************************************/
#error "Ce package n'est pas compatible avec votre OS"
#endif /*****************************************************************/


/* Erreurs implementees uniquement pour Exemple_relecture   */
/* Attention, ils doivent etre utilises a la fois sous UINIX et sous VMS */

#define ACQ_UNKBUF        -100 /* Type de buffer inconnu */
#define ACQ_UNKMSB        -101 /* MSB/LSB non determine en mode AutoSwapBuf */
#define ACQ_NOBEGINRUN    -102 /* File is not at begin of run */
#define ACQ_BADFILESTRUCT -103 /* Bad file structure */
#define ACQ_BADEVTSTRUCT  -104 /* Bad event structure in COMMENT block */
#define ACQ_BADRUNNUM     -105 /* Bad run number */
#define ACQ_STREVTTMP     -106 /* Open error on struct event temporary file */
#define ACQ_NOIMPLEMENTED -107 /* functionnality non implemented for this OS */
#define ACQ_INVSCALERBUF  -108 /* This buffer is not a scaler block */

void gan_tape_erreur(int, const char*);

#ifdef __cplusplus
}
#endif
#endif


