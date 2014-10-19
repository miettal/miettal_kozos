#include "defines.h"
#include "kozos.h"
#include "intr.h"
#include "interrupt.h"
#include "timer.h"
#include "lib.h"
#include "timerdrv.h"

struct timerbuf {
  struct timerbuf *next;
  kz_msgbox_id_t id; /* ��������λ���Υ�å����������� */
  int msec;
};

static struct timerreg {
  struct timerbuf *timers; /* �����ޡ��Хåե��Υ�󥯥ꥹ�� */
  int index; /* ���Ѥ��륿���ޤ��ֹ� */
} timerreg;

/*
 * �ʲ��ϳ���ߥϥ�ɥ�Ǥ��ꡤ��Ʊ���ǸƤФ��Τǡ��饤�֥��ؿ��ʤɤ�
 * �ƤӽФ����ˤ���դ�ɬ�ס�
 * ���ܤȤ��ơ��ʲ��Τ����줫�����ƤϤޤ�ؿ������ƤӽФ��ƤϤ����ʤ���
 * ��������ǽ�Ǥ��롥
 * ������åɤ���ƤФ�뤳�Ȥ�̵���ؿ��Ǥ��롥
 * ������åɤ���ƤФ�뤳�Ȥ����뤬������߶ػߤǸƤӽФ��Ƥ��롥
 * �ޤ��󥳥�ƥ����Ⱦ��֤ǸƤФ�뤿�ᡤ�����ƥࡦ����������Ѥ��ƤϤ����ʤ���
 * (�����ӥ�������������Ѥ��뤳��)
 */
static void timerdrv_intr(void)
{
  struct timerreg *tim = &timerreg;

  if (timer_is_expired(tim->index)) { /* �����޳���� */
    timer_cancel(tim->index);
    kx_send(MSGBOX_ID_TIMDRIVE, TIMERDRV_CMD_EXPIRE, NULL);
  }
}

static int timerdrv_init(void)
{
  memset(&timerreg, 0, sizeof(timerreg));
  timerreg.index = TIMER_DEFAULT_DEVICE;
  return 0;
}

/* ����åɤ�����׵��������� */
static int timerdrv_command(struct timerreg *tim, int cmd, char *p)
{
  struct timerbuf *tmbuf;
  struct timerbuf **tmbufp;
  struct timerreq *req;
  int t, msec;

  switch (cmd) {
  case TIMERDRV_CMD_EXPIRE: /* ��������λ */
    tmbuf = tim->timers;
    if (tmbuf) {
      tim->timers = tmbuf->next;
      kz_send(tmbuf->id, 0, NULL);
      kz_kmfree(tmbuf);
      if (tim->timers)
	timer_start(tim->index, tim->timers->msec, 0);
    }
    break;

  case TIMERDRV_CMD_START: /* �����ޤΥ������� */
    req = (struct timerreq *)p;

    tmbuf = kz_kmalloc(sizeof(*tmbuf));
    tmbuf->next = NULL;
    tmbuf->id   = req->id;
    tmbuf->msec = req->msec;

    t = 0;
    if (tim->timers) {
      t = timer_gettime(tim->index);
    }

    for (tmbufp = &tim->timers;; tmbufp = &(*tmbufp)->next) {
      if (*tmbufp == NULL) {
	*tmbufp = tmbuf;
	if (tmbufp == &tim->timers)
	  timer_start(tim->index, tim->timers->msec, 0);
	break;
      }
      msec = (*tmbufp)->msec - t;
      if (msec < 0) msec = 0;
      if (tmbuf->msec < msec) {
	(*tmbufp)->msec = msec - tmbuf->msec;
	tmbuf->next = *tmbufp;
	*tmbufp = tmbuf;
	timer_start(tim->index, tim->timers->msec, 0);
	break;
      }
      t = 0;
      tmbuf->msec -= msec;
    }

    kz_kmfree(p);
    break;

  default:
    break;
  }

  return 0;
}

int timerdrv_main(int argc, char *argv[])
{
  int cmd;
  char *p;

  timerdrv_init();
  kz_setintr(SOFTVEC_TYPE_TIMINTR, timerdrv_intr); /* ����ߥϥ�ɥ����� */

  while (1) {
    kz_recv(MSGBOX_ID_TIMDRIVE, &cmd, &p);
    timerdrv_command(&timerreg, cmd, p);
  }

  return 0;
}
