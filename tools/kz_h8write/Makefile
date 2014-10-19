TARGET1=kz_h8write
SRCS1=kz_h8write.c serial_linux.c mot.c optparse.c

TARGET2=motdump
SRCS2=motdump.c mot.c

all: $(TARGET1) $(TARGET2)

clean:
	@rm -f $(TARGET1) $(TARGET2)

$(TARGET1):$(SRCS1)
	$(CC) -o $(TARGET1) $(SRCS1) -std=c99 -g -Wall

$(TARGET2):$(SRCS2)
	$(CC) -o $(TARGET2) $(SRCS2) -std=c99 -g -Wall
