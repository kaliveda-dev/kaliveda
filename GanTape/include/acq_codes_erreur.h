#ifdef __cplusplus
extern "C" {
#endif
#define ACQ_OK  0x08018001
/* <you know what? I'm HAPPY!> */
#define ACQ_ON  0x08018009
/* <your acquisition is running> */
#define ACQ_OFF  0x08018011
/* <your acquisition is halted> */
#define ACQ_ADDTAPEOK  0x08018019
/* <successfull adding operation of a storage device > */
#define ACQ_REMTAPEOK  0x08018021
/* <successfull removing operation of a storage device > */
#define ACQ_ENABTAPEOK  0x08018029
/* <successfull enabling operation of storage device > */
#define ACQ_DISABTAPEOK  0x08018031
/* <successfull disabling operation of storage device > */
#define ACQ_MOUNTTAPEOK  0x08018039
/* < successfull mount operation > */
#define ACQ_INIACQEXEC  0x08018041
/* <successfull execution of initialization of the VME crate> */
#define ACQ_TIMOUT  0x0801804B
/* <going out of SEMWAIT through time out condition> */
#define ACQ_MBXNOMSG  0x08018053
/* <there is no message in that mailbox> */
#define ACQ_PURGEDPID  0x0801805B
/* <a communication session was purged...> */
#define ACQ_DBGMOD  0x08018063
/* <starting debug mode for acquisition> */
#define ACQ_NODBGMOD  0x0801806B
/* <ending debug mode for acquisition> */
#define ACQ_CTRLMOD  0x08018073
/* <starting control mode for acquisition> */
#define ACQ_FULLCTRLMOD  0x0801807B
/* <starting full control mode for acquisition> */
#define ACQ_STNDCTRLMOD  0x08018083
/* <starting standard control mode for acquisition (no user call)> */
#define ACQ_USERCTRLMOD  0x0801808B
/* <starting user control mode for acquisition> */
#define ACQ_NOCTRLMOD  0x08018093
/* <no more control is performed on acquisition> */
#define ACQ_STOREMOD  0x0801809B
/* <starting store mode for acquisition> */
#define ACQ_NOSTOREMOD  0x080180A3
/* <acquisition goes on without storage of datas> */
#define ACQ_SIMMOD  0x080180AB
/* <starting simulation mode for acquisition> */
#define ACQ_NOSIMMOD  0x080180B3
/* <ending simulation mode for acquisition> */
#define ACQ_CONNECTION  0x080180BB
/* <a new communication session starts now...> */
#define ACQ_DISCONNECT  0x080180C3
/* <a communication session stops!> */
#define ACQ_ASKFORABORT  0x080180CB
/* <received a demand to abort the application> */
#define ACQ_TAPEISFULL  0x080180D3
/* <tape was full,and dismounted, load another if necessary and mount it> */
#define ACQ_FREECTRLBUF  0x080180DB
/* <a buffer was freed up without control> */
#define ACQ_ENDOFBUFFER  0x080180E3
/* <the buffer doesn't contain any other event> */
#define ACQ_NOACQUIS  0x080180EB
/* <the GANIL acquisition is not started> */
#define ACQ_VISUOFFLINE  0x080180F3
/* <the visualisation runs off line> */
#define ACQ_VISUNOCONNECT  0x080180FB
/* <the visualisation is not actually connected to host> */
#define ACQ_FREEBUFONSTOP  0x08018103
/* <when acquisition stopped, some buffers were freed up> */
#define ACQ_FREEBUFONCRASH  0x0801810B
/* <on crash of control process, some buffers were freed up> */
#define ACQ_ENDOFFILE  0x08018113
/* <attempt to read past end of file> */
#define ACQ_ENDOFTAPE  0x0801811B
/* <attempt to read past end of tape> */
#define ACQ_DUMPALLMOD  0x08018123
/* <starting dump all buffers mode> */
#define ACQ_DUMPNONEMOD  0x0801812B
/* <starting nodump buffers mode> */
#define ACQ_DUMPSTOREMOD  0x08018133
/* <starting dump store buffers mode> */
#define ACQ_DUMPCTRLMOD  0x0801813B
/* <starting dump control buffers mode> */
#define ACQ_NEWCTRLTASK  0x08018143
/* <starting with a new version of user control program> */
#define ACQ_NEWREADTASK  0x0801814B
/* <reload the read VME task (ACQ_READ_DATA)> */
#define ACQ_NEWSFROMVME  0x08018153
/* <received from VME crate this informational message> */
#define ACQ_EXIT  0x0801815B
/* <ask to exit a command session> */
#define ACQ_REPLAYMOD  0x08018163
/* <starting replay mode for acquisition> */
#define ACQ_NOREPLAYMOD  0x0801816B
/* <starting normal mode for acquisition> */
#define ACQ_NOMOREVINBUF  0x08018173
/* <no more event in that buffer> */
#define ACQ_RETPROCCTRL  0x0801817B
/* <something has been sent back to ACQUISITION process> */
#define ACQ_READSTRT  0x08018183
/* <starting ACQ_READ_DATA process> */
#define ACQ_CTRLSTRT  0x0801818B
/* <starting ACQ_CTRL_DATA process> */
#define ACQ_STORESTRT  0x08018193
/* <starting ACQ_STORE_DATA process> */
#define ACQ_NOFILTDEF  0x0801819B
/* <no filter defined> */
#define ACQ_OPNOTCOMP  0x080181A3
/* <operation is not completed, wait a moment...> */
#define ACQ_VMEINIFILEXEC  0x080181AB
/* <auto execution of VME initialization file> */
#define ACQ_VMESTAFILEXEC  0x080181B3
/* <auto execution of VME start of run file> */
#define ACQ_VMEFIRFILEXEC  0x080181BB
/* <auto execution of VME FIRST IN RUN file> */
#define ACQ_VMEENDFILEXEC  0x080181C3
/* <auto execution of VME end of run file> */
#define ACQ_ARGALL  0x080181CB
/* <'all' character argument specified as command parameter> */
#define ACQ_NEWSTORETASK  0x080181D3
/* <reload the storage task (ACQ_STORE_DATA)> */
#define ACQ_AUTENDRUN  0x080181DB
/* <automatic end of run number > */
#define ACQ_STWHAUTENAB  0x080181E3
/* <stop acquisition (automatic mode is disabled, or no tape is available)> */
#define ACQ_NOTIMPL  0x08018960
/* <not yet implemented> */
#define ACQ_GLTOOSHORT  0x08018968
/* <global section contains less bytes than requested> */
#define ACQ_NOCONNECT  0x08018970
/* <got a message without connection code...> */
#define ACQ_NOMORESESSION  0x08018978
/* <session table is full> */
#define ACQ_UNEXPMSG  0x08018980
/* <the header of this message does not match with acquisition rules> */
#define ACQ_UNEXPCODE  0x08018988
/* <the action code of this message is unknown > */
#define ACQ_NOPRIV  0x08018990
/* <you don't have the required privilege for this operation!> */
#define ACQ_PRIVALRATTR  0x08018998
/* <the privilege you want was previously attributed> */
#define ACQ_PRIVNOTALLOWED  0x080189A0
/* <the privilege you want requires a password> */
#define ACQ_ACTNOTAUT  0x080189A8
/* <this action is not authorized in this context> */
#define ACQ_TOOMUCHTAPE  0x080189B0
/* <you already have maximal allowed number of storage devices> */
#define ACQ_ALRALLOC  0x080189B8
/* <this storage device is already yours> */
#define ACQ_NOTALLOC  0x080189C0
/* <this storage device is not actually yours> */
#define ACQ_NOMORETAPE  0x080189C8
/* <you don't have any storage device for your acquisition> */
#define ACQ_NOTMOUNT  0x080189D0
/* <the tape on which you want to work is not mounted> */
#define ACQ_ALREADYMOUNT  0x080189D8
/* <the tape you want to mount is already mounted> */
#define ACQ_ISNOTATAPE  0x080189E0
/* <the name you enter doesn't represent a storage device> */
#define ACQ_TAPEINUSE  0x080189E8
/* <this storage device is actually used by your acquisition> */
#define ACQ_TAPENODISP  0x080189F0
/* <this storage device is not actually available> */
#define ACQ_BADEVENTFORM  0x080189F8
/* <the format of this event is not compatible with IN2P3 standard> */
#define ACQ_SPECTABFULL  0x08018A00
/* <you already got the maximal allowed count of spectra > */
#define ACQ_PARTABFULL  0x08018A08
/* <you already got the maximal allowed count of parameters > */
#define ACQ_DUPPLNAME  0x08018A10
/* <this name is already in this table> */
#define ACQ_UNKNSPEC  0x08018A18
/* <this spectrum was not defined in the table.Check name or number> */
#define ACQ_SPFULLSPACE  0x08018A20
/* <the whole space dedicated to spectra incrementation is used> */
#define ACQ_TOOMANYSPEC  0x08018A28
/* <too many spectra to start at the same time> */
#define ACQ_SPALRINC  0x08018A30
/* <some of these spectra are already incremented> */
#define ACQ_SPWRNGALRINC  0x08018A38
/* <some spectra to start were already being incremented> */
#define ACQ_SPCLRGALRINC  0x08018A40
/* <the spectra already incremented have been cleared> */
#define ACQ_SPNOTALLSUSP  0x08018A48
/* <some of the spectra you want to restart were not suspended> */
#define ACQ_SPNOTALLINC  0x08018A50
/* <some of the spectra you want to suspend are not started> */
#define ACQ_SPNOTINTABLE  0x08018A58
/* <the spectrum you want to replace is not in the dictionnary> */
#define ACQ_INCRSPEC  0x08018A60
/* <the spectrum you want to replace is actually incremented> */
#define ACQ_PARNOTINTABLE  0x08018A68
/* <the parameter you want to replace was not defined> */
#define ACQ_USEDPARAM  0x08018A70
/* <the parameter you want to replace is actually in use> */
#define ACQ_NOSUCHPAR  0x08018A78
/* <one parameter you want to use doesn't exist> */
#define ACQ_NOSUCHCOEF  0x08018A80
/* <the coefficient you want to change doesn't exist> */
#define ACQ_COEFTABFULL  0x08018A88
/* <you already got the maximal allowed count of user coefficients> */
#define ACQ_ERALLOCVM  0x08018A90
/* <virtual memory allocation failure > */
#define ACQ_NOTALLSTOPPED  0x08018A98
/* <all spectra cannot be stopped (remote connection ??)> */
#define ACQ_ERCODERSIZE  0x08018AA0
/* <more bits were coded for this parameter than in description> */
#define ACQ_STORNONEXIST  0x08018AA8
/* <the spectrum you want save doesn't exist actually> */
#define ACQ_ERRLOGTAB  0x08018AB0
/* <error detected in logical tables...> */
#define ACQ_NOSUCHSPEC  0x08018AB8
/* <this spectrum was not defined> */
#define ACQ_NOINCRSPEC  0x08018AC0
/* <this spectrum is not actually incremented> */
#define ACQ_CHRUNWHILACT  0x08018AC8
/* <cannot change run number while acquisition runs> */
#define ACQ_CTRLUSRERR  0x08018AD0
/* <this error was signaled in the user part of control process> */
#define ACQ_UNKNCOEF  0x08018AD8
/* <this coeff was not defined in the table.Check name or number> */
#define ACQ_UNKNPARAM  0x08018AE0
/* <this parameter was not defined in the table.Check name or number> */
#define ACQ_LOSTCONNECTION  0x08018AE8
/* <sorry,lost connection with DECnet partner> */
#define ACQ_ACQALRRUN  0x08018AF0
/* <Your acquisition is already running...> */
#define ACQ_INVPASSW  0x08018AF8
/* <Invalid password> */
#define ACQ_DEVNOTMOUNT  0x08018B00
/* <The device you want to access is not mounted> */
#define ACQ_DEVWRITLOCK  0x08018B08
/* <This device is write-locked> */
#define ACQ_CTRLBUSY  0x08018B10
/* <The control process is busy...Try again later!> */
#define ACQ_NOCHGWHRUN  0x08018B18
/* <cannot reload any process while acquisition is running> */
#define ACQ_NOABTWHMOUN  0x08018B20
/* <Please dismount tapes before abort aquisition process> */
#define ACQ_ERVMESYNC  0x08018B28
/* < Some error occurs during synchronization with remote VME crate> */
#define ACQ_WARNFROMVME  0x08018B30
/* <received from VME crate this warning message> */
#define ACQ_NOINIWHRUN  0x08018B38
/* <cannot init VME crate while acquisition is running> */
#define ACQ_BUFALREADYGOT  0x08018B40
/* <current control buffer was already read from your task> */
#define ACQ_NOCTRLBUFDISP  0x08018B48
/* <there is currently no control buffer because acq is stopped > */
#define ACQ_ERRCTRLBUF  0x08018B50
/* <an error occurs in control buffer copy (twice changed)> */
#define ACQ_NOCURRCTRLBUF  0x08018B58
/* <there is no available ctrl buffer, please try again> */
#define ACQ_RAFRFULL  0x08018B60
/* <auxilliary array for refreshment of spectrum is full> */
#define ACQ_WRBUFCTRL  0x08018B68
/* <will store a control buffer, please contact manager> */
#define ACQ_TOOMANYINCR  0x08018B70
/* <too many incremented spectra related to a parameter> */
#define ACQ_CNTOU_INVBITMP  0x08018B78
/* <invalid bitmap dimensions> */
#define ACQ_CNTOU_OUTBITMP  0x08018B80
/* <contour not within related bitmap> */
#define ACQ_CNTOU_TOOTWIST  0x08018B88
/* <too many intersections between contour and horizontal line> */
#define ACQ_CNTOU_FUNNY  0x08018B90
/* <something weird in the contour point list ; check it> */
#define ACQ_TOOMANYPOINTS  0x08018B98
/* <too many points in the current contour> */
#define ACQ_INVPOINTNUMB  0x08018BA0
/* <invalid points number> */
#define ACQ_NOSUCHCNTOU  0x08018BA8
/* <this contour was not defined> */
#define ACQ_INVCONTSCAL  0x08018BB0
/* <invalid contour scaling factor> */
#define ACQ_INVCONTNAM  0x08018BB8
/* <invalid contour name> */
#define ACQ_CONTOUTABFULL  0x08018BC0
/* <you already got the maximal allowed count of contours > */
#define ACQ_UNKNCONTOUR  0x08018BC8
/* <this contour was not defined in the table.Check name or number> */
#define ACQ_FAMINOCNTOU  0x08018BD0
/* <No contour belongs to this family. Check name> */
#define ACQ_CNTOUNOTIMPL  0x08018BD8
/* <type of contour not yet implemented> */
#define ACQ_BITMAPTABFULL  0x08018BE0
/* <you already got the maximal allowed count of bitmaps > */
#define ACQ_NOBITMAP  0x08018BE8
/* <No bitmap for this filter> */
#define ACQ_INVFILTNAM  0x08018BF0
/* <invalid filter name> */
#define ACQ_UNKNFILTER  0x08018BF8
/* <this filter was not defined in the table.Check name or number> */
#define ACQ_FILTERTABFULL  0x08018C00
/* <you already got the maximal allowed count of filters > */
#define ACQ_INVFILTSYNTX  0x08018C08
/* <invalid filter syntax> */
#define ACQ_INVPARVAL  0x08018C10
/* <invalid parameter value specified> */
#define ACQ_DUPPLFILTER  0x08018C18
/* <this filter is already in the table> */
#define ACQ_TOOLONGFILTDEF  0x08018C20
/* <too long filter def. string; split it into several filters> */
#define ACQ_NOFILDELWHCOND  0x08018C28
/* <Warning ! A spectrum is conditionned by this filter !> */
#define ACQ_NOFILDELWHCOMP  0x08018C30
/* <Warning ! This filter is used to define a compound filter !> */
#define ACQ_INVFILESPEC  0x08018C38
/* <invalid file specification> */
#define ACQ_NOTAPACTWHRUN  0x08018C40
/* <skip actions on tape are not allowed while running> */
#define ACQ_VMEINIFILNOFND  0x08018C48
/* <VME initialization file not found for auto execution> */
#define ACQ_VMESTAFILNOFND  0x08018C50
/* <VME start of run file not found for auto execution> */
#define ACQ_VMEFIRFILNOFND  0x08018C58
/* <VME FIRST IN RUN file not found for auto execution> */
#define ACQ_VMEENDFILNOFND  0x08018C60
/* <VME end of run file not found for auto execution> */
#define ACQ_NONSTWHMOUN  0x08018C68
/* <Please dismount tapes before reload storage process> */
#define ACQ_NOMODCHGWHRUN  0x08018C70
/* <buffer selection mode is not allowed while acquisition is running> */
#define ACQ_ERRPARAM  0x08018C82
/* <parameter error> */
#define ACQ_VMAXTBIG  0x08018C8A
/* <maximal value out of range> */
#define ACQ_VMINTBIG  0x08018C92
/* <minimal value out of range> */
#define ACQ_VTOOBIG  0x08018C9A
/* <value out of range> */
#define ACQ_CNXREFUS  0x08018CA2
/* <connection to ACQUISITION refused> */
#define ACQ_TOOMUCHCHAR  0x08018CAA
/* <too much characters in this definition > */
#define ACQ_RAFBUFOVF  0x08018CB2
/* <overflow of the refresh buffer> */
#define ACQ_NOREPCTRL  0x08018CBA
/* <the ACQ_CTRL_DATA process doesn't answer...sorry!> */
#define ACQ_IOFORT  0x08018CC2
/* <i/o error detected in fortran calls> */
#define ACQ_INVARG  0x08018CCA
/* <invalid or missing argument(s) in your command> */
#define ACQ_ERRDURREAD  0x08018CD2
/* <error occurs while reading a tape or disk file> */
#define ACQ_ERRNUMSPEC  0x08018CDA
/* <call to acq_inc_user... routines for unknown spectra> */
#define ACQ_ERRTYPSPEC  0x08018CE2
/* <call to acq_inc_user... routines for spectra of another type> */
#define ACQ_ERRSIZSPEC  0x08018CEA
/* <call to acq_inc_user... with an index larger than the spectra size> */
#define ACQ_BADTAPEFORM  0x08018CF2
/* <format of tape or disk file is not compatible with IN2P3 standard> */
#define ACQ_ERRFROMVME  0x08018CFA
/* <a fatal error message was issued from VME crate> */
#define ACQ_VMECOMLEN  0x08018D02
/* <Length of vme command line too big> */
#define ACQ_TAPEISVERYFULL  0x08018D0A
/* <problem of synchro with VME, and must close the file on tape> */
#define ACQ_VMETCPNOMEM  0x08018D12
/* < TCPVME: not enough memory > */
#define ACQ_TCPREPTOOSMALL  0x08018D1A
/* < TCPVME: answer buffer is too small> */
#define ACQ_IOFAMFILE  0x08018D22
/* <Input/Output error with the spectrum family file> */
#define ACQ_ERRSETSYMBFAM  0x08018D2A
/* <Cannot set the symbol for one spectra family> */
#define ACQ_STOSPECIO  0x08018D32
/* <i/o error in storage operation of spectra> */
#define ACQ_STOCONTOURIO  0x08018D3A
/* <i/o error in storage operation of contours> */
#define ACQ_READCONTOURIO  0x08018D42
/* <i/o error in reading operation of contours> */
#define ACQ_ERRFILTFILEIO  0x08018D4A
/* <error in filter file input/output> */
#define ACQ_INVDATINBUF  0x08018D52
/* <a buffer contains invalid datas, contact system manager> */
#define ACQ_NODEVMOUNT  0x08018D5A
/* <no storage operation is possible because no device is mounted> */
#define ACQ_NODLINVALID  0x08018FA4
/* <the ODL'number is invalid (not initialized or false)> */
#define ACQ_READEXIT  0x08018FAC
/* <the process ACQ_READ_DATA exits now> */
#define ACQ_STOREEXIT  0x08018FB4
/* <the process ACQ_STORE_DATA exits now> */
#define ACQ_CTRLEXIT  0x08018FBC
/* <the process ACQ_CTRL_DATA exits now> */
#define ACQ_ACQUISEXIT  0x08018FC4
/* <the process ACQUISITION exits now> */
#define ACQ_PRCRASH  0x08018FCC
/* <one of the acquisition process recently crashed> */
#define ACQ_FATERRREC  0x08018FD4
/* < did receive a fatal error code message > */
#define ACQ_ERRFICCOEF  0x08018FDC
/* <error detected in the initialization file of user coefficients> */
#define ACQ_ERRFICSPECTRE  0x08018FE4
/* <error detected in the initialization file of spectra> */
#define ACQ_ERRFICPARBR  0x08018FEC
/* <error detected in the initialization file of parameters> */
#define ACQ_ERRFICPARCA  0x08018FF4
/* <error detected in the initialization file of calculated parameters> */
#define ACQ_ERRDICOSPEC  0x08018FFC
/* <error detected in spectra's description tables> */
#define ACQ_ERRDICOPAR  0x08019004
/* <error detected in parameters' description tables> */
#define ACQ_ERRDICOCNTOU  0x0801900C
/* <error detected in contours' description tables> */
#define ACQ_ERRDICOFILT  0x08019014
/* <error detected in filters' description tables> */
#define ACQ_ERREVTCALC  0x0801901C
/* <error in user program; too many calc. parameters put into event> */
#define ACQ_ERREVNTLEN  0x08019024
/* <error detected in the control event length> */
#define ACQ_EVTDESCIO  0x0801902C
/* <i/o error by reading the event descriptor file> */
#define ACQ_EVTDESCFMT  0x08019034
/* <format error in the event descriptor file> */
#define ACQ_EVTDESCLEN  0x0801903C
/* <the lentgh of the event descriptor file is too big> */
#define ACQ_PARDESCFMT  0x08019044
/* <a format error was detected in the parameter table> */
#define ACQ_CTRLIOEXIT  0x0801904C
/* <the process ACQ_CTRL_IO exits now> */
#define ACQ_UNKNODEID  0x08019054
/* <the remote node identifier is unknown (symbol ACQ_REMOTE_NODE_ID)> */
#define ACQ_CTRLUSRABT  0x0801905C
/* <this abort error was signaled in the user part of control process> */
#define ACQ_DRB32ERR  0x08019064
/* <a fatal error was detected in the DRB32 driver...> */
#define ACQ_ERROPNDECNET  0x0801906C
/* <error occurs when opening a decnet task_to_task connection> */
#define ACQ_ERRSPECLEN  0x08019074
/* <error on length while read_spectrum operation> */
#define ACQ_ERRLOCKSPEC  0x0801907C
/* <fatal error on lock counter...please call manager> */
#define ACQ_ERRALLOCSPEC  0x08019084
/* <fatal error on mapping while read_spectrum operation> */
#define ACQ_STOBUFTOOBIG  0x0801908C
/* <the buffer size is too big to store under IN2P3 standard> */
#define ACQ_FATPRCRASH  0x08019094
/* <the acquisition process recently crashed> */
#define ACQ_NOMORESTORE  0x0801909C
/* <acquisition stopped because of tape or disk drive error> */
#define ACQ_INVDATABUF  0x080190A4
/* <This buffer is not a data buffer (not EVENTDB header)> */
#define ACQ_ERRDATABUF  0x080190AC
/* <An error was found in this data buffer > */
#define ACQ_READCRASH  0x080190B4
/* < Process ACQ_READ_DATA recently crashed> */
#define ACQ_CTRLCRASH  0x080190BC
/* < Process ACQ_CTRL_DATA process recently crashed> */
#define ACQ_STORCRASH  0x080190C4
/* < Process ACQ_STORE_DATA process recently crashed> */
#define ACQ_ERRINIVME  0x080190CC
/* < Error from VME during initialization phase> */
#define ACQ_VMENOBUFREP  0x080190D4
/* < TCPVME: no answer buffer and not enough memory, close connection> */
#define ACQ_UNKNDATLINK  0x080190DC
/* < Type of data link is unknown, see symbol ACQ_TYPE_OF_DATA_LINK> */
#define ACQ_CENTRALERR  0x080190E4
/* < a fatal error was detected in the central multiplexer crate...> */
#define ACQ_NOREQSPACE  0x080190EC
/* <not enough free space on disk to start a new run> */
#define ACQ_DISKFULL  0x080190F4
/* <no more disk space; current run stopped> */
#define ACQ_TOOMCHINVDAT  0x080190FC
/* <too much consecutive invalid buffers. Contact system manager > */
#define ACQ_NOVMEBUFDISP  0x08018643
/* <there is no ready buffer in VME crate> */
#define ACQ_BUFINATTENDU  0x08018648
/* <store (control) buffer when nostore (nocontrol) from VME> */
#define ACQ_BUFTYPUNKNOWN  0x08018650
/* <receive a buffer with an unknown type from VME> */
#define ACQ_BUFTYPERR  0x08018658
/* <control process receive a non control buffer> */
#define ACQ_BUFVIDE  0x08018660
/* <a new control buffer doesn't contain any event...> */
#define ACQ_VMELBUFER  0x08018668
/* <the received buffer has not the right length> */
#define ACQ_FIFOEMPTY  0x080188C3
/* <attempt to get from an empty queue> */
#define ACQ_SIZTOOBIG  0x080188C8
/* <queue size is too big> */
#define ACQ_BUFSIZTOOBIG  0x080188D0
/* <buffer size is too big> */
#define ACQ_FIFOFULL  0x080188D8
/* <attempt to put in queue which is already full> */
#define ACQ_FIFODISAB  0x080188E0
/* <attempt to put or get in queue while disabled> */
#define ACQ_QNOEMPTY  0x080188EA
/* <attempt to modify queue characteristics while not empty> */
#define ACQ_BUFQNINIT  0x080188F2
/* <buffer-queue not initialized> */
#define ACQ_ERLOCKMGR  0x080188FC
/* <error detected in lock manager...call system manager> */
#ifdef __cplusplus
}
#endif
