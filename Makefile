CC=gcc
CFLAGS=-ansi -pedantic
EXEC=hy345sh

all: $(EXEC)

$(EXEC): hy345sh.c
	$(CC) $(CFLAGS) hy345sh.c -o $(EXEC)

clean:
	rm -f $(EXEC)
	rm -f $(EXEC) *.txt
