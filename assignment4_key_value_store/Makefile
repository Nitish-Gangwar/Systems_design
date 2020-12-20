#hello: shell.c
		#gcc -o hello shell.c
CC=g++
CFLAGS= -Wall -Wextra -g

hello: server client


server: KVServer.cpp KVStore.cpp KVServerLibrary.cpp KVStore.h
	$(CC) $(CFLAGS) KVServer.cpp -o server -pthread

client: KVClient.cpp KVClientLibrary.cpp KVClientLibrary.h
	$(CC) $(CFLAGS) KVClient.cpp -o client

clean:
	rm server
	rm client
	

