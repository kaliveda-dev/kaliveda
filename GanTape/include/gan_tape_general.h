/*****************************************************************************
 * Fichier     : gan_tape_general.h                                 *
 * Programme   :                                                    *
 *                                                                           *
 * Auteur      : OPDEBECK                                           *
 * Date     ; 05 Avril 1996                                      *
 *                                                                           *
 * Objet    : Types et macros d'usage general.                   *
 *
 *      Modifications :
 *           B. Raine le 14/4/99
 *             define true, false conditionne
 *           B. raine 22 avril 04,
 *             on definit le type BOOL pour compatibilte C/C++
 *             dans tous les environnements ( a cause du CC de Solaris)
 *****************************************************************************/

#ifndef __GAN_TAPE_GENERAL_H
#define __GAN_TAPE_GENERAL_H

#ifdef __cplusplus
extern "C" {
#endif
#include <assert.h>


#if defined ( __VMS ) || defined ( VMS )
#include <lib$routines.h>
#endif

/* #define NDEBUG         */

#define MALLOC(x) ((x*) malloc(sizeof(x)))
#define CALLOC(n,x) ((x*) calloc(n,sizeof(x)))

#define ERROR 0x0000FFFF

#define BOT        1
#define WR_PROTECT 8

#define MAX_CARACTERES 160  /* Taille std des chaines utilisees */
#define DVI_TAILLE     64  /* Taille des chaines retournees par lib$getdvi */

#define MAX_INT16 32767
#define MIN_INT16 -32768

#define UNLOAD  1
#define NOUNLOAD 2

/* B. raine 22 avril 04,
   on definit le type BOOL pour compatibilte C/C++
   dans tous les environnements
*/
#ifndef __cplusplus
#ifndef true
typedef enum BOOLEEN { false, true } bool;
#endif
#endif
/* pour test typedef bool BOOL;*/
typedef int BOOL;


typedef enum MODE_R_W { o_read, o_write } mode_r_w;

/* Definition du descripteur fourni aux fonctions  */

typedef struct GAN_TAPE_DESC {

   char DevName[MAX_CARACTERES];
   int Lun;
   bool Is_protected;

} gan_tape_desc;

#ifdef __cplusplus
}
#endif

#endif

