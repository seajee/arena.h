
test: test.c arena.h
	cc -Wall -Wextra -DDEBUG -o test test.c

clean:
	rm -rf test
