CC=gcc -m32
CXX=g++ -m32
CPP=gcc -E
AR=ar -crD
RANLIB=ar -r
LD=g++ -m32

CPPFLAGS=-Wall -Werror -g

all:convert-pinyin-table

convert-pinyin-table:convert-pinyin-table.o

clean:
	rm -f *.o  convert-pinyin-table
