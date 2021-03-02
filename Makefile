CC = gcc
CFLAGS = -Wall -Wextra -pedantic
FILES = ipk-server.c ipk-client.c

all: ipk-client ipk-server



server: ipk-server.c
	$(CC) $(CFLAGS) -o $@ server.c


client: ipk-client.c
	$(CC) $(CFLAGS) -o $@ client.c

clean:
	rm -f *.o *.out ipk-client ipk-server *~
