#include "defines.h"
#include "kozos.h"
#include "interrupt.h"
#include "serial.h"
#include "lib.h"

/* �����ƥࡦ�������ȥ桼�����������ε�ư */
static int start_threads(int argc, char *argv[])
{
  kz_run(consdrv_main,  "consdrv",  1, 0x100, 0, NULL);
  kz_run(command_main,  "command",  8, 0x100, 0, NULL);
  kz_run(timerdrv_main, "timerdrv", 2, 0x100, 0, NULL);
#if 0
  kz_run(clock_main,    "clock",    9, 0x100, 0, NULL);
#endif
  kz_run(netdrv_main,   "netdrv",  10, 0x100, 0, NULL);
  kz_run(ethernet_main, "ethernet",11, 0x100, 0, NULL);
  kz_run(arp_main,      "arp",     11, 0x100, 0, NULL);
  kz_run(ip_main,       "ip",      12, 0x100, 0, NULL);
  kz_run(icmp_main,     "icmp",    12, 0x100, 0, NULL);
  kz_run(tcp_main,      "tcp",     12, 0x100, 0, NULL);
  kz_run(udp_main,      "udp",     12, 0x100, 0, NULL);
  kz_run(httpd_main,    "httpd",   14, 0x100, 0, NULL);
  kz_run(tftp_main,     "tftp",    14, 0x100, 0, NULL);

  kz_chpri(15); /* ͥ���̤򲼤��ơ������ɥ륹��åɤ˰ܹԤ��� */
  INTR_ENABLE; /* �����ͭ���ˤ��� */
  while (1) {
    asm volatile ("sleep"); /* �����ϥ⡼�ɤ˰ܹ� */
  }

  return 0;
}

int main(void)
{
  INTR_DISABLE; /* �����̵���ˤ��� */

#ifdef SIMULATOR
  extern int bss_start, ebss;

  memset(&bss_start, 0, (long)&ebss - (long)&bss_start);

  /* ���եȥ������������ߥ٥������������� */
  softvec_init();

  /* ���ꥢ��ν���� */
  serial_init(SERIAL_DEFAULT_DEVICE);

#if 0
  /* DRAM�ν���� */
  dram_init();
#endif
#endif

  puts("kozos boot succeed!\n");

  /* OS��ư��� */
  kz_start(start_threads, "idle", 0, 0x100, 0, NULL);
  /* �����ˤ���äƤ��ʤ� */

  return 0;
}
