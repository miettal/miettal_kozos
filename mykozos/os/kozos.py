#!/usr/bin/env python
# coding:utf-8
#
# kozos.py
#
# Author:   Hiromasa Ihara (miettal)
# URL:      http://miettal.com
# License:  MIT License
# Created:  2014-08-27
#

import serial
from threading import Thread, RLock
from xmodem import XMODEM
import time
import sys

class shell() :
  def __init__(self, line, speed, image) :
    self.con = serial.Serial(line, speed, timeout=0.01)
    self.lock = RLock()
    def auto_print() :
      while True :
        with self.lock :
          sys.stdout.write(self.con.read(32))
        time.sleep(0.01)
    self.thread = Thread(target=auto_print)
    self.thread.daemon = True
    self.thread.start()
    self.command("")
    self.command("down")

    while True :
      self.command("load")

      with self.lock :
        def getc(size, timeout=1.0):
          start = time.time()

          data = ""
          while len(data) < size :
            data += self.con.read()
            now = time.time()

            if now-start > timeout :
              break
          return data
        def putc(data, timeout=1.0):
          self.con.write(data)

        modem = XMODEM(getc, putc)
        stream = file(image, 'rb')
        ret = modem.send(stream, retry=4, timeout=1.0)
        if ret == True :
          break

    self.command("run")
    with self.lock :
      while True :
        if "succeed" in self.con.readline() :
          break

  def send_str(self, data) :
    time.sleep(0.05)
    for x in data :
      with self.lock :
        self.con.write(x)
        self.con.flush()
      time.sleep(0.05)
      

  def command(self, cmd) :
    self.send_str(cmd+"\n")

class gdb() :
  def __init__(self, line, speed) :
    self.con = serial.Serial(line, speed)

  def stub_puts(self, buf) :
    while True :
      data = ""
      data += "$"
      data += buf

      sum_ = 0
      for c in buf :
        sum_ += ord(c)

      data += "#"

      data += hex((sum_>>4)&0xf)[-1]
      data += hex(sum_&0xf)[-1]

      self.con.write(data)

      c = self.con.read()
      if c == '+' :
        break;

    return buf

  def stub_gets(self) :
    line = ""
    while True :
      c = self.con.read()
      if 0 < len(c) :
        if c == "#" :
          self.con.read()
          self.con.read()
          break;
        elif c == '$' :
          pass
        else :
          line += c

    self.con.write('+')
    return line

  def get_registers(self) :
    self.stub_puts('g')
    data = self.stub_gets()
    registers = []
    for x in range(10) :
      registers.append(int(data[x*8:x*8+8], 16))

    return registers

  def get_memory(self, addr, byte) :
    self.stub_puts('m'+str(hex(addr)[2:])+','+str(hex(byte)[2:]))
    data = self.stub_gets()
    return int(data, 16)
