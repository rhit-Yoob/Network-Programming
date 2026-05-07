# Network Programming in C

A collection of three networking projects written in C, covering core networking concepts from raw socket communication to HTTP proxying.

---

## 1 — TCP & UDP Sockets

### Overview
Implements basic client-server communication using both TCP and UDP protocols in C. Explores the differences between connection-oriented and connectionless communication.

### What It Does
- TCP server and client that establish a reliable connection and exchange messages
- UDP server and client that send datagrams without a persistent connection
- Demonstrates the trade-offs between reliability (TCP) and speed/simplicity (UDP)

### How to Run

**TCP:**
```bash
# In one terminal (server)
./tcp_server <port>

# In another terminal (client)
./tcp_client <hostname> <port>
```

**UDP:**
```bash
# In one terminal (server)
./udp_server <port>

# In another terminal (client)
./udp_client <hostname> <port>
```

---

## 2 — File Transfer over Sockets

### Overview
Extends socket programming to support transferring files of any type between a client and server using a custom application protocol. Implemented in C using binary file I/O.

### What It Does
- `iWant <file>` — client requests a file from the server; server sends it if it exists
- `uTake <file>` — client sends a local file up to the server
- Works with any file type: `.txt`, `.pdf`, `.jpg`, `.mp3`, `.mp4`, etc.
- Files are read and written in binary mode to handle non-text data correctly
- Both client and server remain running across multiple transfers

### Directory Structure
```
file-transfer/
├── client/
│   ├── client.c
│   └── received_files/     # Files downloaded from the server land here
└── server/
    ├── server.c
    ├── store/              # Files available for the client to request
    └── received_files/     # Files uploaded by the client land here
```

### How to Run
```bash
# Start the server
cd file-transfer/server
./server <port>

# Start the client (in a separate terminal)
cd file-transfer/client
./client <hostname> <port>
```

### Example Session
```
> iWant dir1/photo.jpg
  What directory would you like to save this file?
> .
  file transfer started...
  file transfer of 3172645 bytes complete and placed in current directory

> uTake report.pdf
  What directory on the server would you like to save this file?
> received_files
  file transfer started...
  file transfer of 14253 bytes to server complete and placed in received_files

> exit
  See ya!
```

---

## 3 — HTTP Proxy

### Overview
A concurrent HTTP proxy server written in C that sits between a web browser and remote web servers. The proxy accepts HTTP requests from a browser, forwards them to the target server, and returns the response back to the client.

### What It Does
- Accepts HTTP GET requests from a browser configured to use the proxy
- Parses the absolute URL to extract host, port, and path
- Forwards a well-formed HTTP/1.0 request to the remote server
- Streams the server's response back to the browser
- Handles multiple simultaneous clients using `fork()`
- Returns a `501 Not Implemented` response for any method other than GET
- Returns a `400 Bad Request` response for malformed requests

### How to Run
```bash
# Build
make

# Run the proxy
./proxy <port>
```

Then configure your browser to use `localhost:<port>` as its HTTP proxy.

### Testing with Telnet
```bash
telnet localhost <port>
GET http://example.com/ HTTP/1.0
```

### Notes
- Supports **HTTP only** (not HTTPS)
- Tested with Firefox configured to use HTTP/1.0 with keepalive disabled
- Concurrent requests are handled by forking a child process per connection

---

## Building

Each project has its own directory with a `Makefile`. To build:

```bash
cd <project-directory>/
make
```

Requires `gcc` and a Unix-like environment (Linux or macOS).
