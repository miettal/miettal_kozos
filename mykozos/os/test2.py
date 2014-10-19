#!/usr/bin/env python
# coding:utf-8
#
# test2.py
#
# Author:   Hiromasa Ihara (miettal)
# URL:      http://miettal.com
# License:  MIT License
# Created:  2014-08-15
#

from subprocess import *
import sys
import time

def get_symtable(filename) :
  p = Popen(['h8300-elf-nm', filename], stdin=PIPE, stdout=PIPE)
  l = []
  for x in ''.join(p.communicate()[0]).split('\n') :
    if len(x.split(' ')) < 3 :
      continue
    addr = int(x.split(' ')[0], 16)
    type_ = x.split(' ')[1]
    label = x.split(' ')[2]
    if type_!= 't' :
      continue
    if label[0] == '.' :
      continue
    l.append({
      "addr":addr,
      "label":label[1:],
    })
  return l

def get_sym(addr, table) :
  a = 0xffffffff

  for x in table :
    if 0 < x["addr"] - addr and a - addr:
      a = x["addr"]
      l = x["label"]

  return {"addr":a, "label":l}


addr = 0x405a8
table = get_symtable('kozos.elf')
print get_sym(addr, table)


#for x in p.communicate() :
#  print x.split(' ')
#time.sleep(0.1)
#p.stdout.seek(0, 2)
#
#p.stdin.write('p/a 0x405a8\n')
#p.stdin.flush()
#while True :
#  #sys.stdout.write("TEST")
#  sys.stdout.write(p.stdout.read(1))
#  sys.stdout.flush()
