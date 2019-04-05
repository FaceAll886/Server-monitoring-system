CC := gcc
CFLAGS := -Wall -g -Os

SHDIR := ../common

OBJS = client.o $(SHDIR)/common.o

all: client

ftclient: $(OBJS)
	@$(CC) -o client $(CFLAGS) $(OBJS)

$(OBJS) : %.o: %.c 
	@$(CC) -c $(CFLAGS) $< -o $@

.PHONY:
clean:
	@rm -f *.o client
	@rm -f ../common/*.o
	@echo Done cleaning