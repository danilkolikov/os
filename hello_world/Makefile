compiler = gcc -Wall -Wextra

build: hello_world

hello_world: hello_world.o
	$(compiler) $< -o $@

hello_world.o: hello_world.c
	$(compiler) -c $<  

clean: 
	rm *.o

rebuild: clean build

