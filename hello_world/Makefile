compiler = gcc -Wall -Wextra

build: hello_world

%: %.o
	$(compiler) $< -o $@

%.o: %.c
	$(compiler) -c $<  

clean: 
	rm -f *.o

rebuild: clean build

