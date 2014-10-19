#ifndef _KOZOS_H_INCLUDED_
#define _KOZOS_H_INCLUDED_

#include "defines.h"
#include "interrupt.h"
#include "syscall.h"

/* システム・コール */
kz_thread_id_t kz_run(kz_func_t func, char *name, int priority, int stacksize,
		      int argc, char *argv[]);
void kz_exit(void);
int kz_wait(void);
int kz_sleep(void);
int kz_wakeup(kz_thread_id_t id);
kz_thread_id_t kz_getid(void);
int kz_chpri(int priority);
void *kz_kmalloc(int size);
int kz_kmfree(void *p);
int kz_send(kz_msgbox_id_t id, int size, char *p);
kz_thread_id_t kz_recv(kz_msgbox_id_t id, int *sizep, char **pp);
int kz_setintr(softvec_type_t type, kz_handler_t handler);

/* サービス・コール */
int kx_wakeup(kz_thread_id_t id);
void *kx_kmalloc(int size);
int kx_kmfree(void *p);
int kx_send(kz_msgbox_id_t id, int size, char *p);

/* ライブラリ関数 */
void kz_start(kz_func_t func, char *name, int priority, int stacksize,
	      int argc, char *argv[]);
void kz_sysdown(void);
void kz_sysreset(void);
void kz_syscall(kz_syscall_type_t type, kz_syscall_param_t *param);
void kz_srvcall(kz_syscall_type_t type, kz_syscall_param_t *param);

/* システム・タスク */
int consdrv_main(int argc, char *argv[]);
int timerdrv_main(int argc, char *argv[]);
int netdrv_main(int argc, char *argv[]);

/* ユーザ・タスク */
int command_main(int argc, char *argv[]);
int clock_main(int argc, char *argv[]);
int ethernet_main(int argc, char *argv[]);
int arp_main(int argc, char *argv[]);
int ip_main(int argc, char *argv[]);
int icmp_main(int argc, char *argv[]);
int tcp_main(int argc, char *argv[]);
int udp_main(int argc, char *argv[]);
int httpd_main(int argc, char *argv[]);
int tftp_main(int argc, char *argv[]);

#define kz_sysdown() do{\
  puts("system down! at file:" __FILE__", line:");\
  putval(__LINE__);\
  puts("\n\n");\
  ((void (*)(void))0)();\
}while(0)

#endif
