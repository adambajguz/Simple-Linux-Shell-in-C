CFLAGS = -g -O2 -std=gnu11 # tutaj mo¿na dodawaæ inne flagi kompilatora
LIBS = -lm # tutaj mo¿na dodawaæ biblioteki

OBJ =\
	parser.o \
	filemanip.o \
	history.o \
	main.o

all: main

clean:
		rm -f *.o shell
.c.o:
		$(CC) -c $(INCLUDES) $(CFLAGS) $<

main: $(OBJ)
		$(CC) $(OBJ) $(LIBS) -o shell