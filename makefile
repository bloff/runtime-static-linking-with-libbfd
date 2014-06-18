TARGET = main test_unit.o
LIBS = -liberty -lz -lbfd
CC = gcc
CFLAGS = -fPIE -g
# -g -Wall

.PHONY: default all clean

default: $(TARGET)
all: default

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@


main: main.o
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS) 

clean:
	-rm -f *.o
	-rm -f $(TARGET)
	
	
# gcc -c test_unit.c -T simple_obj.ld -o test_unit.o -nostdlib -nostartfiles -nodefaultlibs	
# objcopy --remove-section .eh_frame --remove-section .comment --remove-section .note.GNU-stack test_unit.o test_unit.o