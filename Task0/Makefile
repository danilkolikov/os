compiler = gcc -Wall -Wextra
name = hello-world

build: hello-world

hello-world: hello-world.c
	$(compiler) $< -o $(name) 
clean: 
	rm *.o

rebuild: clean build
