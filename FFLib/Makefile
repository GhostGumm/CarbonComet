CC	=	gcc -g3 -std=c11

CFLAGS	=	`pkg-config --cflags --libs glib-2.0` `pkg-config --cflags --libs json-glib-1.0` -I./includes -I./includes/exts -I./includes/loops -arch x86_64 /usr/lib/libcurl.dylib `pkg-config --cflags --libs nopoll`

SRC	=	./main.c\
		./src/channel.c\
		./src/cometd.c\
		./src/conn.c\
		./src/ext.c\
		./src/event.c\
		./src/error.c\
		./src/inbox.c\
		./src/http.c\
		./src/json.c\
		./src/transport.c\
		./src/transport_long_polling.c\
		./src/loop.c\
		./src/msg.c\
		./src/exts/logger.c\
		./src/exts/yammer.c\
		./src/loops/gthread.c

SRS	=	./server.c\

all:
	$(CC) -o client $(CFLAGS) $(SRC)

server:
	$(CC) -o server $(CFLAGS) $(SRS)
