/*
 *    fonc_spectra.c
 *
 * Luigi Martina --
 *
 *
 *
 *      read_spectre : fonction C de relecture d'un spectre
 *
 *      prototype : int read_spectre(char *FileName, tete_spec *SpectreEntete, unsigned char *Table, int Size, int *erreur)
 *
 *      appel     : status = read_spectre(FileName, &SpectreEntete, Table, Size, &erreur);
 *
 *      arguments :
 *                 - FileName      : nom de fichier associe au spectre
 *                 - SpectreEntete : pointeur sur structure entete spectre
 *       - Table         : pointeur tableau d'octets destine a recevoir le spectre
 *          - Size          : taille en octets du tableau destine a recevoir le spectre
 *          - erreur        : pointeur sur un entier destine a recevoir un code erreur
 *
 *
 *      retour    :
 *                   le status vaut 0 si tout est correct sinon,
 *                 -1 = erreur nom fichier
 *       -2 = pb open fichier
 *       -3 = pb read entete spectre
 *       -4 = taille insuffisante tableau
 *                 -5 = pb read tableau
 *                 -6 =
 *                 -7 = pb write entete
 *       -8 = pb write data
 *
 *
 *      fortran :   status = read_spectre_f(%ref(FileName), tete, table, %val(size), erreur)
 *
 *      save_spectre : fonction C de sauvegarde d'un spectre
 *
 *      prototype : int save_spectre(char *FileName, tete_spec *SpectreEntete, unsigned char *Table, int Size, int *erreur)
 *
 *      appel     : status = save_spectre(FileName, &SpectreEntete, Table, Size, &erreur);
 *
 *      arguments :
 *                 - FileName      : nom de fichier associe au spectre
 *                 - SpectreEntete : pointeur sur structure entete spectre
 *       - Table         : pointeur tableau d'octets destine a recevoir le spectre
 *          - Size          : taille en octets du tableau destine a recevoir le spectre
 *          - erreur        : pointeur sur un entier destine a recevoir un code erreur
 *
 *
 *      retour    :
 *                   le status vaut 0 si tout est correct sinon,
 *                 -1 = erreur nom fichier
 *       -2 = pb open fichier
 *       -3 =
 *       -4 = taille insuffisante tableau
 *                 -5 =
 *                 -6 =
 *                 -7 = pb write entete
 *       -8 = pb write data
 *
 *      fortran :   status = save_spectre_f(%ref(FileName), tete, table, %val(size), erreur)
 *
 */

/* -------------------- exemples appels a read_spectre et save_spectre -------------------------

 **********************
 * 1-   programme C   *
 **********************


   #include <stdio.h>
   #include <string.h>

   #include "gan_acq_buf.h"
   #include "gan_acq_swap_buf.h"
   #include "def_spectre.h"

   .....
   .....
   char *FileName = "../data/monodim.dat";
   int  Table[16384];
   int  Size = 16384;
   int  erreur;
   tete_spec SpectreEntete;

   appel a read_spectre
   status = read_spectre(FileName,&SpectreEntete,Table,Size,&erreur);
   .....
   .....
   appel a save_spectre
   status = save_spectre(FileName,&SpectreEntete,Table,Size,&erreur);



 ****************************
 * 2  programme fortran     *
 ****************************

   program test
   implicit none


        include 'def_spectre.for'



   record /entete_spectre/tete

   integer *4 status,i,size
        integer*4 monodim(16384)
        integer*4 erreur
        integer *4 size

        character *80 nom_fic
   byte c_nom_fic(80)

c--------------- ouverture fichier spectre ---------------

        print *,'test_read_spectre, entrez un nom de fichier :'
        read (5,'(a)') nom_fic
        print *,nom_fic
        size = 16384

c---------------- lecture d'un spectre --------------------

        stat = forstring_to_cstring(%ref(nom_fic), %ref(c_nom_fic), %val(len(nom_fic)))

        stat = read_spectre (%ref(c_nom_fic),tete,monodim,%val(size),erreur)


        ......
        ......

c---------------- sauvegarde d'un spectre -----------------

        stat = forstring_to_cstring(%ref(nom_fic), %ref(c_nom_fic), %val(len(nom_fic)))

        stat = save_spectre(%ref(c_nom_fic), tete, monodim,%val (size), erreur)



*/



#include <stdio.h>
#include <string.h>

#ifdef __VMS
#include <stdlib.h>
#endif

#include "GEN_TYPE.H"

#include "gan_acq_buf.h"

#include "gan_acq_swap_buf.h"

#include "def_spectre.h"

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define PATHLEN 256

#define RECL_SIZE (511*4)

static int get_offset_spectre(char* FileName, int* Offset, int* FlagSwap);

int debug_spectra = FALSE;
int AutoswBuf, Swbufon;


/*----------------------*/
/* set_debug_spectra    */
/*----------------------*/

int set_debug_spectre(int val)
{

   if (val) {
      debug_spectra = TRUE;
      printf("Passage en mode avec debug spectra\n\r");
   } else {
      debug_spectra = FALSE;
      printf("Passage en mode sans debug spectra\n\r");
   }
   return 0;
}

int set_debug_spectre_(int val)
{
   return set_debug_spectre(val);
}



/*-----------------------------*/
/* read_spectre fortran (UNIX) */
/*-----------------------------*/

int read_spectre_(char* FileName, tete_spec* SpectreEntete,
                  unsigned char* Table, int Size, int* error)
{
   return read_spectre(FileName, SpectreEntete, Table, Size, error);

}


/*-----------------------------*/
/* read_spectre fortran (UNIX) */
/*-----------------------------*/

int read_spectre_entete_(char* FileName, tete_spec* SpectreEntete, int* error)
{
   return read_spectre_entete(FileName, SpectreEntete, error);

}

/*---------------------------*/
/* read_spectre C (VMS/UNIX) */
/*---------------------------*/

int read_spectre_entete(char* FileName, tete_spec* SpectreEntete, int* error)
{
   FILE* fd;
   int nb_items;
   int status = 0;
   int FlagSwap = FALSE;
   int NbBytes, Offset;


   /* on recherche l'offset de debut */
   status = get_offset_spectre(FileName, &Offset, &FlagSwap);
   if (status != 0) {
      printf("read_spectre : get_offset_spectre -> status = %d\n\r", status);
      return status;
   }

   if (debug_spectra)
      printf("get_offset-> Offset = %d FlagSwap = %d\n\r", Offset, FlagSwap);

   /* ouverture du fichier */
   if ((fd = fopen(FileName, "r")) != NULL) {
      /* skip info de controle si necessaire */
      nb_items = fread(SpectreEntete, sizeof(char), Offset, fd);

      /* lecture entete du spectre */
      nb_items = fread(SpectreEntete, sizeof(char), sizeof(tete_spec), fd);

      /* test resultat de la lecture */
      if (nb_items == sizeof(tete_spec)) {
         /* Swap entete du spectre */
         if (FlagSwap == TRUE)
            swap_spectre_entete(SpectreEntete);

      } else
         status = -3;

      fclose(fd);
   } else
      status = -2;

   *error = status;
   return status;
}

/*---------------------------*/
/* read_spectre C (VMS/UNIX) */
/*---------------------------*/

int read_spectre(char* FileName, tete_spec* SpectreEntete, unsigned char* Table, int Size, int* error)
{
   FILE* fd;
   int i, nb_items;
   int rang, status = 0;
   int FlagSwap = FALSE;
   int data_size, Offset;
   int nb_fois, reste, nb_octets;
   short temp;


   /* on recherche l'offset de debut */
   status = get_offset_spectre(FileName, &Offset, &FlagSwap);
   if (status != 0) {
      printf("read_spectre : get_offset_spectre -> status = %d\n\r", status);
      return status;
   }

   if (debug_spectra)
      printf("get_offset-> Offset = %d FlagSwap = %d\n\r", Offset, FlagSwap);

   /* ouverture du fichier */
   if ((fd = fopen(FileName, "r")) != NULL) {
      /* skip info controle si presente */
      nb_items = fread(SpectreEntete, sizeof(char), Offset, fd);

      /* lecture entete du spectre */
      nb_items = fread(SpectreEntete, sizeof(char), sizeof(tete_spec), fd);

      /* test resultat de la lecture */
      if (nb_items == sizeof(tete_spec)) {
         /* Swap entete du spectre */
         if (FlagSwap == TRUE)
            swap_spectre_entete(SpectreEntete);


         /* taille des data en octets */
         data_size = spectre_data_size(SpectreEntete);

         if (data_size <= Size) {

            /* calcul du nombre d'iterations */
            reste = data_size % (RECL_SIZE - 2);

            nb_fois = data_size / (RECL_SIZE - 2);

            if (reste)
               nb_fois++;

            /* taille a lire en un seul coup */
            if (nb_fois > 1)
               nb_octets = RECL_SIZE - 2;
            else
               nb_octets = reste;

            if (debug_spectra)
               printf("read_spectre : data_size = %d reste = %d nb_octets = %d nb_fois = %d\n\r",
                      data_size, reste, nb_octets, nb_fois);
            rang = 0;

            /* boucle de lecture des data */
            for (i = 0; i < nb_fois; i++) {

               /* Skip info controle si elle existe */
               fread((char*)&temp, sizeof(char), Offset, fd);


               /* test si dernier tour */
               if (i ==  nb_fois - 1)
                  nb_octets = reste;

               /* Lecture des Data */
               nb_items = fread(&Table[rang], sizeof(char), nb_octets, fd);

               rang += nb_octets;

               /* test si fread OK */
               if (nb_items != nb_octets) {
                  printf("read_spectre : fread data error\n\r");
                  status = -5;
                  break;
               }

            }
         } else
            status = -4;

      } else
         status = -3;

      /* si tout s'est bien passe */
      if (status == 0) {
         /* test si swap data necessaire */
         if (FlagSwap == TRUE)
            swap_spectre_data(SpectreEntete, Table);

      }

      fclose(fd);
   } else
      status = -2;

   /* maj code erreur */
   *error = status;

   return status;
}


/*--------------------------*/
/* affiche_spectre_info_   */
/*--------------------------*/


int affiche_spectre_info_(tete_spec* SpectreEntete)
{

   return affiche_spectre_info(SpectreEntete);
}


/*----------------------*/
/* AfficheSpectreInfo   */
/*----------------------*/

int affiche_spectre_info(tete_spec* SpectreEntete)
{


   printf("spectre . . . ....%.16s\n\r", SpectreEntete->nom_spectre);
   printf("num_run . . . . . %d\n\r", SpectreEntete->num_run);
   printf("nom_run . . . . . %.16s\n\r", SpectreEntete->nom_run);

   printf("date. . . . . ....%.12s %.8s \n\r", SpectreEntete->date, SpectreEntete->heure);
   printf("num_spectre . ....%d\n\r", SpectreEntete->num_spectre);
   printf("type_spectre. . . %.3s\n\r", SpectreEntete->type_spectre);

   printf("x dim . . . . ....%d\n\r", SpectreEntete->dim_x);
   printf("taille_canal. . . %d\n\r", SpectreEntete->taille_canal);


   if (SpectreEntete->dim_y) {
      printf("y dim . . . . ..%d\n\r", SpectreEntete->dim_y);
      printf("nom_par_y ....  %.16s\n\r", SpectreEntete->nom_par_y);
   }
   return 0;
}



/*-----------------------*/
/* SwapSpectreEntete     */
/*-----------------------*/

int swap_spectre_entete(tete_spec* SpectreEntete)
{
   TABTETSPEC
   int i;
   char* p = (char*)SpectreEntete;

   if (debug_spectra)
      printf("pointeur entete = $%x\n\r", SpectreEntete);

   i = 0;

   do {

      switch (SwapEntSpe[i].Type) {
         case ENTIER32  :
            SwapInt32((unsigned int*)p, SwapEntSpe[i].Nombre * sizeof(int));
            p += sizeof(int) * SwapEntSpe[i].Nombre;
            break;
         case ENTIER16   :
            SwapInt16((unsigned short*)p, SwapEntSpe[i].Nombre * sizeof(short));
            p += sizeof(short) * SwapEntSpe[i].Nombre;
            break;

         case OCTET   :
            p += sizeof(char) * SwapEntSpe[i].Nombre;

         default   :
            break;
      }
      i++;

   } while (SwapEntSpe[i].Type);


   return 0;
}


/*---------------------*/
/* spectre_data_size_f */
/*---------------------*/

int spectre_data_size_(tete_spec* SpectreEntete)
{
   return spectre_data_size(SpectreEntete);
}


/*-------------------*/
/* spectre_data_size */
/*-------------------*/

int spectre_data_size(tete_spec* SpectreEntete)
{

   int Size;


   /* spectre monodim ou bidim */
   if (SpectreEntete->dim_y == 0)
      Size = SpectreEntete->dim_x * SpectreEntete->taille_canal / 8;
   else
      Size = SpectreEntete->dim_x * SpectreEntete->dim_y * SpectreEntete->taille_canal / 8;

   return Size;
}

/*------------------*/
/* SwapSpectreData  */
/*------------------*/

int swap_spectre_data(tete_spec* SpectreEntete, unsigned char* Table)
{
   int ByteSize;
   int err = 0;


   ByteSize =  spectre_data_size(SpectreEntete);

   if (SpectreEntete->taille_canal / 8 == sizeof(short))
      SwapInt16((unsigned short*)Table, ByteSize);
   else if (SpectreEntete->taille_canal / 8 == sizeof(int))
      SwapInt32((unsigned int*)Table, ByteSize);
   else
      err = -1;
   return err;
}

/*--------------------*/
/* AfficheSpectreData */
/*--------------------*/

#define NBVAL_LIG 8

int affiche_spectre_data(tete_spec* SpectreEntete, unsigned char* Table, int Nbre)
{
   int i, j, NbLig;
   unsigned short* TabShort = (unsigned short*)Table;
   unsigned int*   TabInt = (unsigned int*)Table;

   NbLig = Nbre / NBVAL_LIG;
   if (Nbre % NBVAL_LIG)
      NbLig += 1;


   if (SpectreEntete->taille_canal / 8 == sizeof(short)) {
      for (j = 0; j < NbLig; j++) {
         printf("\n\r");
         for (i = 0 ; i < NBVAL_LIG; i++)
            printf("%-5d ", TabShort[i + j * NBVAL_LIG]);
      }
   } else {
      for (j = 0; j < NbLig; j++) {
         printf("\n\r");
         for (i = 0; i < NBVAL_LIG; i++)
            printf("%-5d ", TabInt[i + NBVAL_LIG * j]);
      }
   }
   return 0;
}

/*-----------------------*/
/* GetOffsetSpectre      */
/*-----------------------*/

#define  MAGIC         0xcafefadel   /* cafefade  en hexadecimal     */

#define  PATT_MONODIM     0x3144  /* correspond a aacii '1D'  */

#define  PATT_BIDIM       0x3244  /* correspond a ascii '2D'  */



int get_offset_spectre(char* FileName, int* Offset, int* FlagSwap)
{
   FILE* fd;
   unsigned char buf[256];
   int nb_items;
   int found = FALSE;
   int i, status = 0;
   int cond, GoodOffset;
   unsigned int Magic;
   unsigned int SwapMagic;
   unsigned short PattMonodim;
   unsigned short PattBidim;
   unsigned short SwapPattMonodim;
   unsigned short SwapPattBidim;

   unsigned short* p_short;
   unsigned int* p_int;

   tete_spec SpectreEntete;

   union {
      unsigned short val;
      unsigned char buf[2];
   } S_U;

   union {
      unsigned int  val;
      unsigned char buf[4];
   } L_U;




   /* Init des differents Patterns */
   SwapPattMonodim = PattMonodim = PATT_MONODIM;
   SwapPattBidim = PattBidim = PATT_BIDIM;
   SwapInt16(&SwapPattMonodim, 2);
   SwapInt16(&SwapPattBidim, 2);
   SwapMagic = Magic = MAGIC;
   SwapInt32(&SwapMagic, 4);


   /* ouverture du fichier */
   if ((fd = fopen(FileName, "r")) != NULL) {
      /* lecture entete du spectre */
      nb_items = fread(buf, sizeof(char), sizeof(tete_spec), fd);

      /* valeur normale de l'offset pour le mot magique */
      GoodOffset = (char*)&SpectreEntete.magik - (char*)&SpectreEntete;

      /* on recherche le mot magique */
      for (i = 0; i < sizeof(SpectreEntete); i++) {
         /* on remplit la structure L_I */
         L_U.buf[0] = buf[i];
         L_U.buf[1] = buf[i + 1];
         L_U.buf[2] = buf[i + 2];
         L_U.buf[3] = buf[i + 3];

         /* test si on trouve le mot magique (swappe ou non) */
         if ((L_U.val == Magic) || (L_U.val == SwapMagic)) {
            /* on positionne l'offset */
            *Offset = i - GoodOffset;

            if (L_U.val == Magic)
               *FlagSwap = FALSE;
            else
               *FlagSwap = TRUE;
            found = TRUE;

            /* on sort le no version */
            memcpy(&SpectreEntete, buf + *Offset, sizeof(tete_spec));
            if (debug_spectra)
               printf("version = %4s\n\r", SpectreEntete.version);
         }
      }

      /* test si ancien fichier spectre ? */
      if (found == FALSE) {

         /* valeur normale de l'offset pour le mot magique */
         GoodOffset = (char*)&SpectreEntete.type_spectre - (char*)&SpectreEntete;

         /* on recherche le Patt bidim ou monodim */
         for (i = 0; i < sizeof(tete_spec); i++) {
            /*p_short = (unsigned short *)(buf + i);*/
            S_U.buf[0] = buf[i];
            S_U.buf[1] = buf[i + 1];

            /* condition sur Pattern */
            cond = (S_U.val == PattMonodim) || (S_U.val == SwapPattMonodim)
                   || (S_U.val == PattBidim)   || (S_U.val == SwapPattBidim);
            if (cond) {

               *Offset = i - GoodOffset;

               /* recopie entete spectre avec offset */
               memcpy(&SpectreEntete, buf + *Offset, sizeof(tete_spec));
               if ((SpectreEntete.taille_canal == 16)
                     || (SpectreEntete.taille_canal == 32)) {
                  found = TRUE;
                  *FlagSwap = FALSE;
                  break;
               } else {
                  /* on tente le swap */
                  SwapInt32((unsigned int*)&SpectreEntete.taille_canal, 4);
                  if ((SpectreEntete.taille_canal == 16)
                        || (SpectreEntete.taille_canal == 32)) {
                     found = TRUE;
                     *FlagSwap = TRUE;
                     break;
                  }
               }
            }    /* fin if cond */
         }    /* fin boucle for */
      }   /* fin if found false */

      fclose(fd);
   } else
      status = -2;

   /* test si patt trouve */
   if ((found == FALSE)
         && (status == 0))
      status = -1;

   return status;
}


/*----------------------------*/
/* save_spectre fortran (VMS) */
/*----------------------------*/

int  save_spectre_(char* FileName, tete_spec* SpectreEntete,
                   unsigned char* Table, int Size, int* error)
{
   return save_spectre(FileName, SpectreEntete, Table, Size, error);
}



/*---------------------------*/
/* save_spectre C (VMS/UNIX) */
/*---------------------------*/


int  save_spectre(char* FileName, tete_spec* SpectreEntete, unsigned char* Table, int Size, int* error)
{
   FILE* fd;
   int nb_items;
   int SizeData, status = 0;

   printf("save_spectre: sizeof(tete_spec) = %d\n\r", sizeof(tete_spec));

   /* ouverture du fichier */
   printf("save_spectre : FileName = %s\n\r", FileName);

   if ((fd = fopen(FileName, "w")) != NULL) {
      /* calcul taille du spectre */
      SizeData = spectre_data_size(SpectreEntete);

      if (debug_spectra)
         printf("save_spectre : SizeData = %d Size = %d\n\r", SizeData, Size);

      if (SizeData <= Size) {

         /* lecture entete du spectre */
         nb_items = fwrite(SpectreEntete, sizeof(char), sizeof(tete_spec), fd);

         /* test si ecriture OK */
         if (nb_items == sizeof(tete_spec)) {

            nb_items = fwrite(Table, sizeof(char), SizeData, fd);
            if (nb_items == SizeData)
               status = 0;
            else
               status = -8;
         } else
            status = -7;
      } else
         status = -4;

      /* fermeture du fichier */
      fclose(fd);
   } else
      status = -2;

   /* maj code erreur */
   *error = status;

   return status;
}
