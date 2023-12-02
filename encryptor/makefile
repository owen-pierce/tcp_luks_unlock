CC=clang
CFLAGS=-g
BINS=encryptor
OBJS=encryptor.o aes.o

all: $(BINS)

encryptor: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ -lsodium

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

clean:
	rm -rf *.o encryptor

