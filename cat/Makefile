compiler = gcc -Wall -Wextra

build: cat

%: %.o
	$(compiler) $< -o $@

%.o: %.c
	$(compiler) -c $<

clean: 
	rm *.o

rebuild: clean build
