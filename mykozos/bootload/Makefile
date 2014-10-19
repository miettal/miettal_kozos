PREFIX  = /usr/local
ARCH    = h8300-elf
BINDIR  = $(PREFIX)/bin
ADDNAME = $(ARCH)-
FIRMDIR = ../os

AR      = $(BINDIR)/$(ADDNAME)ar
AS      = $(BINDIR)/$(ADDNAME)as
CC      = $(BINDIR)/$(ADDNAME)gcc
LD      = $(BINDIR)/$(ADDNAME)ld
NM      = $(BINDIR)/$(ADDNAME)nm
OBJCOPY = $(BINDIR)/$(ADDNAME)objcopy
OBJDUMP = $(BINDIR)/$(ADDNAME)objdump
RANLIB  = $(BINDIR)/$(ADDNAME)ranlib
STRIP   = $(BINDIR)/$(ADDNAME)strip

H8WRITE = ../../tools/h8write/h8write
KZ_H8WRITE = ../../tools/kz_h8write/kz_h8write

# FreeBSD-4.x:/dev/cuaaX, FreeBSD-6.x:/dev/cuadX, FreeBSD(USB):/dev/cuaUx
# Linux:/dev/ttySx, Linux(USB):/dev/ttyUSBx, Windows:comX
H8WRITE_SERDEV = /dev/ttyUSB0

OBJS  = vector.o startup.o intr.o main.o interrupt.o
OBJS += lib.o serial.o xmodem.o elf.o dram.o
#OBJS += firmdata.o

TARGET = kzload
FIRMWARE = kozos

CFLAGS = -Wall -mh -nostdinc -nostdlib -fno-builtin
#CFLAGS += -mint32 # intを32ビットにすると掛算／割算ができなくなる
CFLAGS += -I.
#CFLAGS += -g
CFLAGS += -Os
#CFLAGS += -O0
CFLAGS += -DKZLOAD
#CFLAGS += -DSIMULATOR

LFLAGS = -static -T ld.scr -L.

.SUFFIXES: .c .o
.SUFFIXES: .s .o
.SUFFIXES: .S .o

all :		$(TARGET)

$(TARGET) :	$(OBJS)
		$(CC) $(OBJS) -o $(TARGET) $(CFLAGS) $(LFLAGS)
		cp $(TARGET) $(TARGET).elf
		$(STRIP) $(TARGET)

main.o :	main.c $(FIRMWARE)
		$(CC) -c $(CFLAGS) $<
		$(OBJCOPY) --add-section=.firmdata=$(FIRMWARE) --set-section-flags=.firmdata=contents,alloc,load,readonly,data $@ $@

.c.o :		$<
		$(CC) -c $(CFLAGS) $<

.s.o :		$<
		$(CC) -c $(CFLAGS) $<

.S.o :		$<
		$(CC) -c $(CFLAGS) $<

#firmdata.o :	$(FIRMWARE)
#		$(LD) --format binary --oformat elf32-h8300 -r $(FIRMWARE) -o firmdata.o
#		$(LD) -r -T firmdata.scr firmdata.o.tmp -o firmdata.o

$(FIRMWARE) :	$(FIRMDIR)/$(FIRMWARE)
		cp $(FIRMDIR)/$(FIRMWARE) .

$(FIRMDIR)/$(FIRMWARE) :
		cd $(FIRMDIR) ; $(MAKE)

$(TARGET).mot :	$(TARGET)
		$(OBJCOPY) -O srec $(TARGET) $(TARGET).mot

image :		$(TARGET).mot

write :		$(TARGET).mot
		$(H8WRITE) -3069 -f20 $(TARGET).mot $(H8WRITE_SERDEV)

kzwrite :	$(TARGET).mot
		$(KZ_H8WRITE) -3069 -f20 $(TARGET).mot $(H8WRITE_SERDEV)

clean :
		rm -f $(OBJS) $(TARGET) $(TARGET).elf $(TARGET).mot
		rm -f $(FIRMWARE)
