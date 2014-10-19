#ifndef _NETDRV_H_INCLUDED_
#define _NETDRV_H_INCLUDED_

#define NETDRV_DEVICE_NUM 1
#define NETDRV_CMD_USE  'u' /* �������ͥåȡ��ɥ饤�Фλ��ѳ��� */
#define NETDRV_CMD_SEND 's' /* �������ͥåȤؤΥե졼����� */
#define NETDRV_CMD_RECVINTR 'r' /* ���������� */
#define NETDRV_CMD_SENDINTR 'i' /* ���������� */

#define MACADDR_SIZE 6
#define IPADDR_SIZE 4

#define ETHERNET_TYPE_IP  0x0800
#define ETHERNET_TYPE_ARP 0x0806

#define IP_PROTOCOL_ICMP  1
#define IP_PROTOCOL_TCP   6
#define IP_PROTOCOL_UDP  17

#define DEFAULT_NETBUF_SIZE 1800

/*
 * �ѥ��åȤȤ�MAC���ɥ쥹����Ȥ���������Ȥꤹ�뤿��ι�¤�Ρ�
 */
struct netbuf {
  unsigned char cmd;
  short size;
  struct netbuf *next;

  union {
    union {
      struct {
	uint8 addr[MACADDR_SIZE];
      } macaddr;
      struct {
	uint32 addr;
      } ipaddr;
    } common;
    union {
      struct {
	uint8 dst_macaddr[MACADDR_SIZE];
	uint16 type;
	uint32 dst_ipaddr;
      } send;
    } ethernet;
    union {
      struct {
	uint8 protocol;
	unsigned char cmd;
	kz_msgbox_id_t id;
      } regproto;
      struct {
	uint8 protocol;
	uint32 dst_addr;
      } send;
    } ip;
    union {
      struct {
	int number;
	uint32 ipaddr;
      } send;
    } icmp;
    union {
      struct {
	uint16 port;
	kz_msgbox_id_t id;
      } accept;
      struct {
	uint16 port;
	uint32 ipaddr;
	kz_msgbox_id_t id;
      } connect;
      struct {
	int number;
      } establish;
      struct {
	int number;
      } close;
      struct {
	int number;
      } send;
    } tcp;
    union {
      struct {
	uint16 src_port;
	uint16 dst_port;
	uint32 ipaddr;
	kz_msgbox_id_t id;
      } regport;
      struct {
	uint16 port;
      } relport;
      struct {
	uint16 port;
      } recv;
      struct {
	uint16 src_port;
	uint16 dst_port;
	uint32 ipaddr;
      } send;
    } udp;
    union {
      struct {
	uint32 ipaddr;
      } start;
    } tftp;
  } option;

  /*
   * union�Υ������������ܿ��Ǥ�ǡ�������Ƭ���μ¤ˣ��Х��ȥ��饤��
   * �����褦�ˡ������ǥݥ��󥿤�������롥
   */
  char *top; /* �ǡ�������Ƭ��ؤ��ݥ��� */

  char data[0];
};

#endif
