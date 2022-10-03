# make rule primaria con dummy target ‘all’--> non crea alcun file all ma fa un complete build
# che dipende dai target client e server scritti sotto
all: dev serv

# make rule per il device 
dev: device.o 
	gcc -Wall -o dev device.o

device.o: device.c
	gcc -c -Wall -o device.o device.c

# make rule per il server 
serv: server.o serverCommands.o deviceCommands.o util.o vector.o
	gcc -Wall server.o serverCommands.o deviceCommands.o util.o vector.o -o serv

server.o: server.c
	gcc -Wall -c -g server.o server.c 

serverCommands.o: server/src/serverCommands.c 
	gcc -Wall -c -g server/src/serverCommands.o server/src/serverCommands.c

deviceCommands.o: server/src/deviceCommands.c 
	gcc -Wall -c -g server/src/deviceCommands.o server/src/deviceCommands.c

util.o: server/src/util.c 
	gcc -Wall -c -g server/src/util.o server/src/util.c

vector.o: server/src/vector.c 
	gcc -Wall -c -g server/src/vector.o server/src/vector.c

# pulizia dei file della compilazione (eseguito con make clean)
clean:
	rm *o dev serv