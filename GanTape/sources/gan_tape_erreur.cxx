/*****************************************************************************
 *                                                                           *
 *       Projet       :  Bibiotheque GANIL_TAPE                              *
 *                                                                           *
 *       Programme    :  gan_tape_erreur.c                                   *
 *                                                                           *
 *       Auteur       :  B. Raine                                            *
 *       Date         :  6 dec 1996                                          *
 *                                                       *
 *                                                                           *
 *       Objet        : affichage des messages d'erreurs du package          *
 *                      de relecture de bandes compilable sous               *
 *       VMS et sous UNIX.                                    *
 *       Modifications :                                                     *
 *           8 juillet 99
 *                                                                           *
 *****************************************************************************/

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "GanTape/GEN_TYPE.H"
#include "gan_tape_erreur.h"

#if defined ( __VMS ) || defined ( VMS )
#include <ssdef.h>
#include <lib$routines.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct err_struct {
   int ErrNum;
   char ErrMes[256];
} ERR_STRUCT;

static ERR_STRUCT GanTapeErr[] = {
   ACQ_ERRPARAM, "parameter error",
   ACQ_ISNOTATAPE, "the name you enter doesn't represent a storage device",
   ACQ_NOTALLOC, "this storage device is not currently yours",
   ACQ_ALRALLOC, "this storage device is already yours",
   ACQ_NOTMOUNT, "the tape on which you want to work is not mounted",
   ACQ_ALREADYMOUNT, "the tape you want to mount is already mounted",
   ACQ_ENDOFFILE, "attempt to read past end of file",
   ACQ_ENDOFTAPE, "attempt to read past end of tape",
   ACQ_ERRDURREAD, "error occurs while reading a tape or disk file",
   ACQ_DEVWRITLOCK, "This device is write-locked",
   ACQ_UNKBUF, "Type de buffer inconnu ",
   ACQ_UNKMSB, "MSB/LSB non detremine en mode AutoSwapBuf",
   ACQ_NOBEGINRUN, "File is not at begin of run",
   ACQ_BADFILESTRUCT, "Bad file structure",
   ACQ_BADEVTSTRUCT, "Bad event structure in COMMENT block",
   ACQ_BADRUNNUM, "Bad run number",
   ACQ_STREVTTMP, "Open error on struct event temporary file",
   ACQ_ENDOFBUFFER, "The buffer doesn't contain any other event",
   ACQ_INVDATABUF, "This buffer is not a data buffer (not EVENTDB header)",
   ACQ_RAFBUFOVF, "Overflow of the refresh buffer",
   ACQ_ERREVNTLEN, "Error detected in the control event length",
   ACQ_INVARG, "bad argument",
   ACQ_ERRDATABUF, "An error was found in this data buffer",
   ACQ_BADEVENTFORM, "unknown event format",
   ACQ_INVSCALERBUF, "invalid scaler block (not SCALER header)",
   ACQ_OK, ""
};

void gan_tape_erreur(int Status, const char* message)
{
   ERR_STRUCT* pErr;
   int Cont = TRUE;

   /* On commence par traiter errno ...   */
   if (errno != 0 && Status != ACQ_OK) {
      printf("\n >> ERREUR :  ");
      perror("");
      printf(" dans fonction %s ...\n", message);
   }
   /* B. Raine le 14 oct 97
     On commente le else if car si errno n'est pas connu de l'OS, on
      ne sait pas pourquoi on a une erreur =>  else if ( Status != ACQ_OK ) { */

   if (Status != ACQ_OK) {

      /* B. Raine le 14 oct 97
         On remplace le test sur cont par un break car sinon l'indice pErr est
         faux; il vaut +1 */
      for (pErr = GanTapeErr; (pErr->ErrNum != ACQ_OK); pErr++) {
         if (pErr->ErrNum == Status)
            break;
      }


      printf("\n >>ERREUR %d dans fonction %s :\n       ", Status, message);
      if (pErr->ErrNum == ACQ_OK) {
#if defined ( __VMS ) || defined ( VMS ) /*******************************/
         /* si vms et code d'erreur inconnu, alors on appel lib$signal */
         if (pErr->ErrNum == ACQ_OK)
            lib$signal(Status & 0xfffffff9);
         else
#endif /*****************************************************************/
            printf("Code d'erreur inconnu");
      } else
         printf("< %s > ", pErr->ErrMes);

   }

}

#ifdef __cplusplus
}
#endif
