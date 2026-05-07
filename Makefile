# To compile all three programs, list all executable names here.
# by Brian Yoo
all: client server

# Creates the executable "client" using the flag "-o".
client: client.o
	gcc client.o -o client -g -lm -Wall

# Creates the object file "client.o" using the flag "-c".
client.o: client.c
	gcc client.c -c -g -lm -Wall

# Creates the executable "server" using the flag "-o".
server: server.o
	gcc server.o -o server -g -lm -Wall

# Creates the object file "server.o" using the flag "-c".
server.o: server.c
	gcc server.c -c -g -lm -Wall

clean:
	rm -f *.o client server
