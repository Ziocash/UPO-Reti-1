CFLAGS += -Wall -Wextra -pedantic -g

.PHONY: all client server clean

all: client server

client:
	$(CC) $(CFLAGS) client.c -o client

server:
	$(CC) $(CFLAGS) server.c -o server

clean:
	$(RM) client server