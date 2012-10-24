
CC = gcc
CFLAGS = `pkg-config opencv gtk+-2.0 --cflags --libs`
LDFLAGS = 
DEPS = pixelLib.h starAlign.h starStack.h arguments.h fileManage.h gui.h
SRCS = $(DEPS:.h=.c) main.c 
OBJS = $(SRCS:.c=.o) 
EXEC = openStarStack 


%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) -I/opt/local/include -std=c99 

$(EXEC): $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS)	-L/opt/local/lib -largp  
    
clean:
	rm -f *.o
