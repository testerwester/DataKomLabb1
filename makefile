server : server.o 
	gcc -o server -Wall server.o 

client : client.o
	gcc -o client -Wall client.o