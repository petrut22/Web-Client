CC=g++
CFLAGS = -std=c++11 -g -Werror -Wall 

client: client.cpp requests.cpp helpers.cpp buffer.cpp
	$(CC) -o client client.cpp requests.cpp helpers.cpp buffer.cpp -ljson-c

run: client
	./client

clean:
	rm -f *.o client
