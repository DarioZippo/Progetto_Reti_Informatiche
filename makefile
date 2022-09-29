# make rule primaria con dummy target ‘all’--> non crea alcun file all ma fa un complete build
# che dipende dai target client e server scritti sotto
all: dev serv

# make rule per il client
dev: device.o vector.o
	gcc -Wall device.o vector.o -o dev

device.o: device.c
	gcc -Wall -c -g device.c

# make rule per il server
serv: server.o vector.o
	gcc -Wall server.o vector.o -o serv

server.o: server.c
	gcc -Wall -c -g server.c