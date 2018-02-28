/*****************************************************************************
 *                                                                           *
 *       Projet       :  Bibiotheque GANIL_TAPE                              *
 *                                                                           *
 *       Programme    :  ACQ_MT_FCT_GANIL.c                                  *
 *                                                                           *
 *       Auteur       :  OPDEBECK                                            *
 *       Date         :  16 juillet 1996                                     *
 *                                                       *
 *                                                                           *
 *       Objet        :  Source des differentes fonctions de traitement et   *
 *       de relecture de buffer fournies par le GANIL pour    *
 *                      les environnements VMS et UNIX.                      *
 *                                                                           *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gan_tape_file.h"
#include "gan_tape_general.h"

#include "acq_mt_fct_ganil.h" /* Prototypes bibliotheque EXEMPLE_RELECTURE */

#include "ERR_GAN.H" /* Pour definir le code retour de rd_evstr.c */
#include "gan_tape_erreur.h"  /* Fichier d'erreurs */

#if defined ( __VMS ) || defined ( VMS ) /*******************************/

#include <libdef.h>
#include <lib$routines.h>
#include <ssdef.h>

#include <unixio.h>
#include <file.h> /* Pour la gestion du fichier temporaire. */


#elif defined ( __unix__ ) || (__unix)  /***********************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>


#else    /****************************************************************/
#error "Ce package n'est pas compatible avec votre OS"
#endif /*****************************************************************/


#define FICHIER_STR "evt_str.tmp"

/*****************************************************************************
 *                                                                           *
 *           Function  : acq_mt_ini_run_c                                    *
 *           Objet     : Initialisation du contexte de relecture d'une bande.*
 *                       !! La bande en question doit etre allouee, montee   *
 *                       et ouverte.                                         *
 *                                                                           *
 *           Entree    : Lun  -- Numero d'unite logique du fichier ouvert.   *
 *                       Buff -- Buffer de lecture des donnees.              *
 *                       Size -- Taille du buffer de lecture.                *
 *                       RunNumber -- Numero de run attendu. Il s'agit de    *
 *                         l'adrresse de la variable afin de retourner le    *
 *                         no de run effectivement lu.                       *
 *                       StrEvent[] -- Buffer de mise en memoire.            *
 *                       StrEventSize -- Taille du buffer precedent.         *
 *                                                                           *
 *           Retour    : Status -- Etat d'execution de la commande.          *
 *        .. Ces codes peuvent changer de denomination ..     *
 *                           - ACQ_OK si aucun probleme.                     *
 *                           - ACQ_NOBEGINRUN si la bande n'est pas en debut *
 *                                            de run.                        *
 *                           - ACQ_BADFILESTRUCT si mauvaise structure de    *
 *                                            device.                        *
 *                           - ACQ_BADEVTSTRUCT si mauvaise structure d'evt. *
 *                           - ACQ_BADRUNNUM si mauvais numero de run.       *
 *                           - ACQ_STREVTTMP si problem d'acces au fichier   *
 *                                   temporaire de structure evenement       *
 *                           - si autre erreur. ( contenue dans errno )      *
 *                                                                           *
 ****************************************************************************/



int acq_mt_ini_run_c(int Lun, in2p3_buffer_struct* Buff, int Size,
                     int* RunNumber, char* StrEvent, int StrEventSize)
{

   void libere_fichier(int);

   char Header[9];      /* Header formate en string version 'C'.     */
   char Date[21];    /* Date formate en string version 'C'.     */
   int Size_tempo, Boucle = 0, Status, Bloc_number = 0, Valeur;
   int Nb_struct_bloc, Lun_tempo, Decompte, Debut, Depart, Quant;
   bool Continue = true;


   if (StrEventSize > 32767)
      StrEventSize = 32767;

   /* Sequence de verification visuelle      */
   printf("\n>>> Sequence inirun :        Lun = %d\n"
          "                             rec = %d\n"
          "                             size= %d\n",
          Lun, Size, StrEventSize);

   /* Lecture d'un bloc sur bande   --> Lecture aveugle  */
   while (Continue) {

      Bloc_number++;
      Size_tempo = Size;
      Status = acq_mt_read_c(Lun, Buff->Buffer, &Size_tempo);
      if (Status != ACQ_OK) {
         printf("\n>>> Erreur de lecture du bloc no %d",
                Bloc_number);
         return (Status);
      } else {
         strncpy(Header, Buff->les_donnees.Ident, 8);
         Header[8] = '\0';     /* Pour delimiter la 'string' */
      }

      if (strcmp(Header, FILEH_Id) == 0)
         Continue = true;
      else if (strcmp(Header, EVENTH_Id) == 0)
         Continue = false;
      else if (Bloc_number == 2) {
         Status = ACQ_BADFILESTRUCT;
         return (Status);
      } else {
         Status = ACQ_NOBEGINRUN;
         return (Status);
      }

   }  /* En sortie du while, le buffer doit contenir un "EVENTH".*/

   /* Traitement du buffer de type "EVENTH"        */
   /* - Recuperation du no de run               */
   strncpy(Header, Buff->les_donnees.cas.Buf_eventh.NumeroRun, 8);
   Header[8] = '\0';
   Valeur = atoi(Header);    /* Converti un string entier */
   if (*RunNumber == 0)
      *RunNumber = Valeur;
   else if (*RunNumber != Valeur) {
      Status = ACQ_BADRUNNUM;
      return (Status);
   } /* A ce niveau, Le numero de  run est correct    */

   /* - Recuperation de la date & l'heure du run   */
   strncpy(Date, Buff->les_donnees.cas.Buf_eventh.Date, 20);
   Date[20] = '\0';
   printf("\n>>> Date du run : %20s\n", Date);

   /* - Recuperation du nombre de blocs " COMMENT"        */
   strncpy(Header, Buff->les_donnees.cas.Buf_eventh.NbBlocStrEvt, 4);
   Header[4] = '\0';
   Valeur = atoi(Header);
   Nb_struct_bloc = Valeur;

   /* Traitement des blocs " COMMENT" en passant par un fichier
    * temporaire.                   */


   /* Ouverture d'un fichier temporaire      */
   Status = open(FICHIER_STR, O_RDWR | O_CREAT | O_TRUNC, 0664);
   if (Status == -1) {   /* Erreur d'ouverture   */
      Status = ACQ_STREVTTMP;
      return (Status);
   } else Lun_tempo = Status; /* Ouverture correcte du fichier. */
   /* On va maintenant lire tout les blocs " COMMENT"    */
   Decompte = Nb_struct_bloc;
   while (Decompte) {
      Size_tempo = Size;
      Status = acq_mt_read_c(Lun, Buff->Buffer, &Size_tempo);
      if (Status != ACQ_OK) {
         libere_fichier(Lun_tempo);
         return (Status);
      }
      /* On recupere maintenent l'entete.       */
      if (strncmp(Buff->les_donnees.Ident, COMMENT_Id, 8) != 0) {
         Status = ACQ_BADFILESTRUCT;
         libere_fichier(Lun_tempo);
         return (Status);
      }
      /* Recuperation de la taille de la structure.      */
      strncpy(Header, Buff->les_donnees.cas.Buf_ascii, 4);
      Header[4] = '\0';
      if (strcmp(Header, "****") == 0) {
         Status = ACQ_BADEVTSTRUCT;  /* Structure illisible.*/
         libere_fichier(Lun_tempo);
         return (Status);
      } else Valeur = atoi(Header);
      /* On va maintenant ecrire les 'Valeur' caracteres qui suivent
       * dans le fichier temporaire.            */
      Depart = 4;
      Debut = Depart;
      Boucle = Depart;
      while (Boucle++ < Valeur + Depart) {
         /* On remplace le code 0x0D par le code fin de ligne
            du systeme courant */
         if (Buff->les_donnees.cas.Buf_ascii[Boucle] == 0x0d) {
            Buff->les_donnees.cas.Buf_ascii[Boucle] = '\n';
         }
      }
      /* On ecrit tout en une seule fois */
      Status = write(Lun_tempo, &Buff->les_donnees.cas.
                     Buf_ascii[Debut], Valeur);

      if (Status == -1) {
         libere_fichier(Lun_tempo);
         Status = ACQ_STREVTTMP;
         return (Status);
      }

      Decompte--;
   }

   /* On passe maintenant la main a rd_evstr ( .. )   */
   libere_fichier(Lun_tempo);
   Status = rd_evstr(FICHIER_STR, StrEvent, StrEventSize);
   if (Status == GR_OK)
      Status = ACQ_OK;
   else Status = ACQ_ERREVNTLEN;

   /* On va maintenent supprimer le fichier temporaire.     */
   remove(FICHIER_STR);

   return (Status);
}



/*****************************************************************************
 *                                                                           *
 *           Function  : get_next_event                                      *
 *           Objet     : Permet d'obtenir l'evenement suivant dans un buffer.*
 *                                                                           *
 *           Entree    : Buff -- Buffer de lecture des donnees.              *
 *                       Size -- Taille du buffer de lecture.                *
 *                       *Event -- Buffer d'evenement.                       *
 *                       SizeEvent -- Taille du buffer precedent.            *
 *                       NumeroEvent -- Numero d'evenement.                  *
 *       plus ensemble de variables globales utilisees pour   *
 *       les boucles.                                         *
 *                                                                           *
 *           Retour    : Status -- Etat d'execution de la commande.          *
 *        .. Ces codes peuvent changer de denomination ..     *
 *                              - ACQ_OK si aucun probleme.                  *
 *                              - ACQ_INVDATABUF si pas de buffer correct.   *
 *                              - ACQ_RAFBUFOVF si debordement.              *
 *                              - ACQ_ENDOFBUFFER si plus d'evenement.       *
 *                                                                           *
 *****************************************************************************/

int get_next_event(in2p3_buffer_struct* Buff, int Size,
                   short int* Event, int SizeEvent, int* NumeroEvent)
{

   /* Variables reutilisees a chaque appel en reutilisant leur valeur
      courante => on les declare en static */
   static int Index, Pos, EvtTaille;

   int Status, Boucle, SizeTempo;
   char Header[9];

   strncpy(Header, Buff->les_donnees.Ident, 8);
   Header[8] = '\0';

   if (strcmp(Header, EVENTDB_Id) != 0 &&
         strcmp(Header, EVENTCT_Id) != 0)
      Status = ACQ_INVDATABUF;
   else Status = ACQ_OK;
   if (Status == ACQ_OK) {
      if (*NumeroEvent == -1) {
         Pos = 0;
         Index = 0;
      }
      EvtTaille = Buff->les_donnees.cas.Buff[Pos];
      if (EvtTaille != 0) {
         /* On prend le plus petit des deux valeurs :EvtTaille,SizeEvent. */
         SizeTempo = EvtTaille < SizeEvent ? EvtTaille : SizeEvent;
         for (Boucle = 0; Boucle < SizeTempo; Boucle++)
            Event[Boucle] = Buff->les_donnees.cas.Buff[Pos + Boucle];
         Index++;
         Pos = Pos + EvtTaille;
         if (Pos > (Size - 36) / 2)
            Status = ACQ_RAFBUFOVF;
         else {
            Status = ACQ_OK;
            *NumeroEvent = Event[1];
         }
      } else {
         Status = ACQ_ENDOFBUFFER;
         *NumeroEvent = -1;
      }
      return (Status);


   }
}


/*****************************************************************************
 *                                                                           *
 *           Function  : get_next_param                                      *
 *           Objet     : Permet d'obtenir le parametre suivant dans un buffer*
 *                                                                           *
 *           Entree    : AddrParam -- Adresse du param courant dans buffer.  *
 *                       Ligne -- Ligne de 80 caracteres a formater.         *
 *                       Size -- Longueur utile.                             *
 *                       Addr -- Adresse courante en retour, incrementee.    *
 *                       Suite -- Drapeau de suite ( 1=Suite, 0 sinon ).     *
 *                                                                           *
 *           Retour    : Status -- Etat d'execution de la commande.          *
 *                              - ACQ_OK si aucun probleme.                  *
 *                              - ACQ_ENDOFBUFFER si buffer plein.           *
 *                                                                           *
 *****************************************************************************/
int get_next_param(int AddrParam, char* Ligne, int Size, int Addr, int Retour)
{



   /* A completer ................... */

   return (ACQ_OK);


}

/*****************************************************************************
 *                                                                           *
 *           Function  : libere_fichier                                      *
 *           Objet     : Permet de fermer un fichier et de liberer le LUN.   *
 *                                                                           *
 *           Entree    : Le_fichier -- Lun du fichier a clore.               *
 *                                                                           *
 *           Retour    : neant.                                              *
 *                                                                           *
 ****************************************************************************/

void libere_fichier(int Le_fichier)
{

   int Etat;

   Etat = close(Le_fichier);

}




/********************************* FIN ***************************************/
