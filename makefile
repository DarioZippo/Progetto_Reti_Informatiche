# make rule primaria con dummy target ‘all’--> non crea alcun file all ma fa un complete build
# che dipende dai target client e server scritti sotto
all: dev serv

# make rule per il device 
dev: device.o ./device/src/globals.o ./device/src/vector.o
	gcc -Wall -o dev device.o ./device/src/globals.o ./device/src/vector.o

device.o: device.c
	gcc -Wall -c -g device.c

dev_globals.o: globals.c 
	gcc -Wall -c -g device/src/globals.c

dev_vector.o: vector.c 
	gcc -Wall -c -g device/src/vector.c

# make rule per il server 
serv: server.o  ./server/src/globals.o ./server/src/serverCommands.o ./server/src/deviceCommands.o ./server/src/util.o ./server/src/vector.o ./server/src/message.o ./server/src/record.o
	gcc -Wall server.o ./server/src/globals.o ./server/src/serverCommands.o ./server/src/deviceCommands.o ./server/src/util.o ./server/src/vector.o ./server/src/message.o ./server/src/record.o -o serv

server.o: server.c
	gcc -Wall -c -g server.c 

serv_globals.o: globals.c 
	gcc -Wall -c -g server/src/globals.c

serverCommands.o: serverCommands.c 
	gcc -Wall -c -g server/src/serverCommands.c

deviceCommands.o: deviceCommands.c 
	gcc -Wall -c -g server/src/deviceCommands.c

util.o: util.c 
	gcc -Wall -c -g server/src/util.c

serv_vector.o: vector.c 
	gcc -Wall -c -g server/src/vector.c

message.o: message.c
	gcc -Wall -c -g server/src/message.c

record.o: record.c
	gcc -Wall -c -g server/src/record.c

# pulizia dei file della compilazione (eseguito con make clean)
clean:
	mkdir -p ./device/chat_user1
	mkdir -p ./device/chat_user2
	mkdir -p ./device/chat_user3
	rm *o dev serv
	rm ./server/src/*.o ./device/src/*.o
	echo "user2\nuser3" > device/contacts/user1.txt
	echo "user1\nuser3" > device/contacts/user2.txt
	echo "user1\nuser2" > device/contacts/user3.txt
	echo "" > server/documents/saved_messages.txt