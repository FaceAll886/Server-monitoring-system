CC := gcc
CFLAGS := -Wall -g -Os

SHDIR := ../common

OBJS = master.o $(SHDIR)/common.o

all: master

master: $(OBJS)
	@$(CC) -o master $(CFLAGS) $(OBJS)

$(OBJS) : %.o: %.c 
	@$(CC) -c $(CFLAGS) $< -o $@

.PHONY:
clean:
	@rm -f *.o master
	@rm -f ../common/*.o
	@echo Done cleaning
