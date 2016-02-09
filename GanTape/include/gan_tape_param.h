/*****************************************************************************
 *                                                                           *
 * Fichier     : GAN_TAPE_PARAM.H                                   *
 * Programme   :                                                    *
 *                                                                           *
 * Auteur      : OPDEBECK                                           *
 * Date     : 03 Mai 1996                                        *
 * Modifie le  : 17 juillet pour les definitions de buffer.         *
 *                        B. Raine le 3/12/96
 *                           Les definitions de buffers sont mise dans       *
 *                           acq_gan_buf.h                                   *
 * Objet    : Tailles des differents buffers a ajuster.          *
 *                      Tous les parametres peuvent etre modifies a volonte  *
 *       mais en conservant une coherence.                    *
 *
 *          Modifications :
 *             B. Raine le 23 oct 98
 *               On supprime la definition du type d'architecture, car test
 *               automatique de l'architecture dans package gan_acq_swap_buf
 *                                                                           *
 ****************************************************************************/

#ifndef __GAN_TAPE_PARAM_H
#define __GAN_TAPE_PARAM_H

#ifdef __cplusplus
extern "C" {
#endif

#define DENSITY   6250
#define BUFSIZE   16384
#define EVBSIZE   8192
#define EVCSIZE   8192
#define STRUCTEVENTSIZE 8192

#define MIN_DENSIT 800
#define MED_DENSIT 1600
#define MAX_DENSIT 6250

#define MAX_SKIP_BLOCK 32767
#define MIN_SKIP_BLOCK -32768

#define MAX_SKIP_FILE 32767    /* ??? Pas raisonnable !!! */
#define MIN_SKIP_FILE -32768

/* Definition du format de reconstruction des evenements de control
   ----------------------------------------------------------------
 CTRLFORM est utilise comme option de reconstruction pour definir le
   format des evenements de control au standard GANIL desires :
      EVCT_FIX pour evenement de control de taille fixe,
      EVCT_VAR pour evenement de control variable */

#define CTRLFORM EVCT_VAR  /* Format variable pour evenement de control */


/* Definition des options de swap
   ------------------------------
   Si AutoSwapBuf est defini a TRUE:
      les buffers sont swappes automatiquement, lorsqu'ils sont lus par la
      fonction acq_mt_read_c, en fonction de la structure des mots machine et
      des informations ecrites sur la bande dans le bloc FILEH (MSB ou LSB).
      Ce champ est mis a jour lors de la creation des bandes d'acquisition
      GANIL. Dans ce cas la valeur de SwapBufOn est ignoree
   Si AutoSwapBuf est defini a FALSE:
      les buffers seront swappes en fonction de la valeur de SwapBufOn sans
      se preoccuper de l'architecture de la machine ni des informations
      contenues dans le bloc FILEH.
      SwapBufOn = TRUE => on swappe
                  FALSE=> pas de swap
*/
#define AutoSwapBuf TRUE     /* Option swap automatique */
#define SwapBufOn   FALSE    /* Flag ignore si AutoSwapBuf est a TRUE */


#ifdef __cplusplus
}
#endif


#endif






