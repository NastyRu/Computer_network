CC := gcc

server.out: server.o queue.o
	$(CC) -g3 $^ -o $@

client.out: client.o
	$(CC) $^ -o $@

%.o: %.c queue.h
	$(CC) -c $<

clean:
	rm *.o *.out
