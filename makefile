memoria: memoria.c
	gcc $(CFLAGS) -o memoria memoria.c -lncurses
clean:
	rm memoria *.o
