all: build
	./client.out
build:
	gcc client.c -g -o client.out
clean:
	rm -f client.out server.out

