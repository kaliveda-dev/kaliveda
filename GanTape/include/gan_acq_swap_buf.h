/*****************************************************************************
 *                                                                           *
 * Fichier     : gan_acq_swap_buf.h                                 *
 *                                                                           *
 * Auteur      : Bruno Raine                                        *
 * Date     : 03 dec 96                                          *
 * Modifie le  : 7 juin 1999 par F. Saillant (ajout SwapBufToHost)  *
 *                                                                           *
 * Objet    : Description du package gan_acq_swap_buf            *
 *      Remarques       : Necessite d'inclure avant ce fichier               *
 *                           GEN_TYPE.H et gan_acq_buf.h                     *
 *                                                                           *
 ****************************************************************************/
#ifndef __GanAcqSwapBuf_H
#define __GanAcqSwapBuf_H

#ifdef __cplusplus
extern "C" {
#endif
/* Definition des prototypes */

/***************************************************************************************/
/* Transforme un buffer pour qu'il soit correct pour la machine sur laquelle on tourne */
/***************************************************************************************/

int SwapBufToHost(char* Buff, int Size);

/* Swappe les buffers d'acquisition GANIL en fonction de leur type et des
   macros d'options definies (en principe dans gan_acq_param.h) */
int acq_swap_buf(in2p3_buffer_struct* Buff, int Size);

/* Routines a usage general pour swapper des entiers 32 et 16 bits
   Attention pour les deux fonctions qui suivent, aucun control n'est fait
   sur la valeur de NbOctets. Ce doit etre un multiple de 4 pour SwapInt32 et
   un multiple de 2 pour SwapInt16 */
void SwapInt32(UNSINT32* Buf, int NbOctets);
void SwapInt16(UNSINT16* Buf, int NbOctets);
void SwapDoubleWord(UNSINT32* Buf, int NbOctets);

#ifdef __cplusplus
}
#endif
#endif
