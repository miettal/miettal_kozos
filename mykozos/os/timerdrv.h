#ifndef _TIMERDRV_H_INCLUDED_
#define _TIMERDRV_H_INCLUDED_

#define TIMERDRV_DEVICE_NUM 1
#define TIMERDRV_CMD_EXPIRE 'e' /* ��������λ */
#define TIMERDRV_CMD_START  's' /* �����ޤΥ������� */

struct timerreq {
  kz_msgbox_id_t id; /* ��������λ���Υ�å����������� */
  int msec;
};

#endif
