CC=clang
CFLAGS=-g
BINS=encryptor
OBJS=encryptor.o aes.o

all: $(BINS)

encryptor: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

clean:
	rm -rf *.dSYM $(BINS)

