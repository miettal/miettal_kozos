#ifndef _TIMER_H_INCLUDED_
#define _TIMER_H_INCLUDED_

#define TIMER_START_FLAG_CYCLE (1<<0)

int timer_start(int index, int msec, int flags); /* �����޳��� */
int timer_is_expired(int index);                 /* ��������λ�������� */
int timer_expire(int index);                     /* ��������λ���� */
int timer_cancel(int index);                     /* �����ޥ���󥻥� */
int timer_is_running(int index);                 /* ������ư���椫�� */
int timer_gettime(int index);                    /* �����ޤθ����� */

#endif
