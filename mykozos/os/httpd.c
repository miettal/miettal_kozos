#include "defines.h"
#include "kozos.h"
#include "netdrv.h"
#include "tcp.h"
#include "lib.h"

static char header[] =
"HTTP/1.0 200 OK\r\n"
/*"Date: Sat, 23 Oct 2010 12:00:00 GMT\r\n"*/
"Server: KOZOS-httpd/1.0\r\n"
"Content-Type: text/html\r\n"
"Content-Length: #####\r\n"
"\r\n";

static char top_document[] =
"<html>\n"
"<head>\n"
"<title>This is KOZOS!</title>\n"
"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=EUC-JP\">\n"
"</head>\n"
"<body bgcolor=\"yellow\" text=\"blue\" link=\"red\" vlink=\"black\">\n"
"<center>\n"
"<h1>HTTP server on KOZOS (#####)</h1>\n"
"<p><a href=\"about.html\">KOZOS�Ȥϡ�</a>\n"
"<p><a href=\"kozos.html\">KOZOS�θ���</a>\n"
"<p><a href=\"makeos.html\">�ȹ��ߣϣӤ��äƤߤޤ��󤫡�</a>\n"
"</center>\n"
"</body>\n"
"</html>\n";

static char about_document[] =
"<html>\n"
"<head>\n"
"<title>KOZOS�Ȥϡ�</title>\n"
"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=EUC-JP\">\n"
"</head>\n"
"<body bgcolor=\"brown\" text=\"white\" link=\"yellow\" vlink=\"black\">\n"
"<center><h1>KOZOS�Ȥϡ�(#####)</h1></center>\n"
"<p>KOZOS�Ϻ�湰μ���������Ƥ��롤�ؽ��������ȹ���OS�Ǥ�����˽����ŻҤ�H8/3069F�ޥ�����ܡ��ɤ�ư����ȹ���OS�μ��ư��ʤɤγؽ����Ǥ��ޤ���\n"
"<p>�ʲ������ǡ��ĿͤǤγؽ��Ѥ˸����Ƥ��ޤ���\n"
"<ul>\n"
"<li>������ȹ���OS�������١��������������̤�1800�����٤Ⱦ��ʤ������ؼԤǤ��ɲ��ǽ��\n"
"<li>ư���ޥ�����ܡ��ɤ�4000�����٤Ȱ²��ʤΤǡ��ĿͤǤ������ǽ���ޤ��½��ʤɤ����Ѥ�����ˤ⡤���̤ҤȤ�ˤҤȤĶ���������Ʋ�ǽ�ʲ��ʡ�\n"
"<li>�Ȥ�Ω�ƺѤߥܡ��ɤΤ���Ⱦ���դ��ʤɤ����פǡ���ä�����������γؽ�����ǽ��\n"
"<li>���ꥢ���ͳ�ǥե�å���ROM�񤭴�����ǽ�ʤ��ᡤICE��ROM�饤���ʤɤ����ס�\n"
"<li>CPU(H8)��ܡ��ɤ��������Τ������ܸ�λ�����˭�١�\n"
"<li>�֡��ȥ�������ޤ᤿�ե륹����å�OS�Ǥ��뤿�������κǽ餫��γؽ�����ǽ��\n"
"<li>��ȯ�Ķ���GNU�Ķ��Ǥ��뤿�������CPU�Υ���ѥ���˰�¸���ʤ��Ĥ֤��θ����ؽ�����ǽ��\n"
"<li>���ҡ�12���ƥåפǺ�� �ȹ��ߣϣӼ�������פ˾ܺ٤ʲ��⤢�ꡥ\n"
"<li>�����ץ󥽡������饤���󥹤Τ���������¤����ͳ��\n"
"</ul>\n"
"<p>�ȹ���OS�����Ѥ�������½��ʤɹԤ��Ȥ������⡤�ȹ���OS��������¤��ư������Τ�Τ����򤹤롤��ʬ�ǲ�¤��ä�������򿼤��Ȥ��ä��ؽ��˸����Ƥ��ޤ����ּ���Ū�˻Ȥ������OS�פǤʤ���������¤�����򤷡���¤�����굡ǽ�ɲä����ꤷ�Ƥ����ä�ͷ�֤����OS�פȤ����򤯤�������\n"
"<hr>\n"
"<ul>\n"
"<li><a href=\"index.html\">�ȥåץڡ��������</a>\n"
"</ul>\n"
"<address>\n"
"�᡼���<a href=\"mailto:hsakai@saturn.dti.ne.jp\">hsakai@saturn.dti.ne.jp</a>�ޤ�\n"
"</address>\n"
"</body>\n"
"</html>\n";

static char kozos_document[] =
"<html>\n"
"<head>\n"
"<title>KOZOS�θ���</title>\n"
"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=EUC-JP\">\n"
"</head>\n"
"<body bgcolor=\"brown\" text=\"white\" link=\"yellow\" vlink=\"black\">\n"
"<center><h1>KOZOS�θ���(#####)</h1></center>\n"
"<p>KOZOS��KOZOS�ץ������Ȥˤ�곫ȯ���ʤ���Ƥ��ޤ���(�ޤ�����ͤǤ�äƤ���ΤǤ���)\n"
"<p>�ʲ������Ǥ˼¸�����Ƥ��ޤ���\n"
"<ul>\n"
"<li>�ϣӤȤ��Ƥκ���¤ε�ǽ����ĥߥ˥ޥ�ʥ����ͥ�\n"
"<li>�ȼ��Υ֡��ȥ�����\n"
"<li>�ޥ��������ư����������I/O����(����ߴ���)�����������̿�\n"
"<li>H8��SH2��PowerPC�ʤɤǤ�ư��\n"
"<li>���ꥢ���ͳ�ǤΥ��ޥ�ɱ���\n"
"<li>DRAM�б�\n"
"<li>�����ޥ����ӥ�\n"
"<li>ethernet��TCP/IP�ˤ���̿���ǽ\n"
"<li>�ʰ�web������\n"
"<li>���ߥ�졼����Ǥ�ư��\n"
"<li>���ҡ�12���ƥåפǺ�� �ȹ��ߣϣӼ�������פν���\n"
"</ul>\n"
"<p>�ʲ��Ϻ���ʤ�Ƥ���ͽ��κ�Ȱ����Ǥ���\n"
"<ul>\n"
"<li>�ǥХå��б�\n"
"<li>�ե�����ž����ǽ\n"
"<li>���ץꥱ��������ưŪ�¹�\n"
"<li>���ޥե�����\n"
"<li>ROM��\n"
"<li>�ꥢ�륿���ಽ\n"
"<li>�����ͥ������˥ǥХå���ǽ�����\n"
"<li>SH2�ؤ����б�\n"
"<li>��12���ƥåפǺ�� �ȹ��ߣϣӼ�������פ�³�Ԥ����\n"
"</ul>\n"
"<p>��ꤿ�����ȤϤ��äѤ�����\n"
"<hr>\n"
"<ul>\n"
"<li><a href=\"index.html\">�ȥåץڡ��������</a>\n"
"</ul>\n"
"<address>\n"
"�᡼���<a href=\"mailto:hsakai@saturn.dti.ne.jp\">hsakai@saturn.dti.ne.jp</a>�ޤ�\n"
"</address>\n"
"</body>\n"
"</html>\n";

static char makeos_document[] =
"<html>\n"
"<head>\n"
"<title>�ȹ���OS���äƤߤޤ��󤫡�</title>\n"
"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=EUC-JP\">\n"
"</head>\n"
"<body bgcolor=\"brown\" text=\"white\" link=\"yellow\" vlink=\"black\">\n"
"<center><h1>�ȹ���OS���äƤߤޤ��󤫡�(#####)</h1></center>\n"
"<p>���ȹ��ߡס��ȹ��ߥ����ƥ�ס֥���٥ǥåɡפȤ������դ����̤Ǥ�褯ʹ�����褦�ˤʤäƤ��ޤ������ȹ���ʬ��˶�̣�Τ���ͤ����߿��Ϥʤ��ʤ�¿���Ȼפ��ΤǤ����������Ʊ���ˡ֤ȤäĤ��ˤ����ס֤褯�狼��ʤ��פȤ����ä����ۤ��ʹ�����Ȥ�����ޤ����ʤ��֤ȤäĤ��ˤ����פΤǤ��礦����\n"
"<p>�ȹ��ߥ����ƥ���ٶ��򤷤����Ȼפä��Ȥ��������Ф��ü���ᤤ�Τ�Ŭ���ʥܡ��ɥ���ԥ塼�������ꤷ�ơ�Linux�ʤ�ITRON�ʤ�Ȥ��ä�OS�򥤥󥹥ȡ��뤷�ơ���ʬ�Ǥ����äƤߤ뤳�ȤǤ�������ˤ���OS���ɤ������ư������򤷤��ꡤOS�˼������Ʋ�¤�����ꤷ�Ƥ�����ХХå���Ǥ��礦��\n"
"<p>������Linux�Υ������������̤�OS�鿴�Ԥ����Ϥ��ɤ߲򤤤Ƥ����Τϡ������ʺ�����ɬ�פǤ��礦��ư�����ˤ����ɤ�Ȥ��Ƥ⡤���줬�ɤ��ʤΤ���Ƚ�̤�������ǰ��ϫ�Ǥ��������ITRON�μ����Ǥ���TOPPERS��HOS�ˤĤ��Ƥ�Ʊ�ͤǤ��礦�������¿CPU�б���¿��ǽ���Τ���˳��ز���������٤�⤯���Ƥ��ä���̤ʤΤǼ���OS�Ȥ��Ƥ��������Ѥ��Ȼפ��ΤǤ��������Τ���˴ؿ��ƤӽФ���ޥ�������ʤɤγ��ؤ������ɤߤˤ����ʤäƤ��뤳�Ȥϻ��¤ǡ�OS�鿴�Ԥ����Ϥǥѥä��ɤ߲򤱤�褦�ʤ�ΤǤϤʤ��ʤäƤ��ޤ���\n"
"<p>�ǤϤ�äȼ�ڤ�OS��١����ˤ��Ƴؽ�����Ф����ΤǤ��礦����������ˤϥ����ץ󥽡����Ǽ�ڤ��ȹ���OS��¿������ޤ��������������Ǥ⤦�ҤȤ�����ˤʤ�Τϡ�����OS��ư���ܡ��ɤǤ����������ȼ�OS��¿����CPU��ɾ���ܡ��ɤʤɤǤ�ư�������ˤ��Ƥ��ޤ���ɾ���ܡ��ɤ�OS�ܿ��Ȥ����������Ǹ���Ⱥ�Ŭ�����ʤΤǤ������Ŀ����ѤȤ������Ǹ���ȡ���̿Ū�ʷ�٤�����ޤ�������Ϥʤ�Ȥ��äƤ�֤��⤤�פΤǤ��������ߡ��������ߤ���Τ�����Ǥ���������Ū���̳�Τ���˸��漼���Ҥǹ�������ʤ�С��ɤ��Ȥ������ȤϤʤ��ΤǤ��礦��������OS�ٶ��Τ���˸Ŀͤǹ�������ˤϡ�����äȵ��ڤˤ��㤨�ʤ���ۤǤ���\n"
"<p>�ȹ���OS�ν��Ҥ��͡�����ޤ������鿴�Ԥ��Ŀͥ�٥�ǳؽ��Ǥ���Ȥ������Ȥ򿿷��˹ͤ��ƽ񤫤줿��Τ������˾��ʤ��פ��ޤ����оݤȤ���OS��ͭ���ǿ����ߤ⤷���ꡤͭ���γ�ȯ�Ķ������Ѥ��Ƥ����ꡤ������ü�ʥܡ��ɤ��оݤˤ��Ƥ����Τ⤢��ޤ�������Ǥϸ��¤Ȥ��ơ��Ŀͥ�٥�ǻ�ޤ��󡥤ۤ�Ȥ��˺��٤Ȥ��������Ǥ������Τ����꤬���ȹ��ߤ��񤷤��פȻפ��Ƥ�����ͳ�Τ褦�˻פ��ޤ����ޤ������ν��Ҥ�OS��ֻȤ��פ���Τ�ΤǤ��äơ�OS��ֺ��פ�����������ʤ���Ƥ����ΤϤۤȤ�ɤ���ޤ���\n"
"<p>�������⹻���Ǥ⤪���������ϰϤ��㤨��褦�ʶ�ۤΥܡ��ɤ�ư��ơ��鿴�ԤǤ⽽ʬ���ɲ�Ǥ��Ƶ��ڤ˲�¤�Ǥ���褦��ñ��ʥ����������ɹ����ǡ��ӥ�ɤ��ü�ʥ�����ץȼ¹Ԥʤɤ�ɬ�פȤ�������ȯ�Ķ���̵��������Ǥ��ơ�����δĶ��ˤ��Ф��ʤ�����Ĥ֤��������ؽ����Ǥ��ơ������ץ󥽡����Ǥ��뤳�ȡ��Ŀͤγؽ��Ѥˤϡ����Τ褦�ʾ�郎ɬ�ܤǤ������������Τ褦��OS����γؽ��ѤΥ���ץ�ˤʤ��ȹ���OS��̵���ΤǤ����Ƥ��������伫�Ȥ�����������Τ��������ߤ����ä��ΤǤ���\n"
"<p>����OS�Ȥ����߷פ��줿OS�Ǥϡ������ξ����������������Ȥ��Բ�ǽ�Ǥ����ʤ��ʤ�С��߷׻��ۤ���ᤫ��ۤʤäƤ��뤫��Ǥ�����ᤫ��ؽ���Ū�Ȥ����߷פ��줿�ȹ���OS��ɬ�פʤΤǤ���\n"
"<p>KOZOS�Ϥ��Τ褦�ʹͤ��Ǻ���Ƥ��ޤ���PC��������С����Ȥ�5000�ߤ��餤�ǤҤȤȤ���������Ϥ���äơ��鿴�ԤǤ��ɤߤ���륳�����̡�ï�Ǥ��ñ�˲�¤�Ǥ��ơ��ѥäȤ��������Ȥ��Ǥ��롥����ʥݥꥷ���Ǻ���Ƥ��ޤ��������󥪡��ץ󥽡������ե꡼���եȤǤ���\n"
"<p>H8�Ѥ�KOZOS�Ǥϥ֡��ȥ��������������������١�OS�����Τ��������������٤Ǥ�������ʤ�С���ʬ�Ǥ��ɤߤ����褦�ʵ������Ƥ��ޤ��󤫡� ����äȴ�ĥ��м�ʬ�ˤ����ΤǤϤʤ������Ȥ������ˤʤ�ΤǤϤʤ��Ǥ��礦����������ܡ��ɤ�4000�߼塤�����ʥܡ��ɤʤΤ�Ⱦ���դ����餺�����ꥢ���ͳ�ǥե�å���ROM�ν񤭴�����ǽ�ʤΤ�ROM�饤�����פ���ROM�����Ƥ�����Ƥ�¿������Τ�ï�Ǥ������ǽ�Ǥ�����ȯ�Ķ���FreeBSD��GNU/Linux��Cygwin�Τ����줫��GNU��ȯ�Ķ������ꤷ�Ƥ���ΤǱ����ϰϤ⹭�����塹�ˤĤ֤��������ؽ����Ǥ��ޤ����񻺥ܡ��ɡ���CPU�ʤΤ����ܸ�����⽼�¡��ۤ�Ȥ�ï�Ǥ��ޤ���\n"
"<p>���ε�����ȹ���OS���ٶ����Ƥߤ�ΤϤɤ��Ǥ��礦��������ʤ�ǡ�OS�򼫺�Ƥߤ�ΤϤɤ��Ǥ��礦����OS������ٶ��ˤʤ뤷���ȤƤ����򤤤Ǥ��衪\n"
"<hr>\n"
"<ul>\n"
"<li><a href=\"index.html\">�ȥåץڡ��������</a>\n"
"</ul>\n"
"<address>\n"
"�᡼���<a href=\"mailto:hsakai@saturn.dti.ne.jp\">hsakai@saturn.dti.ne.jp</a>�ޤ�\n"
"</address>\n"
"</body>\n"
"</html>\n";

static char unknown_document[] =
"<html>\n"
"<head>\n"
"<title>Unknown Request</title>\n"
"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=EUC-JP\">\n"
"</head>\n"
"<body bgcolor=\"yellow\" text=\"blue\" link=\"red\" vlink=\"black\">\n"
"<center>\n"
"<h1>Unknown Request (#####)</h1>\n"
"<p><a href=\"index.html\">�ȥåץڡ���</a>\n"
"</center>\n"
"</body>\n"
"</html>\n";

static struct documents {
  char *counterp;
  char *filename;
  char *document;
} documents[] = {
  { NULL, "/index.html", top_document },
  { NULL, "/about.html", about_document },
  { NULL, "/kozos.html", kozos_document },
  { NULL, "/makeos.html", makeos_document },
  { NULL, NULL, unknown_document }
};

static void send_accept()
{
  struct netbuf *buf;
  buf = kz_kmalloc(sizeof(*buf));
  buf->cmd = TCP_CMD_ACCEPT;
  buf->option.tcp.accept.id = MSGBOX_ID_HTTPD;
  buf->option.tcp.accept.port = 80;
  kz_send(MSGBOX_ID_TCPPROC, 0, (char *)buf);
}

static void send_close(int number)
{
  struct netbuf *buf;
  buf = kz_kmalloc(sizeof(*buf));
  buf->cmd = TCP_CMD_CLOSE;
  buf->option.tcp.close.number = number;
  kz_send(MSGBOX_ID_TCPPROC, 0, (char *)buf);
}

static void send_write(int number, int size, char *data)
{
  struct netbuf *buf;
  buf = kz_kmalloc(DEFAULT_NETBUF_SIZE);
  memset(buf, 0, DEFAULT_NETBUF_SIZE);
  buf->cmd = TCP_CMD_SEND;
  buf->top = buf->data;
  buf->size = size;
  buf->option.tcp.send.number = number;
  memcpy(buf->top, data, size);
  kz_send(MSGBOX_ID_TCPPROC, 0, (char *)buf);
}

static void send_string(int number, char *str)
{
  int len, size;
  len = strlen(str);
  while (len > 0) {
    size = (len > 512) ? 512 : len;
    send_write(number, size, str);
    len -= size;
    str += size;
  }
}

static char *val2str(int value)
{
  static char str[11];
  int i;
  char *top;

  memset(str, '0', sizeof(str));
  str[10] = '\0';
  top = &str[9];

  for (; value > 0; value--) {
    str[9]++;
    for (i = 10 - 1; i >= 0; i--) {
      if ((str[i] > '9') && (i > 0)) {
        str[i] = '0';
        str[i - 1]++;
      }
      if ((&str[i] < top) && (str[i] > '0'))
        top = &str[i];
    }
  }

  return top;
}

static int parse(int number, char *str)
{
  static int count = 0;
  static char *length_p = NULL;
  char *filename, *p;
  struct documents *docs;

  if (strncmp(str, "GET", 3))
    return 0;

  for (p = str + 3; *p == ' '; p++)
    ;
  filename = p;
  p = strchr(p, ' ');
  *p = '\0';

  if (!strcmp(filename, "/"))
    filename = "/index.html";

  for (docs = documents; docs->filename; docs++)
    if (!strcmp(filename, docs->filename))
      break;

  if (!docs->counterp) docs->counterp = strchr(docs->document, '#');
  memset(docs->counterp, ' ', 5);
  p = val2str(count);
  memcpy(docs->counterp, p, strlen(p));
  count++;

  if (!length_p) length_p = strchr(header, '#');
  memset(length_p, ' ', 5);
  p = val2str(strlen(docs->document));
  memcpy(length_p, p, strlen(p));

  send_string(number, header);
  send_string(number, docs->document);
  return 1;
}

int httpd_main(int argc, char *argv[])
{
  char *p = NULL, *r;
  char *buffer;
  int number = 0, ret;
  struct netbuf *buf;

  send_accept();

  buffer = kz_kmalloc(DEFAULT_NETBUF_SIZE);

  while (1) {
    kz_recv(MSGBOX_ID_HTTPD, NULL, (void *)&buf);

    switch (buf->cmd) {
    case TCP_CMD_ESTAB:
      number = buf->option.tcp.establish.number;
      p = buffer;
      break;

    case TCP_CMD_CLOSE:
      number = 0;
      send_accept();
      break;

    case TCP_CMD_RECV:
      memcpy(p, buf->top, buf->size);
      p += buf->size;
      *p = '\0';

      r = strchr(buffer, '\n');
      if (r) {
	*r = '\0';
	r++;
	ret = parse(number, buffer);
	memmove(buffer, r, p - r + 1);
	p -= (r - buffer);
	if (ret) send_close(number);
      }
      break;
    }

    kz_kmfree(buf);
  }

  return 0;
}
