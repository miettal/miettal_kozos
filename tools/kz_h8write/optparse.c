/**
 * @file optparse.c
 * @author Shinichiro Nakamura
 * @brief オプション解析モジュールの実装。
 */

/*
 * ===============================================================
 *  Option parse library
 *  Version 0.0.1
 * ===============================================================
 * Copyright (c) 2012 Shinichiro Nakamura
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

#include <string.h>
#include "optparse.h"

int optparse_char(
        int argc,
        char **argv,
        void *extobj,
        int (*callback)(
            const char option, const char *argument, void *extobj))
{
    for (int i = 1; i < argc;) {
        int r = 0;
        int c = 0;
        if (argv[i][0] == '-') {
            /*
             * 与えられた文字列は引数である。
             */
            if (strlen(argv[i]) > 2) {
                /*
                 * 同じ箇所にアーギュメントが指定されている。
                 */
                r = callback(argv[i][1], &argv[i][2], extobj);
                c = 1;
            } else {
                if ((i + 1) < argc) {
                    /*
                     * まだ続きがありそうだ。
                     * 先を見ておこう。
                     */
                    if (argv[i + 1][0] == '-') {
                        /*
                         * その引数にアーギュメントはない。
                         */
                        r = callback(argv[i][1], "", extobj);
                        c = 1;
                    } else {
                        /*
                         * その引数にアーギュメントがある。
                         */
                        r = callback(argv[i][1], argv[i + 1], extobj);
                        c = 2;
                    }
                } else {
                    /*
                     * もう先がない。
                     * ということはアーギュメントはない。
                     */
                    r = callback(argv[i][1], "", extobj);
                    c = 1;
                }
            }
        } else {
            /*
             * 与えられた文字列はオプションでない
             * 単独アーギュメントである。
             */
            r = callback('\x00', &argv[i][0], extobj);
            c = 1;
        }
        i += c;
        if (r != 0) {
            break;
        }
    }
    return 0;
}

int optparse_text(
        int argc,
        char **argv,
        void *extobj,
        int (*callback)(
            const char *option, const char *argument, void *extobj))
{
    for (int i = 1; i < argc;) {
        int r = 0;
        int c = 0;
        if (argv[i][0] == '-') {
            /*
             * 与えられた文字列は引数である。
             */
            if ((i + 1) < argc) {
                /*
                 * まだ続きがありそうだ。
                 * 先を見ておこう。
                 */
                if (argv[i + 1][0] == '-') {
                    /*
                     * その引数にアーギュメントはない。
                     */
                    r = callback(&argv[i][1], "", extobj);
                    c = 1;
                } else {
                    /*
                     * その引数にアーギュメントがある。
                     */
                    r = callback(&argv[i][1], argv[i + 1], extobj);
                    c = 2;
                }
            } else {
                /*
                 * もう先がない。
                 * ということはアーギュメントはない。
                 */
                r = callback(&argv[i][1], "", extobj);
                c = 1;
            }
        } else {
            /*
             * 与えられた文字列はオプションでない
             * 単独アーギュメントである。
             */
            r = callback("", &argv[i][0], extobj);
            c = 1;
        }
        i += c;
        if (r != 0) {
            break;
        }
    }
    return 0;
}

