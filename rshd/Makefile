compiler = g++ -std=c++11 -Wall -Wextra

build: raw_message.o rshd.o rshd_server.o util.o wraps.o
	$(compiler) $^ -o rshd


%.o: %.cpp
	$(compiler) -c $<  

clean: 
	rm -f *.o

rebuild: clean build

