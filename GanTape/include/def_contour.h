/*
 *    def_contour.h
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

#ifndef __DEF_CONTOUR_H
#define __DEF_CONTOUR_H

#ifdef __cplusplus
extern "C" {
#endif
#define ENTIER32   1
#define ENTIER16   2
#define OCTET      3


#ifndef ELEMENT_SWAP

typedef struct ELEMENT_SWAP {
   INT32 Type;
   INT32 Nombre;

} element_swap;
#define ELEMENT_SWAP

#endif



#define TAB_TET_CTR element_swap SwapEnteteContour[]  = {{OCTET,32},{ENTIER32,1}, {OCTET,20}, \
      {ENTIER32,2}, {OCTET,32}, {ENTIER32,2}, {OCTET,40}, {ENTIER32,6}, {OCTET,152},\
      {ENTIER32,2}, {OCTET,8}, {ENTIER32,5}, {OCTET,20}, {ENTIER32,1}, {OCTET,128}, {0,0}};

#define TAB_TET_FAM_CTR element_swap SwapEnteteFamContour[]  = {{OCTET,16},{ENTIER32,1},{0,0}};


/*
 *b. piquet  alignement des variables dans les structures communes...
 */


#ifdef  __alpha
#pragma member_alignment save
#pragma nomember_alignment
#endif


/*------------------------------------------------------------------------------
 * Definition de la structure de l'entete d'une famille de contours
 *------------------------------------------------------------------------------*/



typedef struct {
   char  nom_famille[16];       /* nom de la famille                   */
   int        nomb_cont;        /* nombre de contours dans la famille    */
} T_ENTETE_FAM_CTR_ACQ ;


/*------------------------------------------------------------------------------
 * definition de la structure de l'entete d'un fichier contour
 *------------------------------------------------------------------------------*/


typedef struct {
   char     nom_contour[16] ;    /* nom du contour                      */
   char  nom_famille[16];   /* nom de la famille eventuelle du contour  */
   int   num_cont;      /* no contour dans la famille (1 si seul)    */
   char  date[12];      /* date de creation ("jj-mmm-aaaa")          */
   char  heure[8];      /* heure de creation ("hh:mn:ss")               */
   int   num_spec_or;      /* numero du spectre d'origine                  */
   int   num_run_or ;      /* numero du run du spectre d'origine        */
   char  nom_par_x[16];    /* nom du parametre x (15 car utiles)        */
   char  nom_par_y[16];    /* nom du parametre y (15 car utiles)        */
   int   codeur_x ;     /* val max du codeur en x                    */
   int   codeur_y ;     /* val max du codeur en y                    */
   char  mode_coord[8] ;      /* coordonnees 'codeurs' ou 'spectre'        */
   char  unite_x[8] ;      /* unite du parametre en x                   */
   char  unite_y[8] ;      /* unite du parametre en y                   */
   char  nom_spectre[16] ; /* nom du spectre si mode 'spectre'          */
   int   dim_x ;        /* dimension du spectre en x                 */
   int   dim_y ;        /* dimension du spectre en y                 */
   int   min_x ;        /* numero du canal min en x                  */
   int   min_y  ;    /* numero du canal min en y                  */
   int   max_x ;        /* numero du canal max en x                  */
   int   max_y ;     /* numero du canal max en y                     */

   char  reserve_1[24] ;
   char  commentaire[80];  /* commentaire utilisateur                  */
   char  reserve_2[48] ;
   float longueur ;     /* longueur du contour                       */
   float surface ;      /* surface du contour                        */
   char  type[8];    /* type contour 'ferme','separatr'           */
   int   x_orig ;    /* x min du contour (en coord.'mod_coord')   */
   int   y_orig ;    /* y min du contour                          */
   int   x_dim_cont ;      /* dimension du contour en x                 */
   int   y_dim_cont ;      /* dimension du contour en y                 */
   int   n_points ;     /* nombre de points du contour                  */
   char  form_coord[4] ;      /* format des coordonnees (I*2,I*4)          */
   char  reserve_3[16] ;
   char  version[4];    /* no version -> V2.0            */
   int   magik;         /* mot magique 0xcafefadel                      */
   char  reserve_4[128];
} T_ENTETE_CTR_ACQ ;

/*
 *b. piquet  alignement des variables dans les structures communes...
 */

#ifdef __alpha
#pragma member_alignment restore
#endif

/* definition des protos de fonctions */

int read_contour_name(char* FileName, char* ContourName, \
                      T_ENTETE_CTR_ACQ*  ContourEntete, unsigned char* Table, int Size, int* err);

int read_contour_num(char* FileName, int ContourNum, \
                     T_ENTETE_CTR_ACQ*  ContourEntete, unsigned char* Table, int Size, int* err);
/*
int read_contour_num_(char *FileName, int ContourNum, \
      T_ENTETE_CTR_ACQ  *ContourEntete, unsigned char *Table, int Size, int *err);

int read_contour_name_ (char *FileName, char *ContourName, \
      T_ENTETE_CTR_ACQ  *ContourEntete, unsigned char *Table, int Size, int *err);
*/
int read_contour_entete_num(char* FileName, int ContourNum, \
                            T_ENTETE_CTR_ACQ*  ContourEntete, int* err);

int read_contour_entete_name(char* FileName, char* ContourName, \
                             T_ENTETE_CTR_ACQ*  ContourEntete, int* err);

int read_contour_entete_num_(char* FileName, int ContourNum, \
                             T_ENTETE_CTR_ACQ*  ContourEntete, int* err);

int read_contour_entete_name_(char* FileName, char* ContourName, \
                              T_ENTETE_CTR_ACQ*  ContourEntete, int* err);

int liste_contour(char* Filename, T_ENTETE_FAM_CTR_ACQ*  ContourEntete, char* Names[]);

int liste_contour_(char* Filename, T_ENTETE_FAM_CTR_ACQ*  ContourEntete, char* Names[]);


int affiche_contour_info(T_ENTETE_CTR_ACQ*  ContourEntete);

int affiche_contour_info_(T_ENTETE_CTR_ACQ*  ContourEntete);

int affiche_contour_data(T_ENTETE_CTR_ACQ*  ContourEntete, unsigned char* Table, int Nbre);

int affiche_contour_data_(T_ENTETE_CTR_ACQ*  ContourEntete, unsigned char* Table, int Nbre);

int contour_data_size(T_ENTETE_CTR_ACQ*  ContourEntete);

int contour_data_size_(T_ENTETE_CTR_ACQ*  ContourEntete);

int swap_contour_data(T_ENTETE_CTR_ACQ*  ContourEntetetete_spec, unsigned char* Table);

int swap_contour_entete(T_ENTETE_CTR_ACQ*  ContourEntete);

int save_contour(char* FileName, char* FamName, \
                 T_ENTETE_CTR_ACQ*  ContourEntete, unsigned char* Table, int Size, int* err);

int save_contour_(char* FileName, char* FamName, \
                  T_ENTETE_CTR_ACQ*  ContourEntete, unsigned char* Table, int Size, int* err);


int filtrage_spectre(char* FileSpectreIn, char* FileContourFam, \
                     char* ContourName, char* FileSpectreOut);

int filtrage_spectre_(char* FileSpectreIn, char* FileContourFam, \
                      char* ContourName, char* FileSpectreOut);


int set_debug_contour(int val);

int set_debug_contour_(int val);

void dump(char* Buf, int Size);

void dump_(char* Buf, int Size);






#ifdef __cplusplus
}
#endif
#endif





