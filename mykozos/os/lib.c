#include "defines.h"
#include "serial.h"
#include "lib.h"

void *memset(void *b, int c, long len)
{
  char *p;
  for (p = b; len > 0; len--)
    *(p++) = c;
  return b;
}

void *memcpy(void *dst, const void *src, long len)
{
  char *d = dst;
  const char *s = src;
  for (; len > 0; len--)
    *(d++) = *(s++);
  return dst;
}

void *memmove(void *dst, const void *src, long len)
{
  char *d = dst;
  const char *s = src;

  if (d < s) {
    for (; len > 0; len--)
      *(d++) = *(s++);
  } else {
    d += len;
    s += len;
    for (; len > 0; len--)
      *(--d) = *(--s);
  }

  return dst;
}

int memcmp(const void *b1, const void *b2, long len)
{
  const char *p1 = b1, *p2 = b2;
  for (; len > 0; len--) {
    if (*p1 != *p2)
      return (*p1 > *p2) ? 1 : -1;
    p1++;
    p2++;
  }
  return 0;
}

int strlen(const char *s)
{
  int len;
  for (len = 0; *s; s++, len++)
    ;
  return len;
}

char *strcpy(char *dst, const char *src)
{
  char *d = dst;
  for (;; dst++, src++) {
    *dst = *src;
    if (!*src) break;
  }
  return d;
}

int strcmp(const char *s1, const char *s2)
{
  while (*s1 || *s2) {
    if (*s1 != *s2)
      return (*s1 > *s2) ? 1 : -1;
    s1++;
    s2++;
  }
  return 0;
}

int strncmp(const char *s1, const char *s2, int len)
{
  while ((*s1 || *s2) && (len > 0)) {
    if (*s1 != *s2)
      return (*s1 > *s2) ? 1 : -1;
    s1++;
    s2++;
    len--;
  }
  return 0;
}

char *strchr(char *s, int c)
{
  unsigned char *p = s;
  while (*p) {
    if (*p == c)
      return p;
    p++;
  }
  return NULL;
}

/* １文字送信 */
int putc(unsigned char c)
{
  if (c == '\n')
    serial_send_byte(SERIAL_DEFAULT_DEVICE, '\r');
  return serial_send_byte(SERIAL_DEFAULT_DEVICE, c);
}

/* １文字受信 */
unsigned char getc(void)
{
  unsigned char c = serial_recv_byte(SERIAL_DEFAULT_DEVICE);
  c = (c == '\r') ? '\n' : c;
  putc(c); /* エコー・バック */
  return c;
}

/* 文字列送信 */
int puts(unsigned char *str)
{
  while (*str)
    putc(*(str++));
  return 0;
}

/* 文字列受信 */
int gets(unsigned char *buf)
{
  int i = 0;
  unsigned char c;
  do {
    c = getc();
    if (c == '\n')
      c = '\0';
    buf[i++] = c;
  } while (c);
  return i - 1;
}

/* 数値の16進表示 */
int putxval(unsigned long value, int column)
{
  char buf[9];
  char *p;

  p = buf + sizeof(buf) - 1;
  *(p--) = '\0';

  if (!value && !column)
    column++;

  while (value || column) {
    *(p--) = "0123456789abcdef"[value & 0xf];
    value >>= 4;
    if (column) column--;
  }

  puts(p + 1);

  return 0;
}

/* 数値の10進表示 */
int putval(int value)
{
  int i,j;
  int p;
  int v = value;
  int cnt = 0;
  do{
    v /= 10;
    cnt++;
  }while(v!=0);

  for (i = 0; i < cnt; i++) {
    int p = 1;
    for (j = 0; j < cnt-i-1; j++) {
      p *= 10;
    }
    putc('0'+value/p%10);
  }

  return 0;
}

uint16 ntoh2(uint16 n)
{
  uint8 *p = (uint8 *)&n;
  return (p[0] << 8) | p[1];
}

uint16 hton2(uint16 h)
{
  uint16 n;
  uint8 *p = (uint8 *)&n;
  p[0] = (h >> 8) & 0xff;
  p[1] = h & 0xff;
  return n;
}

uint32 ntoh4(uint32 n)
{
  uint32 h;
  uint8 *p = (uint8 *)&n;
  h  = (uint32)p[0] << 24;
  h |= (uint32)p[1] << 16;
  h |= (uint32)p[2] <<  8;
  h |= (uint32)p[3];
  return h;
}

uint32 hton4(uint32 h)
{
  uint32 n;
  uint8 *p = (uint8 *)&n;
  p[0] = (h >> 24) & 0xff;
  p[1] = (h >> 16) & 0xff;
  p[2] = (h >>  8) & 0xff;
  p[3] = h & 0xff;
  return n;
}
