#!/usr/bin/env python
# coding:utf-8
#
# autorun.py
#
# Author:   Hiromasa Ihara (miettal)
# URL:      http://miettal.com
# License:  MIT License
# Created:  2014-10-20
#

from kozos import shell, gdb

sh = shell('/dev/ttyUSB0', 38400, 'kozos')
sh.send_str([chr(0x03)])
