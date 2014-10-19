#include "defines.h"
#include "kozos.h"
#include "intr.h"
#include "interrupt.h"
#include "lib.h"
#include "rtl8019.h"
#include "ethernet.h"
#include "netdrv.h"

static unsigned char my_macaddr[MACADDR_SIZE];

static struct netbuf *send_buf_head;
static struct netbuf **send_buf_tail = &send_buf_head;

static void recv_packet()
{
  struct netbuf *pkt;

  while (1) {
    pkt = kz_kmalloc(DEFAULT_NETBUF_SIZE);
    memset(pkt, 0, DEFAULT_NETBUF_SIZE);
    pkt->cmd  = ETHERNET_CMD_RECV;

    /*
     * ethernet�ե졼�ब14�Х��Ȥǣ����ܿ��Ǥʤ��Τǡ�ethernet�ե졼��ʹߤ�
     * ���Х��ȥ��饤����Ȥ����褦�ˡ��ǡ�������Ƭ�򣲥Х��ȶ����롥
     */
    pkt->top  = pkt->data + 2;

    pkt->size = rtl8019_recv(0, pkt->top);
    if (pkt->size > 0) {
      kz_send(MSGBOX_ID_ETHPROC, 0, (char *)pkt);
    } else {
      kz_kmfree(pkt);
      break;
    }
  }
}

static void send_packet(struct netbuf *buf)
{
  /* ��ü���ɲ� */
  if (buf) {
    buf->next = NULL;
    *send_buf_tail = buf;
    send_buf_tail = &buf->next;
  }

  if (send_buf_head) {
    if (!buf || !rtl8019_intr_is_send_enable(0)) {
      /* ��Ƭ����ȴ���Ф� */
      buf = send_buf_head;
      send_buf_head = send_buf_head->next;
      if (!send_buf_head)
	send_buf_tail = &send_buf_head;
      buf->next = NULL;

      /*
       * ���������̵���ʤ�С��������Ϥ���Ƥ��ʤ��Τ��������Ϥ��롥
       * ���������ͭ���ʤ���������Ϥ���Ƥ��ꡤ��������ߤα�Ĺ��
       * �����Хåե���Υǡ������缡���������Τǡ����⤷�ʤ��Ƥ褤��
       */
      if (!rtl8019_intr_is_send_enable(0)) {
	rtl8019_intr_send_enable(0); /* ���������ͭ���� */
      }

      rtl8019_send(0, buf->size, buf->top);
      kz_kmfree(buf);
    }
  }
}

/* ����ߥϥ�ɥ� */
static void netdrv_intr(void)
{
  struct netbuf *pkt;

  rtl8019_intr_clear(0);

  if (rtl8019_is_recv_enable(0)) {
    rtl8019_intr_clear_recv(0);
    pkt = kx_kmalloc(DEFAULT_NETBUF_SIZE);
    memset(pkt, 0, DEFAULT_NETBUF_SIZE);
    pkt->cmd  = NETDRV_CMD_RECVINTR;
    kx_send(MSGBOX_ID_NETPROC, 0, (char *)pkt);
  }

  if (rtl8019_is_send_enable(0)) {
    rtl8019_intr_clear_send(0);
    pkt = kx_kmalloc(DEFAULT_NETBUF_SIZE);
    memset(pkt, 0, DEFAULT_NETBUF_SIZE);
    pkt->cmd  = NETDRV_CMD_SENDINTR;
    kx_send(MSGBOX_ID_NETPROC, 0, (char *)pkt);
  }
}

static int netdrv_init(void)
{
  rtl8019_init(0, my_macaddr);
  return 0;
}

/* ����åɤ�����׵��������� */
static int netdrv_proc(struct netbuf *buf)
{
  int ret = 0;

  switch (buf->cmd) {
  case NETDRV_CMD_USE: /* �������ͥåȡ��ɥ饤�Фλ��ѳ��� */
    /* rtl8019_init(0, my_macaddr); */ /* netdrv_init()�ǹԤäƤ���Τ����� */
    rtl8019_intr_recv_enable(0); /* ���������ͭ����(��������) */
    buf = kz_kmalloc(sizeof(*buf));
    buf->cmd = ETHERNET_CMD_MACADDR;
    memcpy(buf->option.common.macaddr.addr, my_macaddr, MACADDR_SIZE);
    kz_send(MSGBOX_ID_ETHPROC, 0, (char *)buf);
    break;

  case NETDRV_CMD_SEND: /* �������ͥåȤؤΥե졼����� */
    /* ����ػߤˤ���ɬ�פ�̵�� */
    send_packet(buf);
    ret = 1;
    break;

  case NETDRV_CMD_RECVINTR: /* �������ͥåȤ���Υե졼����� */
    /* ����ػߤˤ���ɬ�פ�̵�� */
    recv_packet();
    break;

  case NETDRV_CMD_SENDINTR: /* �������ͥåȤؤΥե졼����ϴ�λ */
    /* ����ػߤˤ���ɬ�פ�̵�� */
    if (!send_buf_head) {
      /* �����ǡ�����̵���ʤ�С�����������λ */
      rtl8019_intr_send_disable(0);
    } else {
      /* �����ǡ���������ʤ�С���³���������� */
      send_packet(NULL);
    }
    break;

  default:
    break;
  }

  return ret;
}

int netdrv_main(int argc, char *argv[])
{
  struct netbuf *buf;
  int ret;

  netdrv_init();
  kz_setintr(SOFTVEC_TYPE_ETHINTR, netdrv_intr); /* ����ߥϥ�ɥ����� */

  while (1) {
    kz_recv(MSGBOX_ID_NETPROC, NULL, (void *)&buf);
    ret = netdrv_proc(buf);
    if (!ret) kz_kmfree(buf);
  }

  return 0;
}
