#include "defines.h"
#include "kozos.h"
#include "interrupt.h"
#include "serial.h"
#include "lib.h"

/* システム・タスクとユーザ・タスクの起動 */
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

  kz_chpri(15); /* 優先順位を下げて，アイドルスレッドに移行する */
  INTR_ENABLE; /* 割込み有効にする */
  while (1) {
    asm volatile ("sleep"); /* 省電力モードに移行 */
  }

  return 0;
}

int main(void)
{
  INTR_DISABLE; /* 割込み無効にする */

#ifdef SIMULATOR
  extern int bss_start, ebss;

  memset(&bss_start, 0, (long)&ebss - (long)&bss_start);

  /* ソフトウエア・割り込みベクタを初期化する */
  softvec_init();

  /* シリアルの初期化 */
  serial_init(SERIAL_DEFAULT_DEVICE);

#if 0
  /* DRAMの初期化 */
  dram_init();
#endif
#endif

  puts("kozos boot succeed!\n");

  /* OSの動作開始 */
  kz_start(start_threads, "idle", 0, 0x100, 0, NULL);
  /* ここには戻ってこない */

  return 0;
}
