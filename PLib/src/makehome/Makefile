CC	=	gcc -arch x86_64 -g3 -I../  -I/usr/local/Cellar/jansson/2.7/include -I/usr/local/lib -lcurl -I/usr/local/include

SRC	=	../*.c

CFLAGS	=	`pkg-config --cflags --libs jansson`


all:
	$(CC) $(SRC) $(CFLAGS)
