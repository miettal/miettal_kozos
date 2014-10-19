/**
 * @file mot.h
 * @author Shinichiro Nakamura
 * @brief motファイルモジュールのインターフェース定義。
 */

/*
 *
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

#ifndef MOT_H
#define MOT_H

typedef struct {
    void (*cb_data)(const unsigned int addr, const unsigned char *buf, const int siz);
} mot_t;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief motファイルを読み込む。
 *
 * @param filename ファイル名。(NULLを与えると標準入力を入力とする。)
 * @param p mot構造体。(コールバック関数などの情報を参照する。)
 *
 * @retval 0 成功
 * @retval 0以外 エラー番号。
 */
int mot_read(const char *filename, mot_t *p);

#ifdef __cplusplus
}
#endif

#endif

