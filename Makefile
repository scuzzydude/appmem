CPPFLAGS=-Iinc
CC=gcc
all: appmemtest

#%.o:	%.c
#	  $(CC) $(CPPFLAGS)	-c	-o	$@	$^

appmemtest:	appmem_test.o appmemlib/am_flat.o	appmemlib/am_stata.o appmemlib/appmemlib.o appmemlib/appmem_virtd.o
	 $(CC) -lrt -o $@ $^


clean:
	rm -rf *.o appmemtest appmemlib/*.o
