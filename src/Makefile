all:	chatserver chatclient

CC=cc
CFLAGS= -Wall -g -I/usr/include/mysql `pkg-config --cflags --libs gtk+-2.0`

LIB_FILE=-lpthread -lmysqlclient

.c.o:
	$(CC) $(CFLAGS) $(DFLAGS) -c $<
	
db_ctrl.o: db_ctrl.c db_ctrl.h
chat_server.o: chat_server.c chat_server.h
chat_parse.o: chat_parse.c chat_parse.h
chat_socket.o: chat_socket.c chat_socket.h
chat_client.o: chat_client.c chat_client.h
chat_server.o: chat_server.c chat_server.h
interface.o: interface.c interface.h    
callback.o: callback.c callback.h    

chatclient: chat_client.o chat_parse.o chat_socket.o interface.o callback.o
	$(CC) -o chatclient  $(CFLAGS) chat_client.o chat_parse.o chat_socket.o interface.o callback.o `pkg-config --cflags --libs gtk+-2.0`

chatserver:	chat_server.o chat_parse.o chat_socket.o db_ctrl.o
	$(CC) -o chatserver $(CFLAGS) chat_server.o chat_parse.o chat_socket.o db_ctrl.o $(LIB_FILE)

clean:
	rm -f chatserver chatclient *.o *~
