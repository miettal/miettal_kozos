#!/usr/bin/env python
# coding:utf-8
#
# test.py
#
# Author:   Hiromasa Ihara (miettal)
# URL:      http://miettal.com
# License:  MIT License
# Created:  2014-08-13
#

import sys
import serial
from subprocess import *
from kozos import shell, gdb
import time

def get_symtable(filename) :
  p = Popen(['h8300-elf-nm', filename], stdin=PIPE, stdout=PIPE)
  table = []
  for x in ''.join(p.communicate()[0]).split('\n') :
    if len(x.split(' ')) < 3 :
      continue
    addr = int(x.split(' ')[0], 16)
    type_ = x.split(' ')[1]
    label = x.split(' ')[2]
#    print hex(addr), label
    if type_ != 't' and type_ != 'T' :
      continue
    if label[0] == '.' :
      continue
    table.append({
      "addr":addr,
      "label":label[1:],
    })
  table = sorted(table, key=lambda x: x["addr"])
  return table

def get_sym(addr, table) :
  before = filter(lambda x:x["addr"] <= addr, table)
  before = sorted(before, key=lambda x: x["addr"])

  if len(before) == 0 :
    return {'addr':0x0, 'label':"unknown"}
  else :
    return before[-1]

table = get_symtable('kozos.elf')
sh = shell('/dev/ttyUSB0', 38400, 'kozos')
gdb = gdb('/dev/ttyUSB1', 38400)

while True :
  sh.send_str([chr(0x03)])
  #sh.command("debug")
  gdb.stub_gets()

  registers = gdb.get_registers()
  print "\nregisters"
  for (i, register) in enumerate(registers) :
    if i < 8 :
      print "ER{}->{:06x}".format(i, register)
    elif i == 8 :
      print "CCR->{:06x}".format(register)
    else :
      print "PC ->{:06x}".format(register)

  print "\ncall stack"
  sym = get_sym(registers[9], table)
  print "PC             {:06x}->{:s}".format(
    registers[9],
    sym['label'],
    )

  fp = registers[6]
  while True :
    data = gdb.get_memory(fp+4, 4)
    sym = get_sym(data, table)
    if sym['label'] == "etext" or sym['label'] == 'unknown' :
      break
    print "STACK [{:06x}] {:06x}->{:s}".format(fp+4-registers[6],
      data,
      sym['label'],
      )
    fp = gdb.get_memory(fp, 4)

  gdb.stub_puts('c')
  #time.sleep(1)

