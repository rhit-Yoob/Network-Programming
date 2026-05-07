# To compile all three programs, list all executable names here.
# by Brian Yoo
all: proxy

# Creates the executable "proxy" using the flag "-o".
proxys: Proxy/proxy.o
	gcc Proxy/proxy.o -o proxy -g -lm -Wall

# Creates the object file "proxy.o" using the flag "-c".
Proxy/proxy.o: Proxy/proxy.c
	gcc Proxy/proxy.c -c -g -lm -Wall

clean:
	rm -f *.o proxy
