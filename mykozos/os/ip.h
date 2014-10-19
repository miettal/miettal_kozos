#ifndef _IP_H_INCLUDED_
#define _IP_H_INCLUDED_

#define IP_CMD_REGPROTO 'p' /* �ץ�ȥ����ֹ����Ͽ */
#define IP_CMD_RECV     'r' /* IP�ѥ��åȤμ��� */
#define IP_CMD_SEND     's' /* IP�ѥ��åȤ����� */

uint16 ip_calc_checksum(int size, void *buf);

#endif
