CC=clang
CFLAGS=-g
BINS=tcp_server
OBJS=tcp_server.o myqueue.o aes.o ini.o ezini.o

all: $(BINS)

tcp_server: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ -lpthread

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

clean:
	rm -rf *.o tcp_server

