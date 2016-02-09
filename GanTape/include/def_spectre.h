/*
 *    def_spectre.h
 *
 * Luigi Martina -- Nov 2000
 *
 *      Fonction :
 *
 *
 *      Includes necessaires avant : gent_type.h acq_tcp_struct.h
 *
 *      Modifications :
 *
 *
 *
 *
 *
 */

#ifndef __DEF_SPECTRE_H
#define __DEF_SPECTRE_H
#ifdef __cplusplus
extern "C" {
#endif

#define ENTIER32 1
#define ENTIER16 2
#define OCTET 3


#ifndef ELEMENT_SWAP

typedef struct ELEMENT_SWAP {
   INT32 Type;
   INT32 Nombre;

} element_swap;
#define ELEMENT_SWAP

#endif


#define TABTETSPEC element_swap SwapEntSpe[]=   { {ENTIER32,1},{ENTIER32,1},{OCTET,24}, \
      {ENTIER32,1},{OCTET,48},{ENTIER32,4},{OCTET,4}, \
      {ENTIER32,6},{OCTET,128},{0,0} };

/*
 *b. piquet  alignement des variables dans les structures communes...
 */

#ifdef __alpha
#pragma member_alignment save
#pragma nomember_alignment
#endif

typedef struct TETE_SPEC { /* Structure pour les entetes de spectres */
   INT32 num_run;    /* No de run            */
   INT32 magik;         /* mot magique de swapping 0xcafefade   */
   BYTE version[4];             /* numero de version = Vx.y             */
   BYTE reserve_1[4];      /* reserve           */
   BYTE nom_run[16];    /* nom du run           */
   INT32 num_spectre;      /* No de spectre        */
   BYTE type_spectre[2];   /* type de spectre ( 1D ou 2D )  */
   BYTE reserve_2[10];     /* reserve           */
   BYTE nom_spectre[16];   /* nom du spectre ( 15 carac. utilises )*/
   BYTE date[12];    /* date de creation ("jj-mmm-aaaa"->11 car ut)*/
   BYTE heure[8];    /* heure de creation ("hh:mn:ss" -> 8 car ut )*/
   INT32 nat_spec;      /* reserve ulterieure         */
   INT32 codeur_x;      /* valeur max. du codeur en X       */
   INT32 codeur_y;      /* valeur max. du codeur en Y       */
   INT32 taille_canal;     /* taille utile par canal ( 16 ou 32 )    */
   BYTE type_canal[4];     /* type variable ( "I*4" ou "I*2" )    */
   INT32 dim_x;         /* dimension du spectre en X     */
   INT32 dim_y;         /* dimension du spectre en Y     */
   INT32 min_x;         /* No du canal min en X       */
   INT32 min_y;         /* No du canal min en Y       */
   INT32 max_x;         /* No du canal max en X       */
   INT32 max_y;         /* No du canal max en Y       */
   BYTE commentaire[80];   /* commentaire utilisateur       */
   BYTE nom_par_x[16];     /* nom du param X ( 15 carac. utiles )    */
   BYTE nom_par_y[16];     /* nom du param Y ( ---------------- )    */
   BYTE unite_x[8];     /* unite en X ( 8 caract util )  */
   BYTE unite_y[8];     /* unite en Y ( ------------- )  */

} tete_spec;

/*
 *b. piquet  alignement des variables dans les structures communes...
 */
#ifdef __alpha
#pragma member_alignment restore
#endif


/* definition des protos de fonctions */

int read_spectre(char* FileName, tete_spec* SpecEntete, unsigned char* Table, int Size, int* err);

int read_spectre_(char* FileName, tete_spec* SpecEntete, unsigned char* Table, int Size, int* err);

int affiche_spectre_info(tete_spec* SpectreEntete);

int affiche_spectre_info_(tete_spec* SpectreEntete);

int affiche_spectre_info_f_(tete_spec* SpectreEntete);

int affiche_spectre_data(tete_spec* SpectreEntete, unsigned char* Table, int Nbre);

int affiche_spectre_data_(tete_spec* SpectreEntete, unsigned char* Table, int Nbre);

int spectre_data_size(tete_spec* SpectreEntete);

int spectre_data_size_(tete_spec* SpectreEntete);

int swap_spectre_data(tete_spec* SpectreEntete, unsigned char* Table);

int swap_spectre_entete(tete_spec* SpectreEntete);

int save_spectre(char* FileName, tete_spec* SpectreEntete, unsigned char* Table, int Size, int* err);

int save_spectre_(char* FileName, tete_spec* SpectreEntete, unsigned char* Table, int Size, int* err);

int save_spectre_f(char* FileName, tete_spec* SpectreEntete, unsigned char* Table, int Size, int* err);


int read_spectre_entete(char* FileName, tete_spec* SpectreEntete, int* error);

int read_spectre_entete_(char* FileName, tete_spec* SpectreEntete, int* error);

int set_debug_spectre(int val);

int set_debug_spectre_(int val);

void dump(char* Buf, int Size);

#ifdef __cplusplus
}
#endif

#endif
