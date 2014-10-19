#include "defines.h"
#include "interrupt.h"
#include "serial.h"
#include "xmodem.h"
#include "elf.h"
#include "dram.h"
#include "lib.h"

static int init(void)
{
  /* �ʲ��ϥ�󥫡�������ץȤ�������Ƥ��륷��ܥ� */
  extern int romdata_start, data_start, edata, bss_start, ebss;

#ifndef SIMULATOR
  /*
   * �ǡ����ΰ��BSS�ΰ���������롥���ν����ʹߤǤʤ��ȡ�
   * �����Х��ѿ������������Ƥ��ʤ��Τ���ա�
   */
  memcpy(&data_start, &romdata_start, (long)&edata - (long)&data_start);
  memset(&bss_start, 0, (long)&ebss - (long)&bss_start);
#endif

  /* ���եȥ������������ߥ٥������������� */
  softvec_init();

  /* ���ꥢ��ν���� */
  serial_init(SERIAL_DEFAULT_DEVICE);

  /* DRAM�ν���� */
  dram_init();

  return 0;
}

/* �����16�ʥ���׽��� */
static int dump(char *buf, long size)
{
  long i;

  if (size < 0) {
    puts("no data.\n");
    return -1;
  }
  for (i = 0; i < size; i++) {
    putxval(buf[i], 2);
    if ((i & 0xf) == 15) {
      puts("\n");
    } else {
      if ((i & 0xf) == 7) puts(" ");
      puts(" ");
    }
  }
  puts("\n");

  return 0;
}

static void wait()
{
  volatile long i;
  for (i = 0; i < 15000; i++)
    ;
}

int main(void)
{
  static char buf[16];
  static long size = -1;
  static unsigned char *loadbuf = NULL;
  char *entry_point;
  void (*f)(void);
  extern int buffer_start; /* ��󥫡�������ץȤ��������Ƥ���Хåե� */
  extern int firmdata_start, efirmdata;

  INTR_DISABLE; /* �����̵���ˤ��� */

  init();

  puts("kzload (kozos boot loader) started.\n");

  while (1) {
    puts("kzload> "); /* �ץ��ץ�ɽ�� */
    gets(buf); /* ���ꥢ�뤫��Υ��ޥ�ɼ��� */

    if (!strcmp(buf, "load")) { /* XMODEM�ǤΥե�����Υ�������� */
      loadbuf = (char *)(&buffer_start);
      size = xmodem_recv(loadbuf);
      wait(); /* ž�����ץ꤬��λ��ü�����ץ�����椬���ޤ��Ԥ���碌�� */
      if (size < 0) {
        puts("\nXMODEM receive error!\n");
      } else {
        puts("\nXMODEM receive succeeded.\n");
      }
    } else if (!strcmp(buf, "loadfirm")) { /* ɸ��Υե������Ÿ�� */
      loadbuf = (char *)(&buffer_start);
      memcpy(loadbuf, &firmdata_start, (long)&efirmdata - (long)&firmdata_start);
      puts("\nloaded default firmware.\n");
    } else if (!strcmp(buf, "dump")) { /* �����16�ʥ���׽��� */
      puts("size: ");
      putxval(size, 0);
      puts("\n");
      dump(loadbuf, size);
    } else if (!strcmp(buf, "run")) { /* ELF�����ե�����μ¹� */
      entry_point = elf_load(loadbuf); /* ������Ÿ��(����) */
      if (!entry_point) {
        puts("run error!\n");
      } else {
        puts("starting from entry point: ");
        putxval((unsigned long)entry_point, 0);
        puts("\n");
        f = (void (*)(void))entry_point;
        f(); /* �����ǡ����ɤ����ץ����˽������Ϥ� */
        /* �����ˤ��֤äƤ��ʤ� */
      }
    } else if (!strcmp(buf, "ramchk")) {
      dram_check();
    } else if (!strcmp(buf, "ramchk2")) {
      dram_check2();
    } else if (!strcmp(buf, "ramclr")) {
      dram_clear();
    } else {
      puts("unknown.\n");
    }
  }

  return 0;
}
