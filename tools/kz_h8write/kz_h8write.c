/**
 * @file kz_h8write.c
 * @author Shinichiro Nakamura
 * @brief H8/3069F write for KOZOSの実装。
 */

/*
 * ===============================================================
 *  KOZOS H8/3069F Flash Writer.
 * ===============================================================
 * Copyright (c) 2010-2012 Shinichiro Nakamura
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 * ===============================================================
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "serial.h"
#include "mot.h"
#include "optparse.h"
#include "kz_h8write.h"

/**
 * @brief プログラム終了用マクロ。
 * @details このマクロはシリアルポートのオープン成功後の処理に用いる。
 * @param CODE シェルに返すコード番号。
 */
#define PROGEXIT(CODE) serial_close(serial); exit(CODE)

/**
 * @brief シリアルタイムアウト時間。
 * @details この定義はシリアルポートからの読み出しで使用される。
 */
#define SERIAL_TIMEOUT_MS 500

/**
 * @brief 切り上げ処理。
 * @details 数値AをBで切り上げる。
 */
#define ROUND_OUT(A,B) ((((A)+((B)-1))/(B))*(B))

/**
 * @brief ユーザオプション構造体。
 */
typedef struct {
    char cpu_name[BUFSIZ];      /**< CPU名。 */
    int cpu_freq;               /**< CPU周波数。 */
    char mot_file[BUFSIZ];      /**< MOTファイル名。NULLなら標準入力。 */
    char serial_port[BUFSIZ];   /**< シリアルポート名。 */
    int version;                /**< バージョンフラグ。 */
    int help;                   /**< ヘルプフラグ。 */
    int debug;                  /**< デバッグフラグ。 */
    int error;                  /**< オプション指定エラー検出フラグ。 */
} user_option_t;

#define USER_OPTION_CPU_NAME(P)     ((P)->cpu_name)
#define USER_OPTION_CPU_FREQ(P)     ((P)->cpu_freq)
#define USER_OPTION_MOT_FILE(P)     ((P)->mot_file)
#define USER_OPTION_SERIAL_PORT(P)  ((P)->serial_port)
#define USER_OPTION_VERSION(P)      ((P)->version)
#define USER_OPTION_HELP(P)         ((P)->help)
#define USER_OPTION_DEBUG(P)        ((P)->debug)
#define USER_OPTION_ERROR(P)        ((P)->error)

static unsigned char memory_image[0x100000 + 0x100];
static unsigned int memory_lastaddr = 0;

int com_getc(SERIAL *serial);
int com_putc(SERIAL *serial, unsigned char c);
int write_request(
        SERIAL *serial,
        const unsigned char cmd,
        const unsigned char *buf,
        const unsigned char siz);
int read_response(
        SERIAL *serial,
        unsigned char *buf,
        size_t siz);
enum ErrorCode bitrate_sequence(SERIAL *serial);
enum ErrorCode inquiry_device(SERIAL *serial, uint32_t *devcode);
enum ErrorCode select_device(SERIAL *serial, const uint32_t devcode);
enum ErrorCode inquiry_clockmode(SERIAL *serial);
enum ErrorCode select_clockmode(SERIAL *serial);
enum ErrorCode select_bitrate(SERIAL *serial, const int baudrate, const int clock_mhz);
enum ErrorCode program(SERIAL *serial);

/**
 * @brief motモジュールのコールバック関数。
 * @details motモジュールはファイルを読み込みながらこの関数を呼ぶ。
 * @param addr データ先頭アドレス。
 * @param buf バッファへのポインタ。
 * @param siz バッファに格納されたデータバイトサイズ。
 */
void mot_callback(
        const unsigned int addr,
        const unsigned char *buf,
        const int siz)
{
    unsigned int lastaddr = addr + siz;
    for (int i = 0; i < siz; i++) {
        memory_image[addr + i] = buf[i];
    }
    if (memory_lastaddr < lastaddr) {
        memory_lastaddr = lastaddr;
    }
}

/**
 * @brief シリアルポートから１文字読み出す。
 * @details このインターフェースは最大SERIAL_TIMEOUT_MS[ms]の着信を待つ。
 *
 * @param serial シリアルハンドラ。
 *
 * @return 文字。
 */
int com_getc(SERIAL *serial)
{
    unsigned char c;
    if (serial_read_with_timeout(serial, &c, 1, SERIAL_TIMEOUT_MS) != 0) {
        return EOF;
    }
    return c;
}

/**
 * @brief シリアルポートへ１文字書き出す。
 *
 * @param serial シリアルハンドラ。
 * @param c 文字。
 *
 * @return 書き出した文字。
 */
int com_putc(SERIAL *serial, unsigned char c)
{
    if (serial_write(serial, &c, 1) != 0) {
        return EOF;
    }
    return (int)c;
}

/**
 * @brief リクエストを書き込む。
 *
 * @param serial シリアルハンドラ。
 * @param cmd コマンド。
 * @param buf バッファへのポインタ。
 * @param siz バッファに格納されたデータバイト数。
 *
 * @retval 0 成功。
 * @retval 0以外 失敗。エラー番号。
 */
int write_request(
        SERIAL *serial,
        const unsigned char cmd,
        const unsigned char *buf,
        const unsigned char siz)
{
    /*
     * コマンドとサイズを書き込む。
     */
    if (serial_write(serial, &cmd, 1) != 0) {
        return -1;
    }
    if (serial_write(serial, &siz, 1) != 0) {
        return -2;
    }
    /*
     * データを書き込む。
     */
    if (serial_write(serial, buf, siz) != 0) {
        return -3;
    }
    /*
     * チェックサムを計算して書き込む。
     */
    unsigned char chksum = 0;
    chksum += cmd;
    chksum += siz;
    for (int i = 0; i < siz; i++) {
        chksum += buf[i];
    }
    chksum = -chksum;
    if (serial_write(serial, &chksum, 1) != 0) {
        return -4;
    }
    return 0;
}

/**
 * @brief レスポンスを読み込む。
 * @details
 * プロセッサからは以下のフォーマットで情報が返る。
 * 1. コマンド。
 * 2. サイズ。(コマンド、サイズ、チェックサムを除く。)
 * 3. データ。
 * 4. チェックサム。
 *
 * @param serial シリアルハンドラ。
 * @param buf バッファ。
 * @param siz バッファサイズ。
 *
 * @retval 1以上 読み込んだ文字数。
 * @retval 0以下 エラー。
 */
int read_response(SERIAL *serial, unsigned char *buf, size_t siz)
{
    /*
     * コマンド。
     */
    if (serial_read_with_timeout(serial, &buf[0], 1, SERIAL_TIMEOUT_MS) != 0) {
        return -1;
    }

    /*
     * データサイズ。
     */
    if (serial_read_with_timeout(serial, &buf[1], 1, SERIAL_TIMEOUT_MS) != 0) {
        return -2;
    }
    const int datcnt = buf[1];

    /*
     * データ。
     */
    if (serial_read_with_timeout(serial, &buf[2], datcnt, SERIAL_TIMEOUT_MS) != 0) {
        return -3;
    }

    /*
     * チェックサム。
     */
    if (serial_read_with_timeout(serial, &buf[2 + datcnt], 1, SERIAL_TIMEOUT_MS) != 0) {
        return -4;
    }

    /*
     * チェックサム検証。
     */
    int chksum = 0;
    for (int i = 0; i < datcnt + 3; i++) {
        chksum += buf[i];
    }
    if ((chksum & 0xff) != 0x00) {
        return -5;
    }
    return 0;
}

/**
 * @brief ビットレート合わせ込みのシーケンスを実行する。
 *
 * @details
 * ビットレートの合わせ込みは以下のシーケンスで行う。
 * 1. 0x00を最大30回送信する。
 * 2. 合わせ込みが完了すると0x00が返る。
 * 3. この時点で0x55を送信する。
 * 4. レスポンスコードが返る。
 *    成功：0xE6
 *    失敗：0xFF
 *
 * @param serial シリアルハンドラ。
 *
 * @return エラーコード。
 */
enum ErrorCode bitrate_sequence(SERIAL *serial)
{
    unsigned char sc, rc;
    int i;
    for (i = 0; i < 30; i++) {
        /*
         * ビットレート計測用マーカを送信する。
         */
        sc = 0x00;
        if (serial_write(serial, &sc, 1) != 0) {
            return SerialWriteError;
        }
        /*
         * 何かを受信したらコードを確認する。
         * コードが正しければ応答を行い、そのレスポンスを検証する。
         */
        if (serial_read_with_timeout(serial, &rc, 1, SERIAL_TIMEOUT_MS) == 0) {
            if (rc != 0x00) {
                /*
                 * 最初の応答は0x00のはず。
                 */
                return InvalidPrimaryResponseCode;
            }
            sc = 0x55;
            if (serial_write(serial, &sc, 1) != 0) {
                return SerialWriteError;
            }
            if (serial_read_with_timeout(serial, &rc, 1, SERIAL_TIMEOUT_MS) != 0) {
                /*
                 * 0x55に対する応答があるはず。
                 */
                return NoSecondaryResponseCode;
            }
            if (rc == 0xe6) {
                return NoError;
            } else {
                /*
                 * ２回目の応答は0xe6のはず。
                 */
                return InvalidSecondaryResponseCode;
            }
        }
    }
    /*
     * レスポンスが全くない。
     */
    return NoPrimaryResponseCode;
}

/**
 * @brief サポートデバイス問い合わせ(0x20)を実行する。
 *
 * @param serial シリアルハンドラ。
 * @param devcode デバイスコード。
 *
 * @return エラーコード。
 */
enum ErrorCode inquiry_device(SERIAL *serial, uint32_t *devcode)
{
    unsigned char buf[BUFSIZ];
    com_putc(serial, 0x20);
    if (read_response(serial, &buf[0], sizeof(buf)) != 0) {
        /*
         * レスポンスが異常だった場合、
         * サポートしていないデバイスとして返す。
         */
        return UnsupportedDevice;
    }
    *devcode =
        (buf[4] << (8 * 3)) |
        (buf[5] << (8 * 2)) |
        (buf[6] << (8 * 1)) |
        (buf[7] << (8 * 0));
    return NoError;
}

/**
 * @brief デバイス選択(0x10)を実行する。
 *
 * @param serial シリアルハンドラ。
 * @param devcode デバイスコード。
 *
 * @return エラーコード。
 */
enum ErrorCode select_device(SERIAL *serial, const uint32_t devcode)
{
    unsigned char c;
    unsigned char buf[4];
    buf[0] = devcode >> (8 * 3);
    buf[1] = devcode >> (8 * 2);
    buf[2] = devcode >> (8 * 1);
    buf[3] = devcode >> (8 * 0);
    write_request(serial, 0x10, buf, sizeof(buf));
    c = com_getc(serial);
    if (c != 0x06) {
        /*
         * ACKでない場合、続けてエラーコードが返る。
         * この時、上位にはデバイスコードの不一致として返す。
         */
        c = com_getc(serial);
        return UnmatchedDeviceCode;
    }
    return NoError;
}

/**
 * @brief クロックモード問い合わせ(0x21)を実行する。
 * @param serial シリアルハンドラ。
 */
enum ErrorCode inquiry_clockmode(SERIAL *serial) {
    unsigned char buf[BUFSIZ];
    com_putc(serial, 0x21);
    if (read_response(serial, &buf[0], sizeof(buf)) != 0) {
        return UnsupportedClockMode;
    }
    return NoError;
}

/**
 * @brief クロックモード選択(0x11)を実行する。
 *
 * @param serial シリアルハンドラ。
 *
 * @return エラーコード。
 */
enum ErrorCode select_clockmode(SERIAL *serial) {
    unsigned char c;
    unsigned char buf[1];
    buf[0] = 0;
    write_request(serial, 0x11, buf, 1);
    c = com_getc(serial);
    if (c != 0x06) {
        /*
         * ACKでない場合、続けてエラーコードが返る。
         * この時、上位にはクロックモードの不一致として返す。
         */
        c = com_getc(serial);
        return UnmatchedClockMode;
    }
    return NoError;
}

/**
 * @brief 新ビットレート選択(0x3F)を実行する。
 *
 * @param serial シリアルハンドラ。
 * @param baudrate ボーレート。
 * @param clock_mhz クロック。単位はMHz。
 */
enum ErrorCode select_bitrate(
        SERIAL *serial,
        const int baudrate,
        const int clock_mhz)
{
    const int baudval = baudrate / 100;
    const int clkval = clock_mhz * 100;
    unsigned char buf[7];
    unsigned char c;
    buf[0] = baudval / 0x100;
    buf[1] = baudval % 0x100;
    buf[2] = clkval / 0x100;
    buf[3] = clkval % 0x100;
    buf[4] = 1;
    buf[5] = 1;
    buf[6] = 1;
    write_request(serial, 0x3f, buf, sizeof(buf));
    c = com_getc(serial);
    if (c != 0x06) {
        /*
         * ACKでない場合、続けてエラーコードが返る。
         * この時、上位には不正なビットレートとして返す。
         */
        c = com_getc(serial);
        return InvalidBitrate;
    }
    /*
     * 新ビットレートによる双方向ACK確認。
     */
    com_putc(serial, 0x06);
    c = com_getc(serial);
    if (c != 0x06) {
        return InvalidBitrate;
    }
    return NoError;
}

/**
 * @brief 書き込み消去ステータス遷移(0x40)を実行する。
 *
 * @param serial シリアルハンドラ。
 *
 * @return エラーコード。
 */
enum ErrorCode program(SERIAL *serial)
{
    unsigned char c;

    /*
     * 書き込み消去ステータス遷移を実行。
     */

    /*
     * H8/3069の場合、メモリ消去にかかる最大時間は
     * 40000[ms]とデータシートに記されている。
     * com_getcはSERIAL_TIMEOUT_MS時間のタイムアウトが
     * 設定されている。
     * ここでは最大で(40000 / SERIAL_TIMEOUT_MS)回の読み出し
     * を試行して、期待する応答が帰ってくれば消去に成功
     * したものとみなす。
     *
     * 従来の多くの書き込みソフトウェアはこの処理の多くが
     * 固定値だった。
     * もしかしたら、「このH8はもう書けなくなった。」と思う物でも、
     * 消去に成功してしまうかもしれない。
     */
    fprintf(stderr, "Waiting for erase done:");
    com_putc(serial, 0x40);
    const int POLL_N_ERASE = (40000 / SERIAL_TIMEOUT_MS);
    for (int i = 0; i < POLL_N_ERASE; i++) {
        c = com_getc(serial);
        if (c == 0x06) {
            break;
        }
        fprintf(stderr, ".");
    }
    fprintf(stderr, "\n");
    if (c != 0x06) {
        /*
         * ACKでない場合、続けてエラーコードが返る。
         * この時、上位には書き込み消去失敗として返す。
         */
        c = com_getc(serial);
        return WriteEraseFail;
    }

    /*
     * ユーザマット書き込み選択を実行。
     */
    com_putc(serial, 0x43);
    c = com_getc(serial);
    if (c != 0x06) {
        /*
         * ACKでない場合、続けてエラーコードが返る。
         */
        c = com_getc(serial);
        return WriteEraseFail;
    }

    /*
     * 128バイト書き込みを実行する。
     */
    fprintf(stderr, "Programming:");
    for (int i = 0; i < (int)ROUND_OUT(memory_lastaddr, 128); i += 128) {
        unsigned char buf[256];
        unsigned char chksum = 0;
        buf[0] = 0x50;
        buf[1] = i >> 24;
        buf[2] = i >> 16;
        buf[3] = i >>  8;
        buf[4] = i >>  0;
        for (int j = 0; j < 128; j++) {
            buf[5 + j] = memory_image[i + j];
        }
        for (int j = 0; j < 128 + 5; j++) {
            chksum += buf[j];
        }
        buf[128 + 5] = -chksum;
        if (serial_write(serial, buf, 128 + 6) != 0) {
            return WriteEraseFail;
        }
        /*
         * H8/3069の場合、書き込みにかかる最大時間は
         * 3000[ms]とデータシートに記されている。
         * com_getcはSERIAL_TIMEOUT_MS時間のタイムアウトが
         * 設定されている。
         * ここでは最大で(3000 / SERIAL_TIMEOUT_MS)回の読み出し
         * を試行して、期待する応答が帰ってくれば書き込みに成功
         * したものとみなす。
         *
         * 従来の多くの書き込みソフトウェアはこの処理の多くが
         * 固定値だった。
         * もしかしたら、「このH8はもう書けなくなった。」と思う物でも、
         * 書き込みにも成功してしまうかもしれない。
         */
        const int POLL_N_WRITE = (3000 / SERIAL_TIMEOUT_MS);
        for (int i = 0; i < POLL_N_WRITE; i++) {
            c = com_getc(serial);
            if (c == 0x06) {
                break;
            }
            fprintf(stderr, ":");
        }
        if (c != 0x06) {
            /*
             * ACKでない場合、続けてエラーコードが返る。
             */
            c = com_getc(serial);
            return WriteEraseFail;
        }
        fprintf(stderr, ".");
    }
    fprintf(stderr, "\n");

    /*
     * 書き込み終了コマンドを送信する。
     */
    {
        unsigned char buf[32];
        unsigned char chksum = 0;
        buf[0] = 0x50;
        buf[1] = 0xff;
        buf[2] = 0xff;
        buf[3] = 0xff;
        buf[4] = 0xff;
        for (int i = 0; i < 5; i++) {
            chksum += buf[i];
        }
        buf[5] = -chksum;
        if (serial_write(serial, buf, 6) != 0) {
            return WriteEraseFail;
        }
        c = com_getc(serial);
        if (c != 0x06) {
            /*
             * ACKでない場合、続けてエラーコードが返る。
             */
            c = com_getc(serial);
            return WriteEraseFail;
        }
    }

    return NoError;
}

int optparse_callback(const char option, const char *argument, void *extobj)
{
    user_option_t *user_option = (user_option_t *)extobj;

    /*
     * USER_OPTION_MOT_FILE()とUSER_OPTION_SERIAL_PORT()は別に取得する。
     */
    switch (option) {
        case '3':
            strcpy(USER_OPTION_CPU_NAME(user_option), "3");
            strcat(USER_OPTION_CPU_NAME(user_option), argument);
            break;
        case 'F':
        case 'f':
            if (sscanf(argument, "%i", &USER_OPTION_CPU_FREQ(user_option)) != 1) {
                USER_OPTION_ERROR(user_option) = 1;
            }
            break;
        case 'V':
        case 'v':
            USER_OPTION_VERSION(user_option) = 1;
            break;
        case 'H':
        case 'h':
            USER_OPTION_HELP(user_option) = 1;
            break;
        case 'D':
        case 'd':
            USER_OPTION_DEBUG(user_option) = 1;
            break;
    }

    return 0;
}

/**
 * @brief エントリーポイント。
 * @param argc 引数の数。
 * @param argv 引数の中身。
 * @return シェルに返す値。
 */
int main(int argc, char **argv)
{
    SERIAL *serial = NULL;
    enum ErrorCode ec = NoError;
    uint32_t devcode = 0;
    user_option_t user_option;

    /*
     * バナーを表示する。
     */
    fprintf(stderr, "=================================================\n");
    fprintf(stderr, " H8/3069F Flash Writer for KOZOS (Version %d.%d.%d)\n",
            VERSION_MAJOR,
            VERSION_MINOR,
            VERSION_RELEASE);
    fprintf(stderr, " Copyright(C) 2011-2012 Shinichiro Nakamura\n");
    fprintf(stderr, "=================================================\n");

    /*
     * オプションのデフォルト値を設定する。
     */
    strcpy(USER_OPTION_CPU_NAME(&user_option), "3069");
    USER_OPTION_CPU_FREQ(&user_option)    = 20;
    strcpy(USER_OPTION_MOT_FILE(&user_option), "");
    strcpy(USER_OPTION_SERIAL_PORT(&user_option), "");
    USER_OPTION_VERSION(&user_option)     = 0;
    USER_OPTION_HELP(&user_option)        = 0;
    USER_OPTION_DEBUG(&user_option)       = 0;
    USER_OPTION_ERROR(&user_option)       = 0;

    /*
     * 文字ベースでオプションを解析する。
     */
    optparse_char(argc, argv, &user_option, optparse_callback);

    /*
     * オプションの指定に誤りがある場合には、使い方を表示して終了する。
     */
    if ((argc < 3) || USER_OPTION_ERROR(&user_option)
            || USER_OPTION_VERSION(&user_option) || USER_OPTION_HELP(&user_option)) {
        fprintf(stderr, "%s [options...] [mot file] [interface]\n", argv[0]);
        fprintf(stderr, "\n");
        fprintf(stderr, " Target CPU        : -3069        (Default:3069)\n");
        fprintf(stderr, " Target Frequency  : -f[20 | 25]  (Default:20)\n");
        fprintf(stderr, " Version           : -v\n");
        fprintf(stderr, " Help              : -h\n");
        fprintf(stderr, " Debug             : -d\n");
        exit(1);
    }

    /*
     * 与えられた引数の最後の二つは、それぞれMOTファイルとシリアルポート名であるという仮定。
     */
    strcpy(USER_OPTION_MOT_FILE(&user_option), argv[argc - 2]);
    strcpy(USER_OPTION_SERIAL_PORT(&user_option), argv[argc - 1]);

    /*
     * デバッグ用の出力。
     */
    if (USER_OPTION_DEBUG(&user_option)) {
        fprintf(stderr, "Target CPU       : %s\n", USER_OPTION_CPU_NAME(&user_option));
        fprintf(stderr, "Target Frequency : %d\n", USER_OPTION_CPU_FREQ(&user_option));
        fprintf(stderr, "MOT File         : %s\n", USER_OPTION_MOT_FILE(&user_option));
        fprintf(stderr, "Seiral Port      : %s\n", USER_OPTION_SERIAL_PORT(&user_option));
    }

    /*
     * 与えられたオプションを検証する。
     */
    if (strcmp(USER_OPTION_CPU_NAME(&user_option), "3069") != 0) {
        /*
         * 3069F以外はエラーとする。
         * 3069F以外で検証するつもりは今のところないから対象外とする。
         * プロトコルが同じなら書き込めるだろう。
         */
        fprintf(stderr, "Unsupported CPU name found.\n");
        exit(1);
    }
    if ((USER_OPTION_CPU_FREQ(&user_option) != 20) && (USER_OPTION_CPU_FREQ(&user_option) != 25)) {
        /*
         * 20MHzでもなく、25MHzでもなければエラー。
         */
        fprintf(stderr, "Unsupported CPU frequency value found.\n");
        exit(1);
    }

    /*
     * メモリイメージを初期化する。
     */
    for (int i = 0; i < sizeof(memory_image); i++) {
        memory_image[i] = 0xff;
    }

    /*
     * motファイルを読み込む。
     */
    mot_t mot;
    mot.cb_data = mot_callback;
    if (mot_read(USER_OPTION_MOT_FILE(&user_option), &mot) != 0) {
        fprintf(stderr, "file read error\n");
        exit(1);
    }

    /*
     * シリアルポートをオープンする。
     */
    serial = serial_open(USER_OPTION_SERIAL_PORT(&user_option), SerialBaud19200);
    if (serial == NULL) {
        fprintf(stderr, "com port open error\n");
        exit(1);
    }

    /*
     * ビットレート合わせ込みのシーケンスを実行する。
     *
     * ビットレート合わせ込みシーケンスに初回時失敗する事がある。
     * マイコンの電源投入時やシリアルケーブル接続時に
     * プロセッサのUARTポートに意図しない信号が入る事が考えられる。
     *
     * そこで、本プログラムでは数回の試行を行うことにした。
     * これはユーザがかけている手間をプログラムが代行する事を意味する。
     */
    for (int i = 0; i < 2; i++) {
        ec = bitrate_sequence(serial);
        if (ec == NoError) {
            break;
        } else {
            /*
             * デバッグ用出力。
             *
             * ビットレートシーケンスは初回に失敗する可能性がある。
             * これに対してエラーを表示してしまうとユーザが混乱する。
             * よってデバッグ用出力としての位置づけにしておく。
             * リトライした結果として成功した事を知りたい時にのみ
             * 表示を有効にすると良い。
             */
            if (USER_OPTION_DEBUG(&user_option)) {
                fprintf(stderr, "Bitrate sequence failed.\n");
            }
        }
    }
    if (ec != NoError) {
        fprintf(stderr, "Bitrate sequence failed. (code=0x%02x)\n", (int)ec);
        PROGEXIT(1);
    }
    fprintf(stderr, "Bitrate sequence: Done.\n");

    /*
     * サポートデバイス問い合わせを実行する。
     */
    ec = inquiry_device(serial, &devcode);
    if (ec != NoError) {
        fprintf(stderr, "Inquiry device failed. (code=0x%02x)\n", (int)ec);
        PROGEXIT(1);
    }
    fprintf(stderr, "Inquiry device: Done.\n");

    /*
     * デバイス選択を実行する。
     */
    ec = select_device(serial, devcode);
    if (ec != NoError) {
        fprintf(stderr, "Select device failed. (code=0x%02x)\n", (int)ec);
        PROGEXIT(1);
    }
    fprintf(stderr, "Select device: Done.\n");

    /*
     * クロックモード問い合わせを実行する。
     */
    ec = inquiry_clockmode(serial);
    if (ec != NoError) {
        fprintf(stderr, "Inquiry clock mode failed. (code=0x%02x)\n", (int)ec);
        PROGEXIT(1);
    }
    fprintf(stderr, "Inquiry clock mode: Done.\n");

    /*
     * クロックモード選択を実行する。
     */
    ec = select_clockmode(serial);
    if (ec != NoError) {
        fprintf(stderr, "Select clock mode failed. (code=0x%02x)\n", (int)ec);
        PROGEXIT(1);
    }
    fprintf(stderr, "Select clock mode: Done.\n");

    /*
     * 新ビットレート選択を実行する。
     */
    ec = select_bitrate(serial, 19200, USER_OPTION_CPU_FREQ(&user_option));
    if (ec != NoError) {
        fprintf(stderr, "Select bitrate failed. (code=0x%02x)\n", (int)ec);
        PROGEXIT(1);
    }
    fprintf(stderr, "Select bitrate: Done.\n");

    /*
     * 書き込み消去ステータス遷移を実行する。
     * 同時に128バイト書き込みを実行する。
     */
    ec = program(serial);
    if (ec != NoError) {
        fprintf(stderr, "Program failed. (code=0x%02x)\n", (int)ec);
        PROGEXIT(1);
    }
    fprintf(stderr, "Program: Done.\n");

    /*
     * 上記のステップ全てに成功した時にのみCompleteが表示される。
     */
    fprintf(stderr, "Complete.\n");
    PROGEXIT(0);
}

