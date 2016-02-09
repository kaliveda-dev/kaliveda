#ifdef __cplusplus
extern "C" {
#endif
#define R_SBUF_NORMAL   0x08078009
/* <normal successful completion> */
#define R_SBUF_NOBUF   0x08078013
/* < Pas de buffer disponible > */
#define R_SBUF_BADCODE  0x08078018
/* < Code erreur inconnu > */
#define R_SBUF_OPEN   0x08078024
/* < error on VAX-VME connection (SBUF OPEN) > */
#define R_SBUF_CLOSE   0x0807802C
/* < error on VAX-VME connection (SBUF CLOSE) > */
#define R_SBUF_SETUP  0x08078034
/* < error on VAX-VME connection (SBUF SETUP) > */
#define R_SBUF_START   0x0807803C
/* < error on VAX-VME connection (SBUF START) > */
#define R_SBUF_READ   0x08078044
/* < fatal error on VAX-VME connection; try ACQ SET/NEWREAD > */

#ifdef __cplusplus
}
#endif
