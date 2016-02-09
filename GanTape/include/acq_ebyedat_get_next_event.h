#ifdef __cplusplus
extern "C" {
#endif


/* Get next in EBYEDAT data buffers (exogam) */
int acq_ebyedat_get_next_event(UNSINT16* Buffer,
                               UNSINT16** EvtAddr,
                               int*   EvtNum,
                               int    EvtFormat);

/* Get next in EBYEDAT data buffers (exogam) ; reentrant version */
int acq_ebyedat_get_next_event_r(UNSINT16* Buffer,
                                 UNSINT16** EvtAddr,
                                 int*   EvtNum,
                                 int    EvtFormat,
                                 UNSINT16** NextEvent);

#ifdef __cplusplus
}
#endif
