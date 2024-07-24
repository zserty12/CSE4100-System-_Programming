CC = gcc
target = testlib
objects = main.o list.o hash.o bitmap.o hex_dump.o

$(target): $(objects)
	$(CC) -o $(target) $(objects)

$(objects) : list.h hash.h bitmap.h hex_dump.h

.PHONY : clean
clean :
	rm $(target) $(objects)
