#include "defines.h"
#include "kozos.h"
#include "intr.h"
#include "stub.h"
#include "interrupt.h"

enum {
  STUB_SIGHUP  =  1,
  STUB_SIGILL  =  4,
  STUB_SIGTRAP =  5,
  STUB_SIGEMT  =  7,
  STUB_SIGBUS  = 10,
  STUB_SIGSEGV = 11,
};

#define BUFFER_SIZE 2048
static char recvbuf[BUFFER_SIZE];
static char sendbuf[BUFFER_SIZE];

#define REGISTERS_NUM 10
#define REGISTERS_SIZE (4 * REGISTERS_NUM)

/* gdb/include/gdb/sim-h8300.h */
/* gdb/h8300-tdep.c */
enum {
  REG_ER0, REG_ER1, REG_ER2, REG_ER3,
  REG_ER4, REG_ER5, REG_ER6, REG_ER7,
  REG_CCR, REG_PC,
};

static unsigned long registers[REGISTERS_NUM];

/* 独立したシリアルドライバを利用するため，別名を定義してインポートする */
#define serial_init debug_serial_init
#define serial_is_send_enable debug_serial_is_send_enable
#define serial_send_byte debug_serial_send_byte
#define serial_is_recv_enable debug_serial_is_recv_enable
#define serial_recv_byte debug_serial_recv_byte
#define serial_intr_is_send_enable debug_serial_intr_is_send_enable
#define serial_intr_send_enable debug_serial_intr_send_enable
#define serial_intr_send_disable debug_serial_intr_send_disable
#define serial_intr_is_recv_enable debug_serial_intr_is_recv_enable
#define serial_intr_recv_enable debug_serial_intr_recv_enable
#define serial_intr_recv_disable debug_serial_intr_recv_disable
#include "serial.c"

static void putDebugChar(char c)
{
  serial_send_byte(SERIAL_DEBUG_DEVICE, c);
}

static int getDebugChar()
{
  return serial_recv_byte(SERIAL_DEBUG_DEVICE);
}

static char *stub_strcpy(char *dst, const char *src)
{
  char *d = dst;
  for (;; dst++, src++) {
    *dst = *src;
    if (!*src) break;
  }
  return d;
}

static void *stub_memcpy(void *dst, const void *src, int size)
{
  char *d = dst;
  const char *s = src;
  for (; size; size--, d++, s++) {
    *d = *s;
  }
  return dst;
}

static int a2h(unsigned char c)
{
  if ((c >= '0') && (c <= '9'))
    return c - '0';
  if ((c >= 'a') && (c <= 'f'))
    return c - 'a' + 10;
  if ((c >= 'A') && (c <= 'F'))
    return c - 'A' + 10;
  return -1;
}

static char h2a(int val)
{
  if ((val >= 0) && (val < 16))
    return "0123456789abcdef"[val];
  return '\0';
}

static unsigned char *stub_gets(unsigned char *buffer)
{
  int i = 0;
  unsigned char c = 0, sum = 0, datasum = 0;
  enum {
    GETS_STATUS_WAIT_DOLLAR,
    GETS_STATUS_READY_READ,
    GETS_STATUS_READ_DATA,
    GETS_STATUS_READ_SUM_UPPER,
    GETS_STATUS_READ_SUM_LOWER,
    GETS_STATUS_END,
  } status;

  status = GETS_STATUS_WAIT_DOLLAR;

  while (status != GETS_STATUS_END) {
    if (status != GETS_STATUS_READY_READ)
      c = getDebugChar();
    switch (status) {
      case GETS_STATUS_WAIT_DOLLAR:
        if (c == '$') status = GETS_STATUS_READY_READ;
        break;
      case GETS_STATUS_READY_READ:
        i = 0;
        sum = datasum = 0;
        status = GETS_STATUS_READ_DATA;
        break;
      case GETS_STATUS_READ_DATA:
        if (c == '$') {
          status = GETS_STATUS_READY_READ;
          break;
        }
        if (c == '#') {
          buffer[i] = '\0';
          status = GETS_STATUS_READ_SUM_UPPER;
          break;
        }
        sum += c;
        buffer[i++] = c;
        break;
      case GETS_STATUS_READ_SUM_UPPER:
        datasum = a2h(c) << 4;
        status = GETS_STATUS_READ_SUM_LOWER;
        break;
      case GETS_STATUS_READ_SUM_LOWER:
        datasum |= a2h(c);
        if (sum != datasum) {
          putDebugChar('-');
          status = GETS_STATUS_WAIT_DOLLAR;
        } else {
          status = GETS_STATUS_END;
        }
        break;
      default:
        break;
    }
  }

  putDebugChar('+');
  if ((i > 2) && (buffer[2] == ':')) {
    putDebugChar(buffer[0]);
    putDebugChar(buffer[1]);
    return &buffer[3];
  }
  return buffer;
}

static unsigned char *stub_puts(unsigned char *buffer)
{
  unsigned char c, sum;
  int i;

  while (1) {
    putDebugChar('$');

    i = 0;
    sum = 0;
    while ((c = buffer[i++]) != '\0') {
      putDebugChar(c);
      sum += c;
    }

    putDebugChar('#');
    putDebugChar(h2a((sum >> 4) & 0xf));
    putDebugChar(h2a(sum & 0xf));

    if (getDebugChar() == '+')
      break;
  }

  return buffer;
}

static unsigned char *read_memory(void *addr, unsigned char *buffer, int size)
{
  unsigned char c;
  int i;

  for (i = 0; i < size; i++) {
    c = ((unsigned char *)addr)[i];
    *(buffer++) = h2a((c >> 4) & 0xf);
    *(buffer++) = h2a(c & 0xf);
  }
  *buffer = 0;

  return buffer;
}

static void *write_memory(unsigned char *buffer, void *addr, int size)
{
  unsigned char c;
  unsigned char *cp = addr;
  int i;

  for (i = 0; i < size; i++) {
    c  = a2h(*(buffer++)) << 4;
    c |= a2h(*(buffer++));
    *(cp++) = c;
  }

  return cp;
}

static int a2val(unsigned char **pp, long *valp)
{
  int size = 0;
  long v, val = 0;
  unsigned char *p;

  p = *pp;
  while (1) {
    v = a2h(*p);
    if (v < 0) break;
    val = (val << 4) | v;
    p++;
    size++;
  }
  *valp = val;
  *pp = p;

  return size;
}

#define H8_300H
#define BREAK_CODE "\x57\x30"
#define BREAK_CODE_SIZE 2

struct instruction {
  void *addr;
  char byte[BREAK_CODE_SIZE];
} instructions[2];

static int get_instruction_size(void *addr)
{
  unsigned char *code = addr;
  int bh, bl, dh, size;
  static char size_all[0x100] = {
    /*1 2 3 4 5 6 7  8 9 a b c d e f*/
    2,1,2,2,2,2,2,2, 2,2,2,2,2,2,2,2, /* 0 */
    2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2, /* 1 */
    2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2, /* 2 */
    2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2, /* 3 */
    2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2, /* 4 */
    2,2,2,2,2,2,2,2, 4,2,4,2,4,2,4,2, /* 5 */
    2,2,2,2,2,2,2,2, 2,2,1,1,2,2,4,4, /* 6 */
    2,2,2,2,2,2,2,2, 8,4,6,4,4,4,4,4, /* 7 */

    2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2, /* 8 */
    2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2, /* 9 */
    2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2, /* a */
    2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2, /* b */
    2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2, /* c */
    2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2, /* d */
    2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2, /* e */
    2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2, /* f */
  };
  static char size_6a[0x10] = {
    4,0,6,0,4,0,0,0, 4,0,6,0,4,0,0,0,
  };
  static char size_6b[0x10] = {
    4,0,6,0,0,0,0,0, 4,0,6,0,0,0,0,0,
  };
  static char size_01[0x10] = {
    1,0,0,0,1,0,0,0, 2,0,0,0,4,4,0,4,
  };
  static char size_01_00_6b[0x10] = {
    6,0,8,0,0,0,0,0, 6,0,8,0,0,0,0,0,
  };

  bh = (code[1] >> 4) & 0xf;
  bl = code[1] & 0xf;
  dh = (code[3] >> 4) & 0xf;

  size = size_all[code[0]];

  if (size == 1) {
    switch (code[0]) {
      case 0x6a: size = size_6a[bh]; break;
      case 0x6b: size = size_6b[bh]; break;
      case 0x01:
                 if (bl == 0x0) {
                   size = size_01[bh];
                   if (size == 1) {
                     switch (code[2]) {
                       case 0x69: size =  4; break;
                       case 0x6d: size =  4; break;
                       case 0x6f: size =  6; break;
                       case 0x78: size = 10; break;
                       case 0x6b:
                                  size = size_01_00_6b[dh];
                                  break;
                       default:
                                  break;
                     }
                   }
                 }
                 break;
      default:
                 break;
    }
  }

  return size;
}

static void *get_jmp_addr(void *addr)
{
  unsigned char *code = addr;
  unsigned long pc, jmp_addr = 0;
  int regnum;
  unsigned char *memaddr;

  pc = (unsigned long)((char *)addr + get_instruction_size(addr));

#define ULONG1(v0) ((unsigned long)((unsigned char)(v0)))
#define ULONG2(v0,v1)    ((ULONG1(v0) <<  8) |  ULONG1(v1))
#define ULONG3(v0,v1,v2) ((ULONG1(v0) << 16) | (ULONG1(v1) << 8) | ULONG1(v2))
#define SLONG1(v0)    ((signed long)((signed char)(v0)))
#define SLONG2(v0,v1) ((signed long)((signed short)ULONG2(v0,v1)))

  //putc('\n');
  //// トラップ命令
  //if (code[0] == 0x57) {
  //  switch(code[1]){
  //    case 0x00: // syscal
  //      jmp_addr = SOFTVECS[SOFTVEC_TYPE_SYSCALL];
  //      break;
  //    case 0x10: // softerr
  //      jmp_addr = SOFTVECS[SOFTVEC_TYPE_SOFTERR];
  //      break;
  //    case 0x20: // softerr
  //      jmp_addr = SOFTVECS[SOFTVEC_TYPE_SOFTERR];
  //      break;
  //    case 0x30: // bkpoint
  //      // do nothing
  //      break;
  //    default :
  //      break;
  //  }
  //  putc('\n');
  //}

  /* 0x40〜0x4f (bXX) ８ビットPC相対分岐命令 */
  if ((code[0] & 0xf0) == 0x40) {
    jmp_addr = pc + SLONG1(code[1]); /* 符号拡張が必要 */
  }

#ifdef H8_300H
  /* 0x5800〜0x58f0 (bXX d:16) 16ビットPC相対分岐命令(H8/300Hのみ) */
  if ((code[0] == 0x58) && ((code[1] & 0x0f) == 0)) {
    jmp_addr = pc + SLONG2(code[2], code[3]); /* 符号拡張が必要 */
  }
#endif

  /* 0x59 (jmp @ERn), 0x5d (jsr @ERn) ジャンプ命令(絶対アドレス) */
  if ((code[0] == 0x59) || (code[0] == 0x5d)) {
    if ((code[1] & ~0x70) == 0) { /* 下位４ビットがゼロ以外のときは不正命令*/
      /*レジスタ間接*/
      regnum = (code[1] >> 4) & 0x7;
      jmp_addr = registers[regnum];
    }
  }

  /* 0x5a (jmp @aa:24), 0x5e (jsr @aa:24) 24ビットジャンプ(絶対アドレス) */
  if ((code[0] == 0x5a) || (code[0] == 0x5e)) {
    jmp_addr = ULONG3(code[1], code[2], code[3]);
  }

  /* 0x5b (jmp @@aa:8), 0x5f (jsr @@aa:8) ８ビットジャンプ(メモリ間接) */
  if ((code[0] == 0x5b) || (code[0] == 0x5f)) { /* JMP/JSR @@aa:8 */
    memaddr = (unsigned char *)ULONG1(code[1]); /* ８ビット絶対アドレス */
#ifdef H8_300H /* 24ビット指定で最上位バイトは無視 */
    jmp_addr = ULONG3(memaddr[1], memaddr[2], memaddr[3]);
#else /* 16ビット指定 */
    jmp_addr = ULONG2(memaddr[0], memaddr[1]);
#endif
  }

  /* 0x55 (bsr d:8) ８ビットPC相対サブルーチンコール */
  if (code[0] == 0x55) {
    jmp_addr = pc + SLONG1(code[1]); /* 符号拡張が必要 */
  }

  /* 0x5c (bsr d:16) 16ビットPC相対サブルーチンコール */
  /*
   * 通常のブランチ命令では16ビットアドレス指定はH8/300Hでのオプションだが，
   * bsrはH8/300でも16ビット命令が利用できるようだ．
   */
  if ((code[0] == 0x5c) && (code[1] == 0x00)) {
    jmp_addr = pc + SLONG2(code[2], code[3]); /* 符号拡張が必要 */
  }

  /* 0x54 (rts,rts/l) 関数復帰 */
  if (code[0] == 0x54) {
    regnum = (code[1] >> 4) & 0x7;
    memaddr = (unsigned char *)registers[regnum];
#ifdef H8_300H
    jmp_addr = ULONG3(memaddr[1], memaddr[2], memaddr[3]);
#else
    jmp_addr = ULONG2(memaddr[0], memaddr[1]);
#endif
  }

  /* 0x56 (rte,rte/l) 割込み復帰 */
  if (code[0] == 0x56) {
    regnum = (code[1] >> 4) & 0x7;
    memaddr = (unsigned char *)registers[regnum];
#ifdef H8_300H
    jmp_addr = ULONG3(memaddr[1], memaddr[2], memaddr[3]);
#else
    /* スタック先頭はCCRの値なので，PCは２バイト目以降から復旧する */
    jmp_addr = ULONG2(memaddr[2], memaddr[3]);
#endif
  }

  jmp_addr &= ~0xff000001;

  return (void *)jmp_addr;
}

static void store_instruction(struct instruction *inst, void *addr)
{
  inst->addr = addr;
  stub_memcpy(inst->byte, inst->addr, BREAK_CODE_SIZE);
}

static void restore_instruction(struct instruction *inst)
{
  if (inst->addr) {
    stub_memcpy(inst->addr, inst->byte, BREAK_CODE_SIZE);
    inst->addr = NULL;
  }
}

static int check_instruction(struct instruction *inst, void *addr)
{
  return (inst->addr && (inst->addr == addr)) ? 1 : 0;
}

static void init_softstep()
{
  instructions[0].addr = NULL;
  instructions[1].addr = NULL;
}

static void set_softstep(unsigned long pc)
{
  void *addr = (void *)pc;
  void *jmp_addr, *next_addr;

  jmp_addr = get_jmp_addr(addr);
  next_addr = (char *)addr + get_instruction_size(addr);

  store_instruction(&instructions[0], next_addr);
  stub_memcpy(next_addr, BREAK_CODE, BREAK_CODE_SIZE);

  if (jmp_addr) {
    store_instruction(&instructions[1], jmp_addr);
    stub_memcpy(jmp_addr, BREAK_CODE, BREAK_CODE_SIZE);
  }
}

static void reset_softstep()
{
  restore_instruction(&instructions[1]);
  restore_instruction(&instructions[0]);
}

static unsigned long check_softstep(unsigned long pc)
{
  unsigned long prev = pc - BREAK_CODE_SIZE;
  if (check_instruction(&instructions[0], (void *)prev)) return prev;
  if (check_instruction(&instructions[1], (void *)prev)) return prev;
  return pc;
}

static void handle_exception()
{
  int sig = STUB_SIGTRAP;
  unsigned char *p;
  long addr, size;

  p = sendbuf;
  *(p++) = 'T';
  *(p++) = h2a((sig >> 4) & 0xf);
  *(p++) = h2a(sig & 0xf);

#if 0
  *(p++) = h2a((REG_PC >> 4) & 0xf);
  *(p++) = h2a(REG_PC & 0xf);
  *(p++) = ':';
  p = read_memory((char *)&registers[REG_PC], p, 4);
  *(p++) = ';';
#endif
  *(p++) = '\0';

  stub_puts(sendbuf);

  while (1) {
    stub_gets(recvbuf);

    sendbuf[0] = '\0'; /* not support */

    p = recvbuf;
    switch (*(p++)) {
      case '?':
        sendbuf[0] = 'S';
        sendbuf[1] = h2a((sig >> 4) & 0xf);
        sendbuf[2] = h2a(sig & 0xf);
        sendbuf[3] = '\0';
        break;

      case 'g':
        read_memory(registers, sendbuf, REGISTERS_SIZE);
        break;

      case 'G':
        write_memory(sendbuf, registers, REGISTERS_SIZE);
        break;

      case 'm':
        a2val(&p, &addr);
        if (*(p++) != ',') {
          stub_strcpy(sendbuf, "E01");
          break;
        }
        a2val(&p, &size);
        read_memory((void *)addr, sendbuf, size);
        break;
      case 'M':
        a2val(&p, &addr);
        if (*(p++) != ',') {
          stub_strcpy(sendbuf, "E01");
          break;
        }
        a2val(&p, &size);
        if (*(p++) != ':') {
          stub_strcpy(sendbuf, "E02");
          break;
        }
        write_memory(p, (void *)addr, size);
        stub_strcpy(sendbuf, "OK");
        break;
      case 's':
        set_softstep(registers[REG_PC]);
        /* fall through */
      case 'c':

        /*
         * breakpoint:
         remote.c:remote_insert_breakpoint()
         ->
         memory_insert_breakpoint()
         ->
         mem-break.c:default_memory_insert_breakpoint()
         ->
         gdbarch_breakpoint_from_pc()
         ->
         h8300-tdep.c:h8300_breakpoint_from_pc()

         h8300_breakpoint_from_pc()
         breakpoint[] = { 0x7A, 0xFF }; bpt instruction???
         breakpoint[] = { 0x01, 0x80 }; ??? sleep instruction???
         */

        return;
      default:
        break;
    }
    stub_puts(sendbuf);
  }

  return;
}

static void store_registers(unsigned long sp)
{
  registers[REG_ER0] = ((unsigned long *)sp)[0];
  registers[REG_ER1] = ((unsigned long *)sp)[1];
  registers[REG_ER2] = ((unsigned long *)sp)[2];
  registers[REG_ER3] = ((unsigned long *)sp)[3];
  registers[REG_ER4] = ((unsigned long *)sp)[4];
  registers[REG_ER5] = ((unsigned long *)sp)[5];
  registers[REG_ER6] = ((unsigned long *)sp)[6];
  registers[REG_ER7] = sp + sizeof(int) * 8;

  registers[REG_CCR] = ((((unsigned long *)sp)[7]) >> 24) & 0xff;
  registers[REG_PC ] = ((unsigned long *)sp)[7] & 0xffffff;
}

static void restore_registers(unsigned long sp)
{
  ((unsigned long *)sp)[0] = registers[REG_ER0];
  ((unsigned long *)sp)[1] = registers[REG_ER1];
  ((unsigned long *)sp)[2] = registers[REG_ER2];
  ((unsigned long *)sp)[3] = registers[REG_ER3];
  ((unsigned long *)sp)[4] = registers[REG_ER4];
  ((unsigned long *)sp)[5] = registers[REG_ER5];
  ((unsigned long *)sp)[6] = registers[REG_ER6];
  /* ((unsigned long *)sp)[7] = sp; */

  ((unsigned long *)sp)[7]  = registers[REG_CCR] << 24;
  ((unsigned long *)sp)[7] |= registers[REG_PC ];
}

static void break_intr(softvec_type_t type, unsigned long sp)
{
  int i;
  int send_enable[SERIAL_SCI_NUM];
  int recv_enable[SERIAL_SCI_NUM];

  for (i = 0; i < SERIAL_SCI_NUM; i++) {
    send_enable[i] = serial_intr_is_send_enable(i);
    recv_enable[i] = serial_intr_is_recv_enable(i);
    serial_intr_send_disable(i);
    serial_intr_recv_disable(i);
  }

  store_registers(sp);
  registers[REG_PC] = check_softstep(registers[REG_PC]);
  reset_softstep();
  handle_exception();
  restore_registers(sp);

  for (i = 0; i < SERIAL_SCI_NUM; i++) {
    if (send_enable[i]) serial_intr_send_enable(i);
    if (recv_enable[i]) serial_intr_recv_enable(i);
  }
}

void set_debug_traps()
{
  serial_init(SERIAL_DEBUG_DEVICE);
  /* set interrupt vector for handling breakpoint. */
  softvec_setintr(SOFTVEC_TYPE_BKPOINT, break_intr);
  init_softstep();
}

void force_break()
{
  asm volatile ("trapa #3");
}
