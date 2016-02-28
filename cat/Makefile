compiler = gcc -Wall -Wextra

build: cat

cat: cat.o
	$(compiler) $< -o $@

cat.o: cat.c
	$(compiler) -c $<

clean: 
	rm *.o

rebuild: clean build
