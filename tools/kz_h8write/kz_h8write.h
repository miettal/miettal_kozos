/**
 * @file kz_h8write.h
 * @author Shinichiro Nakamura
 * @brief H8/3069F write for KOZOSのインターフェース定義。
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

#ifndef KZ_H8WRITE_H
#define KZ_H8WRITE_H

#define VERSION_MAJOR 0
#define VERSION_MINOR 2
#define VERSION_RELEASE 1

/**
 * @brief エラーコード。
 */
enum ErrorCode {
    NoError = 0,
    SerialOpenError,
    SerialWriteError,
    InvalidPrimaryResponseCode,
    NoPrimaryResponseCode,
    InvalidSecondaryResponseCode,
    NoSecondaryResponseCode,
    UnsupportedDevice,
    UnmatchedDeviceCode,
    UnsupportedClockMode,
    UnmatchedClockMode,
    InvalidBitrate,
    WriteEraseFail,
};

#endif

