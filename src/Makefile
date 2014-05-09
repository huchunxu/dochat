all:	chatserver chatclient

CC=cc
CFLAGS= -Wall -g

LIB_FILE=-lpthread

.c.o:
	$(CC) $(CFLAGS) $(DFLAGS) -c $<

chat_server.o: chat_server.c chat.h
chat_parse.o: chat_parse.c chat.h
chat_socket.o: chat_socket.c chat.h
chat_client.o: chat_client.c chat.h
chat_server.o: chat_server.c chat.h

chatclient: chat_client.o chat_parse.o chat_socket.o
	$(CC) -o chatclient  $(DFLAGS) chat_client.o chat_parse.o chat_socket.o

chatserver:	chat_server.o chat_parse.o chat_socket.o
	$(CC) -o chatserver $(DFLAGS) chat_server.o chat_parse.o chat_socket.o $(LIB_FILE)

clean:
	rm -f chatserver chatclient *.o *~
