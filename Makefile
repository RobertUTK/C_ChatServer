PROG = chat_server

LIBFDR = /home/cosc360/libfdr
GS_DIR = /home/cosc360/lab9/gs

CC = gcc
INCLUDES = -I$(LIBFDR)/include/ -pthread
CFLAGS = -g -Wall -MD -std=gnu99 $(INCLUDES)

SRC = $(wildcard src/*.c)
OBJ = $(SRC:.c=.o)
LIBS = $(LIBFDR)/lib/libfdr.a

all: $(PROG)

$(PROG): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

chatroom: src/chatroom.c src/chatroom.h
	$(CC) $(CFLAGS) -c src/chatroom.c

client: src/client.c src/client.h
	$(CC) $(CFLAGS) -c src/client.c

.PHONY: gs ga auto-gs auto-ga clean

gs: $(PROG)
	$(GS_DIR)/gradescript $(test) $(host) $(port)

ga: $(PROG)
	$(GS_DIR)/gradeall $(host) $(port)

auto-gs: $(PROG)
	$(GS_DIR)/gradescript-auto $(test)

auto-ga: $(PROG)
	$(GS_DIR)/gradeall-auto

clean:
	rm -rf $(PROG) $(OBJ) $(OBJ:.o=.d) tmp-* output-* server.log *.o *.d

-include $(OBJ:.o=.d)
