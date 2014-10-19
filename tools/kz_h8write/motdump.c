/**
 * @file motdump.c
 * @author Shinichiro Nakamura
 * @brief motファイルダンププログラムの実装。
 */

/*
 * ===============================================================
 *  mot dump utility
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

#include <stdio.h>
#include <ctype.h>
#include "mot.h"

unsigned char memory_image[0x100000 + 0x100];
unsigned int memory_lastaddr = 0;

void cb_data(
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
 * @brief 1行に表示する桁数。
 */
#define COLSIZ 16

/**
 * @brief 切り上げ処理。
 * @details 数値AをBで切り上げる。
 */
#define ROUND_OUT(A,B) ((((A)+((B)-1))/(B))*(B))

/**
 * @brief エントリポイント。
 */
int main(int argc, char **argv) {
    if (argc != 2) {
        printf("%s [filename]\n", argv[0]);
        return 1;
    }
    mot_t mot;
    mot.cb_data = cb_data;
    if (mot_read(argv[1], &mot) == 0) {
        unsigned char cbuf[COLSIZ];
        unsigned int ccnt = 0;
        for (unsigned int addr = 0; addr < ROUND_OUT(memory_lastaddr, COLSIZ); addr++) {
            /*
             * 行頭表示処理。
             */
            if ((addr % COLSIZ) == 0) {
                printf("\n");
                printf("0x%04X :", addr);
            }
            /*
             * データ表示処理。
             */
            printf(" %02X", memory_image[addr] & 0xff);
            cbuf[addr % COLSIZ] = memory_image[addr] & 0xff;
            ccnt++;
            /*
             * 行端表示処理。
             */
            if (((addr + 1) % COLSIZ) == 0) {
                if (ccnt > 0) {
                    printf(" | ");
                    for (int i = 0; i < COLSIZ; i++) {
                        if (i < (int)ccnt) {
                            printf("%c", isprint(cbuf[i] & 0xff)
                                    ? (cbuf[i] & 0xff) : '.');
                        } else {
                            printf(" ");
                        }
                    }
                }
                ccnt = 0;
            }
        }
    } else {
        printf("Read failed.\n");
    }
    printf("\n");
    return 0;
}

