compiler = gcc -Wall -Wextra

build: midtermsh

%: %.o
	$(compiler) $< -o $@

%.o: %.c
	$(compiler) -c $<

clean: 
	rm -f *.o

rebuild: clean build
