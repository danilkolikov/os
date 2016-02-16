compiler = gcc -Wall -Wextra
name = hello_world

build: hello_world

hello_world: hello_world.c
	$(compiler) $< -o $(name) 
clean: 
	rm *.o

rebuild: clean build
