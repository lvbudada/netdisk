SRCS:=$(wildcard *.c)
OBJS:=$(patsubst %.c,%.o,$(SRCS))
ELF:=server
CC:=gcc
CFLAGS:=-Wall -I../include -I /usr/include/mysql/
$(ELF):$(OBJS)
	gcc $^ -o $@ -lpthread -lcrypt -lmysqlclient
.PYONY:clean
clean:
	rm -rf $(OBJS) $(ELF)
