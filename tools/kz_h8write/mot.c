/**
 * @file mot.c
 * @author Shinichiro Nakamura
 * @brief motファイルモジュールの実装。
 */

/*
 * ===============================================================
 *  mot file interface library
 *  Version 0.0.3
 * ===============================================================
 * Copyright (c) 2010-2011 Shinichiro Nakamura
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

#include "mot.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

/**
 * @brief 文字列に含まれる制御コードを取り除く。
 * @details
 * この関数は制御コードを見つけるとNULLに置き換える。
 * 文字列の途中に制御コードが見つかった場合、
 * それはNULLとなり文字列の終端となる。
 * 文字列の途中に制御コードが含まれるケースを今回は
 * 考慮する必要はない。
 */
static void text_trim(char *txt) {
    const int len = strlen(txt);
    for (int i = 0; i < len; i++) {
        if ((txt[i] == '\r') || (txt[i] == '\n') || (txt[i] == '\t')) {
            txt[i] = '\0';
        }
    }
}

/**
 * @brief 与えられた文字を１６進数として解釈し値を得る。
 * @param c 文字。
 * @retval 0以上 値。
 * @retval 負値 エラー。
 */
static unsigned int get_hex_num(const char c) {
    static const char *hextxt = "0123456789ABCDEF";
    for (int i = 0; i < 16; i++) {
        if (c == hextxt[i]) {
            return i;
        }
    }
    return -1;
}

/**
 * @brief １レコードをパースする。
 * @param p mot構造体。(コールバック関数などの情報を参照する。)
 * @param txt １レコード。
 * @retval 0 成功
 * @retval 0以外 エラー番号。
 */
static int parse_record(mot_t *p, const char *txt)
{
    unsigned char buf[BUFSIZ];

    /*
     * 与えられた文字列は少なくとも４バイト長なければおかしい。
     */
    const int txtlen = strlen(txt);
    if (txtlen < 4) {
        return -1;
    }

    /*
     * 最初の文字は必ずSでなければならない。
     */
    if (txt[0] != 'S') {
        return -2;
    }

    /*
     * 次の文字はレコードタイプである。
     */
    if (!isdigit(txt[1])) {
        return -3;
    }

    /*
     * その次の２バイトはレコードに含まれるアドレスからチェックサムまでのデータバイト数である。
     * ここでバイト数とテキスト長を比較して正当なレコードであることを検証する。
     */
    const int datlen = (get_hex_num(txt[2]) * 0x10) + (get_hex_num(txt[3] * 0x01));
    if ((datlen * 2) != txtlen - 4) {
        return -4;
    }

    /*
     * チェックサムを検証する。
     * チェックサムはデータ長からチェックサムの前までの加算に対して１の補数をとったものである。
     *
     * 1. チェックサムの計算とデータの格納を行う。
     * 2. チェックサムの妥当性を検証する。
     *
     * 同時に内部バッファ(buf)にデータをコピーする。
     * 内部バッファ(buf)にはデータバイト数以降のデータが格納される。
     */
    unsigned char chksum = datlen;
    for (int i = 0; i < datlen; i++) {
        buf[i] = (get_hex_num(txt[4 + (i * 2)]) * 0x10) + (get_hex_num(txt[5 + (i * 2)] * 0x01));
        if (i < datlen - 1) {
            chksum += buf[i];
        }
    }
    unsigned char verify = ~buf[datlen - 1];
    if (chksum != verify) {
        return -5;
    }

    /*
     * 全てが妥当なデータと見なせる。
     * よってコールバックを呼び出す。
     * ここでレコードの種類によって呼び出す関数を分けることもできる。
     *
     * この関数に渡された元データのゼロから数えて第１バイト目がレコードの種類を示している。
     */
    switch (txt[1]) {
        case '0':
            /*
             * スタートレコード。
             */
            // @todo 必要になったら実装する。
            break;
        case '1':
            /*
             * データレコード。(16ビットアドレスデータ)
             */
            if (p->cb_data != NULL) {
                unsigned int addr =
                    (buf[0] * 0x00000100) +
                    (buf[1] * 0x00000001);
                p->cb_data(addr, buf + 2, datlen - 2 - 1);
            }
            break;
        case '2':
            /*
             * データレコード。(24ビットアドレスデータ)
             */
            if (p->cb_data != NULL) {
                unsigned int addr =
                    (buf[0] * 0x00010000) +
                    (buf[1] * 0x00000100) +
                    (buf[2] * 0x00000001);
                p->cb_data(addr, buf + 3, datlen - 3 - 1);
            }
            break;
        case '3':
            /*
             * データレコード。(32ビットアドレスデータ)
             */
            if (p->cb_data != NULL) {
                unsigned int addr =
                    (buf[0] * 0x01000000) +
                    (buf[1] * 0x00010000) +
                    (buf[2] * 0x00000100) +
                    (buf[3] * 0x00000001);
                p->cb_data(addr, buf + 4, datlen - 4 - 1);
            }
            break;
        case '4':
            /*
             * シンボルレコード。
             */
            // @todo 必要になったら実装する。
            break;
        case '5':
            /*
             * データレコード数。
             */
            // @todo 必要になったら実装する。
            break;
        case '6':
            /*
             * 未使用。
             */
            // @todo 必要になったら実装する。
            break;
        case '7':
            /*
             * データレコード終了。(32ビットアドレスデータ)
             */
            // @todo 必要になったら実装する。
            break;
        case '8':
            /*
             * データレコード終了。(24ビットアドレスデータ)
             */
            // @todo 必要になったら実装する。
            break;
        case '9':
            /*
             * データレコード終了。(16ビットアドレスデータ)
             */
            // @todo 必要になったら実装する。
            break;
        default:
            /*
             * 不明なデータ。
             */
            break;
    }

    return 0;
}

/**
 * @brief motファイルを読み込む。
 *
 * @param filename ファイル名。(NULLを与えると標準入力を入力とする。)
 * @param p mot構造体。(コールバック関数などの情報を参照する。)
 *
 * @retval 0 成功
 * @retval 0以外 エラー番号。
 */
int mot_read(const char *filename, mot_t *p)
{
    char buf[BUFSIZ];
    FILE *fp = NULL;

    if (filename == NULL) {
        /*
         * 標準入力を入力用ファイルとして用いる。
         */
        fp = stdin;
        if (fp == NULL) {
            return -1;
        }
    } else {
        /*
         * ファイルをオープンする。
         */
        fp = fopen(filename, "r");
        if (fp == NULL) {
            return -1;
        }
    }

    /*
     * レコードを１つずつ読み込んでパースする。
     */
    while (fgets(buf, sizeof(buf), fp)) {
        const int len = strlen(buf);
        if (len > 0) {
            text_trim(buf);
            if (parse_record(p, buf) != 0) {
                fclose(fp);
                return -2;
            }
        }
    }

    /*
     * ファイルをクローズする。
     */
    fclose(fp);
    return 0;
}

