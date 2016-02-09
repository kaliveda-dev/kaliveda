#ifdef __cplusplus
extern "C" {
#endif
#define R_TCP_NORMAL   0x08058009
/* <normal successful completion> */
#define R_TCP_HOST   0x08058010
/* < TCP/IP: Erreur host inconnu > */
#define R_TCP_SOCK   0x08058018
/* < TCP/IP: Erreur TcpOpen - socket() > */
#define R_TCP_BIND   0x08058020
/* < TCP/IP: Erreur TcpOpen - bind() > */
#define R_TCP_LISTEN   0x08058028
/* < TCP/IP: Erreur TcpOpen - listen() > */
#define R_TCP_CLONE   0x08058030
/* < TCP/IP: Erreur TcpOpen - accept() > */
#define R_TCP_CONNECT   0x08058038
/* < TCP/IP: Erreur TcpOpen - connect()  > */
#define R_TCP_CLOSE   0x08058040
/* < TCP/IP: Erreur close > */
#define R_TCP_READ   0x08058048
/* < TCP/IP: Erreur TcpRead - read() > */
#define R_TCP_WRITE   0x08058050
/* < TCP/IP: Erreur TcpWrite - write() > */
#define R_TCP_LGBUF   0x08058058
/* < TCP/IP: Erreur TcpWrite, longueur buffer < 0  > */
#define R_TCP_TIMEOUT   0x08058060
/* < TCP/IP: Time out  > */
#define R_TCP_BADCODE  0x08058068
/* < TCP/IP: Code erreur inconnu > */
#ifdef __cplusplus
}
#endif
