CFLAGS?=-g -Wall -Wno-unused-value

all: locks_exec

locks_exec: threads.c nlocks.c
	$(CC) $(CFLAGS) -o out1 threads.c -lpthread
	$(CC) $(CFLAGS) -o out2 nlocks.c -lpthread
	
run: locks_exec
	./out1 $(THREADS)
	./out2

	

