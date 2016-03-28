compiler = gcc -Wall -Wextra

build: sigusr

%: %.o
	$(compiler) $< -o $@

%.o: %.c
	$(compiler) -c $<

clean: 
	rm -f *.o

rebuild: clean build
