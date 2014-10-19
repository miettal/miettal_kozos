#include "defines.h"
#include "kozos.h"
#include "consdrv.h"
#include "timerdrv.h"
#include "netdrv.h"
#include "icmp.h"
#include "tftp.h"
#include "stub.h"
#include "lib.h"

/* コンソール・ドライバの使用開始をコンソール・ドライバに依頼する */
static void send_use(int index)
{
  char *p;
  p = kz_kmalloc(3);
  p[0] = '0';
  p[1] = CONSDRV_CMD_USE;
  p[2] = '0' + index;
  kz_send(MSGBOX_ID_CONSOUTPUT, 3, p);
}

/* コンソールへの文字列出力をコンソール・ドライバに依頼する */
static void send_write(char *str)
{
  char *p;
  int len;
  len = strlen(str);
  p = kz_kmalloc(len + 2);
  p[0] = '0';
  p[1] = CONSDRV_CMD_WRITE;
  memcpy(&p[2], str, len);
  kz_send(MSGBOX_ID_CONSOUTPUT, len + 2, p);
}

/* タイマのカウント開始をタイマ・ドライバに依頼する */
static void send_start(int msec)
{
  struct timerreq *req;
  req = kz_kmalloc(sizeof(*req));
  req->id = MSGBOX_ID_CONSINPUT;
  req->msec = msec;
  kz_send(MSGBOX_ID_TIMDRIVE, TIMERDRV_CMD_START, (char *)req);
}

/* pingの開始をicmpタスクに依頼する */
static void send_icmp()
{
  struct netbuf *buf;
  buf = kz_kmalloc(sizeof(*buf));
  buf->cmd = ICMP_CMD_SEND;
  buf->option.icmp.send.number = 3;
  buf->option.icmp.send.ipaddr = 0xc0a80a01; /* 192.168.10.1 */
  kz_send(MSGBOX_ID_ICMPPROC, 0, (char *)buf);
}

/* tftpの開始をtftpタスクに依頼する */
static void send_tftp()
{
  struct netbuf *buf;
  buf = kz_kmalloc(sizeof(*buf));
  buf->cmd = TFTP_CMD_START;
  buf->option.tftp.start.ipaddr = 0xc0a80a01; /* 192.168.10.1 */
  kz_send(MSGBOX_ID_TFTP, 0, (char *)buf);
}

char *func(char *str)
{
  static char buf[32];
  strcpy(buf, str);
  return buf;
}

int command_main(int argc, char *argv[])
{
  char *p;
  int size;

  send_use(SERIAL_DEFAULT_DEVICE);
  // GDB用トラップの設定
  set_debug_traps();

  while (1) {
    send_write("command> "); /* プロンプト表示 */

    /* コンソールからの受信文字列を受け取る */
    kz_recv(MSGBOX_ID_CONSINPUT, &size, &p);
    if (p == NULL) {
      send_write("expired.\n");
      continue;
    }
    p[size] = '\0';

    if (!strncmp(p, "echo", 4)) { /* echoコマンド */
      send_write(p + 4); /* echoに続く文字列を出力する */
      send_write("\n");
    } else if (!strncmp(p, "timer", 5)) { /* timerコマンド */
      send_write("timer start.\n");
      send_start(1000);
    } else if (!strncmp(p, "ping", 4)) { /* pingコマンド */
      send_write("ping start.\n");
      send_icmp();
    } else if (!strncmp(p, "tftp", 4)) { /* tftpコマンド */
      send_write("tftp start.\n");
      send_tftp();
    } else if (!strncmp(p, "debug", 5)) { /* デバッガ起動 */
      force_break();
    } else if (!strncmp(p, "call", 4)) { /* ダミー関数の呼び出し */
      send_write(func(p + 4));
    } else if (!strncmp(p, "down", 5)) { /* リセット */
      kz_sysdown();
    } else {
      send_write("unknown.\n");
    }

    kz_kmfree(p);
  }

  return 0;
}
