CC = gcc
SHDIR = ../common
CFLAGS = -I $(SHDIR) -Wall -lpthread -lz -L/usr/bin/mysql -I/usr/include/mysql -lm -lmysqlclient          
#-lm 数学库 把-lmysqlclient一定放在编译最后

OBJS = $(SHDIR)/common.o client.o

all : client
	@echo "Finish!"
#@符号保证不输出该字符串

client: $(OBJS)
	@$(CC) -o $@  $(OBJS) $(CFLAGS)

#%所有.o文件依赖与.c文件 $< 所有的依赖项  $@表示目标文件
$(OBJS): %.o : %.c
	@$(CC) -c $< -o $@ $(CFLAGS)

.PHONY: clean   #伪目标
clean:
	@rm -f *.o pihealthd
	@rm -f $(SHDIR)/common.o
	@echo "Clean done"

#install:

