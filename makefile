# make rule primaria con dummy target ‘all’--> non crea alcun file all ma fa un complete build
# che dipende dai target client e server scritti sotto
all: dev serv

# make rule per il device 
dev: device.o vector.o
	gcc -Wall -o dev device.o vector.o

device.o: device.c
	gcc -Wall -c -g device.c

vector.o: device/vector.c 
	gcc -Wall -c -g device/vector.c

# make rule per il server 
serv: server.o serverCommands.o deviceCommands.o util.o vector.o messaggio.o record.o
	gcc -Wall server.o serverCommands.o deviceCommands.o util.o vector.o messaggio.o record.o -o serv

server.o: server.c
	gcc -Wall -c -g server.c 

serverCommands.o: server/src/serverCommands.c 
	gcc -Wall -c -g server/src/serverCommands.c

deviceCommands.o: server/src/deviceCommands.c 
	gcc -Wall -c -g server/src/deviceCommands.c

util.o: server/src/util.c 
	gcc -Wall -c -g server/src/util.c

vector.o: server/src/vector.c 
	gcc -Wall -c -g server/src/vector.c

messaggio.o: server/src/messaggio.c
	gcc -Wall -c -g server/src/messaggio.c

record.o: server/src/record.c
	gcc -Wall -c -g server/src/record.c

# pulizia dei file della compilazione (eseguito con make clean)
clean:
	rm *o dev serv