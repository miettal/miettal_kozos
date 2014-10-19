#ifndef _DEFINES_H_INCLUDED_
#define _DEFINES_H_INCLUDED_

#define NULL ((void *)0)
#define SERIAL_DEFAULT_DEVICE 1
#define SERIAL_DEBUG_DEVICE 2
#define TIMER_DEFAULT_DEVICE  1

typedef unsigned char  uint8;
typedef unsigned short uint16;
typedef unsigned long  uint32;

typedef uint32 kz_thread_id_t;
typedef int (*kz_func_t)(int argc, char *argv[]);
typedef void (*kz_handler_t)(void);

typedef enum {
  MSGBOX_ID_CONSINPUT = 0,
  MSGBOX_ID_CONSOUTPUT,
  MSGBOX_ID_TIMDRIVE,
  MSGBOX_ID_TIMEXPIRE,
  MSGBOX_ID_NETPROC,
  MSGBOX_ID_ETHPROC,
  MSGBOX_ID_ARPPROC,
  MSGBOX_ID_ARPADRLIST,
  MSGBOX_ID_ARPPKTLIST,
  MSGBOX_ID_IPPROC,
  MSGBOX_ID_ICMPPROC,
  MSGBOX_ID_TCPPROC,
  MSGBOX_ID_TCPCONLIST,
  MSGBOX_ID_HTTPD,
  MSGBOX_ID_UDPPROC,
  MSGBOX_ID_UDPPORTLIST,
  MSGBOX_ID_TFTP,
  MSGBOX_ID_NUM
} kz_msgbox_id_t;

#endif
