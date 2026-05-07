# To compile all three programs, list all executable names here.
# by Brian Yoo
all: clients servers

# Creates the executable "client" using the flag "-o".
clients: Client/client.o
	gcc Client/client.o -o clients -g -lm -Wall

# Creates the object file "client.o" using the flag "-c".
client.o: Client/client.c
	gcc Client/client.c -c -g -lm -Wall

# Creates the executable "server" using the flag "-o".
servers: Server/server.o
	gcc Server/server.o -o servers -g -lm -Wall

# Creates the object file "server.o" using the flag "-c".
server.o: Server/server.c
	gcc Server/server.c -c -g -lm -Wall

clean:
	rm -f *.o clients servers
