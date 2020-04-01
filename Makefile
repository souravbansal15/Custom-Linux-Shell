all: build exe
build:
	gcc sh.c -o sh
exe:
	./sh
clean:
	rm sh